#ifndef _DEVICESTATUS_HPP_
#define _DEVICESTATUS_HPP_

#include <Arduino.h>

class DeviceStatus {
    private:
        bool _initDone;
        uint16_t _failure_count;
        bool _signalled;
        String _failure_message;
    
    public:
        DeviceStatus(const String &failure_message);
        void initDone();
        void fail();
        void fail(const String &message);
        bool isInitDone();
        bool isFail();
        uint16_t failureCount();
        void signalled();
        bool shouldSignal();
        void recover();
        String failureMessage();
};

#endif