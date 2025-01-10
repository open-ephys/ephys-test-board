#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "Adafruit_SSD1306.h"

// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define SPI_PORT spi0
#define OLED_MOSI   27
#define OLED_SCK   26
#define OLED_DC    24
#define OLED_nCS    29
#define OLED_nRESET 25

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_SCK, OLED_DC, OLED_nRESET, OLED_nCS);

int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(OLED_nCS,   GPIO_FUNC_SIO);
    gpio_set_function(OLED_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(OLED_MOSI, GPIO_FUNC_SPI);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(OLED_nCS, GPIO_OUT);
    gpio_put(OLED_nCS, 1);

    gpio_set_dir(OLED_nRESET, GPIO_OUT);
    gpio_put(OLED_nRESET, 1);


    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
