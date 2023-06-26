#ifndef Voltmeter_h
#define Voltmeter_h

#include "Arduino.h"

class Voltmeter
{
  public:
    Voltmeter(int pin, int speed);

    void setTarget(int targetValue);
    void setSpeed(int speed); //Speed in steps/s

    bool update();

  private:
    int pin;
    int speed;
    int currentValue;
    int targetValue;

    unsigned long delayBetweenUpdates;
    unsigned long lastUpdate;

    void updateDelayBetweenUpdates();
};
#endif
