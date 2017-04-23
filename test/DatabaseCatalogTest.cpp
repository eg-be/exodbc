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


	TEST_F(DatabaseCatalogTest, SearchTables)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// we should find some tables if not restricting anything
		TableInfosVector allTables = dbCat.SearchTables();
		EXPECT_TRUE(allTables.size() > 0);

		// now restrict to tables named '%tmp' - we should have some in our test-db:
		// that should be less than all tables:
		TableInfosVector tmpTables = dbCat.SearchTables(u8"%tmp");
		EXPECT_GT(allTables.size(), tmpTables.size());
	}


	TEST_F(DatabaseCatalogTest, SearchTablesByName)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// we should find some tables if not restricting anything
		TableInfosVector allTables = dbCat.SearchTablesByName();
		EXPECT_TRUE(allTables.size() > 0);

		// now restrict to tables named '%tmp' - we should have some in our test-db:
		// that should be less than all tables:
		TableInfosVector tmpTables = dbCat.SearchTablesByName(u8"%tmp");
		EXPECT_GT(allTables.size(), tmpTables.size());

		// theoretically, setting catalog name and schema name to nullptr, should
		// be equal to calling them with '%', if pattern value arguments are used:
		TableInfosVector allTables2 = dbCat.SearchTables();
		EXPECT_EQ(allTables.size(), allTables2.size());
	}

} //namespace exodbc
