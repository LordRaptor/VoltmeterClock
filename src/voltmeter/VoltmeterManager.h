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
        0, 10, 20, 30, 40, 50, 60,
        70, 80, 90, 100, 110, 120,
        130, 140, 150, 160, 170, 180,
        188, 198, 207, 218, 227, 240};

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