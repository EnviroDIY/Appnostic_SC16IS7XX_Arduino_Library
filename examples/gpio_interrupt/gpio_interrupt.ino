/**
 * @example{lineno} gpio_interrupt.ino
 *
 * @brief I2C UART GPIO Interrupt Test
 *
 * This example shows how to enable input on a pin and
 * configure interrupts for it. Note that the input pin
 * is active low.
 *
 * NOTE: On the Arduino Uno there are only two interrupt pins on
 * pins 2 and 3 which are not used by the SC16IS7XX.
 * On the R4 Minima all pins can support interrupts.
 *
 * Connect a button or switch to GPIO 0 of the UART interface module.
 *
 */

#include <pins_arduino.h>
#include <Appnostic_SC16IS752.h>

int8_t powerPin = -1;

Appnostic_SC16IS752 ExtSerial(SC16IS752_CHANNEL_A);

#define GPIO_PIN 0
#define SC16IS7XX_IRQ_PIN 3

bool interrupted = false;

/**
 * @brief interrupt handler for the SC16IS7XX IRQ
 */
void onInterrupt() {
    interrupted = true;
}

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

    // enable the interrupt controller
    ExtSerial.enableInterruptControl(true);

    pinMode(SC16IS7XX_IRQ_PIN, INPUT);  // no pull required
    attachInterrupt(digitalPinToInterrupt(SC16IS7XX_IRQ_PIN), onInterrupt,
                    FALLING);  // interrupt transitions from high to low
}

void loop() {
    if (interrupted == true) {
        interrupted = false;
        if (ExtSerial.isr() == SC16IS7XX_INT_GPIO) {
            Serial.print("Interrupt Pin: ");
            Serial.print(GPIO_PIN);
            Serial.print(", State: ");
            Serial.println(ExtSerial.digitalRead(GPIO_PIN));
        }
    }
    delay(100);
}
