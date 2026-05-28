#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"

#include "lcd/lcd.h"
#include "sensor/sensor.h"
#include "pump/pump.h"

//setare pini pt i2c
#define I2C_SDA_PIN 0 //GP0
#define I2C_SCL_PIN 1 //GP1

//setare pini pentru senzor umiditate
#define SOIL_SENSOR_PIN 26 //GP26
#define ADC_NUM 0 //ADC0 

//limite critice
#define LIMIT_DRY 25
#define LIMIT_WET 85

#define PIN_BTN_KILL_SWITCH 16
#define PIN_BTN_UDARE_MANUALA 17

volatile bool kill_switch_activat = false;
volatile bool udare_manuala_activata = false;
volatile uint32_t ultima_apasare_buton = 0;
int umidity_percentage = 0;

enum pompa {
    POMPA_OPRITA,
    POMPA_PORNITA,
    POMPA_PAUZA
};

enum sistem {
    STARE_NORMALA,
    STARE_PREA_USCAT,
    STARE_PREA_UD,
    STARE_UDARE_MANUALA,
    STARE_KILL_SWITCH
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
        return; 
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
  
    //initializare i2c
    i2c_init(i2c0, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    //initiliazare adc
    adc_init();
    adc_set_clkdiv(48000);

    lcd_init(i2c0);
    sensor_init(SOIL_SENSOR_PIN, ADC_NUM);
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

    sleep_ms(2000); // asteptam 2 secunde pentru a ne asigura ca totul e gat

    start_web_server("Pico_Irigare", "parola1234");
    
    char linia1[17] = "                ";
    char linia2[17] = "                ";
    bool stare_clipire = false;

    enum pompa stare_pompa = POMPA_OPRITA;
    enum sistem stare_sistem = STARE_NORMALA; 

    uint32_t timer_display = 0;
    uint32_t timer_pompa = 0;

    uint16_t media_bruta = 0;
    int procent = 0;

    while(true)
    {   
        uint32_t timp_curent = to_ms_since_boot(get_absolute_time());

        //actualizam display-ul, stare_clipire si citirile de la senzor o data pe secunda 
        if (timp_curent - timer_display >= 1000) {
            timer_display = timp_curent;
            stare_clipire = !stare_clipire; // Schimbăm starea pentru efectul de "Blink"
            sensor_read(&media_bruta, &procent); 
            umidity_percentage = procent;
        }

        //in starea de kill switch, functionalitatile pompei sunt oprite
        if(kill_switch_activat) {
            stare_sistem = STARE_KILL_SWITCH;
            udare_manuala_activata = false;
        }
        else {
            if(udare_manuala_activata) {
                stare_sistem = STARE_UDARE_MANUALA;
            }
            else {
                if(procent <= LIMIT_DRY) {
                    stare_sistem = STARE_PREA_USCAT;
                }
                else if(procent >= LIMIT_WET) {
                    stare_sistem = STARE_PREA_UD;
                }
                else {
                    stare_sistem = STARE_NORMALA;
                }               
            }
        }
        
        switch(stare_sistem){
            case STARE_KILL_SWITCH:
                pump_turn_off();
                stare_pompa = POMPA_OPRITA;
                lcd_backlight(true);
                sprintf(linia1, "!!!OPRIT!!!");
                sprintf(linia2, "KILL SWITCH ON");
                break;
            case STARE_NORMALA:
                pump_turn_off();
                stare_pompa = POMPA_OPRITA;
                lcd_backlight(true);
                sprintf(linia1, "Umiditate: %d%%", procent);
                sprintf(linia2, "Brut: %d", media_bruta);
                break;
            case STARE_PREA_UD:
                pump_turn_off();
                stare_pompa = POMPA_OPRITA;
                lcd_backlight(stare_clipire);
                if (stare_clipire) {
                    sprintf(linia1, "!!!CRITIC!!!");
                    sprintf(linia2, "PREA UD: %d%%", procent);
                } else {
                    sprintf(linia1, "                "); 
                    sprintf(linia2, "                ");
                }
                break;
            case STARE_PREA_USCAT:
            case STARE_UDARE_MANUALA:
                lcd_backlight(stare_clipire);
                
                if (stare_clipire) {
                    if (stare_sistem == STARE_UDARE_MANUALA) {
                        sprintf(linia1, "!!!MANUAL!!!");
                        sprintf(linia2, "UDARE ACTIVATA");
                    } else {
                        sprintf(linia1, "!!!CRITIC!!!");
                        sprintf(linia2, "PREA USCAT: %d%%", procent);
                    }
                } else {
                    sprintf(linia1, "                "); 
                    sprintf(linia2, "                ");
                }

                switch (stare_pompa){
                    case POMPA_OPRITA:
                        pump_turn_on();
                        stare_pompa = POMPA_PORNITA;
                        timer_pompa = timp_curent; // se porneste cronometrul pompei
                        break;
                    case POMPA_PORNITA:
                        if (timp_curent - timer_pompa >= 3000) { // daca pompa a functionat timp de 3 secunde
                             pump_turn_off();
                             stare_pompa = POMPA_PAUZA;
                             timer_pompa = timp_curent; // se porneste cronometrul de pauza
                             if(stare_sistem == STARE_UDARE_MANUALA) {
                                 udare_manuala_activata = false; 
                             }
                        }
                        break;
                    case POMPA_PAUZA:
                        if (timp_curent - timer_pompa >= 5000) { 
                             stare_pompa = POMPA_OPRITA;
                        }
                        break;
                }
                break;
        }

        //desenam pe display
        lcd_set_cursor(0, 0);
        lcd_string(linia1);
        lcd_set_cursor(1, 0);
        lcd_string(linia2);

    }
}