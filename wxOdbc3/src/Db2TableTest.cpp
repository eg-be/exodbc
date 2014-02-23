/*!
* \file Db2TableTest.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 23.02.2014
* 
* [Brief CPP-file description]
*/ 

// Own header
#include "Db2TableTest.h"

// Same component headers
#include "GenericTestTables.h"
#include "Db2Params.h"
#include "Db2TestTables.h"
#include "wxOdbc3Test.h"

// Other headers
#include "cppunit/config/SourcePrefix.h"
#include "db.h"
#include "dbtable.h"
#include <vector>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// DbTableTest
// -----------
namespace DB2_10_05
{


	void TableTest::setUp()
	{
		m_dsn = DB2_DSN;
		TableTestBase::setUp(m_dsn);
	}


	void TableTest::tearDown()
	{
		TableTestBase::tearDown();
		m_dsn = L"";
	}

}


namespace DB2
{
	void TableTestBase::setUp(const std::wstring& dsn)
	{
		// Create DbConnectInfs for various databases
		m_pConnectInfDb2 = new wxDbConnectInf(NULL, dsn, DB2_USER, DB2_PASS);
		m_pDbDb2 = new wxDb(m_pConnectInfDb2->GetHenv());
		m_connectedDb2 = m_pDbDb2->Open(m_pConnectInfDb2);

		// tearDown will care to delete the allocated things, it is called in all cases, even
		// if this assertion throws
		CPPUNIT_ASSERT( m_connectedDb2 );
	}


	void TableTestBase::tearDown()
	{
		delete m_pConnectInfDb2;
		delete m_pDbDb2;
	}


	void TableTestBase::testOpenExistingNoChecks()
	{
		CPPUNIT_ASSERT( m_connectedDb2 );

		IntTypesTable* pTable = NULL;
		try
		{
			pTable = new IntTypesTable(m_pDbDb2);
			// Open without any checking
			CPPUNIT_ASSERT( pTable->Open(false, false) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbDb2, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testOpenExistingCheckPrivilegs()
	{
		// This test fails because of ticket #4
		//CPPUNIT_ASSERT( m_connectedDb2 );

		//IntTypesTable* pTable = NULL;
		//try
		//{
		//	pTable = new IntTypesTable(m_pDbDb2);
		//	// Open with checking privileges
		//	CPPUNIT_ASSERT( pTable->Open(true, false) );
		//}
		//CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbDb2, pTable);
		//if(pTable)
		//	delete pTable;
	}


	void TableTestBase::testOpenExistingCheckExistance()
	{
		CPPUNIT_ASSERT( m_connectedDb2 );

		IntTypesTable* pTable = NULL;
		try
		{
			pTable = new IntTypesTable(m_pDbDb2);
			CPPUNIT_ASSERT( pTable->Open(false, true) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbDb2, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testOpenExistingCheckBoth()
	{
		// This test fails because of ticket #4
		//CPPUNIT_ASSERT( m_connectedMySql );

		//CPPUNIT_ASSERT( m_pQueryTypesTable->Open(true, true) );
	}


	void TableTestBase::testOpenNotExistingNoChecks()
	{
		CPPUNIT_ASSERT( m_connectedDb2 );

		NotExistingTable* pTable = NULL;
		try
		{
			pTable = new NotExistingTable(m_pDbDb2);
			// Open without any checking - should work even if table does not exist
			// Some drivers report an error on binding.
			// I dont know yet what is "correct": See Ticket #14
			if(m_dsn != DB2_DSN)
			{
				CPPUNIT_ASSERT( pTable->Open(false, false) );
			}
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbDb2, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testOpenNotExistingCheckPrivilegs()
	{
		CPPUNIT_ASSERT( m_connectedDb2 );

		NotExistingTable* pTable = NULL;
		try
		{
			pTable = new NotExistingTable(m_pDbDb2);
			// Open with checking privileges - should fail, how to have privs on a non existing table=
			CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT( pTable->Open(true, false)) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbDb2, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testOpenNotExistingCheckExistance()
	{
		CPPUNIT_ASSERT( m_connectedDb2 );

		NotExistingTable* pTable = NULL;
		try
		{
			pTable = new NotExistingTable(m_pDbDb2);
			// must fail
			CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT( pTable->Open(false, true)) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbDb2, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testOpenNotExistingCheckBoth()
	{
		CPPUNIT_ASSERT( m_connectedDb2 );

		NotExistingTable* pTable = NULL;
		try
		{
			pTable = new NotExistingTable(m_pDbDb2);
			// must fail
			CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT( pTable->Open(true, true)) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbDb2, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testIntTypes()
	{
		CPPUNIT_ASSERT( m_connectedDb2 );

		IntTypesTable* pTable = NULL;
		try
		{
			pTable = new IntTypesTable(m_pDbDb2);
			// Open table
			CPPUNIT_ASSERT( pTable->Open( ) );

			// test for min-max values 
			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 1"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int16_t) -32768, pTable->m_smallInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 2"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int16_t) 32767, pTable->m_smallInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 3"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int32_t) -2147483648, pTable->m_int );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 4"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int32_t) 2147483647, pTable->m_int );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 5"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int64_t) -9223372036854775808, pTable->m_bigInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 6"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int64_t) 9223372036854775807, pTable->m_bigInt );

			// Test for NULL-Values
			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 1"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT( !pTable->IsColNull(0) );
			CPPUNIT_ASSERT( !pTable->IsColNull(1) );
			CPPUNIT_ASSERT( pTable->IsColNull(2) );
			CPPUNIT_ASSERT( pTable->IsColNull(3) );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 3"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT( !pTable->IsColNull(0) );
			CPPUNIT_ASSERT( pTable->IsColNull(1) );
			CPPUNIT_ASSERT( !pTable->IsColNull(2) );
			CPPUNIT_ASSERT( pTable->IsColNull(3) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbDb2, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testCharTypes()
	{
		CPPUNIT_ASSERT( m_connectedDb2 );

		CharTypesTable* pTable = NULL;
		try
		{
			pTable = new CharTypesTable(m_pDbDb2);
			CPPUNIT_ASSERT( pTable->Open() );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.CHARTYPES WHERE IDCHARTYPES = 1"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( wxString(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), wxString(pTable->m_varchar));

			// Note: DB2 pads with whitespaces
			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.CHARTYPES WHERE IDCHARTYPES = 2"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( wxString(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 "), wxString(pTable->m_char));

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.CHARTYPES WHERE IDCHARTYPES = 3"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( wxString(L"הצüאיט"), wxString(pTable->m_varchar));

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.CHARTYPES WHERE IDCHARTYPES = 4"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( wxString(L"הצüאיט                                                                                                                    "), wxString(pTable->m_char));

			// Test for NULL-Values
			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.CHARTYPES WHERE IDCHARTYPES = 4"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT( !pTable->IsColNull(0) );
			CPPUNIT_ASSERT( pTable->IsColNull(1) );
			CPPUNIT_ASSERT( !pTable->IsColNull(2) );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.CHARTYPES WHERE IDCHARTYPES = 1"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT( !pTable->IsColNull(0) );
			CPPUNIT_ASSERT( !pTable->IsColNull(1) );
			CPPUNIT_ASSERT( pTable->IsColNull(2) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbDb2, pTable);

		if(pTable)
			delete pTable;
	}


	void TableTestBase::testFloatTypes()
	{
		CPPUNIT_ASSERT( m_connectedDb2 );

		FloatTypesTable* pTable = NULL;
		try
		{
			pTable = new FloatTypesTable(m_pDbDb2);

			CPPUNIT_ASSERT( pTable->Open() );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.FLOATTYPES WHERE IDFLOATTYPES = 1"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( 0.0, pTable->m_float );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.FLOATTYPES WHERE IDFLOATTYPES = 2"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( 3.141, pTable->m_float );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.FLOATTYPES WHERE IDFLOATTYPES = 3"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( -3.141, pTable->m_float );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.FLOATTYPES WHERE IDFLOATTYPES = 4"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( 0.0, pTable->m_double );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.FLOATTYPES WHERE IDFLOATTYPES = 5"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( 3.141592, pTable->m_double );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.FLOATTYPES WHERE IDFLOATTYPES = 6"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( -3.141592, pTable->m_double );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.FLOATTYPES WHERE IDFLOATTYPES = 7"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( 0.0f, pTable->m_real );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.FLOATTYPES WHERE IDFLOATTYPES = 8"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( 3.141592f, pTable->m_real);

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.FLOATTYPES WHERE IDFLOATTYPES = 9"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( -3.141592f, pTable->m_real );

			// Test for NULL Values
			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.FLOATTYPES WHERE IDFLOATTYPES = 1"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT( !pTable->IsColNull(0) );
			CPPUNIT_ASSERT( pTable->IsColNull(1) );
			CPPUNIT_ASSERT( !pTable->IsColNull(2) );
			CPPUNIT_ASSERT( pTable->IsColNull(3) );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.FLOATTYPES WHERE IDFLOATTYPES = 4"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT( !pTable->IsColNull(0) );
			CPPUNIT_ASSERT( !pTable->IsColNull(1) );
			CPPUNIT_ASSERT( pTable->IsColNull(2) );
			CPPUNIT_ASSERT( pTable->IsColNull(3) );

		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbDb2, pTable)

			if(pTable)
				delete pTable;
	}


	//void TableTestBase::testDateTypes()
	//{
	//	CPPUNIT_ASSERT( m_connectedMySql );

	//	DateTypesTable* pTable = NULL;
	//	try
	//	{

	//		pTable = new DateTypesTable(m_pDbMySql);

	//		CPPUNIT_ASSERT( pTable->Open() );

	//		CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.datetypes WHERE iddatetypes = 1"));
	//		//		CPPUNIT_ASSERT( pTable->GetNext() );
	//		//		CPPUNIT_ASSERT_EQUAL( 0.0, pTable->m_float );

	//	}
	//	CATCH_LOG_RETHROW_DELETE_TABLE(pTable)

	//		if(pTable)
	//			delete pTable;
	//}

}