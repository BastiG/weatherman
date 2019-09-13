#include "mqtt.h"

#include "reading.h"

int _mqtt_failed = 0;

void mqttConnected(bool sessionPresent) {
  Serial.print("MQTT connected - ");
  Serial.println(sessionPresent);
  iot.mqtt.subscribe((MAIN_TOPIC + "/send/" + ID + "/#").c_str(),2);
  iot.mqtt.subscribe((MAIN_TOPIC + "/send/all/#").c_str(),2);
  iot.mqtt.publish((MAIN_TOPIC + "/status/online").c_str(), 1, true, ID.c_str());
};

void mqttDisconnected(AsyncMqttClientDisconnectReason reason) {
  //Serial.println("MQTT connection lost");
}

void mqttSubscribed(uint16_t packetId, uint8_t qos) {
  Serial.println("Successfully subscribed");
};

void mqttMessage(char* topic, char* payload,
  AsyncMqttClientMessageProperties properties,
  size_t len, size_t index, size_t total) {
  Serial.println("MQTT received subscribed topic:");
  Serial.print("Topic:");
  Serial.println(topic);
  Serial.print("Payload:");
  Serial.println(payload);
  
  String sendTemperature = iot.hostname + "/send/temperature";
  String sendLuminosity = iot.hostname + "/send/luminosity";
  String sendHumidity = iot.hostname + "/send/humidity";
  String sendPressure = iot.hostname + "/send/pressure";
  String sendRain = iot.hostname + "/send/rain";
  String sendWindSpeed = iot.hostname + "/send/wind/speed";
  String sendWindDirection = iot.hostname + "/send/wind/direction";
  String sendAll = iot.hostname + "/send/all";
  String sendStatus = iot.hostname + "/send/status";
  
  if (sendTemperature == topic || sendAll == topic) {
    resetTemperature();
  }
  if (sendLuminosity == topic || sendAll == topic) {
    resetLuminosity();
  }
  if (sendHumidity == topic || sendAll == topic) {
    resetHumidity();
  }
  if (sendPressure == topic || sendAll == topic) {
    resetPressure();
  }
  if (sendRain == topic || sendAll == topic) {
    resetRainLevel();
  }
  if (sendWindSpeed == topic || sendAll == topic) {
    resetWindSpeed();
  }
  if (sendWindDirection == topic || sendAll == topic) {
    resetWindDirection();
  }
  if (sendStatus == topic) {
    sendMessage("status/bmp280", ID, 0, false, !bmp280_status.isInitDone() || bmp280_status.isFail() ? "FAILED" : "OK");
    sendMessage("status/tsl2591", ID, 0, false, !tsl2591_status.isInitDone() || tsl2591_status.isFail() ? "FAILED" : "OK");
    sendMessage("status/si7021", ID, 0, false, !si7021_status.isInitDone() || si7021_status.isFail() ? "FAILED" : "OK");

    sendMessage("status/anemometer", ID, 0, false, !anenometer_status.isInitDone() || anenometer_status.isFail() ? "FAILED" : "OK");
    sendMessage("status/raingauge", ID, 0, false, !rainGauge_status.isInitDone() || rainGauge_status.isFail() ? "FAILED" : "OK");
    sendMessage("status/windvane", ID, 0, false, !windvane_status.isInitDone() || windvane_status.isFail() ? "FAILED" : "OK");
  }
};

void mqttPublished(uint16_t packetId) {
  Serial.println("MQTT message published");
};

bool sendMessage(String type, String id, int qos, bool persistent, String payload) {
  if (!iot.mqtt.connected()) {
    Serial.println("Not connected to MQTT, try & connect");
    iot.mqtt.connect();
  }
  if (iot.mqtt.connected()) {
    String topic = MAIN_TOPIC + "/" + type + "/" + id;
    uint16_t rc = iot.mqtt.publish(topic.c_str(), qos, persistent, payload.c_str());
    if (rc == 0) {
      Serial.print("MQTT publish failed: ");
      Serial.println(_mqtt_failed);
      if (_mqtt_failed++ > 5) {
        Serial.println("Forcing reconnect");
        iot.mqtt.disconnect(true);
        iot.mqtt.connect();
      }
      return false;
    }
    _mqtt_failed = 0;
    return true;
  } else {
    return false;
  }
}