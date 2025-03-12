#pragma once

#include "ephys-tester.h"

typedef enum {
  EXT_SIGNAL = 0,
  INT_SIGNAL = 1,
} signal_source_t;

int sr_init();
int sr_source(signal_source_t source);
int sr_update(sr_bit_arr_t const *bit_arr);

inline bool sr_equal(sr_bit_arr_t *bit_arr_0, sr_bit_arr_t *bit_arr_1)
{
    for (int i = 0; i < sizeof(sr_bit_arr_t); i++) 
    {
        if (bit_arr_0->bits[i] != bit_arr_1->bits[i]) return false;
    }

    return true;
}

inline void sr_clear(sr_bit_arr_t *bit_arr)
{
    for (int i = 0; i < sizeof(sr_bit_arr_t) / sizeof(bit_arr->bits[0]); i++)
    {
        bit_arr->bits[i] = 0;
    }
}

inline int sr_set_bit(sr_bit_arr_t *bit_arr, int b)
{
    if (b < 0 || b >= MAX_NUM_CHANNELS) {
        return -1; // Invalid channel number
    }

    bit_arr->bits[b / SR_BITS_PER_ELEMENT] |= (1 << (b % SR_BITS_PER_ELEMENT));
    return 0;
}

inline int sr_clear_bit(sr_bit_arr_t *bit_arr, int b)
{
    if (b < 0 || b >= MAX_NUM_CHANNELS) {
        return -1; // Invalid channel number
    }

    bit_arr->bits[b / SR_BITS_PER_ELEMENT] &= ~(1 << (b % SR_BITS_PER_ELEMENT));
    return 0;
}

inline int sr_get_bit(sr_bit_arr_t const *bit_arr, int b)
{
    if (b < 0 || b >= MAX_NUM_CHANNELS) {
        return -1; // Invalid channel number
    }

    return bit_arr->bits[b / SR_BITS_PER_ELEMENT] & (1 << (b % SR_BITS_PER_ELEMENT)) ? 1 : 0;
}