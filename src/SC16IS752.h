#pragma once

#ifndef _APPNOSTIC_SC16IS752_H_
#define _APPNOSTIC_SC16IS752_H_

#include "SC16IS7XX.h"

#define SC16IS752_CHANNEL_A 0x00
#define SC16IS752_CHANNEL_B 0x01
#define SC16IS752_CHANNEL_BOTH 0x00

// Define config for Serial.begin(baud, config);
// Copied from Arduino's HardwareSerial.h for compatibility with
// Serial.begin(baud, config) style calls
#define SERIAL_5N1 (0x00)  // 00 00 0 000
#define SERIAL_6N1 (0x02)  // 00 00 0 010
#define SERIAL_7N1 (0x04)  // 00 00 0 100
#define SERIAL_8N1 (0x06)  // 00 00 0 110
#define SERIAL_5N2 (0x08)  // 00 00 1 000
#define SERIAL_6N2 (0x0A)  // 00 00 1 010
#define SERIAL_7N2 (0x0C)  // 00 00 1 100
#define SERIAL_8N2 (0x0E)  // 00 00 1 110
#define SERIAL_5E1 (0x20)  // 00 10 0 000
#define SERIAL_6E1 (0x22)  // 00 10 0 010
#define SERIAL_7E1 (0x24)  // 00 10 0 100
#define SERIAL_8E1 (0x26)  // 00 10 0 110
#define SERIAL_5E2 (0x28)  // 00 10 1 000
#define SERIAL_6E2 (0x2A)  // 00 10 1 010
#define SERIAL_7E2 (0x2C)  // 00 10 1 100
#define SERIAL_8E2 (0x2E)  // 00 10 1 110
#define SERIAL_5O1 (0x30)  // 00 11 0 000
#define SERIAL_6O1 (0x32)  // 00 11 0 010
#define SERIAL_7O1 (0x34)  // 00 11 0 100
#define SERIAL_8O1 (0x36)  // 00 11 0 110
#define SERIAL_5O2 (0x38)  // 00 11 1 000
#define SERIAL_6O2 (0x3A)  // 00 11 1 010
#define SERIAL_7O2 (0x3C)  // 00 11 1 100
#define SERIAL_8O2 (0x3E)  // 00 11 1 110

/**
 * @brief SC16IS752 dual-channel UART driver.
 */
class SC16IS752 : public SC16IS7XX, public Stream {
 private:
    uint8_t _channel;
    bool    _peek_flag = 0;  ///< Flag to indicate if there's a peeked byte
    int _peek_buf = -1;  ///< peeked byte value, valid only if _peek_flag is set

    uint8_t FIFOAvailableSpace();
    int     rawRead();
    int     rawRead(uint8_t* buf, size_t size);
    void    EnableTransmit(uint8_t tx_enable);

 public:
    SC16IS752(uint8_t channel);

    // uart configuration
    void enableFIFO(bool enabled);
    void resetFIFO(bool rx);
    void setFIFOTriggerLevel(bool rx, uint8_t length);
    void setBaudrate(uint32_t baudRate);
    void setLine(uint8_t dataBits, uint8_t parity, uint8_t stopBits);
    void setLine(uint8_t config);

    // uart begin/end, aligned with HardwareSerial
    void begin(unsigned long baud) {
        begin(baud, SERIAL_8N1);
    }
    void begin(unsigned long baud, uint8_t config);
    void begin(unsigned long baud, uint8_t dataBits, uint8_t parity,
               uint8_t stopBits);
    void end() {}

    // stream reading
    int read();
    int read(uint8_t* buf, size_t size);
    int available();
    int peek();

    // stream writing
    size_t write(const uint8_t* buf, size_t size);
    size_t write(uint8_t c);
    size_t write(const char* str);
    void   flush();
};

/**
 * @brief Default external serial instance.
 */
extern SC16IS752 ExtSerial;

#endif
