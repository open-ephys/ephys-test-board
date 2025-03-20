#include "parse.h"
#include "sr.h"
#include "mode.h"

// TODO: cant this be made on the fly? Why do we need to have it hangingin around??
// static sr_bit_arr_t switches;

static int channels_to_bitarr(const uint8_t *channels, uint8_t num_channels, sr_bit_arr_t *bit_arr)
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

int channels_init(const mode_context_t *ctx)
{
    sr_bit_arr_t switches;
    sr_init();
    sr_clear(&switches);
    sr_update(&switches);
}

int channels_update(const mode_context_t *ctx)
{
    sr_bit_arr_t switches;

    switch (ctx->test_dest)
    {
        case TEST_OPEN:
            sr_clear(&switches);
            break;
        case TEST_CYCLE_CHANNEL:
        case TEST_SINGLE_CHANNEL:
            sr_clear(&switches);
            channels_to_bitarr(&ctx->channel_map[ctx->channel_idx], 1, &switches);
            break;
        case TEST_ALL_CHANNEL:
            sr_clear(&switches);
            channels_to_bitarr(ctx->channel_map, ctx->num_channels, &switches);
            break;
        default:
            return -1; // Invalid
    }

    sr_update(&switches);

    return 0;
}

