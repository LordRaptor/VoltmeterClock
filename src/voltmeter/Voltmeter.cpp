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
};

void Voltmeter::setSpeed(int speed)
{
    this->speed = speed;
};

bool Voltmeter::update()
{
    unsigned long now = millis();
    if (currentValue != targetValue)
    {
        currentValue = lerp(currentValue, targetValue, speed, now - lastUpdate);
    }
    lastUpdate = now; //Keep updating this so there is no sudden jumps

    analogWrite(pin, currentValue);
    return currentValue == targetValue;
}

int Voltmeter::getCurrentValue()
{
    return currentValue;
}
