/*!
 * \file DbTableTest.h
 * \author Elias Gerber <eg@zame.ch>
 * \date 09.02.2014
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef DBTABLETEST_H
#define DBTABLETEST_H

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

// DbTableTest
// ----------------
class DbTableTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( DbTableTest );
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
CPPUNIT_TEST_SUITE_REGISTRATION( DbTableTest );


#endif // DBTABLETEST_H
