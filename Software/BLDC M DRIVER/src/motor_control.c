#include "motor_control.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"

static pwm_duty_t current_duty = {0};
static bool motor_enabled = false;

// PWM slice information
static uint slice_a, slice_b, slice_c;

void motor_control_init(void) {
    // Initialize GPIO for PWM outputs
    // Phase A
    gpio_set_function(GPIO_INLA, GPIO_FUNC_PWM);
    gpio_set_function(GPIO_INHA, GPIO_FUNC_PWM);
    slice_a = pwm_gpio_to_slice_num(GPIO_INLA);
    
    // Phase B
    gpio_set_function(GPIO_INLB, GPIO_FUNC_PWM);
    gpio_set_function(GPIO_INHB, GPIO_FUNC_PWM);
    slice_b = pwm_gpio_to_slice_num(GPIO_INLB);
    
    // Phase C
    gpio_set_function(GPIO_INLC, GPIO_FUNC_PWM);
    gpio_set_function(GPIO_INHC, GPIO_FUNC_PWM);
    slice_c = pwm_gpio_to_slice_num(GPIO_INLC);
    
    // Configure PWM frequency
    // Default clock is 125MHz
    // For 20kHz: divider = 125MHz / (20kHz * 4096) ≈ 1.5
    pwm_set_clkdiv(slice_a, 1.5f);
    pwm_set_clkdiv(slice_b, 1.5f);
    pwm_set_clkdiv(slice_c, 1.5f);
    
    // Set PWM wrap (top value)
    uint16_t wrap_val = 4096 - 1;
    pwm_set_wrap(slice_a, wrap_val);
    pwm_set_wrap(slice_b, wrap_val);
    pwm_set_wrap(slice_c, wrap_val);
    
    // Enable PWM slices
    pwm_set_enabled(slice_a, true);
    pwm_set_enabled(slice_b, true);
    pwm_set_enabled(slice_c, true);
    
    motor_enabled = true;
}

void motor_control_set_duty(float duty_a, float duty_b, float duty_c) {
    // Clamp duty cycles to [-1.0, 1.0]
    if (duty_a > 1.0f) duty_a = 1.0f;
    if (duty_a < -1.0f) duty_a = -1.0f;
    if (duty_b > 1.0f) duty_b = 1.0f;
    if (duty_b < -1.0f) duty_b = -1.0f;
    if (duty_c > 1.0f) duty_c = 1.0f;
    if (duty_c < -1.0f) duty_c = -1.0f;
    
    current_duty.duty_a = duty_a;
    current_duty.duty_b = duty_b;
    current_duty.duty_c = duty_c;
    
    if (!motor_enabled) {
        pwm_set_chan_level(slice_a, PWM_CHAN_A, 0);
        pwm_set_chan_level(slice_a, PWM_CHAN_B, 0);
        pwm_set_chan_level(slice_b, PWM_CHAN_A, 0);
        pwm_set_chan_level(slice_b, PWM_CHAN_B, 0);
        pwm_set_chan_level(slice_c, PWM_CHAN_A, 0);
        pwm_set_chan_level(slice_c, PWM_CHAN_B, 0);
        return;
    }
    
    // Convert duty cycles to PWM levels
    // Positive duty = High side active
    // Negative duty = Low side active
    uint16_t wrap_val = 4095;
    
    // Phase A (INHA on GPIO_INHA, INLA on GPIO_INLA)
    uint16_t level_a_h = (duty_a > 0) ? (uint16_t)(duty_a * wrap_val) : 0;
    uint16_t level_a_l = (duty_a < 0) ? (uint16_t)((-duty_a) * wrap_val) : 0;
    pwm_set_chan_level(slice_a, PWM_CHAN_A, level_a_h);  // GPIO_INHA
    pwm_set_chan_level(slice_a, PWM_CHAN_B, level_a_l);  // GPIO_INLA
    
    // Phase B (INHB on GPIO_INHB, INLB on GPIO_INLB)
    uint16_t level_b_h = (duty_b > 0) ? (uint16_t)(duty_b * wrap_val) : 0;
    uint16_t level_b_l = (duty_b < 0) ? (uint16_t)((-duty_b) * wrap_val) : 0;
    pwm_set_chan_level(slice_b, PWM_CHAN_A, level_b_h);  // GPIO_INHB
    pwm_set_chan_level(slice_b, PWM_CHAN_B, level_b_l);  // GPIO_INLB
    
    // Phase C (INHC on GPIO_INHC, INLC on GPIO_INLC)
    uint16_t level_c_h = (duty_c > 0) ? (uint16_t)(duty_c * wrap_val) : 0;
    uint16_t level_c_l = (duty_c < 0) ? (uint16_t)((-duty_c) * wrap_val) : 0;
    pwm_set_chan_level(slice_c, PWM_CHAN_A, level_c_h);  // GPIO_INHC
    pwm_set_chan_level(slice_c, PWM_CHAN_B, level_c_l);  // GPIO_INLC
}

void motor_control_set_duty_struct(const pwm_duty_t *duties) {
    motor_control_set_duty(duties->duty_a, duties->duty_b, duties->duty_c);
}

void motor_control_stop(void) {
    motor_control_set_duty(0, 0, 0);
}

pwm_duty_t motor_control_get_duty(void) {
    return current_duty;
}

void motor_control_enable(bool enable) {
    motor_enabled = enable;
    if (!enable) {
        motor_control_stop();
    }
}
