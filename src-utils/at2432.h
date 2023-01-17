//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   at2432.h
/// @author Petr Vanek

#pragma once

#include <inttypes.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"

/**
 * @brief  Serial EEPROM, 32K (4096 x 8) basic operation
 *
 */
class AT2432
{

public:

    /**
     * @brief Construct a new AT2432 object
     * 
     * @param i2c - I2C interface
     * @param sda  - SDA pin
     * @param scl  - SCL pin
     * @param address  - address - default fixed to 0x68 (only for debug purposes) e.g. 0x57
     */
    AT2432(i2c_inst_t *i2c,
               uint8_t sda,
               uint8_t scl,
               uint8_t address = 0x57);
  
    /**
     * @brief Initializes the object and sets the I2C if needed
     *
     * @param initI2c true - performs the pin and I2C setup
     * @return true - successful initialization
     * @return false - something failed
     */
    bool init(bool initI2c);

    /**
     * @brief  writes the octet from memory space
     * 
     * @param addr memory location address
     * @param data  - octet value
     */
    void writeIO(uint16_t addr, const uint8_t data);
    
    /**
     * @brief reads the octet to memory
     * 
     * @param addr memory location address 
     * @return uint8_t  - octet value
     */
    uint8_t readIO(uint16_t addr);

    /**
     * @brief clears the checksum 
     * 
     */
    void clearCheckSum() {
        _chksum = 0;
    }

    /**
     * @brief Get the checksum value from read/write operations
     * 
     * @return uint8_t 
     */
    uint8_t checkSum() {
        return _chksum;
    }


private:
    i2c_inst_t *_i2c{nullptr};
    uint8_t _sda{0};
    uint8_t _scl{0};
    uint8_t _address{0};
    uint8_t _chksum {0};

};
