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

	void BigIntVisitorTest::SetUp()
	{
		m_pbINull = (SQLBIGINT*)&m_bINull;
		m_pbIMin = (SQLBIGINT*)&m_bIMin;
		m_pbIMax = (SQLBIGINT*)&m_bIMax;

		m_piNull = (SQLINTEGER*)&m_iNull;
		m_piMin = (SQLINTEGER*)&m_iMin;
		m_piMax = (SQLINTEGER*)&m_iMax;

		m_psINull = (SQLSMALLINT*)&m_sINull;
		m_psIMin = (SQLSMALLINT*)&m_sIMin;
		m_psIMax = (SQLSMALLINT*)&m_sIMax;
	}


	void BigIntVisitorTest::TearDown()
	{

	}


	// BigIntVisitor
	// -------------
	TEST_F(BigIntVisitorTest, FromBigInt)
	{
		SQLBIGINT b0 = boost::apply_visitor(BigintVisitor(), m_pbINull);
		SQLBIGINT bMin = boost::apply_visitor(BigintVisitor(), m_pbIMin);
		SQLBIGINT bMax = boost::apply_visitor(BigintVisitor(), m_pbIMax);

		EXPECT_EQ(m_bINull, b0);
		EXPECT_EQ(m_bIMin, bMin);
		EXPECT_EQ(m_bIMax, bMax);
	}


	TEST_F(BigIntVisitorTest, FromInt)
	{
		SQLBIGINT i0 = boost::apply_visitor(BigintVisitor(), m_piNull);
		SQLBIGINT iMin = boost::apply_visitor(BigintVisitor(), m_piMin);
		SQLBIGINT iMax = boost::apply_visitor(BigintVisitor(), m_piMax);

		EXPECT_EQ(m_iNull, i0);
		EXPECT_EQ(m_iMin, iMin);
		EXPECT_EQ(m_iMax, iMax);
	}


	TEST_F(BigIntVisitorTest, FromSmallInt)
	{
		SQLBIGINT s0 = boost::apply_visitor(BigintVisitor(), m_psINull);
		SQLBIGINT sMin = boost::apply_visitor(BigintVisitor(), m_psIMin);
		SQLBIGINT sMax = boost::apply_visitor(BigintVisitor(), m_psIMax);

		EXPECT_EQ(m_sINull, s0);
		EXPECT_EQ(m_sIMin, sMin);
		EXPECT_EQ(m_sIMax, sMax);
	}

	// Interfaces
	// ----------

} //namespace exodbc
