# Appnostic SC16IS7XX Arduino Library

This is a library for the SC16IS750/SC16IS751/SC16IS752 series of UART interfaces by NXP.

Made with love by the team at Appnostic!

To install, use the Arduino IDE Library Manager.

## Using This Library With Other Hardware

As the library interfaces with the SC16IS75XX via I2C or SPI it should communicate just fine with most hardware variants provided the following is observed:

- The default crystal frequency is 14.7456MHz (147456000Hz). If a different crystal is used you must call the `setCrystalFrequency(frequency)` method before `setBaudRate` so that the correct divisor can be calculated.
- If using interrupts take note that only pins 2 and 3 on the Arduino Uno facilitate external interrupts. The Minima has all digital pins available.
- If using SPI the default pin is the CS pin as denoted by the board support package, usually pin 10 on the Uno.
- YMMV for STM32 or ESP32 hardware. Pull requests welcomed.

## Submitting Pull Requests

If you would like to submit patches to support your specific hardware and use cases please feel free :)
