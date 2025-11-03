#include "ephys-tester.h"
#include "eeprom.h"
#include "hardware/i2c.h"
#include <string.h>

#define EEPROM_ADDRESS 0b01010000

// Version 1.0 EEPROM layout address offsets and sizes
#define EEPROM_MAGIC_OFFSET 0x0000
#define EEPROM_MAGIC_SIZE 10
#define EEPROM_LAYOUT_VER_OFFSET 0x000A
#define EEPROM_NAME_OFFSET 0x000C
#define EEPROM_PCB_REV_OFFSET 0x002C
#define EEPROM_NUM_MAPS_OFFSET 0x002D

// Channel map layout (each map uses 0x400 byte blocks starting at 0x0400)
#define EEPROM_MAP_BASE_OFFSET 0x0400
#define EEPROM_MAP_SIZE 0x0400
#define EEPROM_MAP_NUM_CHAN_OFFSET 0x0000  // Relative to map base
#define EEPROM_MAP_NAME_OFFSET 0x0001      // Relative to map base
#define EEPROM_MAP_CHAN_MAP_OFFSET 0x0021  // Relative to map base

static int eeprom_read(i2c_inst_t *i2c, uint8_t addr, uint16_t reg, void *dst, uint16_t len)
{
    uint8_t buf[] = {reg >> 8, reg & 0xFF};
    i2c_write_blocking(i2c, addr, buf, 2, true);
    return i2c_read_blocking(i2c, addr, dst, len, false);
}

int eeprom_init()
{
    gpio_init(MODULE_DETECT);
    gpio_set_dir(MODULE_DETECT, GPIO_IN);

    gpio_init(MODULE_SDA);
    gpio_set_function(MODULE_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(MODULE_SDA);

    gpio_init(MODULE_SCL);
    gpio_set_function(MODULE_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(MODULE_SCL);

    i2c_init(i2c1, 100000);

    return 0;
}

int eeprom_get_map_name(int map_index, char *map_name)
{
    if (gpio_get(MODULE_DETECT)) {
        uint16_t map_base = EEPROM_MAP_BASE_OFFSET + (map_index * EEPROM_MAP_SIZE);
        int rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, map_base + EEPROM_MAP_NAME_OFFSET, map_name, EEPROM_MAP_NAME_SIZE);
        if (rc != EEPROM_MAP_NAME_SIZE) return -1;
    } else {
        return -1;
    }

    return 0;
}

int eeprom_read_module(mode_context_t *ctx, int map_index)
{
    if (gpio_get(MODULE_DETECT) || map_index == -1) {

        // Read and verify magic string
        char magic_buf[EEPROM_MAGIC_SIZE + 1];
        int rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, EEPROM_MAGIC_OFFSET, magic_buf, EEPROM_MAGIC_SIZE);
        magic_buf[EEPROM_MAGIC_SIZE] = '\0'; // Null terminate
        if (rc != EEPROM_MAGIC_SIZE || strcmp(magic_buf, "open-ephys")) goto default_config;

        // Read layout version
        uint8_t layout_ver[2];
        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, EEPROM_LAYOUT_VER_OFFSET, layout_ver, 2);
        if (rc != 2) goto default_config;

        // For now, we only support version 1.0
        if (layout_ver[0] != 1 || layout_ver[1] != 0) goto default_config;

        // Read module name
        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, EEPROM_NAME_OFFSET, ctx->module.name, EEPROM_MODULE_NAME_SIZE);
        if (rc != EEPROM_MODULE_NAME_SIZE) goto default_config;

        // Read PCB revision
        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, EEPROM_PCB_REV_OFFSET, &ctx->module.pcb_rev, 1);
        if (rc != 1) goto default_config;

        // Read number of maps
        uint8_t num_maps;
        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, EEPROM_NUM_MAPS_OFFSET, &num_maps, 1);
        if (rc != 1 || num_maps == 0) goto default_config;

        // Check if requested map index is valid
        if (map_index < 0 || map_index >= num_maps) goto default_config;

        // Calculate the base address for the requested map
        uint16_t map_base = EEPROM_MAP_BASE_OFFSET + (map_index * EEPROM_MAP_SIZE);

        // Read number of channels for the requested map
        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, map_base + EEPROM_MAP_NUM_CHAN_OFFSET, &ctx->channel_map.num_channels, 1);
        if (rc != 1 || ctx->channel_map.num_channels > MAX_NUM_CHANNELS) goto default_config;

        // Read channel map name
        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, map_base + EEPROM_MAP_NAME_OFFSET, ctx->channel_map.name, EEPROM_MAP_NAME_SIZE);
        if (rc != EEPROM_MAP_NAME_SIZE) goto default_config;

        // Read channel map
        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, map_base + EEPROM_MAP_CHAN_MAP_OFFSET, ctx->channel_map.channel_map, ctx->channel_map.num_channels);
        if (rc != ctx->channel_map.num_channels) goto default_config;

        return num_maps;

    } else {

default_config:
        eeprom_set_default(ctx);
        return - 1;
    }
}

int eeprom_set_default(mode_context_t *ctx)
{
    ctx->channel_map.index = -1;
    strncpy(ctx->module.name, "Default-128", sizeof(ctx->module.name) - 1);
    strncpy(ctx->channel_map.name, "Default 128 Ch.", sizeof(ctx->module.name) - 1);
    ctx->module.pcb_rev = ' ';
    ctx->channel_map.num_channels = MAX_NUM_CHANNELS;
    for (uint i = 0; i < MAX_NUM_CHANNELS; i++) {
        ctx->channel_map.channel_map[i] = i;
    }

    return 0;
}