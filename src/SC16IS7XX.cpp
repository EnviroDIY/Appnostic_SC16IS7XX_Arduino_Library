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

#ifdef __AVR__
#define WIRE Wire
#define SPI_SS PIN_SPI_SS
#elif defined(ARDUINO_MINIMA)  // SC16IS7XX minima r4 support
#define WIRE Wire
#define SPI_SS PIN_SPI_CS
#elif defined(ESP8266) || defined(ESP32)  // ESP8266/ESP32
#define WIRE Wire
#define SPI_SS PIN_SPI_SS
#elif ESP32  // ESP8266
#define WIRE Wire
#define SPI_SS PIN_SPI_SS
#else  // Arduino Due
#define WIRE Wire1
#define SPI_SS PIN_SPI_SS
#endif  // ifdef __AVR__

bool SC16IS7XX::_initialized = false;

/*** REGISTERS *****************************************************/

/**
 * @brief writes to the register of the device via i2c or spi
 * @param reg_addr the address of the register to write to
 * @param val the value to write to the register
 */
void SC16IS7XX::writeRegister(uint8_t reg_addr, uint8_t val) {
    if (device_protocol == SC16IS7XX_PROTOCOL_I2C) {
        WIRE.beginTransmission(device_address);
        WIRE.write(reg_addr);
        WIRE.write(val);
        WIRE.endTransmission(true);
    } else {
        ::digitalWrite(device_address, LOW);
        delayMicroseconds(10);
        SPI.transfer(reg_addr);
        SPI.transfer(val);
        delayMicroseconds(10);
        ::digitalWrite(device_address, HIGH);
    }
}

/**
 * @brief reads a register from the device via i2c or spi
 * @param reg_addr the address of the register to write to
 * @return the value read from the register
 */
uint8_t SC16IS7XX::readRegister(uint8_t reg_addr) {
    uint8_t result = 0;

    if (device_protocol == SC16IS7XX_PROTOCOL_I2C) {
        WIRE.beginTransmission(device_address);
        WIRE.write(reg_addr);
        WIRE.endTransmission(false);
        WIRE.requestFrom(device_address, (uint8_t)1);
        result = WIRE.read();
    } else {
        ::digitalWrite(device_address, LOW);
        delayMicroseconds(10);
        SPI.transfer(0x80 | reg_addr);
        result = SPI.transfer(0xff);
        delayMicroseconds(10);
        ::digitalWrite(device_address, HIGH);
    }

    return result;
}

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
    uint8_t reg;

    reg = readRegister(SC16IS7XX_REG_IOCONTROL << 3);
    reg |= 0x08;
    writeRegister(SC16IS7XX_REG_IOCONTROL << 3, reg);
}

/*** I2C *********************************************************/

/**
 * @brief begins an i2c session for the target address
 * @param addr the i2c address of the device. If the address is between 0x48 and
 * 0x57, it is used directly. Otherwise, it is right shifted by one bit and used
 * as the address. This allows for both 7-bit and 8-bit address formats to be
 * used.
 * @return true if the device was successfully initialized, false otherwise
 */
bool SC16IS7XX::begin_i2c(uint8_t addr) {
    if ((addr >= 0x48) && (addr <= 0x57)) {
        device_address = addr;
    } else {
        device_address = (addr >> 1);
    }
    device_protocol = SC16IS7XX_PROTOCOL_I2C;

    if (_initialized == true) {
        return true;  // i2c already running
    }

    WIRE.begin();  // start i2c
    WIRE.setClock(400000);
    resetDevice();
    delayMicroseconds(100);  // let things settle
    _initialized = true;
    return ping();
}

/**
 * @brief shorthand method to start i2c with the SC16IS7XX default address
 * @return true if the device was successfully initialized, false otherwise
 */
bool SC16IS7XX::begin_i2c() {
    return begin_i2c(SC16IS7XX_ADDRESS_AA);
}

/*** SPI *********************************************************/

/**
 * @brief sets up SPI
 * @note untested, but assumed working
 * @param cs the chip select pin to use for SPI communication
 * @return true if the device was successfully initialized, false otherwise
 */
bool SC16IS7XX::begin_spi(uint8_t cs) {
    device_protocol = SC16IS7XX_PROTOCOL_SPI;
    device_address  = cs;

    if (_initialized == true) {
        return true;  // spi already running
    }

    ::pinMode(device_address, OUTPUT);
    ::digitalWrite(device_address, HIGH);
    SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
    SPI.begin();
    resetDevice();
    delayMicroseconds(100);  // let things settle
    _initialized = true;
    return ping();
}

/**
 * @brief shorthand to start spi at default CS pin
 * @return true if the device was successfully initialized, false otherwise
 */
bool SC16IS7XX::begin_spi() {
    return begin_spi(SPI_SS);
}

/*** GPIO **************************************************/

/**
 * @brief sets the io direction of a gpio pin
 * @param pin the output pin number on the port expander (0 - 7)
 * @param mode The pin mode, either INPUT or OUTPUT
 */
void SC16IS7XX::pinMode(uint8_t pin, uint8_t mode) {
    uint8_t tmp_iodir;

    tmp_iodir = readRegister(SC16IS7XX_REG_IODIR << 3);

    if (mode == OUTPUT) {
        tmp_iodir |= (0x01 << pin);
    } else {
        tmp_iodir &= (uint8_t)~(0x01 << pin);
    }

    writeRegister(SC16IS7XX_REG_IODIR << 3, tmp_iodir);
}

/**
 * @brief sets the state of the gpio pin
 * @param pin the output pin number on the port expander (0 - 7)
 * @param state the pin state, either LOW (0) or HIGH (1)
 */
void SC16IS7XX::digitalWrite(uint8_t pin, uint8_t state) {
    uint8_t tmp_iostate;

    tmp_iostate = readRegister(SC16IS7XX_REG_IOSTATE << 3);

    if (state == 1) {
        tmp_iostate |= (0x01 << pin);
    } else {
        tmp_iostate &= (uint8_t)~(0x01 << pin);
    }

    writeRegister(SC16IS7XX_REG_IOSTATE << 3, tmp_iostate);
}

/**
 * @brief returns the state of a gpio pin
 * @param pin the pin number on the port expander (0 - 7)
 * @return the pin state, either LOW (0) or HIGH (1)
 */
uint8_t SC16IS7XX::digitalRead(uint8_t pin) {
    uint8_t tmp_iostate;

    tmp_iostate = readRegister(SC16IS7XX_REG_IOSTATE << 3);

    if ((tmp_iostate & (0x01 << pin)) == 0) { return 0; }
    return 1;
}

/**
 * @brief sets the interrupt enable register to enable interrupts
 * @note enables all six types of interrupts
 * @param enabled true enables interrupts, false disables them
 */
void SC16IS7XX::enableInterruptControl(bool enabled) {
    writeRegister(SC16IS7XX_REG_IER << 3, enabled);
}

/**
 * @brief configures the io interrupt register to generate an interrupt on pin
 * state change
 * @param pin the pin to configure an interrupt on
 * @param enabled true enables the interrupt, false disables it
 */
void SC16IS7XX::setPinInterrupt(uint8_t pin, bool enabled) {
    uint8_t tmp_iostate;

    tmp_iostate = readRegister(SC16IS7XX_REG_IOINTENA << 3);

    if (enabled == true) {
        tmp_iostate |= (0x01 << pin);
    } else {
        tmp_iostate &= (uint8_t)~(0x01 << pin);
    }

    writeRegister(SC16IS7XX_REG_IOINTENA << 3, tmp_iostate);
}

/**
 * @brief returns the interrupt status of a pin
 * @param pin the pin number on the port expander (0 - 7)
 * @return the interrupt status, either LOW (0) or HIGH (1)
 */
uint8_t SC16IS7XX::getPinInterrupt(uint8_t pin) {
    uint8_t tmp_iostate;

    tmp_iostate = readRegister(SC16IS7XX_REG_IOINTENA << 3);

    if ((tmp_iostate & (0x01 << pin)) == 0) { return 0; }
    return 1;
}

/**
 * @brief This will need some sort of manual tracking. Perhaps keep a record of
 * interrupt-enabled pins and track their changes.
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
    uint8_t irq_src;

    irq_src = readRegister(SC16IS7XX_REG_IIR << 3);
    // irq_src = (irq_src >> 1);
    // irq_src &= 0x3F;

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
    writeRegister(SC16IS7XX_REG_IOSTATE << 3, state);
}

/**
 * @brief Read all GPIO states from the IOSTATE register.
 * @return uint8_t Current GPIO state bitmask.
 */
uint8_t SC16IS7XX::getPortState() {
    return readRegister(SC16IS7XX_REG_IOSTATE << 3);
}

/**
 * @brief Set all GPIO direction bits at once.
 * @param mode Bitmask written to the IODIR register.
 */
void SC16IS7XX::setPortMode(uint8_t mode) {
    writeRegister(SC16IS7XX_REG_IODIR << 3, mode);
}

/**
 * @brief Read all GPIO direction bits from the IODIR register.
 * @return uint8_t Current GPIO direction bitmask.
 */
uint8_t SC16IS7XX::getPortMode() {
    return readRegister(SC16IS7XX_REG_IODIR << 3);
}

/**
 * @brief Select which GPIO pin is tied to modem signaling.
 * @param gpio Modem GPIO selection value.
 */
void SC16IS7XX::setModemPin(modem_gpio_t gpio) {
    uint8_t tmp_iocontrol;

    tmp_iocontrol = readRegister(SC16IS7XX_REG_IOCONTROL << 3);
    if (gpio == MODEM_PIN_GPIO_0) {
        tmp_iocontrol |= 0x02;
    } else {
        tmp_iocontrol &= 0xFD;
    }
    writeRegister(SC16IS7XX_REG_IOCONTROL << 3, tmp_iocontrol);
}

/**
 * @brief Enable or disable GPIO state latching.
 * @param enabled True to enable latching, false to disable.
 */
void SC16IS7XX::setGPIOLatch(bool enabled) {
    uint8_t tmp_iocontrol;

    tmp_iocontrol = readRegister(SC16IS7XX_REG_IOCONTROL << 3);
    if (enabled == false) {
        tmp_iocontrol &= 0xFE;
    } else {
        tmp_iocontrol |= 0x01;
    }
    writeRegister(SC16IS7XX_REG_IOCONTROL << 3, tmp_iocontrol);
}
