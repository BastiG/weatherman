#ifndef _WEATHERMAN_SENSORHUB_HPP_
#define _WEATHERMAN_SENSORHUB_HPP_

#include "DeviceStatus.hpp"
#include "Basecamp.hpp"
#include "mqtt.hpp"
#include <Adafruit_BMP280.h>
#include <Adafruit_Si7021.h>
#include <Adafruit_TSL2591.h>
#include "RainGauge.hpp"
#include "Anemometer.hpp"
#include "WindVane.hpp"

template <class T>
struct Sensor {
    DeviceStatus status;
    T* sensor;
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

        template <class T>
        void prepareSensor(std::vector<Sensor<T>>& sensor_list);
    public:
        SensorHub(Basecamp *iot, MqttWeatherClient *mqtt);

        void setupBasecamp(void);
        bool isBasecampReady(void);

        void setupBmp280(Adafruit_BMP280* bmp280);

        void prepareBmp280(void);
        void readTemperature(void);

/*        void setupSi7021(Adafruit_Si7021* si7021);
        void setupTsl2591(Adafruit_TSL2591* tsl2591);
        void setupRainGauge(RainGauge* raingauge);
        void setupAnenometer(Anenometer* anenometer);
        void setupWindVane(WindVane* windvane);*/
};

//void setupBasecamp(Basecamp *iot);
void setupBmp280(void);
void setupSi7021(void);
void setupTsl2591(void);
void setupRainGauge(void);
void setupAnenometer(void);
void setupWindVane(void);

#endif