/**
 * @example{lineno} basic_i2c_receive.ino
 *
 * @brief Simple I2C UART receive test.
 *
 * Connect an external USB SERIAL device to the SC16IS7XX but remember to
 * swap the TXD and RXD. Anything sent will be echoed in the serial monitor.
 *
 * NO8007   USB SERIAL
 * ======   ==========
 * GND      GND
 * TXD      RXD
 * RXD      TXD
 *
 * Set the terminal to 115200 bps, 8 bits, no parity, 1 stop bit.
 *
 */

#include <SC16IS752.h>

int8_t powerPin = -1;

SC16IS752 ExtSerial(SC16IS752_CHANNEL_A);

void setup() {
    // power the chip if necessary
    if (powerPin >= 0) {
        pinMode(powerPin, OUTPUT);
        digitalWrite(powerPin, HIGH);
        delay(100);  // let things settle
    }

    Serial.begin(115200);
    while (!Serial) delay(100);

    Serial.println("SC16IS7XX Test");

    Serial.print("Checking for the SC16IS7XX...");
    if (!ExtSerial.begin_i2c()) {
        Serial.println("not found. Please ensure that the module\r\nis plugged "
                       "in and securely fastened to the baseboard.");
        while (true) delay(100);
    }
    Serial.println("found!");

    ExtSerial.enableFIFO(true);  // enable fifo
    ExtSerial.setBaudrate(115200);
    ExtSerial.setLine(8, 0, 1);  // 8,n,1
}

void loop() {
    if (ExtSerial.available() > 0) { Serial.print((char)ExtSerial.read()); }
    delay(100);
}
