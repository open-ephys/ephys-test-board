#pragma once

#include "pico/stdlib.h"

// Pin definitions
#define MODULE_I2C          i2c1
#define MODULE_SDA          2
#define MODULE_SCL          3
#define MODULE_DETECT       5 

#define VBUS_DETECT         4

#define DAC_PIO             pio0
#define DAC_MOSI            7
#define DAC_nCS             9
#define DAC_SCLK            10
#define DAC_nLDAC           11

#define SR_nSRCLR           12
#define SR_SRCLK            13
#define SR_RCLK             14
#define SR_SER              15

#define TEST_SEL            16

#define OLED_PWR             19

#define ENC_PIO             pio1
#define ENC_A               21
#define ENC_B               22
#define ENC_BUT             23

#define OLED_SPI            spi1
#define OLED_nDC            24
#define OLED_nRES           25
#define OLED_SCLK           26
#define OLED_MOSI           27
#define OLED_nCS            29

#define BATT_MON_PIN        28
#define BATT_MON_ADC        2

// Screen constants
#define SCREEN_WIDTH        128
#define SCREEN_HEIGHT       64

// Signal constants
#define MAX_AMPLITUDE_UV    5000.0f
#define DAC_MAX_SHIFT       10 // Max bit shift for DAC attenuation

// Automatic channel increment dwell time in milliseconds
#define AUTO_CHAN_SDWELL_MS 1000
#define AUTO_CHAN_FDWELL_MS 100

// Battery monitor period in milliseconds
#define BATT_MON_PERIOD_MS  1000
#define BATT_MON_AVGS       30

// Module EEPROM constants
#define MODULE_NAME_BYTES   20

// Shift register bit array
#define MAX_NUM_CHANNELS    128                                     // 128 channels max
typedef struct sr_bit_arr_t { int32_t bits[4]; } sr_bit_arr_t;
#define SR_BITS_PER_ELEMENT (8 * sizeof(int32_t))                   // Bits per array element of sr_bit_arr_t

// Stringify macro for compile-time stringification
#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)
