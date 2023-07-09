#ifndef LedManager_h
#define LedManager_h

#include "Arduino.h"
#include "controller/PWMController.h"

enum LedMode {
    saved_level,
    blinking,
    pulsing,
    off
};

class LedManager {
    public:
    LedManager(byte pin1, byte pin2, byte pin3);

    void begin();

    void update();

    void setMode(LedMode mode);

    void setBlinkInterval(unsigned long speed);

    void changeLedLevel();
    void saveLedLevel();
    void restoreLedLevel();


    private:
    const byte LED_LEVELS_COUNT = 5;
    const byte LED_LEVELS[5] = {0, 64, 128, 192, 255};

    LedMode mode = off;

    unsigned long blinkInterval = 750;
    unsigned long lastBlink = 0;

    PWMController* controller;
};

#endif