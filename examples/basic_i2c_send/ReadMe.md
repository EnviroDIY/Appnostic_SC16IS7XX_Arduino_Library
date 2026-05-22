# Simple I2C UART send test <!--! {#example_basic_i2c_send} -->

Connect an external USB SERIAL device to the SC16IS7XX.
Remember to swap the TXD and RXD.
This example sends serial data from the microcontroller to the external UART device.

The sketch currently instantiates `SC16IS752`.
Change that type to `SC16IS740`, `SC16IS750`, `SC16IS760`, or `SC16IS762` for those chips.

_______

<!--! @section example_basic_i2c_send_code The Complete Code -->

<!--! @include{lineno} basic_i2c_send/basic_i2c_send.ino -->
