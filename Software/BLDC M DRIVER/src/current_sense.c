#include "current_sense.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include <math.h>

static current_sense_state_t current_state = {0};

// ADC channel mappings for RP2354
// GPIO 26, 27, 28 map to ADC0, ADC1, ADC2
#define ADC_CHANNEL_A 0  // GPIO_CUR_A (GPIO 26)
#define ADC_CHANNEL_B 1  // GPIO_CUR_B (GPIO 27)
#define ADC_CHANNEL_C 2  // GPIO_CUR_C (GPIO 28)

void current_sense_init(void) {
    // Initialize ADC
    adc_init();
    
    // Initialize GPIO for ADC inputs
    adc_gpio_init(GPIO_CUR_A);
    adc_gpio_init(GPIO_CUR_B);
    adc_gpio_init(GPIO_CUR_C);
    
    // Set ADC clock divider (default is fine for most applications)
    // adc_set_clkdiv(0);
}

float current_sense_adc_to_amps(uint16_t adc_value) {
    // Convert ADC reading to voltage
    float v_adc = (adc_value / ADC_RESOLUTION) * ADC_VREF;
    
    // Subtract reference (typically Vref/2 = 1.65V)
    float v_out = v_adc - (ADC_VREF / 2.0f);
    
    // Formula: Current = (V_out - V_ref/2) / (Gain * R_shunt)
    // Positive value indicates current flowing through low-side MOSFET
    float current_amps = v_out / (DRV8323_GAIN * DRV8323_SHUNT_RESISTOR);
    
    return current_amps;
}

void clarke_transform(float ia, float ib, float ic, float *i_alpha, float *i_beta) {
    // Clarke Transform: converts 3-phase currents to 2-phase (alpha-beta)
    // i_alpha = ia
    // i_beta = (2*ib + ic) / sqrt(3)  ... actually: (ia - ib/2 - ic/2) is perpendicular to ia
    // Simplified version often used:
    // i_alpha = ia
    // i_beta = (2*ib - ic) / sqrt(3)
    
    // More standard Clarke transform (DC-component removal):
    *i_alpha = ia;
    *i_beta = (ia + 2.0f * ib) / (1.732050808f); // 1/sqrt(3) ≈ 0.577
}

void inv_clarke_transform(float i_alpha, float i_beta, float *ia, float *ib, float *ic) {
    // Inverse Clarke Transform
    *ia = i_alpha;
    *ib = (-i_alpha + 1.732050808f * i_beta) / 2.0f;
    *ic = (-i_alpha - 1.732050808f * i_beta) / 2.0f;
}

void current_sense_update(void) {
    // Select and read ADC channels
    adc_select_input(ADC_CHANNEL_A);
    uint16_t adc_a = adc_read();
    
    adc_select_input(ADC_CHANNEL_B);
    uint16_t adc_b = adc_read();
    
    adc_select_input(ADC_CHANNEL_C);
    uint16_t adc_c = adc_read();
    
    // Convert to currents
    current_state.ia_amps = current_sense_adc_to_amps(adc_a);
    current_state.ib_amps = current_sense_adc_to_amps(adc_b);
    current_state.ic_amps = current_sense_adc_to_amps(adc_c);
    
    // Apply Clarke Transform
    clarke_transform(current_state.ia_amps, current_state.ib_amps, current_state.ic_amps,
                     &current_state.i_alpha, &current_state.i_beta);
    
    // Check for over-current condition
    float magnitude = sqrtf(current_state.i_alpha * current_state.i_alpha + 
                           current_state.i_beta * current_state.i_beta);
    float current_ma = magnitude * 1000.0f;
    
    if (current_ma > CURRENT_SENSE_CUTOFF_MA) {
        current_state.over_current = true;
    } else if (current_ma < CURRENT_SENSE_MAX_MA) {
        current_state.over_current = false;
    }
}

current_sense_state_t current_sense_get_state(void) {
    return current_state;
}
