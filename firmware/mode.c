#include "mode.h"
#include "lut.h"

#include <stdio.h>
#include <string.h>

static inline void increment_mode(mode_context_t *ctx)
{
    ctx->test_dest = ctx->test_dest == TEST_NUM_DESTINATIONS - 1 ?
        ctx->test_dest :
        (mode_test_t)((ctx->test_dest + 1));
}

static inline void decrement_mode(mode_context_t *ctx)
{
    ctx->test_dest = ctx->test_dest == TEST_OPEN ?
        ctx->test_dest :
        (mode_test_t)((ctx->test_dest - 1));
}

static inline void increment_waveform(mode_signal_t *sig)
{
    sig->waveform = sig->waveform == WAVEFORM_NUM_WAVEFORMS - 1 ?
        sig->waveform :
        (mode_waveform_t)(sig->waveform + 1);
}

static inline void decrement_waveform(mode_signal_t *sig)
{
    sig->waveform = sig->waveform == WAVEFORM_GND ?
        sig->waveform :
        (mode_waveform_t)(sig->waveform - 1);
}

static inline void change_channel_idx(mode_context_t *ctx, int delta, bool override)
{
    if (ctx->test_dest != TEST_SINGLE_CHANNEL && !override)
        return;

    int new_channel_idx = (int)ctx->channel_idx + delta;
    if (new_channel_idx < 0)
        ctx->channel_idx = ctx->num_channels - 1;
    else if (new_channel_idx > ctx->num_channels - 1)
        ctx->channel_idx = 0;
    else
        ctx->channel_idx = new_channel_idx;
}

static inline void increment_amplitude(mode_signal_t *sig)
{
    sig->amp_rshift = sig->amp_rshift == 0 ?
        0 :
        sig->amp_rshift - 1;
}

static inline void decrement_amplitude(mode_signal_t *sig)
{
    sig->amp_rshift = sig->amp_rshift == 10 ?
        10 :
        sig->amp_rshift + 1;
}

static inline void increment_freq_hz(mode_signal_t *sig)
{
    if (sig->waveform == WAVEFORM_EXTERNAL)
        return;

    sig->freq_lut_idx = sig->freq_lut_idx == NUM_FREQS - 1 ?
        NUM_FREQS - 1 :
        sig->freq_lut_idx + 1;
}

static inline void decrement_freq_hz(mode_signal_t *sig)
{
    if (sig->waveform == WAVEFORM_EXTERNAL)
        return;

    sig->freq_lut_idx = sig->freq_lut_idx == 0 ?
        0 :
        sig->freq_lut_idx - 1;
}

static const char* string_mode(const mode_context_t *const ctx)
{
    switch(ctx->test_dest)
    {
        case TEST_OPEN:
            return "Open";
        case TEST_SINGLE_CHANNEL:
            return "Single channel";
        case TEST_ALL_CHANNEL:
            return "All channels";
        case TEST_CYCLE_CHANNEL:
            return "Cycle channels";
        default:
            return "Invalid mode";
    }
}

static const char* string_waveform(const mode_signal_t *const sig)
{
    switch(sig->waveform)
    {
        case WAVEFORM_GND:
            return "Ground";
        case WAVEFORM_SINE:
            return "Sine";
        case WAVEFORM_SAW:
            return "Sawtooth";
        case WAVEFORM_SPIKES:
            return "Spikes";
        case WAVEFORM_EXTERNAL:
            return "External";
        default:
            return "Invalid";
    }
}

static char *string_channel_idx(const mode_context_t *const ctx)
{
    if (ctx->test_dest == TEST_SINGLE_CHANNEL || ctx->test_dest == TEST_CYCLE_CHANNEL)
    {
        static char str[5];
        snprintf(str, 5, "%u",ctx->channel_idx);
        return str;
    } else {
        return "~";
    }
}

static char *string_amplitude_uV(const mode_signal_t *const sig)
{
    if (sig->waveform != WAVEFORM_GND && sig->waveform != WAVEFORM_EXTERNAL)
    {
        static char str[8];
        snprintf(str, 8, "%f3.4", MAX_AMPLITUDE_UV / (1 << sig->amp_rshift));
        return str;
    } else {
        return "~";
    }
}

static char *string_freq_hz(const mode_signal_t *const sig)
{
    if (sig->waveform != WAVEFORM_GND && sig->waveform != WAVEFORM_EXTERNAL)
    {
        static char str[8];
        snprintf(str, 8, "%f4.1", (float)FREQ_LUT[sig->freq_lut_idx][0] / (float)FREQ_LUT[sig->freq_lut_idx][1]);
        return str;
    } else {
        return "~";
    }
}

void mode_init(mode_context_t *ctx)
{
    ctx->selection = SELECTION_MODE;
    ctx->test_dest = TEST_SINGLE_CHANNEL;
    ctx->channel_idx = 0;
    ctx->num_channels = MAX_NUM_CHANNELS;
    for (int i = 0; i++; i < MAX_NUM_CHANNELS) { ctx->channel_map[i] = i; }
    ctx->signal.waveform = WAVEFORM_SINE;
    ctx->signal.amp_rshift = 0;
    ctx->signal.freq_lut_idx = DEFAULT_FREQ_INDEX;
}

int mode_update_from_knob(mode_context_t *ctx, int delta) //, bool button_pushed)
{

    switch (ctx->selection)
    {
        case SELECTION_MODE:
            delta > 0 ? increment_mode(ctx) : decrement_mode(ctx);
            return 1; // Input routing mode changed
        case SELECTION_WAVEFORM:
            delta > 0 ? increment_waveform(&ctx->signal) : decrement_waveform(&ctx->signal);
            return 2; // Signal changed
        case SELECTION_CHANNEL:
            change_channel_idx(ctx, delta, false);
            return 3; // Channel selection changed
        case SELECTION_AMPLITUDE:
            delta > 0 ? increment_amplitude(&ctx->signal) : decrement_amplitude(&ctx->signal);
            return 2; // Signal changed
        case SELECTION_FREQHZ:
            delta > 0 ? increment_freq_hz(&ctx->signal) : decrement_freq_hz(&ctx->signal);
            return 2; // Signal changed
        default : // Invalid
            return -1; // TODO: error codes
    }
}

inline void mode_increment_channel(mode_context_t *ctx)
{
    change_channel_idx(ctx, 1, true);
}

inline mode_selection_t mode_selection(const mode_context_t *const ctx)
{
    return ctx->selection;
}

char *mode_str(const mode_context_t *const ctx, mode_selection_t selection)
{
    switch (selection)
    {
        case SELECTION_MODE:
            return string_mode(ctx);
        case SELECTION_WAVEFORM:
            return string_waveform(&ctx->signal);
        case SELECTION_CHANNEL:
            return string_channel_idx(ctx);
        case SELECTION_AMPLITUDE:
            return string_amplitude_uV(&ctx->signal);
        case SELECTION_FREQHZ:
            return string_freq_hz(&ctx->signal);
        default:
            return "Invalid";
    }
}