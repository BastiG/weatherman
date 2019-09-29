#pragma once

#include <Arduino.h>
#include <driver/adc.h>
#include "esp_adc_cal.h"

#define DEFAULT_VREF    3300
#define NO_OF_SAMPLES   16

typedef enum {
    WD_N    = (0),
    WD_NNE  = (1),
    WD_NE   = (2),
    WD_ENE  = (3),
    WD_E    = (4),
    WD_ESE  = (5),
    WD_SE   = (6),
    WD_SSE  = (7),
    WD_S    = (8),
    WD_SSW  = (9),
    WD_SW   = (10),
    WD_WSW  = (11),
    WD_W    = (12),
    WD_WNW  = (13),
    WD_NW   = (14),
    WD_NNW  = (15),
    WD_UNKNOWN = (255)
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
        static String toName(wind_direction_t wd);
        static float toDegrees(wind_direction_t wd);
};
