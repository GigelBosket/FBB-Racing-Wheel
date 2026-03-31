#include "encoder_driver.h"
#include "config.h"
#include "pico/stdlib.h"
#include "pico/time.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/sync.h"
#include <math.h>

static encoder_state_t encoder_state = {0};
static volatile int32_t encoder_count = 0;
static volatile int32_t last_state = 0;

// Transition table for quadrature decoding (2-bit Gray code)
// Old state: bits 1-0 = {B, A}, New state: bits 3-2 = {B, A}
static const int8_t quadrature_table[16] = {
    0,  1, -1,  0,
   -1,  0,  0,  1,
    1,  0,  0, -1,
    0, -1,  1,  0
};

void gpio_callback(uint gpio, uint32_t events) {
    uint32_t current_state = (gpio_get(GPIO_ENC_B) << 1) | gpio_get(GPIO_ENC_A);
    
    // Get transition index
    int transition = (last_state << 2) | current_state;
    encoder_count += quadrature_table[transition & 0x0F];
    
    last_state = current_state;
    
    // Check for Z (index) pulse
    if (gpio == GPIO_ENC_Z && (events & GPIO_IRQ_EDGE_RISE)) {
        encoder_state.z_detected = true;
    }
}

void encoder_init(void) {
    // Initialize encoder pins as inputs
    gpio_init(GPIO_ENC_A);
    gpio_init(GPIO_ENC_B);
    gpio_init(GPIO_ENC_Z);
    
    gpio_set_dir(GPIO_ENC_A, GPIO_IN);
    gpio_set_dir(GPIO_ENC_B, GPIO_IN);
    gpio_set_dir(GPIO_ENC_Z, GPIO_IN);
    
    // Enable pull-ups for encoder
    gpio_pull_up(GPIO_ENC_A);
    gpio_pull_up(GPIO_ENC_B);
    gpio_pull_up(GPIO_ENC_Z);
    
    // Set up interrupt callbacks
    gpio_set_irq_enabled_with_callback(GPIO_ENC_A, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled(GPIO_ENC_B, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
    gpio_set_irq_enabled(GPIO_ENC_Z, GPIO_IRQ_EDGE_RISE, true);
    
    // Initial state
    last_state = (gpio_get(GPIO_ENC_B) << 1) | gpio_get(GPIO_ENC_A);
}

void encoder_update(void) {
    // Convert raw count to angles
    // Mechanical angle: one full rotation = ENCODER_CPR counts
    encoder_state.mechanical_angle_rad = (encoder_count / (float)ENCODER_CPR) * 2.0f * M_PI;
    
    // Electrical angle: depends on motor pole pairs
    float raw_electrical = encoder_state.mechanical_angle_rad * MOTOR_POLE_PAIRS;
    
    // Normalize to 0 to 2π range
    encoder_state.electrical_angle_rad = fmodf(raw_electrical, 2.0f * M_PI);
    if (encoder_state.electrical_angle_rad < 0) {
        encoder_state.electrical_angle_rad += 2.0f * M_PI;
    }
    
    // Apply calibration offset (set during startup with Z-index)
    encoder_state.electrical_angle_rad += encoder_state.calibration_offset;
    if (encoder_state.electrical_angle_rad >= 2.0f * M_PI) {
        encoder_state.electrical_angle_rad -= 2.0f * M_PI;
    }
}

encoder_state_t encoder_get_state(void) {
    return encoder_state;
}

void encoder_calibrate_electrical_offset(void) {
    // Wait for Z-index pulse or timeout
    encoder_state.z_detected = false;
    absolute_time_t start_time = get_absolute_time();
    
    while (!encoder_state.z_detected && 
           (absolute_time_diff_us(start_time, get_absolute_time()) < 5000000)) {
        sleep_ms(1);
    }
    
    if (encoder_state.z_detected) {
        // On Z-index, set electrical angle to 0 as reference
        encoder_state.calibration_offset = -encoder_state.electrical_angle_rad;
    } else {
        // No Z-index found, use current position as reference
        encoder_state.calibration_offset = 0;
    }
}

void encoder_reset_count(void) {
    uint32_t save = save_and_disable_interrupts();
    encoder_count = 0;
    restore_interrupts(save);
}

void encoder_set_count(int32_t count) {
    uint32_t save = save_and_disable_interrupts();
    encoder_count = count;
    restore_interrupts(save);
}
