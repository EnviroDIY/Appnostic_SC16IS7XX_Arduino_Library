/**
 * @file SC16IS7XX.h
 * @copyright Stroud Water Research Center
 * Part of the EnviroDIY ModularSensors library for Arduino.
 * This library is published under the BSD-3 license.
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 *
 * @brief Contains the SC16IS7XX class and defines for many of the
 * device's features.
 */

#ifndef _SC16IS7XX_H_
#define _SC16IS7XX_H_

#if ARDUINO >= 100
#include "Arduino.h"
#else  // if ARDUINO >= 100
#include "WProgram.h"
#endif  // if ARDUINO >= 100

#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SPIDevice.h>

/**
 * @def ISR_MEM_ACCESS
 * @brief Defines a memory access location, if needed for the interrupts service
 * routines.
 *
 * On Espressif boards (ESP8266 and ESP32), the ISR must be stored in IRAM
 */
#if (defined(ESP32) || defined(ESP8266)) && !defined(ISR_MEM_ACCESS)
#define ISR_MEM_ACCESS IRAM_ATTR
#else
#define ISR_MEM_ACCESS
#endif  // defined(ESP32) || defined(ESP8266)


/**
 * @anchor chip_features
 * @name Chip Features
 * Features of the SC16IS7XX family of devices.
 */
/**@{*/
#define SC16IS7XX_UART_CHANNELS (2)  ///< Number of UART channels on the device
#define SC16IS7XX_GPIO_PINS (8)      ///< Number of GPIO pins on the device
#define SC16IS7XX_NON_GPIO_INTERRUPTS \
    (11)  ///< The number of non-pin-related interrupt sources (DSR, DTR, CD,
          ///< RI,
          ///< CTS, RTS, XOFF, RLS, RHR, THR, Timeout) on the device. These
///< interrupts are shared between both channels and are not related to
///< the GPIO pins.
#define SC16IS7XX_SPIREG ADDRBIT8_HIGH_TOREAD  ///< SPI register type
/**@}*/

/**
 * @page page_device_addresses Possible Device Addresses
 *
 * Possible **8-bit** I2C addresses for the SC16IS7XX family of devices.
 *
 * Modified from Table 32. SC16IS752/SC16IS762 address map
 *
 * @note For the purposes of the Arduino Wire library, use the 7-bit address!
 * The begin_i2c() function will convert 8-bit addresses to 7-bit if needed.  We
 * list the 8-bit addresses here because the datasheet lists them in 8-bit form
 * and it is easier to cross reference with the datasheet this way. The 7-bit
 * address is just the 8-bit address shifted right by one bit (or divided by 2).
 *
 * | A1  | A0  | Binary**   | Hex Address (W/R) | 7-Bit Address |
 * |-----|-----|------------|-------------------|---------------|
 * | VDD | VDD | 1001 000X  | 0x90 / 0x91       | 0x48          |
 * | VDD | VSS | 1001 001X  | 0x92 / 0x93       | 0x49          |
 * | VDD | SCL | 1001 010X  | 0x94 / 0x95       | 0x4A          |
 * | VDD | SDA | 1001 011X  | 0x96 / 0x97       | 0x4B          |
 * | VSS | VDD | 1001 100X  | 0x98 / 0x99       | 0x4C          |
 * | VSS | VSS | 1001 101X  | 0x9A / 0x9B       | 0x4D          |
 * | VSS | SCL | 1001 110X  | 0x9C / 0x9D       | 0x4E          |
 * | VSS | SDA | 1001 111X  | 0x9E / 0x9F       | 0x4F          |
 * | SCL | VDD | 1010 000X  | 0xA0 / 0xA1       | 0x50          |
 * | SCL | VSS | 1010 001X  | 0xA2 / 0xA3       | 0x51          |
 * | SCL | SCL | 1010 010X  | 0xA4 / 0xA5       | 0x52          |
 * | SCL | SDA | 1010 011X  | 0xA6 / 0xA7       | 0x53          |
 * | SDA | VDD | 1010 100X  | 0xA8 / 0xA9       | 0x54          |
 * | SDA | VSS | 1010 101X  | 0xAA / 0xAB       | 0x55          |
 * | SDA | SCL | 1010 110X  | 0xAC / 0xAD       | 0x56          |
 * | SDA | SDA | 1010 111X  | 0xAE / 0xAF       | 0x57          |
 *
 * ** X = logic 0 for write cycle; X = logic 1 for read cycle.
 */
/**
 * @anchor device_addresses
 * @name Device Addresses
 */
/**@{*/
// A:VDD (positive voltage)
// B:VSS (ground)
// C:SCL (I2C clock)
// D:SDA (I2C data)
#define SC16IS7XX_ADDRESS_AA (0X90)  ///< A tied to VDD, B tied to VDD
#define SC16IS7XX_ADDRESS_AB (0X92)  ///< A tied to VDD, B tied to GND
#define SC16IS7XX_ADDRESS_AC (0X94)  ///< A tied to VDD, B tied to SCL
#define SC16IS7XX_ADDRESS_AD (0X96)  ///< A tied to VDD, B tied to SDA
#define SC16IS7XX_ADDRESS_BA (0X98)  ///< A tied to GND, B tied to VDD
#define SC16IS7XX_ADDRESS_BB (0X9A)  ///< A tied to GND, B tied to GND
#define SC16IS7XX_ADDRESS_BC (0X9C)  ///< A tied to GND, B tied to SCL
#define SC16IS7XX_ADDRESS_BD (0X9E)  ///< A tied to GND, B tied to SDA
#define SC16IS7XX_ADDRESS_CA (0XA0)  ///< A tied to SCL, B tied to VDD
#define SC16IS7XX_ADDRESS_CB (0XA2)  ///< A tied to SCL, B tied to GND
#define SC16IS7XX_ADDRESS_CC (0XA4)  ///< A tied to SCL, B tied to SCL
#define SC16IS7XX_ADDRESS_CD (0XA6)  ///< A tied to SCL, B tied to SDA
#define SC16IS7XX_ADDRESS_DA (0XA8)  ///< A tied to SDA, B tied to VDD
#define SC16IS7XX_ADDRESS_DB (0XAA)  ///< A tied to SDA, B tied to GND
#define SC16IS7XX_ADDRESS_DC (0XAC)  ///< A tied to SDA, B tied to SCL
#define SC16IS7XX_ADDRESS_DD (0XAE)  ///< A tied to SDA, B tied to SDA
/**@}*/

// clang-format off
/**
 * @anchor registers
 * @name Registers
 *
 * | Bit(s) | Name     | Function                                                      |
 * |--------|----------|---------------------------------------------------------------|
 * | 7      | -        | Not used                                                      |
 * | 6:3    | A[3:0]   | UART’s internal register select                               |
 * | 2:1    | CH1, CH0 | Channel select [00 = A, 01 = B, 10 = reserved, 11 = reserved] |
 * | 0      | -        | Not used                                                      |
 *
 * @note Because the address is shifted left by 3 bits and the channel is
 * shifted left by 1 bit, the register address to use in the Wire functions is
 * effectively (reg_addr << 3 | channel << 1) when writing or reading registers
 * for a specific channel on the SC16IS752.
 */
/**@{*/
// clang-format on
// General registers
#define SC16IS7XX_REG_RHR (0x00)  ///< Receiver Holding Register (read only)
#define SC16IS7XX_REG_THR (0X00)  ///< Transmit Holding Register (write only)
#define SC16IS7XX_REG_IER (0X01)  ///< Interrupt Enable Register
#define SC16IS7XX_REG_FCR (0X02)  ///< FIFO Control Register (write only)
#define SC16IS7XX_REG_IIR \
    (0X02)  ///< Interrupt Identification Register (read only)
#define SC16IS7XX_REG_LCR (0X03)  ///< Line Control Register
#define SC16IS7XX_REG_MCR (0X04)  ///< Modem Control Register
#define SC16IS7XX_REG_LSR (0X05)  ///< Line Status Register (read only)
#define SC16IS7XX_REG_MSR (0X06)  ///< Modem Status Register (read only)
#define SC16IS7XX_REG_SPR (0X07)  ///< Scratchpad Register
#define SC16IS7XX_REG_TCR (0X06)  ///< Transmission Control Register
#define SC16IS7XX_REG_TLR (0X07)  ///< Trigger Level Register
#define SC16IS7XX_REG_TXLVL \
    (0X08)  ///< Transmit FIFO Level Register (read only)
#define SC16IS7XX_REG_RXLVL (0X09)  ///< Receive FIFO Level Register (read only)
#define SC16IS7XX_REG_IODIR (0X0A)  ///< I/O Direction Register
#define SC16IS7XX_REG_IOSTATE (0X0B)    ///< I/O State Register
#define SC16IS7XX_REG_IOINTENA (0X0C)   ///< I/O Interrupt Enable Register
#define SC16IS7XX_REG_IOCONTROL (0X0E)  ///< I/O Control Register
#define SC16IS7XX_REG_EFCR (0X0F)       ///< Extra Features Control Register

// Special Registers
#define SC16IS7XX_REG_DLL (0x00)  ///< Divisor Latch LSB
#define SC16IS7XX_REG_DLH (0X01)  ///< Divisor Latch MSB

// Enhanced Registers
#define SC16IS7XX_REG_EFR (0X02)    ///< Enhanced Feature Register
#define SC16IS7XX_REG_XON1 (0X04)   ///< XON1 Register
#define SC16IS7XX_REG_XON2 (0X05)   ///< XON2 Register
#define SC16IS7XX_REG_XOFF1 (0X06)  ///< XOFF1 Register
#define SC16IS7XX_REG_XOFF2 (0X07)  ///< XOFF2 Register
/**@}*/


/**
 * @anchor msr_bits
 * @name Modem Status Register Bits
 *
 * Taken from datasheet table 21
 */
/**@{*/
#define SC16IS7XX_MSR_CD (0X07)   ///< Current CD/DCD State
#define SC16IS7XX_MSR_RI (0X06)   ///< Current RI State
#define SC16IS7XX_MSR_DSR (0X05)  ///< Current DSR State
#define SC16IS7XX_MSR_CTS (0X04)  ///< Current CTS State
#define SC16IS7XX_MSR_DELTA_CD \
    (0X03)  ///< Indicates that CD input has changed state
#define SC16IS7XX_MSR_DELTA_RI \
    (0X02)  ///< Indicates that RI input has changed state
#define SC16IS7XX_MSR_DELTA_DSR \
    (0X01)  ///< Indicates that DSR input has changed state
#define SC16IS7XX_MSR_DELTA_CTS \
    (0X00)  ///< Indicates that CTS input has changed state
/**@}*/


/**
 * @anchor interrupt_bits
 * @name Interrupt Enable Register Bits
 *
 * Taken from datasheet table 11
 */
/**@{*/
#define SC16IS7XX_IER_CTS (0X07)    ///< CTS Interrupt Enable
#define SC16IS7XX_IER_RTS (0X06)    ///< RTS Interrupt Enable
#define SC16IS7XX_IER_XOFF (0X05)   ///< XOFF Interrupt
#define SC16IS7XX_IER_SLEEP (0X04)  ///< Sleep Mode Enable (NOT AN INTERRUPT)
#define SC16IS7XX_IER_MODEM (0X03)  ///< Modem Interrupt
///< @note The modem interrupt is triggered by changes in the RI, CD, DTR, or
///< DSR pins **if and only if** the I/O pins are configured as modem pins.
#define SC16IS7XX_IER_RLS (0X02)  ///< Receiver Line Status Interrupt
#define SC16IS7XX_IER_THR (0X01)  ///< Transmit Holding Register Interrupt
#define SC16IS7XX_IER_RHR (0X00)  ///< Receive Holding Register Interrupt
/**@}*/

/**
 * @anchor interrupt_sources
 * @name Interrupt Identification Register Values
 *
 * Taken from datasheet table 14
 */
/**@{*/
// Listed from increasing to decreasing priority
#define SC16IS7XX_IIR_LINE (0X06)     ///< Line Interrupt 0b00000110
#define SC16IS7XX_IIR_TIMEOUT (0X0c)  ///< Timeout Interrupt 0b00001100
#define SC16IS7XX_IIR_RHR \
    (0X04)  ///< Receive Holding Register Interrupt 0b00000100
#define SC16IS7XX_IIR_THR \
    (0X02)  ///< Transmit Holding Register Interrupt 0b00000010
#define SC16IS7XX_IIR_MODEM (0X00)   ///< Modem Interrupt 0b00000000
#define SC16IS7XX_IIR_GPIO (0x30)    ///< GPIO Interrupt 0b00110000
#define SC16IS7XX_IIR_XOFF (0X10)    ///< XOFF Interrupt 0b00010000
#define SC16IS7XX_IIR_CTSRTS (0X20)  ///< CTS/RTS Interrupt 0b00100000
/**@}*/

/**
 * @page page_supported_interrupts Supported Interrupts
 *
 * The SC16IS7XX supports the following interrupts, which will trigger a low
 * signal on the IRQ pin when triggered:
 *
 *  - GPIO Interrupts
 *    - 8 pin interrupts, one on each GPIO pin, triggered by a change in the
 * state of the pin
 *       - The pin interrupts cannot be configured for rising or falling edge,
 * only change
 *       - If the chip is not configured for "latching" interrupts, it is not
 * possible to tell which pin triggered the interrupt, and the interrupt will be
 * cleared when the IIR register is read. If the chip is configured for
 * "latching" interrupts, then the state of the pin at the time of the interrupt
 * will be latched and can be read from the IOSTATE register, and the interrupt
 * will only be cleared when the state of the pin changes from its state at the
 * time of the interrupt. *
 *  - Flow Control Interrupts:
 *    - DSR (Data Set Ready) – Sent by the Data Communication Equipment (DCE)
 * (e.g., modem) to the Data Terminal Equipment (DTE) (e.g., computer) to
 * indicate it is operational and ready to receive data
 *    - DTR (Data Terminal Ready) is asserted by DTE to indicate that it is
 * powered, ready, and able to communicate.
 *    - CD (Carrier Detect) - also called DCD (Data Carrier Detect) - indicates
 * that a valid carrier signal from the remote device has been detected.
 *    - CTS (Clear to Send) indicates that the device is ready to accept data.
 *    - RTS (Request to Send) indicates that the device is ready to send data.
 *    - XOFF (Transmit Off) indicates that the device should stop sending data.
 *  - Modem Alert Interrupt:
 *    - RI (Ring Indicator) signals that an incoming call or alert condition is
 * present on the connected modem or communications line.
 *  - Line Status Interrupts:
 *    - RLS (Receiver Line Status) indicates an Overrun Error (OE), Framing
 * Error (FE), Parity Error (PE), or Break Interrupt (BI) error in the received
 * data.
 *    - THR (Transmit Holding Register)
 *      - If the transmit FIFO is disabled, the THR interrupt is triggered when
 * the transmit FIFO is empty.
 *      - If the transmit FIFO is enabled, the THR interrupt is triggered when
 * the number of bytes in the transmit FIFO is less than or equal to the value
 * in the Trigger Level Register (TLR).
 *    - RHR (Receive Holding Register)
 *      - If the receive FIFO is disabled, the RHR interrupt is triggered when a
 * byte of data is received and placed in the RHR register.
 *      - If the receive FIFO is enabled, the RHR interrupt is triggered when
 * the number of bytes in the receive FIFO is greater than or equal to the value
 * in the Trigger Level Register (TLR).
 *    - Timeout Interrupt occurs when the UART receives a number of characters
 * and these data are not enough to set off the receive interrupt (because they
 * do not reach the receive trigger level), the UART will generate a time-out
 * interrupt instead, 4 character times after the last character is received.
 * The time-out counter will be reset at the center of each stop bit received or
 * each time the receive FIFO is read.
 *
 * The total number of interrupts that can be attached to the IRQ pin is 19,
 * which includes both GPIO and non-GPIO interrupts.
 *
 * @note
 * - Enabling any of the MODEM, RLS, THR, or RHR interrupts put the module into
 * "Interrupt Mode Operation".  In this mode, the host is informted of the
 * status of the rx and tx via the interrupt signal (the IRQ pin) and should
 * check the IIR register for the source of the interrupt. *
 * - If you do NOT enable any of the MODEM, RLS, THR, or RHR interrupts, then
 * the module is in "Polling Mode Operation" and the host should check the LSR
 * and MSR registers to check the status of the rx and tx. *
 * - Enabling the CTS, RTS, or XOFF interrupts does NOT put the module into
 * "Interrupt Mode Operation". The host should check the IIR register for the
 * source of the interrupt if any of these are enabled, but they can be enabled
 * in either mode of operation.
 */
/**
 * @anchor interrupt_bitmasks
 * @name Interrupt Bitmasks
 */
/**@{*/
#define SC16IS7XX_INT_MASK_RI   \
    (SC16IS7XX_IIR_MODEM << 8 | \
     1 << SC16IS7XX_MSR_DELTA_RI)  ///< RI Interrupt bitmask
#define SC16IS7XX_INT_MASK_CD   \
    (SC16IS7XX_IIR_MODEM << 8 | \
     1 << SC16IS7XX_MSR_DELTA_CD)  ///< CD Interrupt bitmask
#define SC16IS7XX_INT_MASK_DSR  \
    (SC16IS7XX_IIR_MODEM << 8 | \
     1 << SC16IS7XX_MSR_DELTA_DSR)  ///< DSR Interrupt bitmask
#define SC16IS7XX_INT_MASK_CTS   \
    (SC16IS7XX_IIR_CTSRTS << 8 | \
     1 << SC16IS7XX_MSR_DELTA_CTS)  ///< CTS Interrupt bitmask

#define SC16IS7XX_INT_MASK_DTR \
    (SC16IS7XX_IIR_MODEM << 8)  ///< DTR Interrupt bitmask (no MRS bits set)
#define SC16IS7XX_INT_MASK_RTS \
    (SC16IS7XX_IIR_CTSRTS << 8)  ///< RTS Interrupt bitmask (no MRS bits set)

#define SC16IS7XX_INT_MASK_XOFF \
    (SC16IS7XX_IIR_XOFF << 8)  ///< XOFF Interrupt bitmask
#define SC16IS7XX_INT_MASK_RLS \
    (SC16IS7XX_IIR_LINE << 8)  ///< Line Status Interrupt bitmask
#define SC16IS7XX_INT_MASK_THR \
    (SC16IS7XX_IIR_THR << 8)  ///< THR Interrupt bitmask
#define SC16IS7XX_INT_MASK_RHR \
    (SC16IS7XX_IIR_RHR << 8)  ///< RHR Interrupt bitmask
#define SC16IS7XX_INT_MASK_TIMEOUT \
    (SC16IS7XX_IIR_TIMEOUT << 8)  ///< Timeout Interrupt bitmask

#define SC16IS7XX_NO_INTERRUPT (0XFFFF)  ///< Bitmask for no interrupt


/**@}*/

/**
 * @anchor default_values
 * @name Default Values
 */
/**@{*/
#define SC16IS7XX_DEFAULT_ADDRESS \
    (SC16IS7XX_ADDRESS_AA)                     ///< The default I2C address
#define SC16IS7XX_DEFAULT_SPIFREQ (1000000UL)  ///< The default SPI Clock speed
#define SC16IS7XX_DEFAULT_XTAL_FREQ \
    (14745600UL)  ///< The default frequency of the crystal in hertz
/**@}*/

/// @brief  Function pointer type for interrupt callbacks
typedef void (*voidFxnPtr)(void);

/**
 * @brief Base driver for SC16IS7XX family devices using I2C or SPI.
 */
class SC16IS7XX {
 private:
    uint32_t crystal_frequency = SC16IS7XX_DEFAULT_XTAL_FREQ;

    bool _init(void);

    voidFxnPtr ISRcallback[SC16IS7XX_GPIO_PINS + SC16IS7XX_NON_GPIO_INTERRUPTS];
    uint16_t   ISRlist[SC16IS7XX_GPIO_PINS + SC16IS7XX_NON_GPIO_INTERRUPTS];
    uint8_t    nints;  // Stores total number of attached interrupts

 protected:
    Adafruit_I2CDevice* i2c_dev = nullptr;  ///< Pointer to I2C bus interface
    Adafruit_SPIDevice* spi_dev = nullptr;  ///< Pointer to SPI bus interface

    virtual void resetDevice();
    virtual bool ping();

    void storeCallback(uint16_t callbackMask, voidFxnPtr callback);
    void clearCallback(uint16_t callbackMask);
    void callCallback(uint16_t callbackMask);
    /// static pointer to active SC16IS7XX instance, needed for easy ISR
    /// handling
    static SC16IS7XX* _activeObject;

    /// flag to indicate whether the GPIO interrupts are configured for latching
    /// or not. If not, the state of the pin at the time of the interrupt will
    /// not be latched and the interrupt will be cleared when the IIR register
    /// is read. If the GPIO interrupts are configured for latching, then the
    /// state of the pin at the time of the interrupt will be latched and can be
    /// read from the IOSTATE register, and the interrupt will only be cleared
    /// when the state of the pin changes from its state at the time of the
    /// interrupt.
    bool gpioInterruptsLatched = false;

 public:
    /**
     * @brief Construct a new SC16IS7XX object.
     */
    SC16IS7XX() = default;

    /**
     * @brief Destroy the SC16IS7XX object.
     */
    ~SC16IS7XX() = default;

    // i2c
    bool begin_i2c(uint8_t  addr    = SC16IS7XX_DEFAULT_ADDRESS,
                   TwoWire* theWire = &Wire);

    // spi
    bool begin_SPI(uint8_t cs_pin, SPIClass* theSPI = &SPI,
                   uint32_t frequency = SC16IS7XX_DEFAULT_SPIFREQ);
    bool begin_SPI(int8_t cs_pin, int8_t sck_pin, int8_t miso_pin,
                   int8_t   mosi_pin,
                   uint32_t frequency = SC16IS7XX_DEFAULT_SPIFREQ);

    // Need an end function to switch between active objects for the interrupt
    // handling.
    void end();

    // configuration
    void     setCrystalFrequency(uint32_t frequency);
    uint32_t getCrystalFrequency();

    // gpio - need a uniquely named function for each of the digital read/write
    // and pin mode functions to not conflict with defines in some of the cores
    void    pinModeExternal(uint8_t pin, uint8_t mode);
    void    digitalWriteExternal(uint8_t pin, uint8_t state);
    uint8_t digitalReadExternal(uint8_t pin);
    void attachPinInterrupt(uint8_t pin, voidFxnPtr callback, uint8_t = 0);
    void detachPinInterrupt(uint8_t pin);
#if !(defined(ESP32) && defined(ESP_ARDUINO_VERSION_MAJOR) && \
      ESP_ARDUINO_VERSION_MAJOR <= 2)
    // for cores without conflict, these are simpler
    /// @copydoc SC16IS7XX::pinModeExternal
    void pinMode(uint8_t pin, uint8_t mode) {
        pinModeExternal(pin, mode);
    };
    /// @copydoc SC16IS7XX::digitalWriteExternal
    void digitalWrite(uint8_t pin, uint8_t state) {
        digitalWriteExternal(pin, state);
    };
    /// @copydoc SC16IS7XX::digitalReadExternal
    uint8_t digitalRead(uint8_t pin) {
        return digitalReadExternal(pin);
    };
    /// @copydoc SC16IS7XX::attachPinInterrupt
    void attachInterrupt(uint8_t pin, voidFxnPtr callback, uint8_t = 0) {
        attachPinInterrupt(pin, callback);
    };
    /// @copydoc SC16IS7XX::detachPinInterrupt
    void detachInterrupt(uint8_t pin) {
        detachPinInterrupt(pin);
    };
#endif

    void enableSleepMode(bool enabled);
    bool isSleepEnabled();

    void    setPinInterrupt(uint8_t pin, bool enabled);
    uint8_t getPinInterrupt(uint8_t pin);

    void    setPortState(uint8_t state);
    uint8_t getPortState();
    void    setPortMode(uint8_t mode);
    uint8_t getPortMode();

    void setGPIOLatch(bool enabled);

    uint16_t getInterruptSource();
#ifdef SC16IS752_DEBUG_SERIAL
    void printInterruptSource(uint16_t callbackMask);
#endif  // SC16IS752_DEBUG_SERIAL
    void        handleInterrupt(uint16_t callbackMask);
    static void interruptHandler(void);
};

#endif  // _SC16IS7XX_H_

// cSpell:words SPIFREQ SPIREG MISO MOSI
