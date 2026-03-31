#ifndef ENCODER_DRIVER_H
#define ENCODER_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int32_t raw_count;           // Raw encoder count
    float electrical_angle_rad;  // Electrical angle in radians (0 to 2π)
    float mechanical_angle_rad;  // Mechanical angle in radians
    float velocity_rad_per_sec;  // Angular velocity
    bool z_detected;             // Z-index pulse detected
    float calibration_offset;    // Electrical angle offset from mechanical zero
} encoder_state_t;

// Initialize encoder (quadrature decoder)
void encoder_init(void);

// Poll encoder and update state (call frequently)
void encoder_update(void);

// Get current encoder state
encoder_state_t encoder_get_state(void);

// Calibrate electrical offset using Z-index pulse
void encoder_calibrate_electrical_offset(void);

// Reset encoder count to zero
void encoder_reset_count(void);

// Set encoder count directly (useful for test/debug)
void encoder_set_count(int32_t count);

#endif // ENCODER_DRIVER_H
