#include "DeviceStatus.hpp"

DeviceStatus::DeviceStatus(const String &failure_message) {
    _initDone = false;
    _failure_count = 0;
    _signalled = false;
    _failure_message = failure_message;
}

void DeviceStatus::initDone() {
    _initDone = true;
    _failure_count = 0;
}

void DeviceStatus::fail(const String &message) {
    if (isFail()) return;

    fail();
    Serial.println(message);
}

void DeviceStatus::fail() {
    if (isFail()) return;

    _failure_count++;
    _signalled = false;
}

bool DeviceStatus::isFail() {
    return _failure_count > 0;
}

uint16_t DeviceStatus::failureCount() {
    return _failure_count;
}

bool DeviceStatus::isInitDone() {
    return _initDone;
}

void DeviceStatus::signalled() {
    _signalled = true;
}

bool DeviceStatus::shouldSignal() {
    return !_signalled;
}

void DeviceStatus::recover() {
    _signalled = false;
    _failure_count = 0;
}

String DeviceStatus::failureMessage() {
    return _failure_message;
}