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

// Other headers
#include "cppunit/config/SourcePrefix.h"
#include "db.h"
#include "dbtable.h"

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
	CPPUNIT_ASSERT( m_pDbMySql->Open(m_pConnectInfMySql) );
}


void DbTableTest::tearDown()
{
	delete m_pConnectInfMySql;
	delete m_pDbMySql;
}


void DbTableTest::testOpen()
{
	// Open without any checking
	QueryTypesTable* pQueryTypesTable = new QueryTypesTable(m_pDbMySql);
	CPPUNIT_ASSERT_NO_THROW( pQueryTypesTable->Open(false, false) );
	delete pQueryTypesTable;

	// Open with checking privileges
	pQueryTypesTable = new QueryTypesTable(m_pDbMySql);
	CPPUNIT_ASSERT_NO_THROW( pQueryTypesTable->Open(true, false) );
	delete pQueryTypesTable;

	// Open with checking existance
	pQueryTypesTable = new QueryTypesTable(m_pDbMySql);
	CPPUNIT_ASSERT_NO_THROW( pQueryTypesTable->Open(false, true) );
	delete pQueryTypesTable;

	// Open with checking both
	pQueryTypesTable = new QueryTypesTable(m_pDbMySql);
	CPPUNIT_ASSERT_NO_THROW( pQueryTypesTable->Open(true, true) );
	delete pQueryTypesTable;

}

// Interfaces
// ----------

