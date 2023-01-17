
//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   ds3231.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>
#include "ds3231.h"

DS3231::DS3231(i2c_inst_t *i2c,
               uint8_t sda,
               uint8_t scl,
               uint8_t address) : _i2c(i2c),
                                  _sda(sda),
                                  _scl(scl),
                                  _address(address)
{
}

bool DS3231::init(bool initI2c)
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

void DS3231::writeIO(uint8_t addr, const uint8_t data)
{
    uint8_t dta[2];
    dta[0] = addr;
    dta[1] = data;
    i2c_write_blocking(_i2c, _address, dta, 2, false);
}

uint8_t DS3231::readIO(uint8_t addr)
{
    uint8_t data = 0;
    i2c_write_blocking(_i2c, _address, &addr, 1, true);
    i2c_read_blocking(_i2c, _address, &data, 1, false);
    return data;
}

float DS3231::temperature()
{
    auto val = readIO(_tempmsb);
    float temp = float((val & 0x80) ? -((uint8_t)~val + 1) : val);
    temp += float(readIO(_templsb) >> 6) * 0.25f;
    return temp;
}

uint8_t DS3231::num2Bcd(uint8_t val)
{
    return ((val / 10 * 16) + (val % 10));
}

uint8_t DS3231::bcd2Num(uint8_t val)
{
    return ((val / 16 * 10) + (val % 16));
}

int16_t DS3231::year()
{
    return 2000 +  bcd2Num(readIO(_Year));
}

int8_t DS3231::month()
{
    return bcd2Num(readIO(_Month) & 0b01111111);
}

int8_t DS3231::day()
{
    return bcd2Num(readIO(_Date));
}

int8_t DS3231::hour()
{
    int8_t rc = -1;
    auto val = readIO(_Hours);

    if (val & 0b01000000)
    {
        // AM/PM mode
        _pm = val & 0b00100000;
        rc = bcd2Num(val & 0b00011111);
    }
    else
    {
        rc = bcd2Num(val & 0b00111111);
    }

    return rc;
}

int8_t DS3231::minute()
{
    return bcd2Num(readIO(_Minutes));
}

int8_t DS3231::second()
{
    return bcd2Num(readIO(_Seconds));
}

void DS3231::setYear(int16_t val)
{
    writeIO(_Year, num2Bcd((val > 99) ? (val - 2000) : val));
}

void DS3231::setMonth(int8_t val)
{
    writeIO(_Month, num2Bcd(val));
}

void DS3231::setDay(int8_t val)
{
    writeIO(_Date, num2Bcd(val));
}

void DS3231::setWeekDay(int8_t val)
{
     //DS3231 - 1 equals Sunday, then 2 equals Monday, and so on
    writeIO(_Day, num2Bcd(val + 1));
}

int8_t DS3231::weekDay()
{
    //DS3231 - 1 equals Sunday, then 2 equals Monday, and so on
    return bcd2Num(readIO(_Day)) - 1;
}

void DS3231::setHour(int8_t val)
{
    auto pmstat = readIO(_Hours);
    if (pmstat & 0b01000000)
    {
        // AM/PM

        auto tmp = val;
        if (tmp > 11)
        {
            tmp = tmp - 12;
        }
        if (tmp == 0)
        {
            tmp = 12;
        }
        writeIO(_Hours, num2Bcd(tmp) | ((pmstat & 0b01000000) << 5) | 0b01000000);
    }
    else
    {
        writeIO(_Hours, num2Bcd(val) & 0b10111111);
    }
}

void DS3231::setMinute(int8_t val)
{
    writeIO(_Minutes, num2Bcd(val));
}

void DS3231::setSecond(int8_t val)
{
    writeIO(_Seconds, num2Bcd(val));
}

uint8_t DS3231::control(bool first)
{
    return readIO(first ? _CTRL1 : _CTRL2);
}

void DS3231::setControl(bool first, uint8_t val)
{
    writeIO(first ? _CTRL1 : _CTRL2, val);
}

void DS3231::setOSF()
{
    auto r = control(false);
    setControl(false, r & 0b01111111);
}

bool DS3231::isOSF()
{
    auto r = control(false);
    return !(r & 0b10000000);
}

datetime_t DS3231::timeDate()
{
    datetime_t t = {
        .year = year(),
        .month = month(),
        .day = day(),
        .dotw = weekDay(), // 0 is Sunday
        .hour = hour(),
        .min = minute(),
        .sec = second()};
    return t;
}

void DS3231::setDateTime(datetime_t &dt)
{
    setHour(dt.hour);
    setMinute(dt.min);
    setSecond(dt.sec);
    setDay(dt.day);
    setMonth(dt.month);
    setYear(dt.year);
    setWeekDay(dt.dotw);
    setOSF();
}
