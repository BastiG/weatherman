#include "SensorHub.hpp"
#include "MqttWeatherClient.hpp"
#include <functional>
#include <float.h>

#include "constants.h"

SensorHub::SensorHub(Basecamp *iot, MqttWeatherClient *mqtt) :
    _iot(iot), _iot_status("Basecamp failed"), _mqtt(mqtt),
    _bmp280_list(), _si7021_list(), _tsl2591_list(),
    _raingauge_list(), _anenometer_list(), _windvane_list(),
    _last_temperature(NAN), _last_pressure(NAN), _last_humidity(NAN),
    _last_luminosity(NAN), _last_rainlevel(NAN), _last_windspeed(NAN),
    _last_winddirection(WD_UNKNOWN),
    _last_temperature_time(0), _last_pressure_time(0), _last_humidity_time(0),
    _last_luminosity_time(0), _last_rainlevel_time(0), _last_wind_time(0) {
  mqtt->setSensors(this);
}

void SensorHub::setupBasecamp(void) {
  if (_iot->begin()) {
    _iot_status.initDone();

    _iot->web.addInterfaceElement("device_id", "input", "Device ID", "#configform", CONFIG_DEVICE_ID);
    _iot->web.setInterfaceElementAttribute("device_id", "type", "text");

    _iot->web.addInterfaceElement("wind_warning", "input", "Wind Warning", "#configform", CONFIG_WIND_WARNING);
    _iot->web.setInterfaceElementAttribute("wind_warning", "type", "number");

    _iot->web.addInterfaceElement("beacon_timeout", "input", "Beacon Timeout", "#configform", CONFIG_BEACON_TIMEOUT);
    _iot->web.setInterfaceElementAttribute("beacon_timeout", "type", "number");

    if (_iot->configuration.keyExists(CONFIG_BEACON_TIMEOUT)) {
      _beacon_timeout = std::strtoul(_iot->configuration.get(CONFIG_BEACON_TIMEOUT).c_str(), nullptr, 10) * 60 * 1000;
    } else {
      _beacon_timeout = DEFAULT_BEACON_TIMEOUT;
    }
  } else {
    _iot_status.fail("Basecamp init failed");
  }
}

bool SensorHub::isBasecampReady(void) {
  return _iot_status.isInitDone() && !_iot_status.isFail();
}


template <class T>
bool SensorHub::setupSensor(std::vector<Sensor<T>>& sensor_list, T *device, String deviceName, std::function<bool(void)> beginFunc, void* additional) {
  size_t id = sensor_list.size();

  Sensor<T> sensor = {
    .status=new DeviceStatus(deviceName + " " + id + " has failed"),
    .sensor=device,
    .begin=beginFunc,
    .additional=additional
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
  Si7021_Additional *additional = new Si7021_Additional();
  additional->heating_since=0;
  return setupSensor(_si7021_list, si7021, "Si7021", beginFunc, additional);
}

bool SensorHub::setupTsl2591(Adafruit_TSL2591 *tsl2591) {
  std::function<bool()> beginFunc = std::bind(&Adafruit_TSL2591::begin, tsl2591);

  return setupSensor(_tsl2591_list, tsl2591, "TSL2591", beginFunc);
}

bool SensorHub::setupRainGauge(RainGauge *raingauge) {
  std::function<bool()> beginFunc = std::bind(&RainGauge::begin, raingauge);

  return setupSensor(_raingauge_list, raingauge, "Rain Gauge", beginFunc);
}

bool SensorHub::setupAnenometer(Anenometer *anenometer) {
  std::function<bool()> beginFunc = std::bind(&Anenometer::begin, anenometer);

  return setupSensor(_anenometer_list, anenometer, "Anenometer", beginFunc);
}

bool SensorHub::setupWindVane(WindVane *windvane) {
  std::function<bool()> beginFunc = std::bind(&WindVane::begin, windvane);

  return setupSensor(_windvane_list, windvane, "Wine Vane", beginFunc);
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

bool SensorHub::isAnenometerReady(void) {
  return isSensorsReady(_anenometer_list);
}

bool SensorHub::isWindVaneReady(void) {
  return isSensorsReady(_windvane_list);
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

void SensorHub::prepareAnenometer(void) {
  prepareSensors(_anenometer_list);
}

void SensorHub::prepareWindVane(void) {
  prepareSensors(_windvane_list);
}


float SensorHub::readTemperature(void) {
  float temperature = 0;
  uint8_t sources = 0;
  ulong now = millis();

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

  if (isnan(_last_temperature) || abs(_last_temperature - temperature) > _MIN_DELTA_TEMPERATURE ||
      (_beacon_timeout && now - _last_temperature_time > _beacon_timeout)) {
    if (_mqtt->sendMessage("temperature", 1, false, (String)temperature)) {
      Serial.print("Temperature published: "); Serial.println(temperature);
      _last_temperature = temperature;
      _last_temperature_time = now;
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
  ulong now = millis();

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

  if (isnan(_last_pressure) || abs(_last_pressure - pressure) > _MIN_DELTA_PRESSURE ||
      (_beacon_timeout && now - _last_pressure_time > _beacon_timeout)) {
    if (_mqtt->sendMessage("pressure", 1, false, (String)pressure)) {
      Serial.print("Pressure published: "); Serial.println(pressure);
      _last_pressure = pressure;
      _last_pressure_time = now;
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
  ulong now = millis();

  for (uint8_t i = 0; i<_si7021_list.size(); i++) {
    Sensor<Adafruit_Si7021> sensor = _si7021_list[i];
    if (!sensor.status->isInitDone()) continue;

    float value = sensor.sensor->readHumidity();
    if (isnan(value)) {
      sensor.status->fail();
    } else {
      sensor.status->recover();
/*
      if (value > 80) {
        Si7021_Additional *additional = (Si7021_Additional*)sensor.additional;
        if (additional->heating_since == 0) {
          sensor.sensor->setHeater(true);
          additional->heating_since++;
        } else if (additional->heating_since < MAX_HEATER_DURATION) {
          additional->heating_since++;
          continue;
        } else {
          sensor.sensor->setHeader(false);
          additional->heating_since=0;
          continue;
        }
      }
*/
      humidity += value;
      sources++;
    }
  }

  if (sources == 0) {
    resetHumidity();
    return NAN;
  }

  humidity /= sources;

  if (isnan(_last_humidity) || abs(_last_humidity - humidity) > _MIN_DELTA_HUMIDITY ||
      (_beacon_timeout && now - _last_humidity_time > _beacon_timeout)) {
    if (_mqtt->sendMessage("humidity", 1, false, (String)humidity)) {
      Serial.print("Humidity published: "); Serial.println(humidity);
      _last_humidity = humidity;
      _last_humidity_time = now;
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
  ulong now = millis();

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
      if ((event.light == 0) ||
          (event.light > 4294966000.0) || 
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

  if (isnan(_last_luminosity) || abs(_last_luminosity - luminosity) > _MIN_DELTA_LUMINOSITY || (luminosity == 0 && _last_luminosity != 0) ||
      (_beacon_timeout && now - _last_luminosity_time > _beacon_timeout)) {
    if (_mqtt->sendMessage("luminosity", 1, false, (String)luminosity)) {
      Serial.print("Luminosity published: "); Serial.println(luminosity);
      _last_luminosity = luminosity;
      _last_luminosity_time = now;
    } else {
      //lastLuminosity = -100.0;
      Serial.println("Couldn't publish luminosity");
    }
  }

  return _last_luminosity;
}

float SensorHub::readRain(void) {
  float rainlevel = 0;
  uint8_t sources = 0;
  ulong now = millis();

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
    resetRain();
    return NAN;
  }

  rainlevel /= sources;

  if (isnan(_last_rainlevel) || abs(_last_rainlevel - rainlevel) > _MIN_DELTA_RAIN_LEVEL || (rainlevel == 0 && _last_rainlevel != 0) ||
      (_beacon_timeout && now - _last_rainlevel_time > _beacon_timeout)) {
    if (_mqtt->sendMessage("rain", 1, false, (String)rainlevel)) {
      Serial.print("Rain level published: "); Serial.println(rainlevel);
      _last_rainlevel = rainlevel;
      _last_rainlevel_time = now;
    } else {
      Serial.println("Couldn't publish rain level");
    }
  }

  return _last_rainlevel;
}


float SensorHub::readWind(void) {
  float windspeed = 0;
  uint8_t sources = 0;
  ulong now = millis();

  for (uint8_t i=0; i<_anenometer_list.size(); i++) {
    Sensor<Anenometer> sensor = _anenometer_list[i];

    if (!sensor.status->isInitDone()) continue;

    float value = sensor.sensor->getWindSpeed();
    if (isnan(value)) {
      sensor.status->fail();
    } else {
      sensor.status->recover();
      windspeed += value;
      sources++;
    }
  }

  wind_direction_t winddirection = getWindDirection();

  if (sources == 0 || winddirection == WD_UNKNOWN) {
    resetWind();
    return NAN;
  }

  windspeed /= sources;

  if (isnan(_last_windspeed) || abs(_last_windspeed - windspeed) > _MIN_DELTA_WIND_SPEED || (windspeed == 0 && _last_windspeed != 0) || winddirection != _last_winddirection ||
      (_beacon_timeout && now - _last_wind_time > _beacon_timeout)) {
    String message = "{\"speed\":" + String(windspeed) + ",\"degrees\":" + WindVane::toDegrees(winddirection) + ",\"direction\":\"" + WindVane::toName(winddirection) + "\"}";
    if (_mqtt->sendMessage("wind", 1, false, message)) {
      Serial.print("Wind reading published, speed: "); Serial.print(windspeed); Serial.print(", direction: "); Serial.println(winddirection);
      _last_windspeed = windspeed;
      _last_winddirection = winddirection;
      _last_wind_time = now;
    } else {
      Serial.println("Couldn't publish wind reading");
    }
  }

  return _last_windspeed;
}

wind_direction_t SensorHub::getWindDirection(void) {
  std::vector<uint8_t> directions;
  uint8_t max_value = WD_UNKNOWN;
  uint8_t sources = 0;

  for (uint8_t i = 0; i<_windvane_list.size(); i++) {
    Sensor<WindVane> sensor = _windvane_list[i];

    if (!sensor.status->isInitDone()) continue;

    uint8_t value = sensor.sensor->getWindDirection();
    if (value == WD_UNKNOWN) {
      sensor.status->fail();
    } else {
      sensor.status->recover();
      if (value > max_value || sources == 0) {
        if (sources > 0) directions.push_back(max_value);
        max_value = value;
      } else {
        directions.push_back(value);
      }
      sources++;
    }
  }

  if (sources == 0) {
    resetWind();
    return WD_UNKNOWN;
  }

  int16_t delta = 0;
  for (uint8_t i=0; i<directions.size(); i++) {
    uint8_t value = directions[i];
    if (abs(value - max_value) < abs(value + 16 - max_value)) {
      delta += (value - max_value);
    } else {
      delta += (value + 16 - max_value);
    }
  }

  return static_cast<wind_direction_t>((uint8_t)(max_value + 16 + round(delta / sources)) % 16);
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

void SensorHub::resetRain(void) {
  _last_rainlevel = NAN;
}

void SensorHub::resetWind(void) {
  _last_windspeed = NAN;
  _last_winddirection = WD_UNKNOWN;
}
