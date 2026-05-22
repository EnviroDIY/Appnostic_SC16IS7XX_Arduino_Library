# EnviroDIY SC16IS7XX Arduino Library

This is a library for the SC16IS740, SC16IS750, SC16IS760, SC16IS752 and SC16IS762 series of UART interfaces by NXP.

All 5 chips are I2C-bus/SPI bus interfaces to a single or dual-channel high performance UART offering data rates up to 5 Mbit/s, low operating and sleeping current.
All chips except the SC16IS740 also provide 8 additional programmable I/O pins.

The SC16IS740/750/760’s internal register set is backward-compatible with the widely used and widely popular 16C450.
This library should also support those chips since the register sets match.

<!--! @tableofcontents -->

<!--! @m_footernavigation -->

<!--! @if GITHUB -->

- [EnviroDIY SC16IS7XX Arduino Library](#envirodiy-sc16is7xx-arduino-library)
  - [Using This Library With Other Hardware](#using-this-library-with-other-hardware)
  - [Library Credits and Provenance](#library-credits-and-provenance)
  - [Submitting Pull Requests](#submitting-pull-requests)

<!--! @endif -->

## Using This Library With Other Hardware

Use an explicit chip type that matches your hardware:

- SC16IS740: 1 UART channel, 0 GPIO pins
- SC16IS750: 1 UART channel, 8 GPIO pins
- SC16IS760: 1 UART channel, 8 GPIO pins
- SC16IS752: 2 UART channels, 8 GPIO pins
- SC16IS762: 2 UART channels, 8 GPIO pins

The driver is implemented once and configured by compile-time chip traits plus per-instance capability limits. This means the same API is available for all variants, while channel and GPIO bounds are enforced for the selected chip.

Typical setup flow:

1. Create the chip object using the exact class for your device (for example, SC16IS740, SC16IS750, SC16IS760, SC16IS752, or SC16IS762).
2. Initialize transport with begin_i2c(...) or begin_SPI(...).
3. Acquire UART channel objects with uartA() and, for dual-UART chips, uartB().
4. Configure each UART channel (for example enableFIFO, setBaudrate, setLine).

- The default crystal frequency is 14.7456 MHz (14745600 Hz). If a different crystal is used, call `setCrystalFrequency(frequency)` on each SC16IS7xx_UART object before `setBaudrate`, or define `SC16IS7XX_DEFAULT_XTAL_FREQ=xx` with a build flag so the divisor is calculated correctly.
- The default I2C address is 0x4D (8-bit address of 0x90). That is the address for a chip with both A0 and A1 pulled HIGH. If a different address configuration is needed, set it in `begin_i2c(addr, Wire)` or define `SC16IS7XX_DEFAULT_ADDRESS=xx` with a build flag.

## Library Credits and Provenance

Unfortunately, the git history of this library doesn't bring in the full provenance and fork history.
The original source for this library was developed by [Sandbox Electronics](https://github.com/SandboxElectronics/UART_Bridge) for the SC16IS750.
That library was modified by [TD-er](https://github.com/TD-er/SC16IS752) to work with the SC16IS752.
[Appnostic](https://github.com/appnostic-io/Appnostic_SC16IS7XX_Arduino_Library) further modified the library to split it into two classes.
I forked the Appnostic version of the library and have modified it since as documented in the changelog.

## Submitting Pull Requests

If you would like to submit patches to support your specific hardware and use cases please feel free.
