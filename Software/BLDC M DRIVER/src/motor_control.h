#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float duty_a;  // PWM duty cycle for phase A (-1.0 to 1.0, where 0 = off, 1.0 = full on)
    float duty_b;  // PWM duty cycle for phase B
    float duty_c;  // PWM duty cycle for phase C
} pwm_duty_t;

// Initialize PWM channels for all 6 gates (3 high, 3 low)
void motor_control_init(void);

// Set PWM duty cycles for all three phases
// Values should be in range [-1.0, 1.0]
// Negative values apply to low-side, positive to high-side (simplified bridge control)
void motor_control_set_duty(float duty_a, float duty_b, float duty_c);

// Set using calculated duty cycles structure
void motor_control_set_duty_struct(const pwm_duty_t *duties);

// Stop motor (all PWM to zero)
void motor_control_stop(void);

// Get current PWM duty cycles
pwm_duty_t motor_control_get_duty(void);

// Enable/disable motor output
void motor_control_enable(bool enable);

#endif // MOTOR_CONTROL_H
