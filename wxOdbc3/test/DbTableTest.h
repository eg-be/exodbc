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
class IntTypesTable;
class NotExistingTable;

// Structs
// -------

// DbTableTest
// ----------------
class DbTableTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( DbTableTest );
	CPPUNIT_TEST( testOpenExistingNoChecks );
	CPPUNIT_TEST( testOpenExistingCheckPrivilegs );
	CPPUNIT_TEST( testOpenExistingCheckExistance );
	CPPUNIT_TEST( testOpenExistingCheckBoth );

	CPPUNIT_TEST( testOpenNotExistingNoChecks );
	CPPUNIT_TEST( testOpenNotExistingCheckPrivilegs );
	CPPUNIT_TEST( testOpenNotExistingCheckExistance );
	CPPUNIT_TEST( testOpenNotExistingCheckBoth );

	CPPUNIT_TEST( testQueryInteger );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testOpenExistingNoChecks();
	void testOpenExistingCheckPrivilegs();
	void testOpenExistingCheckExistance();
	void testOpenExistingCheckBoth();

	void testOpenNotExistingNoChecks();
	void testOpenNotExistingCheckPrivilegs();
	void testOpenNotExistingCheckExistance();
	void testOpenNotExistingCheckBoth();

	void testQueryInteger();

private:
	wxDbConnectInf*		m_pConnectInfMySql;
	wxDb*				m_pDbMySql;
	bool				m_connectedMySql;

	IntTypesTable*	m_pQueryTypesTable;
	NotExistingTable*	m_pNotExistingTable;
};
CPPUNIT_TEST_SUITE_REGISTRATION( DbTableTest );


#endif // DBTABLETEST_H
