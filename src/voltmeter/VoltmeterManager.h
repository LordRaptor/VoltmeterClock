#ifndef VoltmeterManager_h
#define VoltmeterManager_h

#include "Voltmeter.h"
#include "stdint.h"

enum DisplayMode
{
    analog,
    digital

};

class VoltmeterManager
{
private:
    Voltmeter *hoursVoltmeter;
    Voltmeter *minutesVoltmeter;
    Voltmeter *secondsVoltmeter;

    uint8_t currentHours;
    uint8_t currentMinutes;
    uint8_t currentSeconds;

public:
    VoltmeterManager(Voltmeter *hoursVoltmeter, Voltmeter *minutesVoltmeter, Voltmeter *secondsVoltmeter);

    void begin();

    void setDisplayMode(DisplayMode mode);

    void changeDisplayMode();
    void saveCurrentDisplayMode();
    void resetDisplayMode();

    void updateTime(uint8_t hours, uint8_t minutes, uint8_t seconds);

    bool updateVoltmeters();
};

#endif