#include "ephys-tester.h"
#include "quadrature.h"
#include "hardware/irq.h"

#include <stdio.h>

#define QUAD_COUNTS_PER_DETENT 4

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
    dma_channel_acknowledge_irq0(pio_quad.dma_chan);

    // Displacement-from-anchor approach: register a detent when |raw counter|
    // has moved >= QUAD_COUNTS_PER_DETENT from the last accepted detent
    // position (anchor). Advancing the anchor on each registration provides
    // hysteresis: e.g. a backward bounce of 1-3 counts never reaches the
    // -QUAD_COUNTS_PER_DETENT threshold.
    static bool initialized = false;
    static int32_t anchor = 0;
    int32_t raw = pio_quad.raw_counter;

    if (!initialized)
    {
        anchor = raw;
        initialized = true;
        dma_channel_start(pio_quad.dma_chan);
        return;
    }

    int32_t displacement = raw - anchor;

    if (displacement >= QUAD_COUNTS_PER_DETENT || displacement <= -QUAD_COUNTS_PER_DETENT)
    {
        int detents = displacement / QUAD_COUNTS_PER_DETENT;
        pio_quad.count += detents;
        pio_quad.delta = detents;
        anchor += detents * QUAD_COUNTS_PER_DETENT;
        pio_quad.update = true;
    }

    dma_channel_start(pio_quad.dma_chan);
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