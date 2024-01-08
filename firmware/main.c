// Some messing around with the RP pico to figure out what the firmware might
// end up looking like. I like the idea of a main loop that is decoding a JSON
// message containing a simple set of state information that is then applied to
// hardware.
//
// Some potential commands
// 1. channels -- turn an array of selected channels into a bit array that is
// used to update the electrode switches via shift registers.  This is kinda
// implemented here.
// 2. A timer driven counter and SPI update function that looks into a
// selectable set of waveforms (square, sine, PRBS, and AP sequence). Note that
// this chip has two cores, so this can be handled on the second core to
// preserve perfect timing. Communication via queue or low level FIFO (although
// later is discouraged). Not sure where to store e.g. an AP sequence. There
// are a couple hundred kB of program space on this chip so it might be enough
// for something simple. Otherwise, I think its quite easy to use flash (e.g. I
// think that unless explicitly specified, parts of the program are excecuted
// out of flash).
// 3. Some state information displayed via a couple neopixels (probably want to
// only drive these on update to avoid noise). You must be able to turn them
// off as well.
//
// Note that I'm just messing around here. 
// - All the pin defs will need to be changed, this is not hardware tested
// - The JSON parsing works well but needs to be expanded. 
// - Im including the the CMakeLists.txt but this was built by creating a
//   new folder in the pico-examples repo which gave base build file along with
//   access to the pico-sdk
// - This uses the pico-sdk, which im not including because it should come in
//   as a submodule, but the flash will need to be formated to all booting and
//   loading files.

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "tiny-json.h"

// TODO: Create command specification
#define STR_BUFFER_BYTES    256
#define MAX_COMMANDS        256

// Channel bit array (128 bits, one bit per channel)
#define bit_arr_t           int32_t
#define BIT_ARR_LEN         4

// Bit helpers
// TODO: inline these as real functions so bit_arr_t can be typedef
#define set_bit(A,k)         ( A[(k)/sizeof(bit_arr_t)] |= (1 << ((k)%sizeof(bit_arr_t))) )
#define clear_bit(A,k)       ( A[(k)/sizeof(bit_arr_t)] &= ~(1 << ((k)%sizeof(bit_arr_t))) )
#define read_bit(A,k)        ( A[(k)/sizeof(bit_arr_t)] & (1 << ((k)%sizeof(bit_arr_t))) )

// Pin defintions
#define PIN_ATTEN_SEL       2
#define PIN_SIG_SEL         2

#define PIN_SRCLK           3
#define PIN_RCLK            4
#define PIN_nSRCLR          5
#define PIN_SER             6

#define SPI_PORT            spi0
#define PIN_CS              9
#define PIN_MISO            12
#define PIN_MOSI            13
#define PIN_SCK             14

int parse_channels(json_t const *root, bit_arr_t *bit_arr, size_t bit_arr_len)
{
    json_t const *ch_prop = json_getProperty(root, "channels");
    if (ch_prop == NULL) {
        return -1;
    }

    if (json_getType(ch_prop) != JSON_ARRAY) {
        puts("The \"channels\" property must be an array.");
        return -2;
    }

    // Clear all bits
    for (int i = 0; i < bit_arr_len; i++)
        bit_arr[i] = 0;

    for (json_t const *i = json_getChild(ch_prop); i; i = json_getSibling(i)) {

        // Check type
        if (json_getType(i) != JSON_INTEGER) {
            puts("Elements of \"channels\" array must be integers.");
            return -3;
        }

        // Get the bit index
        int b = json_getInteger(i);

        if (b < 0 || b >= (bit_arr_len * 8 * sizeof(bit_arr_t))) {
            puts("Invalid channel number.");
            return -4;
        }

        set_bit(bit_arr, b);
    }

    return 0;
}

// Set all analog switches to default state 
int set_switch_defaults() 
{
    // Clear the SR and clock the 0 values into output registers
    gpio_put(PIN_RCLK, 0);
    gpio_put(PIN_nSRCLR, 0);
    gpio_put(PIN_RCLK, 1);
    gpio_put(PIN_RCLK, 0);
    gpio_put(PIN_nSRCLR, 1);

    // Set signal select to internal
    gpio_put(PIN_SIG_SEL, 0);

    // Set signal attenution to -XXdB
    gpio_put(PIN_ATTEN_SEL, 0);

    return 0;
}

// Update electrode switches with bit array
int update_sr(bit_arr_t const *bit_arr, size_t bit_arr_len)
{
    // Shift bits into shift register
    gpio_put(PIN_RCLK, 0);

    for (int i = 0; i < bit_arr_len; i++) {

        bit_arr_t bw = bit_arr[i];

        for (int j = 0; j < 8 * sizeof(bit_arr_t); j++) {
            gpio_put(PIN_SRCLK, 0);
            gpio_put(PIN_SER, (bw << i & 0x1));
            gpio_put(PIN_SRCLK, 1);
        }
    }

    gpio_put(PIN_SRCLK, 0);
    gpio_put(PIN_RCLK, 1);
    gpio_put(PIN_RCLK, 0);

    return 0;
}

static inline void cs_select() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 0);  // Active low
    asm volatile("nop \n nop \n nop");
}

static inline void cs_deselect() {
    asm volatile("nop \n nop \n nop");
    gpio_put(PIN_CS, 1);
    asm volatile("nop \n nop \n nop");
}

int __not_in_flash_func(update_dac)(uint16_t value)
{
    cs_select();
    spi_write16_blocking(SPI_PORT, &value, 1);
    cs_deselect();
    return 0;
}

int main() {

    // Initialize input stream
    stdio_init_all();

    // Setup all analog switch inputs
    gpio_init(PIN_ATTEN_SEL); gpio_set_dir(PIN_ATTEN_SEL, GPIO_OUT);
    gpio_init(PIN_SIG_SEL); gpio_set_dir(PIN_SIG_SEL, GPIO_OUT);

    gpio_init(PIN_SRCLK); gpio_set_dir(PIN_SRCLK, GPIO_OUT);
    gpio_init(PIN_RCLK); gpio_set_dir(PIN_RCLK, GPIO_OUT);
    gpio_init(PIN_nSRCLR); gpio_set_dir(PIN_nSRCLR, GPIO_OUT);
    gpio_init(PIN_SER); gpio_set_dir(PIN_SER, GPIO_OUT);

    set_switch_defaults();
    
    // Setup DAC SPI port
    spi_init(SPI_PORT, 10000 * 1000); // 10 MHz (20 MHz max)
    spi_set_format(SPI_PORT, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST); // 16-bit words, mode (0, 0), MSB first
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    // Define SPI CS line and default to high
    gpio_init(PIN_CS); gpio_set_dir(PIN_CS, GPIO_OUT); gpio_put(PIN_CS, 1);

    // Initialize buffers used by the JSON parser
    char str[STR_BUFFER_BYTES];
    json_t buff[MAX_COMMANDS];

    while (1) {

        char *rv = fgets(str, sizeof str, stdin);

        if (rv == NULL) {
            puts("Invalid JSON command string.");
            continue;
        }

        // NB: this call modifies str, so dont try to use it again!
        json_t const *root = json_create(str, buff, sizeof buff / sizeof *buff);

        if (root == NULL) {
            puts("Invalid JSON formatting.");
            continue;
        }

        // Bit array (128 bits)
        bit_arr_t chs[BIT_ARR_LEN];

        if (!parse_channels(root, chs, BIT_ARR_LEN)) {

            update_sr(chs, BIT_ARR_LEN);

            //printf("[");
            //for(int i = 0; i < 4; i++)
            //{

            //    printf("%#10X", chs[i]);
            //}
            //printf("]\n");
        }
    }
}
