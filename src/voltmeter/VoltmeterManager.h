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
    int stepsPerSecond;
};

class VoltmeterManager
{
private:
    PWMController hoursVoltmeter = PWMController();
    PWMController minutesVoltmeter = PWMController();
    PWMController secondsVoltmeter = PWMController();

    uint8_t currentHours;
    uint8_t currentMinutes;
    uint8_t currentSeconds;

    void setVoltmeterTargets();

public:
    VoltmeterManager(VoltmeterConfig config);

    void begin();

    void setDisplayMode(DisplayMode mode);

    void changeDisplayMode();
    void saveCurrentDisplayMode();
    void resetDisplayMode();
    void setStepsPerSecond(int stepsPerSecond);

    void updateTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

    bool updateVoltmeters();
};

#endif