/**
 * I2C UART GPIO Interrupt Test
 *
 * As an alterantive to enabling interrupts on the Arduino this
 * example shows how to poll the interrupt pin.
 *
 * Connect a button or switch to GPIO 0 of the UART interface module.
 * When the button is pressed, LED2 should turn on.
 *
 */

#include <Appnostic_SC16IS752.h>

Appnostic_SC16IS752 ExtSerial(SC16IS752_CHANNEL_A);

#define GPIO_PIN 0
#define NOS8007_IRQ 3

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

    // enable LED2 on the NOS10001 baseboard
    pinMode(9, OUTPUT);
}

void loop() {
    if (digitalRead(NOS8007_IRQ) == LOW) {
        if (ExtSerial.isr() == SC16IS7XX_INT_GPIO) {
            digitalWrite(9, !ExtSerial.digitalRead(GPIO_PIN));
        }
    }
    delay(100);
}
