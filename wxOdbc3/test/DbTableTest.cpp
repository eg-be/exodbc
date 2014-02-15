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
		m_pQueryTypesTable = new IntTypesTable(m_pDbMySql);
		m_pNotExistingTable = new NotExistingTable(m_pDbMySql);
	}
}


void DbTableTest::tearDown()
{
	if(m_connectedMySql)
	{
		delete m_pQueryTypesTable;
		delete m_pNotExistingTable;
	}

	delete m_pConnectInfMySql;
	delete m_pDbMySql;
}


void DbTableTest::testOpenExistingNoChecks()
{
	CPPUNIT_ASSERT( m_connectedMySql );
	// Open without any checking
	CPPUNIT_ASSERT( m_pQueryTypesTable->Open(false, false) );
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

	CPPUNIT_ASSERT( m_pQueryTypesTable->Open(false, true) );
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


void DbTableTest::testQueryInteger()
{
//	CPPUNIT_ASSERT( m_connectedMySql );

	wxDbConnectInf* pConnectInfMySql = new wxDbConnectInf(NULL, MYSQL_DSN, MYSQL_USER, MYSQL_PASS);
	wxDb* pDbMySql = new wxDb(pConnectInfMySql->GetHenv());
	bool conn = pDbMySql->Open(pConnectInfMySql);
	CPPUNIT_ASSERT( conn );

	if( conn )
	{
		IntTypesTable* pTable = new IntTypesTable(pDbMySql);
		bool ok2 = pTable->Open(false, false);
		std::vector<wxString> errs = pDbMySql->GetErrorList();
//		wxString sqlstmt = L"SELECT \"idintegertypes\" FROM \"wxodbc3\".\"integertypes\"";
		wxString sqlstmt = L"SELECT IDT1 FROM TEST.T1";
		bool ok1 = pTable->QueryBySqlStmt(sqlstmt);
		errs = pDbMySql->GetErrorList();
		while(pTable->GetNext())
		{
			int p = 3;
		}
	}

//	// test for min-max values
//	wxString sqlstmt = L"SELECT * FROM wxodbc3.integertypes";
////	TEST_SQL( m_pQueryTypesTable->QueryBySqlStmt(sqlstmt), m_pDbMySql );
//	bool ok1 = m_pQueryTypesTable->QueryBySqlStmt(sqlstmt);
//	CPPUNIT_ASSERT(ok1);
//	bool ok = m_pQueryTypesTable->GetNext();
//	while(ok)
//	{
//		ok = m_pQueryTypesTable->GetNext();
//		int p = 3;
//	}
//	CPPUNIT_ASSERT( m_pQueryTypesTable->GetNext() );
//	CPPUNIT_ASSERT_EQUAL( (int8_t) -128, m_pQueryTypesTable->m_tinyInt );
}
// Interfaces
// ----------

