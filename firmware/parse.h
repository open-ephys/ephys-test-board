#pragma once

#include "ephys-tester.h"
#include "tiny-json.h"

int parse_channels(json_t const *root, sr_bit_arr_t *bit_arr, size_t bit_arr_len);
const char *parse_error(int code);