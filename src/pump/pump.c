#include "pump.h"

#define RELAY_PIN 15 

void pump_turn_on() {
    //pompa porneste cand trimitem curent catre releu
    gpio_set_dir(RELAY_PIN, GPIO_OUT);
    gpio_put(RELAY_PIN, 0);
}

void pump_turn_off() {
    //pompa se opreste cand nu mai trimitem curent catre releu
    gpio_set_dir(RELAY_PIN, GPIO_OUT); 
    gpio_put(RELAY_PIN, 1);
}

void pump_init() {
    // pinul 15 este folosit pentru a trimite curent (OUTPUT)
    gpio_init(RELAY_PIN);
    
    //initial pompa e oprita
    pump_turn_off(); 
}