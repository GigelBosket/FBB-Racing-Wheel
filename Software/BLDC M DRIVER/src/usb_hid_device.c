#include "usb_hid_device.h"
#include "config.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include <string.h>
#include <math.h>

static effect_state_t effect_state = {{0}, 0, 0.0f, 0.0f};
static bool usb_connected = false;

// HID Report sizes
#define HID_REPORT_SIZE_OUT 64
#define HID_REPORT_SIZE_IN 64

// USB HID Descriptor for PID (Physical Interface Device) - Assetto Corsa compatible
// This descriptor tells Windows that we're a force feedback gaming device
const uint8_t desc_hid_report[] = {
    // Usage Page (Sport Controls) for gaming
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x04,        // Usage (Joystick)
    0xa1, 0x01,        // Collection (Application)
    
    // Input Report
    0x09, 0x30,        // Usage (X - for wheel position -180 to +180°)
    0x09, 0x31,        // Usage (Y - not used, but report it anyway)
    0x15, 0x00,        // Logical Minimum (0)
    0x26, 0xff, 0x00,  // Logical Maximum (255)
    0x75, 0x08,        // Report Size (8)
    0x95, 0x02,        // Report Count (2)
    0x81, 0x02,        // Input (Data, Variable, Absolute)
    
    // Output Report - Force Feedback Commands
    0x09, 0x32,        // Usage (Z - force feedback data)
    0x15, 0x00,        // Logical Minimum (0)
    0x26, 0xff, 0x00,  // Logical Maximum (255)
    0x75, 0x08,        // Report Size (8)
    0x95, 0x40,        // Report Count (64)
    0x91, 0x02,        // Output (Data, Variable, Absolute)
    
    0xc0,              // End Collection
};

void usb_hid_init(void) {
    // Initialize TinyUSB stack
    tusb_init();
    
    // Initialize effect state
    memset(&effect_state, 0, sizeof(effect_state));
    effect_state.num_effects = 0;
    
    usb_connected = false;
}

void usb_hid_process(void) {
    // TinyUSB task processing (device mode)
    tud_task();
    
    // Check USB connection status
    usb_connected = tud_ready();
}

effect_state_t usb_hid_get_effects(void) {
    return effect_state;
}

void usb_hid_set_effect(uint8_t effect_id, uint8_t effect_type, 
                        bool enabled, float magnitude, float gain) {
    // Find or create effect slot
    int slot = -1;
    
    // First, try to find existing effect with same ID
    for (int i = 0; i < effect_state.num_effects; i++) {
        if (effect_state.effects[i].effect_id == effect_id) {
            slot = i;
            break;
        }
    }
    
    // If not found and space available, create new
    if (slot == -1 && effect_state.num_effects < MAX_EFFECTS) {
        slot = effect_state.num_effects++;
    }
    
    // If no space, overwrite last (circular buffer)
    if (slot == -1) {
        slot = MAX_EFFECTS - 1;
    }
    
    // Update effect
    effect_state.effects[slot].effect_id = effect_id;
    effect_state.effects[slot].effect_type = effect_type;
    effect_state.effects[slot].enabled = enabled;
    effect_state.effects[slot].magnitude = magnitude;
    effect_state.effects[slot].gain = gain;
}

float usb_hid_calculate_torque_command(float position_rad, float velocity_rad_per_sec) {
    float total_torque = 0.0f;
    
    for (int i = 0; i < effect_state.num_effects; i++) {
        if (!effect_state.effects[i].enabled) continue;
        
        effect_t *effect = &effect_state.effects[i];
        float effect_torque = 0.0f;
        
        switch (effect->effect_type) {
            case EFFECT_CONSTANT_FORCE:
                // Constant force/torque
                effect_torque = effect->magnitude * 10.0f;  // Scale to ~10A max
                break;
                
            case EFFECT_SPRING:
                // Spring effect: F = -k * x
                // Torque = gain * position
                effect_torque = -effect->gain * position_rad * 10.0f;
                break;
                
            case EFFECT_DAMPER:
                // Damper effect: F = -c * v
                // Torque = gain * velocity
                effect_torque = -effect->gain * velocity_rad_per_sec * 10.0f;
                break;
                
            case EFFECT_FRICTION:
                // Friction: constant opposing force
                if (velocity_rad_per_sec > 0.1f) {
                    effect_torque = -effect->gain * 10.0f;
                } else if (velocity_rad_per_sec < -0.1f) {
                    effect_torque = effect->gain * 10.0f;
                }
                break;
                
            default:
                break;
        }
        
        total_torque += effect_torque;
    }
    
    // Clamp total torque
    if (total_torque > 20.0f) total_torque = 20.0f;
    if (total_torque < -20.0f) total_torque = -20.0f;
    
    effect_state.total_torque = total_torque;
    
    // Convert torque to current command (approximate: Torque ≈ Motor_Kt * Current)
    // For hoverboard motor, roughly 0.1 Nm per Ampere
    float iq_command = total_torque / 0.1f;
    
    // Clamp to current limits
    if (iq_command > 25.0f) iq_command = 25.0f;
    if (iq_command < -25.0f) iq_command = -25.0f;
    
    return iq_command;
}

bool usb_hid_is_connected(void) {
    return usb_connected;
}

void usb_hid_send_status(float current_position, uint16_t device_state) {
    uint8_t report[HID_REPORT_SIZE_IN] = {0};
    
    // Report format: [position_byte, state_byte, ...]
    // Position: map from radians to 0-255
    // -π to +π -> 0-255
    float pos_normalized = (current_position + M_PI) / (2.0f * M_PI);
    if (pos_normalized > 1.0f) pos_normalized = 1.0f;
    if (pos_normalized < 0.0f) pos_normalized = 0.0f;
    
    report[0] = (uint8_t)(pos_normalized * 255.0f);
    report[1] = device_state & 0xFF;
    report[2] = (device_state >> 8) & 0xFF;
    
    if (usb_connected && tud_hid_ready()) {
        tud_hid_report(0x00, report, sizeof(report));
    }
}

// TinyUSB callbacks (must be implemented)

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) {
    (void)instance;
    return desc_hid_report;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, 
                               hid_report_type_t report_type, 
                               uint8_t *buffer, uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, 
                           hid_report_type_t report_type, 
                           uint8_t const *buffer, uint16_t buflen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)buflen;
}
