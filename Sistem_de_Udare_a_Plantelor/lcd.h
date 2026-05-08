#pragma once 
#include "pico/stdlib.h"

#include <stdint.h>
#include <stdbool.h>

void lcd_init();
void lcd_clear();
void lcd_set_cursor(int line, int position);
void lcd_string(const char *s);
void lcd_backlight(bool light);