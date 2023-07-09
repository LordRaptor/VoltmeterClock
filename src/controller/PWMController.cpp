#include "PWMController.h"
#include "Arduino.h"

PWMController::PWMController()
{
    this->setSpeed(DEFAULT_SPEED);
};

bool PWMController::addPin(byte pin)
{
    if (enabledPins < MAX_PWN_PINS)
    {
        pwmPins[enabledPins] = pin;
        enabledPins++;
        return true;
    }
    return false;
};

void PWMController::begin()
{
    currentValue = 0;
    targetValue = 0;
    writeOutput();
};

void PWMController::setSpeed(int unitsPerSecond)
{
    this->unitsPerMs = abs(unitsPerSecond) / 1000.f;
};

void PWMController::setTarget(byte target)
{
    this->targetValue = target;
    this->lastUpdate = millis(); // To avoid jumps when setting a new target
};

byte PWMController::getTarget()
{
    return targetValue;
}

byte PWMController::getCurrent()
{
    return currentValue;
}

bool PWMController::reachedTarget()
{
    return currentValue == targetValue;
};

bool PWMController::moveToTarget(time_ms now)
{
    if (currentValue == targetValue)
    {
        return true;
    }

    time_ms timeSinceLastUpdate = now - this->lastUpdate;

    if (timeSinceLastUpdate > 0)
    {
        int changeInValue = round(unitsPerMs * timeSinceLastUpdate);
        if (changeInValue > 0)
        {
            if (currentValue < targetValue)
            {
                currentValue = min(currentValue + changeInValue, targetValue);
            }
            else
            {
                currentValue = max(currentValue - changeInValue, targetValue);
            }

            writeOutput();

            this->lastUpdate = now;
            return currentValue == targetValue;
        }
    }
    return false;
};

void PWMController::jumpToTarget()
{
    if (currentValue != targetValue)
    {
        currentValue = targetValue;
        writeOutput();
    }
};

void PWMController::writeOutput()
{
    for (byte i = 0; i < enabledPins; i++)
    {
        analogWrite(pwmPins[i], currentValue);
    }
}