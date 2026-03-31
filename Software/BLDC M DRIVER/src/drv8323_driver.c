#include "drv8323_driver.h"
#include "config.h"
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"

#define SPI_PORT spi1
#define SPI_BAUD 1000000  // 1MHz SPI clock

void drv8323_init(void) {
    // Initialize SPI
    spi_init(SPI_PORT, SPI_BAUD);
    
    // Set GPIO function for SPI
    gpio_set_function(GPIO_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(GPIO_MISO, GPIO_FUNC_SPI);
    gpio_set_function(GPIO_SCK, GPIO_FUNC_SPI);
    
    // Set up chip select (active low)
    gpio_init(GPIO_CSN);
    gpio_set_dir(GPIO_CSN, GPIO_OUT);
    gpio_put(GPIO_CSN, 1);
    
    // Set up fault and enable pins
    gpio_init(GPIO_DRV_FAULT);
    gpio_set_dir(GPIO_DRV_FAULT, GPIO_IN);
    gpio_pull_up(GPIO_DRV_FAULT);
    
    gpio_init(GPIO_DRV_EN);
    gpio_set_dir(GPIO_DRV_EN, GPIO_OUT);
    gpio_put(GPIO_DRV_EN, 1);  // Enable high (always on)
    
    // Configure DRV8323 with safe defaults
    sleep_ms(100);  // Wait for IC to stabilize
    
    // Clear any faults
    drv8323_clear_faults();
    
    // Set reasonable gate drive strength
    // Write to GATE_DRIVE_HS register
    drv8323_write_reg(DRV8323_REG_GATE_DRIVE_HS, 0x0303);
    drv8323_write_reg(DRV8323_REG_GATE_DRIVE_LS, 0x0303);
    
    // Configure OCP (Over Current Protection)
    // Typical setting: 25-30A with adjustable threshold
    drv8323_write_reg(DRV8323_REG_OCP_CTRL, 0x0A0A);
    
    // Configure current sense amplifiers
    drv8323_configure_current_sense(DRV8323_GAIN);
}

uint16_t drv8323_read_reg(uint8_t reg) {
    // DRV8323 SPI format:
    // Read:  0x80 | (register address)
    // Write: 0x00 | (register address)
    
    uint8_t tx[2], rx[2];
    tx[0] = 0x80 | reg;  // Read command
    tx[1] = 0x00;        // Dummy byte
    
    gpio_put(GPIO_CSN, 0);  // CS low
    sleep_us(1);
    
    spi_write_read_blocking(SPI_PORT, tx, rx, 2);
    
    sleep_us(1);
    gpio_put(GPIO_CSN, 1);  // CS high
    
    // Return 16-bit result (rx[0] << 8) | rx[1]
    return ((uint16_t)rx[0] << 8) | rx[1];
}

void drv8323_write_reg(uint8_t reg, uint16_t value) {
    // Write format: 0x00 | (register address)
    uint8_t tx[3], rx[3];
    tx[0] = 0x00 | reg;       // Write command
    tx[1] = (value >> 8) & 0xFF;
    tx[2] = value & 0xFF;
    
    gpio_put(GPIO_CSN, 0);  // CS low
    sleep_us(1);
    
    spi_write_read_blocking(SPI_PORT, tx, rx, 3);
    
    sleep_us(1);
    gpio_put(GPIO_CSN, 1);  // CS high
    
    sleep_us(5);  // Wait for register write to complete
}

drv8323_fault_t drv8323_get_fault_status(void) {
    drv8323_fault_t status = {0};
    
    uint16_t fault_reg = drv8323_read_reg(DRV8323_REG_FAULT_STATUS);
    
    // Parse fault register bits
    status.fault = (fault_reg & 0x0001) != 0;
    status.uvlo = (fault_reg & 0x0002) != 0;
    status.vcp_uv = (fault_reg & 0x0004) != 0;
    status.gd_uvlo_a = (fault_reg & 0x0008) != 0;
    status.gd_uvlo_b = (fault_reg & 0x0010) != 0;
    status.gd_uvlo_c = (fault_reg & 0x0020) != 0;
    status.gd_oc_a = (fault_reg & 0x0040) != 0;
    status.gd_oc_b = (fault_reg & 0x0080) != 0;
    status.gd_oc_c = (fault_reg & 0x0100) != 0;
    
    return status;
}

void drv8323_clear_faults(void) {
    // Read fault register to clear latched faults
    drv8323_read_reg(DRV8323_REG_FAULT_STATUS);
}

void drv8323_enable_outputs(bool enable) {
    if (enable) {
        gpio_put(GPIO_DRV_EN, 1);
    } else {
        gpio_put(GPIO_DRV_EN, 0);
    }
}

void drv8323_configure_current_sense(float gain) {
    // Current sense amplifier configuration
    // Typical gains are 10V/V or 20V/V
    // For 20V/V gain, use default CSA_CTRL value
    uint16_t csa_ctrl = 0x0000;
    
    if (gain > 15.0f) {
        csa_ctrl |= 0x0001;  // 20V/V gain
    } else {
        csa_ctrl |= 0x0000;  // 10V/V gain  
    }
    
    drv8323_write_reg(DRV8323_REG_CSA_CTRL, csa_ctrl);
}
