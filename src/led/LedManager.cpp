// EEPROM circular buffer configuration
#define BUFFER_START 0x10 // buffer start address
#define BUFFER_LEN 10     // number of data blocks

#include "Arduino.h"
#include "LedManager.h"
#include "EEPROM.h"
#include "eewl.h"
#include "helper/Lerp.h"

byte ledLevelIndex;

EEWL storedState(ledLevelIndex, BUFFER_LEN, BUFFER_START);

LedManager::LedManager(byte pin1, byte pin2, byte pin3)
{
    this->controller = new PWMController();
    this->controller->addPin(pin1);
    this->controller->addPin(pin2);
    this->controller->addPin(pin3);

}

void LedManager::begin()
{
    storedState.begin();
    restoreLedLevel();
    controller->begin();
    setMode(saved_level);
}

void LedManager::update()
{
    switch (mode)
    {
    case off:
        controller->jumpToTarget();
        break;
    case saved_level:
        controller->jumpToTarget();
        break;
    case blinking:
    {
        unsigned long now = millis();
        if ((now - lastBlink) > blinkInterval)
        {
            lastBlink = now;
            controller->setTarget(~controller->getTarget());
        }
        controller->jumpToTarget();
        break;
    }
    case pulsing:
    {

        if (controller->reachedTarget())
        {
            controller->setTarget(~controller->getTarget());
        }
        controller->moveToTarget(millis());
        break;
    }
    default:
        break;
    }
}

void LedManager::changeLedLevel()
{
    ledLevelIndex = (ledLevelIndex + 1) % LED_LEVELS_COUNT;
    controller->setTarget(LED_LEVELS[ledLevelIndex]);
    Serial.print(F("Change LED brightness to "));
    Serial.println(LED_LEVELS[ledLevelIndex]);
}

void LedManager::saveLedLevel()
{
    storedState.put(ledLevelIndex);
    Serial.print(F("Saved LED brightness "));
    Serial.println(LED_LEVELS[ledLevelIndex]);
}

void LedManager::restoreLedLevel()
{
    if (storedState.get(ledLevelIndex))
    {
        Serial.print(F("Loaded LED brightness "));
        Serial.println(LED_LEVELS[ledLevelIndex]);
        controller->setTarget(LED_LEVELS[ledLevelIndex]);
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
        case saved_level:
            controller->setTarget(LED_LEVELS[ledLevelIndex]);
            break;
        case blinking:
            controller->setTarget(0);
            lastBlink = millis();
            break;
        case pulsing:
            controller->setTarget(0);
            controller->setSpeed(128);
            lastBlink = millis();
            break;
        case off:
            controller->setTarget(0);
            break;
        default:
            break;
        }
    }
}

void LedManager::setBlinkInterval(unsigned long blinkInterval)
{
    this->blinkInterval = blinkInterval;
}