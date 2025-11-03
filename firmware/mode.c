#include "mode.h"
#include "lut.h"

#include <stdio.h>

static inline void increment_dest(mode_context_t *ctx)
{
    ctx->test_dest = ctx->test_dest == DEST_NUM_DESTINATIONS - 1 ?
        ctx->test_dest :
        (mode_dest_t)((ctx->test_dest + 1));
}

static inline void decrement_dest(mode_context_t *ctx)
{
    ctx->test_dest = ctx->test_dest == DEST_NONE ?
        ctx->test_dest :
        (mode_dest_t)((ctx->test_dest - 1));
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
    if (ctx->test_dest != DEST_SINGLE_CHANNEL && !override)
        return;

    int new_channel_idx = (int)ctx->channel_idx + delta;
    if (new_channel_idx < 0)
        ctx->channel_idx = ctx->channel_map.num_channels - 1;
    else if (new_channel_idx > ctx->channel_map.num_channels - 1)
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
    sig->amp_rshift = sig->amp_rshift == DAC_MAX_SHIFT ?
        DAC_MAX_SHIFT :
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
        case DEST_NONE:
            return "None";
        case DEST_SINGLE_CHANNEL:
            return "Single channel";
        case DEST_ALL_CHANNEL:
            return "All channels";
        case DEST_CYCLE_CHANNEL_SLOW:
            return "Cycle (" STRINGIFY(AUTO_CHAN_SDWELL_S) " sec)";
        case DEST_CYCLE_CHANNEL_FAST:
            return "Cycle (" STRINGIFY(AUTO_CHAN_FDWELL_MS) " ms)";
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
        case WAVEFORM_SPIKESLF:
            return "Low FR Spikes";
        case WAVEFORM_SPIKESMF:
            return "Med FR Spikes";
        case WAVEFORM_SPIKESHF:
            return "Hi FR Spikes";
        case WAVEFORM_EXTERNAL:
            return "External";
        default:
            return "Invalid";
    }
}

static const char *string_channel_idx(const mode_context_t *const ctx)
{
    if (ctx->test_dest == DEST_SINGLE_CHANNEL || ctx->test_dest == DEST_CYCLE_CHANNEL_SLOW || ctx->test_dest == DEST_CYCLE_CHANNEL_FAST)
    {
        static char str[5];
        snprintf(str, sizeof(str), "%u",ctx->channel_idx);
        return str;
    } else if (ctx->test_dest == DEST_ALL_CHANNEL) {
        return "All";
    } else {
        return "~";
    }
}

static const char *string_amplitude(const mode_signal_t *const sig)
{
    if (sig->waveform == WAVEFORM_SINE || sig->waveform == WAVEFORM_SAW)
    {
        static char str[8];
        snprintf(str, sizeof(str), "%f7.4", MAX_AMPLITUDE_UV / (1 << sig->amp_rshift));
        return str;
    }
    else if (sig->waveform == WAVEFORM_SPIKESLF || sig->waveform == WAVEFORM_SPIKESMF || sig->waveform == WAVEFORM_SPIKESHF)
    {
        static char str[8];
        snprintf(str, sizeof(str), "%f7.4", 1.0f / (1 << sig->amp_rshift));
        return str;
    }
    else // WAVEFORM_GND, WAVEFORM_EXTERNAL
    {
        return "";
    }
}

static const char *string_title_amplitude(const mode_signal_t *const sig)
{
    if (sig->waveform == WAVEFORM_SINE || sig->waveform == WAVEFORM_SAW)
    {
       return "Amp. (uV):";
    }
    else if (sig->waveform == WAVEFORM_SPIKESLF || sig->waveform == WAVEFORM_SPIKESMF || sig->waveform == WAVEFORM_SPIKESHF)
    {
        return "Atten.:";
    }
    else // WAVEFORM_GND, WAVEFORM_EXTERNAL
    {
        return "";
    }
}

static const char *string_freq(const mode_signal_t *const sig)
{
    if (sig->waveform == WAVEFORM_SINE || sig->waveform == WAVEFORM_SAW)
    {
        static char str[8];
        snprintf(str, sizeof(str), "%f5.1", (float)FREQ_LUT[sig->freq_lut_idx][0] / (float)FREQ_LUT[sig->freq_lut_idx][1]);
        return str;
    }
    else if (sig->waveform == WAVEFORM_SPIKESLF || sig->waveform == WAVEFORM_SPIKESMF || sig->waveform == WAVEFORM_SPIKESHF)
    {
        return "30 kHz";
    } else {
        return "";
    }
}

static const char *string_title_freq(const mode_signal_t *const sig)
{
    if (sig->waveform == WAVEFORM_SINE || sig->waveform == WAVEFORM_SAW)
    {
       return "Freq. (Hz):";
    }
    else if (sig->waveform == WAVEFORM_SPIKESLF || sig->waveform == WAVEFORM_SPIKESMF || sig->waveform == WAVEFORM_SPIKESHF)
    {
        return "Fs:";
    }
    else // WAVEFORM_GND, WAVEFORM_EXTERNAL
    {
        return "";
    }
}

void mode_init(mode_context_t *ctx)
{
    ctx->selection = SELECTION_DEST;
    ctx->test_dest = DEST_SINGLE_CHANNEL;
    ctx->channel_idx = 0;
    ctx->channel_map.num_channels = MAX_NUM_CHANNELS;
    for (int i = 0; i < MAX_NUM_CHANNELS; i++) { ctx->channel_map.channel_map[i] = i; }
    ctx->signal.waveform = WAVEFORM_SINE;
    ctx->signal.amp_rshift = 0;
    ctx->signal.freq_lut_idx = DEFAULT_FREQ_INDEX;
    ctx->battery_frac = 1.0;
    ctx->usb_detected = false;
}

void mode_cycle_selection(mode_context_t *ctx)
{
    int wrap_value = SELECTION_NUM_SELECTIONS;
    int inc_value = ctx->selection == SELECTION_DEST ? (ctx->test_dest == DEST_SINGLE_CHANNEL ? 1 : 2) : 1;

    if (ctx->signal.waveform == WAVEFORM_GND || ctx->signal.waveform == WAVEFORM_EXTERNAL)
    {
        wrap_value = SELECTION_AMPLITUDE;
    }
    else if (ctx->signal.waveform == WAVEFORM_SPIKESLF || ctx->signal.waveform == WAVEFORM_SPIKESMF || ctx->signal.waveform == WAVEFORM_SPIKESHF)
    {
        wrap_value = SELECTION_FREQHZ;
    }

    ctx->selection = (mode_selection_t)((ctx->selection + inc_value) % wrap_value);
}

mode_update_result_t mode_update_from_knob(mode_context_t *ctx, int delta)
{
    switch (ctx->selection)
    {
        case SELECTION_DEST:
            delta > 0 ? increment_dest(ctx) : decrement_dest(ctx);
            return MODE_UPDATE_INPUTSOURCE;
        case SELECTION_WAVEFORM:
            delta > 0 ? increment_waveform(&ctx->signal) : decrement_waveform(&ctx->signal);
            return MODE_UPDATE_SIGNAL;
        case SELECTION_CHANNEL:
            change_channel_idx(ctx, delta, false);
            return MODE_UPDATE_CHANNEL;
        case SELECTION_AMPLITUDE:
            delta > 0 ? increment_amplitude(&ctx->signal) : decrement_amplitude(&ctx->signal);
            return MODE_UPDATE_SIGNAL;
        case SELECTION_FREQHZ:
            delta > 0 ? increment_freq_hz(&ctx->signal) : decrement_freq_hz(&ctx->signal);
            return MODE_UPDATE_SIGNAL;
        default :
            return MODE_UPDATE_EINVALID;
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

const char *mode_str(const mode_context_t *const ctx, mode_selection_t selection)
{
    switch (selection)
    {
        case SELECTION_DEST:
            return string_mode(ctx);
        case SELECTION_WAVEFORM:
            return string_waveform(&ctx->signal);
        case SELECTION_CHANNEL:
            return string_channel_idx(ctx);
        case SELECTION_AMPLITUDE:
            return string_amplitude(&ctx->signal);
        case SELECTION_FREQHZ:
            return string_freq(&ctx->signal);
        default:
            return "Invalid";
    }
}

const char *title_str(const mode_context_t *const ctx, mode_selection_t selection)
{
    switch (selection)
    {
        case SELECTION_DEST:
            return "Dest.:";
        case SELECTION_WAVEFORM:
            return "Signal:";
        case SELECTION_CHANNEL:
            return "Channel:";
        case SELECTION_AMPLITUDE:
            return string_title_amplitude(&ctx->signal);
        case SELECTION_FREQHZ:
            return string_title_freq(&ctx->signal);;
        default:
            return "Invalid";
    }
}