#ifndef _WINDVANE_HPP_
#define _WINDVANE_HPP_

#include <Arduino.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"

#define DEFAULT_VREF    3300
#define NO_OF_SAMPLES   16

typedef enum {
    WD_UNKNOWN,
    WD_N,
    WD_NNE,
    WD_NE,
    WD_ENE,
    WD_E,
    WD_ESE,
    WD_SE,
    WD_SSE,
    WD_S,
    WD_SSW,
    WD_SW,
    WD_WSW,
    WD_W,
    WD_WNW,
    WD_NW,
    WD_NNW
} wind_direction_t ;

class WindVane {
    private:
        uint8_t _pin;
        adc1_channel_t _windVane_pin = ADC1_GPIO39_CHANNEL;
        esp_adc_cal_characteristics_t *_adc_chars;
    public:
        WindVane(uint8_t pin);
        ~WindVane(void);
        bool begin(void);
        wind_direction_t getWindDirection(void);
    
    public:
        static String translateWindDirection(wind_direction_t wd);
};

#endif