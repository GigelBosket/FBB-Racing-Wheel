#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "tusb.h"
#include "usb_descriptors.h"


struct InputPayload {
    uint32_t buttons;
    int32_t encoder_diff[6];
};

#define SPI_PORT spi0
#define PIN_MISO 16 
#define PIN_CS   17 
#define PIN_SCK  18 
#define PIN_MOSI 19 

void send_hid_report(InputPayload const& data, uint32_t extra_buttons) {
    if (!tud_hid_ready()) return;
    uint32_t all_buttons = data.buttons | extra_buttons;
    tud_hid_gamepad_report(REPORT_ID_GAMEPAD, 0, 0, 0, 0, 0, 0, 0, all_buttons);
}

int main() {
    stdio_init_all();
    tusb_init();

    spi_init(SPI_PORT, 1000 * 1000);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);
    gpio_put(PIN_CS, 1);

    InputPayload incoming_data = {0};
    uint32_t last_spi_poll = 0;

    while (1) {
        tud_task(); 

        uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - last_spi_poll > 5) {
            last_spi_poll = now;

            gpio_put(PIN_CS, 0);
            spi_read_blocking(SPI_PORT, 0, (uint8_t*)&incoming_data, sizeof(incoming_data));
            gpio_put(PIN_CS, 1);

            uint32_t virtual_buttons = 0;
            for (int i = 0; i < 6; i++) {
                if (incoming_data.encoder_diff[i] > 0) virtual_buttons |= (1 << (18 + (i * 2)));
                if (incoming_data.encoder_diff[i] < 0) virtual_buttons |= (1 << (18 + (i * 2) + 1));
            }

            send_hid_report(incoming_data, virtual_buttons);
        }
    }
}


uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    return 0;
}


void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
}