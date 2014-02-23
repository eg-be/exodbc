/*!
 * \file Db2DbTest.h
 * \author Elias Gerber <eg@zame.ch>
 * \date 23.02.2014
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef DB2DBTEST_H
#define DB2DBTEST_H

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


namespace DB2
{

// DbTest
// ------
class DbTestBase
{ 

public:
	DbTestBase()
		: m_pConnectInfDb2(NULL)
		, m_pDbDb2(NULL)
	{}

public:
	virtual void setUp(const std::wstring& dsn);
	virtual void tearDown();
	
	void testOpen();

protected:
	wxDbConnectInf* m_pConnectInfDb2;
	wxDb*			m_pDbDb2;

	std::wstring		m_dsn;
};

}

namespace DB2_10_05
{
	class DbTest
		: public CPPUNIT_NS::TestFixture
		, public DB2::DbTestBase
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


#endif // DB2DBTEST_H
