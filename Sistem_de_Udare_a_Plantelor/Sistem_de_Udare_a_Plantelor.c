#include <stdio.h>
#include "pico/stdlib.h"

#include "lcd.h"
#include "sensor.h"
#include "pump.h"

//limite critice
#define LIMIT_DRY 25
#define LIMIT_WET 85


int main() {
    // initializare comunicare prin cablu
    stdio_init_all();

    lcd_init();
    sensor_init();
    pump_init();

    char linia1[16];
    char linia2[16];
    bool stare_clipire = false;

    while(true)
    {   
        uint16_t media_bruta;
        int procent;

        //cerem datele de la senzor
        sensor_read(&media_bruta, &procent);

        stare_clipire = !stare_clipire;

        if(procent <= LIMIT_DRY)
        {
            lcd_backlight(stare_clipire); 
            pump_turn_on();

            if (stare_clipire) 
            {
                sprintf(linia1, "!!!CRITIC!!!");
                sprintf(linia2, "PREA USCAT: %d%%", procent);
            } 
            else 
            {
                sprintf(linia1, "                "); 
                sprintf(linia2, "                ");
            }
        }
        else if (procent >= LIMIT_WET)
        {
            lcd_backlight(stare_clipire); 
            pump_turn_off();

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
        } 
        else
        {
            lcd_backlight(true); 
            // daca umiditatea e in limitele normale, oprim pompa
            pump_turn_off();

            sprintf(linia1, "Umiditate: %d%%", procent);
            sprintf(linia2, "Brut: %d", media_bruta);
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