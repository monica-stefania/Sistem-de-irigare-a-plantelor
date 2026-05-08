#include "sensor.h"
#include "hardware/adc.h"
#include "hardware/dma.h"

//setare pini pentru senzor umiditate
#define SOIL_SENSOR_PIN 26 //GP26
#define ADC_NUM 0 //ADC0 

//calibrarea valorilor - cand planta e uda si cand e uscata
#define VALUE_DRY 2770 //0% - a fost testat cand senzorul era in aer
#define VALUE_WET 1100 //100% - a fost testat cand senzorul era in apa 

#define NUM_SAMPLES 10

uint16_t adc_buffer[NUM_SAMPLES]; //aici pune DMA rezultatele
int dma_chan; //canalul DMA folosit
dma_channel_config cfg; 

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

void sensor_init()
{
    //initiliazare adc
    adc_init();
    adc_gpio_init(SOIL_SENSOR_PIN);
    adc_select_input(ADC_NUM);
    adc_set_clkdiv(48000);

    //luam un canal DMA liber
    dma_chan = dma_claim_unused_channel(true);
    //configuarare setari default
    cfg = dma_channel_get_default_config(dma_chan);

    //configurarea canalului
    channel_config_set_transfer_data_size(&cfg, DMA_SIZE_16); //datele vor avea dimesiunea de 16 biti
    channel_config_set_read_increment(&cfg, false);
    channel_config_set_write_increment(&cfg, true);
    channel_config_set_dreq(&cfg, DREQ_ADC);

    adc_fifo_setup(true, true, 1, false, false);
}

void sensor_read(uint16_t *media_bruta_out, int *procent_out)
{
    //pornim transferul DMA
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

    *media_bruta_out = suma / NUM_SAMPLES;
    *procent_out = humidity_percentage(*media_bruta_out);
}