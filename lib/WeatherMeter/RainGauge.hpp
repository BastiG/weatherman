#ifndef _RAINGAUGE_HPP_
#define _RAINGAUGE_HPP_

#include <Arduino.h>

enum rain_timeframe_t {
    RAIN_1M = 60000,
    RAIN_15M = 900000,
    RAIN_1H = 3600000
};

struct rain_reading_t {
    ulong timestamp;
    uint16_t count;
};

class RainGauge {
    private:
        static constexpr float_t _RAIN_FACTOR = 0.2794f;        // bucket tilt means 0.2794mm of rain
        static constexpr uint8_t _DEBOUNCE = 1;                 // minimum 1 ms between interrupts
        static constexpr uint16_t _SAMPLES = 4000;              // number of samples

        uint8_t _pin;
        bool _high_active;

        rain_reading_t _readings[RainGauge::_SAMPLES];
        ulong _last_reading;
        uint16_t _index_reading;
    public:
        RainGauge(uint8_t pin, bool high_active = false);
        ~RainGauge(void);
        bool begin(void);
        void IRAM_ATTR triggered(void);

        float getRainLevel(rain_timeframe_t timeframe);
};

#endif