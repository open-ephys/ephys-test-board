#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "sr.h"
#include "ephys-tester.h"

// TODO: This delay may not even been needed
#define BB_SETTLE_USEC 1

// TODO: All this bit banging should probably be moved to PIO1
int sr_init()
{
    // Setup all analog switch inputs
    gpio_init(SR_nSRCLR); gpio_set_dir(SR_nSRCLR, GPIO_OUT);
    gpio_init(SR_SRCLK); gpio_set_dir(SR_SRCLK, GPIO_OUT);
    gpio_init(SR_RCLK); gpio_set_dir(SR_RCLK, GPIO_OUT);
    gpio_init(SR_SER); gpio_set_dir(SR_SER, GPIO_OUT); gpio_set_outover(SR_SER, GPIO_OVERRIDE_INVERT); // High -> ground channel, Low -> tie to signal
    gpio_init(TEST_SEL); gpio_set_dir(TEST_SEL, GPIO_OUT);

    // Clear the SR and clock the 0 values into output registers
    gpio_put(SR_RCLK, 0);
    gpio_put(SR_nSRCLR, 0);
    gpio_put(SR_RCLK, 1);
    gpio_put(SR_RCLK, 0);
    gpio_put(SR_nSRCLR, 1);

    // Set signal select to internal
    gpio_put(TEST_SEL, INT_SIGNAL);

    return 0;
}

int sr_update(sr_bit_arr_t const *bit_arr) //, size_t bit_arr_len)
{
    // Shift bits into shift register
    gpio_put(SR_RCLK, 0);
    sleep_us(BB_SETTLE_USEC);

    int total_bits = 8 * sizeof(sr_bit_arr_t); // bit_arr_len * 8 * sizeof(sr_bit_arr_t);

    for (int i = total_bits - 1; i >= 0; i--) { // MSB first -- this is a shift register
        gpio_put(SR_SRCLK, 0); sleep_us(BB_SETTLE_USEC);
        gpio_put(SR_SER, sr_get_bit(bit_arr, i)); sleep_us(BB_SETTLE_USEC);
        gpio_put(SR_SRCLK, 1); sleep_us(BB_SETTLE_USEC);
    }

    // Move shift register to output register
    gpio_put(SR_SRCLK, 0); sleep_us(BB_SETTLE_USEC);
    gpio_put(SR_RCLK, 1); sleep_us(BB_SETTLE_USEC);
    gpio_put(SR_RCLK, 0); sleep_us(BB_SETTLE_USEC);

    return 0;
}

int sr_source(signal_source_t source)
{
    gpio_put(TEST_SEL, source);
    return 0;
}