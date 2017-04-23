/*!
 * \file DatabaseCatalogTest.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 20.04.2017
 * \copyright GNU Lesser General Public License Version 3
 * 
 * [Brief CPP-file description]
 */ 

// Own header
#include "DatabaseCatalogTest.h"

// Same component headers
#include "ManualTestTables.h"
#include "exOdbcGTestHelpers.h"

// Other headers
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/DatabaseCatalog.h"

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
using namespace exodbc;

namespace exodbctest
{
	void DatabaseCatalogTest::SetUp()
	{
		// Set up is called for every test
		m_pEnv->Init(OdbcVersion::V_3);
		m_pDb = OpenTestDb(m_pEnv);

	}


	void DatabaseCatalogTest::TearDown()
	{
		if(m_pDb->IsOpen())
		{
			m_pDb->Close();
		}
	}


	TEST_F(DatabaseCatalogTest, Init)
	{
		DatabaseCatalog dbCat;

		// Fail for not allocated or connected connection handle
		{
			SqlDbcHandlePtr pDbc = SqlDbcHandle::Create(m_pEnv->GetSqlEnvHandle());
			LogLevelSetter ll(LogLevel::None);
			EXPECT_THROW(dbCat.Init(pDbc, SqlInfoProperties()), AssertionException);
		}

		// But succeed for valid connection
		EXPECT_NO_THROW(dbCat.Init(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties()));
	}


	TEST_F(DatabaseCatalogTest, FindPatternValue)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		TableInfosVector tables = dbCat.FindTables()
	}

} //namespace exodbc
