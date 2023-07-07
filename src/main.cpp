#include <Arduino.h>
#include <voltmeter/VoltmeterManager.h>
#include <led/LedManager.h>
#include <RTClib.h>
#include <avdweb_Switch.h>

// Voltmeters
#define HOURS_VOLTMETER_PIN 11
#define MINUTES_VOLTMETER_PIN 10
#define SECONDS_VOLTMETER_PIN 9
#define VOLTMETER_STEPS_PER_SECOND 10

//LEDs
#define HOURS_LED_PIN 6
#define MINUTES_LED_PIN 5
#define SECONDS_LED_PIN 3

//Buttons
#define BUTTON_1_PIN 2
#define BUTTON_2_PIN 7
#define BUTTON_3_PIN 4

// put function declarations here:
void startupRoutine();
void displayTimeLoop();
void settingStateLoop();
void calibrationStateLoop();
void writeTimetoSerial(uint8_t hours, uint8_t minutes, uint8_t seconds);
void enterSettings();
void exitSettings();
void buttonPushedCallbackFunction(void *ref);
void buttonLongPressedCallbackFunction(void *ref);

const unsigned long NORMAL_STATE_DELAY = 5000;

const byte BUTTON_1_ID = 1;
const byte BUTTON_2_ID = 2;
const byte BUTTON_3_ID = 3;


// Do not use Pins 5 and 6 for Voltmeters
enum ClockState
{
  startup,
  displayTime,
  setting,
  calibration
};
ClockState state = startup;

struct SettingsData
{
  byte hours = 0;
  byte minutes = 0;
  byte seconds = 0;
  unsigned long lastBlink = 0;
  byte blinkState = 0;
};
SettingsData settingsData = {};

LedManager ledManager(HOURS_LED_PIN, MINUTES_LED_PIN, SECONDS_LED_PIN);

VoltmeterManager voltmeterManager(
    new Voltmeter(HOURS_VOLTMETER_PIN, VOLTMETER_STEPS_PER_SECOND),
    new Voltmeter(MINUTES_VOLTMETER_PIN, VOLTMETER_STEPS_PER_SECOND),
    new Voltmeter(SECONDS_VOLTMETER_PIN, VOLTMETER_STEPS_PER_SECOND));

Switch button1 = Switch(BUTTON_1_PIN, INPUT_PULLUP, LOW, 50, 1000, 250, 10);
Switch button2 = Switch(BUTTON_2_PIN);
Switch button3 = Switch(BUTTON_3_PIN);

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

  ledManager.begin();
  ledManager.update();

  voltmeterManager.begin();

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
  state = displayTime;
}

void displayTimeLoop()
{

  //TODO: Remove/refactor delay
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
  settingsData.hours = dt.hour();
  settingsData.minutes = dt.minute();
  settingsData.seconds = 0;

  writeTimetoSerial(settingsData.hours, settingsData.minutes, settingsData.seconds);

  voltmeterManager.setDisplayMode(digital);
  voltmeterManager.updateTime(settingsData.hours, settingsData.minutes, settingsData.seconds);

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
    settingsData.hours = (settingsData.hours + 1) % 24;
    writeTimetoSerial(settingsData.hours, settingsData.minutes, settingsData.seconds);
    voltmeterManager.updateTime(settingsData.hours, settingsData.minutes, settingsData.seconds);
  }
  else if (button3.pushed())
  {
    settingsData.minutes = (settingsData.minutes + 1) % 60;
    writeTimetoSerial(settingsData.hours, settingsData.minutes, settingsData.seconds);
    voltmeterManager.updateTime(settingsData.hours, settingsData.minutes, settingsData.seconds);
  }


}

void exitSettings()
{

  state = displayTime;
  rtc.adjust(DateTime(2023, 1, 1, settingsData.hours, settingsData.minutes, settingsData.seconds));
  lastDisplayUpdate = 0; // Force an update

  voltmeterManager.resetDisplayMode();

  ledManager.setMode(constant);

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
    // Enter settings
    enterSettings();
  }
}