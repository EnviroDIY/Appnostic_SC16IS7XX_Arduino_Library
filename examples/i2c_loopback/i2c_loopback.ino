/**
 * I2C UART Loopback Test
 *
 * This is a simple loopback test between channels A and B of the NOS8007
 * UART interface module.
 *
 * Connect a wire from the TXD on CHANNEL B to the RXD of CHANNEL A.
 *
 * Output:
 *
 * NOS8007 Test
 * Checking for NOS8007...found!
 * Loopback data received
 * Loopback data received
 * Loopback data received
 * ...
 */

#include <Appnostic_SC16IS752.h>

Appnostic_SC16IS752 ExtSerialA(SC16IS752_CHANNEL_A);
Appnostic_SC16IS752 ExtSerialB(SC16IS752_CHANNEL_B);

int i = 0;

void setup() {
    // enable NOS8007 power by setting the EN pin of
    // the NOS10001 baseboard to HIGH
    pinMode(A3, OUTPUT);
    digitalWrite(A3, HIGH);
    delay(100);  // let things settle

    Serial.begin(115200);
    while (!Serial) delay(100);

    Serial.println("NOS8007 Test");

    Serial.print("Checking for NOS8007...");
    if (!ExtSerialA.begin_i2c()) {
        Serial.println("not found. Please ensure that the module\r\nis plugged "
                       "in and securely fastened to the baseboard.");
        while (true) delay(100);
    }
    Serial.println("found!");

    // set some parameters
    ExtSerialA.setFIFO(true);  // enable fifo
    ExtSerialA.setBaudrate(115200);
    ExtSerialA.setLine(8, 0, 1);  // 8,n,1

    // instantiate the second channel. There is no need for additional checking
    // here.
    ExtSerialB.begin_i2c();
    ExtSerialB.setFIFO(true);  // enable fifo
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
