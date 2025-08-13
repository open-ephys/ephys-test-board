#pragma once

extern "C"
{
#include "mode.h"
}

int oled_init();
int oled_update_map_menu(const mode_context_t *const ctx);
int oled_update_main_menu(const mode_context_t *const ctx);