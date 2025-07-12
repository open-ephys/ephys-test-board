#pragma once

#include "hardware/pio.h"
#include "quadrature.pio.h"
#include "mode.h"

int batt_mon_init();
bool batt_mon_monitor(mode_context_t *ctx);
