/*!
 * \file Db2DbTest.cpp
 * \author Elias Gerber <eg@zame.ch>
 * \date 23.02.2014
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
#include "Db2DbTest.h"

// Same component headers
#include "Db2Params.h"
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
namespace DB2_10_05
{


	void DbTest::setUp()
	{
		m_dsn = DB2_DSN;
		DbTestBase::setUp(m_dsn);
	}


	void DbTest::tearDown()
	{
		DbTestBase::tearDown();
		m_dsn = L"";
	}

}



namespace DB2
{
	void DbTestBase::setUp(const std::wstring& dsn)
	{
		// Create DbConnectInfs for various databases
		m_pConnectInfDb2 = new wxDbConnectInf(NULL, dsn, DB2_USER, DB2_PASS);
		m_pDbDb2 = new wxDb(m_pConnectInfDb2->GetHenv());
	}


	void DbTestBase::tearDown()
	{
		if(m_pConnectInfDb2)
			delete m_pConnectInfDb2;
		if(m_pDbDb2)
			delete m_pDbDb2;
	}


	void DbTestBase::testOpen()
	{
		// Open with failing on unsupported datatypes
		CPPUNIT_ASSERT( m_pDbDb2->Open(m_pConnectInfDb2, true) );

		// Try to open one with not only forward-cursors
		wxDb* pdbDb2BC = new wxDb(m_pConnectInfDb2->GetHenv(), false);
		try
		{
			CPPUNIT_ASSERT( pdbDb2BC->Open(m_pConnectInfDb2, true));
		}
		catch(CPPUNIT_NS::Exception e)
		{
			delete pdbDb2BC;
			throw e;
		}
		delete pdbDb2BC;
	}

}


// Interfaces
// ----------
