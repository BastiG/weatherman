#include "setup.h"

#include "mqtt.h"

void setupBasecamp(void) {
  if (iot.begin()) {
    iot_status.initDone();
    iot.mqtt.onConnect(mqttConnected);
    iot.mqtt.onSubscribe(mqttSubscribed);
    iot.mqtt.onMessage(mqttMessage);
    iot.mqtt.onPublish(mqttPublished);
    iot.mqtt.onDisconnect(mqttDisconnected);

    iot.web.addInterfaceElement("device_id", "input", "Device ID", "#configform", "DeviceId");
    iot.web.setInterfaceElementAttribute("device_id", "type", "text");
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
