/*!
* \file VisitorsTests.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 31.03.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "VisitorsTest.h"

// Same component headers
// Other headers
#include "Visitors.h"

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
		EXPECT_EQ(L"2.6011982999999999e-06", s3);
	}


	TEST_F(WStringVisitorTest, FromTimestamp)
	{
		SQL_TIMESTAMP_STRUCT t0 = InitTimestamp(13, 55, 56, 123000000, 26, 01, 1983);
		wstring s1 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQL_TIMESTAMP_STRUCT*)&t0));
		EXPECT_EQ(L"1983-01-26T13:55:56.123000000", s1);

		SQL_TIMESTAMP_STRUCT t1 = InitTimestamp(13, 55, 03, 10000000, 26, 01, 1983);
		wstring s2 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQL_TIMESTAMP_STRUCT*)&t1));
		EXPECT_EQ(L"1983-01-26T13:55:03.010000000", s2);
	}


	TEST_F(WStringVisitorTest, FromTime)
	{
		SQL_TIME_STRUCT t0 = InitTime(13, 05, 1);
		wstring s1 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQL_TIME_STRUCT*)&t0));
		EXPECT_EQ(L"13:05:01", s1);
	}


#if HAVE_MSODBCSQL_H
	TEST_F(WStringVisitorTest, FromTime2)
	{
		SQL_SS_TIME2_STRUCT t2 = InitTime2(13, 05, 1, 123456000);
		wstring s1 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQL_SS_TIME2_STRUCT*)&t2));
		EXPECT_EQ(L"13:05:01.123456000", s1);

		SQL_SS_TIME2_STRUCT t22 = InitTime2(13, 05, 1, 10000000);
		wstring s22 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQL_SS_TIME2_STRUCT*)&t22));
		EXPECT_EQ(L"13:05:01.010000000", s22);
	}
#endif


	TEST_F(WStringVisitorTest, FromDate)
	{
		SQL_DATE_STRUCT d0 = InitDate(26, 01, 1983);
		wstring s1 = boost::apply_visitor(WStringVisitor(), BufferPtrVariant((SQL_DATE_STRUCT*)&d0));
		EXPECT_EQ(L"1983-01-26", s1);

		TableOpenFlags fs = 4;
		if (fs & TOF_DO_NOT_QUERY_PRIMARY_KEYS)
		{
			int p = 3;
		}
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
		EXPECT_EQ("2.6011982999999999e-06", s3);
	}


	TEST_F(StringVisitorTest, FromTimestamp)
	{
		SQL_TIMESTAMP_STRUCT t0 = InitTimestamp(13, 55, 56, 123000000, 26, 01, 1983);
		string s1 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQL_TIMESTAMP_STRUCT*)&t0));
		EXPECT_EQ("1983-01-26T13:55:56.123000000", s1);

		SQL_TIMESTAMP_STRUCT t1 = InitTimestamp(13, 55, 7, 10000000, 26, 01, 1983);
		string s2 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQL_TIMESTAMP_STRUCT*)&t1));
		EXPECT_EQ("1983-01-26T13:55:07.010000000", s2);
	}


	TEST_F(StringVisitorTest, FromTime)
	{
		SQL_TIME_STRUCT t0 = InitTime(13, 05, 1);
		string s1 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQL_TIME_STRUCT*)&t0));
		EXPECT_EQ("13:05:01", s1);
	}


#if HAVE_MSODBCSQL_H
	TEST_F(StringVisitorTest, FromTime2)
	{
		SQL_SS_TIME2_STRUCT t2 = InitTime2(13, 05, 1, 123456000);
		string s1 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQL_SS_TIME2_STRUCT*)&t2));
		EXPECT_EQ("13:05:01.123456000", s1);

		SQL_SS_TIME2_STRUCT t22 = InitTime2(13, 05, 1, 10000000);
		string s22 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQL_SS_TIME2_STRUCT*)&t22));
		EXPECT_EQ("13:05:01.010000000", s22);
	}
#endif


	TEST_F(StringVisitorTest, FromDate)
	{
		SQL_DATE_STRUCT d0 = InitDate(26, 01, 1983);
		string s1 = boost::apply_visitor(StringVisitor(), BufferPtrVariant((SQL_DATE_STRUCT*)&d0));
		EXPECT_EQ("1983-01-26", s1);
	}


	// DoubleVisitor
	// -------------
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


	// RealVisitor
	// -----------
	TEST_F(RealVisitorTest, FromSmallInt)
	{
		SQLREAL r0 = boost::apply_visitor(RealVisitor(), pSmallIntNull);
		SQLREAL rMin = boost::apply_visitor(RealVisitor(), pSmallIntMin);
		SQLREAL rMax = boost::apply_visitor(RealVisitor(), pSmallIntMax);

		EXPECT_EQ((SQLREAL)SMALL_INT_NULL, r0);
		EXPECT_EQ((SQLREAL)SMALL_INT_MIN, rMin);
		EXPECT_EQ((SQLREAL)SMALL_INT_MAX, rMax);
	}


	TEST_F(RealVisitorTest, FromReal)
	{
		SQLREAL rNull = 0.0;
		SQLREAL rPosRef = 3.1415f;
		SQLREAL rNegRef = -3.1415f;
		SQLREAL r0 = boost::apply_visitor(RealVisitor(), BufferPtrVariant((SQLREAL*)&rNull));
		SQLREAL rPos = boost::apply_visitor(RealVisitor(), BufferPtrVariant((SQLREAL*)&rPosRef));
		SQLREAL rNeg = boost::apply_visitor(RealVisitor(), BufferPtrVariant((SQLREAL*)&rNegRef));

		EXPECT_EQ(rNull, r0);
		EXPECT_EQ(rPosRef, rPos);
		EXPECT_EQ(rNegRef, rNeg);
	}



	// TimestampVisitor
	// ----------------
	TEST_F(TimestampVisitorTest, FromDate)
	{
		SQL_DATE_STRUCT d0 = InitDate(26, 01, 1983);
		SQL_TIMESTAMP_STRUCT ts1 = boost::apply_visitor(TimestampVisitor(), BufferPtrVariant((SQL_DATE_STRUCT*)&d0));
		EXPECT_EQ(26, ts1.day);
		EXPECT_EQ(1, ts1.month);
		EXPECT_EQ(1983, ts1.year);
		EXPECT_EQ(0, ts1.hour);
		EXPECT_EQ(0, ts1.minute);
		EXPECT_EQ(0, ts1.second);
		EXPECT_EQ(0, ts1.fraction);
	}


	TEST_F(TimestampVisitorTest, FromTime)
	{
		SQL_TIME_STRUCT t0 = InitTime(13, 05, 1);
		SQL_TIMESTAMP_STRUCT ts1 = boost::apply_visitor(TimestampVisitor(), BufferPtrVariant((SQL_TIME_STRUCT*)&t0));
		EXPECT_EQ(0, ts1.day);
		EXPECT_EQ(0, ts1.month);
		EXPECT_EQ(0, ts1.year);
		EXPECT_EQ(13, ts1.hour);
		EXPECT_EQ(5, ts1.minute);
		EXPECT_EQ(1, ts1.second);
		EXPECT_EQ(0, ts1.fraction);
	}


#if HAVE_MSODBCSQL_H
	TEST_F(TimestampVisitorTest, FromTime2)
	{
		SQL_SS_TIME2_STRUCT t0 = InitTime2(13, 05, 1, 123456000);
		SQL_TIMESTAMP_STRUCT ts1 = boost::apply_visitor(TimestampVisitor(), BufferPtrVariant((SQL_SS_TIME2_STRUCT*)&t0));
		EXPECT_EQ(0, ts1.day);
		EXPECT_EQ(0, ts1.month);
		EXPECT_EQ(0, ts1.year);
		EXPECT_EQ(13, ts1.hour);
		EXPECT_EQ(5, ts1.minute);
		EXPECT_EQ(1, ts1.second);
		EXPECT_EQ(123456000, ts1.fraction);
	}
#endif


	TEST_F(TimestampVisitorTest, FromTimestamp)
	{
		SQL_TIMESTAMP_STRUCT t0 = InitTimestamp(13, 55, 56, 123000000, 26, 01, 1983);
		SQL_TIMESTAMP_STRUCT ts1 = boost::apply_visitor(TimestampVisitor(), BufferPtrVariant((SQL_TIMESTAMP_STRUCT*)&t0));
		EXPECT_EQ(26, ts1.day);
		EXPECT_EQ(1, ts1.month);
		EXPECT_EQ(1983, ts1.year);
		EXPECT_EQ(13, ts1.hour);
		EXPECT_EQ(55, ts1.minute);
		EXPECT_EQ(56, ts1.second);
		EXPECT_EQ(123000000, ts1.fraction);
	}


	// NumericVisitor
	// --------------
	TEST_F(NumericVisitorTest, FromNumeric)
	{
		SQLCHAR nVal[16];
		SQL_NUMERIC_STRUCT numRef = InitNumeric(18, 10, 1, nVal);
		SQL_NUMERIC_STRUCT n = boost::apply_visitor(NumericVisitor(), BufferPtrVariant((SQL_NUMERIC_STRUCT*)&numRef));
		EXPECT_EQ(0, memcmp(&numRef, &n, sizeof(SQL_NUMERIC_STRUCT)));
	}


	TEST_F(CharPtrVisitorTest, FromAny)
	{
		const SQLCHAR* bMin = boost::apply_visitor(CharPtrVisitor(), BufferPtrVariant((SQLBIGINT*)&BIG_INT_MIN));
		EXPECT_EQ(0, memcmp(bMin, &BIG_INT_MIN, sizeof(SQLBIGINT)));

		const SQLCHAR* iMin = boost::apply_visitor(CharPtrVisitor(), BufferPtrVariant((SQLINTEGER*)&I_MIN));
		EXPECT_EQ(0, memcmp(iMin, &I_MIN, sizeof(SQLINTEGER)));

		const SQLCHAR* sMin = boost::apply_visitor(CharPtrVisitor(), BufferPtrVariant((SQLSMALLINT*)&SMALL_INT_MIN));
		EXPECT_EQ(0, memcmp(sMin, &SMALL_INT_MIN, sizeof(SQLSMALLINT)));

		std::string s("Hello world");
		const SQLCHAR* schar = boost::apply_visitor(CharPtrVisitor(), BufferPtrVariant((SQLCHAR*)s.c_str()));
		EXPECT_EQ(0, memcmp(schar, s.c_str(), s.length() + 1));

		std::wstring ws(L"Hello world");
		const SQLCHAR* swchar = boost::apply_visitor(CharPtrVisitor(), BufferPtrVariant((SQLWCHAR*)ws.c_str()));
		EXPECT_EQ(0, memcmp(swchar, ws.c_str(), (ws.length() + 1) * sizeof(SQLWCHAR)));

		double dRef = 3.14159265359;
		const SQLCHAR* d = boost::apply_visitor(CharPtrVisitor(), BufferPtrVariant((SQLDOUBLE*)&dRef));
		EXPECT_EQ(0, memcmp(&dRef, d, sizeof(SQLDOUBLE)));

		SQL_TIMESTAMP_STRUCT tsRef = InitTimestamp(13, 55, 56, 123000000, 26, 01, 1983);
		const SQLCHAR* ts = boost::apply_visitor(CharPtrVisitor(), BufferPtrVariant((SQL_TIMESTAMP_STRUCT*)&tsRef));
		EXPECT_EQ(0, memcmp(&tsRef, ts, sizeof(SQL_TIMESTAMP_STRUCT)));

		SQL_TIME_STRUCT tRef = InitTime(13, 05, 1);
		const SQLCHAR* t = boost::apply_visitor(CharPtrVisitor(), BufferPtrVariant((SQL_TIME_STRUCT*)&tRef));
		EXPECT_EQ(0, memcmp(&tRef, t, sizeof(SQL_TIME_STRUCT)));

		SQL_DATE_STRUCT dateRef = InitDate(26, 01, 1983);
		const SQLCHAR* date = boost::apply_visitor(CharPtrVisitor(), BufferPtrVariant((SQL_DATE_STRUCT*)&dateRef));
		EXPECT_EQ(0, memcmp(&dateRef, date, sizeof(SQL_DATE_STRUCT)));

		SQLCHAR nVal[16];
		SQL_NUMERIC_STRUCT numRef = InitNumeric(18, 10, 1, nVal);
		const SQLCHAR* n = boost::apply_visitor(CharPtrVisitor(), BufferPtrVariant((SQL_NUMERIC_STRUCT*)&numRef));
		EXPECT_EQ(0, memcmp(&numRef, n, sizeof(SQL_NUMERIC_STRUCT)));

#if HAVE_MSODBCSQL_H
		SQL_SS_TIME2_STRUCT t2Ref = InitTime2(13, 05, 1, 123456000);
		const SQLCHAR* t2 = boost::apply_visitor(CharPtrVisitor(), BufferPtrVariant((SQL_SS_TIME2_STRUCT*)&t2Ref));
		EXPECT_EQ(0, memcmp(&t2Ref, t2, sizeof(SQL_SS_TIME2_STRUCT)));
#endif

	}

	// Interfaces
	// ----------

} //namespace exodbc
