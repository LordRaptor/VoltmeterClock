#include "Arduino.h"
#include "Voltmeter.h"
#include "helper/Lerp.h"

Voltmeter::Voltmeter(int pin, int speed)
{
    this->pin = pin;
    this->speed = speed;
    this->currentValue = 0;
    this->targetValue = 0;
    this->lastUpdate = 0;
};

void Voltmeter::setTarget(int target)
{
    this->targetValue = target % 256;
    this->lastUpdate = millis();
};

void Voltmeter::setSpeed(int speed)
{
    this->speed = speed;
};

bool Voltmeter::update()
{
    if (currentValue != targetValue)
    {
        unsigned long now = millis();
        int newCurrent = lerp(currentValue, targetValue, speed, now - lastUpdate);
        if (newCurrent != currentValue)
        {
            lastUpdate = now;
            analogWrite(pin, currentValue);
        }
        currentValue = newCurrent;
    }

    return currentValue == targetValue;
}

int Voltmeter::getCurrentValue()
{
    return currentValue;
}
