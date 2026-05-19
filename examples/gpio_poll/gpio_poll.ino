/**
 * @example{lineno} gpio_poll.ino
 *
 * @brief I2C UART GPIO Polling Test
 *
 * As an alternative to enabling interrupts on the Arduino this
 * example shows how to poll the interrupt pin.
 *
 * Connect a button or switch to GPIO 0 of the UART interface module.
 * When the button is pressed, LED2 should turn on.
 *
 */

#include <SC16IS752.h>

int8_t powerPin = -1;

SC16IS752 ExtSerial(SC16IS752_CHANNEL_A);

#define GPIO_PIN 0
#define SC16IS7XX_IRQ_PIN 3
#define LED_PIN 9

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
    ExtSerial.pinMode(GPIO_PIN, INPUT);

    // enable interrupts for the pin
    ExtSerial.setPinInterrupt(GPIO_PIN, true);

    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    if (digitalRead(SC16IS7XX_IRQ_PIN) == LOW) {
        if (ExtSerial.isr() == SC16IS7XX_INT_GPIO) {
            digitalWrite(LED_PIN, !ExtSerial.digitalRead(GPIO_PIN));
        }
    }
    delay(100);
}
