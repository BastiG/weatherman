#include "Logger.hpp"

#include "MqttWeatherClient.hpp"

Logger::Logger(MqttWeatherClient& mqtt)
    : _mqtt(mqtt) { }

void Logger::log(String& message) {

#ifdef DEBUG
    Serial.println(message);
#else
    _mqtt.sendMessage("log", 0, false, message);
#endif

}

void Logger::log(const char* message) {
    String string(message);
    log(string);
}