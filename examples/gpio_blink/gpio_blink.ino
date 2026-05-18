/**
 * I2C UART GPIO Test
 *
 * The basic blink
 *
 * Connect an LED to GPIO 0 of the UART interface module.
 *
 */

#include <Appnostic_SC16IS752.h>

Appnostic_SC16IS752 ExtSerial(SC16IS752_CHANNEL_A);

#define GPIO_PIN 0

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

    // set the pin mode
    ExtSerial.pinMode(GPIO_PIN, OUTPUT);

    // set the pin low to start
    ExtSerial.digitalWrite(GPIO_PIN, LOW);
}

void loop() {
    ExtSerial.digitalWrite(GPIO_PIN, HIGH);
    delay(500);
    ExtSerial.digitalWrite(GPIO_PIN, LOW);
    delay(500);
}
