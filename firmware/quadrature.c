#include "ephys-tester.h"
#include "quadrature.h"
#include "hardware/irq.h"

#include <stdio.h>

static struct pio_quad_inst_t {
    PIO pio;
    uint sm;
    int dma_chan;
    volatile int32_t raw_counter;
    volatile int count;
    volatile int delta;
    volatile bool update;

} pio_quad;

static void knob_turned_handler()
{
    dma_channel_acknowledge_irq0(pio_quad.dma_chan); // Acknowledge

    // Hysteresis
    // static int32_t hyst = 0;
    // int current_count = (pio_quad.raw_counter + hyst) >> 2;

    int current_count = pio_quad.raw_counter >> 2;


    if (current_count != pio_quad.count )
    {
        // Four raw counts between each detent
        // NB: we dont want to do integer division here because this will round toward zero
        // for both positive and negative numbers. We want to always round towards
        // negative infinity. Otherwise when we transition to negative numbers, 7
        // raw_counter values [-3 to 3] will be mapped to 0.
        //hyst = current_count > pio_quad.count ? 4 : -4;
        pio_quad.update = true;
        pio_quad.delta = current_count - pio_quad.count;
        pio_quad.count = current_count;
    } else {
        pio_quad.update = false;
    }

    dma_channel_start(pio_quad.dma_chan); // Retrigger
}

int quad_init()
{
    pio_quad.pio = ENC_PIO;
    pio_quad.sm = 0;
    pio_quad.dma_chan = 0;
    pio_quad.raw_counter = 0;
    pio_quad.count = 0;
    pio_quad.delta = 0;
    pio_quad.update = false;

    uint quadrature_prog_offs = pio_add_program(pio_quad.pio, &quadrature_decoder_program);
    pio_quadrature_init(pio_quad.pio, pio_quad.sm, ENC_A, &pio_quad.dma_chan, &pio_quad.raw_counter, knob_turned_handler);
    knob_turned_handler();

    return 0;
}

inline bool quad_pending_turn()
{
    return pio_quad.update;
}

inline void quad_acknowledge_turn()
{
    pio_quad.update = false;
}

inline int quad_get_count()
{
    return pio_quad.count;
}

inline int quad_get_delta()
{
    return pio_quad.delta;
}