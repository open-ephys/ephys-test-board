#include "ad5683.h"
#include "ephys-tester.h"

#define AD5683_WRITE_AND_LATCH_CMD (0b0011u << 28)

// NB: Modified from https://github.com/raspberrypi/pico-examples/blob/master/pio/spi/pio_spi.c
static void __time_critical_func(pio_spi_write32_nonblocking)(const pio_spi_inst_t *spi, const uint32_t src) {

    io_rw_32 *txfifo = &spi->pio->txf[spi->sm];
    const io_rw_32 *rxfifo = &spi->pio->rxf[spi->sm];

    if (!pio_sm_is_tx_fifo_full(spi->pio, spi->sm))
    {
        *txfifo = src;
        (void) *rxfifo; // NB: Read from RX FIFO to clear it
    }
}

// NB: Modified from https://github.com/raspberrypi/pico-examples/blob/master/pio/spi/pio_spi.c
static void __time_critical_func(pio_spi_write32_blocking)(const pio_spi_inst_t *spi, const uint32_t src) {
    size_t tx_remain = 1, rx_remain = 1;
    io_rw_32 *txfifo = &spi->pio->txf[spi->sm];
    const io_rw_32 *rxfifo = &spi->pio->rxf[spi->sm];

    while (tx_remain || rx_remain) {
        if (tx_remain && !pio_sm_is_tx_fifo_full(spi->pio, spi->sm)) {
            *txfifo = src;
            --tx_remain;
        }

        // NB: Block until transaction is complete
        if (rx_remain && !pio_sm_is_rx_fifo_empty(spi->pio, spi->sm)) {
            (void) *rxfifo; // NB: Read from RX FIFO to clear it
            --rx_remain;
        }
    }
}

int ad5683_init(const pio_spi_inst_t *spi)
{
    const float clkdiv = 1.0f; // NB: ~31 MHz SCLK with this implementation
    uint cpha1_prog_offs = pio_add_program(spi->pio, &spi_cpha1_cs_program);
    pio_spi_cs_init(spi->pio,
                    spi->sm,
                    cpha1_prog_offs,
                    24,
                    clkdiv,
                    1,
                    0,
                    DAC_nCS,
                    DAC_MOSI,
                    DAC_MOSI);

    return 0;
}

signal_clip_t ad5683_write_dac_rs(const pio_spi_inst_t *spi, uint16_t code, uint16_t rshift, uint16_t offset)
{
    signal_clip_t rc = CLIP_NONE;
    int32_t val = ((int32_t)code - DAC_MIDSCALE) >> rshift;
    val += offset;

    if (val < 0) { val = 0; rc = CLIP_LOW;  }
    if (val > DAC_FULLSCALE) { val = DAC_FULLSCALE; rc = CLIP_HIGH; }

    // NB: PIO SPI controller will take 24 bits starting at position 31 and
    // working down
    pio_spi_write32_nonblocking(spi, AD5683_WRITE_AND_LATCH_CMD | (uint32_t)val << 12);
    return rc;
}

signal_clip_t ad5683_write_dac_scale(const pio_spi_inst_t *spi, uint16_t code, float scale, uint16_t offset)
{
    signal_clip_t rc = CLIP_NONE;
    int32_t val = (int32_t)((code - DAC_MIDSCALE) * scale);
    val += offset;

    if (val < 0) { val = 0; rc = CLIP_LOW;  }
    if (val > DAC_FULLSCALE) { val = DAC_FULLSCALE; rc = CLIP_HIGH; }

    // NB: PIO SPI controller will take 24 bits starting at position 31 and
    // working down
    pio_spi_write32_nonblocking(spi, AD5683_WRITE_AND_LATCH_CMD | (uint32_t)val << 12);
    return rc;
}