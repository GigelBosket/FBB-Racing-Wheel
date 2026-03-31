#ifndef CURRENT_SENSE_H
#define CURRENT_SENSE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    float ia_amps;  // Phase A current (amps)
    float ib_amps;  // Phase B current (amps)
    float ic_amps;  // Phase C current (amps)
    float i_alpha;  // Clarke transform alpha component
    float i_beta;   // Clarke transform beta component
    bool over_current;  // Over-current flag
} current_sense_state_t;

// Initialize ADC for current sensing (SOA, SOB, SOC)
void current_sense_init(void);

// Read raw ADC values and apply Clarke transform
// Should be called synchronized with PWM (e.g., when low-side MOSFET is ON)
void current_sense_update(void);

// Get current state
current_sense_state_t current_sense_get_state(void);

// Convert raw ADC value to current in amps
float current_sense_adc_to_amps(uint16_t adc_value);

// Clarke Transform: Convert Phase A, B, C to Alpha, Beta
// Input: 3-phase currents
// Output: 2-phase (α,β) representation
void clarke_transform(float ia, float ib, float ic, float *i_alpha, float *i_beta);

// Inverse Clarke Transform: Convert Alpha, Beta back to Phase currents
// Useful for debugging
void inv_clarke_transform(float i_alpha, float i_beta, float *ia, float *ib, float *ic);

#endif // CURRENT_SENSE_H
