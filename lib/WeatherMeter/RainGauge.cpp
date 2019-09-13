#include "RainGauge.hpp"

extern "C"
{
	extern void __attachInterruptFunctionalArg(uint8_t pin, void (*userFunc)(void*), void * arg, int intr_type, bool functional);
}

RainGauge::RainGauge(uint8_t pin, bool high_active) :
    _pin(pin), _high_active(high_active),
    _last_reading(0), _index_reading(0) {
}

RainGauge::~RainGauge() {
}

void IRAM_ATTR _raingauge_triggered(void* arg)
{
	if (arg)
	{
	  ((RainGauge*)arg)->triggered();
	}
}

bool RainGauge::begin() {
    for (uint16_t i = 0; i<RainGauge::_SAMPLES; i++) {
        _readings[i].timestamp = 0;
        _readings[i].count = 0;
    }
    _readings[_index_reading].timestamp = millis();

    pinMode(_pin, _high_active ? INPUT_PULLDOWN : INPUT_PULLUP);
    __attachInterruptFunctionalArg (_pin, _raingauge_triggered, this, _high_active ? RISING : FALLING, true);
    return true;
}

void IRAM_ATTR RainGauge::triggered(void) {
    ulong now = millis();
    if (now - _last_reading < RainGauge::_DEBOUNCE)
        return;

    _last_reading = now;
    _readings[_index_reading].count++;
}

float RainGauge::getRainLevel(rain_timeframe_t timeframe) {
    ulong count = 0;
    ulong now = millis();
    ulong start = (timeframe > now) ? 1 : now - timeframe;
    ulong min = now;

    for (uint16_t i = 0; i<RainGauge::_SAMPLES; i++) {
        if (_readings[i].timestamp >= start) {
            count += _readings[i].count;
            if (_readings[i].timestamp < min)
                min = _readings[i].timestamp;
        }
    }

    if (now - _readings[_index_reading].timestamp > 1000) {
        _index_reading = (_index_reading + 1) % RainGauge::_SAMPLES;
        _readings[_index_reading].timestamp = now;
        _readings[_index_reading].count = 0;
    }

    float correction = (float)timeframe / (now - min);          // interpolation of missing values (if any)

    return (float)count * RainGauge::_RAIN_FACTOR * correction;
}