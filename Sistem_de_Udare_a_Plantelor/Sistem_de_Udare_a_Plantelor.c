#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

//setare pini pt i2c
#define I2C_PORT i2c0
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1

// Adresa I2C a ecranului 
const int LCD_ADDR = 0x27; 

//functii pt display
void i2c_write_byte(uint8_t val) {
    i2c_write_blocking(I2C_PORT, LCD_ADDR, &val, 1, false);
}

void lcd_toggle_enable(uint8_t val) {
    sleep_us(600);
    i2c_write_byte(val | 0x04); // Enable HIGH
    sleep_us(600);
    i2c_write_byte(val & ~0x04); // Enable LOW
    sleep_us(600);
}

void lcd_send_byte(uint8_t val, int mode) {
    uint8_t high = mode | (val & 0xF0) | 0x08; 
    uint8_t low  = mode | ((val << 4) & 0xF0) | 0x08;
    i2c_write_byte(high);
    lcd_toggle_enable(high);
    i2c_write_byte(low);
    lcd_toggle_enable(low);
}

void lcd_clear(void) {
    lcd_send_byte(0x01, 0); // Comanda de stergere
    sleep_ms(2);
}

void lcd_set_cursor(int line, int position) {
    // Linia 0 incepe la 0x80, Linia 1 incepe la 0xC0
    int val = (line == 0) ? 0x80 + position : 0xC0 + position;
    lcd_send_byte(val, 0);
}

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
    
    lcd_send_byte(0x28, 0); 
    lcd_send_byte(0x0C, 0); 
    lcd_clear();
}


int main() {
    // initializare comunicare prin cablu
    stdio_init_all();

    // initializare protocol I2C pe pinii Pico
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);

    // pornire ecran
    lcd_init();

    // scriem ceva pe ecran
    lcd_set_cursor(0, 2); 
    lcd_string("Hello World!");

    lcd_set_cursor(1, 1); 
    lcd_string("Test");

    // mentinerea programului activ
    while (true) {
        sleep_ms(1000);
    }
}