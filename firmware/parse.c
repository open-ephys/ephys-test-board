#include "parse.h"
#include "sr.h"

int parse_channels(json_t const *root, sr_bit_arr_t *bit_arr, size_t bit_arr_len)
{
    json_t const *ch_prop = json_getProperty(root, "channels");
    if (ch_prop == NULL) {
        return -1;
    }

    if (json_getType(ch_prop) != JSON_ARRAY) {
        return -2;
    }

    // Clear all bits
    for (int i = 0; i < bit_arr_len; i++)
        bit_arr[i] = 0;

    for (json_t const *i = json_getChild(ch_prop); i; i = json_getSibling(i)) {

        // Check type
        if (json_getType(i) != JSON_INTEGER) {
            return -2;
        }

        // Get the bit index
        int b = json_getInteger(i);

        if (sr_set_bit(bit_arr, bit_arr_len, b)){
            return -3;
        }
    }

    return 0;
}

const char *parse_error(int code)
{
    switch (code)
    {
        case -1:
            return "Requested property does not exist in JSON structure";
        case -2:
            return "Requested property does not match expected type";
        case -3:
            return "Invalid channel number";
        default:
            return "Unknown error";
    }
}