#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// ============ GPIO PIN DEFINITIONS ============
// Motor Phase Control (PWM outputs to DRV8323)
#define GPIO_INLA 16    // Phase A Low-side
#define GPIO_INHA 17    // Phase A High-side
#define GPIO_INLB 14    // Phase B Low-side
#define GPIO_INHB 15    // Phase B High-side
#define GPIO_INLC 12    // Phase C Low-side
#define GPIO_INHC 13    // Phase C High-side

// DRV8323 SPI Interface
#define GPIO_MOSI 19
#define GPIO_MISO 20
#define GPIO_CSN  21
#define GPIO_SCK  22

// Encoder Inputs (Quadrature)
#define GPIO_ENC_A 23
#define GPIO_ENC_B 24
#define GPIO_ENC_Z 25   // Index pulse (optional calibration)

// Current Sense Inputs (to DRV8323 SOA, SOB, SOC)
#define GPIO_CUR_A 26
#define GPIO_CUR_B 27
#define GPIO_CUR_C 28

// Overvoltage & Power Management
#define GPIO_VIN_MON   29   // Voltage monitor (24V / 10 = 2.4V at ADC)
#define GPIO_MOSFET_EN 11   // Enable dissipation resistor when overvoltage detected

// DRV8323 Control
#define GPIO_DRV_EN 18      // Enable (set high always)
#define GPIO_DRV_FAULT 10   // Fault input (optional, for safety)

// ============ SYSTEM CONFIGURATION ============
#define PWM_FREQUENCY 20000 // 20kHz PWM frequency
#define FOC_LOOP_FREQ 20000 // 20kHz FOC control loop
#define USB_POLL_FREQ 1000  // 1kHz USB polling on Core 0

// ============ MOTOR & ELECTRICAL PARAMETERS ============
#define MOTOR_POLE_PAIRS 15         // Hoverboard motor typically has 15-20 pole pairs
#define MOTOR_MAX_DEGREES 900       // ±450° rotation (typical racing wheel)
#define ENCODER_CPR 3000            // Counts per revolution (depends on physical encoder)
#define ENCODER_ELECTRICAL_OFFSET_POS_OFFSET 0  // Calibrated at startup with Z-index

// ============ CONTROL LOOP PARAMETERS ============
#define IQ_PID_KP 0.5f      // Proportional gain for current loop
#define IQ_PID_KI 0.1f      // Integral gain
#define IQ_PID_KD 0.01f     // Derivative gain

#define POSITION_PID_KP 0.3f  // For position tracking (if used)
#define POSITION_PID_KI 0.05f
#define POSITION_PID_KD 0.02f

#define TORQUE_SLEW_RATE 50.0f  // Limit rapid torque changes (A/ms) for safety

// ============ CURRENT SENSE PARAMETERS ============
#define DRV8323_SHUNT_RESISTOR 0.01f  // 10mΩ shunt resistor (ohms)
#define DRV8323_GAIN 20.0f             // Internal amplifier gain (typical)
#define ADC_VREF 3.3f                  // ADC reference voltage
#define ADC_RESOLUTION 4096.0f         // 12-bit ADC = 4096 levels

#define CURRENT_SENSE_MAX_MA 40000     // 40A max (in mA)
#define CURRENT_SENSE_WARN_MA 30000    // 30A warning threshold
#define CURRENT_SENSE_CUTOFF_MA 35000  // 35A hard cutoff

// ============ VOLTAGE MONITORING ============
#define VBATT_NOMINAL 24.0f         // Nominal battery voltage
#define VBATT_OVERVOLT_THRESH 25.0f // Overvoltage threshold for dissipation resistor
#define VBATT_DIVIDER_RATIO 10.0f    // 24V / 10 = 2.4V at ADC pin
#define VBATT_ADC_PIN 29

// ============ USB HID PARAMETERS ============
#define USB_VENDOR_ID 0x1234
#define USB_PRODUCT_ID 0x5678
#define USB_MANUFACTURER "DIY"
#define USB_PRODUCT "Direct Drive Wheel"
#define USB_SERIAL "DD-WHEEL-001"

// ============ EFFECT PARAMETERS ============
#define MAX_EFFECTS 4  // Number of simultaneous effects supported
#define EFFECT_CONSTANT_FORCE 0x01
#define EFFECT_SPRING 0x02
#define EFFECT_DAMPER 0x03
#define EFFECT_FRICTION 0x04

// ============ SAFETY LIMITS ============
#define WATCHDOG_TIMEOUT_MS 1000  // 1 second watchdog for USB communication
#define CALIBRATION_TIMEOUT_MS 5000 // 5 seconds for startup calibration

#endif // CONFIG_H
