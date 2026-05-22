/**
 * @example{lineno} basic_i2c_send.ino
 *
 * @brief Simple I2C UART send test.
 *
 * Connect an external USB SERIAL device to the SC16IS7XX.
 * Remember to swap the TXD and RXD.
 *
 * SC16IS7XX   USB SERIAL
 * ======   ==========
 * GND      GND
 * TXD      RXD
 * RXD      TXD
 *
 * Set the terminal to 115200 bps, 8 bits, no parity, 1 stop bit.
 *
 * Output:
 * Hello world [1]
 * Hello world [2]
 * Hello world [3]
 * ...
 */

#include <SC16IS7xx.h>

int8_t powerPin = -1;

// Create the port expander object and channel A serial interface reference
SC16IS752       ExtPort;
SC16IS7xx_UART& ExtSerial = ExtPort.uartA();

int i = 0;

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

    ExtSerial.enableFIFO(true);  // enable fifo
    ExtSerial.setBaudrate(115200);
    ExtSerial.setLine(8, 0, 1);  // 8,n,1
}

void loop() {
    i++;
    ExtSerial.print("Hello world [");
    ExtSerial.print(i);
    ExtSerial.println("]");
    delay(1000);
}
