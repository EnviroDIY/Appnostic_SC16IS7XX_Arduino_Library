/**
 * I2C UART GPIO Interrupt Test
 *
 * This example shows how to enable input on a pin and
 * configure interrupts for it. Note that the input pin
 * is active low.
 *
 * NOTE: On the Arduino Uno there are only two interrupt pins on
 * pins 2 and 3 which are not used by NOS8007.
 * On the R4 Minima all pins can support interrupts.
 *
 * Connect a button or switch to GPIO 0 of the UART interface module.
 *
 */

#include <Appnostic_SC16IS752.h>

Appnostic_SC16IS752 ExtSerial(SC16IS752_CHANNEL_A);

#define GPIO_PIN 0
#define NOS8007_IRQ 3

bool interrupted = false;

/**
 * @brief interrupt handler for NOS8007 IRQ
 */
void onInterrupt() {
    interrupted = true;
}

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
    ExtSerial.pinMode(GPIO_PIN, INPUT);

    // enable interrupts for the pin
    ExtSerial.setPinInterrupt(GPIO_PIN, true);

    // enable the interrupt controller
    ExtSerial.enableInterruptControl(true);

    // on the NOS10001 Arduino baseboard the IRQ of the NOS8007
    // is on pin D7
    pinMode(NOS8007_IRQ, INPUT);  // no pull required
    attachInterrupt(digitalPinToInterrupt(NOS8007_IRQ), onInterrupt,
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
