/*!
* \file MySqlTableTest.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 09.02.2014
* 
* [Brief CPP-file description]
*/ 

// Own header
#include "MySqlTableTest.h"

// Same component headers
#include "GenericTestTables.h"
#include "MySqlParams.h"
#include "MySqlTestTables.h"
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
namespace MySql_5_2
{


	void TableTest::setUp()
	{
		m_dsn = DSN_MYSQL_5_2;
		TableTestBase::setUp(m_dsn);
	}


	void TableTest::tearDown()
	{
		TableTestBase::tearDown();
		m_dsn = L"";
	}

}


namespace MySql_3_51
{


	void TableTest::setUp()
	{
		m_dsn = DSN_MYSQL_3_51;
		TableTestBase::setUp(m_dsn);
	}


	void TableTest::tearDown()
	{
		TableTestBase::tearDown();
		m_dsn = L"";
	}

}


namespace MySql
{
	void TableTestBase::setUp(const std::wstring& dsn)
	{
		// Create DbConnectInfs for various databases
		m_pConnectInfMySql = new wxDbConnectInf(NULL, dsn, USER_MYSQL, PASS_MYSQL);

		// bool ok = m_pConnectInfMySql->SetSqlAttrOdbcVersion(SQL_OV_ODBC2);
		unsigned long odbcVersion = m_pConnectInfMySql->ReadSqlAttrOdbcVersion();
		CPPUNIT_ASSERT_EQUAL(SQL_OV_ODBC2, odbcVersion);

		m_pDbMySql = new wxDb(m_pConnectInfMySql->GetHenv());
		m_connectedMySql = m_pDbMySql->Open(m_pConnectInfMySql);

		// tearDown will care to delete the allocated things, it is called in all cases, even
		// if this assertion throws
		CPPUNIT_ASSERT( m_connectedMySql );
	}


	void TableTestBase::tearDown()
	{
		delete m_pConnectInfMySql;
		delete m_pDbMySql;
	}


	void TableTestBase::testOpenExistingNoChecks()
	{
		CPPUNIT_ASSERT( m_connectedMySql );

		IntTypesTable* pTable = NULL;
		try
		{
			pTable = new IntTypesTable(m_pDbMySql);
			// Open without any checking
			CPPUNIT_ASSERT( pTable->Open(false, false) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbMySql, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testOpenExistingCheckPrivilegs()
	{
		// This test fails because of ticket #4
		//CPPUNIT_ASSERT( m_connectedMySql );

		//// Open with checking privileges
		//CPPUNIT_ASSERT( m_pQueryTypesTable->Open(true, false) );
	}


	void TableTestBase::testOpenExistingCheckExistance()
	{
		CPPUNIT_ASSERT( m_connectedMySql );

		IntTypesTable* pTable = NULL;
		try
		{
			pTable = new IntTypesTable(m_pDbMySql);
			CPPUNIT_ASSERT( pTable->Open(false, true) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbMySql, pTable);
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
		CPPUNIT_ASSERT( m_connectedMySql );

		NotExistingTable* pTable = NULL;
		try
		{
			pTable = new NotExistingTable(m_pDbMySql);
			// Open without any checking - should work even if table does not exist
			// Some drivers report an error on binding.
			// I dont know yet what is "correct": See Ticket #14
			if(m_dsn != DSN_MYSQL_5_2)
			{
				CPPUNIT_ASSERT( pTable->Open(false, false) );
			}
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbMySql, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testOpenNotExistingCheckPrivilegs()
	{
		CPPUNIT_ASSERT( m_connectedMySql );

		NotExistingTable* pTable = NULL;
		try
		{
			pTable = new NotExistingTable(m_pDbMySql);
			// Open with checking privileges - should fail, how to have privs on a non existing table=
			CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT( pTable->Open(true, false)) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbMySql, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testOpenNotExistingCheckExistance()
	{
		CPPUNIT_ASSERT( m_connectedMySql );

		NotExistingTable* pTable = NULL;
		try
		{
			pTable = new NotExistingTable(m_pDbMySql);
			// must fail
			CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT( pTable->Open(false, true)) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbMySql, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testOpenNotExistingCheckBoth()
	{
		CPPUNIT_ASSERT( m_connectedMySql );

		NotExistingTable* pTable = NULL;
		try
		{
			pTable = new NotExistingTable(m_pDbMySql);
			// must fail
			CPPUNIT_ASSERT_ASSERTION_FAIL( CPPUNIT_ASSERT( pTable->Open(true, true)) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbMySql, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testIntTypes()
	{
		CPPUNIT_ASSERT( m_connectedMySql );

		IntTypesTable* pTable = NULL;
		try
		{
			pTable = new IntTypesTable(m_pDbMySql);
			// Open table
			CPPUNIT_ASSERT( pTable->Open() );

			// test for min-max values
			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 1"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int8_t) -128, pTable->m_tinyInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 2"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int8_t) 127, pTable->m_tinyInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 3"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int16_t) -32768, pTable->m_smallInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 4"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int16_t) 32767, pTable->m_smallInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 5"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int32_t) -8388608, pTable->m_mediumInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 6"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int32_t) 8388607, pTable->m_mediumInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 7"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int32_t) -2147483648, pTable->m_int );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 8"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int32_t) 2147483647, pTable->m_int );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 9"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int64_t) -9223372036854775808, pTable->m_bigInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 10"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (int64_t) 9223372036854775807, pTable->m_bigInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 11"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (uint8_t) 255, pTable->m_utinyInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 12"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (uint16_t) 65535, pTable->m_usmallInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 13"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (uint32_t) 16777215, pTable->m_umediumInt );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 14"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT_EQUAL( (uint32_t) 4294967295, pTable->m_uint );

			// Fails with MySql ODBC Connector 5.x. See Ticket #16
			if(m_dsn != DSN_MYSQL_5_2)
			{
				CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 15"));
				CPPUNIT_ASSERT( pTable->GetNext());
				CPPUNIT_ASSERT_EQUAL( (uint64_t) 18446744073709551615, pTable->m_ubigInt );
			}

			// Test for NULL-Values
			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 15"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT( !pTable->IsColNull(0) );
			CPPUNIT_ASSERT( pTable->IsColNull(1) );
			CPPUNIT_ASSERT( pTable->IsColNull(2) );
			CPPUNIT_ASSERT( pTable->IsColNull(3) );
			CPPUNIT_ASSERT( pTable->IsColNull(4) );
			CPPUNIT_ASSERT( pTable->IsColNull(5) );
			CPPUNIT_ASSERT( pTable->IsColNull(6) );
			CPPUNIT_ASSERT( pTable->IsColNull(7) );
			CPPUNIT_ASSERT( pTable->IsColNull(8) );
			CPPUNIT_ASSERT( pTable->IsColNull(9) );
			CPPUNIT_ASSERT( !pTable->IsColNull(10) );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 1"));
			CPPUNIT_ASSERT( pTable->GetNext());
			CPPUNIT_ASSERT( !pTable->IsColNull(0) );
			CPPUNIT_ASSERT( !pTable->IsColNull(1) );
			CPPUNIT_ASSERT( pTable->IsColNull(2) );
			CPPUNIT_ASSERT( pTable->IsColNull(3) );
			CPPUNIT_ASSERT( pTable->IsColNull(4) );
			CPPUNIT_ASSERT( pTable->IsColNull(5) );
			CPPUNIT_ASSERT( pTable->IsColNull(6) );
			CPPUNIT_ASSERT( pTable->IsColNull(7) );
			CPPUNIT_ASSERT( pTable->IsColNull(8) );
			CPPUNIT_ASSERT( pTable->IsColNull(9) );
			CPPUNIT_ASSERT( pTable->IsColNull(10) );	
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbMySql, pTable);
		if(pTable)
			delete pTable;
	}


	void TableTestBase::testCharTypes()
	{
		CPPUNIT_ASSERT( m_connectedMySql );

		CharTypesTable* pTable = NULL;
		try
		{
			pTable = new CharTypesTable(m_pDbMySql);
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
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbMySql, pTable);

		if(pTable)
			delete pTable;
	}


	void TableTestBase::testFloatTypes()
	{
		CPPUNIT_ASSERT( m_connectedMySql );

		FloatTypesTable* pTable = NULL;
		try
		{
			pTable = new FloatTypesTable(m_pDbMySql);

			CPPUNIT_ASSERT( pTable->Open() );

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
			// Note: MySQL uses as '.'
			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 7"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( wxString(L"0.0000000000"), wxString(pTable->m_decimal_15_10 ));

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 8"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( wxString(L"33333.1415926530"), wxString(pTable->m_decimal_15_10 ));

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 9"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( wxString(L"-33333.1415926530"), wxString(pTable->m_decimal_15_10 ) );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 10"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( wxString(L"0"), wxString(pTable->m_decimal_10_0 ));

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 11"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( wxString(L"1234567890"), wxString(pTable->m_decimal_10_0 ));

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 12"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( wxString(L"-1234567890"), wxString(pTable->m_decimal_10_0 ) );
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbMySql, pTable)

		if(pTable)
			delete pTable;
	}


	void TableTestBase::testDateTypes()
	{
		CPPUNIT_ASSERT( m_connectedMySql );

		DateTypesTable* pTable = NULL;
		try
		{

			pTable = new DateTypesTable(m_pDbMySql);

			CPPUNIT_ASSERT( pTable->Open() );

			CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.datetypes WHERE iddatetypes = 1"));
			CPPUNIT_ASSERT( pTable->GetNext() );
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 26, pTable->m_date.day);
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 1, pTable->m_date.month);
			CPPUNIT_ASSERT_EQUAL( (SQLSMALLINT) 1983, pTable->m_date.year);

			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 26, pTable->m_datetime.day);
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 1, pTable->m_datetime.month);
			CPPUNIT_ASSERT_EQUAL( (SQLSMALLINT) 1983, pTable->m_datetime.year);
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 13, pTable->m_datetime.hour);
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 55, pTable->m_datetime.minute);
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 56, pTable->m_datetime.second);

			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 13, pTable->m_time.hour);
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 55, pTable->m_time.minute);
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 56, pTable->m_time.second);		
			
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 26, pTable->m_timestamp.day);
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 1, pTable->m_timestamp.month);
			CPPUNIT_ASSERT_EQUAL( (SQLSMALLINT) 1983, pTable->m_timestamp.year);
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 13, pTable->m_timestamp.hour);
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 55, pTable->m_timestamp.minute);
			CPPUNIT_ASSERT_EQUAL( (SQLUSMALLINT) 56, pTable->m_timestamp.second);
			// Note: MySql has no nanoseconds?

			CPPUNIT_ASSERT_EQUAL( (SQLSMALLINT) 1983, pTable->m_year);
		}
		CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbMySql, pTable)

		if(pTable)
			delete pTable;
	}

}