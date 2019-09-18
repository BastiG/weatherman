#include "sensorhub.hpp"

#include "main.h"
#include <functional>

SensorHub::SensorHub(Basecamp *iot, MqttWeatherClient *mqtt) :
  _iot(iot), _iot_status("Basecamp failed"), _mqtt(mqtt),
  _bmp280_list(), _si7021_list(), _tsl2591_list(),
  _raingauge_list(), _anenometer_list(), _windvane_list() {
}

void SensorHub::setupBasecamp(void) {
  if (_iot->begin()) {
    _iot_status.initDone();

    _iot->web.addInterfaceElement("device_id", "input", "Device ID", "#configform", "DeviceId");
    _iot->web.setInterfaceElementAttribute("device_id", "type", "text");
  } else {
    _iot_status.fail("Basecamp init failed");
  }
}

bool SensorHub::isBasecampReady(void) {
  return _iot_status.isInitDone() && !_iot_status.isFail();
}

void SensorHub::setupBmp280(Adafruit_BMP280 *bmp280) {
  int id = _bmp280_list.size();
  std::function<bool()> beginfunc = std::bind(&Adafruit_BMP280::begin, bmp280, (unsigned char)'W', (unsigned char)'X');

  Sensor<Adafruit_BMP280> sensor = { .status=DeviceStatus(String("BMP280 ") + id + " failed"), .sensor=bmp280, .begin=beginfunc };
  if (sensor.begin()) {
    sensor.status.initDone();
  } else {
    sensor.status.fail(String("Unable to find BMP280 sensor ") + id);
  }
  _bmp280_list.push_back(sensor);
}

template <class T>
void SensorHub::prepareSensor(std::vector<Sensor<T>>& sensor_list) {
  for (int i=0; i<sensor_list.size(); i++) {
    Sensor<T> sensor = sensor_list[i];
    if (!sensor.status.isInitDone()) {
      sensor.begin();
    }
    if (sensor.status.isFail())  {
      if (sensor.status.shouldSignal() && _mqtt->sendMessage("error", 1, false, sensor.status.failureMessage())) {
        sensor.status.signalled();
      }
    }
  }
}

void SensorHub::prepareBmp280(void) {
  prepareSensor(_bmp280_list);
}


void setupBasecamp(Basecamp *iot) {
  if (iot->begin()) {
    iot_status.initDone();

    iot->web.addInterfaceElement("device_id", "input", "Device ID", "#configform", "DeviceId");
    iot->web.setInterfaceElementAttribute("device_id", "type", "text");
  } else {
    iot_status.fail("Basecamp init failed");
  }
}


void setupBmp280(void) {
  if (bmp280.begin()) {
    bmp280_status.initDone();
  } else {
    bmp280_status.fail("Unable to find BMP280 sensor");
  }
}

void setupSi7021(void) {
  if (si7021.begin()) {
    si7021_status.initDone();
  } else {
    si7021_status.fail("Unable to find SI7021 sensor");
  }
}

void setupTsl2591(void) {
  if (tsl2591.begin()) {
    tsl2591_status.initDone();
    tsl2591.setGain(TSL2591_GAIN_MED);
    tsl2591.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  } else {
    tsl2591_status.fail("Unable to find TSL2591 sensor");
  }
}

void setupRainGauge(void) {
  if (rainGauge.begin()) {
    rainGauge_status.initDone();
  } else {
    rainGauge_status.fail("Unable to initialize rain gauge");
  }
}

void setupAnenometer(void) {
  if (anenometer.begin()) {
    anenometer_status.initDone();
    anenometer.setStormWarning(storm_warning, 20);
  } else {
    anenometer_status.fail("Unable to initialize anenometer");
  }
}

void setupWindVane(void) {
  if (windVane.begin()) {
    windvane_status.initDone();
  } else {
    windvane_status.fail("Unable to initialize wind vane");
  }
}
