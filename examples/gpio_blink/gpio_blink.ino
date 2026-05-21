/**
 * @example{lineno} gpio_blink.ino
 *
 * @brief Simple GPIO blink test using I2C communication with the SC16IS7XX.
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
    // NOTE: For everything except the ESP32 , you could simply use
    // `ExtSerial.pinMode` and `ExtSerial.digitalWrite` here, but for the ESP32,
    // we need to use the 'Ex' versions of these functions to avoid conflicts
    // with the built-in pin functions.
    ExtSerial.pinModeExternal(GPIO_PIN, OUTPUT);
    // set the pin low to start
    ExtSerial.digitalWriteExternal(GPIO_PIN, LOW);
}

void loop() {
    // NOTE: For everything except the ESP32 , you could simply use
    // `ExtSerial.digitalWrite` here, but for the ESP32, we need to use the 'Ex'
    // versions of these functions to avoid conflicts with the built-in pin
    // functions.
    ExtSerial.digitalWriteExternal(GPIO_PIN, HIGH);
    delay(500);
    ExtSerial.digitalWriteExternal(GPIO_PIN, LOW);
    delay(500);
}
