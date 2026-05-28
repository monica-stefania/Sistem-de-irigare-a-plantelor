//fisier cu functii pt display

#include "lcd.h"


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
static i2c_inst_t *lcd_i2c;

void i2c_write_byte(uint8_t val) {
    i2c_write_blocking(lcd_i2c, LCD_ADDR, &val, 1, false);
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

void lcd_init(i2c_inst_t *i2c) {
    lcd_i2c = i2c;

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