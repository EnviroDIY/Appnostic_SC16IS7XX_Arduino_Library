/**
 * Simple I2C UART send test.
 *
 * Connect an external USB SERIAL device to NOS8007 but remember to
 * swap the TXD and RXD.
 *
 * NO8007   USB SERIAL
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

#include <Appnostic_SC16IS752.h>

Appnostic_SC16IS752 ExtSerial(SC16IS752_CHANNEL_A);

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
    if (!ExtSerial.begin_i2c()) {
        Serial.println("not found. Please ensure that the module\r\nis plugged "
                       "in and securely fastened to the baseboard.");
        while (true) delay(100);
    }
    Serial.println("found!");

    ExtSerial.setFIFO(true);  // enable fifo
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
