#pragma once

#define OUTPUT 1
#define INPUT 0

#define HIGH 1
#define LOW 0

#include "pico/stdlib.h"

inline void pinMode(int pin, int mode)
{
    gpio_init(pin);
    gpio_set_dir(pin, mode);
}

inline void digitalWrite(int pin, int state)
{
    gpio_put(pin, state);
}

inline void delay(int milliseconds)
{
    sleep_ms(milliseconds);
}

class SPIClass
{
public:
    uint8_t transfer(uint8_t val);
    void begin();
};
