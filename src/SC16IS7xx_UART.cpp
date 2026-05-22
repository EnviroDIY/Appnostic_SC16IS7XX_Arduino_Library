/**
 * @file SC16IS7xx_UART.cpp
 * @copyright Stroud Water Research Center
 * Part of the EnviroDIY ModularSensors library for Arduino.
 * This library is published under the BSD-3 license.
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 *
 * @brief Implements the SC16IS7xx_UART class.
 */

#include "SC16IS7xx.h"

/**
 * @brief constructor for SC16IS7xx_UART
 * @param owner Pointer to the SC16IS7xx object that owns this UART channel
 * @param channel The UART channel number (0 for channel A, 1 for channel B)
 */
SC16IS7xx_UART::SC16IS7xx_UART(SC16IS7xx* owner, uint8_t channel)
    : _owner(owner),
      _channel(channel),
      _crystalFrequency(SC16IS7XX_DEFAULT_XTAL_FREQ),
      _peek_flag(0),
      _peek_buf(-1) {}

/*** UART CONFIGURATION *****************************************/

/**
 * @brief sets the crystal frequency in hertz.
 * @param frequency the frequency of the crystal in hertz
 */
void SC16IS7xx_UART::setCrystalFrequency(uint32_t frequency) {
    _crystalFrequency = frequency;
}

/**
 * @brief gets the crystal frequency in hertz.
 * @return the frequency of the crystal in hertz
 */
uint32_t SC16IS7xx_UART::getCrystalFrequency() const {
    return _crystalFrequency;
}

/**
 * @brief enables fifo buffer
 * @param enabled true to enable FIFO, false to disable
 */
void SC16IS7xx_UART::enableFIFO(bool enabled) {
    Adafruit_BusIO_Register     FCR(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_FCR << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits enable_bit(&FCR, 1, 0);
    enable_bit.write(enabled);
}

/**
 * @brief resets tx or rx fifo buffer
 * @param rx true to reset RX FIFO, false to reset TX FIFO
 */
void SC16IS7xx_UART::resetFIFO(bool rx) {
    Adafruit_BusIO_Register FCR(_owner->i2c_dev, _owner->spi_dev,
                                SC16IS7XX_SPIREG,
                                (SC16IS7XX_REG_FCR << 3 | _channel << 1));
    // reset bit for rx fifo is bit 1, for tx fifo is bit 2
    Adafruit_BusIO_RegisterBits reset_bit(&FCR, 1, rx ? 1 : 2);
    reset_bit.write(true);
}

/**
 * @brief Set FIFO trigger threshold for RX or TX.
 * @param rx True to set RX trigger threshold, false for TX.
 * @param length Trigger threshold value.
 */
void SC16IS7xx_UART::setFIFOTriggerLevel(bool rx, uint8_t length) {
    // the TX FIFO, the TCR (Transmission Control Register), and the TLR
    // (Trigger Level Register) are considered to be enhanced functions, so we
    // need to set EFR[4] to '1' to be able to write to and use trigger level
    // interrupts.
    Adafruit_BusIO_Register     EFR(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_EFR << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits enable_bit_e(&EFR, 1, 4);
    enable_bit_e.write(true);

    // Enable TCR and TLR registers by setting modem control register bit 2
    // (MCR[2]) to '1'
    Adafruit_BusIO_Register     MCR(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_MCR << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits enable_bit(&MCR, 1, 2);
    enable_bit.write(true);

    // Set the FIFO trigger level. Trigger levels are specified in characters
    // from 4 to 60 in steps of 4, while the TLR stores N/4.
    Adafruit_BusIO_Register     TLR(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_TLR << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits length_bits(&TLR, 4, rx ? 4 : 0);
    if (length < 4) {
#if defined(SC16IS7XX_DEBUG_SERIAL)
        SC16IS7XX_DEBUG_SERIAL.println(
            "FIFO trigger length out of range; clamping to 4");
#endif  // SC16IS7XX_DEBUG_SERIAL
        length = 4;
    } else if (length > 60) {
#if defined(SC16IS7XX_DEBUG_SERIAL)
        SC16IS7XX_DEBUG_SERIAL.println(
            "FIFO trigger length out of range; clamping to 60");
#endif  // SC16IS7XX_DEBUG_SERIAL
        length = 60;
    }
    length_bits.write(length / 4);

    // reset the enable bit for enhanced functions back to '0' to prevent
    // unintended consequences of leaving it enabled
    enable_bit_e.write(false);

#if 0
    Adafruit_BusIO_Register FCR(_owner->i2c_dev, _owner->spi_dev, SC16IS7XX_SPIREG,
                                (SC16IS7XX_REG_FCR << 3 | _channel << 1));
    // length bits for rx fifo are in bits 7:6, for tx fifo are in bits 5:4
    // in both cases, the higher bit is the MSB (ie, MSB_FIRST)
    Adafruit_BusIO_RegisterBits length_bits(&FCR, 2, rx ? 6 : 4);
    length_bits.write(length);
#endif
}

/**
 * @brief sets the baud rate
 * @param baudRate the target baud rate to set
 *
 * The output frequency of the baud rate generator is determined by the
 * following formula: Baudrate = (Crystal Frequency / Prescaler) / (16 *
 * Divisor)
 */
void SC16IS7xx_UART::setBaudrate(uint32_t baudRate) {
    uint32_t divisor;
    uint8_t  prescaler;

    // check if the device has sleep mode enabled.
    // the divisor cannot be changed while sleep mode is enabled, so we need to
    // check the current state before attempting to change the baud rate and if
    // sleep mode is enabled, we need to temporarily disable it to change the
    // baud rate and then re-enable it if it was previously enabled.
    bool sleep_enabled = _owner->isSleepEnabled();
    if (sleep_enabled) { _owner->enableSleepMode(false); }

    // The maximum divisor is 0xFFFF, so if the calculated divisor is greater
    // than that, we need to use a prescaler to divide the input clock to the
    // baud rate generator and then calculate the divisor based on the divided
    // clock. The prescaler can be set to divide the clock by 1 or 4 using the
    // MCR[7] bit.

    // calculate the divisor assuming no prescaler (prescaler = 1)
    prescaler = 1;
    divisor   = (getCrystalFrequency() / prescaler) / (baudRate * 16);

    if (divisor > 0xFFFF) {
        // if the divisor is too large, set the prescaler to divide the clock by
        // 4 and recalculate the divisor
        prescaler = 4;
        divisor   = (getCrystalFrequency() / prescaler) / (baudRate * 16);
    }

    // the prescaler is considered to be an enhanced function, so we need to set
    // EFR[4] to '1' to be able to write to and use trigger level interrupts.
    Adafruit_BusIO_Register     EFR(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_EFR << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits enable_bit_e(&EFR, 1, 4);
    enable_bit_e.write(true);

    // set the initial clock divisor (prescaler) value from the MCR[7] bit
    Adafruit_BusIO_Register     MCR(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_MCR << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits prescaler_bit(&MCR, 1, 7);
    prescaler_bit.write(prescaler == 4 ? 1 : 0);

    // reset the enable bit for enhanced functions back to '0' to prevent
    // unintended consequences of leaving it enabled
    enable_bit_e.write(false);

    // enable the divisor latch to allow us to write to the divisor registers
    // The Special Register set (the two divisor registers) is accessible only
    // when LCR[7] = logic 1 and LCR is not 0xBF (0b10111111 - No break, forced
    // parity, 2 stop bits, 8 data bits)
    Adafruit_BusIO_Register     LCR(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_LCR << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits latch_enable(&LCR, 1, 7);
    latch_enable.write(true);

    // write to DLL
    Adafruit_BusIO_Register DLL(_owner->i2c_dev, _owner->spi_dev,
                                SC16IS7XX_SPIREG,
                                (SC16IS7XX_REG_DLL << 3 | _channel << 1));
    DLL.write((uint8_t)divisor);
    // write to DLH
    Adafruit_BusIO_Register DLH(_owner->i2c_dev, _owner->spi_dev,
                                SC16IS7XX_SPIREG,
                                (SC16IS7XX_REG_DLH << 3 | _channel << 1));
    DLH.write((uint8_t)(divisor >> 8));

    // disable the divisor latch to lock the divisor and allow us access to the
    // general registers again
    latch_enable.write(false);

    // re-enable sleep mode if it was previously enabled
    if (sleep_enabled) { _owner->enableSleepMode(true); }


#if defined(SC16IS7XX_DEBUG_SERIAL)
    float actual_baudrate = (getCrystalFrequency() / prescaler) /
        (16 * divisor);
    float error = (actual_baudrate - baudRate) * 100 / baudRate;
    SC16IS7XX_DEBUG_SERIAL.print("Desired baudrate: ");
    SC16IS7XX_DEBUG_SERIAL.println(baudRate, DEC);
    SC16IS7XX_DEBUG_SERIAL.print("Prescaler: ");
    SC16IS7XX_DEBUG_SERIAL.println(prescaler, DEC);
    SC16IS7XX_DEBUG_SERIAL.print("Calculated divisor: ");
    SC16IS7XX_DEBUG_SERIAL.println(divisor, DEC);
    SC16IS7XX_DEBUG_SERIAL.print("Actual baudrate: ");
    SC16IS7XX_DEBUG_SERIAL.println(actual_baudrate, DEC);
    SC16IS7XX_DEBUG_SERIAL.print("Baudrate error: ");
    SC16IS7XX_DEBUG_SERIAL.println(error, DEC);
#endif  // SC16IS7XX_DEBUG_SERIAL
}

/**
 * @brief sets the line parameters
 * @param dataBits the number of data bits (5, 6, 7, or 8)
 * @param parity the parity mode (0: none, 1: odd, 2: even, 3: force '1', 4:
 * force '0')
 * @param stopBits the number of stop bits (1 or 2)
 */
void SC16IS7xx_UART::setLine(uint8_t dataBits, uint8_t parity,
                             uint8_t stopBits) {
    Adafruit_BusIO_Register LCR(_owner->i2c_dev, _owner->spi_dev,
                                SC16IS7XX_SPIREG,
                                (SC16IS7XX_REG_LCR << 3 | _channel << 1));

    uint8_t tmp_lcr = LCR.read();
    tmp_lcr &= 0xC0;  // Clear the lower six bit of LCR (LCR[0] to LCR[5]

#if defined(SC16IS7XX_DEBUG_SERIAL)
    SC16IS7XX_DEBUG_SERIAL.print("LCR Register:0x");
    SC16IS7XX_DEBUG_SERIAL.println(tmp_lcr, HEX);
#endif  // SC16IS7XX_DEBUG_SERIAL

    // data bit length
    // LCR[0:1]
    // | LCR[1] | LCR[0] | Data Bits        |
    // |--------|--------|------------------|
    // | 0      | 0      | 5 bits           |
    // | 0      | 1      | 6 bits           |
    // | 1      | 0      | 7 bits           |
    // | 1      | 1      | 8 bits           |
    switch (dataBits) {
        case 5: break;
        case 6: tmp_lcr |= 0x01; break;
        case 7: tmp_lcr |= 0x02; break;
        case 8: tmp_lcr |= 0x03; break;
        default: tmp_lcr |= 0x03; break;
    }

    // stop bits
    // LCR[2] - 0: 1 stop bit, 1: 1.5 stop bits (when data bits = 5) or 2 stop
    // bits
    if (stopBits == 2) { tmp_lcr |= 0x04; }

    // parity
    // LCR[5:3]
    // | LCR[5] | LCR[4] | LCR[3] | Parity Mode       |
    // |--------|--------|--------|-------------------|
    // | x      | x      | 0      | No parity        |
    // | 0      | 0      | 1      | odd parity       |
    // | 0      | 1      | 1      | even parity      |
    // | 1      | 0      | 1      | force '1' parity |
    // | 1      | 1      | 1      | force '0' parity |
    switch (parity) {
        case 0:  // no parity
            break;
        case 1:  // odd parity
            tmp_lcr |= 0x08;
            break;
        case 2:  // even parity
            tmp_lcr |= 0x18;
            break;
        case 3:  // force '1' parity
            tmp_lcr |= 0x28;
            break;
        case 4:  // force '0' parity
            tmp_lcr |= 0x38;
            break;
        default: break;
    }

    LCR.write(tmp_lcr);
}

/**
 * @brief Set line parameters using a pre-defined config enum value.
 *
 * @param config The typed configuration enum encoding the data bits, parity,
 * and stop bits.
 */
void SC16IS7xx_UART::setLine(SC16IS7xxSerialConfig config) {
    setLine(static_cast<uint8_t>(config));
}


/**
 * @brief Set line parameters using a pre-defined config value, aligned with
 * Arduino HardwareSerial config values (eg, SERIAL_8N1).
 *
 * @param config the configuration value encoding the data bits, parity, and
 * stop bits
 */
void SC16IS7xx_UART::setLine(uint8_t config) {
    uint8_t dataBits = 8;
    uint8_t parity   = 0;
    uint8_t stopBits = 1;

    // extract data bits from config (bits 0:2)
    switch (config & 0x07) {
        case 0x00: dataBits = 5; break;
        case 0x02: dataBits = 6; break;
        case 0x04: dataBits = 7; break;
        case 0x06: dataBits = 8; break;
        default: dataBits = 8; break;
    }

    // extract stop bits from config (bit 3)
    if (config & 0x08) { stopBits = 2; }

    // extract parity from config (bits 4:5)
    switch (config & 0x30) {
        case 0x00: parity = 0; break;  // no parity
        case 0x20: parity = 2; break;  // even parity
        case 0x30: parity = 1; break;  // odd parity
        default: parity = 0; break;    // default to no parity
    }

    setLine(dataBits, parity, stopBits);
}

/**
 * @brief Enable or disable hardware flow control by setting the pin control
 *
 * @param enabled true to enable hardware flow control, false to disable
 */
void SC16IS7xx_UART::enableFlowControl(bool enabled) {
    // set the pin controls to modem pins to enable hardware flow control and to
    // GPIO pins to disable hardware flow control.
    // For channel 0/A GPIO[7:4] emulate RIA, CDA, DTRA, DSRA
    // For channel 1/B GPIO[3:0] emulate RIB, CDB, DTRB, DSRB
    Adafruit_BusIO_Register IOControl(_owner->i2c_dev, _owner->spi_dev,
                                      SC16IS7XX_SPIREG,
                                      SC16IS7XX_REG_IOCONTROL << 3);
    //   pins 0-3 are controlled by bit 2, pins 4-7 are controlled by bit 1
    Adafruit_BusIO_RegisterBits modem_pin_bit(&IOControl, 1,
                                              _channel > 0 ? 1 : 2);
    modem_pin_bit.write(enabled ? 1 : 0);
}

/**
 * @brief sets the interrupt enable register to enable modem/pin change
 * interrupts
 * @param enabled true enables interrupts, false disables them
 *
 * The modem interrupt (IIR = 0bxx000000 = 0x00) is triggered when there is a
 * change in the state of the modem input pins (CD, DSR, DTR, RI). The modem
 * interrupt is cleared by reading the MSR register or the IIR register.
 *
 * The modem interrupt only works if the pin is configured to be a modem pin and
 * not an I/O pin in the IOControl register. Pins 0-3 apply to channel 1/B and
 * are controlled by bit 2 of the IOControl register, while pins 4-7 apply to
 * channel 0/A and are controlled by bit 1.
 */
void SC16IS7xx_UART::enableModemInterrupt(bool enabled) {
    // Modem interrupts only work when flow control is enabled, so enable flow
    // control to enable modem interrupts.
    if (enabled) { enableFlowControl(true); }
    Adafruit_BusIO_Register     IER(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_IER << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits modem(&IER, 1, SC16IS7XX_IER_MODEM);
    modem.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable CTS interrupts
 * @param enabled true enables interrupts, false disables them
 *
 * The CTS interrupt (IIR = 0bxx100000 = 0x20) is triggered when the state of
 * the CTS pin changes. The CTS interrupt is cleared by reading the IIR register
 * or by a change in the state of the CTS pin.
 */
void SC16IS7xx_UART::enableCTSInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_IER << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits cts(&IER, 1, SC16IS7XX_IER_CTS);
    cts.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable RTS interrupts
 * @param enabled true enables interrupts, false disables them
 *
 * The RTS interrupt (IIR = 0bxx100000 = 0x20) is triggered when the state of
 * the RTS pin changes. The RTS interrupt is cleared by reading the IIR register
 * or by a change in the state of the RTS pin.
 */
void SC16IS7xx_UART::enableRTSInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_IER << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits rts(&IER, 1, SC16IS7XX_IER_RTS);
    rts.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable XOFF interrupts
 * @param enabled true enables interrupts, false disables them
 *
 * The XOFF interrupt (IIR = 0bxx10000 = 0x10) is triggered when the XOFF
 * character is received on the serial input. The XOFF interrupt is cleared by
 * reading the IIR register or when an XON character is received.
 */
void SC16IS7xx_UART::enableXOFFInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_IER << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits xoff(&IER, 1, SC16IS7XX_IER_XOFF);
    xoff.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable receive line status
 * interrupts
 * @param enabled true enables interrupts, false disables them
 *
 * Triggered when Overrun Error (OE), Framing Error (FE), Parity Error (PE), or
 * Break Interrupt (BI) errors occur in characters in the RX FIFO.  Cleared when
 * all characters with errors have be read from the Rx FIFO.
 */
void SC16IS7xx_UART::enableRLSInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_IER << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits rls(&IER, 1, SC16IS7XX_IER_RLS);
    rls.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable transmit holding register
 * interrupts
 * @param enabled true enables interrupts, false disables them
 *
 * If the transmit FIFO is disabled, the THR interrupt (IIR[1]) is triggered
 * when the transmit FIFO is empty.
 *
 * If the transmit FIFO is enabled, the THR interrupt is triggered when the
 * number of bytes in the TX FIFO is greater than  to the value in the trigger
 * level register.
 *
 * The THR interrupt is cleared when enough data has been sent on the Tx line to
 * bring the number of bytes in the TX FIFO below the trigger level.
 */
void SC16IS7xx_UART::enableTHRInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_IER << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits thr(&IER, 1, SC16IS7XX_IER_THR);
    thr.write(enabled);
}

/**
 * @brief sets the interrupt enable register to enable receive holding register
 * interrupts
 * @param enabled true enables interrupts, false disables them
 *
 * If the receive FIFO is disabled, the RHR interrupt (IIR[2]) is triggered when
 * a byte of data is received and placed in the RHR register.
 *
 * If the receive FIFO is enabled, the RHR interrupt (IIR[4]) is triggered when
 * the number of bytes in the RX FIFO reaches the trigger level.
 *
 * The RHR interrupt is cleared by reading the RX FIFO (that is, the RHR
 * register).
 */
void SC16IS7xx_UART::enableRHRInterrupt(bool enabled) {
    Adafruit_BusIO_Register     IER(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_IER << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits rhr(&IER, 1, SC16IS7XX_IER_RHR);
    rhr.write(enabled);
}

// RI - ring indicator
/**
 * @brief Enable the RI (Ring Indicator) interrupt and attach a callback
 * function to be called when the RI interrupt is triggered.
 *
 * RI (Ring Indicator) signals that an incoming call or alert condition is
 * present on the connected modem or communications line.
 *
 * @param callback The function to execute when the RI interrupt is triggered.
 * This function should take no parameters and return void.
 */
void SC16IS7xx_UART::attachRIInterrupt(voidFxnPtr callback) {
    // enable the modem interrupt
    enableModemInterrupt(true);
    // store the callback
    _owner->storeCallback(SC16IS7XX_INT_MASK_RI, callback);
}
/**
 * @brief Disable the RI (Ring Indicator) interrupt callback.
 *
 * Clears the stored callback for the RI interrupt. The shared modem interrupt
 * remains enabled for any other modem interrupt sources still in use.
 */
void SC16IS7xx_UART::detachRIInterrupt() {
    // NOTE: We don't disable the modem interrupt here because the modem
    // interrupt may be shared by multiple sources and we don't want to disable
    // it if other sources are still in use.
    // clear the callback for the interrupt
    _owner->clearCallback(SC16IS7XX_INT_MASK_RI);
}

// CD - carrier detect (aka Data Carrier Detect (DCD))
/**
 * @brief Enable the CD (Carrier Detect) interrupt and attach a callback
 * function to be called when the CD interrupt is triggered.
 *
 * CD (Carrier Detect), also called DCD, indicates that a valid carrier signal
 * from the remote device has been detected.
 *
 * @param callback The function to execute when the CD interrupt is triggered.
 * This function should take no parameters and return void.
 */
void SC16IS7xx_UART::attachCDInterrupt(voidFxnPtr callback) {
    // enable the modem interrupt
    enableModemInterrupt(true);
    // store the callback
    _owner->storeCallback(SC16IS7XX_INT_MASK_CD, callback);
}
/**
 * @brief Disable the CD (Carrier Detect) interrupt callback.
 *
 * Clears the stored callback for the CD interrupt. The shared modem interrupt
 * remains enabled for any other modem interrupt sources still in use.
 */
void SC16IS7xx_UART::detachCDInterrupt() {
    // NOTE: We don't disable the modem interrupt here because the modem
    // interrupt may be shared by multiple sources and we don't want to disable
    // it if other sources are still in use.
    // clear the callback for the interrupt
    _owner->clearCallback(SC16IS7XX_INT_MASK_CD);
}

// DSR - data set ready
/**
 * @brief Enable the DSR (Data Set Ready) interrupt and attach a callback
 * function to be called when the DSR interrupt is triggered.
 *
 * DSR (Data Set Ready) – Sent by the Data Communication Equipment (DCE) (e.g.,
 * modem) to the Data Terminal Equipment (DTE) (e.g., computer) to indicate it
 * is operational and ready to receive data
 *
 * @param callback The function to execute when the DSR interrupt is triggered.
 * This function should take no parameters and return void.
 */
void SC16IS7xx_UART::attachDSRInterrupt(voidFxnPtr callback) {
    // enable the modem interrupt
    enableModemInterrupt(true);
    // store the callback
    _owner->storeCallback(SC16IS7XX_INT_MASK_DSR, callback);
}
/**
 * @brief Disable the DSR (Data Set Ready) interrupt callback.
 *
 * Clears the stored callback for the DSR interrupt. The shared modem interrupt
 * remains enabled for any other modem interrupt sources still in use.
 */
void SC16IS7xx_UART::detachDSRInterrupt() {
    // NOTE: We don't disable the modem interrupt here because the modem
    // interrupt may be shared by multiple sources and we don't want to disable
    // it if other sources are still in use.
    // clear the callback for the interrupt
    _owner->clearCallback(SC16IS7XX_INT_MASK_DSR);
}

// DTR - data terminal ready
/**
 * @brief Enable the DTR (Data Terminal Ready) interrupt and attach a callback
 * function to be called when the DTR interrupt is triggered.
 *
 * DTR (Data Terminal Ready) is asserted by the Data Terminal Equipment (DTE)
 * to indicate that it is powered, ready, and able to communicate.
 *
 * @param callback The function to execute when the DTR interrupt is triggered.
 * This function should take no parameters and return void.
 */
void SC16IS7xx_UART::attachDTRInterrupt(voidFxnPtr callback) {
    // enable the modem interrupt
    enableModemInterrupt(true);
    // store the callback
    _owner->storeCallback(SC16IS7XX_INT_MASK_DTR, callback);
}
/**
 * @brief Disable the DTR (Data Terminal Ready) interrupt callback.
 *
 * Clears the stored callback for the DTR interrupt. The shared modem interrupt
 * remains enabled for any other modem interrupt sources still in use.
 */
void SC16IS7xx_UART::detachDTRInterrupt() {
    // NOTE: We don't disable the modem interrupt here because the modem
    // interrupt may be shared by multiple sources and we don't want to disable
    // it if other sources are still in use.
    // clear the callback for the interrupt
    _owner->clearCallback(SC16IS7XX_INT_MASK_DTR);
}

/**
 * @brief Enable the CTS (Clear To Send) interrupt and attach a callback
 * function to be called when the CTS interrupt is triggered.
 *
 * CTS (Clear To Send) is a hardware flow-control signal indicating that the
 * connected device is ready to receive transmitted data.
 *
 * @param callback The function to execute when the CTS interrupt is triggered.
 * This function should take no parameters and return void.
 */
void SC16IS7xx_UART::attachCTSInterrupt(voidFxnPtr callback) {
    // enable the CTS interrupt
    enableCTSInterrupt(true);
    // store the callback
    _owner->storeCallback(SC16IS7XX_INT_MASK_CTS, callback);
}
/**
 * @brief Disable the CTS (Clear To Send) interrupt callback.
 *
 * Disables the CTS interrupt source and clears the stored callback.
 */
void SC16IS7xx_UART::detachCTSInterrupt() {
    // disable the CTS interrupt
    enableCTSInterrupt(false);
    // clear the callback for the interrupt
    _owner->clearCallback(SC16IS7XX_INT_MASK_CTS);
}

/**
 * @brief Enable the RTS (Request To Send) interrupt and attach a callback
 * function to be called when the RTS interrupt is triggered.
 *
 * RTS (Request To Send) is a hardware flow-control signal used to indicate
 * transmit readiness between connected devices.
 *
 * @param callback The function to execute when the RTS interrupt is triggered.
 * This function should take no parameters and return void.
 */
void SC16IS7xx_UART::attachRTSInterrupt(voidFxnPtr callback) {
    // enable the RTS interrupt
    enableRTSInterrupt(true);
    // store the callback
    _owner->storeCallback(SC16IS7XX_INT_MASK_RTS, callback);
}
/**
 * @brief Disable the RTS (Request To Send) interrupt callback.
 *
 * Disables the RTS interrupt source and clears the stored callback.
 */
void SC16IS7xx_UART::detachRTSInterrupt() {
    // disable the RTS interrupt
    enableRTSInterrupt(false);
    // clear the callback for the interrupt
    _owner->clearCallback(SC16IS7XX_INT_MASK_RTS);
}

/**
 * @brief Enable the XOFF interrupt and attach a callback function to be called
 * when the XOFF interrupt is triggered.
 *
 * The XOFF interrupt is used with software flow control to signal that the
 * remote device has requested transmission to pause.
 *
 * @param callback The function to execute when the XOFF interrupt is
 * triggered. This function should take no parameters and return void.
 */
void SC16IS7xx_UART::attachXOFFInterrupt(voidFxnPtr callback) {
    // enable the XOFF interrupt
    enableXOFFInterrupt(true);
    // store the callback
    _owner->storeCallback(SC16IS7XX_INT_MASK_XOFF, callback);
}
/**
 * @brief Disable the XOFF interrupt callback.
 *
 * Disables the XOFF interrupt source and clears the stored callback.
 */
void SC16IS7xx_UART::detachXOFFInterrupt() {
    // disable the XOFF interrupt
    enableXOFFInterrupt(false);
    // clear the callback for the interrupt
    _owner->clearCallback(SC16IS7XX_INT_MASK_XOFF);
}

/**
 * @brief Enable the RLS (Receiver Line Status) interrupt and attach a callback
 * function to be called when the RLS interrupt is triggered.
 *
 * The RLS interrupt reports UART line-status conditions such as overrun,
 * parity, framing, and break errors.
 *
 * @param callback The function to execute when the RLS interrupt is triggered.
 * This function should take no parameters and return void.
 */
void SC16IS7xx_UART::attachRLSInterrupt(voidFxnPtr callback) {
    // enable the RLS interrupt
    enableRLSInterrupt(true);
    // store the callback
    _owner->storeCallback(SC16IS7XX_INT_MASK_RLS, callback);
}
/**
 * @brief Disable the RLS (Receiver Line Status) interrupt callback.
 *
 * Disables the RLS interrupt source and clears the stored callback.
 */
void SC16IS7xx_UART::detachRLSInterrupt() {
    // disable the RLS interrupt
    enableRLSInterrupt(false);
    // clear the callback for the interrupt
    _owner->clearCallback(SC16IS7XX_INT_MASK_RLS);
}

/**
 * @brief Enable the THR (Transmit Holding Register) interrupt and attach a
 * callback function to be called when the THR interrupt is triggered.
 *
 * The THR interrupt indicates that the transmit holding register is ready to
 * accept additional data for transmission.
 *
 * @param callback The function to execute when the THR interrupt is triggered.
 * This function should take no parameters and return void.
 */
void SC16IS7xx_UART::attachTHRInterrupt(voidFxnPtr callback) {
    // enable the THR interrupt
    enableTHRInterrupt(true);
    // store the callback
    _owner->storeCallback(SC16IS7XX_INT_MASK_THR, callback);
}
/**
 * @brief Disable the THR (Transmit Holding Register) interrupt callback.
 *
 * Disables the THR interrupt source and clears the stored callback.
 */
void SC16IS7xx_UART::detachTHRInterrupt() {
    // disable the THR interrupt
    enableTHRInterrupt(false);
    // clear the callback for the interrupt
    _owner->clearCallback(SC16IS7XX_INT_MASK_THR);
}

/**
 * @brief Enable the RHR (Receive Holding Register) interrupt and attach a
 * callback function to be called when the RHR interrupt is triggered.
 *
 * The RHR interrupt indicates that received data is available in the receive
 * holding register and ready to be read.
 *
 * @param callback The function to execute when the RHR interrupt is triggered.
 * This function should take no parameters and return void.
 */
void SC16IS7xx_UART::attachRHRInterrupt(voidFxnPtr callback) {
    // enable the RHR interrupt
    enableRHRInterrupt(true);
    // store the callback
    _owner->storeCallback(SC16IS7XX_INT_MASK_RHR, callback);
}
/**
 * @brief Disable the RHR (Receive Holding Register) interrupt callback.
 *
 * Disables the RHR interrupt source and clears the stored callback.
 */
void SC16IS7xx_UART::detachRHRInterrupt() {
    // disable the RHR interrupt
    enableRHRInterrupt(false);
    // clear the callback for the interrupt
    _owner->clearCallback(SC16IS7XX_INT_MASK_RHR);
}

/**
 * @brief Attach a callback function for the timeout interrupt.
 *
 * The timeout interrupt occurs when received data remains unread in the FIFO
 * for longer than the programmed timeout period. This interrupt does not have
 * a separate enable function in this library.
 *
 * @param callback The function to execute when the timeout interrupt is
 * triggered. This function should take no parameters and return void.
 */
void SC16IS7xx_UART::attachTimeoutInterrupt(voidFxnPtr callback) {
    // this interrupt doesn't have an enable function
    // store the callback
    _owner->storeCallback(SC16IS7XX_INT_MASK_TIMEOUT, callback);
}
/**
 * @brief Detach the timeout interrupt callback.
 *
 * Clears the stored callback for the timeout interrupt. This interrupt does
 * not have a separate enable function in this library.
 */
void SC16IS7xx_UART::detachTimeoutInterrupt() {
    // this interrupt doesn't have an enable function
    // clear the callback for the interrupt
    _owner->clearCallback(SC16IS7XX_INT_MASK_TIMEOUT);
}

/**
 * @brief Begin UART communication with specified baud rate and configuration,
 * aligned with Arduino HardwareSerial begin() method.
 *
 * @param baud The baud rate to set for UART communication.
 * @param config The configuration value encoding the data bits, parity, and
 * stop bits, aligned with Arduino HardwareSerial config values (eg,
 * SERIAL_8N1).
 */
void SC16IS7xx_UART::begin(unsigned long baud, uint8_t config) {
    if (SC16IS7xx::_activeObject != _owner) {
        SC16IS7xx::_activeObject = _owner;
    }
    enableFIFO(true);
    setBaudrate(baud);
    setLine(config);
}

/**
 * @brief Begin UART communication with specified baud rate and typed
 * configuration enum.
 *
 * @param baud The baud rate to set for UART communication.
 * @param config The typed configuration enum encoding the data bits, parity,
 * and stop bits.
 */
void SC16IS7xx_UART::begin(unsigned long baud, SC16IS7xxSerialConfig config) {
    begin(baud, static_cast<uint8_t>(config));
}

/**
 * @brief Begin UART communication with specified baud rate and configuration
 *
 * @param baud The baud rate to set for UART communication.
 * @param dataBits the number of data bits (5, 6, 7, or 8)
 * @param parity the parity mode (0: none, 1: odd, 2: even, 3: force '1', 4:
 * force '0')
 * @param stopBits the number of stop bits (1 or 2)
 */
void SC16IS7xx_UART::begin(unsigned long baud, uint8_t dataBits, uint8_t parity,
                           uint8_t stopBits) {
    if (SC16IS7xx::_activeObject != _owner) {
        SC16IS7xx::_activeObject = _owner;
    }
    enableFIFO(true);
    setBaudrate(baud);
    setLine(dataBits, parity, stopBits);
}

/**
 * @brief Begin UART communication with specified baud rate and default
 * configuration of 8 data bits, no parity, and 1 stop bit (8N1), aligned with
 * Arduino HardwareSerial begin() method.
 *
 * @param baud The baud rate to set for UART communication.
 */
void SC16IS7xx_UART::begin(unsigned long baud) {
    begin(baud, SC16IS7xxSerialConfig::C8N1);
}

/**
 * @brief Get number of bytes available in RX FIFO.
 * @return uint8_t Number of available bytes in RX FIFO.
 */
uint8_t SC16IS7xx_UART::FIFOAvailableData() {
    Adafruit_BusIO_Register RXLVL(_owner->i2c_dev, _owner->spi_dev,
                                  SC16IS7XX_SPIREG,
                                  (SC16IS7XX_REG_RXLVL << 3 | _channel << 1));
#if defined(SC16IS7XX_DEBUG_SERIAL)
    int available_bytes = RXLVL.read();
    SC16IS7XX_DEBUG_SERIAL.print("=====RX FIFO Available data:");
    SC16IS7XX_DEBUG_SERIAL.println(available_bytes, DEC);
    return available_bytes;
#else
    return RXLVL.read();
#endif  // SC16IS7XX_DEBUG_SERIAL
}

/**
 * @brief Get free space in TX FIFO.
 * @return uint8_t Number of available TX FIFO slots.
 */
uint8_t SC16IS7xx_UART::FIFOAvailableSpace() {
    Adafruit_BusIO_Register TXLVL(_owner->i2c_dev, _owner->spi_dev,
                                  SC16IS7XX_SPIREG,
                                  (SC16IS7XX_REG_TXLVL << 3 | _channel << 1));
#if defined(SC16IS7XX_DEBUG_SERIAL)
    int available_bytes = TXLVL.read();
    SC16IS7XX_DEBUG_SERIAL.print("=====TX FIFO Available space:");
    SC16IS7XX_DEBUG_SERIAL.println(available_bytes, DEC);
    return available_bytes;
#else
    return TXLVL.read();
#endif  // SC16IS7XX_DEBUG_SERIAL
}

/**
 * @brief Read one byte from UART RX FIFO.
 * @return int Byte value, or -1 when no data is available.
 */
int SC16IS7xx_UART::rawRead() {
    // We need to check if anything is in the buffer before trying to read,
    // otherwise the SC16IS7xx will return whatever stale data is in the RHR
    // register from the last time it was read, even if that data has already
    // been read from the FIFO.
    if (FIFOAvailableData() == 0) { return -1; }
    Adafruit_BusIO_Register RHR(_owner->i2c_dev, _owner->spi_dev,
                                SC16IS7XX_SPIREG,
                                (SC16IS7XX_REG_RHR << 3 | _channel << 1));
    return RHR.read();
}

int SC16IS7xx_UART::rawRead(uint8_t* buf, size_t size) {
    // We need to check if anything is in the buffer before trying to read,
    // otherwise the SC16IS7xx will return whatever stale data is in the RHR
    // register from the last time it was read, even if that data has already
    // been read from the FIFO.
    if (FIFOAvailableData() == 0) { return -1; }
    Adafruit_BusIO_Register RHR(_owner->i2c_dev, _owner->spi_dev,
                                SC16IS7XX_SPIREG,
                                (SC16IS7XX_REG_RHR << 3 | _channel << 1));
    return RHR.read(buf, size);
}

/**
 * @brief Read one byte from UART RX FIFO, with support for peeked byte.
 *
 * @return int Byte value, or -1 when no data is available.
 */
int SC16IS7xx_UART::read() {
    // if there's a peeked byte, return that instead of reading from the FIFO
    if (_peek_flag) {
#if defined(SC16IS7XX_DEBUG_SERIAL)
        SC16IS7XX_DEBUG_SERIAL.print("==Returning peeked byte: ");
        SC16IS7XX_DEBUG_SERIAL.println(static_cast<char>(_peek_buf));
#endif  // SC16IS7XX_DEBUG_SERIAL
        _peek_flag = 0;
        return _peek_buf;
    }
    // otherwise, read from the FIFO as normal
    return rawRead();
}

/**
 * @brief Read multiple bytes from UART RX FIFO into provided buffer, with
 * support for peeked byte.
 *
 * @param buf The buffer to read bytes into.
 * @param size The maximum number of bytes to read into the buffer.
 * @return The number of bytes read, or -1 if no data is available.
 */
int SC16IS7xx_UART::read(uint8_t* buf, size_t size) {
    // if there's a peeked byte, place that in the first position of the buffer
    // and read the rest from the FIFO
    if (_peek_flag) {
#if defined(SC16IS7XX_DEBUG_SERIAL)
        SC16IS7XX_DEBUG_SERIAL.print("==Appending peeked byte: ");
        SC16IS7XX_DEBUG_SERIAL.println(static_cast<char>(_peek_buf));
#endif  // SC16IS7XX_DEBUG_SERIAL
        _peek_flag = 0;
        *buf       = _peek_buf;
        return rawRead(buf + 1, size - 1) + 1;
    }
    // otherwise, read from the FIFO as normal
    return rawRead(buf, size);
}

/**
 * @brief Return number of bytes waiting in RX FIFO.
 * @return int Number of readable bytes.
 */
int SC16IS7xx_UART::available() {
#if defined(SC16IS7XX_DEBUG_SERIAL)
    if (_peek_flag) {
        SC16IS7XX_DEBUG_SERIAL.println("==Available: 1 byte in peek buffer");
    }
#endif  // SC16IS7XX_DEBUG_SERIAL
    return FIFOAvailableData() + (_peek_flag ? 1 : 0);
}

/**
 * @brief Peek one byte from RX FIFO without consuming it.
 * @return int Next byte value, or -1 when no data is available.
 */
int SC16IS7xx_UART::peek() {
    if (_peek_flag == 0) {
        _peek_buf = read();
        if (_peek_buf >= 0) { _peek_flag = 1; }
    }
    return _peek_buf;
}

/**
 * @brief Write a buffer to UART.
 * @param buf Pointer to bytes to transmit.
 * @param size Number of bytes to send.
 * @return size_t Number of bytes written.
 */
size_t SC16IS7xx_UART::write(const uint8_t* buf, size_t size) {
    if (size == 0) return 0;

    // Pointer to where in the buffer we're up to
    // A const cast is need to cast-away the constant-ness of the buffer (ie,
    // modify it).
    uint8_t* txPtr      = const_cast<uint8_t*>(buf);
    size_t   bytesSent  = 0;
    uint32_t stallStart = 0;

    do {
        // check how much space is available in the TX FIFO
        size_t space = FIFOAvailableSpace();
        if (space == 0) {
            if (stallStart == 0) { stallStart = millis(); }
            // Prevent a busy-spin when FIFO space is unavailable.
            yield();
            // wait, but no longer than the timeout, for space to become
            // available in the FIFO
            if ((millis() - stallStart) >= _timeout) { return bytesSent; }
            continue;
        }
        stallStart = 0;
        // Number of bytes to send from buffer in this command, equal to the
        // number of spaces available in the TX FIFO or the number of bytes
        // remaining in the buffer, whichever is smaller. We use a chunked
        // approach to sending the buffer to ensure that we don't attempt to
        // write more bytes than are available in the TX FIFO and to ensure that
        // we don't read past the end of the buffer.
        size_t sendLength = space;
        // Ensure the program doesn't read past the allocated memory
        if (txPtr + space > const_cast<uint8_t*>(buf) + size) {
            sendLength = const_cast<uint8_t*>(buf) + size - txPtr;
        }
        // write out the number of bytes for this chunk
        Adafruit_BusIO_Register THR(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_THR << 3 | _channel << 1));
        THR.write(txPtr, sendLength);
        bytesSent += sendLength;  // bump up number of bytes sent
        txPtr += sendLength;      // bump up the pointer
    } while (bytesSent < size);
    return bytesSent;
}

/**
 * @brief Write one byte to UART TX register.
 * @param c Byte to transmit.
 * @return size_t Number of bytes written.
 */
size_t SC16IS7xx_UART::write(uint8_t c) {
    return write(&c, 1);
}
/**
 * @brief Write a null-terminated string to UART.
 *
 * @param str Pointer to the null-terminated string to transmit.
 * @return size_t Number of bytes written.
 */
size_t SC16IS7xx_UART::write(const char* str) {
    if (str == nullptr) return 0;
    return write((const uint8_t*)str, strlen(str));
}

/**
 * @brief Block until TX shift register is empty.
 */
void SC16IS7xx_UART::flush() {
    Adafruit_BusIO_Register     LSR(_owner->i2c_dev, _owner->spi_dev,
                                    SC16IS7XX_SPIREG,
                                    (SC16IS7XX_REG_LSR << 3 | _channel << 1));
    Adafruit_BusIO_RegisterBits thr_empty_bit(&LSR, 1, 7);
    while (!thr_empty_bit.read()) {
        // do nothing, just wait
    }
}


// cSpell:words SPIFREQ SPIREG MISO MOSI DTRA DSRA DTRB DSRB
