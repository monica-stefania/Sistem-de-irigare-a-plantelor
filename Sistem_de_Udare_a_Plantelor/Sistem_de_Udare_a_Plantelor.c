#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/dma.h"
#include "hardware/adc.h"

//setare pini pt i2c
#define I2C_SDA_PIN 0 //GP0
#define I2C_SCL_PIN 1 //GP1

//setare pini pentru senzor umiditate
#define SOIL_SENSOR_PIN 26 //GP26
#define ADC_NUM 0 //ADC0 

//calibrarea valorilor - cand planta e uda si cand e uscata
#define VALUE_DRY 2770 //0% - a fost testat cand senzorul era in aer
#define VALUE_WET 1100 //100% - a fost testat cand senzorul era in apa 

//limite critice
#define LIMIT_DRY 25
#define LIMIT_WET 85

#define NUM_SAMPLES 10
uint16_t adc_buffer[NUM_SAMPLES]; //aici pune DMA rezultatele

// Adresa I2C a ecranului 
const int LCD_ADDR = 0x27; 
const int LCD_ENABLE_BIT = 0x04;
const int LCD_BACKLIGHT = 0x08; //led-ul albastru din fundal
const int LCD_CLEAR_DISPLAY = 0x01;
const int LCD_ENTRY_MODE_SET = 0x04;
const int LCD_ENTRY_LEFT = 0x02;
const int LCD_FUNCTION_SET = 0x20;
const int LCD_2LINE = 0x08;
const int LCD_DISPLAY_CONTROL = 0x08;
const int LCD_DISPLAY_ON = 0x04;

bool lcd_on = true; //variabila globala pentru aprindere si stingere display-ului atunci cand valorile sunt critice

//functii pt display
void i2c_write_byte(uint8_t val) {
    i2c_write_blocking(i2c_default, LCD_ADDR, &val, 1, false);
}

void lcd_toggle_enable(uint8_t val) {
    sleep_us(600);
    i2c_write_byte(val | LCD_ENABLE_BIT); // Enable HIGH
    sleep_us(600);
    i2c_write_byte(val & ~LCD_ENABLE_BIT); // Enable LOW
    sleep_us(600);
}

void lcd_send_byte(uint8_t val, int mode) {
    //mode = 0 pentru comenzi
    //mode = 1 pentru date

    uint8_t light = lcd_on ? LCD_BACKLIGHT : 0x00;
    uint8_t high = mode | (val & 0xF0) | light; 
    uint8_t low  = mode | ((val << 4) & 0xF0) | light;

    i2c_write_byte(high);
    lcd_toggle_enable(high);
    i2c_write_byte(low);
    lcd_toggle_enable(low);
}

//sterge ecranul display-ului
void lcd_clear(void) {
    lcd_send_byte(LCD_CLEAR_DISPLAY, 0); // Comanda de stergere
    sleep_ms(2);
}

//muta cursorul pe ecran pentru a seta unde vrem sa scriem
void lcd_set_cursor(int line, int position) {
    // Linia 0 (sus) incepe la 0x80, Linia 1(jos) incepe la 0xC0
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(val, 0);
}

//trimite litera cu litera catre ecran
void lcd_string(const char *s) {
    while (*s) {
        lcd_send_byte(*s++, 1); // Trimitem literele (modul Date = 1)
    }
}

void lcd_init() {
    sleep_ms(50); // asteptam pornirea ecranului
    lcd_send_byte(0x03, 0);
    lcd_send_byte(0x03, 0);
    lcd_send_byte(0x03, 0);
    lcd_send_byte(0x02, 0); // setare mod 4-bit
    
    lcd_send_byte(LCD_ENTRY_MODE_SET | LCD_ENTRY_LEFT, 0);
    lcd_send_byte(LCD_FUNCTION_SET | LCD_2LINE, 0);
    lcd_send_byte(LCD_DISPLAY_CONTROL | LCD_DISPLAY_ON, 0);
    lcd_clear();
}

void lcd_backlight(bool light)
{
    lcd_on = light;
    if(lcd_on)
    {
        i2c_write_byte(LCD_BACKLIGHT);
    }
    else
    {
        i2c_write_byte(0x00);
    }
}

int humidity_percentage(uint16_t val)
{
    //daca senzorul da o valoare extrema o plafonam pentru a nu avea valori negative sau peste 100
    if (val > VALUE_DRY)
        val = VALUE_DRY;
    if (val < VALUE_WET)
        val = VALUE_WET;

    int precentage = 100 - ((val - VALUE_WET) * 100) / (VALUE_DRY - VALUE_WET);
    return precentage;
}

int main() {
    // initializare comunicare prin cablu
    stdio_init_all();

    // initializare protocol I2C pe pinii Pico
    i2c_init(i2c_default, 400 * 1000);

    //alocam pinii 0 si 1 pentru i2c
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);

    //activam rezistentele interne de pull-up
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // pornire ecran
    lcd_init();

    //initiliazare adc
    adc_init();
    adc_gpio_init(SOIL_SENSOR_PIN);
    adc_select_input(ADC_NUM);
    adc_set_clkdiv(48000);

    //initializare dma

    //luam un canal DMA liber
    uint dma_chan = dma_claim_unused_channel(true);

    //configuarare setari default
    dma_channel_config cfg = dma_channel_get_default_config(dma_chan);

    //configurarea canalului
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16); //datele vor avea dimesiunea de 16 biti
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_dreq(&cfg, DREQ_ADC);

    adc_fifo_setup(true, true, 1, false, false);

    char linia1[16];
    char linia2[16];

    bool stare_clipire = false;

    while(true)
    {
        dma_channel_configure(dma_chan, &cfg, adc_buffer, &adc_hw->fifo, NUM_SAMPLES, true);
        adc_run(true); 
        dma_channel_wait_for_finish_blocking(dma_chan);
        adc_run(false); 
        adc_fifo_drain();

        //calculam media
        uint32_t suma = 0;
        for(int i=0; i<NUM_SAMPLES; i++) {
            suma += adc_buffer[i];
        }
        uint16_t media_bruta = suma / NUM_SAMPLES;

        //calcul procent
        int procent = humidity_percentage(media_bruta);

        stare_clipire = !stare_clipire;

        if(procent <= LIMIT_DRY)
        {
            lcd_backlight(stare_clipire); 

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

            sprintf(linia1, "Umiditate: %d%%", procent);
            sprintf(linia2, "Brut: %d", media_bruta);
        }
        lcd_clear();
        lcd_set_cursor(0, 0);
        lcd_string(linia1);
        lcd_set_cursor(1, 0);
        lcd_string(linia2);

        sleep_ms(1000);
    }
}