#pragma once

#include <Basecamp.hpp>
#include <Arduino.h>
#include "SensorHub.hpp"

extern "C" {
  class SensorHub;
  class MqttWeatherClient;
}

typedef void (*mqtt_callback_t)(MqttWeatherClient*, char*);

class MqttWeatherClient {
  private:
    static String getTopicRoot(void);

    void mqttConnected(bool sessionPresent);
    void mqttDisconnected(AsyncMqttClientDisconnectReason reason);
    void mqttSubscribed(uint16_t packetId, uint8_t qos);
    void mqttMessage(char* topic, char* payload,
      AsyncMqttClientMessageProperties properties,
      size_t len, size_t index, size_t total);
    void mqttPublished(uint16_t packetId);

    void registerCallback(String branch, String topic, bool register_wildcard, mqtt_callback_t callback);
    void sensorCallback(void (SensorHub::*sensorFunc)(void), char *callback);

    std::map<String, std::vector<mqtt_callback_t>> messageCallbacks;

    uint8_t _mqtt_failed;
    AsyncMqttClient* _mqtt;

    SensorHub *_sensorHub;
    Configuration *_configuration;

    String _id;

    void mqttSendStatus(char* payload);

  public:
    MqttWeatherClient(AsyncMqttClient *client, Configuration *configuration);
    bool sendMessage(String type, int qos, bool persistent, String payload);
    bool sendMessage(String type, int qos, bool persistent, JsonObject& payload);

    void setLWT(void);
    void setSensors(SensorHub *sensorHub);
    String getOnlineTopic(void);
};
