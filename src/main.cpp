#include <math.h>

#include "main.h"
#include "setup.h"
#include "mqtt.h"
#include "reading.h"

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

DeviceStatus iot_status("Basecamp failed");
DeviceStatus bmp280_status("BMP280 failed");
DeviceStatus tsl2591_status("TSL2591 failed");
DeviceStatus si7021_status("SI7021 failed");
DeviceStatus anenometer_status("Anenometer failed");
DeviceStatus rainGauge_status("Rain gauge failed");
DeviceStatus windvane_status("Wind vane failed");

String ID("0");
const String MAIN_TOPIC("weatherman");



void storm_warning(float wind_speed) {
  sendMessage("wind/warning", ID, 0, false, (String)wind_speed);
}





void setup() {
  setupBasecamp();

  pinMode(PIN_LED, OUTPUT);

  setupBmp280();
  setupTsl2591();
  setupSi7021();

  setupRainGauge();
  setupAnenometer();
  setupWindVane();

  if (iot.configuration.keyExists("DeviceId"))
    ID = iot.configuration.get("DeviceId");

  Serial.println("Init complete, ID=" + ID);

  resetTemperature();
  resetLuminosity();
  resetPressure();
  resetHumidity();
  resetRainLevel();
  resetWindSpeed();
  resetWindDirection();
}

bool handleStatus(DeviceStatus &status, void (*setupFunc)(void)) {
  if (!status.isInitDone()) {
    setupFunc();
  }

  if (status.isFail()) {
    if (status.shouldSignal() && sendMessage("error", ID, 1, false, status.failureMessage())) {
      status.signalled();
    }
    return false;
  }
  return true;
}

void loop() {
  if (!handleStatus(iot_status, setupBasecamp) || iot.wifi.status() == WL_NO_SHIELD) {
    for (int i=0; i<BLINK_IOT_FAIL; i++) {
      digitalWrite(PIN_LED, (i % 2 == 0) ? HIGH : LOW);
      delay(DELAY_1 / BLINK_IOT_FAIL);
    }
  } else {
    digitalWrite(PIN_LED, HIGH);
    delay(DELAY_1);

    handleStatus(bmp280_status, setupBmp280);
    handleStatus(tsl2591_status, setupTsl2591);
    handleStatus(si7021_status, setupSi7021);

    handleStatus(rainGauge_status, setupRainGauge);
    handleStatus(anenometer_status, setupAnenometer);
    handleStatus(windvane_status, setupWindVane);

    readTemperature();
    readPressure();
    readLuminosity();
    readHumidity();

    readWindSpeed();
    readRainLevel();
    readWindDirection();
  }

  digitalWrite(PIN_LED, LOW);
  delay(DELAY_2);
}
