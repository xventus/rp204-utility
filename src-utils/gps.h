//
// vim: ts=4 et
// Copyright (c) 2022 Petr Vanek, petr@fotoventus.cz
//
/// @file   gps.h
/// @author Petr Vanek

#pragma once

#include <inttypes.h>
#include "pico/time.h"
#include "time_utils.h"

/**
 * @brief simplified parsing of gps data, designed mainly for time data acquisition
 * 
 */
class GPS
{

    /*
     Note:
     $GPRMC,,V,,,,,,,,,,N*53
     $GPRMC,050251.00,V,,,,,,,201222,,,N*7F
     $GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70
              1    2    3    4    5     6    7    8      9     10  11 12

           1   220516     Time Stamp
           2   A          Position status - validity - A-ok, V-invalid
           3   5133.82    Latitude (DDmm.mm)
           4   N          Latitude direction: North/South
           5   00042.24   Longitude  (DDDmm.mm)
           6   W          Longitude direction - East/West
           7   173.8      Speed in knots (x.x)
           8   231.8      Track made good, degrees (x.x)
           9   130694     Date Stamp dd/mm/yy (xxxxxx)
           10  004.2      Magnetic variation, degrees (x.x)
           11  W          Magnetic variation direction -East/West
                          Positioning system mode indicator
           12  *70        checksum


     $GPRMC,hhmmss.ss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,x.x,a*hh
     1    = UTC of position fix
     2    = Data status (V=navigation receiver warning)
     3    = Latitude of fix
     4    = N or S
     5    = Longitude of fix
     6    = E or W
     7    = Speed over ground in knots
     8    = Track made good in degrees True
     9    = UT date
     10   = Magnetic variation degrees (Easterly var. subtracts from true course)
     11   = E or W
     12   = Checksum

     */

    /**
     * @brief supported sentences from GPS
     * 
     */
    enum class Sentence
    {
        UNKNOWN,
        GPRMC // Recommended minimum specific GPS/Transit data
    };

    /**
     * @brief Parsing sequences
     * 
     */
    enum class BufferContent
    {
        UNKNOWN,
        Timestamp,
        PositionStatus,
        Latitude,
        LatiDirection,
        Longitude,
        LongDirection,
        Speed,
        Track,
        Date,
        MagVariation,
        MagVarDirection,
        ModeInd,
        CheckSum
    };

public:
    GPS();

    /**
     * @brief set the default state
     * 
     */
    void init();

    /**
     * @brief processing of each received character
     * 
     * @param inp 
     */
    void parse(uint8_t inp);

    /**
     * @brief information on the validity of time & dates
     * 
     * @return true 
     * @return false 
     */
    bool isValidTime()
    {
        return _validDateTime;
    }

    /**
     * @brief test if positional data is available
     * 
     * @return true 
     * @return false 
     */
    bool isValidPosition()
    {
        return _validPosition;
    }

    /**
     * @brief return the decoded year
     * 
     * @return uint16_t 
     */
    int16_t year()
    {
        return (_fxdate % 100) + 2000;
    }

    /**
     * @brief return the decoded month
     * 
     * @return uint8_t 
     */
    int8_t month()
    {
        return (_fxdate / 100) % 100;
    }

    /**
     * @brief return the decoded day
     * 
     * @return uint8_t 
     */
    int8_t day()
    {
        return _fxdate / 10000;
    }

    int8_t weekDay() {
        return TimeUtils::dayOfWeek(year(), month(), day());
    }

    /**
     * @brief return the decoded hour
     * 
     * @return int8_t 
     */
    int8_t hour()
    {
        return _fxtime / 1000000;
    }

    /**
     * @brief return the decoded minute
     * 
     * @return int8_t 
     */
    int8_t minute()
    {
        return (_fxtime / 10000) % 100;
    }

    /**
     * @brief return the decoded second
     * 
     * @return int8_t 
     */
    int8_t second()
    {
        return (_fxtime / 100) % 100;
    }

    /**
     * @brief return the decoded centi second
     * 
     * @return int8_t 
     */
    int8_t centisecond()
    {
        return _fxtime % 100;
    }

    /**
     * @brief return the decoded longitude
     * 
     * @return double 
     */
    double longitude()
    {
        return _fxlongitude;
    }

    /**
     * @brief return the decoded latitude
     * 
     * @return double 
     */
    double latitude()
    {
        return _fxlatitude;
    }

    /**
     * @brief return the decoded speed
     * 
     * @return double 
     */
    double speed()
    {
        return _fxspeed;
    }

    /**
     * @brief sets the last time as invalid
     * 
     */
    void resetValidTime()
    {
        _validDateTime = false;
        _fxtime = 0;
        _fxdate = 0;
    }

    /**
     * @brief sets the last position as invalid
     * 
     */
    void resetPosition()
    {
        _validPosition = false;
        _fxlatitude = 0;
        _fxlongitude = 0;
        _fxspeed = 0;
    }

    static uint8_t dayOfWeek(int16_t year, int8_t month, int8_t day);
    
    datetime_t timeDate();

private:

    
    /**
     * @brief marks the processing as invalid and prepares to receive the character
     * 
     */
    void invalidContnet();
    
    /**
     * @brief clear the buffer
     * 
     */
    void resetContnet();

    /**
     * @brief find out the sentence type and send it to the process
     * 
     */
    void finalizeSentence();

    /**
     * @brief processes the specific part of the expression in the sentence that is stored in the buffer
     * 
     */
    void process();
    
    /**
     * @brief converts string to integer
     * 
     * @param term 
     * @return int32_t 
     */
    int32_t str2Int(const char *term);
    
    /**
     * @brief converts string to degree 
     * 
     * @param str 
     * @return double 
     */
    double str2Degr(const char *str);


private:
    
    bool _skipCheck{false};
    int32_t _time{0}, _fxtime{0};          // temporary time & fix  time
    uint32_t _date{0}, _fxdate{0};         // temporary date & fix date
    double _speed{0}, _fxspeed{0};         // temporary & fix speed in km/h
    double _latitude{0}, _fxlatitude{0};   // temporary & fix latitude
    double _longitude{0}, _fxlongitude{0}; // temporary & fix longitude
    bool _validPosition{false};            // valid GPS position flag
    bool _validDateTime{false};            // valid date time flag 
    uint8_t _checksum{0};                  // computed checksum 
    uint8_t _bufferPos{0};                 // position in the buffer 
    char _buffer[15];                      // operation buffer 
    BufferContent _bufferReq{BufferContent::UNKNOWN};   // buffer content type
    Sentence _sentType{Sentence::UNKNOWN};              // processing sentence type
};
