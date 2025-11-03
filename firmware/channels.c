#include "parse.h"
#include "sr.h"
#include "mode.h"

static int channels_to_bitarr(const uint8_t *channels, uint8_t num_channels, sr_bit_arr_t *bit_arr)
{
    // Clear all bits
    sr_clear(bit_arr);

    for (int i = 0; i < num_channels; i++)
    {
        if (sr_set_bit(bit_arr, *(channels + i)))
        {
            return -1; // TODO: Channel out of range
        }
    }

    return 0;
}

int channels_init()
{
    sr_bit_arr_t switches;
    sr_init();
    sr_clear(&switches);
    sr_update(&switches);
}

int channels_update(const mode_context_t *ctx)
{
    sr_bit_arr_t switches;

    int rc = 0;

    switch (ctx->test_dest)
    {
        case DEST_NONE:
            sr_clear(&switches);
            break;
        case DEST_CYCLE_CHANNEL_SLOW:
        case DEST_CYCLE_CHANNEL_FAST:
        case DEST_SINGLE_CHANNEL:
            sr_clear(&switches);
            rc = channels_to_bitarr(&ctx->channel_map.channel_map[ctx->channel_idx], 1, &switches);
            break;
        case DEST_ALL_CHANNEL:
            sr_clear(&switches);
            rc = channels_to_bitarr(ctx->channel_map.channel_map, ctx->channel_map.num_channels, &switches);
            break;
        default:
            return -1; // TODO: Invalid test signal selection
    }

    return rc != 0 ? rc : sr_update(&switches);
}

int channels_update_manual(const uint8_t *channels, size_t num_channels)
{
    sr_bit_arr_t switches;
    int rc =  channels_to_bitarr(channels, num_channels, &switches);
    return rc != 0 ? rc : sr_update(&switches);
}