#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "oled.h"
#include "ephys-tester.h"


int oled_init()
{
    gpio_init(OLED_PWR); gpio_set_dir(OLED_PWR, GPIO_OUT);
    gpio_put(OLED_PWR, 1);

    return 0;
}