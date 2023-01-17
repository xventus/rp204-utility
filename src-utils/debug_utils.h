//
// vim: ts=4 et
// Copyright (c) 2023 Petr Vanek, petr@fotoventus.cz
//
/// @file   debug_utils.h
/// @author Petr Vanek

#pragma once

#include "at2432.h"

/**
 * @brief a set of utilities for debugging and tuning the interface
 *
 */
class DebugUtils
{

public:
    /**
     * @brief display of EEPROM content via AT object
     *
     * @param at object instance AT2432
     * @param from initial address
     * @param num number of addresses to display
     */
    static void memdump(AT2432 &at, uint16_t from, uint8_t num)
    {
        printf("\n    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F\n");
        for (uint16_t addr = from; addr <= (from + num); addr++)
        {
            if (addr % 16 == 0)
            {
                printf("%02x: ", addr);
            }

            printf("%02X", at.readIO(addr));
            printf(addr % 16 == 15 ? "\n" : "  ");
        }
    }

    /**
     * @brief I2C device search
     *
     * @param i2c nstance i2c interface
     */
    static void i2cscan(i2c_inst_t *i2c)
    {
        printf("\n    0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

        for (int addr = 0; addr < (1 << 7); ++addr)
        {
            if (addr % 16 == 0)
            {
                printf("%02x: ", addr);
            }

            uint8_t data;
            printf(i2c_read_blocking(i2c, addr, &data, 1, false) < 0 ? "." : "*");
            printf(addr % 16 == 15 ? "\n" : "  ");
        }
    }

   

    /**
     * @brief Prints contnet of datetime_t
     * 
     * @param dt rp2040 Datetime structure
     */
    static void printDatetime(const datetime_t &dt) {
        const char* dow[7] = {"Sun\0", "Mon\0", "Tue\0", "Wed\0", "Thu\0", "Fri\0", "Sat\0"};
        printf("%02d:%02d:%02d  %02d/%02d/%04d [%s]\n",  dt.hour, dt.min, dt.sec, dt.day,  dt.month, dt.year, (dt.dotw<7) ? dow[dt.dotw] : "Err");
    }
};
