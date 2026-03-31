#include <stdint.h>
#include <string.h>
#include "tusb.h"
#include "config.h"

// =============================================================================
// Device Descriptors for USB HID Force Feedback Wheel
// =============================================================================

// External descriptor defined in usb_hid_device.c
extern uint8_t const desc_hid_report[];

uint8_t const desc_device[] = {
    0x12,                          // bLength
    TUSB_DESC_DEVICE,              // bDescriptorType
    0x10, 0x01,                    // bcdUSB (1.1, used for HID compatibility)
    0x00,                          // bDeviceClass (0 for composite device using interface descriptors)
    0x00,                          // bDeviceSubClass
    0x00,                          // bDeviceProtocol
    CFG_TUD_ENDPOINT0_SIZE,        // bMaxPacketSize0
    USB_VENDOR_ID & 0xFF, (USB_VENDOR_ID >> 8) & 0xFF,    // idVendor
    USB_PRODUCT_ID & 0xFF, (USB_PRODUCT_ID >> 8) & 0xFF,  // idProduct
    0x00, 0x10,                    // bcdDevice (v1.0)
    0x01,                          // iManufacturer (string index 1)
    0x02,                          // iProduct (string index 2)
    0x03,                          // iSerialNumber (string index 3)
    0x01                           // bNumConfigurations
};

uint8_t const desc_configuration[] = {
    // Configuration Descriptor
    0x09,                          // bLength
    TUSB_DESC_CONFIGURATION,       // bDescriptorType
    0x22, 0x00,                    // wTotalLength (includes endpoints, HID, etc.)
    0x01,                          // bNumInterfaces
    0x01,                          // bConfigurationValue
    0x00,                          // iConfiguration (no string)
    0xa0,                          // bmAttributes (bit 7: bus powered, bit 5: remote wakeup)
    0x32,                          // bMaxPower (100mA)
    
    // Interface Descriptor - HID
    0x09,                          // bLength
    TUSB_DESC_INTERFACE,           // bDescriptorType
    0x00,                          // bInterfaceNumber
    0x00,                          // bAlternateSetting
    0x02,                          // bNumEndpoints (IN and OUT)
    0x03,                          // bInterfaceClass (HID)
    0x00,                          // bInterfaceSubClass (0 = no boot interface)
    0x00,                          // bInterfaceProtocol (0 = none)
    0x00,                          // iInterface
    
    // HID Descriptor
    0x09,                          // bLength
    HID_DESC_TYPE_HID,             // bDescriptorType
    0x10, 0x01,                    // bcdHID (HID version 1.1)
    0x00,                          // bCountryCode
    0x01,                          // bNumDescriptors
    HID_DESC_TYPE_REPORT,          // bDescriptorType (Report)
    0x50, 0x00,                    // wDescriptorLength (80 bytes)
    
    // Endpoint 1 OUT (Host to Device - Effect commands)
    0x07,                          // bLength
    TUSB_DESC_ENDPOINT,            // bDescriptorType
    0x01,                          // bEndpointAddress (EP 1 OUT)
    TUSB_XFER_INTERRUPT,           // bmAttributes
    0x40, 0x00,                    // wMaxPacketSize (64 bytes)
    0x0a,                          // bInterval (10ms)
    
    // Endpoint 2 IN (Device to Host - Status/Position)
    0x07,                          // bLength
    TUSB_DESC_ENDPOINT,            // bDescriptorType
    0x82,                          // bEndpointAddress (EP 2 IN)
    TUSB_XFER_INTERRUPT,           // bmAttributes
    0x40, 0x00,                    // wMaxPacketSize (64 bytes)
    0x0a,                          // bInterval (10ms)
};

// String descriptors
static const char usb_strings[][32] = {
    "Racing Wheel Developer",      // Manufacturer
    "Direct Drive Wheel (FOC)",    // Product
    "DD-WHEEL-001"                 // Serial number
};

uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const *)desc_device;
}

uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;  // Suppress unused warning
    return desc_configuration;
}

// Return string descriptor for given language code and index
uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    // LangID = 0x0409 is English (United States)
    
    #define STRDESC_ARRAY_LEN  (3)
    
    static uint16_t _desc_str[32];
    
    // Assign the language ID
    if (index == 0) {
        _desc_str[1] = 0x0409;  // English - United States
        _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * 1 + 2);
        return _desc_str;
    }
    
    uint8_t chr_count;
    
    if (index < STRDESC_ARRAY_LEN) {
        const char *str = usb_strings[index];
        chr_count = strlen(str);
    } else {
        return NULL;
    }
    
    // Make the string descriptor
    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    
    for (uint8_t i = 0; i < chr_count; i++) {
        _desc_str[1 + i] = usb_strings[index][i];
    }
    
    return _desc_str;
}

uint8_t const * tud_descriptor_hid_report_cb(uint8_t instance) {
    return desc_hid_report;
}
