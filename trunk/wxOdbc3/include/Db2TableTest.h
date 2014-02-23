/*!
* \file Db2TableTest.h
* \author Elias Gerber <eg@zame.ch>
* \date 23.02.2014
* 
* [Brief Header-file description]
*/ 

#pragma once
#ifndef DB2TABLETEST_H
#define DB2TABLETEST_H

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

namespace DB2
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
		//void testCharTypes();
		//void testFloatTypes();
		//void testDateTypes();

	protected:

		wxDbConnectInf*		m_pConnectInfDb2;
		wxDb*				m_pDbDb2;
		bool				m_connectedDb2;

		std::wstring		m_dsn;
	};

}

namespace DB2_10_05
{

	class TableTest 
		: public CPPUNIT_NS::TestFixture
		, public DB2::TableTestBase
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
		//CPPUNIT_TEST( testCharTypes );
		//CPPUNIT_TEST( testFloatTypes );
		//CPPUNIT_TEST( testDateTypes );
		CPPUNIT_TEST_SUITE_END();

	public:
		virtual void setUp();
		virtual void tearDown();
	};

	CPPUNIT_TEST_SUITE_REGISTRATION( TableTest );

}

#endif // DB2TABLETEST_H
