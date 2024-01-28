#include "ToneAlarm.h"
#include "Arduino.h"
#include "EEPROM.h"
#include "eewl.h"

// EEPROM circular buffer configuration
#define BUFFER_START 0x30 // buffer start address
#define BUFFER_LEN 10     // number of data blocks

byte melodyIndex;

EEWL storedMelody(melodyIndex, BUFFER_LEN, BUFFER_START);

int melody[] = {
  NOTE_AS4, NOTE_AS4, NOTE_AS4,
  NOTE_F5, NOTE_C6,
  NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,
  NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,
  NOTE_AS5, NOTE_A5, NOTE_AS5, NOTE_G5, NOTE_C5, NOTE_C5, NOTE_C5,
  NOTE_F5, NOTE_C6,
  NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,

  NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F6, NOTE_C6,
  NOTE_AS5, NOTE_A5, NOTE_AS5, NOTE_G5, NOTE_C5, NOTE_C5,
  NOTE_D5, NOTE_D5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F5,
  NOTE_F5, NOTE_G5, NOTE_A5, NOTE_G5, NOTE_D5, NOTE_E5, NOTE_C5, NOTE_C5,
  NOTE_D5, NOTE_D5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F5,

  NOTE_C6, NOTE_G5, NOTE_G5, REST, NOTE_C5,
  NOTE_D5, NOTE_D5, NOTE_AS5, NOTE_A5, NOTE_G5, NOTE_F5,
  NOTE_F5, NOTE_G5, NOTE_A5, NOTE_G5, NOTE_D5, NOTE_E5, NOTE_C6, NOTE_C6,
  NOTE_F6, NOTE_DS6, NOTE_CS6, NOTE_C6, NOTE_AS5, NOTE_GS5, NOTE_G5, NOTE_F5,
  NOTE_C6
};

int durations[] = {
  8, 8, 8,
  2, 2,
  8, 8, 8, 2, 4,
  8, 8, 8, 2, 4,
  8, 8, 8, 2, 8, 8, 8,
  2, 2,
  8, 8, 8, 2, 4,

  8, 8, 8, 2, 4,
  8, 8, 8, 2, 8, 16,
  4, 8, 8, 8, 8, 8,
  8, 8, 8, 4, 8, 4, 8, 16,
  4, 8, 8, 8, 8, 8,

  8, 16, 2, 8, 8,
  4, 8, 8, 8, 8, 8,
  8, 8, 8, 4, 8, 4, 8, 16,
  4, 8, 4, 8, 4, 8, 4, 8,
  1
};

ToneAlarm::ToneAlarm(byte buzzerPin)
{
    this->buzzerPin = buzzerPin;
};

void ToneAlarm::begin()
{
    pinMode(buzzerPin, OUTPUT);
    noTone(buzzerPin);
    storedMelody.begin();

    if (storedMelody.get(melodyIndex))
    {
        melodyIndex = melodyIndex % MELODY_COUNT;
        Serial.print(F("Loaded melody index "));
        Serial.println(melodyIndex);
    }
}

void ToneAlarm::start()
{
    playing = true;
    currentNote = 0;
    nextNote = 0;
    Serial.println(F("Start alarm tone"));
}

void ToneAlarm::play()
{
    if (!playing)
    {
        return;
    }

    int size = sizeof(durations) / sizeof(int);

    if (nextNote < millis())
    {
        currentNote++;

        if (currentNote >= size) {
            currentNote = 0;
            nextNote = millis() + 3000;
            noTone(buzzerPin);
        } else {
            int duration = 1000 / durations[currentNote];
            noTone(buzzerPin);
            tone(buzzerPin, melody[currentNote], duration);
            nextNote = millis() + duration * 1.3;
        }
  }
}

void ToneAlarm::stop() {
    if (playing) {
        playing = false;
        noTone(buzzerPin);
        Serial.println(F("Stop alarm tone"));
    }
}

void ToneAlarm::changeMelody()
{
    melodyIndex = (melodyIndex + 1) % MELODY_COUNT;
    storedMelody.put(melodyIndex);
    Serial.print(F("Change alarm melody to "));
    Serial.println(melodyIndex);
}
