#ifndef VoltmeterManager_h
#define VoltmeterManager_h

#include "Arduino.h"
#include "stdint.h"
#include "controller/PWMController.h"

enum DisplayMode
{
    analog,
    digital

};

struct VoltmeterConfig {
    byte hoursPin;
    byte minutesPin;
    byte secondsPin;
    int changeRate;
};

class VoltmeterManager
{
private:
    PWMController *hoursVoltmeter;
    PWMController *minutesVoltmeter;
    PWMController *secondsVoltmeter;

    uint8_t currentHours;
    uint8_t currentMinutes;
    uint8_t currentSeconds;

public:
    VoltmeterManager(VoltmeterConfig config);

    void begin();

    void setDisplayMode(DisplayMode mode);

    void changeDisplayMode();
    void saveCurrentDisplayMode();
    void resetDisplayMode();

    void updateTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

    bool updateVoltmeters();
};

#endif