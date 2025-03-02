#include <stdio.h>
#include "pico/stdlib.h"

extern "C"
{
#include "ephys-tester.h"
#include "ad5683.h"
#include "quadrature.h"
#include "oled.h"
#include "parse.h"
#include "sr.h"
#include "tiny-json.h"
#include "sine.h"
}

//#include "Adafruit_GFX.h"
#include "Adafruit_SH110X.h"

// TODO: Create command specification
#define STR_BUFFER_BYTES    256
#define MAX_COMMANDS        256

static void knob_turned() { }

int main()
{
    // // I2C Initialisation. Using it at 400Khz.
    // i2c_init(I2C_PORT, 400*1000);

    // gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    // gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    // OLED
    oled_init();
    Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT,
        OLED_MOSI, OLED_SCLK, OLED_nDC, OLED_nRES, OLED_nCS, OLED_SPI);

    // Splash
    display.setRotation(2);
    display.begin(0, 1);
    display.display();
    sleep_ms(500);

    // Signal select initialization
    sr_init();

    // DAC initialization
    pio_spi_inst_t dac_spi = {
        .pio = DAC_PIO,
        .sm = 0,
    };

    ad5683_init(&dac_spi);

    // Quadrature encoder initialization
    pio_quad_inst_t quad_dec = {
        .pio = ENC_PIO,
        .sm = 0,
        .dma_chan = 0,
        .counter = 0,
        .counter_update_handler = knob_turned
    };

    quad_init(&quad_dec);

    // // Command interface
    // stdio_init_all();

    // // JSON parser buffer initialization
    // char str[STR_BUFFER_BYTES];
    // json_t buff[MAX_COMMANDS];

    // while (1) {

    //     char *rv = fgets(str, sizeof str, stdin);

    //     if (rv == NULL) {
    //         puts("Invalid JSON command string.");
    //         continue;
    //     }

    //     // NB: this call modifies str, so dont try to use it again!
    //     json_t const *root = json_create(str, buff, sizeof buff / sizeof *buff);

    //     if (root == NULL) {
    //         puts("Invalid JSON formatting.");
    //         continue;
    //     }

    //     // Bit array (128 bits)
    //     bit_arr_t chs[BIT_ARR_LEN];

    //     int rc = parse_channels(root, chs, BIT_ARR_LEN);
    //     if (rc) {
    //         puts(parse_error(rc));
    //         continue;
    //     }

    //     sr_update(chs, BIT_ARR_LEN);
    // }

    uint i = 0;
    uint step = 2;
    while (1) {
        ad5683_write_dac(&dac_spi, SINE1024[i % 1024]);
        i += step;
    }

    // while(1)
    // {
    //     // NB: 4 pulses per detent using the PEC12R-4222F-S0024
    //     printf("Count: %ld\n", quad_get_count(&quad_dec) / 4);
    // }

    // while (1) {
    //     uint16_t abscount = 100 * (uint16_t)quad_get_count(&quad_dec);
    //     ad5683_write_dac(&dac_spi, abscount);
    // }

}
