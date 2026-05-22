/**
 * @file SC16IS7xx.cpp
 * @copyright Stroud Water Research Center
 * Part of the EnviroDIY ModularSensors library for Arduino.
 * This library is published under the BSD-3 license.
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 *
 * @brief Implements the SC16IS7xx class.
 */

#include "SC16IS7xx.h"

// Pointer to active SC16IS7xx object
SC16IS7xx* SC16IS7xx::_activeObject = nullptr;

/**
 * @brief Construct a new SC16IS7xx::SC16IS7xx object
 *
 * @param uartChannelCount Number of UART channels on the device.
 * @param gpioPinCount Number of GPIO pins on the device.
 */
SC16IS7xx::SC16IS7xx(uint8_t uartChannelCount, uint8_t gpioPinCount)
    : _uartChannelCount(uartChannelCount > SC16IS7XX_MAX_UART_CHANNELS
                            ? SC16IS7XX_MAX_UART_CHANNELS
                            : uartChannelCount),
      _gpioPinCount(gpioPinCount > SC16IS7XX_MAX_GPIO_PINS
                        ? SC16IS7XX_MAX_GPIO_PINS
                        : gpioPinCount),
      nints(0) {
    for (uint8_t channel = 0; channel < SC16IS7XX_MAX_UART_CHANNELS;
         channel++) {
        _uartStorage[channel].configure(this, channel);
    }
}

/**
 * @brief Destroy the SC16IS7xx::SC16IS7xx object, setting the pointers to the
 * I2C and SPI devices to nullptr and deleting them if they exist. Also clears
 * the active object pointer if it points to this instance.
 */
SC16IS7xx::~SC16IS7xx() {
    if (i2c_dev) {
        delete i2c_dev;
        i2c_dev = nullptr;
    }
    if (spi_dev) {
        delete spi_dev;
        spi_dev = nullptr;
    }
    if (_activeObject == this) { _activeObject = nullptr; }
}

/**
 * @brief sets the GPIO pull direction.
 * @param pullUp true for pull-up, false for pull-down
 *
 * @note While the documentation doesn't clearly say, the GPIO pins on the
 * SC16IS7XX appear to have internal pull-up resistors.  I'm basing this on my
 * testing - where the GPIO pins all ready high unless manually pulled low, and
 * on the fact that when the GPIO pins are configured as modem pins, they are
 * all active low implying they are being pulled up internally.
 *
 * @warning Only change this if your board has external pull-down resistors on
 * the GPIO pins!
 */
void SC16IS7xx::setGPIOPullDirection(bool pullUp) {
    gpioPullDirection = pullUp;
}

/**
 * @brief gets the GPIO pull direction.
 * @return true if pull-up, false if pull-down
 */
bool SC16IS7xx::getGPIOPullDirection() {
    return gpioPullDirection;
}

/*** DEVICE *******************************************************/

/**
 * @brief derived function to reset the device
 */
void SC16IS7xx::resetDevice() {
    Adafruit_BusIO_Register     IOControl(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                          SC16IS7XX_REG_IOCONTROL << 3);
    Adafruit_BusIO_RegisterBits reset_bit(&IOControl, 1, 3);
    reset_bit.write(1);
}

/**
 * @brief tests the device to check if it is online by writing and reading from
 * the scratchpad register on both channels
 * @return true if the device is online, false otherwise
 */
bool SC16IS7xx::ping() {
    for (uint8_t channel = 0; channel < _uartChannelCount; channel++) {
        Adafruit_BusIO_Register SPR(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_SPR << 3 | channel << 1));
        // two attempts to write and read from the scratchpad register
        SPR.write(0x55);
        if (SPR.read() == 0x55) { return true; }
        SPR.write(0xAA);
        if (SPR.read() == 0xAA) { return true; }
    }
    return false;
}


bool SC16IS7xx::_init() {
    // empty interrupt registers and callbacks
    memset(ISRlist, 0, sizeof(ISRlist));
    memset(ISRcallback, 0, sizeof(ISRcallback));
    nints = 0;

    resetDevice();
    delayMicroseconds(100);  // let things settle
    return ping();
}

/*** I2C *********************************************************/

/**
 * @brief begins an i2c session for the target address
 * @param addr Optional parameter for the i2c address of the device. If the
 * address is between 0x48 and 0x57, it is used directly. Otherwise, it is right
 * shifted by one bit and used as the address. This allows for both 7-bit and
 * 8-bit address formats to be used.  If nothing is supplied, the default
 * address of #SC16IS7XX_DEFAULT_ADDRESS() is used.
    @param  theWire Optional parameter for the I2C device we will use. Default
   is "Wire"
 * @return true if the device was successfully initialized, false otherwise
 */
bool SC16IS7xx::begin_i2c(uint8_t addr, TwoWire* theWire) {
    uint8_t seven_bit_addr;
    // if we have one of the possible 7-bit addresses, use it directly
    if ((addr >= 0x48) && (addr <= 0x57)) {
        seven_bit_addr = addr;
    } else if ((addr >= 0x90) &&
               (addr <= 0xAF)) {  // if we have a possible 8-bit address, right
                                  // shift it to get the 7-bit address
        seven_bit_addr = (addr >> 1);
    } else {  // otherwise return false since the address is invalid
        return false;
    }

    // set this as the active object for interrupt handling before we initialize
    // the I2C
    if (_activeObject != this) { _activeObject = this; }

    if (i2c_dev) delete i2c_dev;
    i2c_dev = nullptr;
    if (spi_dev) delete spi_dev;
    spi_dev = nullptr;

    i2c_dev = new Adafruit_I2CDevice(seven_bit_addr, theWire);

    // verify i2c address was found
    if (!i2c_dev->begin()) { return false; }

    return _init();
}


/*** SPI *********************************************************/

/**
 *    @brief  Sets up the hardware and initializes hardware SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  theSPI The SPI object to be used for SPI connections.
 *    @param  frequency The SPI bus frequency
 *    @return True if initialization was successful, otherwise false.
 */
bool SC16IS7xx::begin_SPI(uint8_t cs_pin, SPIClass* theSPI,
                          uint32_t frequency) {
    // set this as the active object for interrupt handling before we initialize
    // the SPI
    if (_activeObject != this) { _activeObject = this; }

    if (i2c_dev) delete i2c_dev;
    if (spi_dev) delete spi_dev;
    i2c_dev = nullptr;

    spi_dev = new Adafruit_SPIDevice(cs_pin,
                                     frequency,              // frequency
                                     SPI_BITORDER_MSBFIRST,  // bit order
                                     SPI_MODE0,              // data mode
                                     theSPI);

    if (!spi_dev->begin()) { return false; }

    return _init();
}

/**
 *    @brief  Sets up the hardware and initializes software SPI
 *    @param  cs_pin The arduino pin # connected to chip select
 *    @param  sck_pin The arduino pin # connected to SPI clock
 *    @param  miso_pin The arduino pin # connected to SPI MISO
 *    @param  mosi_pin The arduino pin # connected to SPI MOSI
 *    @param  frequency The SPI bus frequency
 *    @return True if initialization was successful, otherwise false.
 */
bool SC16IS7xx::begin_SPI(int8_t cs_pin, int8_t sck_pin, int8_t miso_pin,
                          int8_t mosi_pin, uint32_t frequency) {
    // set this as the active object for interrupt handling before we initialize
    // the SPI
    if (_activeObject != this) { _activeObject = this; }

    if (i2c_dev) delete i2c_dev;
    if (spi_dev) delete spi_dev;
    i2c_dev = nullptr;

    spi_dev = new Adafruit_SPIDevice(cs_pin, sck_pin, miso_pin, mosi_pin,
                                     frequency,              // frequency
                                     SPI_BITORDER_MSBFIRST,  // bit order
                                     SPI_MODE0);             // data mode

    if (!spi_dev->begin()) { return false; }

    return _init();
}

/**
 * @brief Ends the current SC16IS7XX session by setting the active object
 * pointer to null.
 */
void SC16IS7xx::end() {
    if (this == _activeObject) { _activeObject = nullptr; }
}

SC16IS7xx_UART* SC16IS7xx::getUART(uint8_t channel) {
    if (channel >= _uartChannelCount) { return nullptr; }
    return &_uartStorage[channel];
}


/*** GPIO **************************************************/

/**
 * @brief sets the io direction of a gpio pin
 * @param pin the output pin number on the port expander (0 - 7)
 * @param mode The pin mode, either INPUT or OUTPUT
 *
 * @note For all cores *except* the ESP32, you can simply use
 * SC16IS7xx::pinMode, SC16IS7xx::digitalWrite, and SC16IS7xx::digitalRead
 * without the 'External' suffix. However, for the ESP32, use the 'External'
 * versions to avoid conflicts with built-in pin functions.
 *
 * @remark While the documentation doesn't clearly say, the GPIO pins on the
 * SC16IS7XX appear to have internal pull-up resistors.  I'm basing this on my
 * testing - where the GPIO pins all ready high unless manually pulled low, and
 * on the fact that when the GPIO pins are configured as modem pins, they are
 * all active low implying they are being pulled up internally.  There is no way
 * to change the pull resistor configuration.
 */
void SC16IS7xx::pinModeExternal(uint8_t pin, uint8_t mode) {
    if (pin >= _gpioPinCount) { return; }
    Adafruit_BusIO_Register     IODir(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                      SC16IS7XX_REG_IODIR << 3);
    Adafruit_BusIO_RegisterBits dir_bit(&IODir, 1, pin % 8);
    dir_bit.write((mode == OUTPUT) ? 1 : 0);
}

/**
 * @brief sets the state of the gpio pin
 * @param pin the output pin number on the port expander (0 - 7)
 * @param state the pin state, either LOW (0) or HIGH (1)
 *
 * @note For all cores *except* the ESP32, you can simply use
 * SC16IS7xx::pinMode, SC16IS7xx::digitalWrite, and SC16IS7xx::digitalRead
 * without the 'External' suffix. However, for the ESP32, use the 'External'
 * versions to avoid conflicts with built-in pin functions.
 */
void SC16IS7xx::digitalWriteExternal(uint8_t pin, uint8_t state) {
    if (pin >= _gpioPinCount) { return; }
    Adafruit_BusIO_Register     IOState(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                        SC16IS7XX_REG_IOSTATE << 3);
    Adafruit_BusIO_RegisterBits state_bit(&IOState, 1, pin % 8);
    state_bit.write(state);
}

/**
 * @brief returns the state of a gpio pin
 * @param pin the pin number on the port expander (0 - 7)
 * @return the pin state, either LOW (0) or HIGH (1)
 *
 * @note For all cores *except* the ESP32, you can simply use
 * SC16IS7xx::pinMode, SC16IS7xx::digitalWrite, and SC16IS7xx::digitalRead
 * without the 'External' suffix. However, for the ESP32, use the 'External'
 * versions to avoid conflicts with built-in pin functions.
 *
 * @warning If GPIO interrups are configured for latching, the state returned by
 * this function will not be valid.
 */
uint8_t SC16IS7xx::digitalReadExternal(uint8_t pin) {
    if (pin >= _gpioPinCount) { return LOW; }
    Adafruit_BusIO_Register     IOState(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                        SC16IS7XX_REG_IOSTATE << 3);
    Adafruit_BusIO_RegisterBits state_bit(&IOState, 1, pin % 8);
    return state_bit.read();
}

/**
 * @brief sets the interrupt enable register to enable sleep mode
 *
 * Sleep mode is an enhanced feature of the SC16IS7xx/SC16IS762 UART. It is
 * enabled when EFR[4], the enhanced functions bit, is set and when IER[4] is
 * set. Sleep mode is entered when:
 *   - The serial data input line, RX, is idle
 *   - The TX FIFO and TX shift register are empty
 *   - There are no interrupts pending except THR
 *   - There is no data in the RX FIFO
 *
 * In Sleep mode, the clock to the UART is stopped. Since most registers are
 * clocked using these clocks, the power consumption is greatly reduced. The
 * UART will wake up when any change is detected on the RX line, when there is
 * any change in the state of the modem input pins, or if data is written to the
 * TX FIFO.
 *
 * @remark Writing to the divisor latches DLL and DLH to set the baud clock must
 * not be done during Sleep mode. Therefore, it is advisable to disable Sleep
 * mode using IER[4] before writing to DLL or DLH.
 *
 * @param enabled true enables sleep mode, false disables it
 */
void SC16IS7xx::enableSleepMode(bool enabled) {
    Adafruit_BusIO_Register     IER(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IER << 3);
    Adafruit_BusIO_RegisterBits sleep(&IER, 1, SC16IS7XX_IER_SLEEP);
    sleep.write(enabled);
}

/**
 * @brief Checks if sleep mode is enabled by reading the interrupt enable
 * register.
 *
 * @note This is NOT a check for whether the device is currently in sleep mode,
 * but rather whether sleep mode is enabled and can be entered when the
 * conditions for sleep mode are met.
 *
 * @return True if sleep mode is enabled, false otherwise
 */
bool SC16IS7xx::isSleepEnabled() {
    Adafruit_BusIO_Register     IER(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IER << 3);
    Adafruit_BusIO_RegisterBits sleep(&IER, 1, SC16IS7XX_IER_SLEEP);
    return sleep.read();
}

/**
 * @brief configures the io interrupt register to generate an interrupt on pin
 * state change
 * @param pin the pin to configure an interrupt on
 * @param enabled true enables the interrupt, false disables it
 *
 * The I/O pin interrupt (IIR = 0bxx110000 = 0x30) is triggered when there is a
 * change in the state of an I/O pin with interrupts enabled in the IOIntEna
 * register.
 *
 * The I/O interrupt clear depends on the interrupt latching configuration.
 *  - If interrupt latching is not enabled, the I/O interrupt is cleared by
 * reading the IIR register or by a change in the state of the pin that
 * triggered the interrupt.
 *  - If interrupt latching is enabled, the modem interrupt is cleared by
 * reading the IOState register.
 *
 * The I/O interrupt only works if the pin is configured to be an I/O pin and
 * not a modem pin in the IOControl register. Pins 0-3 are controlled by bit 2
 * of the IOControl register, while pins 4-7 are controlled by bit 1. Therefore,
 * to enable an interrupt.
 */
void SC16IS7xx::setPinInterrupt(uint8_t pin, bool enabled) {
    if (pin >= _gpioPinCount) {
        // Invalid pin number, do nothing or handle error as needed
        return;
    }

    // Ensure that the pin is set to be an I/O pin and not a modem pin
    Adafruit_BusIO_Register IOControl(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                      SC16IS7XX_REG_IOCONTROL << 3);
    //   pins 0-3 are controlled by bit 2, pins 4-7 are controlled by bit 1
    Adafruit_BusIO_RegisterBits modem_pin_bit(&IOControl, 1, pin > 3 ? 1 : 2);
    modem_pin_bit.write(0);

    // Now enable or disable the interrupt for the specific pin
    Adafruit_BusIO_Register     IOIntEna(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                         SC16IS7XX_REG_IOINTENA << 3);
    Adafruit_BusIO_RegisterBits enable_bit(&IOIntEna, 1, pin % 8);
    enable_bit.write(enabled);
}

/**
 * @brief returns whether or not an interrupt is enabled for a specific pin
 * @param pin the pin number on the port expander (0 - 7)
 * @return the interrupt enable status, either LOW (0) or HIGH (1)
 */
uint8_t SC16IS7xx::getPinInterrupt(uint8_t pin) {
    if (pin >= _gpioPinCount) { return LOW; }
    Adafruit_BusIO_Register     IOIntEna(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                         SC16IS7XX_REG_IOINTENA << 3);
    Adafruit_BusIO_RegisterBits enable_bit(&IOIntEna, 1, pin % 8);
    return enable_bit.read();
}

/**
 * @brief Write all GPIO output states at once.
 * @param state Bitmask written to the IOSTATE register.
 */
void SC16IS7xx::setPortState(uint8_t state) {
    Adafruit_BusIO_Register IOState(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IOSTATE << 3);
    IOState.write(state);
}

/**
 * @brief Read all GPIO states from the IOSTATE register.
 * @return uint8_t Current GPIO state bitmask.
 *
 * @warning If GPIO interrups are configured for latching, the states returned
 * by this function will not be valid.
 */
uint8_t SC16IS7xx::getPortState() {
    Adafruit_BusIO_Register IOState(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IOSTATE << 3);
    return IOState.read();
}

/**
 * @brief Set all GPIO direction bits at once.
 * @param mode Bitmask written to the IODIR register.
 */
void SC16IS7xx::setPortMode(uint8_t mode) {
    Adafruit_BusIO_Register IODir(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                  SC16IS7XX_REG_IODIR << 3);
    IODir.write(mode);
}

/**
 * @brief Read all GPIO direction bits from the IODIR register.
 * @return uint8_t Current GPIO direction bitmask.
 */
uint8_t SC16IS7xx::getPortMode() {
    Adafruit_BusIO_Register IODir(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                  SC16IS7XX_REG_IODIR << 3);
    return IODir.read();
}

/**
 * @brief Enable or disable GPIO state latching.
 * @param enabled True to enable latching, false to disable.
 *
 * Latching Disabled:
 *   - A change in any input generates an interrupt. A read of the input
 * register clears the interrupt. If the input goes back to its initial logic
 * state before the input register is read, then the interrupt is cleared.
 *
 * Latching Enabled:
 *   - A change in the input generates an interrupt and the input logic value is
 * loaded in the bit of the corresponding input state register (IOState). A read
 * of the IOState register clears the interrupt. If the input pin goes back to
 * its initial logic state before the interrupt register is read, then the
 * interrupt is not cleared and the corresponding bit of the IOState register
 * keeps the logic value that initiates the interrupt.
 */
void SC16IS7xx::setGPIOLatch(bool enabled) {
    Adafruit_BusIO_Register     IOControl(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                          SC16IS7XX_REG_IOCONTROL << 3);
    Adafruit_BusIO_RegisterBits latch_bit(&IOControl, 1, 0);
    latch_bit.write(enabled);
    gpioInterruptsLatched = enabled;
}


/**
 * @brief Specifies a named Interrupt Service Routine (ISR) to call when a
 * pin-change interrupt occurs. Replaces any previous function that was attached
 * to the interrupt.
 * @param callbackMask a bitmask representing the interrupt to attach the
 * callback to.
 * @param callback the function to call when the interrupt occurs
 */
void SC16IS7xx::storeCallback(uint16_t callbackMask, voidFxnPtr callback) {
    // Store the interrupt callback.
    // Only store when there is really an ISR to call.
    // This allow for calling attachInterrupt(pin, NULL, mode), we set up all
    // needed register but won't service the interrupt, this way we also don't
    // need to check it inside the ISR.
    if (callback) {
        // Store interrupts to service in order of when they were attached
        // to allow for first come first serve handler
        uint8_t current = 0;

        // Check if we already have this interrupt
        for (current = 0; current < nints; current++) {
            if (ISRlist[current] == callbackMask) { break; }
        }
        if (current == nints) {
            // Need to make a new entry
            if (nints >= (_gpioPinCount + SC16IS7XX_NON_GPIO_INTERRUPTS)) {
                return;
            }
            nints++;
        }
        ISRlist[current] = callbackMask;  // List of interrupt in order of when
                                          // they were attached
        ISRcallback[current] = callback;  // List of callback addressess
    }
}

/**
 * @brief Turns off the given interrupt.
 * @param callbackMask a bitmask representing the interrupt to detach the
 * callback from.
 */
void SC16IS7xx::clearCallback(uint16_t callbackMask) {
    // Remove callback from the ISR list
    uint8_t current;
    for (current = 0; current < nints; current++) {
        if (ISRlist[current] == callbackMask) { break; }
    }
    if (current == nints) return;  // We didn't have it

    // Shift the remainder down
    for (; current < nints - 1; current++) {
        ISRlist[current]     = ISRlist[current + 1];
        ISRcallback[current] = ISRcallback[current + 1];
    }
    nints--;
}

/**
 * @brief Calls the callback function associated with the given interrupt.
 * @param callbackMask a bitmask representing the interrupt to call the
 * callback for.
 */
void SC16IS7xx::callCallback(uint16_t callbackMask) {
    // Find the position of the interrupt in the ISR list
    uint8_t current;
    for (current = 0; current < nints; current++) {
        if (ISRlist[current] == callbackMask) { break; }
    }
    // if we didn't have that callback
    if (current == nints) {
#if defined(SC16IS7XX_DEBUG_SERIAL)
        SC16IS7XX_DEBUG_SERIAL.println("======Matching callback not found!");
#endif  // SC16IS7XX_DEBUG_SERIAL
        return;
    }

    // Call the callback function if found
    if (ISRcallback[current]) { ISRcallback[current](); }
}


/**
 * @brief Specifies a named Interrupt Service Routine (ISR) to call when a
 * pin-change interrupt occurs. Replaces any previous function that was attached
 * to the interrupt.
 * @param pin the pin number on the port expander (0 - 7)
 * @param callback the function to call when the interrupt occurs
 * @remark The SC16IS7XX family of devices only supports pin change interrupts
 * and does not support pull-up or pull-down resistors, so anything put in the
 * mode parameter will be ignored. The interrupt will be triggered on any change
 * of the pin state.
 * @warning If GPIO interrupts are not configured for latching, it is not
 * possible to determine which pin triggered the interrupt.  This means any
 * callback attached using this function will be called when any pin change
 * interrupt occurs, regardless of which pin triggered the interrupt.  If you
 * attempt to attach different functions to different pins without latching
 * enabled, whichever function is attached last will be called.  To determine
 * which pin triggered the interrupt, GPIO interrupts must be configured for
 * latching using setGPIOLatch(true).  Unfortunately, enabling GPIO latching
 * means that you cannot read the current value of any pin using
 * digitalReadExternal or getPortState.
 */
void SC16IS7xx::attachPinInterrupt(uint8_t pin, voidFxnPtr callback, uint8_t) {
    if (pin >= _gpioPinCount) {
        // Invalid pin number, do nothing or handle error as needed
        return;
    }

    uint16_t callbackMask;

    if (gpioInterruptsLatched) {
        // If GPIO interrupts are latched, the callback mask includes the pin
        // number so that we can determine which pin triggered the interrupt in
        // the ISR.
        callbackMask = SC16IS7XX_IIR_GPIO << 8 | (1 << pin);
    } else {
        // If GPIO interrupts are not latched, the callback mask does not
        // include the pin number since we won't be able to determine which pin
        // triggered the interrupt.
        callbackMask = SC16IS7XX_IIR_GPIO << 8;
    }

    // Store the interrupt callback.
    storeCallback(callbackMask, callback);

    // Enable pin interrupt for the pin
    setPinInterrupt(pin, true);
}

/**
 * @brief Turns off the given interrupt.
 * @param pin the pin number on the port expander (0 - 7)
 */
void SC16IS7xx::detachPinInterrupt(uint8_t pin) {
    if (pin >= _gpioPinCount) {
        // Invalid pin number, do nothing or handle error as needed
        return;
    }

    // disable pin interrupt for the pin
    setPinInterrupt(pin, false);

    // Clear the appropriate callback mask based on the current latching mode:
    // per-pin form when latched, global form when not latched.
    uint16_t pinMask      = 1 << pin;
    uint16_t callbackMask = SC16IS7XX_IIR_GPIO << 8 | pinMask;
    uint16_t globalMask   = SC16IS7XX_IIR_GPIO << 8;

    // clear the callback for the interrupt
    if (gpioInterruptsLatched) {
        clearCallback(callbackMask);
    } else {
        clearCallback(globalMask);
    }
}


#if defined(SC16IS7XX_DEBUG_SERIAL)
/**
 * @brief Print out the source of the interrupt
 * @param callbackMask A bitmask representing the source of the interrupt, used
 * to find the appropriate callback in the list of callbacks.
 */
void SC16IS7xx::printInterruptSource(uint16_t callbackMask) {
    uint16_t callbackIIR = callbackMask >> 8;  // upper byte is the IIR source

    switch (callbackIIR) {
        case SC16IS7XX_NO_INTERRUPT >> 8: {
            SC16IS7XX_DEBUG_SERIAL.println("No interrupt pending");
            break;
        }

        case SC16IS7XX_IIR_MODEM: {
            switch (callbackMask) {
                case SC16IS7XX_INT_MASK_RI: {
                    SC16IS7XX_DEBUG_SERIAL.println("Ring Indicator Interrupt");
                    break;
                }
                case SC16IS7XX_INT_MASK_CD: {
                    SC16IS7XX_DEBUG_SERIAL.println("Carrier Detect Interrupt");
                    break;
                }
                case SC16IS7XX_INT_MASK_DSR: {
                    SC16IS7XX_DEBUG_SERIAL.println("Data Set Ready Interrupt");
                    break;
                }
                case SC16IS7XX_INT_MASK_DTR: {
                    SC16IS7XX_DEBUG_SERIAL.println(
                        "Data Terminal Ready Interrupt");
                    break;
                }
            }
            break;
        }

        case SC16IS7XX_IIR_CTSRTS: {
            switch (callbackMask) {
                case SC16IS7XX_INT_MASK_CTS: {
                    SC16IS7XX_DEBUG_SERIAL.println("Clear to Send Interrupt");
                    break;
                }
                case SC16IS7XX_INT_MASK_RTS: {
                    SC16IS7XX_DEBUG_SERIAL.println("Ready to Send Interrupt");
                    break;
                }
            }
            break;
        }

        case SC16IS7XX_IIR_XOFF: {
            SC16IS7XX_DEBUG_SERIAL.println("XOFF Interrupt");
            break;
        }
        case SC16IS7XX_IIR_LINE: {
            SC16IS7XX_DEBUG_SERIAL.println("Line status error");
            break;
        }
        case SC16IS7XX_IIR_THR: {
            SC16IS7XX_DEBUG_SERIAL.println("THR interrupt");
            break;
        }
        case SC16IS7XX_IIR_RHR: {
            SC16IS7XX_DEBUG_SERIAL.println("RHR interrupt");
            break;
        }
        case SC16IS7XX_IIR_TIMEOUT: {
            SC16IS7XX_DEBUG_SERIAL.println("Receiver time-out");
            break;
        }

        case SC16IS7XX_IIR_GPIO: {
            if (!gpioInterruptsLatched) {
                SC16IS7XX_DEBUG_SERIAL.println(
                    "GPIO pin change interrupt; unable to determine which pin "
                    "triggered the interrupt.");
                SC16IS7XX_DEBUG_SERIAL.println(
                    "Use GPIO interrupt latching to determine which pin "
                    "triggered the interrupt.");
                break;
            }

            for (uint8_t pin = 0; pin < _gpioPinCount; pin++) {
                if (((callbackMask & 0x00FFu) & (1u << pin)) == (1u << pin)) {
                    SC16IS7XX_DEBUG_SERIAL.print("GPIO pin change interrupt");
                    SC16IS7XX_DEBUG_SERIAL.print(", pin ");
                    SC16IS7XX_DEBUG_SERIAL.println(pin);
                }
            }
            break;
        }

        default: {
            SC16IS7XX_DEBUG_SERIAL.print("Unknown interrupt source");
            SC16IS7XX_DEBUG_SERIAL.print(", callback mask hex: 0x");
            SC16IS7XX_DEBUG_SERIAL.print(callbackMask, HEX);
            SC16IS7XX_DEBUG_SERIAL.print(", bin: 0b");
            SC16IS7XX_DEBUG_SERIAL.println(callbackMask, BIN);
            break;
        }
    }
}
#endif  // SC16IS7XX_DEBUG_SERIAL


/**
 * @brief Get a bitmask representing the source of the interrupt that can be
 * used to find the appropriate callback in the list of callbacks.
 * @return A bitmask representing the source of the interrupt. The upper byte
 * contains the IIR source, while the lower byte contains additional information
 * about the source (e.g. which pin triggered a GPIO interrupt or the delta bits
 * for a modem interrupt).
 *
 * @warning Calling this function will **clear** most types of interrupts. It
 * will return a different result if polled twice in a row.  If you want to use
 * the result of this function in more than one place, you must store it in a
 * variable instead of calling this function multiple times.
 */
uint16_t SC16IS7xx::getInterruptSource(void) {
    // Calling the routine directly from -here- takes about 1us
    // Depending on where you are in the list it will take longer
    Adafruit_BusIO_Register IIR(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                SC16IS7XX_REG_IIR << 3);

    // read the interrupt source register, store it as 16 bits to have room to
    // store additional information about the interrupt source in the lower byte
    uint16_t irq_reg = IIR.read();

#if defined(SC16IS7XX_DEBUG_SERIAL)
    SC16IS7XX_DEBUG_SERIAL.print("==Interrupt Identification Register");
    SC16IS7XX_DEBUG_SERIAL.print(" hex: 0x");
    SC16IS7XX_DEBUG_SERIAL.print(irq_reg, HEX);
    SC16IS7XX_DEBUG_SERIAL.print(", bin: 0b");
    SC16IS7XX_DEBUG_SERIAL.println(irq_reg, BIN);
#endif  // SC16IS7XX_DEBUG_SERIAL

    // if there's no interrupt, return no interrupt mask
    // bit 0 is the interrupt pending bit, 1 means there's no interrupt
    if ((irq_reg & 0x01) == 1) {
#if defined(SC16IS7XX_DEBUG_SERIAL)
        SC16IS7XX_DEBUG_SERIAL.println("====No interrupt pending");
#endif  // SC16IS7XX_DEBUG_SERIAL
        return SC16IS7XX_NO_INTERRUPT;
    }

    // mask off the FCR mirror [7:6] and interrupt pending [0] bits with 0x3E =
    // 0b00111110 to get the interrupt source
    uint16_t irq_src = irq_reg & 0x3E;

    // Push the source of the interrupt into the upper byte of the callback mask
    uint16_t callbackMask = irq_src << 8;

    switch (irq_src) {
        // Receiver Line Status Error - user must read all errored characters
        // from the RX FIFO to clear
        case SC16IS7XX_IIR_LINE:
        // Receiver time-out interrupt interrupt will be cleared by the next
        // stop bit or by reading the IIR register
        case SC16IS7XX_IIR_TIMEOUT:
        // RHR interrupt - user must read all enough from the RX FIFO to free
        // space to clear
        case SC16IS7XX_IIR_RHR:
            // THR interrupt - the chip must successfully send enough data to
            // free space in the TX FIFO to clear
        case SC16IS7XX_IIR_THR:
        // XOFF interrupt is cleared by reading the IIR register or when an XON
        // character is received
        case SC16IS7XX_IIR_XOFF:
            // Only one type of interrupt with these IIR sources, no XOR needed
            // to differentiate.
            break;

        // modem interrupt (CD, RI, DSR, DTR) is cleared by reading the MSR
        // register or when the pin state changes again
        case SC16IS7XX_IIR_MODEM:
        // CTS,RTS is cleared by reading the MSR register or when the pin state
        // changes again
        case SC16IS7XX_IIR_CTSRTS: {
            // Get the modem status to differentiate between modem and
            // CTS/RTS interrupts since they share the same IIR code
            Adafruit_BusIO_Register modemStatusReg(
                i2c_dev, spi_dev, SC16IS7XX_SPIREG, SC16IS7XX_REG_MSR << 3);
            uint8_t modemStatus = modemStatusReg.read() & 0x0F;
            //^ keep the bottom 4 bits which are the delta bits
#if defined(SC16IS7XX_DEBUG_SERIAL)
            SC16IS7XX_DEBUG_SERIAL.print("==Modem Status Register");
            SC16IS7XX_DEBUG_SERIAL.print(" hex: 0x");
            SC16IS7XX_DEBUG_SERIAL.print(modemStatus, HEX);
            SC16IS7XX_DEBUG_SERIAL.print(", bin: 0b");
            SC16IS7XX_DEBUG_SERIAL.println(modemStatus, BIN);
#endif  // SC16IS7XX_DEBUG_SERIAL
        // tack the modem status bits onto the callback mask
            callbackMask |= modemStatus;
            break;
        }

        // pin change interrupt, cleared by reading the IOState register or when
        // the pin state changes again - unless the interrupt is latched, then
        // it is only cleared by reading the IOState register
        case SC16IS7XX_IIR_GPIO: {
            // if the pin change interrupt is not latched, we cannot know which
            // pin triggered the interrupt
            if (!gpioInterruptsLatched) { break; }

            // Get which pin from the input register
            uint8_t iostate = getPortState();
            // flip the state if needed
            uint16_t iostate_i = iostate & 0xFF;
            if (gpioPullDirection) { iostate_i = ~iostate & 0xFF; }
#if defined(SC16IS7XX_DEBUG_SERIAL)
            SC16IS7XX_DEBUG_SERIAL.print("==IO State Register");
            SC16IS7XX_DEBUG_SERIAL.print(" hex: 0x");
            SC16IS7XX_DEBUG_SERIAL.print(iostate, HEX);
            SC16IS7XX_DEBUG_SERIAL.print(", bin: 0b");
            SC16IS7XX_DEBUG_SERIAL.println(iostate, BIN);
#endif  // SC16IS7XX_DEBUG_SERIAL
        // tack the io bits onto the callback mask
            callbackMask |= iostate_i;
            break;
        }
        default: break;
    }

#if defined(SC16IS7XX_DEBUG_SERIAL)
    SC16IS7XX_DEBUG_SERIAL.print("====Interrupt source callback mask");
    SC16IS7XX_DEBUG_SERIAL.print(" hex: 0x");
    SC16IS7XX_DEBUG_SERIAL.print(callbackMask, HEX);
    SC16IS7XX_DEBUG_SERIAL.print(", bin: 0b");
    SC16IS7XX_DEBUG_SERIAL.println(callbackMask, BIN);
#endif  // SC16IS7XX_DEBUG_SERIAL

    return callbackMask;
}

/**
 * @brief  This calls the appropriate callback function if it is found in the
 * list of callbacks.
 *
 * You should first call getInterruptSource() to get the source of the
 * interrupt, then pass the result of that function to this function to call the
 * appropriate callback.  Do **not** call the getInterruptSource() function
 * multiple times in a row, as it will clear most types of interrupts.  Store
 * the result of getInterruptSource() in a variable if you need to use it in
 * more than one place.
 *
 * If you are attaching an interrupt to a pin on your MCU, you should use the
 * interruptHandler() function instead.
 *
 * @param callbackMask A bitmask representing the source of the interrupt, used
 * to find the appropriate callback in the list of callbacks.
 *
 * On Espressif boards (ESP8266 and ESP32), the ISR must be stored in IRAM
 */
void ISR_MEM_ACCESS SC16IS7xx::handleInterrupt(uint16_t callbackMask) {
    // Multiple GPIO pins can be masked in the same callbackMask, but we want to
    // call the appropriate callback for each pin, so we need to loop through
    // the pins and call the callback for each one.  For
    if (((callbackMask >> 8) == SC16IS7XX_IIR_GPIO) && gpioInterruptsLatched) {
        for (uint8_t pin = 0; pin < _gpioPinCount; pin++) {
            if (((callbackMask & 0x00FFu) & (1u << pin)) == (1u << pin)) {
                uint16_t searchCallbackMask = SC16IS7XX_IIR_GPIO << 8 |
                    (1u << pin);
                callCallback(searchCallbackMask);
            }
        }
    } else {
        callCallback(callbackMask);
    }
}

/**
 * @brief Intermediary used by the ISR - passes off responsibility for the
 * interrupt to the active object.
 *
 * This function is intended to be called by the actual ISR that is attached to
 * the interrupt pin on your MCU. It will delegate the handling of the interrupt
 * to the active SC16IS7XX object.
 *
 * On Espressif boards (ESP8266 and ESP32), the ISR must be stored in IRAM
 */
void ISR_MEM_ACCESS SC16IS7xx::interruptHandler() {
    if (_activeObject)
        _activeObject->handleInterrupt(_activeObject->getInterruptSource());
}

// cSpell:words SPIFREQ SPIREG MISO MOSI
