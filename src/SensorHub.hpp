#ifndef _WEATHERMAN_SENSORHUB_HPP_
#define _WEATHERMAN_SENSORHUB_HPP_

#include "DeviceStatus.hpp"
#include "Basecamp.hpp"
#include "MqttWeatherClient.hpp"
#include <Adafruit_BMP280.h>
#include <Adafruit_Si7021.h>
#include <Adafruit_TSL2591.h>
#include "RainGauge.hpp"
#include "Anemometer.hpp"
#include "WindVane.hpp"

extern "C" {
  class MqttWeatherClient;
}

template <class T>
struct Sensor {
    DeviceStatus *status;
    T *sensor;
    std::function<bool(void)> begin;
};

class SensorHub{
    private:
        Basecamp *_iot;
        DeviceStatus _iot_status;
        MqttWeatherClient *_mqtt;
        std::vector<Sensor<Adafruit_BMP280>> _bmp280_list;
        std::vector<Sensor<Adafruit_Si7021>> _si7021_list;
        std::vector<Sensor<Adafruit_TSL2591>> _tsl2591_list;
        std::vector<Sensor<RainGauge>> _raingauge_list;
        std::vector<Sensor<Anenometer>> _anenometer_list;
        std::vector<Sensor<WindVane>> _windvane_list;

        float _last_temperature;
        float _last_pressure;
        float _last_humidity;
        float _last_luminosity;
        float _last_rainlevel;
        float _last_windspeed;
        wind_direction_t _last_winddirection;

        template <class T>
        bool setupSensor(std::vector<Sensor<T>>& sensor_list, T *device, String deviceName, std::function<bool(void)> beginFunc);
        template <class T>
        void prepareSensors(std::vector<Sensor<T>>& sensor_list);
        template <class T>
        bool isSensorsReady(std::vector<Sensor<T>>& sensor_list);
    public:
        SensorHub(Basecamp *iot, MqttWeatherClient *mqtt);

        void setupBasecamp(void);
        bool isBasecampReady(void);

        bool setupBmp280(Adafruit_BMP280 *bmp280);
        bool setupSi7021(Adafruit_Si7021 *si7021);
        bool setupTsl2591(Adafruit_TSL2591 *tsl2591);
        bool setupRainGauge(RainGauge *raingauge);
        bool setupAnenometer(Anenometer *anenometer);
        bool setupWindVane(WindVane *windvane);

        bool isBmp280Ready(void);
        bool isSi7021Ready(void);
        bool isTsl2591Ready(void);
        bool isRainGaugeReady(void);
        bool isAnenometerReady(void);
        bool isWindVaneReady(void);

        void prepareBmp280(void);
        void prepareSi7021(void);
        void prepareTsl2591(void);
        void prepareRainGauge(void);
        void prepareAnenometer(void);
        void prepareWindVane(void);

        float readTemperature(void);
        float readPressure(void);
        float readHumidity(void);
        float readLuminosity(void);
        float readRainLevel(void);
        float readWindSpeed(void);
        wind_direction_t readWindDirection(void);

        void resetTemperature(void);
        void resetPressure(void);
        void resetHumidity(void);
        void resetLuminosity(void);
        void resetRainLevel(void);
        void resetWindSpeed(void);
        void resetWindDirection(void);
};

#endif