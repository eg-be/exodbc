/*!
* \file VisitorsTests.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 31.03.2015
* \copyright wxWindows Library Licence, Version 3.1
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "VisitorsTest.h"

// Same component headers
// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
using namespace std;

namespace exodbc
{

	using namespace VisitorsTest;

	// BigIntVisitor
	// -------------
	TEST_F(BigIntVisitorTest, FromBigInt)
	{
		SQLBIGINT b0 = boost::apply_visitor(BigintVisitor(), pBigIntNull);
		SQLBIGINT bMin = boost::apply_visitor(BigintVisitor(), pBigIntMin);
		SQLBIGINT bMax = boost::apply_visitor(BigintVisitor(), pBigIntMax);

		EXPECT_EQ(BIG_INT_NULL, b0);
		EXPECT_EQ(BIG_INT_MIN, bMin);
		EXPECT_EQ(BIG_INT_MAX, bMax);
	}


	TEST_F(BigIntVisitorTest, FromInt)
	{
		SQLBIGINT i0 = boost::apply_visitor(BigintVisitor(), pIntNull);
		SQLBIGINT iMin = boost::apply_visitor(BigintVisitor(), pIntMin);
		SQLBIGINT iMax = boost::apply_visitor(BigintVisitor(), pIntMax);

		EXPECT_EQ(I_NULL, i0);
		EXPECT_EQ(I_MIN, iMin);
		EXPECT_EQ(I_MAX, iMax);
	}


	TEST_F(BigIntVisitorTest, FromSmallInt)
	{
		SQLBIGINT s0 = boost::apply_visitor(BigintVisitor(), pSmallIntNull);
		SQLBIGINT sMin = boost::apply_visitor(BigintVisitor(), pSmallIntMin);
		SQLBIGINT sMax = boost::apply_visitor(BigintVisitor(), pSmallIntMax);

		EXPECT_EQ(SMALL_INT_NULL, s0);
		EXPECT_EQ(SMALL_INT_MIN, sMin);
		EXPECT_EQ(SMALL_INT_MAX, sMax);
	}


	// WStringVisitor
	// --------------
	TEST_F(WStringVisitorTest, FromBigInt)
	{
		wstring s0 = boost::apply_visitor(WStringVisitor(), pBigIntNull);
		wstring sMin = boost::apply_visitor(WStringVisitor(), pBigIntMin);
		wstring sMax = boost::apply_visitor(WStringVisitor(), pBigIntMax);

		EXPECT_EQ(L"0", s0);
		EXPECT_EQ(L"-9223372036854775808", sMin);
		EXPECT_EQ(L"9223372036854775807", sMax);
	}


	TEST_F(WStringVisitorTest, FromInt)
	{
		wstring s0 = boost::apply_visitor(WStringVisitor(), pIntNull);
		wstring sMin = boost::apply_visitor(WStringVisitor(), pIntMin);
		wstring sMax = boost::apply_visitor(WStringVisitor(), pIntMax);

		EXPECT_EQ(L"0", s0);
		EXPECT_EQ(L"-2147483648", sMin);
		EXPECT_EQ(L"2147483647", sMax);
	}


	TEST_F(WStringVisitorTest, FromSmallInt)
	{
		wstring s0 = boost::apply_visitor(WStringVisitor(), pSmallIntNull);
		wstring sMin = boost::apply_visitor(WStringVisitor(), pSmallIntMin);
		wstring sMax = boost::apply_visitor(WStringVisitor(), pSmallIntMax);

		EXPECT_EQ(L"0", s0);
		EXPECT_EQ(L"-32768", sMin);
		EXPECT_EQ(L"32767", sMax);
	}


	TEST_F(WStringVisitorTest, FromWString)
	{
		wstring ws1(L"Hello World");
		wstring ws2(L"הצה $ü");
		wstring s1 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQLWCHAR*)ws1.c_str()));
		wstring s2 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQLWCHAR*)ws2.c_str()));

		EXPECT_EQ(ws1, s1);
		EXPECT_EQ(ws2, s2);
	}


	TEST_F(WStringVisitorTest, FromDouble)
	{
		double d1 = 0.0;
		double d2 = 3.14159265359;
		double d3 = 2601.1983E-9;

		wstring s1 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQLDOUBLE*)&d1));
		wstring s2 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQLDOUBLE*)&d2));
		wstring s3 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQLDOUBLE*)&d3));

		EXPECT_EQ(L"0", s1);
		EXPECT_EQ(L"3.1415926535900001", s2);
		EXPECT_EQ(L"2.6011982999999999e-006", s3);
	}


	TEST_F(WStringVisitorTest, FromTimestamp)
	{
		SQL_TIMESTAMP_STRUCT t0 = InitTimestamp(13, 55, 56, 123000000, 26, 01, 1983);
		wstring s1 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQL_TIMESTAMP_STRUCT*)&t0));
		EXPECT_EQ(L"1983-01-26T13:55:56.123000000", s1);

		SQL_TIMESTAMP_STRUCT t1 = InitTimestamp(13, 55, 56, 10000000, 26, 01, 1983);
		wstring s2 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQL_TIMESTAMP_STRUCT*)&t1));
		EXPECT_EQ(L"1983-01-26T13:55:56.010000000", s2);
	}


	TEST_F(WStringVisitorTest, FromTime)
	{
		SQL_TIME_STRUCT t0 = InitTime(13, 05, 1);
		wstring s1 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQL_TIME_STRUCT*)&t0));
		EXPECT_EQ(L"13:05:01", s1);
	}


	TEST_F(WStringVisitorTest, FromDate)
	{
		SQL_DATE_STRUCT d0 = InitDate(26, 01, 1983);
		wstring s1 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQL_DATE_STRUCT*)&d0));
		EXPECT_EQ(L"1983-01-26", s1);
	}


	// StringVisitor
	// --------------
	TEST_F(StringVisitorTest, FromBigInt)
	{
		string s0 = boost::apply_visitor(StringVisitor(), pBigIntNull);
		string sMin = boost::apply_visitor(StringVisitor(), pBigIntMin);
		string sMax = boost::apply_visitor(StringVisitor(), pBigIntMax);

		EXPECT_EQ("0", s0);
		EXPECT_EQ("-9223372036854775808", sMin);
		EXPECT_EQ("9223372036854775807", sMax);
	}


	TEST_F(StringVisitorTest, FromInt)
	{
		string s0 = boost::apply_visitor(StringVisitor(), pIntNull);
		string sMin = boost::apply_visitor(StringVisitor(), pIntMin);
		string sMax = boost::apply_visitor(StringVisitor(), pIntMax);

		EXPECT_EQ("0", s0);
		EXPECT_EQ("-2147483648", sMin);
		EXPECT_EQ("2147483647", sMax);
	}


	TEST_F(StringVisitorTest, FromSmallInt)
	{
		string s0 = boost::apply_visitor(StringVisitor(), pSmallIntNull);
		string sMin = boost::apply_visitor(StringVisitor(), pSmallIntMin);
		string sMax = boost::apply_visitor(StringVisitor(), pSmallIntMax);

		EXPECT_EQ("0", s0);
		EXPECT_EQ("-32768", sMin);
		EXPECT_EQ("32767", sMax);
	}


	TEST_F(StringVisitorTest, FromString)
	{
		string cs1("Hello World");
		string cs2("הצה $ü");
		string s1 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQLCHAR*)cs1.c_str()));
		string s2 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQLCHAR*)cs2.c_str()));

		EXPECT_EQ(cs1, s1);
		EXPECT_EQ(cs2, s2);
	}


	TEST_F(StringVisitorTest, FromDouble)
	{
		double d1 = 0.0;
		double d2 = 3.14159265359;
		double d3 = 2601.1983E-9;

		string s1 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQLDOUBLE*)&d1));
		string s2 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQLDOUBLE*)&d2));
		string s3 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQLDOUBLE*)&d3));

		EXPECT_EQ("0", s1);
		EXPECT_EQ("3.1415926535900001", s2);
		EXPECT_EQ("2.6011982999999999e-006", s3);
	}


	TEST_F(StringVisitorTest, FromTimestamp)
	{
		SQL_TIMESTAMP_STRUCT t0 = InitTimestamp(13, 55, 56, 123000000, 26, 01, 1983);
		string s1 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQL_TIMESTAMP_STRUCT*)&t0));
		EXPECT_EQ("1983-01-26T13:55:56.123000000", s1);

		SQL_TIMESTAMP_STRUCT t1 = InitTimestamp(13, 55, 56, 10000000, 26, 01, 1983);
		string s2 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQL_TIMESTAMP_STRUCT*)&t1));
		EXPECT_EQ("1983-01-26T13:55:56.010000000", s2);
	}


	TEST_F(StringVisitorTest, FromTime)
	{
		SQL_TIME_STRUCT t0 = InitTime(13, 05, 1);
		string s1 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQL_TIME_STRUCT*)&t0));
		EXPECT_EQ("13:05:01", s1);
	}


	TEST_F(StringVisitorTest, FromDate)
	{
		SQL_DATE_STRUCT d0 = InitDate(26, 01, 1983);
		string s1 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQL_DATE_STRUCT*)&d0));
		EXPECT_EQ("1983-01-26", s1);
	}


	TEST_F(DoubleVisitorTest, FromSmallInt)
	{
		SQLDOUBLE d0 = boost::apply_visitor(DoubleVisitor(), pSmallIntNull);
		SQLDOUBLE dMin = boost::apply_visitor(DoubleVisitor(), pSmallIntMin);
		SQLDOUBLE dMax = boost::apply_visitor(DoubleVisitor(), pSmallIntMax);

		EXPECT_EQ((SQLDOUBLE)SMALL_INT_NULL, d0);
		EXPECT_EQ((SQLDOUBLE)SMALL_INT_MIN, dMin);
		EXPECT_EQ((SQLDOUBLE)SMALL_INT_MAX, dMax);
	}


	TEST_F(DoubleVisitorTest, FromInt)
	{
		SQLDOUBLE d0 = boost::apply_visitor(DoubleVisitor(), pIntNull);
		SQLDOUBLE dMin = boost::apply_visitor(DoubleVisitor(), pIntMin);
		SQLDOUBLE dMax = boost::apply_visitor(DoubleVisitor(), pIntMax);

		EXPECT_EQ((SQLDOUBLE)I_NULL, d0);
		EXPECT_EQ((SQLDOUBLE)INT_MIN, dMin);
		EXPECT_EQ((SQLDOUBLE)INT_MAX, dMax);
	}


	TEST_F(DoubleVisitorTest, FromDouble)
	{
		SQLDOUBLE dNull = 0.0;
		SQLDOUBLE dPosRef = 3.14159265359;
		SQLDOUBLE dNegRef = -3.14159265359;
		SQLDOUBLE d0 = boost::apply_visitor(DoubleVisitor(), BufferPtrVariant((SQLDOUBLE*)&dNull));
		SQLDOUBLE dPos = boost::apply_visitor(DoubleVisitor(), BufferPtrVariant((SQLDOUBLE*)&dPosRef));
		SQLDOUBLE dNeg = boost::apply_visitor(DoubleVisitor(), BufferPtrVariant((SQLDOUBLE*)&dNegRef));

		EXPECT_EQ(dNull, d0);
		EXPECT_EQ(dPosRef, dPos);
		EXPECT_EQ(dNegRef, dNeg);
	}


	// Interfaces
	// ----------

} //namespace exodbc
