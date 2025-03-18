#pragma once

// TODO: After magic word, num_maps, and then we repeat the information for each
// headstage the board supports?

// Layout
// Bytes            Content
// -----------------------------------
// 0x00-0x09        "open-ephys"
// 0x0a-0x1d        name, using spaces to pad to 20 characters
// 0x1e             PCB Revision character, 'A', 'B', 'C', ...
// 0x1f             num_chan -- Number of channels
// 0x20-0xa0        Channel map, first num_chan elements are valid

#include "mode.h"

int eeprom_init();
int eeprom_read_module(mode_context_t *ctx);