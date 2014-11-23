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
#include "boost/any.hpp"

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
		ASSERT_TRUE(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
	}

	void TableTest::TearDown()
	{
		if (m_db.IsOpen())
		{
			// Microsoft Sql Server needs a CommitTrans()
			if (m_db.Dbms() == dbmsMS_SQL_SERVER)
			{
				EXPECT_TRUE(m_db.CommitTrans());
			}
			EXPECT_TRUE(m_db.Close());
		}
	}

	// Open
	// ----
	TEST_P(TableTest, OpenManualWithoutCheck)
	{
		// Open a table without checking for privileges or existence
		IntTypesTable table(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(table.Open(false, false));
	}

	TEST_P(TableTest, OpenManualCheckExistence)
	{
		// Open a table with checking for existence
		IntTypesTable table(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(table.Open(false, true));

		// Open a non-existing table
		LOG_ERROR(L"Warning: This test is supposed to spit errors");
		NotExistingTable neTable(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_FALSE(neTable.Open(false, true));
	}

	TEST_P(TableTest, OpenAutoCheckExistence)
	{
		std::wstring tableName = TestTables::GetTableName(L"integertypes", m_odbcInfo.m_namesCase);
		exodbc::Table table(&m_db, tableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(table.Open(false, true));
		bool ok = table.Select(L"");
		int p = 3;
	}

	// Count
	// -----
	TEST_P(TableTest, Count)
	{
		FloatTypesTable table(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(table.Open(false, true));

		size_t all;
		EXPECT_TRUE(table.Count(L"", all));
		EXPECT_EQ(6, all);

		// TODO: We need some GetColumnName function for things like that
		size_t some;
		std::wstring whereStmt = L"tdouble > 0";
		if (m_db.Dbms() == dbmsDB2)
		{
			whereStmt = L"TDOUBLE > 0";
		}
		EXPECT_TRUE(table.Count(whereStmt, some));
		EXPECT_EQ(1, some);
		whereStmt = L"tdouble > 0 OR tfloat > 0";
		if (m_db.Dbms() == dbmsDB2)
		{
			whereStmt = L"TDOUBLE > 0 OR TFLOAT > 0";
		}
		EXPECT_TRUE(table.Count(whereStmt, some));
		EXPECT_EQ(2, some);
	}

	// GetNext
	// -------

	// Read
	// ----


// Interfaces
// ----------

} // namespace exodbc
