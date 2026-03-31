#include "foc_controller.h"
#include "config.h"
#include <math.h>
#include <string.h>

static foc_state_t foc_state = {0};
static pid_controller_t iq_pid = {0};
static pid_controller_t id_pid = {0};

#define SQRT3 1.732050808f
#define TWO_OVER_SQRT3 1.154700538f

void foc_init(void) {
    memset(&foc_state, 0, sizeof(foc_state));
    
    // Initialize Iq PID controller (current loop)
    pid_init(&iq_pid, IQ_PID_KP, IQ_PID_KI, IQ_PID_KD);
    
    // Initialize Id PID controller (typically set to 0 for max torque)
    pid_init(&id_pid, 0.5f, 0.1f, 0.01f);
    
    foc_state.id_target = 0.0f;    // No field weakening, keep Id = 0
    foc_state.iq_target = 0.0f;
}

void park_transform(float i_alpha, float i_beta, float theta, float *id, float *iq) {
    // Park Transform: converts αβ frame to dq frame (rotor-aligned)
    // d-axis: aligned with rotor flux
    // q-axis: perpendicular to d-axis (torque-producing current)
    //
    // Park equations:
    // id = i_alpha * cos(theta) + i_beta * sin(theta)
    // iq = -i_alpha * sin(theta) + i_beta * cos(theta)
    
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);
    
    *id = i_alpha * cos_theta + i_beta * sin_theta;
    *iq = -i_alpha * sin_theta + i_beta * cos_theta;
}

void inv_park_transform(float vd, float vq, float theta, float *v_alpha, float *v_beta) {
    // Inverse Park Transform: converts dq frame back to αβ frame
    //
    // Inverse Park equations:
    // v_alpha = vd * cos(theta) - vq * sin(theta)
    // v_beta = vd * sin(theta) + vq * cos(theta)
    
    float cos_theta = cosf(theta);
    float sin_theta = sinf(theta);
    
    *v_alpha = vd * cos_theta - vq * sin_theta;
    *v_beta = vd * sin_theta + vq * cos_theta;
}

void svpwm(float v_alpha, float v_beta, float *duty_a, float *duty_b, float *duty_c) {
    // Space Vector PWM (SVPWM) modulator
    // Converts 2-phase (alpha-beta) voltage to 3-phase PWM duty cycles
    // This is the standard 60-degree sector method
    
    // Step 1: Calculate sector and time durations
    // Project αβ voltage to 3-phase reference
    float v_a = v_alpha;
    float v_b = -0.5f * v_alpha + (SQRT3 / 2.0f) * v_beta;
    float v_c = -0.5f * v_alpha - (SQRT3 / 2.0f) * v_beta;
    
    // Step 2: Normalize voltage (max = 1.0 represents DC-link voltage)
    float v_ref_mag = sqrtf(v_alpha * v_alpha + v_beta * v_beta);
    if (v_ref_mag > 1.0f) {
        v_a /= v_ref_mag;
        v_b /= v_ref_mag;
        v_c /= v_ref_mag;
    }
    
    // Step 3: Convert to duty cycles (map from [-Vdc/2, Vdc/2] to [0, 1])
    // Add 0.5 offset to shift from [-0.5, 0.5] range to [0, 1] range
    *duty_a = 0.5f + v_a * 0.5f;
    *duty_b = 0.5f + v_b * 0.5f;
    *duty_c = 0.5f + v_c * 0.5f;
    
    // Clamp to [0, 1]
    if (*duty_a > 1.0f) *duty_a = 1.0f;
    if (*duty_a < 0.0f) *duty_a = 0.0f;
    if (*duty_b > 1.0f) *duty_b = 1.0f;
    if (*duty_b < 0.0f) *duty_b = 0.0f;
    if (*duty_c > 1.0f) *duty_c = 1.0f;
    if (*duty_c < 0.0f) *duty_c = 0.0f;
}

void pid_init(pid_controller_t *pid, float kp, float ki, float kd) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output = 0.0f;
}

float pid_update(pid_controller_t *pid, float error, float dt) {
    // Proportional term
    float p_term = pid->kp * error;
    
    // Integral term
    pid->integral += pid->ki * error * dt;
    // Anti-windup: limit integral term
    if (pid->integral > 10.0f) pid->integral = 10.0f;
    if (pid->integral < -10.0f) pid->integral = -10.0f;
    
    // Derivative term
    float d_term = 0.0f;
    if (dt > 0.0f) {
        d_term = pid->kd * (error - pid->prev_error) / dt;
    }
    pid->prev_error = error;
    
    // Total output
    pid->output = p_term + pid->integral + d_term;
    
    return pid->output;
}

void pid_reset(pid_controller_t *pid) {
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output = 0.0f;
}

void foc_update(float theta_elec, float i_alpha, float i_beta, float dt_seconds) {
    // Store electrical angle
    foc_state.theta_elec = theta_elec;
    
    // Step 1: Clarke Transform (already done externally, but we need alpha-beta)
    // Assume alpha-beta currents are passed in
    
    // Step 2: Park Transform - Convert αβ to dq
    park_transform(i_alpha, i_beta, theta_elec, 
                   &foc_state.id_measured, &foc_state.iq_measured);
    
    // Step 3: PI Current Control - Iq loop (torque control)
    float iq_error = foc_state.iq_target - foc_state.iq_measured;
    foc_state.vq = pid_update(&iq_pid, iq_error, dt_seconds);
    
    // Step 4: PI Current Control - Id loop (flux control)
    float id_error = foc_state.id_target - foc_state.id_measured;
    foc_state.vd = pid_update(&id_pid, id_error, dt_seconds);
    
    // Step 5: Inverse Park Transform - Convert dq to αβ
    inv_park_transform(foc_state.vd, foc_state.vq, theta_elec,
                       &foc_state.vout_alpha, &foc_state.vout_beta);
    
    // Step 6: SVPWM Modulation - Convert αβ to 3-phase duty cycles
    // Note: SVPWM is called from motor_control module
}

foc_state_t foc_get_state(void) {
    return foc_state;
}

void foc_set_iq_target(float iq_amps) {
    // Implement torque slew rate limiting for safety
    static float last_iq_target = 0.0f;
    float max_change = TORQUE_SLEW_RATE;  // Amps per millisecond
    
    float change = iq_amps - last_iq_target;
    if (change > max_change) {
        iq_amps = last_iq_target + max_change;
    } else if (change < -max_change) {
        iq_amps = last_iq_target - max_change;
    }
    
    foc_state.iq_target = iq_amps;
    last_iq_target = iq_amps;
}
