#include "mode.h"
#include "lut.h"

#include <stdio.h>

static inline void increment_selection(mode_context_t *ctx)
{
    ctx->selection = (mode_selection_t)((ctx->selection + 1) % SELECTION_NUM_SELECTIONS);
}

static inline void decrement_selection(mode_context_t *ctx)
{
    ctx->selection = (mode_selection_t)((ctx->selection) == 0 ? SELECTION_NUM_SELECTIONS - 1 : ctx->selection - 1);
}

static inline void increment_mode(mode_context_t *ctx)
{
    ctx->test_dest = (mode_test_t)((ctx->test_dest + 1) % TEST_NUM_DESTINATIONS);
}

static inline void decrement_mode(mode_context_t *ctx)
{
    ctx->test_dest = (mode_test_t)((ctx->test_dest) == 0 ? TEST_NUM_DESTINATIONS - 1 : ctx->test_dest - 1);
}

static inline void increment_waveform(mode_signal_t *sig)
{
    sig->waveform = (mode_waveform_t)((sig->waveform + 1) % WAVEFORM_NUM_WAVEFORMS);
}

static inline void decrement_waveform(mode_signal_t *sig)
{
    sig->waveform = (mode_waveform_t)((sig->waveform) == 0 ? WAVEFORM_NUM_WAVEFORMS - 1 : sig->waveform - 1);
}

static inline void change_channel_idx(mode_context_t *ctx, int delta)
{
    if (ctx->test_dest != TEST_SINGLE_CHANNEL)
        return;

    int new_channel_idx = (int)ctx->channel_idx + delta;
    if (new_channel_idx < 0)
        ctx->channel_idx = ctx->num_channels - 1;
    else if (new_channel_idx > ctx->num_channels - 1)
        ctx->channel_idx = 0;
    else
        ctx->channel_idx = new_channel_idx;
}

// TODO: place: 1, 10, 100, 1000
// TODO: MAX_UV
static inline void change_amplitude(mode_signal_t *sig, int delta, int place)
{
    if (sig->waveform == WAVEFORM_EXTERNAL)
        return;

    int new_amplitude_uV = sig->amplitude_uV + delta * place;

    if (new_amplitude_uV < 0)
        sig->amplitude_uV = MAX_AMPLITUDE_UV;
    else if (new_amplitude_uV > MAX_AMPLITUDE_UV)
        sig->amplitude_uV = 0;
    else
        sig->amplitude_uV = new_amplitude_uV;
}

// TODO: place: 1, 10, 100, 1000
// TODO: MAX_UV
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
    if (ctx->test_dest == TEST_SINGLE_CHANNEL)
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
        static char str[5];
        snprintf(str, 5, "%u", sig->amplitude_uV);
        return str;
    } else {
        return "~";
    }
}

static char *string_freq_hz(const mode_signal_t *const sig)
{
    if (sig->waveform != WAVEFORM_GND && sig->waveform != WAVEFORM_EXTERNAL)
    {
        static char str[7];
        snprintf(str, 7, "%f4.1", (float)FREQ_LUT[sig->freq_lut_idx][0] / (float)FREQ_LUT[sig->freq_lut_idx][1]);
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
    ctx->signal.amplitude_uV = 200;
    ctx->signal.freq_lut_idx = DEFAULT_FREQ_INDEX;
}

int mode_update(mode_context_t *ctx, int delta, bool button_pushed)
{
    if (button_pushed)
    {
        delta > 0 ? increment_selection(ctx) : decrement_selection(ctx);
    } else
    {
        switch (ctx->selection)
        {
            case SELECTION_MODE:
                delta > 0 ? increment_mode(ctx) : decrement_mode(ctx);
                break;
            case SELECTION_WAVEFORM:
                delta > 0 ? increment_waveform(&ctx->signal) : decrement_waveform(&ctx->signal);
                break;
            case SELECTION_CHANNEL:
                change_channel_idx(ctx, delta);
                break;
            case SELECTION_AMPLITUDE:
                change_amplitude(&ctx->signal, delta, 10);
                break;
            case SELECTION_FREQHZ:
                delta > 0 ? increment_freq_hz(&ctx->signal) : decrement_freq_hz(&ctx->signal);
                break;
            default : // Invalid
                return -1; // TODO: error codes
        }
    }

    return 0;
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