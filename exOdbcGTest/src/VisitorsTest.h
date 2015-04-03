/*!
* \file VisitorsTest.h
* \author Elias Gerber <egerber@gmx.net>
* \date 31.03.2015
* \copyright wxWindows Library Licence, Version 3.1
*
* [Brief Header-file description]
*/

#pragma once
#ifndef EXCELTEST_H
#define EXCELTEST_H

// Same component headers
#include "exOdbcGTest.h"

// Other headers
#include "gtest/gtest.h"

// System headers

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
namespace exodbc
{
	namespace VisitorsTest
	{
		const SQLBIGINT BIG_INT_NULL = 0;
		const SQLBIGINT BIG_INT_MAX = 9223372036854775807;
		const SQLBIGINT BIG_INT_MIN = (-9223372036854775807 - 1);

		const SQLINTEGER I_NULL = 0;
		const SQLINTEGER I_MAX = 2147483647;
		const SQLINTEGER I_MIN = (-2147483647 - 1);

		const SQLSMALLINT SMALL_INT_NULL = 0;
		const SQLSMALLINT SMALL_INT_MAX = 32767;
		const SQLSMALLINT SMALL_INT_MIN = (-32767 - 1);

		BufferPtrVariant pBigIntNull = (SQLBIGINT*)&BIG_INT_NULL;
		BufferPtrVariant pBigIntMin = (SQLBIGINT*)&BIG_INT_MIN;
		BufferPtrVariant pBigIntMax = (SQLBIGINT*)&BIG_INT_MAX;

		BufferPtrVariant pIntNull = (SQLINTEGER*)&I_NULL;
		BufferPtrVariant pIntMin = (SQLINTEGER*)&I_MIN;
		BufferPtrVariant pIntMax = (SQLINTEGER*)&I_MAX;

		BufferPtrVariant pSmallIntNull = (SQLSMALLINT*)&SMALL_INT_NULL;
		BufferPtrVariant pSmallIntMin = (SQLSMALLINT*)&SMALL_INT_MIN;
		BufferPtrVariant pSmallIntMax = (SQLSMALLINT*)&SMALL_INT_MAX;
	};


	class BigIntVisitorTest : public ::testing::Test
	{

	protected:
		virtual void SetUp() {};
		virtual void TearDown() {};
	};


	class WStringVisitorTest : public ::testing::Test
	{
	protected:
		virtual void SetUp() {};
		virtual void TearDown() {};
	};


	class StringVisitorTest : public ::testing::Test
	{
	protected:
		virtual void SetUp() {};
		virtual void TearDown() {};
	};


	class DoubleVisitorTest : public ::testing::Test
	{
	protected:
		virtual void SetUp() {};
		virtual void TearDown() {};
	};


	class TimestampVisitorTest : public ::testing::Test
	{
	protected:
		virtual void SetUp() {};
		virtual void TearDown() {};
	};

} // namespace exodbc

#endif // EXCELTEST_H