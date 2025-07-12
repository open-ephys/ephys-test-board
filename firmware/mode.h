#pragma once

#include "pico/stdlib.h"
#include "ephys-tester.h"

typedef enum  {
    SELECTION_MODE,
    SELECTION_CHANNEL,
    SELECTION_WAVEFORM,
    SELECTION_AMPLITUDE,
    SELECTION_FREQHZ,
    SELECTION_NUM_SELECTIONS
} mode_selection_t;

typedef enum  {
    TEST_OPEN,
    TEST_SINGLE_CHANNEL,
    TEST_ALL_CHANNEL,
    TEST_CYCLE_CHANNEL,
    TEST_NUM_DESTINATIONS
} mode_test_t;

typedef enum {
    WAVEFORM_GND,
    WAVEFORM_SINE,
    WAVEFORM_SAW,
    WAVEFORM_SPIKES,
    WAVEFORM_EXTERNAL,
    WAVEFORM_NUM_WAVEFORMS
} mode_waveform_t;

typedef struct mode_signal_t {
    mode_waveform_t waveform;
    uint16_t amp_rshift;
    uint16_t freq_lut_idx;
} mode_signal_t;

typedef struct module_t {
    uint8_t sn[16];
    char name[MODULE_NAME_BYTES + 1]; // + 1 for null termination
    char pcb_rev;
} module_t;

typedef struct mode_context_t {
    mode_selection_t selection;             // The user selection
    mode_test_t test_dest;                  // Where to route test signal
    uint8_t channel_map[MAX_NUM_CHANNELS];  // Index is the headstage channel, value is the test board channel
    uint8_t num_channels;                   // Number of headstage channels
    uint8_t channel_idx;                    // Selected headstage channel
    mode_signal_t signal;                   // The test signal
    module_t module;                        // Module metadata
    float battery_frac;                     // Fraction of battery remaining
    bool usb_detected;                      // Indicates if USB is plugged in
} mode_context_t;

void mode_init(mode_context_t *ctx);
int mode_update_from_knob(mode_context_t *ctx, int delta);
void mode_increment_channel(mode_context_t *ctx);
mode_selection_t mode_selection(const mode_context_t *ctx);

/// @brief Gets a string representing the value of a selection within a given mode context.
/// @param ctx Mode context
/// @param selection Mode selection for which a string description is needed.
/// @return string version of the requested selection. Calling this function
/// again may mutate the previously returned string. Make sure to copy this or
/// apply it hardware prior to that.
const char *mode_str(const mode_context_t *ctx, mode_selection_t selection);

static inline void mode_cycle_selection(mode_context_t *ctx)
{
    ctx->selection = (mode_selection_t)((ctx->selection + 1) % SELECTION_NUM_SELECTIONS);
}




