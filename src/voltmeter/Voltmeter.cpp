#include "Arduino.h"
#include "Voltmeter.h"

Voltmeter::Voltmeter(int pin, int speed)
{
    this->pin = pin;
    this->speed = speed;
    this->currentValue = 0;
    this->targetValue = 0;
    this->lastUpdate = 0;

    this->updateDelayBetweenUpdates();
};

void Voltmeter::setTarget(int target)
{
    this->targetValue = target % 256;
};

void Voltmeter::setSpeed(int speed)
{
    this->speed = speed;
    this->updateDelayBetweenUpdates();
};

bool Voltmeter::update()
{
    if (currentValue != targetValue)
    {
        if ((millis() - lastUpdate) > delayBetweenUpdates)
        {
            // Update
            lastUpdate = millis();

            if (currentValue < targetValue)
            {
                currentValue++;
            }
            else if (currentValue > targetValue)
            {
                currentValue--;
            }
        }
    }

    analogWrite(pin, currentValue);
    return currentValue == targetValue;
}

void Voltmeter::updateDelayBetweenUpdates()
{
    this->delayBetweenUpdates = 1000 / speed;
}
