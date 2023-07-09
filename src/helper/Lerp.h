#ifndef Lerp_h
#define Lerp_h

#include "math.h"
#include "Arduino.h"

int lerp(int current, int target, unsigned long unitsPerSecond, unsigned long timePassedInMillis);

#endif