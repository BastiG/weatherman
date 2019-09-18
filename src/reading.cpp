#include "reading.h"

#include "main.h"
#include "sensorhub.hpp"
#include "mqtt.hpp"
#include "constants.h"

float _last_temperature;
float _last_pressure;
float _last_luminosity;
float _last_humidity;

float _last_rain_level;
float _last_wind_speed;
wind_direction_t _last_wind_direction;

/*
void readTemperature(void) {
  float temperature = 0;
  int sources = 0;

  if (bmp280_status.isInitDone()) {
    uint8_t status = bmp280.getStatus();
    //Serial.print("[BMP280 STATUS] "); Serial.println(status);
    if (status == 0xFF) {
      bmp280_status.fail();
    } else {
      if (bmp280_status.isFail()) {
        bmp280.reset();
//        setupBmp280();
        bmp280_status.recover();
      }
      float value = bmp280.readTemperature();
      //Serial.print("[BMP280 TEMP] "); Serial.println(value);
      temperature += value;
      sources++;
    }
  }
  if (si7021_status.isInitDone()) {
    float value = si7021.readTemperature();
    //Serial.print("[SI7021 TEMP] "); Serial.println(value);
    if (isnan(value)) {
      si7021_status.fail();
    } else {
      si7021_status.recover();
      temperature += value;
      sources++;
    }
  }

  if (sources == 0) return;
  temperature /= sources;

  if (isnan(_last_temperature) || abs(_last_temperature - temperature) > _MIN_DELTA_TEMPERATURE) {
    if (mqttClient.sendMessage("temperature", 1, false, (String)temperature)) {
      Serial.print("Temperature published: ");
      Serial.println(temperature);
      _last_temperature = temperature;
    } else {
      //lastTemperature = -100.0;
      Serial.println("Couldn't publish temperature");
    }
  }
}
void readPressure(void) {
  if (!bmp280_status.isInitDone()) return;

  uint8_t status = bmp280.getStatus();
  //Serial.print("[BMP280 STATUS] "); Serial.println(status);
  if (status == 0xFF) {
    bmp280_status.fail();
    return;
  }
  if (bmp280_status.isFail()) {
    bmp280.reset();
//    setupBmp280();
    bmp280_status.recover();
  }
  float pressure = bmp280.readPressure() / 100;
  //Serial.print("[BMP280 PRES] "); Serial.println(pressure);

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
*/
void readLuminosity(void) {
  if (!tsl2591_status.isInitDone()) return;

  uint8_t status = tsl2591.getStatus();
  //Serial.print("[TSL2591 STATUS] "); Serial.println(status);
  if (status == 0xFF) {
    tsl2591_status.fail();
    return;
  }
  if (tsl2591_status.isFail()) {
    tsl2591.reset();
  }

  sensors_event_t event;
  tsl2591.getEvent(&event);
  if ((event.light == 0) |
      (event.light > 4294966000.0) | 
      (event.light <-4294966000.0))
  {
      tsl2591_status.fail();
      return;
  }
  float luminosity = isnan(event.light) ? 0 : event.light;
  tsl2591_status.recover();

  if (isnan(_last_luminosity) || abs(_last_luminosity - luminosity) > _MIN_DELTA_LUMINOSITY || (luminosity == 0 && _last_luminosity != 0)) {
    if (mqttClient.sendMessage("luminosity", 1, false, (String)luminosity)) {
      Serial.print("Luminosity published: ");
      Serial.println(luminosity);
      _last_luminosity = luminosity;
    } else {
      //lastLuminosity = -100.0;
      Serial.println("Couldn't publish luminosity");
    }
  }
}
/*
void readHumidity(void) {
  if (!si7021_status.isInitDone()) return;

  float humidity = si7021.readHumidity();
  //Serial.print("[SI7021 HUM] "); Serial.println(humidity);
  if (isnan(humidity)) {
    si7021_status.fail();
    return;
  }
  si7021_status.recover();

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
*/
void readWindSpeed(void) {
  if (!anenometer_status.isInitDone()) return;

  float wind_speed = anenometer.getWindSpeed();
  if (isnan(wind_speed)) {
    anenometer_status.fail();
    return;
  }
  anenometer_status.recover();

  if (isnan(_last_wind_speed) || abs(_last_wind_speed - wind_speed) > _MIN_DELTA_WIND_SPEED || (wind_speed == 0 && _last_wind_speed != 0)) {
    if (mqttClient.sendMessage("wind/speed", 1, false, (String)wind_speed)) {
      Serial.print("Wind speed published: ");
      Serial.println(wind_speed);
      _last_wind_speed = wind_speed;
    } else {
      Serial.println("Couldn't publish wind speed");
    }
  }
}

void readRainLevel(void) {
  if (!rainGauge_status.isInitDone()) return;

  float rain_level = rainGauge.getRainLevel(RAIN_1H);
  if (isnan(rain_level)) {
    rainGauge_status.fail();
    return;
  }
  rainGauge_status.recover();

  if (isnan(_last_rain_level) || abs(_last_rain_level - rain_level) > _MIN_DELTA_RAIN_LEVEL || (rain_level == 0 && _last_rain_level != 0)) {
    //float rain_level_15m = rainGauge.getRainLevel(RAIN_15M);
    //float rain_level_1m = rainGauge.getRainLevel(RAIN_1M);
    if (mqttClient.sendMessage("rain", 1, false, (String)rain_level)) {
      Serial.print("Rain level published: ");
      Serial.println(rain_level);
      _last_rain_level = rain_level;
    } else {
      Serial.println("Couldn't publish rain level");
    }
  }
}

void readWindDirection(void) {
  wind_direction_t wind_direction = windVane.getWindDirection();
  if (wind_direction == WD_UNKNOWN) {
    windvane_status.fail();
    return;
  }
  windvane_status.recover();

  if (wind_direction != _last_wind_direction) {
    String direction_string = windVane.translateWindDirection(wind_direction);

    if (mqttClient.sendMessage("wind/direction", 1, false, direction_string)) {
      Serial.print("Wind direction published: ");
      Serial.println(direction_string);
      _last_wind_direction = wind_direction;
    } else {
      //lastHumidity = -100.0;
      Serial.println("Couldn't publish wind direction");
    }
  }
}


void resetTemperature(void) {
    _last_temperature = NAN;
}
void resetPressure(void) {
    _last_pressure = NAN;
}
void resetLuminosity(void) {
    _last_luminosity = NAN;
}
void resetHumidity(void) {
    _last_humidity = NAN;
}
void resetWindSpeed(void) {
    _last_wind_speed = NAN;
}
void resetRainLevel(void) {
    _last_rain_level = NAN;
}
void resetWindDirection(void) {
    _last_wind_speed = WD_UNKNOWN;
}
