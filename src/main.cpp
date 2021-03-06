#include <math.h>

#include "MqttWeatherClient.hpp"
#include "SensorHub.hpp"
#include "Logger.hpp"

#include "constants.h"

Basecamp iot{
  Basecamp::
  SetupModeWifiEncryption::
  secured
};

Adafruit_BMP280 bmp280;
Adafruit_TSL2591 tsl2591 = Adafruit_TSL2591(2591);
Adafruit_Si7021 si7021 = Adafruit_Si7021();
Anenometer anenometer(PIN_ANEMOMETER);
RainGauge rainGauge(PIN_RAINGAUGE);
WindVane windVane(PIN_WINDVANE);

MqttWeatherClient mqttClient(&iot.mqtt, &iot.configuration);
Logger logger(mqttClient);

SensorHub sensors(&iot, &mqttClient, logger);


void wind_warning_callback(float wind_speed) {
  mqttClient.sendMessage("wind/warning", 0, false, (String)wind_speed);
}

void setup() {
  sensors.setupBasecamp();
  mqttClient.setLWT();

  pinMode(PIN_LED, OUTPUT);

  sensors.setupBmp280(&bmp280);
  sensors.setupSi7021(&si7021);
  sensors.setupTsl2591(&tsl2591);
  
  sensors.setupRainGauge(&rainGauge);
  sensors.setupAnenometer(&anenometer);
  sensors.setupWindVane(&windVane);

  uint8_t wind_warning = DEFAULT_WIND_WARNING;
  if (iot.configuration.keyExists(CONFIG_WIND_WARNING)) {
    wind_warning = (uint8_t)std::atoi(iot.configuration.get(CONFIG_WIND_WARNING).c_str());
  }
  if (wind_warning) {
    anenometer.setWindWarning(wind_warning_callback, wind_warning);
  }

  Serial.println("Init complete");

  sensors.resetTemperature();
  sensors.resetLuminosity();
  sensors.resetPressure();
  sensors.resetHumidity();
  sensors.resetRain();
  sensors.resetWind();
}

void loop() {
  if (!sensors.isBasecampReady() || iot.wifi.status() == WL_NO_SHIELD) {
    for (int i=0; i<BLINK_IOT_FAIL; i++) {
      digitalWrite(PIN_LED, (i % 2 == 0) ? HIGH : LOW);
      delay(DELAY_1 / BLINK_IOT_FAIL);
    }
  } else {
    digitalWrite(PIN_LED, HIGH);
    delay(DELAY_1);

    sensors.prepareBmp280();
    sensors.prepareSi7021();
    sensors.prepareTsl2591();
    sensors.prepareRainGauge();
    sensors.prepareAnenometer();
    sensors.prepareWindVane();

    sensors.readTemperature();
    sensors.readPressure();
    sensors.readLuminosity();
    sensors.readHumidity();
    sensors.readRain();
    sensors.readWind();
  }

  digitalWrite(PIN_LED, LOW);
  delay(DELAY_2);
}
