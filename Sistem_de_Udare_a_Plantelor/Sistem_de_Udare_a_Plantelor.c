#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"

#include "lcd/lcd.h"
#include "sensor/sensor.h"
#include "pump/pump.h"

//limite critice
#define LIMIT_DRY 25
#define LIMIT_WET 85

#define PIN_BTN_KILL_SWITCH 16
#define PIN_BTN_UDARE_MANUALA 17

volatile bool kill_switch_activat = false;
volatile bool udare_manuala_activata = false;
volatile uint32_t ultima_apasare_buton = 0;

enum pompa {
    OPRIT,
    PORNIT,
    PAUZA
};

enum sistem {
    PREA_USCAT,
    NORMAL,
    PREA_UD
};


void btn_callback(uint gpio, uint32_t events){
    uint32_t timp_curent = to_ms_since_boot(get_absolute_time());

    //DEBOUNCING
    if(timp_curent - ultima_apasare_buton < 250) {
        return; 
    } 
    ultima_apasare_buton = timp_curent;

    //verifica daca butonul e chiar apasat
    if (gpio_get(gpio) != 0) {
        return; // semnal fals, ignoră
    }

    if(gpio == PIN_BTN_KILL_SWITCH){
        kill_switch_activat = !kill_switch_activat;
    }
    else if(gpio == PIN_BTN_UDARE_MANUALA){
        if(kill_switch_activat == false){
            udare_manuala_activata = true;
        }
    }
}



int main() {
    // initializare comunicare prin cablu
    stdio_init_all();

    lcd_init();
    sensor_init();
    pump_init();

    
    //initializare butoane
    gpio_init(PIN_BTN_KILL_SWITCH);
    gpio_set_dir(PIN_BTN_KILL_SWITCH, GPIO_IN);
    gpio_pull_up(PIN_BTN_KILL_SWITCH);

    gpio_init(PIN_BTN_UDARE_MANUALA);
    gpio_set_dir(PIN_BTN_UDARE_MANUALA, GPIO_IN);
    gpio_pull_up(PIN_BTN_UDARE_MANUALA);



    gpio_set_irq_enabled_with_callback(PIN_BTN_KILL_SWITCH, GPIO_IRQ_EDGE_FALL, true, &btn_callback);
    gpio_set_irq_enabled(PIN_BTN_UDARE_MANUALA, GPIO_IRQ_EDGE_FALL, true);

    
    char linia1[16];
    char linia2[16];
    bool stare_clipire = false;

    enum pompa stare_pompa = OPRIT;
    enum sistem stare_sistem = NORMAL; 
    int cronometru = 0;

    while(true)
    {   
        uint16_t media_bruta;
        int procent;

        //cerem datele de la senzor
        sensor_read(&media_bruta, &procent);

        stare_clipire = !stare_clipire;

        if(kill_switch_activat) {
            lcd_backlight(true);
            pump_turn_off();
            stare_pompa = OPRIT;
            cronometru = 0;
            udare_manuala_activata = false;

            sprintf(linia1, "!!!OPRIT!!!");
            sprintf(linia2, "KILL SWITCH ON");
        }
        else {
            switch (stare_sistem)
            {
                case PREA_USCAT:
                    if(procent >= LIMIT_DRY && procent <= LIMIT_WET && !udare_manuala_activata && stare_pompa == OPRIT)
                    {
                        stare_sistem = NORMAL;
                        break;
                    }

                    lcd_backlight(stare_clipire); 

                    if (stare_clipire) 
                    {
                        if(udare_manuala_activata) {
                            sprintf(linia1, "!!!MANUAL!!!");
                            sprintf(linia2, "UDARE ACTIVATA");
                        }
                        else {
                            sprintf(linia1, "!!!CRITIC!!!");
                            sprintf(linia2, "PREA USCAT: %d%%", procent);
                        }
                    } 
                    else 
                    {
                        sprintf(linia1, "                "); 
                        sprintf(linia2, "                ");
                    }

                    switch (stare_pompa)
                    {
                        case OPRIT:
                            pump_turn_on();
                            stare_pompa = PORNIT;
                            cronometru = 3;
                            break;
                        case PORNIT:
                            cronometru--;
                            if(cronometru <= 0)
                            {
                                pump_turn_off();
                                stare_pompa = PAUZA;
                                cronometru = 5;
                            }
                            break;
                        case PAUZA:
                            cronometru--;
                            if(cronometru <= 0)
                            {
                                stare_pompa = OPRIT;
                                udare_manuala_activata = false;
                            }
                            break;
                        default:
                            break;
                    }               
                    break;
                    
                case NORMAL:
                    lcd_backlight(true);
                    
                    if(procent <= LIMIT_DRY || udare_manuala_activata){
                        stare_sistem = PREA_USCAT;
                        break;
                    }
                    else if (procent >= LIMIT_WET){
                        stare_sistem = PREA_UD;
                        break;
                    }
                    
                    // daca umiditatea e in limitele normale, oprim pompa
                    pump_turn_off();
                    cronometru = 0;
                    stare_pompa = OPRIT;
                    
                    sprintf(linia1, "Umiditate: %d%%", procent);
                    sprintf(linia2, "Brut: %d", media_bruta);
                    break;

                case PREA_UD:
                    if(procent >= LIMIT_DRY && procent <= LIMIT_WET)
                    {
                        stare_sistem = NORMAL;
                        break;
                    }

                    lcd_backlight(stare_clipire); 
                    pump_turn_off();
                    stare_pompa = OPRIT;
                    cronometru = 0;

                    if (stare_clipire) 
                    {
                        sprintf(linia1, "!!!CRITIC!!!");
                        sprintf(linia2, "PREA UD: %d%%", procent);
                    } 
                    else 
                    {
                        sprintf(linia1, "                "); 
                        sprintf(linia2, "                ");
                    }
                    break;
                default:
                    break;
            }
        }

        //desenam pe display
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_string(linia1);
        lcd_set_cursor(1, 0);
        lcd_string(linia2);

        sleep_ms(1000);
    }
}