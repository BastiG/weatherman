#include "SensorHub.hpp"

#include "main.h"
#include <functional>
#include <float.h>

#include "constants.h"

SensorHub::SensorHub(Basecamp *iot, MqttWeatherClient *mqtt) :
    _iot(iot), _iot_status("Basecamp failed"), _mqtt(mqtt),
    _bmp280_list(), _si7021_list(), _tsl2591_list(),
    _raingauge_list(), _anenometer_list(), _windvane_list(),
    _last_temperature(NAN), _last_pressure(NAN), _last_humidity(NAN),
    _last_luminosity(NAN), _last_rainlevel(NAN) {
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


template <class T>
bool SensorHub::setupSensor(std::vector<Sensor<T>>& sensor_list, T *device, String deviceName, std::function<bool(void)> beginFunc) {
  size_t id = sensor_list.size();

  Sensor<T> sensor = {
    .status=new DeviceStatus(deviceName + " " + id + " has failed"),
    .sensor=device,
    .begin=beginFunc
  };
  if (sensor.begin()) {
    sensor.status->initDone();
  } else {
    sensor.status->fail(deviceName + " " + id + " could not be initialized");
  }
  sensor_list.push_back(sensor);

  return sensor.status->isInitDone();
}

bool SensorHub::setupBmp280(Adafruit_BMP280 *bmp280) {
  auto begin280 = [](Adafruit_BMP280 *bmp280) { return bmp280->begin(); };
  std::function<bool(void)> beginFunc = std::bind(begin280, bmp280);

  return setupSensor(_bmp280_list, bmp280, "BMP280", beginFunc);
}

bool SensorHub::setupSi7021(Adafruit_Si7021 *si7021) {
  std::function<bool()> beginFunc = std::bind(&Adafruit_Si7021::begin, si7021);

  return setupSensor(_si7021_list, si7021, "Si7021", beginFunc);
}

bool SensorHub::setupTsl2591(Adafruit_TSL2591 *tsl2591) {
  std::function<bool()> beginFunc = std::bind(&Adafruit_TSL2591::begin, tsl2591);

  return setupSensor(_tsl2591_list, tsl2591, "TSL2591", beginFunc);
}

bool SensorHub::setupRainGauge(RainGauge *raingauge) {
  std::function<bool()> beginFunc = std::bind(&RainGauge::begin, raingauge);

  return setupSensor(_raingauge_list, raingauge, "Rain Gauge", beginFunc);
}


template <class T>
bool SensorHub::isSensorsReady(std::vector<Sensor<T>>& sensor_list) {
  for (uint8_t i=0; i<sensor_list.size(); i++) {
    Sensor<T> sensor = sensor_list[i];
    if (sensor.status->isInitDone() && !sensor.status->isFail())
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

bool SensorHub::isTsl2591Ready(void) {
  return isSensorsReady(_tsl2591_list);
}

bool SensorHub::isRainGaugeReady(void) {
  return isSensorsReady(_raingauge_list);
}


template <class T>
void SensorHub::prepareSensors(std::vector<Sensor<T>>& sensor_list) {
  for (uint8_t i=0; i<sensor_list.size(); i++) {
    Sensor<T> sensor = sensor_list[i];
    if (!sensor.status->isInitDone()) {
      if (sensor.begin()) {
        sensor.status->initDone();
      }
    }
    if (sensor.status->isFail())  {
      if (sensor.status->shouldSignal() && _mqtt->sendMessage("error", 1, false, sensor.status->failureMessage())) {
        sensor.status->signalled();
      }
    }
  }
}

void SensorHub::prepareBmp280(void) {
  prepareSensors(_bmp280_list);
}

void SensorHub::prepareSi7021(void) {
  prepareSensors(_si7021_list);
}

void SensorHub::prepareTsl2591(void) {
  prepareSensors(_tsl2591_list);
}

void SensorHub::prepareRainGauge(void) {
  prepareSensors(_raingauge_list);
}


float SensorHub::readTemperature(void) {
  float temperature = 0;
  uint8_t sources = 0;

  for (uint8_t i = 0; i<_bmp280_list.size(); i++) {
    Sensor<Adafruit_BMP280> sensor = _bmp280_list[i];
    if (!sensor.status->isInitDone()) continue;

    uint8_t status = sensor.sensor->getStatus();
    if (status == 0xFF) {
      sensor.status->fail();
    } else {
      if (sensor.status->isFail()) {
        sensor.sensor->reset();
        if (sensor.begin()) {
          sensor.status->recover();
        }
      }
      if (!sensor.status->isFail()) {
        temperature += sensor.sensor->readTemperature();
        sources++;
      }
    }
  }
  for (uint8_t i = 0; i<_si7021_list.size(); i++) {
    Sensor<Adafruit_Si7021> sensor = _si7021_list[i];
    if (!sensor.status->isInitDone()) continue;

    float value = sensor.sensor->readTemperature();
    if (isnan(value)) {
      sensor.status->fail();
    } else {
      sensor.status->recover();
      temperature += value;
      sources++;
    }
  }

  if (sources == 0) {
    resetTemperature();
    return NAN;
  }
  
  temperature /= sources;

  if (isnan(_last_temperature) || abs(_last_temperature - temperature) > _MIN_DELTA_TEMPERATURE) {
    if (_mqtt->sendMessage("temperature", 1, false, (String)temperature)) {
      Serial.print("Temperature published: "); Serial.println(temperature);
      _last_temperature = temperature;
    } else {
      //lastTemperature = -100.0;
      Serial.println("Couldn't publish temperature");
    }
  }

  return _last_temperature;
}

float SensorHub::readPressure(void) {
  float pressure = 0;
  uint8_t sources = 0;

  for (uint8_t i = 0; i<_bmp280_list.size(); i++) {
    Sensor<Adafruit_BMP280> sensor = _bmp280_list[i];
    if (!sensor.status->isInitDone()) continue;

    uint8_t status = sensor.sensor->getStatus();
    if (status == 0xFF) {
      sensor.status->fail();
    } else {
      if (sensor.status->isFail()) {
        sensor.sensor->reset();
        if (sensor.begin()) {
          sensor.status->recover();
        }
      }
      if (!sensor.status->isFail()) {
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
      Serial.print("Pressure published: "); Serial.println(pressure);
      _last_pressure = pressure;
    } else {
      //lastPressure = -100.0;
      Serial.println("Couldn't publish pressure");
    }
  }

  return _last_pressure;
}

float SensorHub::readHumidity(void) {
  float humidity = 0;
  uint8_t sources = 0;

  for (uint8_t i = 0; i<_si7021_list.size(); i++) {
    Sensor<Adafruit_Si7021> sensor = _si7021_list[i];
    if (!sensor.status->isInitDone()) continue;

    float value = sensor.sensor->readHumidity();
    if (isnan(value)) {
      sensor.status->fail();
    } else {
      sensor.status->recover();
      humidity += value;
      sources++;
    }
  }

  if (sources == 0) {
    resetHumidity();
    return NAN;
  }

  humidity /= sources;

  if (isnan(_last_humidity) || abs(_last_humidity - humidity) > _MIN_DELTA_HUMIDITY) {
    if (mqttClient.sendMessage("humidity", 1, false, (String)humidity)) {
      Serial.print("Humidity published: "); Serial.println(humidity);
      _last_humidity = humidity;
    } else {
      //lastHumidity = -100.0;
      Serial.println("Couldn't publish humidity");
    }
  }

  return _last_humidity;
}

float SensorHub::readLuminosity(void) {
  float luminosity = 0;
  uint8_t sources = 0;

  for (uint8_t i = 0; i<_tsl2591_list.size(); i++) {
    Sensor<Adafruit_TSL2591> sensor = _tsl2591_list[i];
    if (!sensor.status->isInitDone()) continue;

    uint8_t status = sensor.sensor->getStatus();
    if (status == 0xFF) {
      sensor.status->fail();
    } else {
      if (sensor.status->isFail()) {
        sensor.sensor->reset();
      }

      sensors_event_t event;
      sensor.sensor->getEvent(&event);
      if ((event.light == 0) |
          (event.light > 4294966000.0) | 
          (event.light <-4294966000.0)) {
        sensor.status->fail();
      } else {
        sensor.status->recover();
      }

      if (!sensor.status->isFail()) {
        luminosity += isnan(event.light) ? 0 : event.light;
        sources++;
      }
    }
  }

  if (sources == 0) {
    resetLuminosity();
    return NAN;
  }
  luminosity /= sources;

  if (isnan(_last_luminosity) || abs(_last_luminosity - luminosity) > _MIN_DELTA_LUMINOSITY || (luminosity == 0 && _last_luminosity != 0)) {
    if (mqttClient.sendMessage("luminosity", 1, false, (String)luminosity)) {
      Serial.print("Luminosity published: "); Serial.println(luminosity);
      _last_luminosity = luminosity;
    } else {
      //lastLuminosity = -100.0;
      Serial.println("Couldn't publish luminosity");
    }
  }

  return _last_luminosity;
}

float SensorHub::readRainLevel(void) {
  float rainlevel = 0;
  uint8_t sources = 0;

  for (uint8_t i=0; i<_raingauge_list.size(); i++) {
    Sensor<RainGauge> sensor = _raingauge_list[i];

    if (!sensor.status->isInitDone()) continue;

    float value = sensor.sensor->getRainLevel(RAIN_1H);
    //float value_15m = rainGauge.getRainLevel(RAIN_15M);
    //float value_1m = rainGauge.getRainLevel(RAIN_1M);
    if (isnan(value)) {
      sensor.status->fail();
    } else {
      sensor.status->recover();
      rainlevel += value;
      sources++;
    }
  }

  if (sources == 0) {
    resetRainLevel();
    return NAN;
  }

  rainlevel /= sources;

  if (isnan(_last_rainlevel) || abs(_last_rainlevel - rainlevel) > _MIN_DELTA_RAIN_LEVEL || (rainlevel == 0 && _last_rainlevel != 0)) {
    if (mqttClient.sendMessage("rain", 1, false, (String)rainlevel)) {
      Serial.print("Rain level published: "); Serial.println(rainlevel);
      _last_rainlevel = rainlevel;
    } else {
      Serial.println("Couldn't publish rain level");
    }
  }

  return _last_rainlevel;
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

void SensorHub::resetLuminosity(void) {
  _last_luminosity = NAN;
}

void SensorHub::resetRainLevel(void) {
  _last_rainlevel = NAN;
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
}*/

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
