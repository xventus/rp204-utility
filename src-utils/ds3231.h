//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   ds3231.h
/// @author Petr Vanek

#pragma once

#include <inttypes.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"

/**
 * @brief - encapsulates the basic real-time control clock
 *        - Basically it works with 0 - 24 hour cycle.
 */
class DS3231
{
public:
    /**
     * @brief addresses of the internal registers of RTC
     *
     */
    static constexpr uint8_t _Seconds = 0x00;
    static constexpr uint8_t _Minutes = 0x01;
    static constexpr uint8_t _Hours = 0x02;
    static constexpr uint8_t _Day = 0x03;
    static constexpr uint8_t _Date = 0x04;
    static constexpr uint8_t _Month = 0x05;
    static constexpr uint8_t _Year = 0x06;
    static constexpr uint8_t _AM1S = 0x07;
    static constexpr uint8_t _AM1M = 0x08;
    static constexpr uint8_t _AM1H = 0x09;
    static constexpr uint8_t _AM1D = 0x0A;

    static constexpr uint8_t _AM2M = 0x0B;
    static constexpr uint8_t _AM2H = 0x0C;
    static constexpr uint8_t _AM2DA = 0x0D;

    static constexpr uint8_t _CTRL1 = 0x0E;
    static constexpr uint8_t _CTRL2 = 0x0F;

    static constexpr uint8_t _tempmsb = 0x11;
    static constexpr uint8_t _templsb = 0x12;

public:
    /**
     * @brief Construct a new DS3231 object
     *
     * @param i2c - I2C interface
     * @param sda  - SDA pin
     * @param scl  - SCL pin
     * @param address  - address - default fixed to 0x68 (only for debug purposes)
     */
    explicit DS3231(i2c_inst_t *i2c,
                    uint8_t sda,
                    uint8_t scl,
                    uint8_t address = 0x68);

    /**
     * @brief Initializes the object and sets the I2C if needed
     *
     * @param initI2c true - performs the pin and I2C setup
     * @return true - successful initialization
     * @return false - something failed
     */
    bool init(bool initI2c = true);

    /**
     * @brief reads year
     *
     * @return int16_t - 0-99
     */
    int16_t year();

    /**
     * @brief reads month
     *
     * @return int8_t  1 - 12
     */
    int8_t month();

    /**
     * @brief reads day
     *
     * @return int8_t  1 - 31
     */
    int8_t day();

    /**
     * @brief reads hour
     *
     * @return int8_t  0 - 23 / 0 - 12
     */
    int8_t hour();

    /**
     * @brief reads minutes
     *
     * @return int8_t  0 - 59
     */
    int8_t minute();

    /**
     * @brief reads seconds
     *
     * @return int8_t  0 - 59
     */
    int8_t second();

    /**
     * @brief reads day of week
     * It operates in the range 0 - 6, compared to an internal storage of 1 - 7
     *
     * @return int8_t 0 - 6 
     */
    int8_t weekDay();

    /**
     * @brief Set the Year
     *
     * @param val 0-99 or 2000-2099
     */
    void setYear(int16_t val);

    /**
     * @brief Set the Month
     *
     * @param val
     */
    void setMonth(int8_t val);

    /**
     * @brief Set the Day
     *
     * @param val
     */
    void setDay(int8_t val);

    /**
     * @brief Set the Hour
     *
     * @param val
     */
    void setHour(int8_t val);

    /**
     * @brief Set the Minute
     *
     * @param val
     */
    void setMinute(int8_t val);

    /**
     * @brief Set the Second
     *
     * @param val
     */
    void setSecond(int8_t val);

    /**
     * @brief Set the Week Day
     *
     * It operates in the range 0 - 6, compared to an internal storage of 1 - 7
     * 
     * @param val  0 - 6
     */
    void setWeekDay(int8_t val);

    /**
     * @brief gets internal temperature
     *
     * @return float
     */
    float temperature();

    /**
     * @brief Set the Control register
     *
     * @param first first == true (0x0e) or second (0x0f)
     * @param val
     */
    void setControl(bool first, uint8_t val);

    /**
     * @brief reads Control register
     *
     * @param first first == true (0x0e) or second (0x0f)
     * @return uint8_t
     */
    uint8_t control(bool first);

    /**
     * @brief oscillator either is stopped or was stopped for some period and may be used to judge the validity of the timekeeping data
     *
     * @return true - if OK
     * @return false  - stopped
     */
    bool isOSF();

    /**
     * @brief sets OSF in running state
     *
     */
    void setOSF();

    /**
     * @brief writes the value to the register
     *
     * @param addr - register address
     * @param data - data to write
     */
    void writeIO(uint8_t addr, const uint8_t data);

    /**
     * @brief read value from the register
     *
     * @param addr - register address
     * @return uint8_t
     */
    uint8_t readIO(uint8_t addr);

    /**
     * @brief converts a number to a BCD representation
     *
     * @param val - number
     * @return uint8_t
     */
    uint8_t num2Bcd(uint8_t val);

    /**
     * @brief converts a BCD to a number representation
     *
     * @param val - BCD representation
     * @return uint8_t
     */
    uint8_t bcd2Num(uint8_t val);

    /**
     * @brief gets datetime into struct
     * 
     * @return datetime_t 
     */
    datetime_t timeDate();

    /**
     * @brief RTC with datetime struct
     * 
     * @param dt 
     */
    void setDateTime(datetime_t& dt);

private:
    i2c_inst_t *_i2c{nullptr};
    uint8_t _sda{0};
    uint8_t _scl{0};
    uint8_t _address{0};
    bool _pm{false};
};
