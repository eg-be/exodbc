﻿/*!
* \file   SqlStructHelperTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 09.08.2014
* \copyright GNU Lesser General Public License Version 3
* 
* [Brief CPP-file description]
*/ 

// Own header
#include "SqlStructHelperTest.h"

// Same component headers
#include "exOdbcTestHelpers.h"

// Other headers
#include "exodbc/SqlStatementCloser.h"
#include "exodbc/GetDataWrapper.h"
#include "exodbc/SetDescriptionFieldWrapper.h"
#include "exodbc/SqlStructHelper.h"

// Debug
#include "DebugNew.h"

using namespace exodbc;

namespace exodbctest
{


	// Static consts
	// -------------

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------


	// HelpersTest
	// =================
	TEST_F(SqlStructHelperTest, GetAllErrors)
	{
		// We except an assertion if not at least one handle is valid
		{
			LogLevelSetter ll(LogLevel::None);
			EXPECT_THROW(ErrorHelper::GetAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, SQL_NULL_HSTMT, SQL_NULL_HDESC), AssertionException);
		}
	}


	TEST_F(SqlStructHelperTest, InitTime)
	{
		SQL_TIME_STRUCT time = SqlStructHelper::InitTime(13, 14, 15);
		EXPECT_EQ(13, time.hour);
		EXPECT_EQ(14, time.minute);
		EXPECT_EQ(15, time.second);
	}


	TEST_F(SqlStructHelperTest, InitDate)
	{
		SQL_DATE_STRUCT date = SqlStructHelper::InitDate(26, 1, 1983);
		EXPECT_EQ(26, date.day);
		EXPECT_EQ(1, date.month);
		EXPECT_EQ(1983, date.year);
	}


	TEST_F(SqlStructHelperTest, InitTimestamp)
	{
		SQL_TIMESTAMP_STRUCT ts = SqlStructHelper::InitTimestamp(13, 14, 15, 123456789, 26, 1, 1983);
		EXPECT_EQ(13, ts.hour);
		EXPECT_EQ(14, ts.minute);
		EXPECT_EQ(15, ts.second);
		EXPECT_EQ(123456789, ts.fraction);
		EXPECT_EQ(26, ts.day);
		EXPECT_EQ(1, ts.month);
		EXPECT_EQ(1983, ts.year);
	}


	TEST_F(SqlStructHelperTest, InitNumeric)
	{
		SQLCHAR val[SQL_MAX_NUMERIC_LEN];
		memset(val, 1, SQL_MAX_NUMERIC_LEN);
		SQL_NUMERIC_STRUCT num = SqlStructHelper::InitNumeric(18, 10, 1, val);
		EXPECT_EQ(18, num.precision);
		EXPECT_EQ(10, num.scale);
		EXPECT_EQ((SQLCHAR)1, num.sign);
		EXPECT_TRUE(memcmp(val, num.val, SQL_MAX_NUMERIC_LEN) == 0);
	}


	TEST_F(SqlStructHelperTest, InitNullNumeric)
	{
		char nullMem[sizeof(SQL_NUMERIC_STRUCT)];
		memset(nullMem, 0, sizeof(SQL_NUMERIC_STRUCT));

		SQL_NUMERIC_STRUCT num = SqlStructHelper::InitNullNumeric();
		EXPECT_TRUE(memcmp(&num, &nullMem, sizeof(SQL_NUMERIC_STRUCT)) == 0);
	}


	TEST_F(SqlStructHelperTest, IsTimeEqual)
	{
		SQL_TIME_STRUCT t1 = SqlStructHelper::InitTime(13, 14, 15);
		SQL_TIME_STRUCT t2 = SqlStructHelper::InitTime(07, 14, 27);
		SQL_TIME_STRUCT t3 = SqlStructHelper::InitTime(13, 14, 15);
		EXPECT_TRUE(SqlStructHelper::IsTimeEqual(t1, t3));
		EXPECT_FALSE(SqlStructHelper::IsTimeEqual(t1, t2));
		EXPECT_FALSE(SqlStructHelper::IsTimeEqual(t3, t2));

	}


	TEST_F(SqlStructHelperTest, IsDateEqual)
	{
		SQL_DATE_STRUCT d1 = SqlStructHelper::InitDate(26, 01, 1983);
		SQL_DATE_STRUCT d2 = SqlStructHelper::InitDate(25, 04, 1983);
		SQL_DATE_STRUCT d3 = SqlStructHelper::InitDate(26, 01, 1983);
		EXPECT_TRUE(SqlStructHelper::IsDateEqual(d1, d3));
		EXPECT_FALSE(SqlStructHelper::IsDateEqual(d1, d2));
		EXPECT_FALSE(SqlStructHelper::IsDateEqual(d3, d2));
	}


	TEST_F(SqlStructHelperTest, IsTimestampEqual)
	{
		SQL_TIMESTAMP_STRUCT ts1 = SqlStructHelper::InitTimestamp(13, 14, 15, 123456789, 26, 1, 1983);
		SQL_TIMESTAMP_STRUCT ts2 = SqlStructHelper::InitTimestamp(13, 14, 15, 12345, 26, 1, 1983);
		SQL_TIMESTAMP_STRUCT ts3 = SqlStructHelper::InitTimestamp(13, 14, 15, 123456789, 26, 1, 1983);

		EXPECT_TRUE(SqlStructHelper::IsTimestampEqual(ts1, ts3));
		EXPECT_FALSE(SqlStructHelper::IsTimestampEqual(ts1, ts2));
		EXPECT_FALSE(SqlStructHelper::IsTimestampEqual(ts3, ts2));
	}
} // namespace exodbctest
