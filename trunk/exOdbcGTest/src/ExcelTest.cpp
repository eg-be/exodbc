/*!
* \file ExcelTest.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 15.03.2015
* \copyright wxWindows Library Licence, Version 3.1
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "ExcelTest.h"

// Same component headers

// Other headers
#include "Database.h"
#include "Table.h"

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

namespace exodbc
{

	void ExcelTest::SetUp()
	{
		// Ensure an Excel-DSN is set on the command line
		ASSERT_TRUE( ! g_excelDsn.empty());

		// Set up is called for every test
		ASSERT_NO_THROW(m_env.AllocateEnvironmentHandle());
		ASSERT_NO_THROW(m_env.SetOdbcVersion(OV_3));
	}


	void ExcelTest::TearDown()
	{

	}


	TEST_F(ExcelTest, OpenDatabase)
	{
		Database db;
		ASSERT_NO_THROW(db.AllocateConnectionHandle(m_env));
		// We cannot set Commit-mode on excel
		EXPECT_NO_THROW(db.Open(g_excelDsn, L"", L""));
	}


	TEST_F(ExcelTest, FindTables)
	{
		Database db;
		ASSERT_NO_THROW(db.AllocateConnectionHandle(m_env));
		ASSERT_NO_THROW(db.Open(L"exodbc_xls", L"", L""));
		std::vector<STableInfo> tables;
		ASSERT_NO_THROW(tables = db.FindTables(L"", L"", L"", L""));
		// Must contain our sheet 'TestTable$'
		bool foundTestTableSheet = false;
		std::vector<STableInfo>::const_iterator it;
		for (it = tables.begin(); it != tables.end(); it++)
		{
			if (it->m_tableName == L"TestTable$")
			{
				foundTestTableSheet = true;
			}
		}
		EXPECT_TRUE(foundTestTableSheet);
	}


	TEST_F(ExcelTest, OpenAutoTableAsWChar)
	{
		Database db;
		ASSERT_NO_THROW(db.AllocateConnectionHandle(m_env));
		ASSERT_NO_THROW(db.Open(L"exodbc_xls", L"", L""));
		// Create Table
		Table tTable(db, L"TestTable$", L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(tTable.SetAutoBindingMode(AutoBindingMode::BIND_ALL_AS_WCHAR));
		EXPECT_NO_THROW(tTable.Open(db));
		// Opening works, but selecting does not
	}


	TEST_F(ExcelTest, SelectManualWCharValues)
	{
		Database db;
		ASSERT_NO_THROW(db.AllocateConnectionHandle(m_env));
		ASSERT_NO_THROW(db.Open(L"exodbc_xls", L"", L""));
		// Create Table: Create it manually, because we need a special Query-statement:
		// Excel reports the Tablename when searching as 'TestTable$', but in sql-queries
		// it needs '[TestTable$]'. We provide a very primitive implementation in the
		// STableInfo to do so.
		// Find the correct table:
		STableInfo tableInfo;
		ASSERT_NO_THROW(tableInfo = db.FindOneTable(L"TestTable$", L"", L"", L""));
		// Update the special query name in there:
		tableInfo.SetSpecialSqlQueryName(L"[TestTable$]");
		// And create the manual table:
		Table tTable(db, 5, tableInfo, AF_READ);
		SQLWCHAR id[512];
		SQLWCHAR ic[512];
		SQLWCHAR fc[512];
		SQLWCHAR tc[512];
		SQLWCHAR mx[512];
		ASSERT_NO_THROW(tTable.SetColumn(0, L"ID", id, SQL_C_WCHAR, sizeof(id)));
		ASSERT_NO_THROW(tTable.SetColumn(1, L"Int", ic, SQL_C_WCHAR, sizeof(id)));
		ASSERT_NO_THROW(tTable.SetColumn(2, L"Float", fc, SQL_C_WCHAR, sizeof(id)));
		ASSERT_NO_THROW(tTable.SetColumn(3, L"Text", tc, SQL_C_WCHAR, sizeof(id)));
		ASSERT_NO_THROW(tTable.SetColumn(4, L"Mixed", mx, SQL_C_WCHAR, sizeof(id)));

		// \todo: This leaks memory if we pass the default flags? if we pass flags TF_NONE it does not leak? whats wrong? -> See Ticket #110 for reason
		// \todo: Do not check existence while opening, it would overwrite our manually set SpecialSqlQueryName -> See Ticket #111 
		ASSERT_NO_THROW(tTable.Open(db, TOF_NONE));
			
		EXPECT_NO_THROW(tTable.Select(L""));
		int rowCount = 0;
		while (tTable.SelectNext())
		{
			rowCount++;
			if ((rowCount >= 4 && rowCount <= 5) || (rowCount >= 11 && rowCount <= 14))
			{
				// \todo: Excel fails on mixed types, see Ticket #109
				EXPECT_TRUE(tTable.IsColumnNull(4));
				LogLevelFatal llf;
				DontDebugBreak ddb;
			}
			else
			{
				EXPECT_FALSE(tTable.IsColumnNull(4));
			}
		}
		EXPECT_EQ(15, rowCount);
	}


	TEST_F(ExcelTest, SelectAutoWCharValues)
	{
		Database db;
		ASSERT_NO_THROW(db.AllocateConnectionHandle(m_env));
		ASSERT_NO_THROW(db.Open(L"exodbc_xls", L"", L""));
		// Create Table: Create it manually, because we need a special Query-statement:
		// Excel reports the Tablename when searching as 'TestTable$', but in sql-queries
		// it needs '[TestTable$]'. We provide a very primitive implementation in the
		// STableInfo to do so.
		// Find the correct table:
		STableInfo tableInfo;
		ASSERT_NO_THROW(tableInfo = db.FindOneTable(L"TestTable$", L"", L"", L""));
		// Update the special query name in there:
		tableInfo.SetSpecialSqlQueryName(L"[TestTable$]");
		// And create the auto table:
		Table tTable(db, tableInfo, AF_READ);
		ASSERT_NO_THROW(tTable.Open(db, TOF_NONE));

		// Select all Rows
		std::wstring id, ic, fc, tc, mx;
		tTable.Select(L"");
		int rowCount = 0;
		while (tTable.SelectNext())
		{
			rowCount++;
			EXPECT_NO_THROW(tTable.GetColumnValue(0, id));
			EXPECT_NO_THROW(tTable.GetColumnValue(1, ic));
			EXPECT_NO_THROW(tTable.GetColumnValue(2, fc));
			EXPECT_NO_THROW(tTable.GetColumnValue(3, tc));
			if ((rowCount >= 4 && rowCount <= 5) || (rowCount >= 11 && rowCount <= 14))
			{
				// \todo: Excel fails on mixed types, see Ticket #109
				EXPECT_TRUE(tTable.IsColumnNull(4));
				LogLevelFatal llf;
				DontDebugBreak ddb;
				EXPECT_THROW(tTable.GetColumnValue(4, mx), AssertionException);
			}
			else
			{
				EXPECT_FALSE(tTable.IsColumnNull(4));
				EXPECT_NO_THROW(tTable.GetColumnValue(4, mx));
			}
		}

		EXPECT_NO_THROW(tTable.Select(L""));

	}

	// Interfaces
	// ----------

} //namespace exodbc
