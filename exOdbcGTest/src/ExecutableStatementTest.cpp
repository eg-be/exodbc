/*!
* \file ExecutableStatementTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2014
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "ExecutableStatementTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "ExecutableStatement.h"
#include "SqlCBufferVisitors.h"

// Debug
#include "DebugNew.h"

using namespace exodbctest;
using namespace std;

namespace exodbc
{
	// Static consts
	// -------------

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------

	void ExecutableStatementTest::SetUp()
	{
		ASSERT_TRUE(g_odbcInfo.IsUsable());

		// Set up Env
		// Try to set to the ODBC v3 : We need that for the tests to run correct. 3.8 is not supported by all databases and we dont use specific stuff from it.
		// except for the TIME2 things, sql server specific. those tests can create their own env.
		m_pEnv->Init(OdbcVersion::V_3);

		// And database
		ASSERT_NO_THROW(m_pDb = OpenTestDb(m_pEnv));
	}

	TEST_F(ExecutableStatementTest, Construct)
	{
		// Construct from valid and invalid database
		EXPECT_NO_THROW(ExecutableStatement stmt(m_pDb, L"SELECT * FROM FOO"));

		DatabasePtr pClosed = make_shared<Database>(m_pEnv);
		EXPECT_THROW(ExecutableStatement stmt(pClosed, L"SELECT * FROM FOO"), AssertionException);
	}


	TEST_F(ExecutableStatementTest, ExecDirectSelectValues)
	{
		// Prepare to select some values
		// Determine query names from a Table
		wstring idColName = GetIdColumnName(TableId::INTEGERTYPES);
		wstring tableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		std::vector<ColumnBufferPtrVariant> columns = iTable.CreateAutoColumnBufferPtrs(false);
		ASSERT_EQ(4, columns.size());

		// Build a select stmt
		wstring queryTableName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		wstringstream ws;
		ws << L"SELECT ";
		auto it = columns.begin();
		QueryNameVisitor qv;
		while (it != columns.end())
		{
			ws << boost::apply_visitor(qv, *it);
			++it;
			if (it != columns.end())
			{
				ws << L", ";
			}
		}
		ws << L" FROM " << queryTableName << L" ORDER BY " << idColName;

		// and bind columns
		ExecutableStatement ds(m_pDb);
		for (size_t i = 0; i < columns.size(); ++i)
		{
			ds.BindColumn(columns[i], (SQLUSMALLINT) i + 1);
		}

		// Execute statement
		ds.ExecuteDirect(ws.str());

		// And iterate through records (test only first 3)
		LongColumnBufferPtr pIdCol = boost::get<LongColumnBufferPtr>(columns[0]);
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(1, *pIdCol);
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(2, *pIdCol);
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(3, *pIdCol);
	}


	TEST_F(ExecutableStatementTest, SelectValuesUsingPreparedWhere)
	{
		// Prepare to select some values
		// Determine query names from a Table
		wstring idColName = GetIdColumnName(TableId::INTEGERTYPES);
		wstring tableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		std::vector<ColumnBufferPtrVariant> columns = iTable.CreateAutoColumnBufferPtrs(false);
		ASSERT_EQ(4, columns.size());

		// Build a select stmt
		wstring queryTableName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		wstringstream ws;
		ws << L"SELECT ";
		auto it = columns.begin();
		QueryNameVisitor qv;
		while (it != columns.end())
		{
			ws << boost::apply_visitor(qv, *it);
			++it;
			if (it != columns.end())
			{
				ws << L", ";
			}
		}
		ws << L" FROM " << queryTableName << L" WHERE " << idColName << L" = ?";

		// and bind columns
		ExecutableStatement ps(m_pDb);
		ps.Prepare(ws.str());
		for (size_t i = 0; i < columns.size(); ++i)
		{
			ps.BindColumn(columns[i], (SQLUSMALLINT)i + 1);
		}
		// And params
		LongColumnBufferPtr pIdCol = boost::get<LongColumnBufferPtr>(columns[0]);
		LongColumnBufferPtr pIntCol = boost::get<LongColumnBufferPtr>(columns[2]);
		ps.BindParameter(pIdCol, 1);

		// Try to select NULL column. no data is expected
		pIdCol->SetNull();

		// Execute statement
		ps.ExecutePrepared();

		EXPECT_FALSE(ps.SelectNext());

		// But not if we set some value on id
		pIdCol->SetValue(4);
		ps.ExecutePrepared();

		// Expect to have our record
		EXPECT_TRUE(ps.SelectNext());
		EXPECT_EQ(4, *pIdCol);
		EXPECT_EQ(2147483647, *pIntCol);

		EXPECT_FALSE(ps.SelectNext());
	}


	TEST_F(ExecutableStatementTest, InsertValues)
	{
		// Prepare to insert some values
		// Determine query names from a Table
		ClearTmpTable(TableId::INTEGERTYPES_TMP);
		wstring tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		std::vector<ColumnBufferPtrVariant> columns = iTable.CreateAutoColumnBufferPtrs(false);
		ASSERT_EQ(4, columns.size());

		wstring queryTableName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		wstringstream ws;
		ws << L"INSERT INTO " << queryTableName << L" (";
		auto it = columns.begin();
		QueryNameVisitor qv;
		while (it != columns.end())
		{
			ws << boost::apply_visitor(qv, *it);
			++it;
			if (it != columns.end())
			{
				ws << L", ";
			}
		}
		ws << L") VALUES(?, ?, ?, ?)";

		ExecutableStatement ps(m_pDb);
		ps.Prepare(ws.str());
		// Bind all params
		SQLUSMALLINT columnNr = 1;
		for (it = columns.begin(); it != columns.end(); ++it)
		{
			ps.BindParameter(*it, columnNr);
			++columnNr;
		}
		EXPECT_EQ(1, 1);
		// Insert some values
		// And execute multiple times
		LongColumnBufferPtr pIdCol = boost::get<LongColumnBufferPtr>(columns[0]);
		for (int i = 200; i < 210; i++)
		{
			pIdCol->SetValue(i);
			ps.ExecutePrepared();
		}
		m_pDb->CommitTrans();

		// try to read 10 records now
		iTable.Open();
		EXPECT_EQ(10, iTable.Count());
	}


	TEST_F(ExecutableStatementTest, DeleteValues)
	{

	}


	TEST_F(ExecutableStatementTest, UpdateValues)
	{

	}


} // namespace exodbc
