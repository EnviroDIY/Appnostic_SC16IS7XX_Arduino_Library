# Examples using the EnviroDIY SC16IS7XX Arduino Library<!--! {#page_the_examples} -->

These example programs demonstrate how to use the EnviroDIY SC16IS7XX Arduino library.

The current sketches instantiate `SC16IS752` explicitly.
If your hardware is a different family member, change the chip object type to `SC16IS740`, `SC16IS750`, `SC16IS760`, or `SC16IS762` as appropriate.

___

<!--! @if GITHUB -->

- [Examples using the EnviroDIY SC16IS7XX Arduino Library](#examples-using-the-envirodiy-sc16is7xx-arduino-library)
  - [Basic I2C UART Receive](#basic-i2c-uart-receive)
  - [Basic I2C UART Send](#basic-i2c-uart-send)
  - [GPIO Blink](#gpio-blink)
  - [GPIO Interrupt](#gpio-interrupt)
  - [GPIO Poll](#gpio-poll)
  - [I2C UART Loopback](#i2c-uart-loopback)

<!--! @endif -->

<!--! @tableofcontents -->

<!--! @m_footernavigation -->

## Basic I2C UART Receive<!--! {#examples_basic_i2c_receive} -->

This receives UART data via the SC16IS7XX over I2C and echoes incoming bytes to the serial monitor.

This sketch currently uses `SC16IS752`.

- [Instructions for the basic I2C receive example](https://envirodiy.github.io/SC16IS7XX/example_basic_i2c_receive.html)
- [The basic I2C receive example on GitHub](https://github.com/EnviroDIY/SC16IS7XX/tree/main/examples/basic_i2c_receive)

<!--! @m_innerpage{example_basic_i2c_receive} -->

## Basic I2C UART Send<!--! {#examples_basic_i2c_send} -->

This sends UART data through the SC16IS7XX over I2C to a connected serial device.

This sketch currently uses `SC16IS752`.

- [Instructions for the basic I2C send example](https://envirodiy.github.io/SC16IS7XX/example_basic_i2c_send.html)
- [The basic I2C send example on GitHub](https://github.com/EnviroDIY/SC16IS7XX/tree/main/examples/basic_i2c_send)

<!--! @m_innerpage{example_basic_i2c_send} -->

## GPIO Blink<!--! {#examples_gpio_blink} -->

This toggles an SC16IS7XX GPIO pin to blink an attached LED.

This sketch currently uses `SC16IS752`.

- [Instructions for the GPIO blink example](https://envirodiy.github.io/SC16IS7XX/example_gpio_blink.html)
- [The GPIO blink example on GitHub](https://github.com/EnviroDIY/SC16IS7XX/tree/main/examples/gpio_blink)

<!--! @m_innerpage{example_gpio_blink} -->

## GPIO Interrupt<!--! {#examples_gpio_interrupt} -->

This configures SC16IS7XX GPIO interrupt behavior and reports interrupt events.

This sketch currently uses `SC16IS752`.

- [Instructions for the GPIO interrupt example](https://envirodiy.github.io/SC16IS7XX/example_gpio_interrupt.html)
- [The GPIO interrupt example on GitHub](https://github.com/EnviroDIY/SC16IS7XX/tree/main/examples/gpio_interrupt)

<!--! @m_innerpage{example_gpio_interrupt} -->

## GPIO Poll<!--! {#examples_gpio_poll} -->

This continuously polls SC16IS7XX GPIO states and reports pin values.

This sketch currently uses `SC16IS752`.

- [Instructions for the GPIO poll example](https://envirodiy.github.io/SC16IS7XX/example_gpio_poll.html)
- [The GPIO poll example on GitHub](https://github.com/EnviroDIY/SC16IS7XX/tree/main/examples/gpio_poll)

<!--! @m_innerpage{example_gpio_poll} -->

## I2C UART Loopback<!--! {#examples_i2c_loopback} -->

This runs a UART loopback test using SC16IS7XX configured over I2C.

This sketch currently uses `SC16IS752`.

- [Instructions for the I2C loopback example](https://envirodiy.github.io/SC16IS7XX/example_i2c_loopback.html)
- [The I2C loopback example on GitHub](https://github.com/EnviroDIY/SC16IS7XX/tree/main/examples/i2c_loopback)

<!--! @m_innerpage{example_i2c_loopback} -->
