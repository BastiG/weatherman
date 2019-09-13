#include "WindVane.hpp"

WindVane::WindVane(uint8_t pin) {
    _pin = pin;
}

WindVane::~WindVane(void) {
    free(_adc_chars);
}

bool WindVane::begin(void) {
    switch (_pin) {
        case 32: _windVane_pin = ADC1_GPIO32_CHANNEL; break;
        case 33: _windVane_pin = ADC1_GPIO33_CHANNEL; break;
        case 34: _windVane_pin = ADC1_GPIO34_CHANNEL; break;
        case 35: _windVane_pin = ADC1_GPIO35_CHANNEL; break;
        case 36: _windVane_pin = ADC1_GPIO36_CHANNEL; break;
        case 37: _windVane_pin = ADC1_GPIO37_CHANNEL; break;
        case 38: _windVane_pin = ADC1_GPIO38_CHANNEL; break;
        case 39: _windVane_pin = ADC1_GPIO39_CHANNEL; break;
        default: return false;
    }
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(_windVane_pin, ADC_ATTEN_11db);

    _adc_chars = (esp_adc_cal_characteristics_t *)calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_11db, ADC_WIDTH_12Bit, DEFAULT_VREF, _adc_chars);

#ifdef DEBUG
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        Serial.println("[ADC-cal] eFuse Vref");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        Serial.println("[ADC-cal] Two Point");
    } else {
        Serial.println("[ADC-cal] Default");
    }
#endif

    return true;
}

// Definition of readings in mV

#define __W_READ    263
#define __NW_READ   447
#define __WNW_READ  637
#define __N_READ    766

#define __NNW_READ  1038
#define __SW_READ   1272
#define __WSW_READ  1370
#define __NE_READ   1810

#define __NNE_READ  1995
#define __S_READ    2378
#define __SSW_READ  2530
#define __SE_READ   2715

#define __SSE_READ  2900
#define __E_READ    3000
#define __ENE_READ  3035
#define __ESE_READ  3095


// Definition of reading (upper) boundaries

#define __W_MAX     __W_READ + (__NW_READ - __W_READ) / 2
#define __NW_MAX    __NW_READ + (__WNW_READ - __NW_READ) / 2
#define __WNW_MAX   __WNW_READ + (__N_READ - __WNW_READ) / 2
#define __N_MAX     __N_READ + (__NNW_READ - __N_READ) / 2

#define __NNW_MAX   __NNW_READ + (__SW_READ - __NNW_READ) / 2
#define __SW_MAX    __SW_READ + (__WSW_READ - __SW_READ) / 2
#define __WSW_MAX   __WSW_READ + (__NE_READ - __WSW_READ) / 2
#define __NE_MAX    __NE_READ + (__NNE_READ - __NE_READ) / 2

#define __NNE_MAX   __NNE_READ + (__S_READ - __NNE_READ) / 2
#define __S_MAX     __S_READ + (__SSW_READ - __S_READ) / 2
#define __SSW_MAX   __SSW_READ + (__SE_READ - __SSW_READ) / 2

#define __SE_MAX    __SE_READ + (__SSE_READ - __SE_READ) / 2
#define __SSE_MAX   __SSE_READ + (__E_READ - __SSE_READ) / 2
#define __E_MAX     __E_READ + (__ENE_READ - __E_READ) / 2
#define __ENE_MAX   __ENE_READ + (__ESE_READ - __ENE_READ) / 2


// Defintion of total reading boundaries

#define __READ_MIN  180
#define __READ_MAX  DEFAULT_VREF-__READ_MIN

wind_direction_t WindVane::getWindDirection(void) {
    uint32_t adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw(_windVane_pin);
    }
    uint32_t value = esp_adc_cal_raw_to_voltage(adc_reading / NO_OF_SAMPLES, _adc_chars);

    //Serial.print("Reading: "); Serial.println(value);

    if (value < __READ_MIN || value > __READ_MAX) {
        //Serial.print("ERROR: Wind reading is out of range: "); Serial.println(value);
        return WD_UNKNOWN;
    }

    if (value < __W_MAX)    return WD_W;
    if (value < __NW_MAX)   return WD_NW;
    if (value < __WNW_MAX)  return WD_WNW;
    if (value < __N_MAX)    return WD_N;
    if (value < __NNW_MAX)  return WD_NNW;
    if (value < __SW_MAX)   return WD_SW;
    if (value < __WSW_MAX)  return WD_WSW;
    if (value < __NE_MAX)   return WD_NE;
    if (value < __NNE_MAX)  return WD_NNE;
    if (value < __S_MAX)    return WD_S;
    if (value < __SSW_MAX)  return WD_SSW;
    if (value < __SE_MAX)   return WD_SE;
    if (value < __SSE_MAX)  return WD_SSE;
    if (value < __E_MAX)    return WD_E;
    if (value < __ENE_MAX)  return WD_ENE;
    return WD_ESE;
}


String WindVane::translateWindDirection(wind_direction_t wd) {
    switch (wd) {
        case WD_N:      return "N";
        case WD_NNE:    return "NNE";
        case WD_NE:     return "NE";
        case WD_ENE:    return "ENE";
        case WD_E:      return "E";
        case WD_ESE:    return "ESE";
        case WD_SE:     return "SE";
        case WD_SSE:    return "SSE";
        case WD_S:      return "S";
        case WD_SSW:    return "SSW";
        case WD_SW:     return "SW";
        case WD_WSW:    return "WSW";
        case WD_W:      return "W";
        case WD_WNW:    return "WNW";
        case WD_NW:     return "NW";
        case WD_NNW:    return "NNW";
        case WD_UNKNOWN:
        default:        return "?";
    }
}