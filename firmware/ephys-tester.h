#pragma once

// Pin defintions
#define SDA                 2
#define SCL                 3

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

#define DEBUG_0             17
#define DEBUG_1             18
#define DEBUG_2             19
#define DEBUG_3             20

#define ENC_PIO             pio1
#define ENC_A               21
#define ENC_B               22
#define ENC_BUT             23

//#define OLED_SPI            spi1
#define OLED_nDC            24
#define OLED_nRES           25
#define OLED_SCLK           26
#define OLED_MOSI           27
#define OLED_nCS            29

// Shift register bit array
#define sr_bit_arr_t        int32_t                     // Bit array element data type
#define SR_BITS_PER_ELEMENT (8 * sizeof(sr_bit_arr_t))  // Bits per array element
#define SR_BIT_ARR_LEN      4                           // Number of bit_arr_t's required to hold 128 bits, one for each channel switch
