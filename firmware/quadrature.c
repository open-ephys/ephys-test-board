#include "ephys-tester.h"
#include "quadrature.h"


int quad_init(pio_quad_inst_t *quad)
{
    uint quadrature_prog_offs = pio_add_program(quad->pio, &quadrature_decoder_program);
    pio_quadrature_init(quad->pio, quad->sm, ENC_A, &quad->dma_chan, &quad->counter);
    return 0;
}

int quad_get_count(const pio_quad_inst_t *quad)
{

    int32_t count = quad->counter;
    uint32_t dma_transfers_rem = dma_channel_hw_addr(quad->dma_chan)->transfer_count;
    if (dma_transfers_rem < 0x80000000) 
    {
        // NB: If we have used half of our available transfers, restart and
        // recover the 0xFFFFFFFF transfer count
        dma_channel_abort(quad->dma_chan);
        dma_channel_start(quad->dma_chan);
    }

    return count / 4 ; // NB: 4 pulses per detent using the PEC12R-4222F-S0024
}