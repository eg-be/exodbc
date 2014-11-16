/*!
 * \file DbTableTest.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "TableTest.h"

// Same component headers
#include "exOdbcGTest.h"
#include "GenericTestTables.h"

// Other headers
#include "Environment.h"
#include "Database.h"

// Debug
#include "DebugNew.h"

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
using namespace std;
//using namespace exodbc;

namespace exodbc
{

	void TableTest::SetUp()
	{
		// Called for every unit-test
		m_odbcInfo = GetParam();
		
		// Set up Env
		ASSERT_TRUE(m_env.AllocHenv());
		ASSERT_TRUE(m_env.SetOdbcVersion(OV_3));

		// And database
		ASSERT_TRUE(m_db.AllocateHdbc(m_env));

//		RecordProperty("DSN", eli::w2mb(m_odbcInfo.m_dsn));
		//m_pEnv = new Environment(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password);
		//HENV henv = m_pEnv->GetHenv();
		//ASSERT_TRUE(henv  != 0);

		//m_pDb = new Database(m_pEnv);
		//ASSERT_TRUE(m_pDb->Open(m_pEnv));
	}

	void TableTest::TearDown()
	{
		//if(m_pDb)
		//{
		//	// Why do we need to commit with DB2? We did not start anything??
		//	m_pDb->CommitTrans();

		//	m_pDb->Close();
		//	delete m_pDb;
		//}
		//if(m_pEnv)
		//	delete m_pEnv;

		//m_pDb = NULL;
		//m_pEnv = NULL;
	}

	// Open
	// ----
	TEST_P(TableTest, Open)
	{
	}

	// GetNext
	// -------

	// Read
	// ----


// Interfaces
// ----------

} // namespace exodbc
