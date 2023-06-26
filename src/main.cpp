#include <Arduino.h>
#include <voltmeter/Voltmeter.h>
#include <time.h>
#include <timeLib.h>
#include <RTClib.h>

// put function declarations here:
void startupRoutine();
void normalStateLoop();
void settingStateLoop();
time_t getTime();

const float maxHours = 23;
const float maxMinutes = 59;
const float maxSeconds = 59;
const int stepsPerSecond = 10;
const unsigned long normalStateDelay = 50;

// Do not use Pins 5 and 6 for Voltmeters
enum ClockState
{
  startup,
  normal,
  setting
};
ClockState state = startup;

Voltmeter hoursVoltmeter(11, stepsPerSecond);
Voltmeter minutesVoltmeter(10, stepsPerSecond);
Voltmeter secondsVoltmeter(9, stepsPerSecond);

RTC_DS3231 rtc;

void setup()
{
  Serial.begin(9600);

  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (true)
      ;
  }

  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, set the time!");
    rtc.adjust(DateTime(2023, 1, 1, 0, 0, 0));
  }

  rtc.disable32K();
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);
  rtc.disableAlarm(1);
  rtc.disableAlarm(2);

  setSyncProvider(getTime);
  setSyncInterval(120);
  // put your setup code here, to run once:
}

void loop()
{
  // put your main code here, to run repeatedly:

  switch (state)
  {
  case startup:
    startupRoutine();
    break;
  case normal:
    normalStateLoop();
    break;
  case setting:
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
  //Maybe get the time directly from the RTC
  int hours = hour();
  int minutes = minute();
  int seconds = second();

  float floatHours = hours + (minutes / 60);
  float floatMinutes = minutes + (seconds / 60);

  hoursVoltmeter.setTarget(floatHours * (255 / maxHours));
  minutesVoltmeter.setTarget(floatMinutes * (255 / maxMinutes));
  secondsVoltmeter.setTarget(seconds * (255 / maxSeconds));

  hoursVoltmeter.update();
  minutesVoltmeter.update();
  secondsVoltmeter.update();
  delay(normalStateDelay);
}

void settingStateLoop() {
  
}

time_t getTime()
{
  DateTime now = rtc.now();
  return (time_t)now.unixtime();
}