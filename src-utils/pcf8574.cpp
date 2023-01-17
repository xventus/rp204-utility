
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   pcf8574.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "pcf8574.h"

PCF8574::PCF8574(i2c_inst_t *i2c,
                 uint8_t sda,
                 uint8_t scl,
                 uint8_t address) : _i2c(i2c),
                                    _sda(sda),
                                    _scl(scl),
                                    _address(address)
{
}

bool PCF8574::init(bool initI2c)
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

    // ready test
    uint8_t data = 0;
    auto rc = i2c_write_blocking(_i2c, _address, &data, 1, true);
    if (rc > 0)
    {
        rc = i2c_read_blocking(_i2c, _address, &data, 1, false);
    }

    return (rc > 0);
}

void PCF8574::writeIO(const uint8_t data)
{
    i2c_write_blocking(_i2c, _address, &data, 1, false);
    _out = data;
}

uint8_t PCF8574::readIO()
{
    uint8_t data{0};
    i2c_read_blocking(_i2c, _address, &data, 1, false);
    return data;
}

void PCF8574::setBit(uint8_t bit, bool val)
{
    if (bit < 8)
    {
        if (val)
            _out |= (1 << bit);
        else
            _out &= ~(1 << bit);
        writeIO(_out);
    }
}

bool PCF8574::bit(uint8_t bit)
{
    if (bit < 8)
    {
        return (readIO() & (1 << bit));
    }
    return false;
}

/*
          ff - No key

          70 07 - 1
          70 0b - 2
          70 0d - 3
          70 0e - A

          b0 07 - 4
          b0 0b - 5
          b0 0d - 6
          b0 0e - B

          d0 07 - 7
          d0 0b - 8
          d0 0d - 9
          d0 0e - C

          e0 07 - *
          e0 0b - 0
          e0 0d - #
          e0 0e - D
      */

uint8_t PCF8574::getKey4x4()
{
    uint8_t rc = 0xff;
    writeIO(0xf0);
    switch (readIO())
    {
    case 0x70:
        rc = 0;
        break;
    case 0xb0:
        rc = 4;
        break;
    case 0xd0:
        rc = 8;
        break;
    case 0xe0:
        rc = 12;
        break;
    default:
        rc = _invalidKey;
    }

    writeIO(0x0f);
    switch (readIO())
    {
    case 0x07:
        rc += 0;
        break;
    case 0x0b:
        rc += 1;
        break;
    case 0x0d:
        rc += 2;
        break;
    case 0x0e:
        rc += 3;
        break;
    default:
        rc = _invalidKey;
    }

    return rc;
}

char PCF8574::getCharKey4x4()
{
    char rc = '\0';
    auto k = getKey4x4();
    if (k < strlen(_map4x4))
    {
        rc = _map4x4[k];
    }
    return rc;
}
