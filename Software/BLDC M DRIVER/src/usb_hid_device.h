#ifndef USB_HID_DEVICE_H
#define USB_HID_DEVICE_H

#include <stdint.h>
#include <stdbool.h>

// USB HID Report IDs
#define USB_REPORT_ID_EFFECT 0x01
#define USB_REPORT_ID_STATUS 0x02

typedef struct {
    uint8_t effect_id;
    uint8_t effect_type;  // Constant, Periodic, Condition, etc.
    bool enabled;
    float magnitude;      // 0.0 to 1.0
    float offset;         // For constant force offset
    float gain;           // For spring/damper effects
    float coefficient;    // For friction, damping characteristics
} effect_t;

typedef struct {
    effect_t effects[4];  // Support up to 4 simultaneous effects
    uint8_t num_effects;
    float total_force;    // Combined force from all effects (N or normalized)
    float total_torque;   // Convert to torque command for FOC
} effect_state_t;

// Initialize USB HID device (runs on Core 0)
void usb_hid_init(void);

// Process USB HID reports from host
// Should be called regularly (e.g., 1kHz on Core 0)
void usb_hid_process(void);

// Get current effect state
effect_state_t usb_hid_get_effects(void);

// Add or update an effect
void usb_hid_set_effect(uint8_t effect_id, uint8_t effect_type, 
                        bool enabled, float magnitude, float gain);

// Calculate total torque from all active effects
// force = constant + spring*position + damper*velocity
float usb_hid_calculate_torque_command(float position_rad, float velocity_rad_per_sec);

// Get current USB connection status
bool usb_hid_is_connected(void);

// Send status report back to host
void usb_hid_send_status(float current_position, uint16_t device_state);

#endif // USB_HID_DEVICE_H
