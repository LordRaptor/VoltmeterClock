#include "SpeedLimitedController.h"
#include "Arduino.h"

SpeedLimitedController::SpeedLimitedController(controller_callback callback, void *callbackInstance, int unitsPerSecond)
{
    this->callback = callback;
    this->callbackInstance = callbackInstance;
    this->setSpeed(unitsPerSecond);
};

void SpeedLimitedController::begin() {
    currentValue = 0;
    targetValue = 0;
    callback(callbackInstance, currentValue);
};

void SpeedLimitedController::setSpeed(int unitsPerSecond)
{
    this->unitsPerMs = abs(unitsPerSecond) / 1000.f;
    Serial.print("Setting speed to ");
    Serial.println(this->unitsPerMs);
};

void SpeedLimitedController::setTarget(byte target)
{
    this->targetValue = target;
    this->lastUpdate = millis(); // To avoid jumps when setting a new target
    Serial.print("Setting new target ");
    Serial.println(this->targetValue);
};

byte SpeedLimitedController::getTarget()
{
    return targetValue;
}

byte SpeedLimitedController::getCurrent() {
    return currentValue;
}

bool SpeedLimitedController::reachedTarget()
{
    return currentValue == targetValue;
};

bool SpeedLimitedController::moveToTarget(time_ms now)
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
                currentValue = constrain(currentValue + changeInValue, currentValue, targetValue);
            }
            else
            {
                currentValue = constrain(currentValue - changeInValue, targetValue, currentValue);
            }

            callback(callbackInstance, currentValue);

            this->lastUpdate = now;
            return currentValue == targetValue;
        }
    }
    return false;
};

void SpeedLimitedController::jumpToTarget()
{
    if (currentValue != targetValue)
    {
        currentValue = targetValue;
        callback(callbackInstance, targetValue);
    }
};