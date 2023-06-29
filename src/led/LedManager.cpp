#include "Arduino.h"
#include "LedManager.h"
#include "EEPROM.h"

LedManager::LedManager(byte pin1, byte pin2, byte pin3)
{
    this->ledCount = 3;
    this->ledPins[0] = pin1;
    this->ledPins[1] = pin2;
    this->ledPins[2] = pin3;
}

void LedManager::begin()
{
    this->ledLevelIndex = EEPROM.read(EEPROM_ADDRESS);
    if (this->ledLevelIndex >= LED_LEVELS_COUNT) {
        this->ledLevelIndex = 0;
    }
    Serial.print(F("Loaded Led level "));
    Serial.println(LED_LEVELS[ledLevelIndex]);
    writeLedOutput();
}

void LedManager::enable()
{
    this->enabled = true;
}

void LedManager::disable()
{
    this->enabled = false;
}

void LedManager::writeLedOutput()
{
    if (enabled)
    {
        for (byte i = 0; i < ledCount; i++)
        {
            analogWrite(ledPins[i], LED_LEVELS[ledLevelIndex]);
        }
    }
}

void LedManager::changeLedLevel()
{
    ledLevelIndex = (ledLevelIndex + 1) % LED_LEVELS_COUNT;
    // EEPROM.write(EEPROM_ADDRESS, ledLevelIndex);
    writeLedOutput();
    Serial.print(F("Increase Led to "));
    Serial.println(LED_LEVELS[ledLevelIndex]);
}