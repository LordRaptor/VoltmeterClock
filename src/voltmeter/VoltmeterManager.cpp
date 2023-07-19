// EEPROM circular buffer configuration
#define BUFFER_START 0x30 // buffer start address
#define BUFFER_LEN 10     // number of data blocks

#define HOURS_INTERVAL (255 / 24.f)
#define MINUTES_INTERVAL (255 / 60.f)
#define SECONDS_INTERVAL (255 / 60.f)

#include "VoltmeterManager.h"
#include "EEPROM.h"
#include "eewl.h"
#include "math.h"

DisplayMode displayMode;

EEWL storedDisplayMode(displayMode, BUFFER_LEN, BUFFER_START);

VoltmeterManager::VoltmeterManager(VoltmeterConfig config)
{
    this->hoursVoltmeter.addPin(config.hoursPin);
    this->hoursVoltmeter.setSpeed(config.stepsPerSecond);

    this->minutesVoltmeter.addPin(config.minutesPin);
    this->minutesVoltmeter.setSpeed(config.stepsPerSecond);

    this->secondsVoltmeter.addPin(config.secondsPin);
    this->secondsVoltmeter.setSpeed(config.stepsPerSecond);
}

void VoltmeterManager::begin()
{
    storedDisplayMode.begin();
    resetDisplayMode();

    this->hoursVoltmeter.begin();
    this->minutesVoltmeter.begin();
    this->secondsVoltmeter.begin();
}

void VoltmeterManager::setDisplayMode(DisplayMode mode)
{
    if (displayMode != mode)
    {
        displayMode = mode;
        Serial.print(F("Set display mode to: "));
        switch (displayMode)
        {
        case analog:
            Serial.println(F("analog"));
            break;
        case digital:
            Serial.println(F("digital"));
            break;
        default:
            Serial.println(displayMode);
            break;
        }

        setVoltmeterTargets();
    }
}

void VoltmeterManager::changeDisplayMode()
{
    switch (displayMode)
    {
    case analog:
        setDisplayMode(digital);
        break;
    case digital:
        setDisplayMode(analog);
        break;
    default:
        break;
    }
}

void VoltmeterManager::saveCurrentDisplayMode()
{
    storedDisplayMode.put(displayMode);
}

void VoltmeterManager::resetDisplayMode()
{
    if (!storedDisplayMode.get(displayMode))
    {
        displayMode = analog;
    }
    setVoltmeterTargets();
}

void VoltmeterManager::setStepsPerSecond(int stepsPerSecond)
{
    hoursVoltmeter.setSpeed(stepsPerSecond);
    minutesVoltmeter.setSpeed(stepsPerSecond);
    secondsVoltmeter.setSpeed(stepsPerSecond);
}

void VoltmeterManager::updateTime(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    if (currentHours != hours || currentMinutes != minutes || currentSeconds != seconds)
    {
        currentHours = hours;
        currentMinutes = minutes;
        currentSeconds = seconds;

        setVoltmeterTargets();
    }
}

void VoltmeterManager::setVoltmeterTargets()
{
    int hoursTarget;
    int minutesTarget;
    int secondsTarget;

    switch (displayMode)
    {
    case analog:
    {
        float floatingMinutes = currentMinutes + (currentSeconds / 60.f);
        float floatingHours = currentHours + (floatingMinutes / 60.f);

        hoursTarget = round(floatingHours * HOURS_INTERVAL);
        minutesTarget = round(floatingMinutes * MINUTES_INTERVAL);
        secondsTarget = round(currentSeconds * SECONDS_INTERVAL);
        break;
    }
    case digital:
    default:
    {
        hoursTarget = round(currentHours * HOURS_INTERVAL);
        minutesTarget = round(currentMinutes * MINUTES_INTERVAL);
        secondsTarget = round(currentSeconds * SECONDS_INTERVAL);
        break;
    }
    }

    hoursVoltmeter.setTarget(constrain(hoursTarget, 0, 255));
    minutesVoltmeter.setTarget(constrain(minutesTarget, 0, 255));
    secondsVoltmeter.setTarget(constrain(secondsTarget, 0, 255));
}

bool VoltmeterManager::updateVoltmeters()
{
    hoursVoltmeter.moveToTarget();
    minutesVoltmeter.moveToTarget();
    secondsVoltmeter.moveToTarget();

    return hoursVoltmeter.reachedTarget() && minutesVoltmeter.reachedTarget() && secondsVoltmeter.reachedTarget();
}
