/*!
* \file ExecutableStatementTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2014
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

// Own header
#include "ExecutableStatementTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "exodbc/ExecutableStatement.h"
#include "exodbc/ColumnBufferVisitors.h"
#include "exodbc/Table.h"

// Debug
#include "DebugNew.h"

using namespace exodbc;
using namespace std;

namespace exodbctest
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
		EXPECT_NO_THROW(ExecutableStatement stmt(m_pDb, u8"SELECT * FROM FOO"));

		DatabasePtr pClosed = make_shared<Database>(m_pEnv);
		EXPECT_THROW(ExecutableStatement stmt(pClosed, u8"SELECT * FROM FOO"), AssertionException);

		// Construct using default c'tor
		ExecutableStatement stmt2;
		EXPECT_NO_THROW(stmt2.Init(m_pDb, false));
		
		// Fail to init twice
		EXPECT_THROW(stmt2.Init(m_pDb, false), AssertionException);
	}


	TEST_F(ExecutableStatementTest, Reset)
	{
		// Construct using default c'tor
		ExecutableStatement stmt;
		EXPECT_NO_THROW(stmt.Init(m_pDb, false));

		// Fail to init twice
		EXPECT_THROW(stmt.Init(m_pDb, false), AssertionException);

		// But not if we reset first
		EXPECT_NO_THROW(stmt.Reset());

		EXPECT_NO_THROW(stmt.Init(m_pDb, false));
	}


	TEST_F(ExecutableStatementTest, ExecDirectSelectValues)
	{
		// Prepare to select some values
		// Determine query names from a Table
		string idColName = GetIdColumnName(TableId::INTEGERTYPES);
		string tableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		std::vector<ColumnBufferPtrVariant> columns = iTable.CreateAutoColumnBufferPtrs(false, false, false);
		ASSERT_EQ(4, columns.size());

		// Build a select stmt
		string queryTableName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		stringstream ws;
		ws << u8"SELECT ";
		auto it = columns.begin();
		QueryNameVisitor qv;
		while (it != columns.end())
		{
			ws << boost::apply_visitor(qv, *it);
			++it;
			if (it != columns.end())
			{
				ws << u8", ";
			}
		}
		ws << u8" FROM " << queryTableName << u8" ORDER BY " << idColName;

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
		string idColName = GetIdColumnName(TableId::INTEGERTYPES);
		string tableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		std::vector<ColumnBufferPtrVariant> columns = iTable.CreateAutoColumnBufferPtrs(false, false, false);
		ASSERT_EQ(4, columns.size());

		// Build a select stmt
		string queryTableName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		stringstream ws;
		ws << u8"SELECT ";
		auto it = columns.begin();
		QueryNameVisitor qv;
		while (it != columns.end())
		{
			ws << boost::apply_visitor(qv, *it);
			++it;
			if (it != columns.end())
			{
				ws << u8", ";
			}
		}
		ws << u8" FROM " << queryTableName << u8" WHERE " << idColName << u8" = ?";

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


	TEST_F(ExecutableStatementTest, SelectFirst)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES);
		string tableQueryName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		string idName = GetIdColumnName(TableId::INTEGERTYPES);

		LongColumnBufferPtr pIdCol = LongColumnBuffer::Create(idName, SQL_UNKNOWN_TYPE);

		string sqlsmt = boost::str(boost::format(u8"SELECT %s FROM %s WHERE %s >= 2 ORDER BY %s ASC") %idName %tableQueryName %idName %idName );
		ExecutableStatement ds(m_pDb);
		ds.BindColumn(pIdCol, 1);
		ds.ExecuteDirect(sqlsmt);

		// We expect some Record that is not the one with id 2 if we move forward a few times
		ds.SelectNext();
		ds.SelectNext();
		ds.SelectNext();

		ASSERT_NE(2, *pIdCol);

		// now Select the first again using SelectFirst
		// we must have the record with id 2
		EXPECT_TRUE(ds.SelectFirst());
		EXPECT_EQ(2, *pIdCol);
	}


	TEST_F(ExecutableStatementTest, SelectLast)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES);
		string tableQueryName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		string idName = GetIdColumnName(TableId::INTEGERTYPES);

		LongColumnBufferPtr pIdCol = LongColumnBuffer::Create(idName, SQL_UNKNOWN_TYPE);

		string sqlsmt = boost::str(boost::format(u8"SELECT %s FROM %s WHERE %s >= 2 ORDER BY %s ASC") % idName %tableQueryName %idName %idName);
		ExecutableStatement ds(m_pDb);
		ds.BindColumn(pIdCol, 1);
		ds.ExecuteDirect(sqlsmt);

		// Directly select the last record
		EXPECT_TRUE(ds.SelectLast());
		EXPECT_EQ(7, *pIdCol);
	}


	TEST_F(ExecutableStatementTest, SelectNext)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES);
		string tableQueryName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		string idName = GetIdColumnName(TableId::INTEGERTYPES);

		LongColumnBufferPtr pIdCol = LongColumnBuffer::Create(idName, SQL_UNKNOWN_TYPE);

		string sqlsmt = boost::str(boost::format(u8"SELECT %s FROM %s WHERE %s >= 2 ORDER BY %s ASC") % idName %tableQueryName %idName %idName);
		ExecutableStatement ds(m_pDb);
		ds.BindColumn(pIdCol, 1);
		ds.ExecuteDirect(sqlsmt);

		// Before any select cursor is not modified
		EXPECT_TRUE(pIdCol->IsNull());

		// Select some records until we reach the end
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(2, *pIdCol);
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(3, *pIdCol);
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(4, *pIdCol);
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(5, *pIdCol);
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(6, *pIdCol);
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(7, *pIdCol);
		
		EXPECT_FALSE(ds.SelectNext());
	}


	TEST_F(ExecutableStatementTest, SelectPrev)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES);
		string tableQueryName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		string idName = GetIdColumnName(TableId::INTEGERTYPES);

		LongColumnBufferPtr pIdCol = LongColumnBuffer::Create(idName, SQL_UNKNOWN_TYPE);

		string sqlsmt = boost::str(boost::format(u8"SELECT %s FROM %s WHERE %s >= 2 ORDER BY %s ASC") % idName %tableQueryName %idName %idName);
		ExecutableStatement ds(m_pDb);
		ds.BindColumn(pIdCol, 1);
		ds.ExecuteDirect(sqlsmt);

		// Before any select cursor is not modified
		EXPECT_TRUE(pIdCol->IsNull());

		// Select some records
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(2, *pIdCol);
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(3, *pIdCol);
		// and move back again
		EXPECT_TRUE(ds.SelectPrev());
		EXPECT_EQ(2, *pIdCol);
		EXPECT_FALSE(ds.SelectPrev());
	}


	TEST_F(ExecutableStatementTest, SelectAbsolute)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES);
		string tableQueryName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		string idName = GetIdColumnName(TableId::INTEGERTYPES);

		LongColumnBufferPtr pIdCol = LongColumnBuffer::Create(idName, SQL_UNKNOWN_TYPE);

		string sqlsmt = boost::str(boost::format(u8"SELECT %s FROM %s WHERE %s >= 2 ORDER BY %s ASC") % idName %tableQueryName %idName %idName);
		ExecutableStatement ds(m_pDb);
		ds.BindColumn(pIdCol, 1);
		ds.ExecuteDirect(sqlsmt);

		// Before any select cursor is not modified
		EXPECT_TRUE(pIdCol->IsNull());

		// Select the 3rd record
		EXPECT_TRUE(ds.SelectAbsolute(3));
		EXPECT_EQ(4, *pIdCol);

		// and the 6th
		EXPECT_TRUE(ds.SelectAbsolute(6));
		EXPECT_EQ(7, *pIdCol);
	}


	TEST_F(ExecutableStatementTest, SelectRelative)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES);
		string tableQueryName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		string idName = GetIdColumnName(TableId::INTEGERTYPES);

		LongColumnBufferPtr pIdCol = LongColumnBuffer::Create(idName, SQL_UNKNOWN_TYPE);

		string sqlsmt = boost::str(boost::format(u8"SELECT %s FROM %s WHERE %s >= 2 ORDER BY %s ASC") % idName %tableQueryName %idName %idName);
		ExecutableStatement ds(m_pDb);
		ds.BindColumn(pIdCol, 1);
		ds.ExecuteDirect(sqlsmt);

		// Before any select cursor is not modified
		EXPECT_TRUE(pIdCol->IsNull());

		// Note: For MySQL to be able to select relative, a record must be selected first. Else, SelectRelative will choose wrong offset
		EXPECT_TRUE(ds.SelectNext());
		EXPECT_EQ(2, *pIdCol);

		// Move by one forward
		EXPECT_TRUE(ds.SelectRelative(1));
		EXPECT_EQ(3, *pIdCol);

		// Move by one forward
		EXPECT_TRUE(ds.SelectRelative(1));
		EXPECT_EQ(4, *pIdCol);

		// And one back again, check complete record
		EXPECT_TRUE(ds.SelectRelative(-1));
		EXPECT_EQ(3, *pIdCol);

		// Select something not in result set
		EXPECT_FALSE(ds.SelectRelative(20));
	}


	TEST_F(ExecutableStatementTest, WriteValues)
	{
		// Prepare to insert some values
		// Determine query names from a Table
		ClearTmpTable(TableId::INTEGERTYPES_TMP);
		string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		std::vector<ColumnBufferPtrVariant> columns = iTable.CreateAutoColumnBufferPtrs(false, false, false);
		ASSERT_EQ(4, columns.size());

		string queryTableName = PrependSchemaOrCatalogName(m_pDb->GetDbms(), tableName);
		stringstream ws;
		ws << u8"INSERT INTO " << queryTableName << u8" (";
		auto it = columns.begin();
		QueryNameVisitor qv;
		while (it != columns.end())
		{
			ws << boost::apply_visitor(qv, *it);
			++it;
			if (it != columns.end())
			{
				ws << u8", ";
			}
		}
		ws << u8") VALUES(?, ?, ?, ?)";

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
} // namespace exodbctest
