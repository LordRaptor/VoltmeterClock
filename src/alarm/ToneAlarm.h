#ifndef ToneAlarm_h
#define ToneAlarm_h
#include "Arduino.h"
#include "pitches.h" 

typedef unsigned long time_ms;
#define MELODY_COUNT 1;

class ToneAlarm
{
public:
    ToneAlarm(byte buzzerPin);

    void begin();

    void start();
    void play();
    void stop();

    void changeMelody();

private:
    byte buzzerPin;

    int currentNote;
    bool playing;

    unsigned long nextNote;
};

#endif