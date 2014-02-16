/*!
 * \file DbTableTest.cpp
 * \author Elias Gerber <eg@zame.ch>
 * \date 09.02.2014
 * 
 * [Brief CPP-file description]
 */ 

//#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#ifndef DBG_NEW
//#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
//#define new DBG_NEW
//#endif
//#endif

// Own header
#include "DbTableTest.h"

// Same component headers
#include "DbParams.h"
#include "TestTables.h"
#include "wxOdbc3Test.h"

// Other headers
#include "cppunit/config/SourcePrefix.h"
#include "db.h"
#include "dbtable.h"
#include <vector>

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// DbTableTest
// -----------
void DbTableTest::setUp()
{
	// Create DbConnectInfs for various databases
	m_pConnectInfMySql = new wxDbConnectInf(NULL, MYSQL_DSN, MYSQL_USER, MYSQL_PASS);
	m_pDbMySql = new wxDb(m_pConnectInfMySql->GetHenv());
	m_connectedMySql = m_pDbMySql->Open(m_pConnectInfMySql);
	CPPUNIT_ASSERT( m_connectedMySql );

	if(m_connectedMySql)
	{
		m_pIntTypesTable = new IntTypesTable(m_pDbMySql);
		m_pNotExistingTable = new NotExistingTable(m_pDbMySql);
	}
}


void DbTableTest::tearDown()
{
	if(m_connectedMySql)
	{
		delete m_pIntTypesTable;
		delete m_pNotExistingTable;
	}

	delete m_pConnectInfMySql;
	delete m_pDbMySql;
}


void DbTableTest::testOpenExistingNoChecks()
{
	CPPUNIT_ASSERT( m_connectedMySql );
	// Open without any checking
	CPPUNIT_ASSERT( m_pIntTypesTable->Open(false, false) );
}


void DbTableTest::testOpenExistingCheckPrivilegs()
{
	// This test fails because of ticket #4
	//CPPUNIT_ASSERT( m_connectedMySql );

	//// Open with checking privileges
	//CPPUNIT_ASSERT( m_pQueryTypesTable->Open(true, false) );
}


void DbTableTest::testOpenExistingCheckExistance()
{
	CPPUNIT_ASSERT( m_connectedMySql );

	CPPUNIT_ASSERT( m_pIntTypesTable->Open(false, true) );
}


void DbTableTest::testOpenExistingCheckBoth()
{
	// This test fails because of ticket #4
	//CPPUNIT_ASSERT( m_connectedMySql );

	//CPPUNIT_ASSERT( m_pQueryTypesTable->Open(true, true) );
}


void DbTableTest::testOpenNotExistingNoChecks()
{
	CPPUNIT_ASSERT( m_connectedMySql );

	// Open without any checking - should work even if table does not exist
	CPPUNIT_ASSERT( m_pNotExistingTable->Open(false, false) );
}


void DbTableTest::testOpenNotExistingCheckPrivilegs()
{
	CPPUNIT_ASSERT( m_connectedMySql );

	// Open with checking privileges - should fail, how to have privs on a non existing table=
	CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT(m_pNotExistingTable->Open(true, false)) );
}


void DbTableTest::testOpenNotExistingCheckExistance()
{
	CPPUNIT_ASSERT( m_connectedMySql );

	// must fail
	CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT(m_pNotExistingTable->Open(false, true)) );
}


void DbTableTest::testOpenNotExistingCheckBoth()
{
	CPPUNIT_ASSERT( m_connectedMySql );

	// must fail
	CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT(m_pNotExistingTable->Open(true, true)) );
}


void DbTableTest::testIntTypes()
{
	CPPUNIT_ASSERT( m_connectedMySql );

	// Open table
	CPPUNIT_ASSERT( m_pIntTypesTable->Open() );
	// test for min-max values
//	TEST_SQL( m_pQueryTypesTable->QueryBySqlStmt(sqlstmt), m_pDbMySql );
	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 1"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (int8_t) -128, m_pIntTypesTable->m_tinyInt );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 2"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (int8_t) 127, m_pIntTypesTable->m_tinyInt );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 3"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (int16_t) -32768, m_pIntTypesTable->m_smallInt );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 4"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (int16_t) 32767, m_pIntTypesTable->m_smallInt );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 5"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (int32_t) -8388608, m_pIntTypesTable->m_mediumInt );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 6"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (int32_t) 8388607, m_pIntTypesTable->m_mediumInt );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 7"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (int32_t) -2147483648, m_pIntTypesTable->m_int );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 8"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (int32_t) 2147483647, m_pIntTypesTable->m_int );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 9"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (int64_t) -9223372036854775808, m_pIntTypesTable->m_bigInt );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 10"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (int64_t) 9223372036854775807, m_pIntTypesTable->m_bigInt );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 11"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (uint8_t) 255, m_pIntTypesTable->m_utinyInt );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 12"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (uint16_t) 65535, m_pIntTypesTable->m_usmallInt );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 13"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (uint32_t) 16777215, m_pIntTypesTable->m_umediumInt );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 14"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (uint32_t) 4294967295, m_pIntTypesTable->m_uint );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 15"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT_EQUAL( (uint64_t) 18446744073709551615, m_pIntTypesTable->m_ubigInt );

	// Test for NULL-Values
	CPPUNIT_ASSERT( !m_pIntTypesTable->IsColNull(0) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(1) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(2) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(3) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(4) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(5) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(6) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(7) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(8) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(9) );
	CPPUNIT_ASSERT( !m_pIntTypesTable->IsColNull(10) );

	CPPUNIT_ASSERT( m_pIntTypesTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 1"));
	CPPUNIT_ASSERT( m_pIntTypesTable->GetNext());
	CPPUNIT_ASSERT( !m_pIntTypesTable->IsColNull(0) );
	CPPUNIT_ASSERT( !m_pIntTypesTable->IsColNull(1) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(2) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(3) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(4) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(5) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(6) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(7) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(8) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(9) );
	CPPUNIT_ASSERT( m_pIntTypesTable->IsColNull(10) );	
}


void DbTableTest::testCharTypes()
{
	CPPUNIT_ASSERT( m_connectedMySql );

	CharTypesTable* pTable = new CharTypesTable(m_pDbMySql);
	CPPUNIT_ASSERT( pTable->Open() );

	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 1"));
	CPPUNIT_ASSERT( pTable->GetNext() );
	CPPUNIT_ASSERT_EQUAL( wxString(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), wxString(pTable->m_varchar));

	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 2"));
	CPPUNIT_ASSERT( pTable->GetNext() );
	CPPUNIT_ASSERT_EQUAL( wxString(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), wxString(pTable->m_char));

	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 3"));
	CPPUNIT_ASSERT( pTable->GetNext() );
	CPPUNIT_ASSERT_EQUAL( wxString(L"הצüאיט"), wxString(pTable->m_varchar));

	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 4"));
	CPPUNIT_ASSERT( pTable->GetNext() );
	CPPUNIT_ASSERT_EQUAL( wxString(L"הצüאיט"), wxString(pTable->m_char));

	// Test for NULL-Values
	CPPUNIT_ASSERT( !pTable->IsColNull(0) );
	CPPUNIT_ASSERT( pTable->IsColNull(1) );
	CPPUNIT_ASSERT( !pTable->IsColNull(2) );

	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 1"));
	CPPUNIT_ASSERT( pTable->GetNext() );
	CPPUNIT_ASSERT( !pTable->IsColNull(0) );
	CPPUNIT_ASSERT( !pTable->IsColNull(1) );
	CPPUNIT_ASSERT( pTable->IsColNull(2) );

	delete pTable;
}


void DbTableTest::testFloatTypes()
{
	CPPUNIT_ASSERT( m_connectedMySql );
	
	FloatTypesTable* pTable = NULL;
	try
	{

		pTable = new FloatTypesTable(m_pDbMySql);
		//CPPUNIT_ASSERT( pTable->Open() );
		bool ok = pTable->Open();
		std::vector<wxString> err = m_pDbMySql->GetErrorList();

		CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 1"));
		CPPUNIT_ASSERT( pTable->GetNext() );
		CPPUNIT_ASSERT_EQUAL( 0.0, pTable->m_float );

		CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 2"));
		CPPUNIT_ASSERT( pTable->GetNext() );
		CPPUNIT_ASSERT_EQUAL( 3.141, pTable->m_float );

		CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 3"));
		CPPUNIT_ASSERT( pTable->GetNext() );
		CPPUNIT_ASSERT_EQUAL( -3.141, pTable->m_float );

		CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 4"));
		CPPUNIT_ASSERT( pTable->GetNext() );
		CPPUNIT_ASSERT_EQUAL( 0.0, pTable->m_double );

		CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 5"));
		CPPUNIT_ASSERT( pTable->GetNext() );
		CPPUNIT_ASSERT_EQUAL( 3.141592, pTable->m_double );

		CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 6"));
		CPPUNIT_ASSERT( pTable->GetNext() );
		CPPUNIT_ASSERT_EQUAL( -3.141592, pTable->m_double );

		// Numeric is not working, see Ticket #15
//		CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 7"));
//		CPPUNIT_ASSERT( pTable->GetNext() );
////		CPPUNIT_ASSERT_EQUAL( 0.0, pTable->m_decimal );
//
//		CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 8"));
//		CPPUNIT_ASSERT( pTable->GetNext() );
//		//CPPUNIT_ASSERT_EQUAL( 3.141592, pTable->m_decimal );
//
//		CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 9"));
//		CPPUNIT_ASSERT( pTable->GetNext() );
//		//CPPUNIT_ASSERT_EQUAL( -3.141592, pTable->m_decimal );

	}
	catch(CPPUNIT_NS::Exception e)
	{
		std::vector<wxString> errors = m_pDbMySql->GetErrorList();
		bool first = false;
		for(size_t i = 0; i < errors.size(); i++)
		{
			if(errors[i].Len() > 0)
			{
				if(!first)
				{
					std::wcout << L"\n";
					first = true;
				}
				std::wcout << errors[i] << L"\n";
			}
		}
		if(pTable)
			delete pTable;
		throw e;
	}
	
	if(pTable)
		delete pTable;
}