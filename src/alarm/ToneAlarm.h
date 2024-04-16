#ifndef ToneAlarm_h
#define ToneAlarm_h
#include "Arduino.h"
#include "pitches.h" 

class ToneAlarm
{
public:
    ToneAlarm(byte buzzerPin);

    void begin();

    void start(byte count);
    void play();
    void stop();

    void changeMelody();

private:
    byte buzzerPin;

    int currentNote;
    int currentMelodyLength;
    byte playCount;

    unsigned long nextNote;

    int getMelodyLenght();
    uint16_t getNote(int index);
    byte getDuration(int index);
};

#endif