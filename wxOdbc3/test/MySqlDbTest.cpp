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
namespace MySql_3_51
{


	void DbTest::setUp()
	{
		m_dsn = DSN_MYSQL_3_51;
		DbTestBase::setUp(m_dsn);
	}


	void DbTest::tearDown()
	{
		DbTestBase::tearDown();
		m_dsn = L"";
	}

}


namespace MySql_5_2
{


	void DbTest::setUp()
	{
		m_dsn = DSN_MYSQL_5_2;
		DbTestBase::setUp(m_dsn);
	}


	void DbTest::tearDown()
	{
		DbTestBase::tearDown();
		m_dsn = L"";
	}

}

namespace MySql
{
	void DbTestBase::setUp(const std::wstring& dsn)
	{
		// Create DbConnectInfs for various databases
		m_pConnectInfMySql = new wxDbConnectInf(NULL, dsn, USER_MYSQL, PASS_MYSQL);
		m_pDbMySql = new wxDb(m_pConnectInfMySql->GetHenv());
	}


	void DbTestBase::tearDown()
	{
		if(m_pConnectInfMySql)
			delete m_pConnectInfMySql;
		if(m_pDbMySql)
			delete m_pDbMySql;
	}


	void DbTestBase::testOpen()
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

}


// Interfaces
// ----------
