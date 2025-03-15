#pragma once

#include "pico/stdlib.h"
#include "ephys-tester.h"

typedef enum  {
    SELECTION_MODE,
    SELECTION_WAVEFORM,
    SELECTION_CHANNEL,
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
    uint16_t amplitude_uV;
    uint16_t freq_lut_idx;
} mode_signal_t;

typedef struct mode_context_t {
    mode_selection_t selection;         // The user selection
    mode_test_t test_dest;              // Where to route test signal
    uint channel_map[MAX_NUM_CHANNELS]; // Index is the headstage channel, value is the test board channel
    uint num_channels;                  // Number of headstage channels
    uint channel_idx;                   // Selected headstage channel
    mode_signal_t signal;               // The test signal
} mode_context_t;

void mode_init(mode_context_t *ctx);
//int mode_reset_channels(mode_context_t *ctx, uint *channel_map, uint num_channels);
int mode_update(mode_context_t *ctx, int delta, bool button_pushed);
mode_selection_t mode_selection(const mode_context_t *ctx);

/// @brief Gets a string representing the value of a selection within a given mode context.
/// @param ctx Mode context
/// @param selection Mode selection for which a string description is needed.
/// @return string version of the requested selection. Calling this function
/// again may mutate the previously returned string. Make sure to copy this or
/// apply it hardware prior to that.
char *mode_str(const mode_context_t *ctx, mode_selection_t selection);

