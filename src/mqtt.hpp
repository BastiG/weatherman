#ifndef _WEATHERMAN_MQTT_HPP_
#define _WEATHERMAN_MQTT_HPP_

#include <Basecamp.hpp>
#include <Arduino.h>

class MqttWeatherClient {
  private:
    static const String MAIN_TOPIC;

    void mqttConnected(bool sessionPresent);
    void mqttDisconnected(AsyncMqttClientDisconnectReason reason);
    void mqttSubscribed(uint16_t packetId, uint8_t qos);
    void mqttMessage(char* topic, char* payload,
      AsyncMqttClientMessageProperties properties,
      size_t len, size_t index, size_t total);
    void mqttPublished(uint16_t packetId);

    std::map<String, std::vector<void (MqttWeatherClient::*)(char*)>> messageCallbacks;

    uint8_t _mqtt_failed;
    AsyncMqttClient* _mqtt;
    String _id;

    void mqttSendStatus(char* payload);

  public:
    MqttWeatherClient(AsyncMqttClient* client, Configuration* configuration);
    void setDeviceId(String id);
    bool sendMessage(String type, int qos, bool persistent, String payload);
    String getId(void);
};

#endif