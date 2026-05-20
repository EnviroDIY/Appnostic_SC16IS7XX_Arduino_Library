/**
 * @file SC16IS752.h
 * @copyright Stroud Water Research Center
 * Part of the EnviroDIY ModularSensors library for Arduino.
 * This library is published under the BSD-3 license.
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 *
 * @brief Contains the SC16IS752 class.
 */

#ifndef _SC16IS752_H_
#define _SC16IS752_H_
#include "SC16IS7XX.h"

#define SC16IS752_CHANNEL_A 0x00  ///< Channel A of the SC16IS752
#define SC16IS752_CHANNEL_B 0x01  ///< Channel B of the SC16IS752

// Define config for Serial.begin(baud, config);
// Copied from Arduino's HardwareSerial.h for compatibility with
// Serial.begin(baud, config) style calls
// To not conflict with Arduino's SERIAL_* defines, we only define these if they
// aren't already defined.  We assume that if SERIAL_8N1 is defined, then all
// the other SERIAL_* configs are also defined.
#ifndef SERIAL_8N1
/// 5 data bits, no parity, 1 stop bit
#define SERIAL_5N1 (0x00)  // 00 00 0 000
/// 6 data bits, no parity, 1 stop bit
#define SERIAL_6N1 (0x02)  // 00 00 0 010
/// 7 data bits, no parity, 1 stop bit
#define SERIAL_7N1 (0x04)  // 00 00 0 100
/// 8 data bits, no parity, 1 stop bit
#define SERIAL_8N1 (0x06)  // 00 00 0 110
/// 5 data bits, no parity, 2 stop bits
#define SERIAL_5N2 (0x08)  // 00 00 1 000
/// 6 data bits, no parity, 2 stop bits
#define SERIAL_6N2 (0x0A)  // 00 00 1 010
/// 7 data bits, no parity, 2 stop bits
#define SERIAL_7N2 (0x0C)  // 00 00 1 100
/// 8 data bits, no parity, 2 stop bits
#define SERIAL_8N2 (0x0E)  // 00 00 1 110
/// 5 data bits, even parity, 1 stop bit
#define SERIAL_5E1 (0x20)  // 00 10 0 000
/// 6 data bits, even parity, 1 stop bit
#define SERIAL_6E1 (0x22)  // 00 10 0 010
/// 7 data bits, even parity, 1 stop bit
#define SERIAL_7E1 (0x24)  // 00 10 0 100
/// 8 data bits, even parity, 1 stop bit
#define SERIAL_8E1 (0x26)  // 00 10 0 110
/// 5 data bits, even parity, 2 stop bits
#define SERIAL_5E2 (0x28)  // 00 10 1 000
/// 6 data bits, even parity, 2 stop bits
#define SERIAL_6E2 (0x2A)  // 00 10 1 010
/// 7 data bits, even parity, 2 stop bits
#define SERIAL_7E2 (0x2C)  // 00 10 1 100
/// 8 data bits, even parity, 2 stop bits
#define SERIAL_8E2 (0x2E)  // 00 10 1 110
/// 5 data bits, odd parity, 1 stop bit
#define SERIAL_5O1 (0x30)  // 00 11 0 000
/// 6 data bits, odd parity, 1 stop bit
#define SERIAL_6O1 (0x32)  // 00 11 0 010
/// 7 data bits, odd parity, 1 stop bit
#define SERIAL_7O1 (0x34)  // 00 11 0 100
/// 8 data bits, odd parity, 1 stop bit
#define SERIAL_8O1 (0x36)  // 00 11 0 110
/// 5 data bits, odd parity, 2 stop bits
#define SERIAL_5O2 (0x38)  // 00 11 1 000
/// 6 data bits, odd parity, 2 stop bits
#define SERIAL_6O2 (0x3A)  // 00 11 1 010
/// 7 data bits, odd parity, 2 stop bits
#define SERIAL_7O2 (0x3C)  // 00 11 1 100
/// 8 data bits, odd parity, 2 stop bits
#define SERIAL_8O2 (0x3E)  // 00 11 1 110
#endif

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
    void enableFlowControl(bool enabled);

    void enableModemInterrupt(bool enabled);
    void enableCTSInterrupt(bool enabled);
    void enableRTSInterrupt(bool enabled);
    void enableXOFFInterrupt(bool enabled);
    void enableRLSInterrupt(bool enabled);
    void enableTHRInterrupt(bool enabled);
    void enableRHRInterrupt(bool enabled);

    // RI - ring indicator
    void attachRIInterrupt(voidFxnPtr callback);
    void detachRIInterrupt();
    // CD - carrier detect (aka Data Carrier Detect (DCD))
    void attachCDInterrupt(voidFxnPtr callback);
    void detachCDInterrupt();
    // DSR - data set ready
    void attachDSRInterrupt(voidFxnPtr callback);
    void detachDSRInterrupt();
    // DTR - data terminal ready
    void attachDTRInterrupt(voidFxnPtr callback);
    void detachDTRInterrupt();

    // CTS - clear to send (used in CTS/RTS hardware flow control)
    void attachCTSInterrupt(voidFxnPtr callback);
    void detachCTSInterrupt();
    // RTS - request to send (used in CTS/RTS hardware flow control)
    void attachRTSInterrupt(voidFxnPtr callback);
    void detachRTSInterrupt();

    // XOFF - software flow control
    void attachXOFFInterrupt(voidFxnPtr callback);
    void detachXOFFInterrupt();

    // RLS - receive line status
    void attachRLSInterrupt(voidFxnPtr callback);
    void detachRLSInterrupt();
    // THR - transmit holding register status/empty
    void attachTHRInterrupt(voidFxnPtr callback);
    void detachTHRInterrupt();
    // RHR - receive holding register status/full
    void attachRHRInterrupt(voidFxnPtr callback);
    void detachRHRInterrupt();
    // Timeout
    void attachTimeoutInterrupt(voidFxnPtr callback);
    void detachTimeoutInterrupt();

    // uart begin/end, aligned with HardwareSerial
    void begin(unsigned long baud);
    void begin(unsigned long baud, uint8_t config);
    void begin(unsigned long baud, uint8_t dataBits, uint8_t parity,
               uint8_t stopBits);
    /// @brief  End UART communication. This function is provided for API
    /// compatibility with HardwareSerial, but does not do anything.
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

#endif  // _SC16IS752_H_
