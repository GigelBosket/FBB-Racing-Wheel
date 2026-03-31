#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/time.h"
#include "hardware/irq.h"
#include "hardware/timer.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"

#include "config.h"
#include "encoder_driver.h"
#include "current_sense.h"
#include "foc_controller.h"
#include "motor_control.h"
#include "drv8323_driver.h"
#include "usb_hid_device.h"

// Synchronization flags and shared data
volatile float g_target_iq = 0.0f;  // Target current from USB HID
volatile float g_wheel_position = 0.0f;  // Current wheel position
volatile float g_wheel_velocity = 0.0f;  // Current wheel angular velocity
volatile uint32_t g_fault_flags = 0;  // System fault flags

#define FAULT_FLAG_OVERCURRENT (1 << 0)
#define FAULT_FLAG_OVERVOLTAGE (1 << 1)
#define FAULT_FLAG_DRV_FAULT (1 << 2)
#define FAULT_FLAG_USB_TIMEOUT (1 << 3)

// Watchdog timeout for USB communication (Core 0 should update this regularly)
static uint32_t usb_last_update_time = 0;
static uint32_t foc_loop_count = 0;

// Voltage monitoring
static void voltage_monitor_init(void) {
    gpio_init(GPIO_MOSFET_EN);
    gpio_set_dir(GPIO_MOSFET_EN, GPIO_OUT);
    gpio_put(GPIO_MOSFET_EN, 0);  // Dissipation resistor off by default
}

static float voltage_monitor_read(void) {
    adc_select_input(3);  // GPIO 29 is ADC input 3
    uint16_t adc_value = adc_read();
    
    // Convert ADC to voltage
    float v_adc = (adc_value / 4096.0f) * 3.3f;
    
    // Account for divider (10x divider from 24V line)
    float v_battery = v_adc * VBATT_DIVIDER_RATIO;
    
    return v_battery;
}

static void voltage_monitor_check(void) {
    static uint32_t monitor_timer = 0;
    
    // Check voltage periodically (every 100ms)
    uint32_t now_us = time_us_32();
    if ((now_us - monitor_timer) < 100000) return;
    monitor_timer = now_us;
    
    float v_batt = voltage_monitor_read();
    
    if (v_batt > VBATT_OVERVOLT_THRESH) {
        // Overvoltage detected - activate dissipation resistor
        gpio_put(GPIO_MOSFET_EN, 1);
        g_fault_flags |= FAULT_FLAG_OVERVOLTAGE;
    } else if (v_batt < VBATT_OVERVOLT_THRESH - 1.0f) {
        // Overvoltage cleared
        gpio_put(GPIO_MOSFET_EN, 0);
        g_fault_flags &= ~FAULT_FLAG_OVERVOLTAGE;
    }
}

// ============================================================================
// CORE 1 - High-speed FOC Control Loop (20kHz)
// ============================================================================
static void core1_main(void) {
    printf("[Core 1] Initializing motor control...\n");
    
    // Initialize all hardware drivers
    motor_control_init();
    drv8323_init();
    current_sense_init();
    encoder_init();
    foc_init();
    voltage_monitor_init();
    
    printf("[Core 1] Hardware initialized. Starting FOC loop...\n");
    
    // Start the FOC timer (20kHz)
    uint32_t loop_period_us = 1000000 / FOC_LOOP_FREQ;  // 50µs for 20kHz
    uint32_t last_update_us = time_us_32();
    
    float prev_position = 0.0f;
    uint32_t prev_time_us = time_us_32();
    
    while (1) {
        uint32_t now_us = time_us_32();
        uint32_t elapsed_us = now_us - prev_time_us;
        
        // Check if it's time for the next FOC update (every ~50µs)
        if (elapsed_us < loop_period_us) {
            continue;  // Wait for next cycle
        }
        
        float dt_seconds = elapsed_us / 1e6f;
        prev_time_us = now_us;
        
        // ---- FOC Control Loop (runs at 20kHz) ----
        
        // 1. Read encoder position and velocity
        encoder_update();
        encoder_state_t enc = encoder_get_state();
        
        // Calculate velocity (rad/s)
        float position_delta = enc.mechanical_angle_rad - prev_position;
        if (position_delta > M_PI) position_delta -= 2.0f * M_PI;
        if (position_delta < -M_PI) position_delta += 2.0f * M_PI;
        g_wheel_velocity = position_delta / dt_seconds;
        prev_position = enc.mechanical_angle_rad;
        
        g_wheel_position = enc.electrical_angle_rad;
        
        // 2. Read phase currents (must sync with PWM)
        current_sense_update();
        current_sense_state_t curr = current_sense_get_state();
        
        // Check over-current fault
        if (curr.over_current) {
            g_fault_flags |= FAULT_FLAG_OVERCURRENT;
            motor_control_stop();
        } else {
            g_fault_flags &= ~FAULT_FLAG_OVERCURRENT;
        }
        
        // 3. Run FOC controller (Clarke + Park + PI + InvPark + SVPWM)
        if (!(g_fault_flags & (FAULT_FLAG_OVERCURRENT | FAULT_FLAG_OVERVOLTAGE))) {
            foc_update(enc.electrical_angle_rad, curr.i_alpha, curr.i_beta, dt_seconds);
            foc_state_t foc = foc_get_state();
            
            // Get SVPWM duty cycles
            float duty_a, duty_b, duty_c;
            svpwm(foc.vout_alpha, foc.vout_beta, &duty_a, &duty_b, &duty_c);
            
            // Apply to motor
            motor_control_set_duty(duty_a, duty_b, duty_c);
        }
        
        // 4. Check DRV8323 faults
        drv8323_fault_t drv_fault = drv8323_get_fault_status();
        if (drv_fault.fault) {
            g_fault_flags |= FAULT_FLAG_DRV_FAULT;
            motor_control_stop();
        }
        
        // 5. Voltage monitoring
        voltage_monitor_check();
        
        foc_loop_count++;
    }
}

// ============================================================================
// CORE 0 - USB HID Communication & Logic (1kHz)
// ============================================================================
int main(void) {
    stdio_init_all();
    
    printf("\n=== BLDC Motor Driver with FOC - RP2354 ===\n");
    printf("Assetto Corsa Force Feedback Wheel\n\n");
    
    // Initialize ADC for voltage monitoring on both cores
    adc_init();
    adc_gpio_init(GPIO_VIN_MON);
    
    // Launch Core 1 with the high-speed FOC loop
    printf("[Core 0] Launching Core 1 (FOC control)...\n");
    multicore_launch_core1(core1_main);
    
    // Initialize USB HID device
    printf("[Core 0] Initializing USB HID device...\n");
    usb_hid_init();
    
    sleep_ms(200);  // Wait for Core 1 to initialize
    
    printf("[Core 0] System ready! Waiting for USB connection...\n");
    printf("[Core 0] Connect to Assetto Corsa to begin\n\n");
    
    // Core 0 main loop (1kHz USB communication)
    uint32_t loop_counter = 0;
    uint32_t last_status_print_ms = 0;
    usb_last_update_time = to_ms_since_boot(get_absolute_time());
    
    while (1) {
        // Process USB HID communication
        usb_hid_process();
        usb_last_update_time = to_ms_since_boot(get_absolute_time());
        
        // Check USB connection status
        if (!usb_hid_is_connected()) {
            // USB disconnected - zero out commands for safety
            g_target_iq = 0.0f;
            foc_set_iq_target(0.0f);
        } else {
            // Calculate torque command from USB effects
            float iq_cmd = usb_hid_calculate_torque_command(g_wheel_position, g_wheel_velocity);
            g_target_iq = iq_cmd;
            foc_set_iq_target(iq_cmd);
        }
        
        // Send status back to host
        uint16_t status_flags = (g_fault_flags & 0xFF);
        usb_hid_send_status(g_wheel_position, status_flags);
        
        // Print status periodically (every 500ms)
        absolute_time_t now = get_absolute_time();
        uint32_t now_ms = to_ms_since_boot(now);
        if ((now_ms - last_status_print_ms) >= 500) {
            last_status_print_ms = now_ms;
            
            encoder_state_t enc = encoder_get_state();
            current_sense_state_t curr = current_sense_get_state();
            foc_state_t foc = foc_get_state();
            
            printf("[%05ld] ", foc_loop_count / 20000);  // FOC cycles / frequency
            printf("Pos: %6.1f° | Vel: %7.1f°/s | ", 
                   (enc.mechanical_angle_rad * 180.0f / M_PI),
                   (g_wheel_velocity * 180.0f / M_PI));
            printf("Iq: %6.2f/%6.2f A | ", foc.iq_measured, foc.iq_target);
            printf("USB: %s | ", usb_hid_is_connected() ? "CONN" : "DISC");
            printf("Fault: 0x%02X\n", status_flags);
            
            // Read and print battery voltage
            float v_batt = voltage_monitor_read();
            printf("       Vbatt: %.1fV | Ia: %6.1f A | Ib: %6.1f A | Ic: %6.1f A\n",
                   v_batt, curr.ia_amps, curr.ib_amps, curr.ic_amps);
        }
        
        // Check watchdog timeout
        uint32_t time_since_usb = now_ms - usb_last_update_time;
        if (time_since_usb > WATCHDOG_TIMEOUT_MS) {
            g_fault_flags |= FAULT_FLAG_USB_TIMEOUT;
            foc_set_iq_target(0.0f);  // Safety: zero torque on timeout
        }
        
        // Sleep for ~1ms (1kHz loop)
        sleep_ms(1);
        loop_counter++;
    }
    
    return 0;
}
