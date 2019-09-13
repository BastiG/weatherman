#ifndef _WEATHERMAN_MQTT_H_
#define _WEATHERMAN_MQTT_H_

#include "main.h"

void mqttConnected(bool sessionPresent);
void mqttDisconnected(AsyncMqttClientDisconnectReason reason);
void mqttSubscribed(uint16_t packetId, uint8_t qos);
void mqttMessage(char* topic, char* payload,
  AsyncMqttClientMessageProperties properties,
  size_t len, size_t index, size_t total);
void mqttPublished(uint16_t packetId);

bool sendMessage(String type, String id, int qos, bool persistent, String payload);

#endif