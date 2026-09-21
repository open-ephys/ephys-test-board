#pragma once

#include "hardware/pio.h"
#include "spi.pio.h"

#include "ephys-tester.h"

typedef struct pio_spi_inst {
    PIO pio;
    uint sm;
    uint cs_pin;
} pio_spi_inst_t;

int ad5683_init(const pio_spi_inst_t *spi);
signal_clip_t ad5683_write_dac_rs(const pio_spi_inst_t *spi, uint16_t code, uint16_t rshift, uint16_t offset);
signal_clip_t ad5683_write_dac_scale(const pio_spi_inst_t *spi, uint16_t code, float scale, uint16_t offset);