#pragma once

#include "basic.h"
#include "config.h"

#define MENU_WIDTH 40
#define MAX_ENTRIES_PER_SCREEN 18
#define MENU_HEIGHT (MAX_ENTRIES_PER_SCREEN+2) //for the delimiters

void drawInstructions(void);
void drawMenu(config_t * config, u8 * state, u8 selected_entry);
