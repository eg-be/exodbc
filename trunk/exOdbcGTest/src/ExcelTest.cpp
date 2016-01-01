/*!
* \file ExcelTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 15.03.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "ExcelTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

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
using namespace exodbctest;

namespace exodbc
{

	void ExcelTest::SetUp()
	{
		// Set up is called for every test
		ASSERT_NO_THROW(m_pEnv->Init(OdbcVersion::V_3));
	}


	void ExcelTest::TearDown()
	{

	}


	//TEST_F(ExcelTest, OpenDatabase)
	//{
	//	Database db;
	//	ASSERT_NO_THROW(db.Init(m_pEnv));
	//	// We cannot set Commit-mode on excel
	//	if (g_odbcInfo.HasConnectionString())
	//	{
	//		EXPECT_NO_THROW(db.Open(g_odbcInfo.m_connectionString));
	//	}
	//	else
	//	{
	//		EXPECT_NO_THROW(db.Open(g_odbcInfo.m_dsn, L"", L""));
	//	}
	//}


	//TEST_F(ExcelTest, FindTables)
	//{
	//	DatabasePtr pDb = OpenTestDb(m_pEnv);
	//	TableInfosVector tables;
	//	ASSERT_NO_THROW(tables = pDb->FindTables(L"", L"", L"", L""));
	//	// Must contain our sheet 'TestTable$'
	//	bool foundTestTableSheet = false;
	//	TableInfosVector::const_iterator it;
	//	for (it = tables.begin(); it != tables.end(); it++)
	//	{
	//		if (it->GetPureName() == L"TestTable$")
	//		{
	//			foundTestTableSheet = true;
	//		}
	//	}
	//	EXPECT_TRUE(foundTestTableSheet);
	//}


	//TEST_F(ExcelTest, OpenAutoTableAsWChar)
	//{
	//	DatabasePtr pDb = OpenTestDb(m_pEnv);
	//	// Create Table
	//	Table tTable(pDb, TableAccessFlag::AF_READ, L"TestTable$");
	//	tTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharSql2BufferMap()));
	//	EXPECT_NO_THROW(tTable.Open());
	//	// Opening works, but selecting does not
	//}


	//TEST_F(ExcelTest, SpecialQueryNameWorkaround)
	//{
	//	// See Ticket #111 - this is fixed and no workarounds are needed
	//	DatabasePtr pDb = OpenTestDb(m_pEnv);
	//	Table tTable(pDb, TableAccessFlag::AF_READ, L"TestTable$");
	//	// Note that excel reports wired datatypes, doubles for ints (1.0000000 instead of 1), etc., so for the tests use chars
	//	tTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharSql2BufferMap()));
	//	EXPECT_NO_THROW(tTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

	//	// Query
	//	EXPECT_NO_THROW(tTable.Select());
	//	EXPECT_TRUE(tTable.SelectNext());
	//	std::wstring id, i, f, t;
	//	EXPECT_NO_THROW(id = tTable.GetWString(0));
	//	EXPECT_NO_THROW(i = tTable.GetWString(1));
	//	EXPECT_NO_THROW(f = tTable.GetWString(2));
	//	EXPECT_NO_THROW(t = tTable.GetWString(3));
	//	
	//	EXPECT_EQ(L"1.0", id);
	//	EXPECT_EQ(L"101.0", i);
	//	EXPECT_EQ(L"1.1111", f);
	//	EXPECT_EQ(L"row1", t);

	//	// No need to set a special query-name using [TestTable$], the Table will handle that during Open()
	//	TableInfo tableInfo;
	//	ASSERT_NO_THROW(tableInfo = pDb->FindOneTable(L"TestTable$", L"", L"", L""));

	//	Table tTable2(pDb, TableAccessFlag::AF_READ, tableInfo);
	//	tTable2.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharSql2BufferMap()));
	//	EXPECT_NO_THROW(tTable2.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

	//	// Query
	//	EXPECT_NO_THROW(tTable2.Select());
	//	EXPECT_TRUE(tTable2.SelectNext());
	//	EXPECT_NO_THROW(id = tTable.GetWString(0));
	//	EXPECT_NO_THROW(i = tTable.GetWString(1));
	//	EXPECT_NO_THROW(f = tTable.GetWString(2));
	//	EXPECT_NO_THROW(t = tTable.GetWString(3));

	//	EXPECT_EQ(L"1.0", id);
	//	EXPECT_EQ(L"101.0", i);
	//	EXPECT_EQ(L"1.1111", f);
	//	EXPECT_EQ(L"row1", t);
	//}


	//TEST_F(ExcelTest, SelectManualWCharValues)
	//{
	//	DatabasePtr pDb = OpenTestDb(m_pEnv);
	//	// Find the correct table:
	//	TableInfo tableInfo;
	//	ASSERT_NO_THROW(tableInfo = pDb->FindOneTable(L"TestTable$", L"", L"", L""));
	//	// No need to set a special query-name using [TestTable$], the Table will handle that during Open()
	//	// And create the manual table:
	//	Table tTable(pDb, TableAccessFlag::AF_READ, tableInfo);
	//	SQLWCHAR id[512];
	//	SQLWCHAR ic[512];
	//	SQLWCHAR fc[512];
	//	SQLWCHAR tc[512];
	//	SQLWCHAR mx[512];
	//	ASSERT_NO_THROW(tTable.SetColumn(0, L"ID", SQL_VARCHAR, id, SQL_C_WCHAR, sizeof(id), ColumnFlag::CF_SELECT));
	//	ASSERT_NO_THROW(tTable.SetColumn(1, L"Int", SQL_VARCHAR, ic, SQL_C_WCHAR, sizeof(id), ColumnFlag::CF_SELECT));
	//	ASSERT_NO_THROW(tTable.SetColumn(2, L"Float", SQL_VARCHAR, fc, SQL_C_WCHAR, sizeof(id), ColumnFlag::CF_SELECT));
	//	ASSERT_NO_THROW(tTable.SetColumn(3, L"Text", SQL_VARCHAR, tc, SQL_C_WCHAR, sizeof(id), ColumnFlag::CF_SELECT));
	//	ASSERT_NO_THROW(tTable.SetColumn(4, L"Mixed", SQL_VARCHAR, mx, SQL_C_WCHAR, sizeof(id), ColumnFlag::CF_SELECT));

	//	// \todo: Do not check existence while opening, it would overwrite our manually set SpecialSqlQueryName -> See Ticket #111 
	//	ASSERT_NO_THROW(tTable.Open(TableOpenFlag::TOF_NONE));
	//		
	//	EXPECT_NO_THROW(tTable.Select(L""));
	//	int rowCount = 0;
	//	while (tTable.SelectNext())
	//	{
	//		rowCount++;
	//		if ((rowCount >= 4 && rowCount <= 5) || (rowCount >= 11 && rowCount <= 14))
	//		{
	//			// \todo: Excel fails on mixed types, see Ticket #109
	//			EXPECT_TRUE(tTable.IsColumnNull(4));
	//			LogLevelFatal llf;
	//			DontDebugBreak ddb;
	//		}
	//		else
	//		{
	//			EXPECT_FALSE(tTable.IsColumnNull(4));
	//		}
	//	}
	//	EXPECT_EQ(15, rowCount);
	//}


	//TEST_F(ExcelTest, SelectAutoWCharValues)
	//{
	//	DatabasePtr pDb = OpenTestDb(m_pEnv);
	//	// Find the correct table:
	//	TableInfo tableInfo;
	//	ASSERT_NO_THROW(tableInfo = pDb->FindOneTable(L"TestTable$", L"", L"", L""));
	//	// And create the auto table:
	//	Table tTable(pDb, TableAccessFlag::AF_READ, tableInfo);
	//	ASSERT_NO_THROW(tTable.Open(TableOpenFlag::TOF_NONE));

	//	// Select all Rows
	//	std::wstring id, ic, fc, tc, mx;
	//	tTable.Select(L"");
	//	int rowCount = 0;
	//	while (tTable.SelectNext())
	//	{
	//		rowCount++;
	//		EXPECT_NO_THROW(id = tTable.GetWString(0));
	//		EXPECT_NO_THROW(ic = tTable.GetWString(1));
	//		EXPECT_NO_THROW(fc = tTable.GetWString(2));
	//		EXPECT_NO_THROW(tc = tTable.GetWString(3));
	//		if ((rowCount >= 4 && rowCount <= 5) || (rowCount >= 11 && rowCount <= 14))
	//		{
	//			// \todo: Excel fails on mixed types, see Ticket #109
	//			EXPECT_TRUE(tTable.IsColumnNull(4));
	//			LogLevelFatal llf;
	//			DontDebugBreak ddb;
	//			EXPECT_THROW(mx = tTable.GetWString(4), NullValueException);
	//		}
	//		else
	//		{
	//			EXPECT_FALSE(tTable.IsColumnNull(4));
	//			EXPECT_NO_THROW(mx = tTable.GetWString(4));
	//		}
	//	}

	//	EXPECT_NO_THROW(tTable.Select(L""));
	//}


	// Interfaces
	// ----------

} //namespace exodbc
