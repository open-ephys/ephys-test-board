#include "ephys-tester.h"
#include "quadrature.h"
#include "hardware/irq.h"

#include <stdio.h>

#define PHASE_DETECTION_MAX_DWELL_MSEC 500

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

    // Detent phase detection via time-spent histogram state
    static uint64_t phase_time[4] = {0};
    static uint8_t  detent_phase  = 0;
    static uint8_t  last_residue  = 0xFF;
    static uint32_t residue_entry_us = 0;

    // raw_counter is signed and residue is index
    uint8_t residue = ((pio_quad.raw_counter % 4) + 4) % 4;

    if (last_residue == 0xFF)
    {
        // First call: just record where we are, nothing to accumulate yet
        last_residue     = residue;
        residue_entry_us = time_us_32();
    }
    else if (residue != last_residue)
    {
        // We moved: credit the time spent in the previous residue
        uint32_t now = time_us_32() / 1000;
        uint32_t elapsed = now - residue_entry_us; // wraps safely for uint32_t
        elapsed = elapsed > PHASE_DETECTION_MAX_DWELL_MSEC ? PHASE_DETECTION_MAX_DWELL_MSEC : elapsed;
        phase_time[last_residue] += elapsed * elapsed; //< PHASE_DETECTION_MAX_DWELL_MSEC ? elapsed : PHASE_DETECTION_MAX_DWELL_MSEC;

        // Update detent_phase to whichever bucket is largest
        if (phase_time[last_residue] > phase_time[detent_phase])
            detent_phase = last_residue;

        last_residue = residue;
        residue_entry_us = now;
    }

    // Four raw counts between each detent
    // NB: we dont want to do integer division here because this will round
    // toward zero for both positive and negative numbers. We want to always
    // round towards negative infinity. Otherwise when we transition to negative
    // numbers, 7 raw_counter values [-3 to 3] will be mapped to 0.
    int current_count = (pio_quad.raw_counter - detent_phase) >> 2;
    int delta = current_count - pio_quad.count;

    if (delta != 0)
    {
        static int last_delta = 0;
        // Only register if moving in the same direction as last move
        if ((delta > 0) == (last_delta > 0) || last_delta == 0)
        {
            pio_quad.delta = delta;
            pio_quad.count = current_count;
            pio_quad.update = true;
        }
        last_delta = delta;
    }
    else
    {
        pio_quad.update = false;
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