/**
 * This is an Arduino library for the NOS8007 based on the
 * SC16IS752 dual UART from NXP.
 *
 * It is possible that this library may work with other vendor
 * devices using I2C or SPI but its primary purpose is for
 * Appnostic devices. Pull requests to improve compatibility are
 * welcomed but issues regarding other vendor devices may not receive
 * priority.
 *
 * Credits:
 * @SandboxElectronics for most of the code
 * @TD-er for the SC16IS752 patches
 *
 * Made with love by the Appnostic team!
 */
#include "Appnostic_SC16IS752.h"

/**
 * @brief constructor for SC16IS752
 * @param channel
 */
Appnostic_SC16IS752::Appnostic_SC16IS752(uint8_t channel) {
    Appnostic_SC16IS752::channel   = channel;
    Appnostic_SC16IS752::peek_flag = 0;
}

/**
 * @brief   slight modification of the register write function to
 *          allow for the separate channels of the SC16IS752
 * @note    uses Appnostic_SC16IS7XX::writeRegister
 * @param channel 0 or 1
 * @param reg_addr
 * @param val
 */
void Appnostic_SC16IS752::writeRegister(uint8_t channel, uint8_t reg_addr,
                                        uint8_t val) {
    Appnostic_SC16IS7XX::writeRegister((reg_addr << 3 | channel << 1), val);
}

/**
 * @brief   slight modification of the register read function to
 *          allow for the separate channels of the SC16IS752
 * @note    uses Appnostic_SC16IS7XX::readRegister
 * @param channel
 * @param reg_addr
 * @return
 */
uint8_t Appnostic_SC16IS752::readRegister(uint8_t channel, uint8_t reg_addr) {
    return Appnostic_SC16IS7XX::readRegister((reg_addr << 3 | channel << 1));
}

/*** DERIVED FUNCTIONS **********************************************/

/**
 * @brief tests the device to check if it is online
 * @return
 */
bool Appnostic_SC16IS752::ping() {
    writeRegister(SC16IS752_CHANNEL_A, SC16IS7XX_REG_SPR, 0x55);

    if (readRegister(SC16IS752_CHANNEL_A, SC16IS7XX_REG_SPR) == 0x55) {
        return true;
    }

    writeRegister(SC16IS752_CHANNEL_A, SC16IS7XX_REG_SPR, 0xAA);

    if (readRegister(SC16IS752_CHANNEL_A, SC16IS7XX_REG_SPR) == 0xAA) {
        return true;
    }

    writeRegister(SC16IS752_CHANNEL_B, SC16IS7XX_REG_SPR, 0x55);

    if (readRegister(SC16IS752_CHANNEL_B, SC16IS7XX_REG_SPR) == 0x55) {
        return true;
    }

    writeRegister(SC16IS752_CHANNEL_B, SC16IS7XX_REG_SPR, 0xAA);

    if (readRegister(SC16IS752_CHANNEL_B, SC16IS7XX_REG_SPR) == 0xAA) {
        return true;
    }

    return false;
}

/*** UART CONFIGURATION *****************************************/

/**
 * @brief enables fifo buffer
 * @param enabled
 */
void Appnostic_SC16IS752::setFIFO(bool enabled) {
    settings.fifo = enabled;

    uint8_t tmp_fcr;

    tmp_fcr = readRegister(channel, SC16IS7XX_REG_FCR);

    if (enabled == false) {
        tmp_fcr &= 0xFE;
    } else {
        tmp_fcr |= 0x01;
    }

    writeRegister(channel, SC16IS7XX_REG_FCR, tmp_fcr);
}

/**
 * @brief resets tx or rx fifo buffer
 * @param rx
 */
void Appnostic_SC16IS752::resetFIFO(bool rx) {
    uint8_t tmp_fcr;

    tmp_fcr = readRegister(channel, SC16IS7XX_REG_FCR);

    if (rx == false) {
        tmp_fcr |= 0x04;
    } else {
        tmp_fcr |= 0x02;
    }
    writeRegister(channel, SC16IS7XX_REG_FCR, tmp_fcr);
}

/**
 * @brief Set FIFO trigger threshold for RX or TX.
 * @param rx True to set RX trigger threshold, false for TX.
 * @param length Trigger threshold value.
 */
void Appnostic_SC16IS752::setFIFOTriggerLevel(bool rx, uint8_t length) {
    uint8_t tmp_reg;

    tmp_reg = readRegister(channel, SC16IS7XX_REG_MCR);
    tmp_reg |= 0x04;
    writeRegister(channel, SC16IS7XX_REG_MCR,
                  tmp_reg);  // SET MCR[2] to '1' to use TLR register or trigger
                             // level control in FCR register

    tmp_reg = readRegister(channel, SC16IS7XX_REG_EFR);
    writeRegister(channel, SC16IS7XX_REG_EFR,
                  tmp_reg |
                      0x10);  // set ERF[4] to '1' to use the  enhanced features
    if (rx == false) {
        writeRegister(channel, SC16IS7XX_REG_TLR,
                      length << 4);  // Tx FIFO trigger level setting
    } else {
        writeRegister(channel, SC16IS7XX_REG_TLR,
                      length);  // Rx FIFO Trigger level setting
    }
    writeRegister(channel, SC16IS7XX_REG_EFR, tmp_reg);  // restore EFR register
}

/**
 * @brief sets the baud rate. nos8007 has been tested to 921600
 * @param baudRate
 */
void Appnostic_SC16IS752::setBaudrate(uint32_t baudRate) {
    settings.baud = baudRate;

    uint16_t divisor;
    uint8_t  prescaler;
    uint8_t  tmp_lcr;

    if ((readRegister(channel, SC16IS7XX_REG_MCR) & 0x80) == 0) {
        prescaler = 1;
    } else {
        prescaler = 4;
    }

    divisor = (getCrystalFrequency() / prescaler) / (baudRate * 16);

    tmp_lcr = readRegister(channel, SC16IS7XX_REG_LCR);
    tmp_lcr |= 0x80;
    writeRegister(channel, SC16IS7XX_REG_LCR, tmp_lcr);

    // write to DLL
    writeRegister(channel, SC16IS7XX_REG_DLL, (uint8_t)divisor);

    // write to DLH
    writeRegister(channel, SC16IS7XX_REG_DLH, (uint8_t)(divisor >> 8));
    tmp_lcr &= 0x7F;
    writeRegister(channel, SC16IS7XX_REG_LCR, tmp_lcr);
}

/**
 * @brief sets the line parameters
 * @param bits
 * @param parity
 * @param stopBits
 */
void Appnostic_SC16IS752::setLine(uint8_t bits, uint8_t parity,
                                  uint8_t stopBits) {
    uint8_t tmp_lcr;

    settings.bits     = bits;
    settings.parity   = parity;
    settings.stopBits = stopBits;

    tmp_lcr = readRegister(channel, SC16IS7XX_REG_LCR);
    tmp_lcr &= 0xC0;  // Clear the lower six bit of LCR (LCR[0] to LCR[5]

    // data bit length
    switch (settings.bits) {
        case 5: break;
        case 6: tmp_lcr |= 0x01; break;
        case 7: tmp_lcr |= 0x02; break;
        case 8: tmp_lcr |= 0x03; break;
        default: tmp_lcr |= 0x03; break;
    }

    // stop bits
    if (settings.stopBits == 2) { tmp_lcr |= 0x04; }

    // parity
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
            tmp_lcr |= 0x03;
            break;
        case 4:  // force '0' parity
            break;
        default: break;
    }

    writeRegister(channel, SC16IS7XX_REG_LCR, tmp_lcr);
}

/**
 * @brief Get number of cached/available bytes in RX FIFO.
 * @return uint8_t Number of available bytes.
 */
uint8_t Appnostic_SC16IS752::FIFOAvailableData() {
    if (fifo_available == 0) {
        fifo_available = readRegister(channel, SC16IS7XX_REG_RXLVL);
    }
    return fifo_available;
}

/**
 * @brief Get free space in TX FIFO.
 * @return uint8_t Number of available TX FIFO slots.
 */
uint8_t Appnostic_SC16IS752::FIFOAvailableSpace() {
    return readRegister(channel, SC16IS7XX_REG_TXLVL);
}

/**
 * @brief Read one byte from UART RX FIFO.
 * @return int Byte value, or -1 when no data is available.
 */
int Appnostic_SC16IS752::read() {
    volatile uint8_t val;

    if (FIFOAvailableData() == 0) {
        return -1;
    } else {
        if (fifo_available > 0) { --fifo_available; }
        val = readRegister(channel, SC16IS7XX_REG_RHR);
        return val;
    }
}

/**
 * @brief Return number of bytes waiting in RX FIFO.
 * @return int Number of readable bytes.
 */
int Appnostic_SC16IS752::available() {
    return readRegister(channel, SC16IS7XX_REG_RXLVL);
}

/**
 * @brief Peek one byte from RX FIFO without consuming it.
 * @return int Next byte value, or -1 when no data is available.
 */
int Appnostic_SC16IS752::peek() {
    if (peek_flag == 0) {
        peek_buf = read();
        if (peek_buf >= 0) { peek_flag = 1; }
    }

    return peek_buf;
}

/**
 * @brief Write one byte to UART TX register.
 * @param val Byte to transmit.
 * @return size_t Number of bytes written.
 */
size_t Appnostic_SC16IS752::write(uint8_t val) {
    uint8_t tmp_lsr;

    do {
        tmp_lsr = readRegister(channel, SC16IS7XX_REG_LSR);
    } while ((tmp_lsr & 0x20) == 0);

    writeRegister(channel, SC16IS7XX_REG_THR, val);

    return 1;
}

/**
 * @brief Write a buffer to UART.
 * @param buf Pointer to bytes to transmit.
 * @param size Number of bytes to send.
 * @return size_t Number of bytes written.
 */
size_t Appnostic_SC16IS752::write(const uint8_t* buf, size_t size) {
    for (int i = 0; i < size; i++) { write(buf[i]); }
    return size;
}

/**
 * @brief Block until TX shift register is empty.
 */
void Appnostic_SC16IS752::flush() {
    uint8_t tmp_lsr;

    do {
        tmp_lsr = readRegister(channel, SC16IS7XX_REG_LSR);
    } while ((tmp_lsr & 0x20) == 0);
}
