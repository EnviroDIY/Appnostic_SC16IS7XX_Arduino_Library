#pragma once

#ifndef _APPNOSTIC_SC16IS752_H_
#define _APPNOSTIC_SC16IS752_H_

#include "Appnostic_SC16IS7XX.h"

#define SC16IS752_CHANNEL_A 0x00
#define SC16IS752_CHANNEL_B 0x01
#define SC16IS752_CHANNEL_BOTH 0x00

/**
 * @brief UART line and FIFO settings used by Appnostic_SC16IS752.
 */
typedef struct {
    bool    fifo = true;    ///< true to enable FIFO, false to disable
    bool    baud = 115200;  ///< baud rate to set
    uint8_t bits = 8;       ///< number of data bits (5, 6, 7, or 8)
    bool parity  = false;   ///< parity mode (0: none, 1: odd, 2: even, 3: force
                            ///< '1', 4: force '0')
    uint8_t stopBits = 1;   ///< number of stop bits (1 or 2)
} uart_settings_t;

/**
 * @brief SC16IS752 dual-channel UART driver.
 */
class Appnostic_SC16IS752 : public Appnostic_SC16IS7XX {
 private:
    uart_settings_t settings;
    uint8_t         channel;
    uint8_t         peek_flag      = 0;
    int             peek_buf       = -1;
    uint8_t         fifo_available = 0;

    uint8_t FIFOAvailableData();
    uint8_t FIFOAvailableSpace();

 public:
    Appnostic_SC16IS752(uint8_t channel);

    // reading and writing from registers
    void    writeRegister(uint8_t channel, uint8_t reg_addr, uint8_t val);
    uint8_t readRegister(uint8_t channel, uint8_t reg_addr);

    // derived functions from base class
    virtual bool ping() override;

    // uart configuration
    void setFIFO(bool enabled);
    void resetFIFO(bool rx);
    void setFIFOTriggerLevel(bool rx, uint8_t length);
    void setBaudrate(uint32_t baudRate);
    void setLine(uint8_t bits, uint8_t parity, uint8_t stopBits);

    // stream reading
    int read();
    int available();
    int peek();

    // stream writing
    size_t write(uint8_t val);
    size_t write(const uint8_t* buf, size_t size);
    void   flush();
};

/**
 * @brief Default external serial instance.
 */
extern Appnostic_SC16IS752 ExtSerial;

#endif
