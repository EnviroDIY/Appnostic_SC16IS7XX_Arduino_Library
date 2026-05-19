/**
 * This is an Arduino library for the SC16IS7XX based on the
 * SC16IS752 dual UART chip from NXP.
 *
 * It is possible that this library may work with other vendor
 * devices using I2C or SPI but its primary purpose is for
 * Appnostic devices. Pull requests to improve compatibility are
 * welcomed but issues regarding other vendor devices may not receive
 * priority.
 *
 * Credits:
 * \@SandboxElectronics for most of the code
 * \@TD-er for the SC16IS752 patches
 *
 * Made with love by the Appnostic team!
 */

#include "SC16IS7XX.h"

/*** CONFIG *******************************************************/

/**
 * @brief sets the crystal frequency in hertz.
 * @note Defaults to 147456000 (Hz).  A 14.7456MHz crystal is commonly used with
 * the SC16IS7XX family and is the default for this library, but other
 * frequencies may be used. The crystal frequency is used to calculate baud
 * rates and should be set correctly for accurate baud rates.
 * @param frequency the frequency of the crystal in hertz
 */
void SC16IS7XX::setCrystalFrequency(uint32_t frequency) {
    crystal_frequency = frequency;
}

/**
 * @brief gets the xtal frequency in hertz.
 * @return the frequency of the crystal in hertz
 */
uint32_t SC16IS7XX::getCrystalFrequency() {
    return crystal_frequency;
}

/*** DEVICE *******************************************************/

/**
 * @brief derived function to reset the device
 */
void SC16IS7XX::resetDevice() {
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
bool SC16IS7XX::ping() {
    for (uint8_t channel = 0; channel < 2; channel++) {
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


bool SC16IS7XX::_init() {
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
 * address of #SC16IS7XX_DEFAULT_ADDRESS is used.
    @param  theWire Optional parameter for the I2C device we will use. Default
   is "Wire"
 * @return true if the device was successfully initialized, false otherwise
 */
bool SC16IS7XX::begin_i2c(uint8_t addr, TwoWire* theWire) {
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

    if (i2c_dev) delete i2c_dev;
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
bool SC16IS7XX::begin_SPI(uint8_t cs_pin, SPIClass* theSPI,
                          uint32_t frequency) {
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
bool SC16IS7XX::begin_SPI(int8_t cs_pin, int8_t sck_pin, int8_t miso_pin,
                          int8_t mosi_pin, uint32_t frequency) {
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


/*** GPIO **************************************************/

/**
 * @brief sets the io direction of a gpio pin
 * @param pin the output pin number on the port expander (0 - 7)
 * @param mode The pin mode, either INPUT or OUTPUT
 */
void SC16IS7XX::pinMode(uint8_t pin, uint8_t mode) {
    Adafruit_BusIO_Register     IODir(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                      SC16IS7XX_REG_IODIR << 3);
    Adafruit_BusIO_RegisterBits dir_bit(&IODir, 1, pin % 8);
    dir_bit.write((mode == OUTPUT) ? 0 : 1);
}

/**
 * @brief sets the state of the gpio pin
 * @param pin the output pin number on the port expander (0 - 7)
 * @param state the pin state, either LOW (0) or HIGH (1)
 */
void SC16IS7XX::digitalWrite(uint8_t pin, uint8_t state) {
    Adafruit_BusIO_Register     IOState(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                        SC16IS7XX_REG_IOSTATE << 3);
    Adafruit_BusIO_RegisterBits state_bit(&IOState, 1, pin % 8);
    state_bit.write(state);
}

/**
 * @brief returns the state of a gpio pin
 * @param pin the pin number on the port expander (0 - 7)
 * @return the pin state, either LOW (0) or HIGH (1)
 */
uint8_t SC16IS7XX::digitalRead(uint8_t pin) {
    Adafruit_BusIO_Register     IOState(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                        SC16IS7XX_REG_IOSTATE << 3);
    Adafruit_BusIO_RegisterBits state_bit(&IOState, 1, pin % 8);
    return state_bit.read();
}

/**
 * @brief sets the interrupt enable register to enable sleep mode
 *
 * Sleep mode is an enhanced feature of the SC16IS752/SC16IS762 UART. It is
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
void SC16IS7XX::enableSleepMode(bool enabled) {
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
bool SC16IS7XX::isSleepEnabled() {
    Adafruit_BusIO_Register     IER(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IER << 3);
    Adafruit_BusIO_RegisterBits sleep(&IER, 1, SC16IS7XX_IER_SLEEP);
    return sleep.read();
}

/**
 * @brief sets the interrupt enable register to enable CTS interrupts
 * @param enabled true enables interrupts, false disables them
 */
void SC16IS7XX::enableCTSInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IER << 3);
    Adafruit_BusIO_RegisterBits cts(&IER, 1, SC16IS7XX_IER_CTS);
    cts.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable RTS interrupts
 * @param enabled true enables interrupts, false disables them
 */
void SC16IS7XX::enableRTSInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IER << 3);
    Adafruit_BusIO_RegisterBits rts(&IER, 1, SC16IS7XX_IER_RTS);
    rts.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable XOFF interrupts
 * @param enabled true enables interrupts, false disables them
 */
void SC16IS7XX::enableXOFFInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IER << 3);
    Adafruit_BusIO_RegisterBits xoff(&IER, 1, SC16IS7XX_IER_XOFF);
    xoff.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable modem/pin change
 * interrupts
 * @param enabled true enables interrupts, false disables them
 */
void SC16IS7XX::enableModemInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IER << 3);
    Adafruit_BusIO_RegisterBits modem(&IER, 1, SC16IS7XX_IER_MODEM);
    modem.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable receive line status
 * interrupts
 * @param enabled true enables interrupts, false disables them
 */
void SC16IS7XX::enableRLSInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IER << 3);
    Adafruit_BusIO_RegisterBits rls(&IER, 1, SC16IS7XX_IER_RLS);
    rls.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable transmit holding register
 * interrupts
 * @param enabled true enables interrupts, false disables them
 */
void SC16IS7XX::enableTHRInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IER << 3);
    Adafruit_BusIO_RegisterBits thr(&IER, 1, SC16IS7XX_IER_THR);
    thr.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable receive holding register
 * interrupts
 * @param enabled true enables interrupts, false disables them
 */
void SC16IS7XX::enableRHRInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IER << 3);
    Adafruit_BusIO_RegisterBits rhr(&IER, 1, SC16IS7XX_IER_RHR);
    rhr.write(enabled);
}

/**
 * @brief configures the io interrupt register to generate an interrupt on pin
 * state change
 * @param pin the pin to configure an interrupt on
 * @param enabled true enables the interrupt, false disables it
 */
void SC16IS7XX::setPinInterrupt(uint8_t pin, bool enabled) {
    if (pin > 7) {
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
uint8_t SC16IS7XX::getPinInterrupt(uint8_t pin) {
    Adafruit_BusIO_Register     IOIntEna(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                         SC16IS7XX_REG_IOINTENA << 3);
    Adafruit_BusIO_RegisterBits enable_bit(&IOIntEna, 1, pin % 8);
    return enable_bit.read();
}

/**
 * @brief Gets the last pin to trigger an interrupt.
 * @warning This doesn't seem to be possible on this device, so this function is
 * not implemented and will always return -1.
 * @return the last pin that triggered an interrupt, or -1 if none
 */
int SC16IS7XX::getLastInterruptPin() {
    return -1;
}

/**
 * @brief used to determine interrupt source. it should really be fleshed out
 * better with callbacks.
 * @return the interrupt source as indicated by the interrupt identification
 * register
 */
uint8_t SC16IS7XX::isr() {
    uint8_t                 irq_src;
    Adafruit_BusIO_Register IIR(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                SC16IS7XX_REG_IIR << 3);

    irq_src = IIR.read() & 0x3E;
    //^ mask off the FCR[7:6] and interrupt pending [0] bits 0b00111110

    switch (irq_src) {
        case SC16IS7XX_INT_LINE:  // Receiver Line Status Error
            break;
        case SC16IS7XX_INT_TIMEOUT:  // Receiver time-out interrupt
            break;
        case SC16IS7XX_INT_RHR:  // RHR interrupt
            break;
        case SC16IS7XX_INT_THR:  // THR interrupt
            break;
        case SC16IS7XX_INT_MODEM:  // modem interrupt;
            break;
        case SC16IS7XX_INT_GPIO:  // input pin change of state
            break;
        case SC16IS7XX_INT_XOFF:  // XOFF
            break;
        case SC16IS7XX_INT_CTSRTS:  // CTS,RTS
            break;
        default: break;
    }

    return irq_src;
}

/**
 * @brief Write all GPIO output states at once.
 * @param state Bitmask written to the IOSTATE register.
 */
void SC16IS7XX::setPortState(uint8_t state) {
    Adafruit_BusIO_Register IOState(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IOSTATE << 3);
    IOState.write(state);
}

/**
 * @brief Read all GPIO states from the IOSTATE register.
 * @return uint8_t Current GPIO state bitmask.
 */
uint8_t SC16IS7XX::getPortState() {
    Adafruit_BusIO_Register IOState(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                    SC16IS7XX_REG_IOSTATE << 3);
    return IOState.read();
}

/**
 * @brief Set all GPIO direction bits at once.
 * @param mode Bitmask written to the IODIR register.
 */
void SC16IS7XX::setPortMode(uint8_t mode) {
    Adafruit_BusIO_Register IODir(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                  SC16IS7XX_REG_IODIR << 3);
    IODir.write(mode);
}

/**
 * @brief Read all GPIO direction bits from the IODIR register.
 * @return uint8_t Current GPIO direction bitmask.
 */
uint8_t SC16IS7XX::getPortMode() {
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
void SC16IS7XX::setGPIOLatch(bool enabled) {
    Adafruit_BusIO_Register     IOControl(i2c_dev, spi_dev, SC16IS7XX_SPIREG,
                                          SC16IS7XX_REG_IOCONTROL << 3);
    Adafruit_BusIO_RegisterBits latch_bit(&IOControl, 1, 0);
    latch_bit.write(enabled);
}

// cSpell:words SPIFREQ SPIREG MISO MOSI
