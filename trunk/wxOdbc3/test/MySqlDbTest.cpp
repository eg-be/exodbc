/*!
 * \file DbTest.cpp
 * \author Elias Gerber <eg@zame.ch>
 * \date 22.09.2013
 * 
 * [Brief CPP-file description]
 */ 

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif

// Own header
#include "MySqlDbTest.h"

// Same component headers
#include "MySqlParams.h"
#include "db.h"

// Other headers
#include "cppunit/config/SourcePrefix.h"

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// DbTest
// ------
void DbTest::setUp()
{
	// Create DbConnectInfs for various databases
	m_pConnectInfMySql = new wxDbConnectInf(NULL, MYSQL_5_2_DSN, MYSQL_USER, MYSQL_PASS);
	m_pDbMySql = new wxDb(m_pConnectInfMySql->GetHenv());
}


void DbTest::tearDown()
{
	if(m_pConnectInfMySql)
		delete m_pConnectInfMySql;
	if(m_pDbMySql)
		delete m_pDbMySql;
}


void DbTest::testOpen()
{
	// Open with failing on unsupported datatypes
	CPPUNIT_ASSERT( m_pDbMySql->Open(m_pConnectInfMySql, true) );

	// Try to open one with not only forward-cursorts
	wxDb* pdbMySqlBC = new wxDb(m_pConnectInfMySql->GetHenv(), false);
	try
	{
		CPPUNIT_ASSERT( pdbMySqlBC->Open(m_pConnectInfMySql, true));
	}
	catch(CPPUNIT_NS::Exception e)
	{
		delete pdbMySqlBC;
		throw e;
	}
	delete pdbMySqlBC;
}

// Interfaces
// ----------
