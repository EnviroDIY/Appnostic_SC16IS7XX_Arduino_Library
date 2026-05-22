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
#include <SC16IS7xx.h>

// The power pin for the external port expander, if necessary. Set to -1 if not
// used.
int8_t powerPin = -1;

// Create the port expander object and channel A serial interface reference
SC16IS752       ExtPort;
SC16IS7xx_UART& ExtSerial = ExtPort.uartA();

// The GPIO pin on the SC16IS7XX to use for the interrupt test
#define GPIO_PIN 0
// The pin on the Arduino to which the SC16IS7XX IRQ pin is connected
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
    if (!ExtPort.begin_i2c()) {
        Serial.println("not found. Please ensure that the module\r\nis plugged "
                       "in and securely fastened to the baseboard.");
        while (true) delay(100);
    }
    Serial.println("found!");

    // attach an interrupt to the pin on the expander
    // NOTE: For everything except the ESP32, you could simply use
    // `ExtSerial.attachInterrupt` here, but for the ESP32, we need to use the
    // 'Pin' version of this function to avoid conflicts with the built-in
    // interrupt functions.
    ExtPort.attachPinInterrupt(GPIO_PIN, onInterrupt);

// set the pin mode for the pin on the Arduino connected to the IRQ pin and
// attach the interrupt handler to it. The interrupt handler will be called when
// the IRQ pin goes low, which indicates that an interrupt has been triggered on
// the SC16IS7XX. The interrupt handler will then find the source of the
// interrupt and call the correct callback for it.
#if defined(digitalPinToInterrupt)
    attachInterrupt(digitalPinToInterrupt(SC16IS7XX_IRQ_PIN), onInterrupt,
                    FALLING);  // interrupt transitions from high to low
#else
    attachInterrupt(SC16IS7XX_IRQ_PIN, SC16IS7xx::interruptHandler,
                    FALLING);  // interrupt transitions from high to low
#endif
}

void loop() {
    if (interrupted == true) {
        interrupted = false;
        Serial.println("Interrupt received!");
    }
    delay(100);
}
