#include "MqttWeatherClient.hpp"
#include <functional>

#include "constants.h"

const String MqttWeatherClient::MAIN_TOPIC = MQTT_MAIN_TOPIC;

MqttWeatherClient::MqttWeatherClient(AsyncMqttClient *mqtt, Configuration *configuration) :
        _mqtt_failed(0), _mqtt(mqtt), _configuration(configuration), _id("0") {
    Serial.println("MqttWeatherClient constructor called");

    using namespace std::placeholders;

    // TODO set LWT

    _mqtt->onConnect(std::bind(&MqttWeatherClient::mqttConnected, this, _1));
    _mqtt->onDisconnect(std::bind(&MqttWeatherClient::mqttDisconnected, this, _1));
    _mqtt->onSubscribe(std::bind(&MqttWeatherClient::mqttSubscribed, this, _1, _2));
    _mqtt->onMessage(std::bind(&MqttWeatherClient::mqttMessage, this, _1, _2, _3, _4, _5, _6));
    _mqtt->onPublish(std::bind(&MqttWeatherClient::mqttPublished, this, _1));
}

void MqttWeatherClient::mqttSendStatus(char* payload) {
    sendMessage("status/uptime", 0, false, String(millis()));

    // TODO want to see individual sensor states? rather than "is one out of all still working"
    sendMessage("status/bmp280", 0, false, _sensorHub ? (_sensorHub->isBmp280Ready() ? "OK" : "FAILED") : "UNKNOWN");
    sendMessage("status/tsl2591", 0, false, _sensorHub ? (_sensorHub->isTsl2591Ready() ? "OK" : "FAILED") : "UNKNOWN");
    sendMessage("status/si7021", 0, false, _sensorHub ? (_sensorHub->isSi7021Ready() ? "OK" : "FAILED") : "UNKNOWN");

    sendMessage("status/anemometer", 0, false, _sensorHub ? (_sensorHub->isAnenometerReady() ? "OK" : "FAILED") : "UNKNOWN");
    sendMessage("status/raingauge", 0, false, _sensorHub ? (_sensorHub->isRainGaugeReady() ? "OK" : "FAILED") : "UNKNOWN");
    sendMessage("status/windvane", 0, false, _sensorHub ? (_sensorHub->isWindVaneReady() ? "OK" : "FAILED") : "UNKNOWN");
}

void MqttWeatherClient::registerCallback(String topic, mqtt_callback_t callback) {
  std::vector<String> topics = {
    MqttWeatherClient::MAIN_TOPIC + "/send/" + _id + "/" + topic,
    MqttWeatherClient::MAIN_TOPIC + "/send/*/" + topic,
    MqttWeatherClient::MAIN_TOPIC + "/send/" + _id + "/*",
    MqttWeatherClient::MAIN_TOPIC + "/send/*/*"
  };

  for (uint8_t i=0; i<topics.size(); i++) {
    if (messageCallbacks.find(topics[i]) == messageCallbacks.end()) {
      messageCallbacks[topics[i]] = std::vector<mqtt_callback_t> { callback };
    } else {
      messageCallbacks[topics[i]].push_back(callback);
    }
  }
}

void MqttWeatherClient::sensorCallback(void (SensorHub::*sensorFunc)(void), char* payload) {
  if (_sensorHub) {
    (_sensorHub->*sensorFunc)();
  }
}

void MqttWeatherClient::mqttConnected(bool sessionPresent) {
  Serial.print("MQTT connected - sessionPresent="); Serial.println(sessionPresent);

  if (_configuration->keyExists(CONFIG_DEVICE_ID)) {
    _id = _configuration->get(CONFIG_DEVICE_ID);
  }

  _mqtt->subscribe((MAIN_TOPIC + "/send/" + _id + "/#").c_str(),2);
  _mqtt->subscribe((MAIN_TOPIC + "/send/*/#").c_str(),2);
  _mqtt->publish((MAIN_TOPIC + "/status/online/" + _id).c_str(), 1, true, String(millis()).c_str());

  messageCallbacks.clear();
  registerCallback("status", [](MqttWeatherClient *client, char *payload) { client->mqttSendStatus(payload); });
  registerCallback("temperature", [](MqttWeatherClient *client, char *payload) { client->sensorCallback(&SensorHub::resetTemperature, payload); });
  registerCallback("luminosity", [](MqttWeatherClient *client, char *payload) { client->sensorCallback(&SensorHub::resetLuminosity, payload); });
  registerCallback("humidity", [](MqttWeatherClient *client, char *payload) { client->sensorCallback(&SensorHub::resetHumidity, payload); });
  registerCallback("pressure", [](MqttWeatherClient *client, char *payload) { client->sensorCallback(&SensorHub::resetPressure, payload); });
  registerCallback("rainlevel", [](MqttWeatherClient *client, char *payload) { client->sensorCallback(&SensorHub::resetRainLevel, payload); });
  registerCallback("speed", [](MqttWeatherClient *client, char *payload) { client->sensorCallback(&SensorHub::resetWindSpeed, payload); });
  registerCallback("wind/direction", [](MqttWeatherClient *client, char *payload) { client->sensorCallback(&SensorHub::resetWindDirection, payload); });
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
  Serial.print("[MQTT] Topic: "); Serial.println(topic);
#ifdef DEBUG
  Serial.print("[MQTT] Payload: "); Serial.println(payload);
#endif

  std::map<String, std::vector<mqtt_callback_t>>::iterator it;
  it = messageCallbacks.find(String(topic));
  if (it != messageCallbacks.end()) {
    for (uint8_t i=0; i<it->second.size(); i++) {
      it->second[i](this, payload);
    }
  } else {
      Serial.println("[MQTT] No message callback handler for topic");
  }
}

void MqttWeatherClient::mqttPublished(uint16_t packetId) {
#ifdef DEBUG
  Serial.println("MQTT message published");
#endif
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

void MqttWeatherClient::setSensors(SensorHub *sensorHub) {
  _sensorHub = sensorHub;
}