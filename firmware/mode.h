#pragma once

#include "pico/stdlib.h"
#include "ephys-tester.h"

typedef enum  {
    SELECTION_DEST,
    SELECTION_CHANNEL,
    SELECTION_WAVEFORM,
    SELECTION_AMPLITUDE,
    SELECTION_FREQHZ,
    SELECTION_NUM_SELECTIONS
} mode_selection_t;

typedef enum  {
    DEST_NONE,
    DEST_SINGLE_CHANNEL,
    DEST_ALL_CHANNEL,
    DEST_CYCLE_CHANNEL_SLOW,
    DEST_CYCLE_CHANNEL_FAST,
    DEST_NUM_DESTINATIONS
} mode_dest_t;

typedef enum {
    WAVEFORM_GND,
    WAVEFORM_SINE,
    WAVEFORM_SAW,
    WAVEFORM_SPIKESLF,
    WAVEFORM_SPIKESMF,
    WAVEFORM_SPIKESHF,
    WAVEFORM_EXTERNAL,
    WAVEFORM_NUM_WAVEFORMS
} mode_waveform_t;

typedef enum  {
    MODE_UPDATE_EINVALID = -1, // Invalid update
    MODE_UPDATE_NONE = 0, // No change
    MODE_UPDATE_SIGNAL = 1, // Signal changed
    MODE_UPDATE_CHANNEL = 2, // Channel selection changed
    MODE_UPDATE_INPUTSOURCE = 3 // Input routing mode changed
} mode_update_result_t;

typedef struct mode_signal_t {
    mode_waveform_t waveform;
    uint16_t amp_rshift;
    uint16_t freq_lut_idx;
} mode_signal_t;

typedef struct module_t {
    char name[EEPROM_MODULE_NAME_SIZE];
    char pcb_rev;
} module_t;

typedef struct channel_map_t {
    int  index;                             // The index of the map in the EEPROM
    char name[EEPROM_MAP_NAME_SIZE];        // Name of selected channel map in the EEPROM
    uint8_t channel_map[MAX_NUM_CHANNELS];  // The channel map. Index is the headstage channel, value is the test board channel
    uint8_t num_channels;                   // Number of headstage channels in the channel map
} channel_map_t;

typedef struct mode_context_t {
    mode_selection_t selection;             // The user selection
    mode_dest_t test_dest;                  // Where to route test signal
    channel_map_t channel_map;              // The channel map
    uint8_t channel_idx;                    // Selected headstage channel
    mode_signal_t signal;                   // The test signal
    module_t module;                        // Module metadata
    float battery_frac;                     // Fraction of battery remaining
    bool usb_detected;                      // Indicates if USB is plugged in
} mode_context_t;

void mode_init(mode_context_t *ctx);
void mode_cycle_selection(mode_context_t *ctx);
mode_update_result_t mode_update_from_knob(mode_context_t *ctx, int delta);
void mode_increment_channel(mode_context_t *ctx);
mode_selection_t mode_selection(const mode_context_t *ctx);

/// @brief Gets a string representing the value of a selection within a given mode context.
/// @param ctx Mode context
/// @param selection Mode selection for which a string description is needed.
/// @return string representation of the requested selection. Calling this function
/// again may mutate the previously returned string. Make sure to copy this or
/// apply it hardware prior to that.
const char *mode_str(const mode_context_t *ctx, mode_selection_t selection);

/// @brief Gets a title string for a given selection.
/// @param ctx Mode context
/// @param selection Mode selection for which a title string is needed.
/// @return title of the requested selection. Calling this function
/// again may mutate the previously returned string. Make sure to copy this or
/// apply it hardware prior to that.
const char *title_str(const mode_context_t *ctx, mode_selection_t selection);
