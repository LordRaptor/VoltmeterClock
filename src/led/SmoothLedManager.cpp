// EEPROM circular buffer configuration
#define BUFFER_START 0x20 // buffer start address
#define BUFFER_LEN 10     // number of data blocks

#include "Arduino.h"
#include "SmoothLedManager.h"
#include "EEPROM.h"
#include "eewl.h"
#include "helper/Lerp.h"

byte level;

EEWL storedStateS(level, BUFFER_LEN, BUFFER_START);

SmoothLedManager::SmoothLedManager(byte pin1, byte pin2, byte pin3)
{
    this->controller.addPin(pin1);
    this->controller.addPin(pin2);
    this->controller.addPin(pin3);

}

SmoothLedManager::SmoothLedManager(byte pin1)
{
    this->controller.addPin(pin1);

}

void SmoothLedManager::begin()
{
    storedStateS.begin();
    restoreLedBrightness();
    controller.begin();
    setMode(saved_level);
}

void SmoothLedManager::update()
{
    switch (mode)
    {
    case off:
        controller.jumpToTarget();
        break;
    case saved_level:
        controller.jumpToTarget();
        break;
    case blinking:
    {
        unsigned long now = millis();
        if ((now - lastBlink) > blinkInterval)
        {
            lastBlink = now;
            controller.setTarget(~controller.getTarget());
        }
        controller.jumpToTarget();
        break;
    }
    case pulsing:
    {

        if (controller.reachedTarget())
        {
            controller.setTarget(~controller.getTarget());
        }
        controller.moveToTarget();
        break;
    }
    default:
        break;
    }
}

bool SmoothLedManager::changeLedBrightness(int change)
{
    byte newLevel = constrain(level + change, 0, 255);
    if (level != newLevel) 
    {
        level = newLevel;
        controller.setTarget(level);
        Serial.print(F("Change LED brightness to "));
        Serial.println(level);
        return true;
    }
    return false;
}

void SmoothLedManager::saveLedBrightness()
{
    storedStateS.put(level);
    Serial.print(F("Saved LED brightness "));
    Serial.println(level);
}

void SmoothLedManager::restoreLedBrightness()
{
    if (storedStateS.get(level))
    {
        Serial.print(F("Loaded LED brightness "));
        Serial.println(level);
        controller.setTarget(level);
    }
}

void SmoothLedManager::setMode(LedMode mode)
{
    if (this->mode != mode)
    {
        // Mode switch
        this->mode = mode;
        switch (this->mode)
        {
        case saved_level:
            controller.setTarget(level);
            break;
        case blinking:
            controller.setTarget(0);
            lastBlink = millis();
            break;
        case pulsing:
            controller.setTarget(0);
            controller.setSpeed(128);
            lastBlink = millis();
            break;
        case off:
            controller.setTarget(0);
            break;
        case on:
            controller.setTarget(255);
            controller.jumpToTarget();
            break;
        default:
            break;
        }
    }
}

void SmoothLedManager::setBlinkInterval(unsigned long blinkInterval)
{
    this->blinkInterval = blinkInterval;
}