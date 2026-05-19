/**
 * @example{lineno} gpio_blink.ino
 *
 * @brief I2C UART GPIO Test
 *
 * The basic blink
 *
 * Connect an LED to GPIO 0 of the UART interface module.
 *
 */

#include <SC16IS752.h>

int8_t powerPin = -1;

SC16IS752 ExtSerial(SC16IS752_CHANNEL_A);

#define GPIO_PIN 0

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
