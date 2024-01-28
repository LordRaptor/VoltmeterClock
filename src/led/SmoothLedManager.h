#ifndef SmoothLedManager_h
#define SmoothLedManager_h

#include "Arduino.h"
#include "controller/PWMController.h"

enum LedMode {
    saved_level,
    blinking,
    pulsing,
    off
};

class SmoothLedManager {
    public:
    SmoothLedManager(byte pin1, byte pin2, byte pin3);
    SmoothLedManager(byte pin1);

    void begin();

    void update();

    void setMode(LedMode mode);

    void setBlinkInterval(unsigned long speed);

    bool changeLedBrightness(int change);
    void saveLedBrightness();
    void restoreLedBrightness();


    private:

    LedMode mode = off;

    unsigned long blinkInterval = 750;
    unsigned long lastBlink = 0;

    PWMController controller = PWMController();
};

#endif