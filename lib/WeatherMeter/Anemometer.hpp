#pragma once

#include <Arduino.h>

struct _anenometer_reading {
    ulong start;
    uint16_t value;
};

class Anenometer {
    private:
        static void IRAM_ATTR triggerCallback(void* arg);
        void IRAM_ATTR trigger(void);

        static constexpr uint16_t _WINDSPEED_FACTOR = 2400;     // 2400m/h per trigger
        static constexpr uint8_t _DEBOUNCE = 1;                 // minimum 1 ms between interrupts
        static constexpr uint16_t _SAMPLES = 3;                 // smooth over 3 readings
        static constexpr uint8_t _DAMPENING = 4;                // older values get dampened by this factor
        static constexpr uint16_t _MININUM_SAMPLE_TIME = 10000; // only read every 10s

        uint8_t _pin;                                           // pin where the anenometer is connected
        bool _high_active;                                      // high active means react on rising flanks

        _anenometer_reading _readings[Anenometer::_SAMPLES];    // buffer for readings
        ulong _last_reading;
        uint8_t _index_reading;
        float _last_sample;
        ulong _last_sample_requested;

        float _warn_threshold;
        float _wind_warned;
        void (*_wind_warning)(float);

    public:
        Anenometer(uint8_t pin, bool high_active = false);
        ~Anenometer(void);
        bool begin(void);
        float getWindSpeed(void);

        void setWindWarning(void (*wind_warning)(float), float warn_threshold);
};
