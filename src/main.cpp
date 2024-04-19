#define VERBOSE_DEBUG

#include <Arduino.h>
#include <voltmeter/VoltmeterManager.h>
#include <led/SmoothLedManager.h>
#include <RTClib.h>
#include <avdweb_Switch.h>
#include <Encoder.h>
#include <alarm/ToneAlarm.h>


// Voltmeters
// Do not use Pins 5 and 6 for Voltmeters
#define HOURS_VOLTMETER_PIN 10
#define MINUTES_VOLTMETER_PIN 9
#define SECONDS_VOLTMETER_PIN 6
#define VOLTMETER_STEPS_PER_SECOND 255

// LEDs
#define LED_PIN 5

// Buzzer
#define BUZZER_PIN 11

// Rotary Encoders
#define HOURS_ENCODER_CLK 2
#define HOURS_ENCODER_DT 3
#define HOURS_ENCODER_SWITCH 14
#define MINUTES_ENCODER_CLK 17
#define MINUTES_ENCODER_DT 16
#define MINUTES_ENCODER_SWITCH 15

// Front switch
#define FRONT_SWITCH_UP 7
#define FRONT_SWITCH_DOWN 8

// Alarm
#define ALARM_LOOP_COUNT 3

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

void enterAlarmSet();
void alarmSetLoop();
void exitAlarmSet();

void buttonSingleClickedCallback(void *ref);
void buttonLongPressedCallback(void *ref);
void buttonPushedCallback(void *ref);

void readEncoders();

const byte HOUR_SWITCH_ID = 1;
const byte MINUTES_SWITCH_ID = 2;
const byte FRONT_SWITCH_UP_ID = 3;
const byte FRONT_SWITCH_DOWN_ID = 4;

enum ClockState
{
  startup,
  displayTime,
  setting,
  calibration,
  setAlarm
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

SmoothLedManager ledManager(LED_PIN);

VoltmeterManager voltmeterManager(
    VoltmeterConfig{
        HOURS_VOLTMETER_PIN,
        MINUTES_VOLTMETER_PIN,
        SECONDS_VOLTMETER_PIN,
        VOLTMETER_STEPS_PER_SECOND});

Switch hourButton = Switch(HOURS_ENCODER_SWITCH, INPUT_PULLUP, LOW, 50, 2000, 250, 10);
Switch minutesButton = Switch(MINUTES_ENCODER_SWITCH, INPUT_PULLUP, LOW, 50, 2000, 250, 10);

Switch alarmSetButton = Switch(FRONT_SWITCH_UP, INPUT_PULLUP, LOW, 50, 1000, 250, 10);

Encoder hourEncoder = Encoder(HOURS_ENCODER_CLK, HOURS_ENCODER_DT);
Encoder minutesEncoder = Encoder(MINUTES_ENCODER_CLK, MINUTES_ENCODER_DT);

struct EncoderData
{
  long encoderPos = 0;
  long newEncoderPos = 0;
  bool encoderUp = false;
  bool encoderDown = false;
};

EncoderData hourEncoderData = {};
EncoderData minutesEncoderData = {};

RTC_DS3231 rtc;

ToneAlarm toneAlarm = ToneAlarm(BUZZER_PIN);

void setup()
{
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);

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
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);
  rtc.disableAlarm(2);

  hourButton.setSingleClickCallback(&buttonSingleClickedCallback, (void *)&HOUR_SWITCH_ID);
  minutesButton.setSingleClickCallback(&buttonSingleClickedCallback, (void *)&MINUTES_SWITCH_ID);

  hourButton.setLongPressCallback(&buttonLongPressedCallback, (void *)&HOUR_SWITCH_ID);
  minutesButton.setLongPressCallback(&buttonLongPressedCallback, (void *)&MINUTES_SWITCH_ID);

  alarmSetButton.setPushedCallback(&buttonPushedCallback, (void *)&FRONT_SWITCH_UP_ID);

  pinMode(FRONT_SWITCH_DOWN, INPUT_PULLUP);

  ledManager.begin();

  voltmeterManager.begin();

  toneAlarm.begin();

  state = startup;
}

void loop()
{
  readEncoders();
  hourButton.poll();
  minutesButton.poll();
  alarmSetButton.poll();

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
  case setAlarm:
    alarmSetLoop();
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

  if (hourEncoderData.encoderUp || hourEncoderData.encoderDown) {
    if (ledManager.changeLedBrightness(hourEncoderData.encoderUp ? 5 : -5))
    {
      ledManager.saveLedBrightness();
    }
    
  }

  unsigned long localNow = millis();
  if (displayStateData.rtcUpdateInterval <= (localNow - displayStateData.lastRTCPoll))
  {
    DateTime rtcNow = rtc.now();
    rtcTimeHolder.hours = rtcNow.hour();
    rtcTimeHolder.minutes = rtcNow.minute();
    if (rtcNow.second() != rtcTimeHolder.seconds)
    {
      rtcTimeHolder.seconds = rtcNow.second();
      displayStateData.lastSecondChange = localNow;
    }

    voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, localNow - displayStateData.lastSecondChange);
    displayStateData.lastRTCPoll = localNow;
  }
  if (displayStateData.serialOutputInterval <= (localNow - displayStateData.lastSerialOutput))
  {
    Serial.print(rtc.getTemperature());
    Serial.print(F("°C "));
    writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
    displayStateData.lastSerialOutput = localNow;
  }

  if (!digitalRead(FRONT_SWITCH_DOWN))
  {
    if (rtc.alarmFired(1)) {
      Serial.println("Alarm fired");
      rtc.clearAlarm(1);
      toneAlarm.start(ALARM_LOOP_COUNT);
    }
    toneAlarm.play();
  } else {
    toneAlarm.stop();

    // Clear the alarm flag if it fired
    if (rtc.alarmFired(1)) {
      rtc.clearAlarm(1);
    }
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
  if (hourButton.pushed())
  {
    exitSettings();
  }
  else if (hourEncoderData.encoderUp || hourEncoderData.encoderDown)
  {
    rtcTimeHolder.hours = constrain(rtcTimeHolder.hours + (hourEncoderData.encoderUp ? 1 : -1), 0, 23);
    writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
    voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);
  }
  else if (minutesEncoderData.encoderUp || minutesEncoderData.encoderDown)
  {
    rtcTimeHolder.minutes =  constrain(rtcTimeHolder.minutes + (minutesEncoderData.encoderUp ? 1 : -1), 0, 59);
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
  ledManager.setMode(on);

  state = calibration;
}

void calibrationStateLoop()
{
  // Used to calibrate the voltmeters
  if (hourEncoderData.encoderUp || hourEncoderData.encoderDown)
  {
    rtcTimeHolder.hours = constrain(rtcTimeHolder.hours + (hourEncoderData.encoderUp ? 1 : -1), 0, 24);
    voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);
    voltmeterManager.printTargets();
    writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
  }
  else if (minutesEncoderData.encoderUp || minutesEncoderData.encoderDown)
  {
    rtcTimeHolder.minutes = constrain(rtcTimeHolder.minutes + (minutesEncoderData.encoderUp ? 5 : -5), 0, 60);
        rtcTimeHolder.seconds = rtcTimeHolder.minutes;
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

void enterAlarmSet() {
  Serial.println(F("Entering alarm set state"));
  voltmeterManager.setDisplayMode(digital);
  state = setAlarm;

  DateTime alarm = rtc.getAlarm1();
  rtcTimeHolder.hours = alarm.hour();
  rtcTimeHolder.minutes = alarm.minute();
  rtcTimeHolder.seconds = 0;

  voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);
  writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
  ledManager.setMode(pulsing);
}

void alarmSetLoop() {
  if (alarmSetButton.released())
  {
    exitAlarmSet();
  }
  else if (hourEncoderData.encoderUp || hourEncoderData.encoderDown)
  {
    rtcTimeHolder.hours = constrain(rtcTimeHolder.hours + (hourEncoderData.encoderUp ? 1 : -1), 0, 23);
    writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
    voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);
  }
  else if (minutesEncoderData.encoderUp || minutesEncoderData.encoderDown)
  {
    rtcTimeHolder.minutes =  constrain(rtcTimeHolder.minutes + (minutesEncoderData.encoderUp ? 5 : -5), 0, 55);
    writeTimetoSerial(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds);
    voltmeterManager.updateTime(rtcTimeHolder.hours, rtcTimeHolder.minutes, rtcTimeHolder.seconds, 0);
  }

  toneAlarm.play();
}

void exitAlarmSet() {

  
  DateTime newAlarmTime = DateTime(0, 0, 0, rtcTimeHolder.hours, rtcTimeHolder.minutes, 0);
  rtc.setAlarm1(newAlarmTime, DS3231_A1_Hour);

  // Force an update
  displayStateData.lastRTCPoll = 0;
  displayStateData.lastSerialOutput = 0;

  voltmeterManager.resetDisplayMode();

  ledManager.setMode(saved_level);

  toneAlarm.stop();

  state = displayTime;
  Serial.print(F("Exiting alarm set state, alarm set to "));
  writeTimetoSerial(newAlarmTime.hour(), newAlarmTime.minute(), 0);
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

  if (b == MINUTES_SWITCH_ID)
  {
    if (state == displayTime)
    {
      // Change voltmeter display mode
      voltmeterManager.changeDisplayMode();
      voltmeterManager.saveCurrentDisplayMode();
      displayStateData.lastRTCPoll = 0; // Force update
    }
     else if (state == setAlarm)
    {
      toneAlarm.changeMelody();
      toneAlarm.start(1);
    }

  }
}

void buttonLongPressedCallback(void *ref)
{
  byte b = *((byte *)ref);

  Serial.print(F("Button long pressed: "));
  Serial.println(b);

  if (b == HOUR_SWITCH_ID && state == displayTime)
  {
    enterSettings();
  }
  else if (b == MINUTES_SWITCH_ID)
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

void buttonPushedCallback(void *ref) {
    byte b = *((byte *)ref);
    if (b == FRONT_SWITCH_UP_ID && state == displayTime) {
      enterAlarmSet();
    }
}

void readEncoders() {
  hourEncoderData.newEncoderPos = hourEncoder.read();
  minutesEncoderData.newEncoderPos = minutesEncoder.read();

  if (abs(hourEncoderData.encoderPos - hourEncoderData.newEncoderPos) >= 4) {
    hourEncoderData.encoderDown = hourEncoderData.encoderPos < hourEncoderData.newEncoderPos;
    hourEncoderData.encoderUp = hourEncoderData.encoderPos > hourEncoderData.newEncoderPos;
    hourEncoderData.encoderPos = hourEncoderData.newEncoderPos;
  }
  else
  {
      hourEncoderData.encoderDown = false;
      hourEncoderData.encoderUp = false;
  }
  

  if (abs(minutesEncoderData.encoderPos - minutesEncoderData.newEncoderPos) >= 4) {
    minutesEncoderData.encoderDown = minutesEncoderData.encoderPos < minutesEncoderData.newEncoderPos;
    minutesEncoderData.encoderUp = minutesEncoderData.encoderPos > minutesEncoderData.newEncoderPos;
    minutesEncoderData.encoderPos = minutesEncoderData.newEncoderPos;
  }
  else
  {
      minutesEncoderData.encoderDown = false;
      minutesEncoderData.encoderUp = false;
  }
  
}