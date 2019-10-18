#pragma once

#include <Arduino.h>

extern "C" {
  class MqttWeatherClient;
}

class Logger {
    public:
        Logger(MqttWeatherClient& mqtt);
        void log(String& message);
        void log(const char* message);
    private:
        MqttWeatherClient& _mqtt;
};
