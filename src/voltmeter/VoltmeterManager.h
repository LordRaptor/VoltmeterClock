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

struct VoltmeterConfig
{
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
    unsigned long currentMillis;

    void setVoltmeterTargets();

    uint8_t hourTargetValues[25] = {
        0, 12, 20, 32, 40, 52, 62,
        72, 81, 92, 102, 113, 122,
        134, 144, 154, 162, 172, 181,
        190, 200, 210, 218, 228, 240};

    uint8_t getHourTarget(float value);

public:
    VoltmeterManager(VoltmeterConfig config);

    void begin();

    void setDisplayMode(DisplayMode mode);

    void changeDisplayMode();
    void saveCurrentDisplayMode();
    void resetDisplayMode();
    void setStepsPerSecond(int stepsPerSecond);

    void updateTime(uint8_t hours, uint8_t minutes, uint8_t seconds, unsigned long millis);

    bool updateVoltmeters();

    void printTargets();
};

#endif