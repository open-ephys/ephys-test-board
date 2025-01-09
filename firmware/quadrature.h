#pragma once

#include "hardware/pio.h"
#include "quadrature.pio.h"

typedef struct pio_quad_inst {
    PIO pio;
    uint sm;
    int dma_chan;
    volatile int32_t counter;
} pio_quad_inst_t;

int quad_init(pio_quad_inst_t *quad);
int quad_get_count(const pio_quad_inst_t *quad);