# Simple I2C UART receive test <!--! {#example_basic_i2c_receive} -->

Connect an external USB SERIAL device to the SC16IS7XX.
Remember to swap the TXD and RXD.
Anything sent will be echoed in the serial monitor.

The sketch currently instantiates `SC16IS752`. Change that type to
`SC16IS740`, `SC16IS750`, `SC16IS760`, or `SC16IS762` for those chips.

_______

<!--! @section example_basic_i2c_receive_code The Complete Code -->

<!--! @include{lineno} basic_i2c_receive/basic_i2c_receive.ino -->
