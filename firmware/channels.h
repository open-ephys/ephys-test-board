#pragma once

#include "ephys-tester.h"

int channels_init();
int channels_update(mode_context_t *ctx);
int channels_update_manual(const uint8_t *channels, size_t num_channels);
