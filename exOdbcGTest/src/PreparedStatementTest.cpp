/*!
* \file SqlHandleTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2014
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "PreparedStatementTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "PreparedStatement.h"
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

	void PreparedStatementTest::SetUp()
	{
		ASSERT_TRUE(g_odbcInfo.IsUsable());

		// Set up Env
		// Try to set to the ODBC v3 : We need that for the tests to run correct. 3.8 is not supported by all databases and we dont use specific stuff from it.
		// except for the TIME2 things, sql server specific. those tests can create their own env.
		m_pEnv->Init(OdbcVersion::V_3);

		// And database
		ASSERT_NO_THROW(m_pDb = OpenTestDb(m_pEnv));
	}

	TEST_F(PreparedStatementTest, Construct)
	{
		// Construct from valid and invalid database
		EXPECT_NO_THROW(PreparedStatement stmt(m_pDb, L"SELECT * FROM FOO"));

		DatabasePtr pClosed = make_shared<Database>(m_pEnv);
		EXPECT_THROW(PreparedStatement stmt(pClosed, L"SELECT * FROM FOO"), AssertionException);
	}


	TEST_F(PreparedStatementTest, BindParameter)
	{
		// Prepare to insert some ints
		// Determine query names from a Table
		ClearTmpTable(TableId::INTEGERTYPES_TMP);
		wstring tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		std::vector<SqlCBufferVariant> columns = iTable.CreateAutoColumnBuffers(false);
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

		PreparedStatement ps(m_pDb, ws.str());
		// Bind all params
		SQLUSMALLINT columnNr = 1;
		for (it = columns.begin(); it != columns.end(); ++it)
		{
			BindParamVisitor pv(columnNr, ps);
			boost::apply_visitor(pv, *it);
			++columnNr;
		}
		EXPECT_EQ(1, 1);
		// Insert some values
		// And execute multiple times
		SqlSLongBuffer idCol = boost::get<SqlSLongBuffer>(columns[0]);
		for (int i = 200; i < 210; i++)
		{
			idCol.SetValue(i);
			ps.Execute();
		}
		m_pDb->CommitTrans();
	}

} // namespace exodbc
