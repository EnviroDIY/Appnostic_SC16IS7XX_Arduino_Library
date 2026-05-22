/**
 * @example{lineno} gpio_poll.ino
 *
 * @brief I2C UART GPIO Polling Test
 *
 * As an alternative to enabling interrupts on the Arduino this
 * example shows how to poll the interrupt register to check for changes on the
 * GPIO pins.
 *
 * Connect a button or switch to GPIO 0 of the UART interface module.
 *
 */

#include <SC16IS7xx.h>

// The power pin for the external port expander, if necessary. Set to -1 if not
// used.
int8_t powerPin = -1;

// Create the port expander object and an empty pointer for the serial interface
SC16IS752       ExtPort;
SC16IS7xx_UART* ExtSerial = nullptr;


// The GPIO pin on the SC16IS7XX to use for the interrupt test
#define GPIO_PIN 0
// The pin on the Arduino to which the SC16IS7XX IRQ pin is connected
#define SC16IS7XX_IRQ_PIN 3
#define LED_PIN 9
void onInterrupt() {
    // NOTE: For everything except the ESP32, you could simply use
    // `ExtSerial.digitalRead` here, but for the ESP32, we need to use the
    // 'External' version of this function to avoid conflicts with the built-in
    // pin functions.
    digitalWrite(LED_PIN, !ExtPort.digitalReadExternal(GPIO_PIN));
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

    // attach an interrupt to the pin on the expander
    // NOTE: For everything except the ESP32, you could simply use
    // `ExtSerial.attachInterrupt` here, but for the ESP32, we need to use the
    // 'Pin' version of this function to avoid conflicts with the built-in
    // interrupt functions.
    ExtPort.attachPinInterrupt(GPIO_PIN, onInterrupt);

    // set the pin mode for the LED pin
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    uint16_t callbackMask = ExtPort.getInterruptSource();
    if (callbackMask != SC16IS7XX_NO_INTERRUPT) {
#if defined(SC16IS7XX_DEBUG_SERIAL)
        ExtPort.printInterruptSource(callbackMask);
#endif  // SC16IS7XX_DEBUG_SERIAL
        ExtPort.handleInterrupt(callbackMask);
    }
    delay(100);
}
