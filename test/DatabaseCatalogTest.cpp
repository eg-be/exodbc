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


	TEST_F(DatabaseCatalogTest, EscapePatternValueArguments)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		string pattern = u8"Foo%Ba_r";
		string escaped = dbCat.EscapePatternValueArguments(pattern);
		
		string escSeq = dbCat.GetSearchPatternEscape();
		string exp = boost::str(boost::format(u8"Foo%s%%Ba%s_r") % escSeq % escSeq);
		EXPECT_EQ(exp, escaped);
	}


	TEST_F(DatabaseCatalogTest, SearchTables)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// we should find some tables if not restricting anything
		TableInfosVector allTables = dbCat.SearchTables(u8"%");
		EXPECT_TRUE(allTables.size() > 0);

		// now restrict to tables named '%tmp' - we should have some in our test-db:
		// that should be less than all tables:
		TableInfosVector tmpTables = dbCat.SearchTables(u8"%tmp");
		EXPECT_GT(allTables.size(), tmpTables.size());
	}


	TEST_F(DatabaseCatalogTest, SearchTablesByTableName)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// we should find some tables if not restricting anything
		TableInfosVector allTables = dbCat.SearchTables(u8"%");
		EXPECT_TRUE(allTables.size() > 0);

		// now restrict to tables named '%tmp' - we should have some in our test-db:
		// that should be less than all tables:
		TableInfosVector tmpTables = dbCat.SearchTables(u8"%tmp");
		EXPECT_GT(allTables.size(), tmpTables.size());

		// theoretically, setting catalog name and schema name to nullptr, should
		// be equal to calling them with '%', if pattern value arguments are used,
		// but at least on sql server we get way more tables, so just test for >=
		TableInfosVector allTables2 = dbCat.SearchTables(u8"%");
		EXPECT_GE(allTables2.size(), allTables.size());
	}


	TEST_F(DatabaseCatalogTest, SearchPatternEscape)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// find some tmp table ending with '_tmp'
		string esc = dbCat.GetSearchPatternEscape();
		EXPECT_FALSE(esc.empty());

		string tableNamePattern = boost::str(boost::format(u8"%%%s_tmp") % esc);
		if (g_odbcInfo.m_namesCase == Case::UPPER)
			boost::algorithm::to_upper(tableNamePattern);
		TableInfosVector tmpTables = dbCat.SearchTables(tableNamePattern);
		EXPECT_FALSE(tmpTables.empty());

		// every table must end with _tmp
		for (TableInfosVector::const_iterator it = tmpTables.begin(); it != tmpTables.end(); ++it)
		{
			string tableName = it->GetPureName();
			EXPECT_TRUE(boost::algorithm::iends_with(tableName, u8"_tmp"));
		}
	}


	TEST_F(DatabaseCatalogTest, SearchTablesByDetectingSchemaOrCatalog)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// find some table
		string tableNamePattern = u8"integertypes";
		if (g_odbcInfo.m_namesCase == Case::UPPER)
		{
			boost::algorithm::to_upper(tableNamePattern);
		}
		TableInfosVector tables = dbCat.SearchTables(tableNamePattern);
		ASSERT_FALSE(tables.empty());

		// can only do test if table has either catalog or schema only
		TableInfo ti = tables.front();
		TableInfosVector tables2;
		if ((ti.HasSchema() && !ti.HasCatalog()) || (!ti.HasSchema() && ti.HasCatalog()))
		{
			string schemaOrCatalogName = ti.HasSchema() ? ti.GetSchema() : ti.GetCatalog();
			tables2 = dbCat.SearchTables(ti.GetPureName(), schemaOrCatalogName, ti.GetType());
		}
		else
		{
			// cannot do test
			LOG_INFO(u8"Skipping test because not either schema or catalog was found");
			return;
		}
		ASSERT_EQ(1, tables2.size());
		EXPECT_EQ(ti, tables.front());
	}


	TEST_F(DatabaseCatalogTest, SearchTablesBySettingSchemaOrCatalog)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// find some table
		string tableNamePattern = u8"integertypes";
		if (g_odbcInfo.m_namesCase == Case::UPPER)
			boost::algorithm::to_upper(tableNamePattern);
		TableInfosVector tables = dbCat.SearchTables(tableNamePattern);
		ASSERT_FALSE(tables.empty());
		
		TableInfo ti = tables.front();
		TableInfosVector tables2;
		if(ti.HasCatalog())
		{
			tables2 = dbCat.SearchTables(ti.GetPureName(), ti.GetCatalog(), false, ti.GetType());
		}
		else if (ti.HasSchema())
		{
			tables2 = dbCat.SearchTables(ti.GetPureName(), ti.GetSchema(), true, ti.GetType());
		}
		else
		{
			// cannot do test
			LOG_INFO(u8"Skipping test because nor schema or catalog was found");
			return;
		}
		ASSERT_EQ(1, tables2.size());
		EXPECT_EQ(ti, tables2.front());
	}


	TEST_F(DatabaseCatalogTest, SearchTablesBySchemaAndCatalog)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// find some table
		string tableNamePattern = u8"integertypes";
		if (g_odbcInfo.m_namesCase == Case::UPPER)
			boost::algorithm::to_upper(tableNamePattern);
		TableInfosVector tables = dbCat.SearchTables(tableNamePattern);
		ASSERT_FALSE(tables.empty());

		TableInfo ti = tables.front();
		TableInfosVector tables2;
		if (ti.HasCatalog() && ti.HasSchema())
		{
			tables2 = dbCat.SearchTables(ti.GetPureName(), ti.GetSchema(), ti.GetCatalog(), u8"");
		}
		else
		{
			// cannot do test
			LOG_INFO(u8"Skipping test because not schema and catalog was found");
			return;
		}
		ASSERT_EQ(1, tables2.size());
		EXPECT_EQ(ti, tables2.front());
	}

} //namespace exodbc
