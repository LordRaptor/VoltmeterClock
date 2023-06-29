#include <Arduino.h>
#include <voltmeter/Voltmeter.h>
#include <led/LedManager.h>
#include <RTClib.h>
#include <avdweb_Switch.h>

// put function declarations here:
void startupRoutine();
void normalStateLoop();
void settingStateLoop();
void calibrationStateLoop();
void writeTimetoSerial(uint8_t hours, uint8_t minutes, uint8_t seconds);
void enterSettings();
void exitSettings();
void buttonPushedCallbackFunction(void *ref);
void buttonLongPressedCallbackFunction(void *ref);

const float MAX_HOURS = 24;
const float MAX_MINUTES = 60;
const float MAX_SECONDS = 59;
const int VOLTMETER_STEPS_PER_SECOND = 10;
const unsigned long NORMAL_STATE_DELAY = 5000;
const unsigned long SETTINGS_BLINK_DELAY = 750;

const byte BUTTON_1_ID = 1;
const byte BUTTON_2_ID = 2;
const byte BUTTON_3_ID = 3;

const byte HOURS_LED_PIN = 6;
const byte MINUTES_LED_PIN = 5;
const byte SECONDS_LED_PIN = 3;

// Do not use Pins 5 and 6 for Voltmeters
enum ClockState
{
  startup,
  normal,
  setting,
  calibration
};
ClockState state = normal;

struct SettingsData
{
  byte hours = 0;
  byte minutes = 0;
  byte seconds = 0;
  unsigned long lastBlink = 0;
  byte blinkState = 0;
};
SettingsData settingsData = {};

Voltmeter hoursVoltmeter(11, VOLTMETER_STEPS_PER_SECOND);
Voltmeter minutesVoltmeter(10, VOLTMETER_STEPS_PER_SECOND);
Voltmeter secondsVoltmeter(9, VOLTMETER_STEPS_PER_SECOND);

LedManager ledManager(HOURS_LED_PIN, MINUTES_LED_PIN, SECONDS_LED_PIN);

Switch button1 = Switch(2, INPUT_PULLUP, LOW, 50, 1000, 250, 10);
Switch button2 = Switch(7);
Switch button3 = Switch(4);

RTC_DS3231 rtc;

unsigned long lastDisplayUpdate = 0;

void setup()
{
  Serial.begin(9600);

  if (!rtc.begin())
  {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    while (true)
      ;
  }
  Serial.println(F("Found RTC"));

  rtc.disable32K();
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);
  rtc.disableAlarm(1);
  rtc.disableAlarm(2);

  button1.setPushedCallback(&buttonPushedCallbackFunction, (void *)&BUTTON_1_ID);
  button2.setPushedCallback(&buttonPushedCallbackFunction, (void *)&BUTTON_2_ID);
  button3.setPushedCallback(&buttonPushedCallbackFunction, (void *)&BUTTON_3_ID);

  button1.setLongPressCallback(&buttonLongPressedCallbackFunction, (void *)&BUTTON_1_ID);
  button2.setLongPressCallback(&buttonLongPressedCallbackFunction, (void *)&BUTTON_2_ID);
  button3.setLongPressCallback(&buttonLongPressedCallbackFunction, (void *)&BUTTON_3_ID);

  ledManager.enable();
  ledManager.begin();

    if (rtc.lostPower())
  {
    Serial.println(F("RTC lost power, set the time!"));
    enterSettings();
  }
}

void loop()
{
  // put your main code here, to run repeatedly:
  button1.poll();
  button2.poll();
  button3.poll();

  switch (state)
  {
  case startup:
    startupRoutine();
    break;
  case normal:
    normalStateLoop();
    break;
  case setting:
    settingStateLoop();
    break;
  case calibration:
    calibrationStateLoop();
    break;
  default:
    break;
  }
}

// put function definitions here:
void startupRoutine()
{
  hoursVoltmeter.setTarget(255);
  minutesVoltmeter.setTarget(255);
  secondsVoltmeter.setTarget(255);
  while (!(hoursVoltmeter.update() && minutesVoltmeter.update() && secondsVoltmeter.update()))
    ;

  hoursVoltmeter.setTarget(0);
  minutesVoltmeter.setTarget(0);
  secondsVoltmeter.setTarget(0);
  while (!(hoursVoltmeter.update() && minutesVoltmeter.update() && secondsVoltmeter.update()))
    ;

  state = normal;
}

void normalStateLoop()
{
  if ((millis() - lastDisplayUpdate) < NORMAL_STATE_DELAY)
  {
    return;
  }
  lastDisplayUpdate = millis();

  DateTime now = rtc.now();
  uint8_t hours = now.hour();
  uint8_t minutes = now.minute();
  uint8_t seconds = now.second();

  writeTimetoSerial(hours, minutes, seconds);

  // float floatHours = hours + (minutes / 60);
  // float floatMinutes = minutes + (seconds / 60);

  // hoursVoltmeter.setTarget(floatHours * (255 / MAX_HOURS));
  // minutesVoltmeter.setTarget(floatMinutes * (255 / MAX_MINUTES));
  // secondsVoltmeter.setTarget(seconds * (255 / MAX_SECONDS));

  // hoursVoltmeter.update();
  // minutesVoltmeter.update();
  // secondsVoltmeter.update();
}

void enterSettings()
{
  Serial.println(F("Entering settings state"));
  state = setting;
  DateTime dt = rtc.now();
  settingsData.hours = dt.hour();
  settingsData.minutes = dt.minute();
  settingsData.seconds = 0;
  writeTimetoSerial(settingsData.hours, settingsData.minutes, settingsData.seconds);

  ledManager.disable();
  analogWrite(HOURS_LED_PIN, 0);
  analogWrite(MINUTES_LED_PIN, 0);
  analogWrite(SECONDS_LED_PIN, 0);
}

void settingStateLoop()
{
  if (button1.pushed())
  {
    exitSettings();
  }
  else if (button2.pushed())
  {
    settingsData.hours = (settingsData.hours + 1) % 24;
    writeTimetoSerial(settingsData.hours, settingsData.minutes, settingsData.seconds);
  }
  else if (button3.pushed())
  {
    settingsData.minutes = (settingsData.minutes + 1) % 60;
    writeTimetoSerial(settingsData.hours, settingsData.minutes, settingsData.seconds);
  }

  unsigned long now = millis();
  if (SETTINGS_BLINK_DELAY < (now - settingsData.lastBlink))
  {
    settingsData.lastBlink = now;
    settingsData.blinkState = ~settingsData.blinkState;
    analogWrite(HOURS_LED_PIN, settingsData.blinkState);
    analogWrite(MINUTES_LED_PIN, settingsData.blinkState);
    analogWrite(SECONDS_LED_PIN, settingsData.blinkState);
  }
}

void exitSettings()
{

  state = normal;
  rtc.adjust(DateTime(2023, 1, 1, settingsData.hours, settingsData.minutes, settingsData.seconds));
  lastDisplayUpdate = 0; // Force an update
  ledManager.enable();
  ledManager.writeLedOutput();
  Serial.println(F("Exiting settings state"));
}

void calibrationStateLoop()
{
  // Used to calibrate the voltmeters
}

void writeTimetoSerial(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
  if (hours < 10)
  {
    Serial.print("0");
  }
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10)
  {
    Serial.print("0");
  }
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10)
  {
    Serial.print("0");
  }
  Serial.println(seconds);
}

void buttonPushedCallbackFunction(void *ref)
{
  byte b = *((byte *)ref);

  Serial.print("Button pushed: ");
  Serial.println(b);

  if (b == BUTTON_2_ID && state == normal)
  {
    // Change LED state
    ledManager.changeLedLevel();
  } else if (b == BUTTON_3_ID && state == normal) {
    
  }
}

void buttonLongPressedCallbackFunction(void *ref)
{
  byte b = *((byte *)ref);

  Serial.print("Button long pressed: ");
  Serial.println(b);

  if (b == BUTTON_1_ID && state == normal)
  {
    // Enter settings
    enterSettings();
  }
}