/*!
* \file ExcelTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 15.03.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

// Own header
#include "ExcelTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "exodbc/Database.h"
#include "exodbc/Table.h"

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

	void ExcelTest::SetUp()
	{
		ASSERT_TRUE(g_odbcInfo.IsUsable());

		// Set up Env
		m_pEnv = Environment::Create(OdbcVersion::V_3);

		// And database
		m_pDb = OpenTestDb(m_pEnv);
	}


	void ExcelTest::TearDown()
	{

	}


	bool ExcelTest::IsExcelDb()
	{
		return m_pDb->GetDbms() == DatabaseProduct::EXCEL;
	}


	TEST_F(ExcelTest, FindTables)
	{
		if (!IsExcelDb())
			return;
		
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();
		TableInfoVector tables;
		ASSERT_NO_THROW(tables = pDbCat->SearchTables(u8"%"));
		// Must contain our sheet 'TestTable$'
		bool foundTestTableSheet = false;
		TableInfoVector::const_iterator it;
		for (it = tables.begin(); it != tables.end(); it++)
		{
			if (it->GetPureName() == u8"TestTable$")
			{
				foundTestTableSheet = true;
			}
		}
		EXPECT_TRUE(foundTestTableSheet);
	}


	TEST_F(ExcelTest, OpenAutoTableAsWChar)
	{
		if (!IsExcelDb())
			return;

		// Create Table
		Table tTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, u8"TestTable$");
		tTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharSql2BufferMap()));
		EXPECT_NO_THROW(tTable.Open());
	}


	TEST_F(ExcelTest, SpecialQueryNameWorkaround)
	{
		if (!IsExcelDb())
			return;

		// See Ticket #111 - this is fixed and no workarounds are needed
		Table tTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, u8"TestTable$");
		// Note that excel reports wired datatypes, doubles for ints (1.0000000 instead of 1), etc., so for the tests use chars
		tTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharSql2BufferMap()));
		EXPECT_NO_THROW(tTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		// Query
		EXPECT_NO_THROW(tTable.Select());
		EXPECT_TRUE(tTable.SelectNext());
		auto idCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(0);
		auto intCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(1);
		auto floatCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(2);
		auto textCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(3);
		
		EXPECT_EQ(L"1.0", idCol->GetWString());
		EXPECT_EQ(L"101.0", intCol->GetWString());
		EXPECT_EQ(L"1.1111", floatCol->GetWString());
		EXPECT_EQ(L"row1", textCol->GetWString());

		// No need to set a special query-name using [TestTable$], the Table will handle that during Open()
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();
		TableInfo tableInfo;
		ASSERT_NO_THROW(tableInfo = pDbCat->FindOneTable(u8"TestTable$", u8"", u8"", u8""));

		Table tTable2(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableInfo);
		tTable2.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharSql2BufferMap()));
		EXPECT_NO_THROW(tTable2.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		// Query
		EXPECT_NO_THROW(tTable2.Select());
		EXPECT_TRUE(tTable2.SelectNext());
		idCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(0);
		intCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(1);
		floatCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(2);
		textCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(3);

		EXPECT_EQ(L"1.0", idCol->GetWString());
		EXPECT_EQ(L"101.0", intCol->GetWString());
		EXPECT_EQ(L"1.1111", floatCol->GetWString());
		EXPECT_EQ(L"row1", textCol->GetWString());
	}


	TEST_F(ExcelTest, SelectManualWCharValues)
	{
		if (!IsExcelDb())
			return;

		// Find the correct table:
		TableInfo tableInfo;
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();
		ASSERT_NO_THROW(tableInfo = pDbCat->FindOneTable(u8"TestTable$", u8"", u8"", u8""));
		// No need to set a special query-name using [TestTable$], the Table will handle that during Open()
		// And create the manual table:
		Table tTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableInfo);
		SQLWCHAR id[512];
		SQLWCHAR ic[512];
		SQLWCHAR fc[512];
		SQLWCHAR tc[512];
		SQLWCHAR mx[512];
		ASSERT_NO_THROW(tTable.SetColumn(0, u8"ID", SQL_VARCHAR, id, SQL_C_WCHAR, sizeof(id), ColumnFlag::CF_SELECT));
		ASSERT_NO_THROW(tTable.SetColumn(1, u8"Int", SQL_VARCHAR, ic, SQL_C_WCHAR, sizeof(id), ColumnFlag::CF_SELECT));
		ASSERT_NO_THROW(tTable.SetColumn(2, u8"Float", SQL_VARCHAR, fc, SQL_C_WCHAR, sizeof(id), ColumnFlag::CF_SELECT));
		ASSERT_NO_THROW(tTable.SetColumn(3, u8"Text", SQL_VARCHAR, tc, SQL_C_WCHAR, sizeof(id), ColumnFlag::CF_SELECT));
		ASSERT_NO_THROW(tTable.SetColumn(4, u8"Mixed", SQL_VARCHAR, mx, SQL_C_WCHAR, sizeof(id), ColumnFlag::CF_SELECT));

		ASSERT_NO_THROW(tTable.Open());
			
		EXPECT_NO_THROW(tTable.Select(u8""));
		int rowCount = 0;
		while (tTable.SelectNext())
		{
			rowCount++;
			if ((rowCount >= 4 && rowCount <= 5) || (rowCount >= 11 && rowCount <= 14))
			{
				// \todo: Excel fails on mixed types, see Ticket #109
				EXPECT_TRUE(tTable.IsColumnNull(4));
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
		if (!IsExcelDb())
			return;

		// Find the correct table:
		TableInfo tableInfo;
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();
		ASSERT_NO_THROW(tableInfo = pDbCat->FindOneTable(u8"TestTable$", u8"", u8"", u8""));
		// And create the auto table:
		Table tTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableInfo);
		tTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharSql2BufferMap()));
		ASSERT_NO_THROW(tTable.Open(TableOpenFlag::TOF_NONE));

		// Select all Rows
		auto idCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(0);
		auto intCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(1);
		auto floatCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(2);
		auto textCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(3);
		auto mixedCol = tTable.GetColumnBufferPtr<WCharColumnBufferPtr>(4);

		std::wstring id, ic, fc, tc, mx;
		tTable.Select(u8"");
		int rowCount = 0;
		while (tTable.SelectNext())
		{
			rowCount++;
			if ((rowCount >= 4 && rowCount <= 5) || (rowCount >= 11 && rowCount <= 14))
			{
				// \todo: Excel fails on mixed types, see Ticket #109
				EXPECT_TRUE(tTable.IsColumnNull(4));
				EXPECT_THROW(mixedCol->GetWString(), NullValueException);
			}
			else
			{
				EXPECT_FALSE(tTable.IsColumnNull(4));
				EXPECT_NO_THROW(mixedCol->GetWString());
			}
		}

		EXPECT_NO_THROW(tTable.Select(u8""));
	}

} //namespace exodbctest
