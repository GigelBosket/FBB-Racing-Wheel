#ifndef DRV8323_DRIVER_H
#define DRV8323_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

// DRV8323RS Register Addresses
#define DRV8323_REG_FAULT_STATUS 0x00
#define DRV8323_REG_VGS_STATUS 0x01
#define DRV8323_REG_DRIVER_CTRL 0x02
#define DRV8323_REG_GATE_DRIVE_HS 0x03
#define DRV8323_REG_GATE_DRIVE_LS 0x04
#define DRV8323_REG_OCP_CTRL 0x05
#define DRV8323_REG_CSA_CTRL 0x06

typedef struct {
    bool fault;
    bool uvlo;
    bool vcp_uv;
    bool gd_uvlo_a;
    bool gd_uvlo_b;
    bool gd_uvlo_c;
    bool gd_oc_a;
    bool gd_oc_b;
    bool gd_oc_c;
} drv8323_fault_t;

// Initialize SPI and DRV8323
void drv8323_init(void);

// Read register from DRV8323
uint16_t drv8323_read_reg(uint8_t reg);

// Write register to DRV8323
void drv8323_write_reg(uint8_t reg, uint16_t value);

// Read fault status
drv8323_fault_t drv8323_get_fault_status(void);

// Clear faults
void drv8323_clear_faults(void);

// Enable/disable outputs
void drv8323_enable_outputs(bool enable);

// Configure current sense amplifiers (gain setting)
void drv8323_configure_current_sense(float gain);

#endif // DRV8323_DRIVER_H
