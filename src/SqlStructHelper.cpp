﻿/*!
* \file SqlStructHelper.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.07.2014
* \brief Source file for SqlStructHelper
* \copyright GNU Lesser General Public License Version 3
*/ 

// Own header
#include "SqlStructHelper.h"

// Same component headers
// Other headers
#include <chrono>

// Debug
#include "DebugNew.h"

// Static consts
// -------------
namespace exodbc
{

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------
	SQL_TIME_STRUCT SqlStructHelper::InitTime(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second) noexcept
	{
		SQL_TIME_STRUCT time;
		time.hour = hour;
		time.minute = minute;
		time.second = second;

		return time;
	}


	SQL_DATE_STRUCT SqlStructHelper::InitDate(SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year) noexcept
	{
		SQL_DATE_STRUCT date;
		date.year = year;
		date.month = month;
		date.day = day;

		return date;
	}


	SQL_TIMESTAMP_STRUCT SqlStructHelper::InitTimestamp(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second, SQLUINTEGER fraction, SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year) noexcept
	{
		SQL_TIMESTAMP_STRUCT timestamp;
		timestamp.hour = hour;
		timestamp.minute = minute;
		timestamp.second = second;
		timestamp.fraction = fraction;
		timestamp.day = day;
		timestamp.month = month;
		timestamp.year = year;

		return timestamp;
	}


	SQL_TIMESTAMP_STRUCT SqlStructHelper::InitUtcTimestamp()
	{
		auto now = std::chrono::system_clock::now();
		auto tt = std::chrono::system_clock::to_time_t(now);
		tm utc_tm = *gmtime(&tt);

		SQL_TIMESTAMP_STRUCT ts;
		ts.hour = utc_tm.tm_hour;
		ts.minute = utc_tm.tm_min;
		ts.second = utc_tm.tm_sec;
		ts.fraction = 0;

		ts.year = 1900 + utc_tm.tm_year;
		ts.month = utc_tm.tm_mon;
		ts.day = utc_tm.tm_mday;

		return ts;
	}


	bool SqlStructHelper::IsTimeEqual(const SQL_TIME_STRUCT& t1, const SQL_TIME_STRUCT& t2) noexcept
	{
		return t1.hour == t2.hour
			&& t1.minute == t2.minute
			&& t1.second == t2.second;
	}


	bool SqlStructHelper::IsDateEqual(const SQL_DATE_STRUCT& d1, const SQL_DATE_STRUCT& d2) noexcept
	{
		return d1.day == d2.day
			&& d1.month == d2.month
			&& d1.year == d2.year;
	}


	bool SqlStructHelper::IsTimestampEqual(const SQL_TIMESTAMP_STRUCT& ts1, const SQL_TIMESTAMP_STRUCT& ts2) noexcept
	{
		return ts1.hour == ts2.hour
			&& ts1.minute == ts2.minute
			&& ts1.second == ts2.second
			&& ts1.fraction == ts2.fraction
			&& ts1.day == ts2.day
			&& ts1.month == ts2.month
			&& ts1.year == ts2.year;
	}


	SQL_NUMERIC_STRUCT SqlStructHelper::InitNumeric(SQLCHAR precision, SQLSCHAR scale, SQLCHAR sign, SQLCHAR val[SQL_MAX_NUMERIC_LEN]) noexcept
	{
		SQL_NUMERIC_STRUCT num;
		num.precision = precision;
		num.scale = scale;
		num.sign = sign;
		memcpy(num.val, val, SQL_MAX_NUMERIC_LEN);
		
		return num;
	}


	SQL_NUMERIC_STRUCT SqlStructHelper::InitNullNumeric() noexcept
	{
		SQL_NUMERIC_STRUCT num;
		memset(&num, 0, sizeof(num));
		return num;
	}


	std::string SqlStructHelper::TimestampToSqlString(const SQL_TIMESTAMP_STRUCT& ts, bool includeFraction /* = false */) noexcept
	{
		std::string s = boost::str(boost::format(u8"%04d-%02d-%02d %02d:%02d:%02d") % ts.year % ts.month % ts.day
			% ts.hour % ts.minute % ts.second);
		if (includeFraction)
		{
			s.append(u8".");
			s.append(boost::str(boost::format(u8"%f") % ((float)ts.fraction / 1000000000.0f)));
		}
		return s;
	}
}


