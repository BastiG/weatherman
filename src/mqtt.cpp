#include "mqtt.hpp"
#include <functional>

#include "main.h"
#include "reading.h"

#include "constants.h"

const String MqttWeatherClient::MAIN_TOPIC = MQTT_MAIN_TOPIC;

MqttWeatherClient::MqttWeatherClient(AsyncMqttClient* mqtt, Configuration* configuration) :
        _mqtt_failed(0), _mqtt(mqtt), _id("0") {
    Serial.println("MqttWeatherClient constructor called");

    using namespace std::placeholders;

    _mqtt->onConnect(std::bind(&MqttWeatherClient::mqttConnected, this, _1));
    _mqtt->onDisconnect(std::bind(&MqttWeatherClient::mqttDisconnected, this, _1));
    _mqtt->onSubscribe(std::bind(&MqttWeatherClient::mqttSubscribed, this, _1, _2));
    _mqtt->onMessage(std::bind(&MqttWeatherClient::mqttMessage, this, _1, _2, _3, _4, _5, _6));
    _mqtt->onPublish(std::bind(&MqttWeatherClient::mqttPublished, this, _1));

    if (configuration->keyExists(CONFIG_DEVICE_ID)) {
      _id = configuration->get(CONFIG_DEVICE_ID);
    }
}

void MqttWeatherClient::setDeviceId(String id) {
    _id = id;

    messageCallbacks.clear();
    messageCallbacks[MAIN_TOPIC + "/send/" + _id + "/status"] = std::vector<void (MqttWeatherClient::*)(char*)> { &MqttWeatherClient::mqttSendStatus };
    messageCallbacks[MAIN_TOPIC + "/send/*/status"] = std::vector<void (MqttWeatherClient::*)(char*)> { &MqttWeatherClient::mqttSendStatus };

    _mqtt->disconnect(true);
}

void MqttWeatherClient::mqttSendStatus(char* payload) {
    sendMessage("status/uptime", 0, false, String(millis()));
    sendMessage("status/bmp280", 0, false, _sensorHub ? (_sensorHub->isBmp280Ready() ? "OK" : "FAILED") : "UNKNOWN");
    sendMessage("status/tsl2591", 0, false, !tsl2591_status.isInitDone() || tsl2591_status.isFail() ? "FAILED" : "OK");
    sendMessage("status/si7021", 0, false, _sensorHub ? (_sensorHub->isSi7021Ready() ? "OK" : "FAILED") : "UNKNOWN");

    sendMessage("status/anemometer", 0, false, !anenometer_status.isInitDone() || anenometer_status.isFail() ? "FAILED" : "OK");
    sendMessage("status/raingauge", 0, false, !rainGauge_status.isInitDone() || rainGauge_status.isFail() ? "FAILED" : "OK");
    sendMessage("status/windvane", 0, false, !windvane_status.isInitDone() || windvane_status.isFail() ? "FAILED" : "OK");
}

void MqttWeatherClient::mqttConnected(bool sessionPresent) {
  Serial.print("MQTT connected - ");
  Serial.println(sessionPresent);
  _mqtt->subscribe((MAIN_TOPIC + "/send/" + _id + "/#").c_str(),2);
  _mqtt->subscribe((MAIN_TOPIC + "/send/*/#").c_str(),2);
  _mqtt->publish((MAIN_TOPIC + "/status/online/" + _id).c_str(), 1, true, String(millis()).c_str());
}

void MqttWeatherClient::mqttDisconnected(AsyncMqttClientDisconnectReason reason) {
  Serial.println("MQTT connection lost");
}

void MqttWeatherClient::mqttSubscribed(uint16_t packetId, uint8_t qos) {
  Serial.println("Successfully subscribed");
}

void MqttWeatherClient::mqttMessage(char* topic, char* payload,
  AsyncMqttClientMessageProperties properties,
  size_t len, size_t index, size_t total) {
  Serial.print("[MQTT] Topic: ");
  Serial.println(topic);
  Serial.print("[MQTT] Payload: ");
  Serial.println(payload);

  Serial.print("Message callback: "); Serial.println(messageCallbacks.size());

  std::map<String, std::vector<void (MqttWeatherClient::*)(char*)>>::iterator it;
  for (it = messageCallbacks.begin(); it != messageCallbacks.end(); ++it) {
      Serial.print(" -> "); Serial.println(it->first);
  }
  it = messageCallbacks.find(String(topic));
  if (it != messageCallbacks.end()) {
      (this->*(it->second[0]))(payload);
  } else {
      Serial.println("[MQTT] No suitable message callback found for topic");
  }
  
  String sendTemperature = MAIN_TOPIC + "/send/temperature";
  String sendLuminosity = MAIN_TOPIC + "/send/luminosity";
  String sendHumidity = MAIN_TOPIC + "/send/humidity";
  String sendPressure = MAIN_TOPIC + "/send/pressure";
  String sendRain = MAIN_TOPIC + "/send/rain";
  String sendWindSpeed = MAIN_TOPIC + "/send/wind/speed";
  String sendWindDirection = MAIN_TOPIC + "/send/wind/direction";
  String sendAll = MAIN_TOPIC + "/send/all";
  String sendStatus = MAIN_TOPIC + "/send/status";
  
  if (sendTemperature == topic || sendAll == topic) {
    if (_sensorHub) _sensorHub->resetTemperature();
  }
  if (sendLuminosity == topic || sendAll == topic) {
    resetLuminosity();
  }
  if (sendHumidity == topic || sendAll == topic) {
    if (_sensorHub) _sensorHub->resetHumidity();
  }
  if (sendPressure == topic || sendAll == topic) {
    if (_sensorHub) _sensorHub->resetPressure();
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
    sendMessage("status/uptime", 0, false, String(millis()));
    sendMessage("status/bmp280", 0, false, _sensorHub ? (_sensorHub->isBmp280Ready() ? "OK" : "FAILED") : "UNKNOWN");
    sendMessage("status/tsl2591", 0, false, !tsl2591_status.isInitDone() || tsl2591_status.isFail() ? "FAILED" : "OK");
    sendMessage("status/si7021", 0, false, _sensorHub ? (_sensorHub->isSi7021Ready() ? "OK" : "FAILED") : "UNKNOWN");

    sendMessage("status/anemometer", 0, false, !anenometer_status.isInitDone() || anenometer_status.isFail() ? "FAILED" : "OK");
    sendMessage("status/raingauge", 0, false, !rainGauge_status.isInitDone() || rainGauge_status.isFail() ? "FAILED" : "OK");
    sendMessage("status/windvane", 0, false, !windvane_status.isInitDone() || windvane_status.isFail() ? "FAILED" : "OK");
  }
}

void MqttWeatherClient::mqttPublished(uint16_t packetId) {
  Serial.println("MQTT message published");
}

bool MqttWeatherClient::sendMessage(String type, int qos, bool persistent, String payload) {
  if (!_mqtt->connected()) {
    Serial.println("Not connected to MQTT, try & connect");
    _mqtt->connect();
  }
  if (_mqtt->connected()) {
    String topic = MAIN_TOPIC + "/" + type + "/" + _id;
    uint16_t rc = _mqtt->publish(topic.c_str(), qos, persistent, payload.c_str());
    if (rc == 0) {
      Serial.print("MQTT publish failed: ");
      Serial.println(_mqtt_failed);
      if (_mqtt_failed++ > 5) {
        Serial.println("Forcing reconnect");
        _mqtt->disconnect(true);
        _mqtt->connect();
      }
      return false;
    }
    _mqtt_failed = 0;
    return true;
  } else {
    return false;
  }
}

String MqttWeatherClient::getId(void) {
  return _id;
}

void MqttWeatherClient::setSensors(SensorHub *sensorHub) {
  _sensorHub = sensorHub;
}