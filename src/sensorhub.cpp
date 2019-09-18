#include "sensorhub.hpp"

#include "main.h"
#include <functional>
#include <float.h>

#include "constants.h"

SensorHub::SensorHub(Basecamp *iot, MqttWeatherClient *mqtt) :
    _iot(iot), _iot_status("Basecamp failed"), _mqtt(mqtt),
    _bmp280_list(), _si7021_list(), _tsl2591_list(),
    _raingauge_list(), _anenometer_list(), _windvane_list(),
    _last_temperature(NAN), _last_pressure(NAN), _last_humidity(NAN) {
  mqtt->setSensors(this);
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

  Sensor<Adafruit_BMP280> sensor = {
    .status=DeviceStatus(String("BMP280 ") + id + " failed"),
    .sensor=bmp280,
    .begin=beginfunc
  };
  if (sensor.begin()) {
    sensor.status.initDone();
  } else {
    sensor.status.fail(String("Unable to find BMP280 sensor ") + id);
  }
  _bmp280_list.push_back(sensor);
}

void SensorHub::setupSi7021(Adafruit_Si7021 *si7021) {
  int id = _si7021_list.size();
  std::function<bool()> beginfunc = std::bind(&Adafruit_Si7021::begin, si7021);

  Sensor<Adafruit_Si7021> sensor = {
    .status=DeviceStatus(String("Si7021 ") + id + " failed"),
    .sensor=si7021,
    .begin=beginfunc
  };
  if (sensor.begin()) {
    sensor.status.initDone();
  } else {
    sensor.status.fail(String("Unable to find Si7021 sensor ") + id);
  }
  _si7021_list.push_back(sensor);
}

template <class T>
void SensorHub::prepareSensors(std::vector<Sensor<T>>& sensor_list) {
  for (uint8_t i=0; i<sensor_list.size(); i++) {
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

template <class T>
bool SensorHub::isSensorsReady(std::vector<Sensor<T>>& sensor_list) {
  for (uint8_t i=0; i<sensor_list.size(); i++) {
    Sensor<T> sensor = sensor_list[i];
    if (sensor.status.isInitDone() && !sensor.status.isFail())
      return true;
  }
  return false;
}

bool SensorHub::isBmp280Ready(void) {
  return isSensorsReady(_bmp280_list);
}

bool SensorHub::isSi7021Ready(void) {
  return isSensorsReady(_si7021_list);
}

void SensorHub::prepareBmp280(void) {
  prepareSensors(_bmp280_list);
}

void SensorHub::prepareSi7021(void) {
  prepareSensors(_si7021_list);
}

float SensorHub::readTemperature(void) {
  float temperature = 0;
  float sources = 0;

  for (uint8_t i = 0; i<_bmp280_list.size(); i++) {
    Sensor<Adafruit_BMP280> sensor = _bmp280_list[i];
    if (sensor.status.isInitDone()) {
      uint8_t status = sensor.sensor->getStatus();
      if (status == 0xFF) {
        sensor.status.fail();
      } else {
        if (sensor.status.isFail()) {
          sensor.sensor->reset();
          sensor.status.recover();
        }
        temperature += sensor.sensor->readTemperature();
        sources++;
      }
    }
  }
  for (uint8_t i = 0; i<_si7021_list.size(); i++) {
    Sensor<Adafruit_Si7021> sensor = _si7021_list[i];
    if (sensor.status.isInitDone()) {
      float value = sensor.sensor->readTemperature();
      if (isnan(value)) {
        sensor.status.fail();
      } else {
        sensor.status.recover();
        temperature += value;
        sources++;
      }
    }
  }

  if (sources == 0) {
    resetTemperature();
    return NAN;
  }
  
  temperature /= sources;

  if (isnan(_last_temperature) || abs(_last_temperature - temperature) > _MIN_DELTA_TEMPERATURE) {
    if (_mqtt->sendMessage("temperature", 1, false, (String)temperature)) {
      Serial.print("Temperature published: ");
      Serial.println(temperature);
      _last_temperature = temperature;
    } else {
      //lastTemperature = -100.0;
      Serial.println("Couldn't publish temperature");
    }
  }
}

float SensorHub::readPressure(void) {
  float pressure = 0;
  float sources = 0;

  for (uint8_t i = 0; i<_bmp280_list.size(); i++) {
    Sensor<Adafruit_BMP280> sensor = _bmp280_list[i];
    if (sensor.status.isInitDone()) {
      uint8_t status = sensor.sensor->getStatus();
      if (status == 0xFF) {
        sensor.status.fail();
      } else {
        if (sensor.status.isFail()) {
          sensor.sensor->reset();
          sensor.status.recover();
        }
        pressure += sensor.sensor->readPressure() / 100;
        sources++;
      }
    }
  }

  if (sources == 0) {
    resetPressure();
    return NAN;
  }

  pressure /= sources;

  if (isnan(_last_pressure) || abs(_last_pressure - pressure) > _MIN_DELTA_PRESSURE) {
    if (mqttClient.sendMessage("pressure", 1, false, (String)pressure)) {
      Serial.print("Pressure published: ");
      Serial.println(pressure);
      _last_pressure = pressure;
    } else {
      //lastPressure = -100.0;
      Serial.println("Couldn't publish pressure");
    }
  }
}

float SensorHub::readHumidity(void) {
  float humidity = 0;
  float sources = 0;

  for (uint8_t i = 0; i<_si7021_list.size(); i++) {
    Sensor<Adafruit_Si7021> sensor = _si7021_list[i];
    if (sensor.status.isInitDone()) {
      float value = sensor.sensor->readHumidity();
      if (isnan(value)) {
        sensor.status.fail();
      } else {
        sensor.status.recover();
        humidity += value;
        sources++;
      }
    }
  }

  if (sources == 0) {
    resetHumidity();
    return NAN;
  }

  humidity /= sources;

  if (isnan(_last_humidity) || abs(_last_humidity - humidity) > _MIN_DELTA_HUMIDITY) {
    if (mqttClient.sendMessage("humidity", 1, false, (String)humidity)) {
      Serial.print("Humidity published: ");
      Serial.println(humidity);
      _last_humidity = humidity;
    } else {
      //lastHumidity = -100.0;
      Serial.println("Couldn't publish humidity");
    }
  }
}

void SensorHub::resetTemperature(void) {
  _last_temperature = NAN;
}

void SensorHub::resetPressure(void) {
  _last_pressure = NAN;
}

void SensorHub::resetHumidity(void) {
  _last_humidity = NAN;
}

/*void setupBasecamp(Basecamp *iot) {
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
}*/

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
