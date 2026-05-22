/**
 * @file SC16IS7xx_UART.h
 * @copyright Stroud Water Research Center
 * Part of the EnviroDIY ModularSensors library for Arduino.
 * This library is published under the BSD-3 license.
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 *
 * @brief Declaration of the SC16IS7xx_UART class.
 */

#ifndef _SC16IS7XX_UART_H_
#define _SC16IS7XX_UART_H_

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else  // if ARDUINO >= 100
#include "WProgram.h"
#endif  // if ARDUINO >= 100

/**
 * @brief UART line format presets for @ref SC16IS7xx_UART::setLine and
 * @ref SC16IS7xx_UART::begin.
 *
 * These values use the Arduino-style packed serial config encoding that is
 * decoded by @ref SC16IS7xx_UART::setLine(uint8_t), then translated into LCR
 * bits.
 *
 * Bit interpretation of each enum value in @ref
 * SC16IS7xx_UART::setLine(uint8_t) (only bits 5:0 are used):
 *
 * | Bits   | Meaning                                           |
 * |:-------|:--------------------------------------------------|
 * | [2:0]  | Data bits: `0x00`=5, `0x02`=6, `0x04`=7, `0x06`=8 |
 * | [3]    | Stop bits: `0`=1 stop bit, `1`=2 stop bits        |
 * | [5:4]  | Parity: `0x00`=none, `0x20`=even, `0x30`=odd      |
 *
 * @note These values match exactly the Arduino HardwareSerial config values
 * (eg, SERIAL_8N1) in the **AVR** core, but may not match config values in
 * other cores. The `setLine(uint8_t)` function will decode these values as
 * described in the table above, so they can be used on any core, but the enum
 * values themselves are only guaranteed to match the AVR HardwareSerial config
 * values.
 */
enum class SC16IS7xxSerialConfig : uint8_t {
    C5N1 = 0x00,  ///< 5 data bits,   no parity, 1 stop bit  (00 0 000)
    C6N1 = 0x02,  ///< 6 data bits,   no parity, 1 stop bit  (00 0 010)
    C7N1 = 0x04,  ///< 7 data bits,   no parity, 1 stop bit  (00 0 100)
    C8N1 = 0x06,  ///< 8 data bits,   no parity, 1 stop bit  (00 0 110)
    C5N2 = 0x08,  ///< 5 data bits,   no parity, 2 stop bits (00 1 000)
    C6N2 = 0x0A,  ///< 6 data bits,   no parity, 2 stop bits (00 1 010)
    C7N2 = 0x0C,  ///< 7 data bits,   no parity, 2 stop bits (00 1 100)
    C8N2 = 0x0E,  ///< 8 data bits,   no parity, 2 stop bits (00 1 110)
    C5E1 = 0x20,  ///< 5 data bits, even parity, 1 stop bit  (10 0 000)
    C6E1 = 0x22,  ///< 6 data bits, even parity, 1 stop bit  (10 0 010)
    C7E1 = 0x24,  ///< 7 data bits, even parity, 1 stop bit  (10 0 100)
    C8E1 = 0x26,  ///< 8 data bits, even parity, 1 stop bit  (10 0 110)
    C5E2 = 0x28,  ///< 5 data bits, even parity, 2 stop bits (10 1 000)
    C6E2 = 0x2A,  ///< 6 data bits, even parity, 2 stop bits (10 1 010)
    C7E2 = 0x2C,  ///< 7 data bits, even parity, 2 stop bits (10 1 100)
    C8E2 = 0x2E,  ///< 8 data bits, even parity, 2 stop bits (10 1 110)
    C5O1 = 0x30,  ///< 5 data bits,  odd parity, 1 stop bit  (11 0 000)
    C6O1 = 0x32,  ///< 6 data bits,  odd parity, 1 stop bit  (11 0 010)
    C7O1 = 0x34,  ///< 7 data bits,  odd parity, 1 stop bit  (11 0 100)
    C8O1 = 0x36,  ///< 8 data bits,  odd parity, 1 stop bit  (11 0 110)
    C5O2 = 0x38,  ///< 5 data bits,  odd parity, 2 stop bits (11 1 000)
    C6O2 = 0x3A,  ///< 6 data bits,  odd parity, 2 stop bits (11 1 010)
    C7O2 = 0x3C,  ///< 7 data bits,  odd parity, 2 stop bits (11 1 100)
    C8O2 = 0x3E,  ///< 8 data bits,  odd parity, 2 stop bits (11 1 110)
};

// Forward declaration of the main class
class SC16IS7xx;

/**
 * @brief Per-UART-channel serial API bound to one SC16IS7xx device.
 */
class SC16IS7xx_UART : public Stream {
 private:
    friend class SC16IS7xx;

    SC16IS7xx_UART();
    SC16IS7xx_UART(SC16IS7xx* owner, uint8_t channel);

    SC16IS7xx* _owner            = nullptr;
    uint8_t    _channel          = SC16IS7XX_CHANNEL_A;
    uint32_t   _crystalFrequency = SC16IS7XX_DEFAULT_XTAL_FREQ;
    bool _peek_flag = false;  ///< Flag to indicate if there's a peeked byte
    int _peek_buf = -1;  ///< peeked byte value, valid only if _peek_flag is set

    uint8_t FIFOAvailableData();
    uint8_t FIFOAvailableSpace();
    int     rawRead();
    int     rawRead(uint8_t* buf, size_t size);
    void    EnableTransmit(uint8_t tx_enable);

    void configure(SC16IS7xx* owner, uint8_t channel);

 public:
    // uart configuration
    void     setCrystalFrequency(uint32_t frequency);
    uint32_t getCrystalFrequency() const;

    void enableFIFO(bool enabled);
    void resetFIFO(bool rx);
    void setFIFOTriggerLevel(bool rx, uint8_t length);
    void setBaudrate(uint32_t baudRate);
    void setLine(uint8_t dataBits, uint8_t parity, uint8_t stopBits);
    void setLine(SC16IS7xxSerialConfig config);
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

    // uart begin; aligned with HardwareSerial
    void begin(unsigned long baud);
    void begin(unsigned long baud, SC16IS7xxSerialConfig config);
    void begin(unsigned long baud, uint8_t config);
    void begin(unsigned long baud, uint8_t dataBits, uint8_t parity,
               uint8_t stopBits);

    // stream reading
    int read() override;
    int read(uint8_t* buf, size_t size);
    int available() override;
    int peek() override;

    // stream writing
    size_t write(const uint8_t* buf, size_t size) override;
    size_t write(uint8_t c) override;
    size_t write(const char* str);
    void   flush() override;
};

#endif  // _SC16IS7XX_UART_H_
