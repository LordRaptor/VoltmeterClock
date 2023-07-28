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
#define VOLTMETER_STEPS_PER_SECOND 255

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

void buttonSingleClickedCallback(void *ref);
void buttonLongPressedCallback(void *ref);

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
  uint8_t hours = 0;
  uint8_t minutes = 0;
  uint8_t seconds = 0;
};
TimeData rtcTimeHolder = {};

struct DisplayTimeStateData
{
  unsigned long lastRTCPoll = 0;
  unsigned long lastSecondChange = 0;
  unsigned long lastSerialOutput = 0;

  unsigned long rtcUpdateInterval = 50;
  unsigned long serialOutputInterval = 10000;
};
DisplayTimeStateData displayStateData = {};

LedManager ledManager(HOURS_LED_PIN, MINUTES_LED_PIN, SECONDS_LED_PIN);

VoltmeterManager voltmeterManager(
    VoltmeterConfig{
        HOURS_VOLTMETER_PIN,
        MINUTES_VOLTMETER_PIN,
        SECONDS_VOLTMETER_PIN,
        VOLTMETER_STEPS_PER_SECOND});

Switch button1 = Switch(BUTTON_1_PIN, INPUT_PULLUP, LOW, 50, 4000, 250, 10);
Switch button2 = Switch(BUTTON_2_PIN, INPUT_PULLUP, LOW, 50, 4000, 250, 10);
Switch button3 = Switch(BUTTON_3_PIN, INPUT_PULLUP, LOW, 50, 1000, 250, 10);

RTC_DS3231 rtc;

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

  button1.setSingleClickCallback(&buttonSingleClickedCallback, (void *)&BUTTON_1_ID);
  button2.setSingleClickCallback(&buttonSingleClickedCallback, (void *)&BUTTON_2_ID);
  button3.setSingleClickCallback(&buttonSingleClickedCallback, (void *)&BUTTON_3_ID);

  button1.setLongPressCallback(&buttonLongPressedCallback, (void *)&BUTTON_1_ID);
  button2.setLongPressCallback(&buttonLongPressedCallback, (void *)&BUTTON_2_ID);
  button3.setLongPressCallback(&buttonLongPressedCallback, (void *)&BUTTON_3_ID);

  ledManager.begin();
  ledManager.update();

  voltmeterManager.begin();

  state = startup;
}

void loop()
{
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

  voltmeterManager.updateVoltmeters();
  ledManager.update();
}

// put function definitions here:
void startupRoutine()
{
  voltmeterManager.setStepsPerSecond(64);
  voltmeterManager.setDisplayMode(digital);
  voltmeterManager.updateTime(24, 60, 60, 0);
  ledManager.setMode(pulsing);

  while (!voltmeterManager.updateVoltmeters())
  {
    ledManager.update();
  }

  voltmeterManager.updateTime(0, 0, 0, 0);
  while (!voltmeterManager.updateVoltmeters())
  {
    ledManager.update();
  }

  voltmeterManager.resetDisplayMode();
  voltmeterManager.setStepsPerSecond(VOLTMETER_STEPS_PER_SECOND);
  ledManager.setMode(saved_level);
  ledManager.restoreLedBrightness();
  ledManager.update();

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
  unsigned long localNow = millis();
  if (displayStateData.rtcUpdateInterval <= (localNow - displayStateData.lastRTCPoll))
  {
    DateTime rtcNow = rtc.now();
    rtcTimeHolder.hours = rtcNow.hour();
    rtcTimeHolder.minutes = rtcNow.minute();
    if (rtcNow.second() != rtcTimeHolder.seconds) {
      rtcTimeHolder.seconds = rtcNow.second();
      displayStateData.lastSecondChange = localNow;      
    }


    voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, localNow - displayStateData.lastSecondChange);
    displayStateData.lastRTCPoll = localNow;
  }
  if (displayStateData.serialOutputInterval <= (localNow - displayStateData.lastSerialOutput))
  {
    Serial.print(rtc.getTemperature());
    Serial.print("°C ");
    writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
    displayStateData.lastSerialOutput = localNow;
  }
}

void enterSettings()
{
  Serial.println(F("Entering settings state"));
  state = setting;

  if (rtc.lostPower())
  {
    rtcTimeHolder.hours = 0;
    rtcTimeHolder.minutes = 0;
    rtcTimeHolder.seconds = 0;
  }
  else
  {
    DateTime dt = rtc.now();
    rtcTimeHolder.hours = dt.hour();
    rtcTimeHolder.minutes = dt.minute();
    rtcTimeHolder.seconds = 0;
  }

  writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);

  voltmeterManager.setDisplayMode(digital);
  voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);

  ledManager.setMode(blinking);
}

void settingStateLoop()
{
  if (button3.pushed())
  {
    exitSettings();
  }
  else if (button1.pushed())
  {
    rtcTimeHolder.hours = (rtcTimeHolder.hours + 1) % 24;
    writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
    voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);
  }
  else if (button2.pushed())
  {
    rtcTimeHolder.minutes = (rtcTimeHolder.minutes + 1) % 60;
    writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
    voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);
  }
}

void exitSettings()
{

  state = displayTime;
  rtc.adjust(DateTime(2023, 1, 1, rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds));

  // Force an update
  displayStateData.lastRTCPoll = 0;
  displayStateData.lastSerialOutput = 0;

  voltmeterManager.resetDisplayMode();

  ledManager.setMode(saved_level);

  Serial.println(F("Exiting settings state"));
}

void enterCalibration()
{
  Serial.println(F("Entering calibration state"));
  voltmeterManager.setDisplayMode(digital);
  rtcTimeHolder.hours = 24;
  rtcTimeHolder.minutes = 60;
  rtcTimeHolder.seconds = 60;

  voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);
  voltmeterManager.printTargets();
  writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
  ledManager.setMode(pulsing);

  state = calibration;
}

void calibrationStateLoop()
{
  // Used to calibrate the voltmeters
  if (button1.singleClick())
  {
    rtcTimeHolder.hours = rtcTimeHolder.hours < 24 ? rtcTimeHolder.hours + 1 : 0;
    voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);
    voltmeterManager.printTargets();
    writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
  }
  else if (button2.singleClick())
  {
    rtcTimeHolder.minutes = rtcTimeHolder.minutes < 60 ? rtcTimeHolder.minutes + 5 : 0;
    voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);
    writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
  }
  else if (button3.singleClick())
  {
    rtcTimeHolder.seconds = rtcTimeHolder.seconds < 60 ? rtcTimeHolder.seconds + 5 : 0;
    voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);
    voltmeterManager.printTargets();
    writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
  }
}

void exitCalibration()
{
  voltmeterManager.resetDisplayMode();
  ledManager.setMode(saved_level);

  // Force an update
  displayStateData.lastRTCPoll = 0;
  displayStateData.lastSerialOutput = 0;

  state = displayTime;
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

void buttonSingleClickedCallback(void *ref)
{
  byte b = *((byte *)ref);

  Serial.print(F("Button clicked: "));
  Serial.println(b);

  if (b == BUTTON_1_ID && state == displayTime)
  {
    // Change LED brightness
    ledManager.changeLedBrightness();
    ledManager.saveLedBrightness();
  }
  else if (b == BUTTON_2_ID && state == displayTime)
  {
    // Change voltmeter display mode
    voltmeterManager.changeDisplayMode();
    voltmeterManager.saveCurrentDisplayMode();
    displayStateData.lastRTCPoll = 0; //Force update
  }
}

void buttonLongPressedCallback(void *ref)
{
  byte b = *((byte *)ref);

  Serial.print(F("Button long pressed: "));
  Serial.println(b);

  if (b == BUTTON_3_ID && state == displayTime)
  {
    enterSettings();
  }
  else if (b == BUTTON_1_ID)
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