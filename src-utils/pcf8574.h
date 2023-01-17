//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   pcf8574.h
/// @author Petr Vanek

#pragma once

#include <inttypes.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"

/**
 * @brief
 *
 */
class PCF8574
{

public:
    const uint8_t _invalidKey = 0xff;

public:
    /**
     * @brief Construct a new PCF8574 object
     *
     * @param i2c - I2C interface
     * @param sda  - SDA pin
     * @param scl  - SCL pin
     * @param address  - address - default fixed to 0x68 (only for debug purposes) e.g. 0x38
     */
    PCF8574(i2c_inst_t *i2c,
            uint8_t sda,
            uint8_t scl,
            uint8_t address = 0x38);

    /**
     * @brief Initializes the object and sets the I2C if needed
     *
     * @param initI2c true - performs the pin and I2C setup
     * @return true - successful initialization
     * @return false - something failed
     */
    bool init(bool initI2c);

    /**
     * @brief writes octet to port
     *
     * @param data
     */
    void writeIO(const uint8_t data);

    /**
     * @brief reads octet from the port
     *
     * @return uint8_t
     */
    uint8_t readIO();

    /**
     * @brief writes bit value to the output pin
     *
     * @param bit 0 - 7 defined pin
     * @param val  - sets the logic value - true / false
     */
    void setBit(uint8_t bit, bool val);

    /**
     * @brief reads bit from the port
     *
     * @param bit - bit position 0 - 7
     * @return true
     * @return false
     */
    bool bit(uint8_t bit);

    /**
     * @brief read scan code for 4x4 keyboard matrix
     *
     * @return uint8_t _invalidKey or 0 - number
     */
    uint8_t getKey4x4();

    /**
     * @brief reads ASCII char from keyboard 4x4
     *
     * @return char '\0' - undefined or N/A
     */
    char getCharKey4x4();

private:
    const char *_map4x4{"123A456B789C*0#D\0"};
    i2c_inst_t *_i2c{nullptr};
    uint8_t _sda{0};
    uint8_t _scl{0};
    uint8_t _address{0};
    uint8_t _out{0xff}; // power on output 0xff
};
