# Voltmeter Clock v2.0

## Controls

![Controls](/images/controls.jpg)


* Knob 1: Rotate to change backlight brightness. Press and hold for 1-2s to enter time setting mode
* Knob 2: Click to change time display mode
* Switch 3: Controls alarm

## Startup
When the clock is connected to power, a startup routine will run, slowly pulsing the backlights and moving all indicators to their maximum position and back, before continuing to normal operation. If the timekeeping was lost, the clock will instead switch to time setting mode.

Use this to confirm that all indicators move freely.

## Backlight
There is a yellow LED backlight installed in each display. Use __Knob 1__ to increase or decrease brightness.  Rotate clockwise to increase and counter-clockwise to decrease backlight brightness. This setting persists, even if power is lost

## Time display mode

The clock supports two modes to display time, _digital_ and _analog_. Press __Knob 2__ to switch between these. This setting persists, even if power is lost.

* _Digital display mode_: Indicators will point to the current hour, minute and second and move in discrete steps as time moves on. So the seconds indicator will move once per seconds, the minute indicator once per minute and the hours indicator once per hour.
* _Analog display mode_: Indicators will move smoothly as time progresses.

## Time setting mode
Press and hold __Knob 1__ until the backlight LEDs start blinking (1-2 seconds). The clock will always switch to _digital display mode_ to make setting the time easier. Rotate __Knob 1__ to set the hour, and __Knob 2__ to set the minute. Press __Knob 1__ again (a short click will be enough) to exit time setting mode. Seconds will always be reset to 0 and start counting again when the new time is confirmed.

## Alarm

__Switch 3__ has three positions:

* __Up:__ Set alarm time
* __Middle:__ Alarm disabled
* __Down:__ Alarm armed

To set the alarm, move the __Switch 3__ to the __up__ position. Use __Knob 1__ and __Knob 2__ to set the hour and minute of the alarm time. Note that the minutes can only be set in 5 min intervals.

While setting the alarm time, press __Knob 2__ to change the alarm melody. 6 different melodies are available

1. Star Wars
1. Game of Thrones
1. Harry Potter
1. Pirates of the Caribbean
1. Tetris
1. Mario

To save the alarm time, move __Switch 3__ to the __middle__ or __down__ position.

Alarm time and selected melody is persisted, even if power is lost.

## Calibration Mode
Not used in normal operation.

Pressing __Knob 2__ for 1-2 seconds will enter calibration mode, backlight LEDs  will start pulsing slowly. This mode is used to calibrate the displays,
adjusting the display potentiometers on the circuit board until the indicators points to the end of the dial scale. __Knob 1__ will move the hour dial, __Knob 2__ will move the minutes and seconds dials.

Long pressing __Knob 1__ again for 1-2 seconds will exit this mode and normal operation will resume.

## Backup power
![Coin cell battery location](/images/BackupBattery.jpg)

The clock includes a slot for a CR2032 3V coin cell battery to ensure time keeping even if main power is lost. To replace the battery, open the clock by removing the four (4) screws at the bottom of the clock and lifting off the outer case. Insert the coin cell battery into the holder located on the small module in the clock (see picture).

## Power Supply
Ideally the clock is powered using a 5.5 X 2.1 jack with a 7-12V power supply, with ~7V being recommended.

Powering the clock via the USB B port (which only provides 5V) will also work, but
the display will not be accurate as the backlight brightness will affect the indicators in this case.

## Serial output
Connecting a USB B cable to the clock, a serial monitor using a baud rate of 115200 will allow you to read the debug output of the clock.
