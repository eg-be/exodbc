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

// Classes
// -------
class DbConnectInfTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( DbConnectInfTest );
	CPPUNIT_TEST( testAllocHenv );
	CPPUNIT_TEST_SUITE_END();

public:
	void testAllocHenv();
};
CPPUNIT_TEST_SUITE_REGISTRATION( DbConnectInfTest );

class DbTest : public CPPUNIT_NS::TestFixture
{ 
	CPPUNIT_TEST_SUITE( DbTest );
	CPPUNIT_TEST( testOpen );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();
	void testOpen();

private:
	wxDbConnectInf* m_pConnectInfMySql;
	wxDb*			m_pDbMySql;
};
CPPUNIT_TEST_SUITE_REGISTRATION( DbTest );

#endif // DBTEST_H
