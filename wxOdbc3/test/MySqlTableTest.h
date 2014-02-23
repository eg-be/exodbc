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
class CharTypesTable;
class FloatTypesTable;
class DateTypesTable;
class NotExistingTable;

// Structs
// -------

namespace MySql
{

	// TableTestBase
	// ----------------
	class TableTestBase
	{

	public:
		virtual void setUp(const std::wstring& dsn);
		virtual void tearDown();

		void testOpenExistingNoChecks();
		void testOpenExistingCheckPrivilegs();
		void testOpenExistingCheckExistance();
		void testOpenExistingCheckBoth();

		void testOpenNotExistingNoChecks();
		void testOpenNotExistingCheckPrivilegs();
		void testOpenNotExistingCheckExistance();
		void testOpenNotExistingCheckBoth();

		void testIntTypes();
		void testCharTypes();
		void testFloatTypes();
		void testDateTypes();

	protected:

		wxDbConnectInf*		m_pConnectInfMySql;
		wxDb*				m_pDbMySql;
		bool				m_connectedMySql;

		//IntTypesTable*		m_pIntTypesTable;
		//CharTypesTable*		m_pCharTypesTable;
		//FloatTypesTable*	m_pFloatTypesTable;
		//DateTypesTable*		m_pDateTypesTable;
		//NotExistingTable*	m_pNotExistingTable;

		std::wstring		m_dsn;
	};

}

namespace MySql_5_2
{

	class TableTest 
		: public CPPUNIT_NS::TestFixture
		, public MySql::TableTestBase
	{
		CPPUNIT_TEST_SUITE( TableTest );
		CPPUNIT_TEST( testOpenExistingNoChecks );
		CPPUNIT_TEST( testOpenExistingCheckPrivilegs );
		CPPUNIT_TEST( testOpenExistingCheckExistance );
		CPPUNIT_TEST( testOpenExistingCheckBoth );

		CPPUNIT_TEST( testOpenNotExistingNoChecks );
		CPPUNIT_TEST( testOpenNotExistingCheckPrivilegs );
		CPPUNIT_TEST( testOpenNotExistingCheckExistance );
		CPPUNIT_TEST( testOpenNotExistingCheckBoth );

		CPPUNIT_TEST( testIntTypes );
		CPPUNIT_TEST( testCharTypes );
		CPPUNIT_TEST( testFloatTypes );
		CPPUNIT_TEST( testDateTypes );
		CPPUNIT_TEST_SUITE_END();

	public:
		virtual void setUp();
		virtual void tearDown();
	};

	CPPUNIT_TEST_SUITE_REGISTRATION( TableTest );

}

namespace MySql_3_51
{

	class TableTest 
		: public CPPUNIT_NS::TestFixture
		, public MySql::TableTestBase
	{
		CPPUNIT_TEST_SUITE( TableTest );
		CPPUNIT_TEST( testOpenExistingNoChecks );
		CPPUNIT_TEST( testOpenExistingCheckPrivilegs );
		CPPUNIT_TEST( testOpenExistingCheckExistance );
		CPPUNIT_TEST( testOpenExistingCheckBoth );

		CPPUNIT_TEST( testOpenNotExistingNoChecks );
		CPPUNIT_TEST( testOpenNotExistingCheckPrivilegs );
		CPPUNIT_TEST( testOpenNotExistingCheckExistance );
		CPPUNIT_TEST( testOpenNotExistingCheckBoth );

		CPPUNIT_TEST( testIntTypes );
		CPPUNIT_TEST( testCharTypes );
		CPPUNIT_TEST( testFloatTypes );
		CPPUNIT_TEST( testDateTypes );
		CPPUNIT_TEST_SUITE_END();

	public:
		virtual void setUp();
		virtual void tearDown();
	};

	CPPUNIT_TEST_SUITE_REGISTRATION( TableTest );

}

#endif // DBTABLETEST_H
