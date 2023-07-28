// EEPROM circular buffer configuration
#define BUFFER_START 0x30 // buffer start address
#define BUFFER_LEN 10     // number of data blocks

#define HOURS_INTERVAL (240 / 24.f)
#define MINUTES_INTERVAL (240 / 60.f)
#define SECONDS_INTERVAL (240 / 60.f)

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

void VoltmeterManager::updateTime(uint8_t hours, uint8_t minutes, uint8_t seconds, unsigned long millis)
{
    hours = constrain(hours, 0, 24);
    minutes = constrain(minutes, 0, 60);
    seconds = constrain(seconds, 0, 60);
    millis = constrain(millis, 0L, 1000L);

    if (currentHours != hours || currentMinutes != minutes || currentSeconds != seconds || (displayMode == analog && currentMillis != millis))
    {
        currentHours = hours;
        currentMinutes = minutes;
        currentSeconds = seconds;
        currentMillis = millis;

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
        float floatingSeconds = currentSeconds + (currentMillis / 1000.f);
        float floatingMinutes = currentMinutes + (floatingSeconds / 60.f);
        float floatingHours = currentHours + (floatingMinutes / 60.f);

        hoursTarget = getHourTarget(floatingHours);
        minutesTarget = round(floatingMinutes * MINUTES_INTERVAL);
        secondsTarget = round(floatingSeconds * SECONDS_INTERVAL);
        break;
    }
    default:
    {
        hoursTarget = getHourTarget(currentHours);
        minutesTarget = round(currentMinutes * MINUTES_INTERVAL);
        secondsTarget = round(currentSeconds * SECONDS_INTERVAL);
        break;
    }
    }

    hoursVoltmeter.setTarget(constrain(hoursTarget, 0, 255));
    minutesVoltmeter.setTarget(constrain(minutesTarget, 0, 255));
    secondsVoltmeter.setTarget(constrain(secondsTarget, 0, 255));
}

uint8_t VoltmeterManager::getHourTarget(float value) {
    long b = round(floor(value));

    if (b >= 24) {
        return hourTargetValues[b];
    }

    float t = value - b;

    return round(hourTargetValues[b] + t * (hourTargetValues[b+1] - hourTargetValues[b]));
}

bool VoltmeterManager::updateVoltmeters()
{
    hoursVoltmeter.moveToTarget();
    minutesVoltmeter.moveToTarget();
    secondsVoltmeter.moveToTarget();

    return hoursVoltmeter.reachedTarget() && minutesVoltmeter.reachedTarget() && secondsVoltmeter.reachedTarget();
}

void VoltmeterManager::printTargets()
{
    Serial.print(hoursVoltmeter.getTarget());
    Serial.print(F(":"));
    Serial.print(minutesVoltmeter.getTarget());
    Serial.print(F(":"));
    Serial.print(secondsVoltmeter.getTarget());
    Serial.print(F("  "));
}
