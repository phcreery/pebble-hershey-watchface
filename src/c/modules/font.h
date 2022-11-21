#pragma once

#include <pebble.h>

float font_draw_char(unsigned char c, int x, int y, float scale, GContext *ctx);
void font_draw_string(char *string, int x, int y, float scale, GContext *ctx);
