#ifndef LedManager_h
#define LedManager_h

#include "Arduino.h"

enum LedMode {
    constant,
    blinking,
    pulsing
};

class LedManager {
    public:
    LedManager(byte pin1, byte pin2, byte pin3);

    void begin();
    void enable();
    void disable();
    void writeLedOutput();
    void changeLedLevel();
    void saveLedLevel();
    void restoreLedLevel();

    private:
    byte ledPins[3];
    int ledCount;
    bool enabled;

    const byte LED_LEVELS_COUNT = 5;
    const byte LED_LEVELS[5] = {0, 64, 128, 192, 255};
};

#endif