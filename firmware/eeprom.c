#include "ephys-tester.h"
#include "eeprom.h"
#include "hardware/i2c.h"
#include <string.h>

#define EEPROM_ADDRESS 0b01010000
#define EEPROM_SN_ADDRESS 0b01011000

#define EEPROM_MAGIC_OFFSET 0
#define EEPROM_NAME_OFFSET 10
#define EEPROM_PCB_REV_OFFSET 30
#define EEPROM_NUM_CHAN_OFFSET 31


// TODO: 16-bit EEPROM
// static int eeprom_read(i2c_inst_t *i2c, uint8_t addr, uint16_t reg, void *dst, uint16_t len) {
//     uint8_t buf[] = {reg >> 8, reg & 0xFF};
//     i2c_write_blocking(i2c, addr, buf, 2, true);
//     return i2c_read_blocking(i2c, addr, dst, len, false);
// }

static int eeprom_read(i2c_inst_t *i2c, uint8_t addr, uint8_t reg, void *dst, uint16_t len) {
    i2c_write_blocking(i2c, addr, &reg, 1, true);
    return i2c_read_blocking(i2c, addr, dst, len, false);
}

int eeprom_init()
{
    gpio_init(MODULE_nDETECT);
    gpio_set_dir(MODULE_nDETECT, GPIO_IN);
    gpio_pull_down(MODULE_nDETECT); // TODO: Remove for rev C where this is actually implemented

    gpio_init(MODULE_SDA);
    gpio_set_function(MODULE_SDA, GPIO_FUNC_I2C);
    gpio_pull_up(MODULE_SDA);

    gpio_init(MODULE_SCL);
    gpio_set_function(MODULE_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(MODULE_SCL);

    i2c_init(i2c1, 100000);
}

int eeprom_read_module(mode_context_t *ctx)
{
    if (!gpio_get(MODULE_nDETECT)) {

        int rc = eeprom_read(MODULE_I2C, EEPROM_SN_ADDRESS, 0x80, &ctx->module.sn, 16);
        if (rc != 16) goto default_config;

        char buf[EEPROM_NAME_OFFSET + 1];
        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, EEPROM_MAGIC_OFFSET, buf, EEPROM_NAME_OFFSET);
        buf[EEPROM_NAME_OFFSET] = '\0'; // Null terminate
        int lala = strcmp(buf, "open-ephys");
        if (rc != EEPROM_NAME_OFFSET || lala) goto default_config;

        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, EEPROM_NAME_OFFSET, &ctx->module.name, MODULE_NAME_BYTES);
        if (rc != MODULE_NAME_BYTES) goto default_config;
        ctx->module.name[MODULE_NAME_BYTES] = '\0'; // Null terminate

        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, EEPROM_PCB_REV_OFFSET, &ctx->module.pcb_rev, 1);
        if (rc != 1) goto default_config;

        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, EEPROM_NUM_CHAN_OFFSET, &ctx->num_channels, 1);
        if (rc != 1 || ctx->num_channels > MAX_NUM_CHANNELS) goto default_config;

        rc = eeprom_read(MODULE_I2C, EEPROM_ADDRESS, EEPROM_NUM_CHAN_OFFSET + 1, ctx->channel_map, ctx->num_channels);
        if (rc != ctx->num_channels) goto default_config;

    } else {

default_config:
        strncpy(ctx->module.name, "Default-128         ", MODULE_NAME_BYTES);
        ctx->module.pcb_rev = ' ';
        ctx->num_channels = MAX_NUM_CHANNELS;
        for (uint i = 0; i < MAX_NUM_CHANNELS; i++) { ctx->channel_map[i] = i; }
    }
}