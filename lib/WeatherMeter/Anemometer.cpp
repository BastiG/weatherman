#include "Anemometer.hpp"

#include <math.h>
#include <functional>

extern "C"
{
	extern void __attachInterruptFunctionalArg(uint8_t pin, void (*userFunc)(void*), void * arg, int intr_type, bool functional);
}

Anenometer::Anenometer(uint8_t pin, bool high_active) :
        _pin(pin), _high_active(high_active),
        _last_reading(0), _index_reading(0),
        _last_sample(0), _last_sample_requested(0),
        _warn_threshold(NAN), _wind_warned(0), _wind_warning(nullptr) {
}

Anenometer::~Anenometer(void) {
}

bool Anenometer::begin(void) {
    using namespace std::placeholders;

    for (int i=0; i<Anenometer::_SAMPLES; i++) {
        _readings[i].start = 0;
        _readings[i].value = 0;
    }
    _readings[_index_reading].start = millis();
    pinMode(_pin, _high_active ? INPUT_PULLDOWN : INPUT_PULLUP);
    __attachInterruptFunctionalArg(_pin, Anenometer::triggerCallback, this, _high_active ? RISING : FALLING, true);
    return true;
}

void IRAM_ATTR Anenometer::triggerCallback(void* arg) {
    reinterpret_cast<Anenometer*>(arg)->trigger();
}

void IRAM_ATTR Anenometer::trigger(void) {
    ulong now = millis();
    if (now - _last_reading < Anenometer::_DEBOUNCE)
        return;

    _last_reading = now;
    _readings[_index_reading].value++;
}

float Anenometer::getWindSpeed(void) {
    ulong now = millis();
    ulong duration = now - _last_sample_requested;
    if (duration < Anenometer::_MININUM_SAMPLE_TIME)
        return _last_sample;

#ifdef _DEBUG_ANENOMETER
    Serial.print("getWindSpeed() over last "); Serial.print(duration); Serial.print("ms, index="); Serial.print(_index_reading);
#endif

    _last_sample_requested = now;

    float windspeed = 0;
    uint8_t index = _index_reading + Anenometer::_SAMPLES;
    uint8_t values = 0;

    _anenometer_reading* reading;
    ulong stop = now;

    bool warning_active = false;

    for (int i=0; i<Anenometer::_SAMPLES; i++) {
#ifdef _DEBUG_ANENOMETER
        Serial.print(", value["); Serial.print((index - i) % Anenometer::_SAMPLES); Serial.print(" @ ");
#endif
        reading = &_readings[(index - i) % Anenometer::_SAMPLES];

        if (reading->start > 0) {
            // Gets triggers per second
            float value = (float)(reading->value * Anenometer::_WINDSPEED_FACTOR)/(stop - reading->start);

            if (!isnan(_warn_threshold)) {
                if (value > _warn_threshold) {
                    if (_wind_warning && (_wind_warned == 0 || abs(value - _wind_warned) > 10) && !warning_active) {
                        _wind_warned = value;
                        _wind_warning(value);
                    }
                    warning_active = true;
                }
            }

            stop = reading->start;
#ifdef _DEBUG_ANENOMETER
            Serial.print(reading->start); Serial.print("]="); Serial.print(reading->value); Serial.print(" / "); Serial.print(value); Serial.print("/s");
#endif
            // Higher weight for most recent values
            windspeed = windspeed * Anenometer::_DAMPENING + value;
            values++;
        }
    }

    if (!warning_active) {
        // no warning active? reset
        _wind_warned = 0;
    }

    // read to next buffer entry
    _index_reading = (_index_reading + 1) % Anenometer::_SAMPLES;
    _readings[_index_reading].start = now;
    _readings[_index_reading].value = 0;

    // weighted average of triggers per second
    _last_sample = windspeed / ((pow(Anenometer::_DAMPENING, values) - 1) / (Anenometer::_DAMPENING - 1));

#ifdef _DEBUG_ANENOMETER
    Serial.print(" => "); Serial.print(windspeed); Serial.print(" / "); Serial.print(_last_sample); Serial.println("km/h");
#endif
    return _last_sample;
}

void Anenometer::setWindWarning(void (*wind_warning)(float), float warn_threshold) {
    _wind_warning = wind_warning;
    _warn_threshold = warn_threshold;
    _wind_warned = 0;
}