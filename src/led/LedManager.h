#ifndef LedManager_h
#define LedManager_h

#include "Arduino.h"

enum LedMode {
    constant,
    blinking,
    pulsing,
    custom
};

class LedManager {
    public:
    LedManager(byte pin1, byte pin2, byte pin3);

    void begin();

    void update();

    void setMode(LedMode mode);

    void setBlinkSpeed(unsigned long speed);

    void changeLedLevel();
    void saveLedLevel();
    void restoreLedLevel();

    void setLedBrightness(byte led, byte level); //Only in custom mode

    private:
    void writeLedOutput();

    byte ledPins[3];
    int ledCount;

    const byte LED_LEVELS_COUNT = 5;
    const byte LED_LEVELS[5] = {0, 64, 128, 192, 255};

    LedMode mode = constant;
    int currentLedBrightness[3] = {0,0,0};
    int targetLetBrightness[3] = {0,0,0};

    unsigned long blinkSpeed = 750;
    unsigned long lastBlink = 0;
    byte blinkState = 0;
};

#endif