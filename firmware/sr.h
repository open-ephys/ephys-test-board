#pragma once

#include "ephys-tester.h"

typedef enum {
  EXT_SIGNAL = 0,
  INT_SIGNAL = 1,
} SIGNAL_SOURCE;

int sr_init();
int sr_source(SIGNAL_SOURCE source);
int sr_update(sr_bit_arr_t const *bit_arr, size_t bit_arr_len);

inline int sr_set_bit(sr_bit_arr_t *bit_arr, size_t bit_arr_len, int b)
{
    if (b < 0 || b >= (bit_arr_len * SR_BITS_PER_ELEMENT)) {
        return -1; // Invalid channel number
    }

    bit_arr[b / SR_BITS_PER_ELEMENT] |= (1 << (b % SR_BITS_PER_ELEMENT));
    return 0;
}

inline int sr_clear_bit(sr_bit_arr_t *bit_arr, size_t bit_arr_len, int b)
{
    if (b < 0 || b >= (bit_arr_len * SR_BITS_PER_ELEMENT)) {
        return -1; // Invalid channel number
    }

    bit_arr[b / SR_BITS_PER_ELEMENT] &= ~(1 << (b % SR_BITS_PER_ELEMENT));
    return 0;
}

inline int sr_read_bit(sr_bit_arr_t const *bit_arr, size_t bit_arr_len, int b)
{
    if (b < 0 || b >= (bit_arr_len * SR_BITS_PER_ELEMENT)) {
        return -1; // Invalid channel number
    }

    return bit_arr[b / SR_BITS_PER_ELEMENT] & (1 << (b % SR_BITS_PER_ELEMENT)) ? 1 : 0;
}