# Voltmeter Clock

## Buttons

Buttons are counted left to right, with the leftmost button being Button 1


* Button 1: Change backlight brightness
* Button 2: Change time display mode
* Button 3: Press for 1-2s to enter time setting mode

## Startup
When the clock is connected to power, a startup routine will run, slowly pulsing the backlights and moving all indicators to their maximum position and back, before continuing to normal operation. If the timekeeping was lost, the clock will instead switch to time setting mode.

Use this to confirm that all indicators move freely.

## Backlight
There is yellow LED backlight installed in each display. Use __Button 1__ to toggle brightness between 0% (Off), 25%, 50%, 75% and 100%. This setting is persisted, even if power is lost

## Time display mode

The clock supports two modes to display time, _digital_ and _analog_. Use __Button 2__ to switch between these. This setting is persisted, even if power is lost.

* _Digital display mode_: Indicators will point to the current hour, minute and second and move in discrete steps as time moves on. So the seconds indicator will move once per seconds, the minute indicator once per minute and the hours indicator once per hour.
* _Analog display mode_: Indicators will move smoothly as time progresses.

## Setting the time
Press __Button 3__ until the backlight LEDs start blinking (1-2 seconds). The clock will always switch to _digital display mode_ to make setting the time easier. __Button 1__ will increment the hour, __Button 2__ will increment the minute. Press __Button 3__ again (a short click will be enough) to exit time setting mode. Seconds will always be reset to 0 and start counting again when the new time is confirmed.

## Calibration Mode
Not used in normal operation.

Pressing __Button 1__ for 4-5 seconds will enter calibration mode, backlight LEDs  will start pulsing slowly. This mode is used to calibrate the displays,
adjusting the display potentiometers on the circuit board until the indicators points to the end of the dial scale. Buttons will increment the dials above them.
Long pressing __Button 1__ again for 4-5 seconds will exit this mode and normal operation will resume.

## Backup power
The clock includes a slot for a CR2032 3V coin cell battery to ensure time keeping even if main power is lost. To replace the battery, open the clock by removing the four (4) screws at the bottom of the clock and lifting off the outer case. Insert the coin cell battery into the holder located on the small module in the clock (see picture).

## Power Supply
Ideally the clock is powered using a 5.5 X 2.1 jack with a 7V power supply. The clock will work with up to 12V, but it is not recommended.

Powering the clock via the USB B port (which only provides 5V) will also work, but
the display will not be accurate as the backlight brightness will affect the indicators in this case.

## Serial output
Connecting a USB B cable to the clock, a serial monitor using a baud rate of 115200 will allow to read the debug output of the clock.
