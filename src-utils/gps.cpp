
//
// vim: ts=4 et
// Copyright (c) 2022 Petr Vanek, petr@fotoventus.cz
//
/// @file   gps.cpp
/// @author Petr Vanek

#include <stdio.h>
#include <memory.h>
#include <math.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "gps.h"

GPS::GPS()
{
}

void GPS::init()
{
	invalidContnet();
}

int32_t GPS::str2Int(const char *str)
{
	int32_t rc = 1;

	if (*str == '-')
	{
		++str;
		rc = -1;
	}

	rc *= 100 * (int32_t)atol(str);
	while (isdigit(*str))
	{
		str++;
	}

	if (*str == '.' && isdigit(str[1]))
	{
		rc += 10 * (str[1] - '0');
		if (isdigit(str[2]))
			rc += str[2] - '0';
	}
	return rc;
}

double GPS::str2Degr(const char *str)
{
	auto num = atof(str);
	auto deg = floor(num / 100.0);
	auto min = num - deg * 100;
	return deg + min / 60.0;
}

void GPS::invalidContnet()
{
	_sentType = Sentence::UNKNOWN;
	_bufferPos = 0;
	memset(_buffer, '\0', sizeof(_buffer));
	_skipCheck = false;
	_time = 0;
	_date = 0;
	_speed = 0;
	_latitude = 0;
	_longitude = 0;
	_checksum = 0;
}

void GPS::resetContnet()
{
	_bufferPos = 0;
	memset(_buffer, '\0', sizeof(_buffer));
}

void GPS::finalizeSentence()
{

	switch (_sentType)
	{
	case Sentence::GPRMC:
		process();
		break;

	case Sentence::UNKNOWN:
		if (strcmp(_buffer, "GPRMC") == 0)
		{
			_sentType = Sentence::GPRMC;
			_bufferReq = BufferContent::Timestamp;
			resetContnet();
		}
		else
		{
			invalidContnet();
		}
		break;
	}
}

void GPS::process()
{
	switch (_bufferReq)
	{
	case BufferContent::Timestamp:
		_time = str2Int(_buffer);
		_bufferReq = BufferContent::PositionStatus;
		resetContnet();
		break;

	case BufferContent::PositionStatus:
		_validPosition = (_buffer[0] == 'A');
		_bufferReq = BufferContent::Latitude;
		resetContnet();
		break;

	case BufferContent::Latitude:
		_latitude = str2Degr(_buffer);
		_bufferReq = BufferContent::LatiDirection;
		resetContnet();
		break;

	case BufferContent::LatiDirection:
		if (_buffer[0] == 'S')
			_latitude *= -1;
		_bufferReq = BufferContent::Longitude;
		resetContnet();
		break;

	case BufferContent::Longitude:
		_longitude = str2Degr(_buffer);
		_bufferReq = BufferContent::LongDirection;
		resetContnet();
		break;

	case BufferContent::LongDirection:
		if (_buffer[0] == 'W')
			_longitude *= -1;
		_bufferReq = BufferContent::Speed;
		resetContnet();
		break;

	case BufferContent::Speed:

		_speed = 1.852 * str2Int(_buffer) / 100.0;

		_bufferReq = BufferContent::Track;
		resetContnet();
		break;

	case BufferContent::Track:
		_date = (uint32_t)atol(_buffer);
		_bufferReq = BufferContent::Date;
		resetContnet();
		break;

	case BufferContent::Date:
		_date = (uint32_t)atol(_buffer);
		_bufferReq = BufferContent::MagVariation;
		resetContnet();
		break;

	case BufferContent::MagVariation:
		_bufferReq = BufferContent::MagVarDirection;
		resetContnet();
		break;

	case BufferContent::MagVarDirection:
		_bufferReq = BufferContent::ModeInd;
		resetContnet();
		break;

	case BufferContent::ModeInd:
		_bufferReq = BufferContent::CheckSum;
		resetContnet();
		break;

	case BufferContent::CheckSum:

		if (strtol(_buffer, NULL, 16) == _checksum)
		{
			_fxtime = _time;
			_fxdate = _date;
			_fxspeed = _speed;
			_validDateTime = true;

			if (_validPosition)
			{
				_fxlatitude = _latitude;
				_fxlongitude = _longitude;
			}
			else
			{
				resetPosition();
			}
		}

		invalidContnet();
		break;
	}
}

void GPS::parse(uint8_t inp)
{

	if (_bufferPos >= sizeof(_buffer) - 1)
	{
		invalidContnet();
	}

	switch (inp)
	{

	// the beginning of the sentence
	case '$':
		_checksum = 0;
		_bufferPos = 0;
		memset(_buffer, '\0', sizeof(_buffer));
		break;

	case ',':
		_checksum ^= inp;
		finalizeSentence();
		break;

	case '\r':
	case '\n':
		finalizeSentence();
		break;

	case '*':
		finalizeSentence();
		_skipCheck = true;
		break;

	default:
		if (!_skipCheck)
			_checksum ^= inp;
		_buffer[_bufferPos++] = inp;

		break;
	}
}

datetime_t GPS::timeDate()
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