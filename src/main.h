#ifndef _WEATHERMAN_MAIN_H_
#define _WEATHERMAN_MAIN_H_

#include <Basecamp.hpp>

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_TSL2591.h>
#include <Adafruit_Si7021.h>
 
#include <DeviceStatus.hpp>

#include <Anemometer.hpp>
#include <RainGauge.hpp>
#include <WindVane.hpp>

#include "mqtt.hpp"


extern MqttWeatherClient mqttClient;

//extern Adafruit_BMP280 bmp280;
//extern Adafruit_TSL2591 tsl2591;
//extern Adafruit_Si7021 si7021;
extern Anenometer anenometer;
//extern RainGauge rainGauge;
extern WindVane windVane;

//extern DeviceStatus iot_status;
//extern DeviceStatus bmp280_status;
//extern DeviceStatus tsl2591_status;
//extern DeviceStatus si7021_status;

extern DeviceStatus anenometer_status;
//extern DeviceStatus rainGauge_status;
extern DeviceStatus windvane_status;


void storm_warning(float wind_speed);

#endif