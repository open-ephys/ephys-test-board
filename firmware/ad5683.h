#pragma once

#include "hardware/pio.h"
#include "spi.pio.h"

typedef struct pio_spi_inst {
    PIO pio;
    uint sm;
    uint cs_pin;
} pio_spi_inst_t;


int ad5683_init(const pio_spi_inst_t *spi);
int ad5683_write_dac(const pio_spi_inst_t *spi, uint16_t code);