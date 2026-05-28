#pragma once
#include "pico/cyw43_arch.h"

extern int umidity_percentage; //variabila globala pentru a stoca procentul de umiditate
extern int pompa_activata;

void start_web_server();
