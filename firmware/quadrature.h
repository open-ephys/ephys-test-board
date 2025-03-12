#pragma once

#include "hardware/pio.h"
#include "quadrature.pio.h"

// typedef struct pio_quad_inst {
//     PIO pio;
//     uint sm;
//     int dma_chan;
//     int delta;
//     volatile int32_t counter;
// } pio_quad_inst_t;

// int quad_init(pio_quad_inst_t *quad);
// int quad_get_count(const pio_quad_inst_t *quad);


int quad_init();
int quad_get_count();

bool quad_pending_turn();
void quad_acknowledge_turn();
int quad_get_count();
int quad_get_delta();
bool quad_get_button();
