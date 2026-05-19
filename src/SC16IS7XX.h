#ifndef _APPNOSTIC_SC16IS7XX_H_
#define _APPNOSTIC_SC16IS7XX_H_

#if ARDUINO >= 100
#include "Arduino.h"
#else  // if ARDUINO >= 100
#include "WProgram.h"
#endif  // if ARDUINO >= 100

#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SPIDevice.h>

// Possible Device Addresses
// Modified from Table 32. SC16IS752/SC16IS762 address map
// NOTE: For the purposes of the Arduino Wire library, use the 7-bit address!

/*
| A1  | A0  | Binary     | Hex Address (W/R) | 7-Bit Address |
|-----|-----|------------|-------------------|---------------|
| VDD | VDD | 1001 000X  | 0x90 / 0x91       | 0x48          |
| VDD | VSS | 1001 001X  | 0x92 / 0x93       | 0x49          |
| VDD | SCL | 1001 010X  | 0x94 / 0x95       | 0x4A          |
| VDD | SDA | 1001 011X  | 0x96 / 0x97       | 0x4B          |
| VSS | VDD | 1001 100X  | 0x98 / 0x99       | 0x4C          |
| VSS | VSS | 1001 101X  | 0x9A / 0x9B       | 0x4D          |
| VSS | SCL | 1001 110X  | 0x9C / 0x9D       | 0x4E          |
| VSS | SDA | 1001 111X  | 0x9E / 0x9F       | 0x4F          |
| SCL | VDD | 1010 000X  | 0xA0 / 0xA1       | 0x50          |
| SCL | VSS | 1010 001X  | 0xA2 / 0xA3       | 0x51          |
| SCL | SCL | 1010 010X  | 0xA4 / 0xA5       | 0x52          |
| SCL | SDA | 1010 011X  | 0xA6 / 0xA7       | 0x53          |
| SDA | VDD | 1010 100X  | 0xA8 / 0xA9       | 0x54          |
| SDA | VSS | 1010 101X  | 0xAA / 0xAB       | 0x55          |
| SDA | SCL | 1010 110X  | 0xAC / 0xAD       | 0x56          |
| SDA | SDA | 1010 111X  | 0xAE / 0xAF       | 0x57          |

**Note:** X = logic 0 for write cycle; X = logic 1 for read cycle.
*/


// A:VDD
// B:GND
// C:SCL
// D:SDA
#define SC16IS7XX_ADDRESS_AA (0X90)  ///< Both A and B tied to VDD
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

// General Registers

// clang-format off
/*
| Bit(s) | Name     | Function                                                      |
|--------|----------|---------------------------------------------------------------|
| 7      | -        | Not used                                                      |
| 6:3    | A[3:0]   | UART’s internal register select                               |
| 2:1    | CH1, CH0 | Channel select [00 = A, 01 = B, 10 = reserved, 11 = reserved] |
| 0      | -        | Not used                                                      |
*/
// clang-format on

// NOTE: Because the address is shifted left by 3 bits and the channel is
// shifted left by 1 bit, the register address to use in the Wire functions is
// effectively (reg_addr << 3 | channel << 1) when writing or reading registers
// for a specific channel on the SC16IS752.

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

// Interrupt Enable Register Bits

// NOTE:
// - Enabling any of the MODEM, RLS, THR, or RHR interrupts put the module
// into "Interrupt Mode Operation".  In this mode, the host is informted of the
// status of the rx and tx via the interrupt signal (the IRQ pin) and should
// check the IIR register for the source of the interrupt.
// - If you do NOT enable any of the MODEM, RLS, THR, or RHR interrupts, then
// the module is in "Polling Mode Operation" and the host should check the LSR
// and MSR registers to check the status of the rx and tx.
// - Enabling the CTS, RTS, or XOFF interrupts does NOT put the module into
// "Interrupt Mode Operation". The host should check the IIR register for the
// source of the interrupt if any of these are enabled, but they can be enabled
// in either mode of operation.

// Taken from datasheet table 11
#define SC16IS7XX_IER_CTS (0X07)    ///< CTS Interrupt Enable
#define SC16IS7XX_IER_RTS (0X06)    ///< RTS Interrupt Enable
#define SC16IS7XX_IER_XOFF (0X05)   ///< XOFF Interrupt
#define SC16IS7XX_IER_SLEEP (0X04)  ///< Sleep Mode Enable (NOT AN INTERRUPT)
#define SC16IS7XX_IER_MODEM (0X03)  ///< Modem Interrupt
// NOTE: The modem interrupt is triggered by changes in the RI, CD/DTR, or DSR
// pins **if** the I/O pins are configured as modem pins.
#define SC16IS7XX_IER_RLS (0X02)  ///< Receiver Line Status Interrupt
#define SC16IS7XX_IER_THR (0X01)  ///< Transmit Holding Register Interrupt
#define SC16IS7XX_IER_RHR (0X00)  ///< Receive Holding Register Interrupt

// Interrupt Identification Register Values
// Taken from datasheet table 14
// Listed from increasing to decreasing priority
#define SC16IS7XX_INT_LINE (0X06)     ///< Line Interrupt 0b00000110
#define SC16IS7XX_INT_TIMEOUT (0X0c)  ///< Timeout Interrupt 0b00001100
#define SC16IS7XX_INT_RHR \
    (0X04)  ///< Receive Holding Register Interrupt 0b00000100
#define SC16IS7XX_INT_THR \
    (0X02)  ///< Transmit Holding Register Interrupt 0b00000010
#define SC16IS7XX_INT_MODEM (0X00)   ///< Modem Interrupt 0b00000000
#define SC16IS7XX_INT_GPIO (0x30)    ///< GPIO Interrupt 0b00110000
#define SC16IS7XX_INT_XOFF (0X10)    ///< XOFF Interrupt 0b00010000
#define SC16IS7XX_INT_CTSRTS (0X20)  ///< CTS/RTS Interrupt 0b00100000

// Application Related
#define SC16IS7XX_PROTOCOL_I2C (0)
#define SC16IS7XX_PROTOCOL_SPI (1)
#define SC16IS7XX_SPIREG ADDRBIT8_HIGH_TOREAD  ///< SPI register type

// Default values
#define SC16IS7XX_DEFAULT_ADDRESS \
    (SC16IS7XX_ADDRESS_AA)                     ///< The default I2C address
#define SC16IS7XX_DEFAULT_SPIFREQ (1000000UL)  ///< The default SPI Clock speed
#define SC16IS7XX_DEFAULT_XTAL_FREQ \
    (14745600UL)  ///< The default frequency of the crystal in hertz

/**
 * @brief Base driver for SC16IS7XX family devices using I2C or SPI.
 */
class SC16IS7XX {
 private:
    uint32_t crystal_frequency = SC16IS7XX_DEFAULT_XTAL_FREQ;

    bool _init(void);

 protected:
    Adafruit_I2CDevice* i2c_dev = nullptr;  ///< Pointer to I2C bus interface
    Adafruit_SPIDevice* spi_dev = nullptr;  ///< Pointer to SPI bus interface

    virtual void resetDevice();
    virtual bool ping();

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

    // configuration
    void     setCrystalFrequency(uint32_t frequency);
    uint32_t getCrystalFrequency();

    // gpio
    virtual void    pinMode(uint8_t pin, uint8_t mode);
    virtual void    digitalWrite(uint8_t pin, uint8_t state);
    virtual uint8_t digitalRead(uint8_t pin);

    void enableSleepMode(bool enabled);
    bool isSleepEnabled();

    void enableCTSInterrupt(bool enabled);
    void enableRTSInterrupt(bool enabled);
    void enableXOFFInterrupt(bool enabled);
    void enableRLSInterrupt(bool enabled);
    void enableTHRInterrupt(bool enabled);
    void enableRHRInterrupt(bool enabled);

    void    setPinInterrupt(uint8_t pin, bool enabled);
    uint8_t getPinInterrupt(uint8_t pin);
    uint8_t isr();
    void    setPortState(uint8_t state);
    uint8_t getPortState();
    void    setPortMode(uint8_t mode);
    uint8_t getPortMode();

    void setGPIOLatch(bool enabled);
};

#endif

// cSpell:words SPIFREQ SPIREG MISO MOSI
