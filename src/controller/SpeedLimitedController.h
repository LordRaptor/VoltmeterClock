#ifndef SpeedLimitedController_h
#define SpeedLimitedController_h

#include "Arduino.h"

typedef void (*controller_callback)(void *, int);
typedef unsigned long time_ms;

class SpeedLimitedController
{
public:
    SpeedLimitedController(controller_callback callback, void *callbackInstance, int unitsPerSecond);

    void begin();

    void setSpeed(int unitsPerSecond);

    void setTarget(byte target);
    byte getTarget();
    byte getCurrent();

    bool moveToTarget(time_ms now);

    bool reachedTarget();

    void jumpToTarget();

private:
    controller_callback callback;
    void *callbackInstance;
    float unitsPerMs;
    byte targetValue;
    byte currentValue;
    unsigned long lastUpdate;
};

#endif