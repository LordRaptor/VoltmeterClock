// EEPROM circular buffer configuration
#define BUFFER_START 0x10 // buffer start address
#define BUFFER_LEN 10     // number of data blocks

#include "Arduino.h"
#include "LedManager.h"
#include "EEPROM.h"
#include "eewl.h"

byte ledLevelIndex;

EEWL storedState(ledLevelIndex, BUFFER_LEN, BUFFER_START);

LedManager::LedManager(byte pin1, byte pin2, byte pin3)
{
    this->ledCount = 3;
    this->ledPins[0] = pin1;
    this->ledPins[1] = pin2;
    this->ledPins[2] = pin3;
}

void LedManager::begin()
{
    storedState.begin();
    restoreLedLevel();
    setMode(constant);

    writeLedOutput();
}

void LedManager::update()
{
    switch (mode)
    {
    case blinking:
    {
        unsigned now = millis();
        if ((now - lastBlink) > blinkSpeed)
        {
            lastBlink = now;
            blinkState = ~blinkState;

            for (byte i = 0; i < ledCount; i++)
            {
                targetLetBrightness[i] = blinkState;
            }
        }
    }
    default:
        break;
    }
    writeLedOutput();
}

void LedManager::changeLedLevel()
{
    ledLevelIndex = (ledLevelIndex + 1) % LED_LEVELS_COUNT;
    writeLedOutput();
    Serial.print(F("Increase Led to "));
    Serial.println(LED_LEVELS[ledLevelIndex]);
}

void LedManager::saveLedLevel()
{
    storedState.put(ledLevelIndex);
    Serial.print(F("Saved Led level "));
    Serial.println(LED_LEVELS[ledLevelIndex]);
}

void LedManager::restoreLedLevel()
{
    if (storedState.get(ledLevelIndex))
    {
        Serial.print(F("Loaded Led level "));
        Serial.println(LED_LEVELS[ledLevelIndex]);
    }
}

void LedManager::setMode(LedMode mode)
{
    if (this->mode != mode)
    {
        // Mode switch
        this->mode = mode;
        switch (this->mode)
        {
        case constant:
            for (byte i = 0; i < ledCount; i++)
            {
                targetLetBrightness[i] = LED_LEVELS[ledLevelIndex];
            }
            break;
        case custom:
            for (byte i = 0; i < ledCount; i++)
            {
                targetLetBrightness[i] = 0;
            }
            break;
        case blinking:
            for (byte i = 0; i < ledCount; i++)
            {
                targetLetBrightness[i] = 0;
            }
            blinkState = 0;
            lastBlink = millis();
        default:
            break;
        }
    }
}

void LedManager::setBlinkSpeed(unsigned long blinkSpeed) {
    this->blinkSpeed = blinkSpeed;
}

void LedManager::setLedBrightness(byte led, byte level)
{
    if (mode == custom && led < ledCount)
    {
        targetLetBrightness[led] = level;
    }
}

void LedManager::writeLedOutput()
{
    for (byte i = 0; i < ledCount; i++)
    {
        if (currentLedBrightness[i] != targetLetBrightness[i])
        {
            analogWrite(ledPins[i], targetLetBrightness[i]);
            currentLedBrightness[i] = targetLetBrightness[i];
        }
    }
}