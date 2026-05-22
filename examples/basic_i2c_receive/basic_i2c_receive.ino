/**
 * @example{lineno} basic_i2c_receive.ino
 *
 * @brief Simple I2C UART receive test.
 *
 * Connect an external USB SERIAL device to the SC16IS7XX.
 * Remember to swap the TXD and RXD.
 * Anything sent will be echoed in the serial monitor.
 *
 * SC16IS7XX   USB SERIAL
 * ======   ==========
 * GND      GND
 * TXD      RXD
 * RXD      TXD
 *
 * Set the terminal to 115200 bps, 8 bits, no parity, 1 stop bit.
 *
 */

#include <SC16IS7xx.h>

int8_t powerPin = -1;

// Create the port expander object and an empty pointer for the serial interface
SC16IS752       ExtPort;
SC16IS7xx_UART* ExtSerial = nullptr;

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
    if (!ExtPort.begin_i2c()) {
        Serial.println("not found. Please ensure that the module\r\nis plugged "
                       "in and securely fastened to the baseboard.");
        while (true) delay(100);
    }
    Serial.println("found!");

    ExtSerial = ExtPort.uartA();
    if (!ExtSerial) {
        Serial.println("failed to create UART channel A");
        while (true) delay(100);
    }

    ExtSerial->enableFIFO(true);  // enable fifo
    ExtSerial->setBaudrate(115200);
    ExtSerial->setLine(8, 0, 1);  // 8,n,1
}

void loop() {
    if (ExtSerial->available() > 0) { Serial.print((char)ExtSerial->read()); }
    delay(100);
}
