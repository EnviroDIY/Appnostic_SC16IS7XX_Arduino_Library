# EnviroDIY SC16IS7XX Arduino Library

This is a library for the SC16IS752 and SC16IS762 series of UART interfaces by NXP.
It should also work with the SC16IS750, though that chip only supports one UART channel.

<!--! @tableofcontents -->

<!--! @m_footernavigation -->

<!--! @if GITHUB -->

- [EnviroDIY SC16IS7XX Arduino Library](#envirodiy-sc16is7xx-arduino-library)
  - [Using This Library With Other Hardware](#using-this-library-with-other-hardware)
  - [Library Credits and Provenance](#library-credits-and-provenance)
  - [Submitting Pull Requests](#submitting-pull-requests)

<!--! @endif -->

## Using This Library With Other Hardware

As the library interfaces with the SC16IS752 or SC16IS762 via I2C or SPI it should communicate just fine with most hardware variants provided the following is observed:

- The default crystal frequency is 14.7456MHz (14745600Hz). If a different crystal is used you must either call `setCrystalFrequency(frequency)` on your `SC16IS7xx_UART` object before `setBaudRate` or define `SC16IS7XX_DEFAULT_XTAL_FREQ=xx` with a build flag so that the correct divisor can be calculated.
- The default I2C address is 0x4D (8-bit address of 0x90). That is the address for a chip with both A0 and A1 pulled HIGH.  If a different address configuration is needed, you must either set it in the `begin_i2c(addr, Wire)` function or define `SC16IS7XX_DEFAULT_ADDRESS=xx` with a build flag.

## Library Credits and Provenance

Unfortunately, the git history of this library doesn't bring in the full provenance and fork history.
The original source for this library was developed by [Sandbox Electronics](https://github.com/SandboxElectronics/UART_Bridge) for the SC16IS750.
That library was modified by [TD-er](https://github.com/TD-er/SC16IS752) to work with the SC16IS752.
[Appnostic](https://github.com/appnostic-io/Appnostic_SC16IS7XX_Arduino_Library) further modified the library to split it into two classes.
I forked the Appnostic version of the library and have modified it since as documented in the changelog.

## Submitting Pull Requests

If you would like to submit patches to support your specific hardware and use cases please feel free.
