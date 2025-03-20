#pragma once

#include "ephys-tester.h"
extern "C"
{
#include "mode.h"
}

int oled_init();
int oled_update(const mode_context_t *const ctx);