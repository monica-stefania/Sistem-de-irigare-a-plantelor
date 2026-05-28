#pragma once 
#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include <stdint.h>
#include <stdbool.h>

void lcd_init(i2c_inst_t *i2c);
void lcd_clear();
void lcd_set_cursor(int line, int position);
void lcd_string(const char *s);
void lcd_backlight(bool light);