
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   at2432.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "at2432.h"

AT2432::AT2432(i2c_inst_t *i2c,
               uint8_t sda,
               uint8_t scl,
               uint8_t address) : _i2c(i2c),
                                  _sda(sda),
                                  _scl(scl),
                                  _address(address)
{
}

bool AT2432::init(bool initI2c)
{
    // only when required
    if (initI2c)
    {
        i2c_init(_i2c, 100 * 1000);
        gpio_set_function(_sda, GPIO_FUNC_I2C);
        gpio_set_function(_scl, GPIO_FUNC_I2C);
        gpio_pull_up(_sda);
        gpio_pull_up(_scl);
    }

    uint8_t data;
    return !(i2c_read_blocking(_i2c, _address, &data, 1, false) < 0);
}

void AT2432::writeIO(uint16_t addr, const uint8_t data)
{
    uint8_t dta[3];
    dta[0] = addr >> 8;     // MSB
    dta[1] = addr & 0xff ;  // LSB
    dta[2] = data;
    i2c_write_blocking(_i2c, _address, dta, 3, false);
    _chksum ^= data;
}

uint8_t AT2432::readIO(uint16_t addr)
{
    uint8_t data = 0;
    uint8_t dta[2];
    dta[0] = addr >> 8; // MSB
    dta[1] = addr & 0xff ; // LSB
    i2c_write_blocking(_i2c, _address, dta, 2, true);
    i2c_read_blocking(_i2c, _address, &data, 1, false);
    _chksum ^= data;
    return data;
}
