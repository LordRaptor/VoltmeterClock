#include <Arduino.h>
#include <voltmeter/VoltmeterManager.h>
#include <led/LedManager.h>
#include <RTClib.h>
#include <avdweb_Switch.h>

// Voltmeters
// Do not use Pins 5 and 6 for Voltmeters
#define HOURS_VOLTMETER_PIN 11
#define MINUTES_VOLTMETER_PIN 10
#define SECONDS_VOLTMETER_PIN 9
#define VOLTMETER_STEPS_PER_SECOND 64

// LEDs
#define HOURS_LED_PIN 6
#define MINUTES_LED_PIN 5
#define SECONDS_LED_PIN 3

// Buttons
#define BUTTON_1_PIN 7
#define BUTTON_2_PIN 8
#define BUTTON_3_PIN 12

// put function declarations here:
void startupRoutine();
void displayTimeLoop();

void writeTimetoSerial(uint8_t hours, uint8_t minutes, uint8_t seconds);

void enterSettings();
void settingStateLoop();
void exitSettings();

void enterCalibration();
void calibrationStateLoop();
void exitCalibration();

void buttonPushedCallbackFunction(void *ref);
void buttonLongPressedCallbackFunction(void *ref);

const unsigned long NORMAL_STATE_DELAY = 5000;

const byte BUTTON_1_ID = 1;
const byte BUTTON_2_ID = 2;
const byte BUTTON_3_ID = 3;

enum ClockState
{
  startup,
  displayTime,
  setting,
  calibration
};
ClockState state = startup;

struct TimeData
{
  byte hours = 0;
  byte minutes = 0;
  byte seconds = 0;
};
TimeData tmpTime = {};

LedManager ledManager(HOURS_LED_PIN, MINUTES_LED_PIN, SECONDS_LED_PIN);

VoltmeterManager voltmeterManager(
    new Voltmeter(HOURS_VOLTMETER_PIN, VOLTMETER_STEPS_PER_SECOND),
    new Voltmeter(MINUTES_VOLTMETER_PIN, VOLTMETER_STEPS_PER_SECOND),
    new Voltmeter(SECONDS_VOLTMETER_PIN, VOLTMETER_STEPS_PER_SECOND));

Switch button1 = Switch(BUTTON_1_PIN, INPUT_PULLUP, LOW, 50, 1000, 250, 10);
Switch button2 = Switch(BUTTON_2_PIN);
Switch button3 = Switch(BUTTON_3_PIN, INPUT_PULLUP, LOW, 50, 4000, 250, 10);

RTC_DS3231 rtc;

unsigned long lastDisplayUpdate = 0;

void setup()
{
  Serial.begin(115200);

  if (!rtc.begin())
  {
    Serial.println(F("Couldn't find RTC"));
    Serial.flush();
    while (true)
      ;
    // TODO: Enter an error state
  }
  Serial.println(F("Found RTC"));

  rtc.disable32K();
  rtc.disableAlarm(1);
  rtc.disableAlarm(2);
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  button1.setPushedCallback(&buttonPushedCallbackFunction, (void *)&BUTTON_1_ID);
  button2.setPushedCallback(&buttonPushedCallbackFunction, (void *)&BUTTON_2_ID);
  button3.setPushedCallback(&buttonPushedCallbackFunction, (void *)&BUTTON_3_ID);

  button1.setLongPressCallback(&buttonLongPressedCallbackFunction, (void *)&BUTTON_1_ID);
  button2.setLongPressCallback(&buttonLongPressedCallbackFunction, (void *)&BUTTON_2_ID);
  button3.setLongPressCallback(&buttonLongPressedCallbackFunction, (void *)&BUTTON_3_ID);

  ledManager.begin();
  ledManager.update();

  voltmeterManager.begin();

  state = displayTime;
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
  case displayTime:
    displayTimeLoop();
    voltmeterManager.updateVoltmeters();
    break;
  case setting:
    settingStateLoop();
    voltmeterManager.updateVoltmeters();
    break;
  case calibration:
    calibrationStateLoop();
    break;
  default:
    break;
  }

  ledManager.update();
}

// put function definitions here:
void startupRoutine()
{
  voltmeterManager.setDisplayMode(analog);
  voltmeterManager.updateTime(24, 60, 60);

  while (!voltmeterManager.updateVoltmeters())
    ;

  voltmeterManager.updateTime(0, 0, 0);
  while (!voltmeterManager.updateVoltmeters())
    ;

  voltmeterManager.resetDisplayMode();

  Serial.println(F("Startup completed"));
  if (rtc.lostPower())
  {
    Serial.println(F("RTC lost power"));
    enterSettings();
  }
  else
  {
    Serial.println(F("Switching to time display mode"));
    state = displayTime;
  }
}

void displayTimeLoop()
{

  // TODO: Remove/refactor delay
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
  voltmeterManager.updateTime(hours, minutes, seconds);
}

void enterSettings()
{
  Serial.println(F("Entering settings state"));
  state = setting;

  DateTime dt = rtc.now();
  tmpTime.hours = dt.hour();
  tmpTime.minutes = dt.minute();
  tmpTime.seconds = 0;

  writeTimetoSerial(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);

  voltmeterManager.setDisplayMode(digital);
  voltmeterManager.updateTime(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);

  ledManager.setMode(blinking);
}

void settingStateLoop()
{
  if (button1.pushed())
  {
    exitSettings();
  }
  else if (button2.pushed())
  {
    tmpTime.hours = (tmpTime.hours + 1) % 24;
    writeTimetoSerial(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);
    voltmeterManager.updateTime(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);
  }
  else if (button3.pushed())
  {
    tmpTime.minutes = (tmpTime.minutes + 1) % 60;
    writeTimetoSerial(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);
    voltmeterManager.updateTime(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);
  }
}

void exitSettings()
{

  state = displayTime;
  rtc.adjust(DateTime(2023, 1, 1, tmpTime.hours, tmpTime.minutes, tmpTime.seconds));
  lastDisplayUpdate = 0; // Force an update

  voltmeterManager.resetDisplayMode();

  ledManager.setMode(saved_level);

  Serial.println(F("Exiting settings state"));
}

void enterCalibration()
{
  Serial.println(F("Entering calibration state"));
  voltmeterManager.setDisplayMode(digital);
  tmpTime.hours = 24;
  tmpTime.minutes = 60;
  tmpTime.seconds = 60;

  voltmeterManager.updateTime(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);
  ledManager.setMode(pulsing);

//  while (!voltmeterManager.updateVoltmeters())
//    ;

  state = calibration;
}

void calibrationStateLoop()
{
  // Used to calibrate the voltmeters
  if (button1.pushed())
  {
    tmpTime.hours = (tmpTime.hours + 1) % 25;
    voltmeterManager.updateTime(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);
    writeTimetoSerial(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);
  }
  else if (button2.pushed())
  {
    tmpTime.minutes = (tmpTime.minutes + 1) % 60;
    voltmeterManager.updateTime(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);
    writeTimetoSerial(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);
  }
  else if (button3.pushed())
  {
    tmpTime.seconds = (tmpTime.seconds + 1) % 60;
    voltmeterManager.updateTime(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);
    writeTimetoSerial(tmpTime.hours, tmpTime.minutes, tmpTime.seconds);
  }
}

void exitCalibration()
{
  voltmeterManager.resetDisplayMode();
  lastDisplayUpdate = 0; // Force an update
  ledManager.setMode(saved_level);
  state = startup;
  Serial.println(F("Exiting calibration state"));
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

  if (b == BUTTON_2_ID && state == displayTime)
  {
    // Change LED state
    ledManager.changeLedLevel();
    ledManager.saveLedLevel();
  }
  else if (b == BUTTON_3_ID && state == displayTime)
  {
    voltmeterManager.changeDisplayMode();
    voltmeterManager.saveCurrentDisplayMode();
  }
}

void buttonLongPressedCallbackFunction(void *ref)
{
  byte b = *((byte *)ref);

  Serial.print("Button long pressed: ");
  Serial.println(b);

  if (b == BUTTON_1_ID && state == displayTime)
  {
    enterSettings();
  }
  else if (b == BUTTON_3_ID)
  {
    if (state == displayTime)
    {
      enterCalibration();
    }
    else if (state == calibration)
    {
      exitCalibration();
    }
  }
}