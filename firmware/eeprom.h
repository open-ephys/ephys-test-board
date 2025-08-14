#pragma once

// EEPROM Layout Version 1.0
//
// ┌──────────────┬─────────────────────┬─────────────────────────────────────────────────────────┐
// │      ID      │   Address Range     │                      Description                        │
// ├──────────────┼─────────────────────┼─────────────────────────────────────────────────────────┤
// │ MAGIC        │ 0x0000 - 0x0009     │ The ASCII character string "open-ephys"                 │
// │ LAYOUT_VER   │ 0x000A - 0x000B     │ Major.Minor version (e.g. 1.0)                          │
// │ NAME         │ 0x000C - 0x002B     │ Null-terminated module name (max 32 bytes)              │
// │ PCB_REV      │ 0x002C              │ Single ASCII char for PCB revision (e.g. 'A')           │
// │ NUM_MAPS     │ 0x002D              │ Number of channel maps (uint8_t)                        │
// │ RESERVED     │ 0x002E - 0x03FF     │ Reserved space                                          │
// ├──────────────┼─────────────────────┼─────────────────────────────────────────────────────────┤
// │ NUM_CHAN_0   │ 0x0400              │ Number of channels in map 0 (uint8_t)                   │
// │ MAP_NAME_0   │ 0x0401 - 0x0420     │ Null-terminated name for map 0 (max 32 bytes)           │
// │ CHAN_MAP_0   │ 0x0421 - 0x0421+N   │ Channel map 0 data (uint8_t[NUM_CHAN_0])                │
// ├──────────────┼─────────────────────┼─────────────────────────────────────────────────────────┤
// │ NUM_CHAN_1   │ 0x0800              │ Number of channels in map 1 (uint8_t)                   │
// │ MAP_NAME_1   │ 0x0801 - 0x0820     │ Null-terminated name for map 1 (max 32 bytes)           │
// │ CHAN_MAP_1   │ 0x0821 - 0x0821+N   │ Channel map 1 data (uint8_t[NUM_CHAN_1])                │
// ├──────────────┼─────────────────────┼─────────────────────────────────────────────────────────┤
// │     ...      │        ...          │ Additional maps at 0x0400 byte intervals...             │
// ├──────────────┼─────────────────────┼─────────────────────────────────────────────────────────┤
// │ NUM_CHAN_N   │ 0x0400+(N*0x0400)   │ Number of channels in map N (uint8_t)                   │
// │ MAP_NAME_N   │ 0x0401+(N*0x0400)   │ Null-terminated name for map N (max 32 bytes)           │
// │              │  - 0x0420+(N*0x0400)│                                                         │
// │ CHAN_MAP_N   │ 0x0421+(N*0x0400)   │ Channel map N data (uint8_t[NUM_CHAN_N])                │
// │              │  - 0x0421+(N*0x0400)│                                                         │
// │              │     + NUM_CHAN_N    │                                                         │
// ───────────────┴─────────────────────┴─────────────────────────────────────────────────────────┘

#include "mode.h"

int eeprom_init();
int eeprom_get_map_name(int map_index, char *map_name);
int eeprom_read_module(mode_context_t *ctx, int map_index);
int eeprom_set_default(mode_context_t *ctx);