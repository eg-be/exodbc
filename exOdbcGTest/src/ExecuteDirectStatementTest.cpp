/*!
* \file ExecuteDirectStatementTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 27.11.2014
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "ExecuteDirectStatementTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "ExecuteDirectStatement.h"
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

	void ExecuteDirectStatementTest::SetUp()
	{
		ASSERT_TRUE(g_odbcInfo.IsUsable());

		// Set up Env
		// Try to set to the ODBC v3 : We need that for the tests to run correct. 3.8 is not supported by all databases and we dont use specific stuff from it.
		// except for the TIME2 things, sql server specific. those tests can create their own env.
		m_pEnv->Init(OdbcVersion::V_3);

		// And database
		ASSERT_NO_THROW(m_pDb = OpenTestDb(m_pEnv));
	}

	TEST_F(ExecuteDirectStatementTest, Construct)
	{
		// Construct from valid and invalid database
		EXPECT_NO_THROW(ExecuteDirectStatement stmt(m_pDb));

		DatabasePtr pClosed = make_shared<Database>(m_pEnv);
		EXPECT_THROW(ExecuteDirectStatement stmt(pClosed), AssertionException);
	}


	TEST_F(ExecuteDirectStatementTest, SelectValues)
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

		// Create statement to execute
		// and bind columns
		ExecuteDirectStatement ds(m_pDb);
		for (size_t i = 0; i < columns.size(); ++i)
		{
			ds.BindColumn(columns[i], (SQLUSMALLINT)i + 1);
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

} // namespace exodbc
