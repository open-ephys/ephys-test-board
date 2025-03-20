#pragma once

#include "hardware/pio.h"
#include "quadrature.pio.h"

int quad_init();
int quad_get_count();
bool quad_pending_turn();
void quad_acknowledge_turn();
int quad_get_count();
int quad_get_delta();
