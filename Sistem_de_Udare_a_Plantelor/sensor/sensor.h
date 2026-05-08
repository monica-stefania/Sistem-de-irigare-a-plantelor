#pragma once
#include "pico/stdlib.h"

#include <stdint.h>

void sensor_init();
void sensor_read(uint16_t *media_bruta_out, int *procent_out);