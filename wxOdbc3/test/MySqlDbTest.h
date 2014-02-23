/*!
 * \file DbTest.h
 * \author Elias Gerber <eg@zame.ch>
 * \date 22.09.2013
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef DBTEST_H
#define DBTEST_H

// Same component headers
// Other headers
#include "cppunit/extensions/HelperMacros.h"

// System headers

// Forward declarations
// --------------------
class wxDbConnectInf;
class wxDb;

// Structs
// -------


namespace MySql
{

// DbTest
// ------
class DbTestBase
{ 

public:
	DbTestBase()
		: m_pConnectInfMySql(NULL)
		, m_pDbMySql(NULL)
	{}

public:
	virtual void setUp(const std::wstring& dsn);
	virtual void tearDown();
	
	void testOpen();

protected:
	wxDbConnectInf* m_pConnectInfMySql;
	wxDb*			m_pDbMySql;

	std::wstring		m_dsn;
};

}

namespace MySql_3_51
{
	class DbTest
		: public CPPUNIT_NS::TestFixture
		, public MySql::DbTestBase
	{
		CPPUNIT_TEST_SUITE( DbTest );
		CPPUNIT_TEST( testOpen );
		CPPUNIT_TEST_SUITE_END();

	public:
		virtual void setUp();
		virtual void tearDown();

	};
	CPPUNIT_TEST_SUITE_REGISTRATION( DbTest );
}


namespace MySql_5_2
{
	class DbTest
		: public CPPUNIT_NS::TestFixture
		, public MySql::DbTestBase
	{
		CPPUNIT_TEST_SUITE( DbTest );
		CPPUNIT_TEST( testOpen );
		CPPUNIT_TEST_SUITE_END();

	public:
		virtual void setUp();
		virtual void tearDown();

	};
	CPPUNIT_TEST_SUITE_REGISTRATION( DbTest );
}

#endif // DBTEST_H
