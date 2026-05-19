/**
 * @example{lineno} i2c_loopback.ino
 *
 * @brief I2C UART Loopback Test
 *
 * This is a simple loopback test between channels A and B of the SC16IS7XX
 * UART interface module.
 *
 * Connect a wire from the TXD on CHANNEL B to the RXD of CHANNEL A.
 *
 * Output:
 *
 * the SC16IS7XX Test
 * Checking for the SC16IS7XX...found!
 * Loopback data received
 * Loopback data received
 * Loopback data received
 * ...
 */

#include <SC16IS752.h>

int8_t powerPin = -1;

SC16IS752 ExtSerialA(SC16IS752_CHANNEL_A);
SC16IS752 ExtSerialB(SC16IS752_CHANNEL_B);

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
    if (!ExtSerialA.begin_i2c()) {
        Serial.println("not found. Please ensure that the module\r\nis plugged "
                       "in and securely fastened to the baseboard.");
        while (true) delay(100);
    }
    Serial.println("found!");

    // set some parameters
    ExtSerialA.enableFIFO(true);  // enable fifo
    ExtSerialA.setBaudrate(115200);
    ExtSerialA.setLine(8, 0, 1);  // 8,n,1

    // instantiate the second channel. There is no need for additional checking
    // here.
    ExtSerialB.begin_i2c();
    ExtSerialB.enableFIFO(true);  // enable fifo
    ExtSerialB.setBaudrate(115200);
    ExtSerialB.setLine(8, 0, 1);  // 8,n,1
}

void loop() {
    // send data on channel b
    ExtSerialB.write(0x55);

    if (ExtSerialA.available() > 0) {
        if (ExtSerialA.read() != 0x55)
            Serial.println("Error receiving loopback data");
        else
            Serial.println("Loopback data received");
    }
    delay(1000);
}
