#include "parse.h"
#include "sr.h"
#include "mode.h"

// TODO: cant this be made on the fly? Why do we need to have it hangingin around??
// static sr_bit_arr_t switches;

static int channels_to_bitarr(uint *channels, size_t num_channels, sr_bit_arr_t *bit_arr)
{
    // Clear all bits
    sr_clear(bit_arr);

    // TODO: Mapped channels
    for (int i = 0; i < num_channels; i++)
    {
        if (sr_set_bit(bit_arr, *(channels + i)))
        {
            return -1; // TODO: Channel out of range
        }
    }

    return 0;
}

int channels_init(mode_context_t *ctx)
{
    // TODO: Load map from EEPROM
    // TODO: detected when module is attached and removed to reload
    // // I2C Initialisation. Using it at 400Khz.
    // i2c_init(I2C_PORT, 400*1000);
    // gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    // gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);

    // TODO: Place holder for above
    ctx->num_channels = MAX_NUM_CHANNELS;
    for (uint i = 0; i < MAX_NUM_CHANNELS; i++) { ctx->channel_map[i] = i; }

    sr_bit_arr_t switches;
    sr_init();
    sr_clear(&switches);
    sr_update(&switches);
}

int channels_update(mode_context_t *ctx)
{
    sr_bit_arr_t switches;

    switch (ctx->test_dest)
    {

        case TEST_OPEN:
            sr_clear(&switches);
            break;
        case TEST_SINGLE_CHANNEL:
            sr_clear(&switches);
            channels_to_bitarr(&ctx->channel_idx, 1, &switches);
            break;
        case TEST_ALL_CHANNEL:
            sr_clear(&switches);
            channels_to_bitarr(ctx->channel_map, ctx->num_channels, &switches);
            break;
        case TEST_CYCLE_CHANNEL:
            // TODO: set up callback
            return 0; // Callback handles updates
        default:
            return -1; // Invalid
    }

    sr_update(&switches);

    return 0;
}

