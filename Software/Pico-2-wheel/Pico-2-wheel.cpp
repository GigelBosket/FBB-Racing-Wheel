#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/spi.h"


struct InputPayload {
    uint32_t buttons;
    int32_t encoder_diff[6];
};

struct EncoderMap {
    uint8_t pinA; 
    uint8_t pinB;
};

// 2. CONFIGURATION
#define ROWS 5
#define COLS 6
#define NUM_ENCODERS 6 
#define DEBOUNCE_THRESHOLD 5 

const uint row_pins[ROWS] = {2, 3, 4, 5, 6};
const uint col_pins[COLS] = {7, 8, 9, 10, 11, 12};

#define SPI_PORT spi0
#define PIN_MISO 16 
#define PIN_CS   17 
#define PIN_SCK  18 
#define PIN_MOSI 19 


EncoderMap encoders[NUM_ENCODERS] = {
    {0, 1},   
    {2, 3},   
    {4, 5},   
    {6, 7},   
    {8, 9},   
    {10, 11} 
};


void core1_entry() {
    for (int i = 0; i < ROWS; i++) {
        gpio_init(row_pins[i]);
        gpio_set_dir(row_pins[i], GPIO_IN);
        gpio_pull_down(row_pins[i]);
    }
    for (int i = 0; i < COLS; i++) {
        gpio_init(col_pins[i]);
        gpio_set_dir(col_pins[i], GPIO_OUT);
        gpio_put(col_pins[i], 0);
    }

    while (1) {
        uint32_t scan = 0;
        for (int c = 0; c < COLS; c++) {
            gpio_put(col_pins[c], 1);
            sleep_us(2); 
            for (int r = 0; r < ROWS; r++) {
                if (gpio_get(row_pins[r])) {
                    scan |= (1 << (c * ROWS + r));
                }
            }
            gpio_put(col_pins[c], 0);
        }
        multicore_fifo_push_blocking(scan);
    }
}

int main() {
    stdio_init_all();

    spi_init(SPI_PORT, 1000 * 1000); 
    spi_set_slave(SPI_PORT, true);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    gpio_set_function(PIN_CS, GPIO_FUNC_SPI);

    multicore_launch_core1(core1_entry);

    uint8_t debounce_counters[30] = {0};
    uint8_t encoder_states[6] = {0}; 
    const int8_t encoder_table[] = {0,1,-1,0,-1,0,0,1,1,0,0,-1,0,-1,1,0};
    
    InputPayload payload = {0};

    while (1) {
        if (multicore_fifo_rvalid()) {
            uint32_t raw_bits = multicore_fifo_pop_blocking();

            uint32_t current_debounced_state = 0;
            for (int i = 0; i < 30; i++) {
                bool is_high = (raw_bits >> i) & 1;
                if (is_high) {
                    if (debounce_counters[i] < DEBOUNCE_THRESHOLD) debounce_counters[i]++;
                } else {
                    if (debounce_counters[i] > 0) debounce_counters[i]--;
                }
                if (debounce_counters[i] == DEBOUNCE_THRESHOLD) current_debounced_state |= (1 << i);
            }

            for (int e = 0; e < NUM_ENCODERS; e++) {
                uint8_t a = (current_debounced_state >> encoders[e].pinA) & 1;
                uint8_t b = (current_debounced_state >> encoders[e].pinB) & 1;
                uint8_t state = (encoder_states[e] << 2) | (a << 1) | b;
                payload.encoder_diff[e] += encoder_table[state & 0x0F];
                encoder_states[e] = (a << 1) | b;
            }
            payload.buttons = current_debounced_state;


            spi_write_read_blocking(SPI_PORT, (uint8_t*)&payload, NULL, sizeof(payload));

            for(int i=0; i<6; i++) payload.encoder_diff[i] = 0;
        }
    }
}