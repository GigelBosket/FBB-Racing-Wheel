#ifndef FOC_CONTROLLER_H
#define FOC_CONTROLLER_H

#include <stdint.h>
#include <math.h>

typedef struct {
    float kp, ki, kd;
    float integral;
    float prev_error;
    float output;
} pid_controller_t;

typedef struct {
    float theta_elec;    // Electrical rotor angle (radians, 0 to 2π)
    float iq_target;     // Target quadrature current (amps) - sets torque
    float id_target;     // Target direct current (typically 0 for max torque)
    float iq_measured;   // Measured quadrature current
    float id_measured;   // Measured direct current
    float vd, vq;        // Control voltage outputs (Park frame)
    float vout_alpha, vout_beta; // Inverse Park transform output
} foc_state_t;

// Initialize FOC controller
void foc_init(void);

// Main FOC control loop (should run at 20kHz)
// Takes measured currents and electrical angle, outputs voltage commands
void foc_update(float theta_elec, float i_alpha, float i_beta, float dt_seconds);

// Get current FOC state
foc_state_t foc_get_state(void);

// Set target torque current (Iq command)
void foc_set_iq_target(float iq_amps);

// Park Transform: Convert αβ frame to dq frame (rotor-aligned)
// Input: alpha, beta currents and electrical angle theta
// Output: direct (d) and quadrature (q) components
void park_transform(float i_alpha, float i_beta, float theta, float *id, float *iq);

// Inverse Park Transform: Convert dq frame back to αβ frame
// Input: direct, quadrature voltages and electrical angle theta
// Output: alpha, beta voltages
void inv_park_transform(float vd, float vq, float theta, float *v_alpha, float *v_beta);

// SVPWM (Space Vector PWM) modulator
// Takes alpha, beta voltages and outputs PWM duty cycles for three phases
void svpwm(float v_alpha, float v_beta, float *duty_a, float *duty_b, float *duty_c);

// PID controller update
float pid_update(pid_controller_t *pid, float error, float dt);

// Initialize PID controller
void pid_init(pid_controller_t *pid, float kp, float ki, float kd);

// Reset PID state (e.g., on fault)
void pid_reset(pid_controller_t *pid);

#endif // FOC_CONTROLLER_H
