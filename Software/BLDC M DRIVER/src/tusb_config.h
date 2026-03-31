#ifndef TUSB_CONFIG_H
#define TUSB_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// ============================================================================
// TinyUSB Configuration for RP2354
// ============================================================================

// Allowed values: CFG_TUSB_DEBUG = 0 for no debug, 1, 2, 3
#define CFG_TUSB_DEBUG 0

// Device mode with HID and CDC support
#define CFG_TUD_ENABLED 1
#define CFG_TUH_ENABLED 0

// USB Port mode
#define CFG_TUSB_RHPORT0_MODE OPT_MODE_DEVICE

// Enable device class drivers
#define CFG_TUD_HID 1
#define CFG_TUD_CDC 0
#define CFG_TUD_MSC 0
#define CFG_TUD_MIDI 0
#define CFG_TUD_VENDOR 0

// HID configuration
#define CFG_TUD_HID_EP_BUFSIZE 64
#define CFG_TUD_HID_BUFSIZE 256

// String descriptor support
#define CFG_TUD_ENABLE_MANUFACTURER_DESC 1
#define CFG_TUD_ENABLE_PRODUCT_DESC 1
#define CFG_TUD_ENABLE_SERIAL_DESC 1  // Use on-chip unique ID

// USB Endpoint configuration
#define CFG_TUD_ENDPOINT0_SIZE 64

// Speed: 0 = Full Speed only, 1 = High Speed with fallback
#define BOARD_TUD_RHPORT_SPEED OPT_MODE_FULL_SPEED

#ifdef __cplusplus
}
#endif

#endif // TUSB_CONFIG_H
