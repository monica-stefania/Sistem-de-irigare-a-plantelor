#pragma once
#include "pico/stdlib.h"
#include "hardware/adc.h"

#include <stdint.h>

void sensor_init(uint8_t gpio_pin, uint8_t adc_channel);
void sensor_read(uint16_t *media_bruta_out, int *procent_out);