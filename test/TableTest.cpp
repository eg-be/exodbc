/*!
 * \file TableTest.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
 * 
 * [Brief CPP-file description]
 */ 

// Own header
#include "TableTest.h"

// Same component headers
#include "exOdbcTest.h"
#include "ManualTestTables.h"
#include "exOdbcTestHelpers.h"

// Other headers
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/Exception.h"
#include "exodbc/ColumnBufferVisitors.h"

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
	void TableTest::SetUp()
	{
		ASSERT_TRUE(g_odbcInfo.IsUsable());

		// Set up Env
		// Try to set to the ODBC v3 : We need that for the tests to run correct. 3.8 is not supported by all databases and we dont use specific stuff from it.
		// except for the TIME2 things, sql server specific. those tests can create their own env.
		m_pEnv->Init(OdbcVersion::V_3);

		// And database
		ASSERT_NO_THROW(m_pDb = OpenTestDb(m_pEnv));
	}


	void TableTest::TearDown()
	{
		if (m_pDb->IsOpen())
		{
			EXPECT_NO_THROW(m_pDb->Close());
		}
	}
	

	TEST_F(TableTest, Init)
	{
		// Fail if we try to init a table with a not open database
		DatabasePtr pDb = Database::Create(m_pEnv);
		Table table;
		EXPECT_THROW(table.Init(pDb, TableAccessFlag::AF_READ, TableInfo()), AssertionException);

		// but succeed later
		EXPECT_NO_THROW(table.Init(m_pDb, TableAccessFlag::AF_READ, TableInfo()));
	}


	// Open / Close
	// ------------
	TEST_F(TableTest, OpenNoAccessFlags)
	{
		// Opening a table without access flags should not bind any columns at all
		// but reading a TableInfo from it must work.
		string intTableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_NONE, intTableName);
		EXPECT_NO_THROW(iTable.Open());

		// TableInfo must be available
		TableInfo ti = iTable.GetTableInfo();
		EXPECT_EQ(intTableName, ti.GetName());

		// But no columns have been created
		EXPECT_EQ(0, iTable.GetColumnBufferCount());
	}


	TEST_F(TableTest, OpenCountOnly)
	{
		// Opening a table for COUNT only
		string intTableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_COUNT_WHERE, intTableName);
		EXPECT_NO_THROW(iTable.Open());

		// TableInfo must be available
		TableInfo ti = iTable.GetTableInfo();
		EXPECT_EQ(intTableName, ti.GetName());

		// But no columns have been created
		EXPECT_EQ(0, iTable.GetColumnBufferCount());

		// But we are able to COUNT:
		SQLUBIGINT c1 = iTable.Count();
		EXPECT_GT(c1, 0);
		SQLUBIGINT c2 = iTable.Count(boost::str(boost::format(u8"%s > 0") % GetIdColumnName(TableId::INTEGERTYPES)));
		EXPECT_EQ(c1, c2);
	}


	TEST_F(TableTest, OpenManualReadOnlyWithoutCheck)
	{
		// Open an existing table without checking for privileges or existence
		MIntTypesTable table(m_pDb);
		EXPECT_NO_THROW(table.Open(TableOpenFlag::TOF_NONE));

		// If we pass in the TableInfo directly we should also be able to "open"
		// a totally non-sense table:
		TableInfo neTableInfo(u8"NotExisting", u8"", u8"", u8"", u8"");
		Table neTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, neTableInfo);
		SQLINTEGER idNotExisting = 0;
		neTable.SetColumn(0, u8"idNotExistring", SQL_INTEGER, &idNotExisting, SQL_C_SLONG, sizeof(idNotExisting), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		EXPECT_NO_THROW(neTable.Open(TableOpenFlag::TOF_NONE));
		// \todo So we can prove in the test that we will fail doing a SELECT later
	}


	TEST_F(TableTest, OpenManualWritableWithoutCheckPrivOrExist)
	{
		// Open an existing table without checking for privileges or existence
		Table iTable(m_pDb, TableAccessFlag::AF_READ_WRITE, GetTableName(TableId::INTEGERTYPES));
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		SQLINTEGER bii = 0;
		int type = SQL_C_SLONG;
		iTable.SetColumn(0, ToDbCase(u8"idintegertypes"), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), ColumnFlag::CF_SELECT | ColumnFlag::CF_INSERT | ColumnFlag::CF_PRIMARY_KEY);
		iTable.SetColumn(1, ToDbCase(u8"tsmallint"), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), ColumnFlag::CF_SELECT | ColumnFlag::CF_INSERT | ColumnFlag::CF_UPDATE);
		iTable.SetColumn(2, ToDbCase(u8"tint"), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), ColumnFlag::CF_SELECT);
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			// access has no bigint
			iTable.SetColumn(3, ToDbCase(u8"tbigint"), SQL_INTEGER, &bii, SQL_C_SLONG, sizeof(bii), ColumnFlag::CF_SELECT | ColumnFlag::CF_INSERT);
		}
		else
		{
			iTable.SetColumn(3, ToDbCase(u8"tbigint"), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), ColumnFlag::CF_SELECT | ColumnFlag::CF_INSERT);
		}
		EXPECT_NO_THROW(iTable.Open(TableOpenFlag::TOF_NONE));
	}


	TEST_F(TableTest, OpenAutoDefaultCtr)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table table;
		EXPECT_NO_THROW(table.Init(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName, u8"", u8"", u8""));
		EXPECT_NO_THROW(table.Open());
	}


	TEST_F(TableTest, OpenManualDefaultCtr)
	{
		// Open an existing manual table for reading
		std::string intTypesTableName = GetTableName(TableId::INTEGERTYPES);

		Table iTable;
		iTable.Init(m_pDb, TableAccessFlag::AF_READ, intTypesTableName);
		// Set Columns
		SQLINTEGER id;
		SQLSMALLINT tSmallint;
		SQLINTEGER tInt;
		SQLBIGINT tBigint;

		iTable.SetColumn(0, ToDbCase(u8"idintegertypes"), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		iTable.SetColumn(1, ToDbCase(u8"tsmallint"), SQL_INTEGER, &tSmallint, SQL_C_SSHORT, sizeof(tSmallint), ColumnFlag::CF_SELECT);
		iTable.SetColumn(2, ToDbCase(u8"tint"), SQL_INTEGER, &tInt, SQL_C_SLONG, sizeof(tInt), ColumnFlag::CF_SELECT);
		iTable.SetColumn(3, ToDbCase(u8"tbigint"), SQL_INTEGER, &tBigint, SQL_C_SBIGINT, sizeof(tBigint), ColumnFlag::CF_SELECT);
		
		EXPECT_NO_THROW(iTable.Open());
	}


	TEST_F(TableTest, CopyCtr)
	{
		// Create a table
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table t1(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName);

		// Create a copy of that table
		Table c1(t1);
		EXPECT_EQ(t1.m_initialTableName, c1.m_initialTableName);
		EXPECT_EQ(t1.m_initialSchemaName, c1.m_initialSchemaName);
		EXPECT_EQ(t1.m_initialCatalogName, c1.m_initialCatalogName);
		EXPECT_EQ(t1.m_initialTypeName, c1.m_initialTypeName);
		EXPECT_EQ(t1.GetAccessFlags(), c1.GetAccessFlags());

		// If we open the table..
		c1.Open();

		// and create another copy from it..
		Table c2(c1);
		EXPECT_EQ(t1.m_initialTableName, c1.m_initialTableName);
		EXPECT_EQ(t1.m_initialSchemaName, c1.m_initialSchemaName);
		EXPECT_EQ(t1.m_initialCatalogName, c1.m_initialCatalogName);
		EXPECT_EQ(t1.m_initialTypeName, c1.m_initialTypeName);
		EXPECT_EQ(t1.GetAccessFlags(), c1.GetAccessFlags());
		// .. we should already have the table info on the copy
		EXPECT_TRUE(c2.HasTableInfo());
		EXPECT_EQ(c1.GetTableInfo(), c2.GetTableInfo());
	}


	TEST_F(TableTest, OpenManualCheckExistence)
	{
		// Open a table with checking for existence
		MIntTypesTable table(m_pDb);
		EXPECT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		// Open a non-existing table
		{
			LOG_ERROR(u8"Warning: This test is supposed to spit errors");
			MNotExistingTable neTable(m_pDb);
			EXPECT_THROW(neTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE), exodbc::Exception);
		}
	}


	TEST_F(TableTest, OpenManualPrimaryKeys)
	{
		// Open a table by defining primary keys manually
		Table iTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE | TableAccessFlag::AF_DELETE_PK, GetTableName(TableId::INTEGERTYPES_TMP), u8"", u8"", u8"");
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		iTable.SetColumn(0, ToDbCase(u8"idintegertypes"), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), ColumnFlag::CF_SELECT |ColumnFlag::CF_PRIMARY_KEY);
		iTable.SetColumn(1, ToDbCase(u8"tsmallint"), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), ColumnFlag::CF_SELECT);
		iTable.SetColumn(2, ToDbCase(u8"tint"), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), ColumnFlag::CF_SELECT);

		// Opening for writing must work
		EXPECT_NO_THROW(iTable.Open(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS));

		// But opening if primary keys are not defined must fail
		Table iTable2(m_pDb, TableAccessFlag::AF_SELECT_WHERE | TableAccessFlag::AF_DELETE_PK, GetTableName(TableId::INTEGERTYPES_TMP), u8"", u8"", u8"");
		SQLINTEGER id2 = 0;
		SQLSMALLINT si2 = 0;
		SQLINTEGER i2 = 0;
		SQLBIGINT bi2 = 0;
		iTable2.SetColumn(0, ToDbCase(u8"idintegertypes"), SQL_INTEGER, &id2, SQL_C_SLONG, sizeof(id2), ColumnFlag::CF_SELECT);
		iTable2.SetColumn(1, ToDbCase(u8"tsmallint"), SQL_INTEGER, &si2, SQL_C_SSHORT, sizeof(si2), ColumnFlag::CF_SELECT);
		iTable2.SetColumn(2, ToDbCase(u8"tint"), SQL_INTEGER, &i2, SQL_C_SLONG, sizeof(i2), ColumnFlag::CF_SELECT);
		EXPECT_THROW(iTable2.Open(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS), Exception);

		// But if we open for select only, we do not care about the primary keys
		Table iTable3(m_pDb, TableAccessFlag::AF_SELECT_WHERE, GetTableName(TableId::INTEGERTYPES_TMP), u8"", u8"", u8"");
		SQLINTEGER id3 = 0;
		SQLSMALLINT si3 = 0;
		SQLINTEGER i3 = 0;
		SQLBIGINT bi3 = 0;
		iTable3.SetColumn(0, ToDbCase(u8"idintegertypes"), SQL_INTEGER, &id3, SQL_C_SLONG, sizeof(id3), ColumnFlag::CF_SELECT);
		iTable3.SetColumn(1, ToDbCase(u8"tsmallint"), SQL_INTEGER, &si3, SQL_C_SSHORT, sizeof(si3), ColumnFlag::CF_SELECT);
		iTable3.SetColumn(2, ToDbCase(u8"tint"), SQL_INTEGER, &i3, SQL_C_SLONG, sizeof(i3), ColumnFlag::CF_SELECT);

		EXPECT_NO_THROW(iTable3.Open(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS));
	}


	TEST_F(TableTest, OpenAutoWithoutCheck)
	{
		// Open an auto-table without checking for privileges or existence
		// This makes only sense if we've already determined the correct TableInfo structure
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		TableInfo tableInfo;
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();
		ASSERT_NO_THROW(tableInfo = pDbCat->FindOneTable(tableName, u8"", u8"", u8""));

		exodbc::Table table(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableInfo);
		EXPECT_NO_THROW(table.Open(TableOpenFlag::TOF_NONE));

		// If we try to open an auto-table this will never work if you've passed invalid information:
		// As soon as the columns are searched, we expect to fail
		{
			LOG_ERROR(u8"Warning: This test is supposed to spit errors");
			TableInfo neTableInfo(u8"NotExisting", u8"", u8"", u8"", u8"");
			Table neTable(m_pDb, TableAccessFlag::AF_READ, neTableInfo);
			EXPECT_THROW(neTable.Open(TableOpenFlag::TOF_NONE), Exception);
		}
	}


	TEST_F(TableTest, OpenAutoCheckExistence)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table table(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName, u8"", u8"", u8"");
		EXPECT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		// Open a non-existing table
		{
			LOG_ERROR(u8"Warning: This test is supposed to spit errors");
			std::string neName = GetTableName(TableId::NOT_EXISTING);
			exodbc::Table neTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, neName, u8"", u8"", u8"");
			EXPECT_THROW(neTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE), Exception);
		}
	}


	TEST_F(TableTest, OpenManualWithUnsupportedColumn)
	{
		Table iTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, GetTableName(TableId::INTEGERTYPES), u8"", u8"", u8"");
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		int type = SQL_C_SLONG;
		
		string idName = GetIdColumnName(TableId::INTEGERTYPES);

		iTable.SetColumn(0, ToDbCase(u8"idintegertypes"), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), ColumnFlag::CF_SELECT);
		iTable.SetColumn(1, ToDbCase(u8"tsmallint"), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), ColumnFlag::CF_SELECT);
		iTable.SetColumn(2, ToDbCase(u8"tint"), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), ColumnFlag::CF_SELECT);
		// Set some silly type 30666 for the last column
		iTable.SetColumn(3, ToDbCase(u8"tbigint"), 30666, &bi, SQL_C_SBIGINT, sizeof(bi), ColumnFlag::CF_SELECT);
		// Expect to fail if we explicitly check the type info:
		EXPECT_THROW(iTable.Open(TableOpenFlag::TOF_CHECK_DB_TYPE_INFOS), Exception);
		// But not if we do not check
		EXPECT_NO_THROW(iTable.Open());
		iTable.Close();
		// Or also not if we check, but ask to skip unsupported:
		EXPECT_NO_THROW(iTable.Open(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS | TableOpenFlag::TOF_CHECK_DB_TYPE_INFOS));
	}


	TEST_F(TableTest, OpenAutoSkipUnsupportedColumn)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table nst(m_pDb, TableAccessFlag::AF_READ, tableName);

		// Remove support for SHORT and BIGINT
		Sql2BufferTypeMapPtr pTypeMap = m_pDb->GetSql2BufferTypeMap();
		pTypeMap->ClearType(SQL_BIGINT);
		pTypeMap->ClearType(SQL_SMALLINT);

		// Expect to fail if we open with default flags
		ASSERT_THROW(nst.Open(), NotSupportedException);
		// But not if we pass the flag to skip
		{
			ASSERT_NO_THROW(nst.Open(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// The table is: id, short, int, bigint
		// but we fail to read xml, so we have only indexes
		// 0, 1 which map to
		// id, int

		EXPECT_NO_THROW(nst.GetColumnBufferPtr<LongColumnBufferPtr>(0));
		EXPECT_NO_THROW(nst.GetColumnBufferPtr<LongColumnBufferPtr>(1));

		EXPECT_THROW(nst.GetColumnBufferPtr<LongColumnBufferPtr>(2), IllegalArgumentException);
	}


	TEST_F(TableTest, SelectFromAutoWithSkippedUnsupportedColumn)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		std::string idColName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table nst(m_pDb, TableAccessFlag::AF_READ, tableName);

		// Remove support for SHORT and BIGINT
		Sql2BufferTypeMapPtr pTypeMap = m_pDb->GetSql2BufferTypeMap();
		pTypeMap->ClearType(SQL_BIGINT);
		pTypeMap->ClearType(SQL_SMALLINT);

		// we do not fail to open if we pass the flag to skip
		{
			EXPECT_NO_THROW(nst.Open(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// The table is: id, short, int, bigint
		// but we fail to read xml, so we have only indexes
		// 0, 1 which map to
		// id, int
		LongColumnBufferPtr pId = nst.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pInt = nst.GetColumnBufferPtr<LongColumnBufferPtr>(1);

		EXPECT_THROW(nst.GetColumnBufferPtr<LongColumnBufferPtr>(2), IllegalArgumentException);

		EXPECT_NO_THROW(nst.Select(boost::str(boost::format(u8"%s = 4") % idColName)));
		EXPECT_TRUE(nst.SelectNext());
		EXPECT_EQ(4, pId->GetValue());
		EXPECT_EQ(2147483647, pInt->GetValue());
	}


	TEST_F(TableTest, Close)
	{
		// Create table
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName, u8"", u8"", u8"");

		// Close when not open must fail
		{
			EXPECT_THROW(iTable.Close(), AssertionException);
		}

		// Open a Table read-only
		ASSERT_NO_THROW(iTable.Open());

		// Close must work
		EXPECT_NO_THROW(iTable.Close());

		// Close an already closed table
		{
			EXPECT_THROW(iTable.Close(), AssertionException);
		}
	}


	TEST_F(TableTest, OpenAndCloseAndOpenAndClose)
	{
		// Create a Table, open for reading
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName, u8"", u8"", u8"");
		ASSERT_NO_THROW(iTable.Open());

		// Close Table
		EXPECT_NO_THROW(iTable.Close());

		// Open again for reading
		EXPECT_NO_THROW(iTable.Open());

		// And close again
		EXPECT_NO_THROW(iTable.Close());
	}


	TEST_F(TableTest, SetAccessFlags)
	{
		// Create a Table, for reading only
		std::string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName, u8"", u8"", u8"");

		// Before opening it, change the mode, then open
		ASSERT_NO_THROW(iTable.SetAccessFlags(TableAccessFlag::AF_READ_WRITE));
		// Do not forget to set the primary keys if this is an Access database
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		ASSERT_NO_THROW(iTable.Open());

		// We cannot change the mode if the table is open
		{
			EXPECT_THROW(iTable.SetAccessFlags(TableAccessFlag::AF_READ), AssertionException);
		}

		// But when we close first we can
		EXPECT_NO_THROW(iTable.Close());
		EXPECT_NO_THROW(iTable.SetAccessFlags(TableAccessFlag::AF_READ));
		// And we can open it again
		EXPECT_NO_THROW(iTable.Open());
	}


	TEST_F(TableTest, IsQueryOnly)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName, u8"", u8"", u8"");
		
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(TableAccessFlag::AF_UPDATE_PK);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(TableAccessFlag::AF_UPDATE_PK);
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(TableAccessFlag::AF_INSERT);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(TableAccessFlag::AF_INSERT);
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(TableAccessFlag::AF_DELETE_WHERE);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(TableAccessFlag::AF_DELETE_WHERE);
		EXPECT_TRUE(iTable.IsQueryOnly());

		// and one that is initially rw
		exodbc::Table iTable2(m_pDb, TableAccessFlag::AF_READ_WRITE, tableName, u8"", u8"", u8"");
		EXPECT_FALSE(iTable2.IsQueryOnly());
		iTable2.ClearAccessFlag(TableAccessFlag::AF_INSERT);
		EXPECT_FALSE(iTable2.IsQueryOnly());

		iTable2.ClearAccessFlag(TableAccessFlag::AF_DELETE_PK);
		EXPECT_FALSE(iTable2.IsQueryOnly());

		iTable2.ClearAccessFlag(TableAccessFlag::AF_UPDATE_PK);
		iTable2.ClearAccessFlag(TableAccessFlag::AF_UPDATE_WHERE);
		iTable2.ClearAccessFlag(TableAccessFlag::AF_DELETE_WHERE);
		EXPECT_TRUE(iTable2.IsQueryOnly());

		// remove read, we are no longer query only
		iTable2.ClearAccessFlag(TableAccessFlag::AF_SELECT_WHERE);
		iTable2.ClearAccessFlag(TableAccessFlag::AF_SELECT_PK);
		EXPECT_FALSE(iTable2.IsQueryOnly());
	}


	TEST_F(TableTest, MissingAccessFlagsThrowOnWrite)
	{
		// Open a read-only table
		std::string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName, u8"", u8"", u8"");

		iTable.Open();

		// we cannot not insert, delete or update it
		{
			EXPECT_THROW(iTable.Insert(), AssertionException);
			EXPECT_THROW(iTable.DeleteByPkValues(), AssertionException);
			EXPECT_THROW(iTable.Delete(u8""), AssertionException);
			EXPECT_THROW(iTable.UpdateByPkValues(), AssertionException);
			EXPECT_THROW(iTable.Update(u8""), AssertionException);
		}

		// we test that writing works in all those cases where we open RW
	}


	// Select / GetNext
	// ----------------
	TEST_F(TableTest, Select)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName);
		ASSERT_NO_THROW(iTable.Open());

		EXPECT_NO_THROW(iTable.Select(u8""));

		iTable.SelectClose();
	}


	TEST_F(TableTest, SelectByPkValues)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_SELECT_PK, tableName);
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		iTable.Open();

		EXPECT_NO_THROW(iTable.SelectByPkValues());

		iTable.SelectClose();
	}


	TEST_F(TableTest, SelectOrder)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		std::string idColName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName);
		ASSERT_NO_THROW(iTable.Open());

		EXPECT_NO_THROW(iTable.Select(u8"", boost::str(boost::format(u8"%s ASC") % idColName)));

		// Test that we are ordered ascending
		SQLINTEGER prev = 0;
		LongColumnBufferPtr pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		while (iTable.SelectNext())
		{
			EXPECT_TRUE(prev < pIdCol->GetValue());
			prev = pIdCol->GetValue();
		}

		// and descending
		prev++;
		EXPECT_NO_THROW(iTable.Select(u8"", boost::str(boost::format(u8"%s DESC") % idColName)));
		while (iTable.SelectNext())
		{
			EXPECT_TRUE(prev > pIdCol->GetValue());
			prev = pIdCol->GetValue();
		}

	}


	TEST_F(TableTest, SelectFlag)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName);
		
		// remove the select flag from the id column before opening the table
		auto columns = iTable.CreateAutoColumnBufferPtrs(false, true, false);
		ColumnFlagsPtr pFlag = boost::apply_visitor(ColumnFlagsPtrVisitor(), columns[0]);
		auto idCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		pFlag->Clear(ColumnFlag::CF_SELECT);

		iTable.Open();

		EXPECT_NO_THROW(iTable.Select(u8""));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(idCol->IsNull());
	}


	TEST_F(TableTest, SelectFirst)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		std::string idName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open(TableOpenFlag::TOF_SCROLLABLE_CURSORS));

		LongColumnBufferPtr pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pIntCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		// We expect some Record that is not the one with id 2 if we move forward three times
		std::string sqlWhere = boost::str(boost::format(u8"%s >= 2 ORDER BY %s ASC") % idName % idName);
		ASSERT_NO_THROW(iTable.Select(sqlWhere));
		ASSERT_TRUE(iTable.SelectNext());
		ASSERT_TRUE(iTable.SelectNext());
		ASSERT_TRUE(iTable.SelectNext());

		ASSERT_NE(2, *pIdCol);

		// now Select the first again using SelectFirst
		// we must have the record with id 2
		EXPECT_TRUE(iTable.SelectFirst());
		EXPECT_EQ(2, *pIdCol);
		EXPECT_TRUE(pIntCol->IsNull());
	}


	TEST_F(TableTest, SelectLast)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		std::string idName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open(TableOpenFlag::TOF_SCROLLABLE_CURSORS));

		LongColumnBufferPtr pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pIntCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		// We expect some Record that is not the one with id 2 if we move away from that one
		std::string sqlWhere = boost::str(boost::format(u8"%s >= 2 ORDER BY %s ASC") % idName % idName);
		ASSERT_NO_THROW(iTable.Select(sqlWhere));
		ASSERT_TRUE(iTable.SelectNext());
		ASSERT_TRUE(iTable.SelectNext());
		ASSERT_TRUE(iTable.SelectNext());

		ASSERT_NE(2, *pIdCol);

		// now Select the last record
		EXPECT_TRUE(iTable.SelectLast());
		EXPECT_EQ(7, *pIdCol);
		EXPECT_EQ(26, *pIntCol);
	}


	TEST_F(TableTest, SelectAbsolute)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		std::string idName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open(TableOpenFlag::TOF_SCROLLABLE_CURSORS));

		LongColumnBufferPtr pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pIntCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		// Move the 3rd record in the result-set. 
		std::string sqlWhere = boost::str(boost::format(u8"%s >= 2 ORDER BY %s ASC") % idName % idName);
		ASSERT_NO_THROW(iTable.Select(sqlWhere));
		EXPECT_TRUE(iTable.SelectAbsolute(3));
		EXPECT_EQ(4, *pIdCol);
		EXPECT_EQ(2147483647, *pIntCol);

		// Select something not in result set
		EXPECT_FALSE(iTable.SelectAbsolute(20));
	}


	TEST_F(TableTest, SelectRelative)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		std::string idName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open(TableOpenFlag::TOF_SCROLLABLE_CURSORS));

		LongColumnBufferPtr pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pIntCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		// We expect some Records
		std::string sqlWhere = boost::str(boost::format(u8"%s >= 2 ORDER BY %s ASC") % idName % idName);
		ASSERT_NO_THROW(iTable.Select(sqlWhere));
		// Note: For MySQL to be able to select relative, a record must be selected first. Else, SelectRelative will choose wrong offset
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(2, *pIdCol);
		EXPECT_TRUE(pIntCol->IsNull());

		// Move by one forward
		EXPECT_TRUE(iTable.SelectRelative(1));
		EXPECT_EQ(3, *pIdCol);
		
		// Move by one forward
		EXPECT_TRUE(iTable.SelectRelative(1));
		EXPECT_EQ(4, *pIdCol);

		// And one back again, check complete record
		EXPECT_TRUE(iTable.SelectRelative(-1));
		EXPECT_EQ(3, *pIdCol);
		EXPECT_EQ((-2147483647 - 1), *pIntCol);

		// Select something not in result set
		EXPECT_FALSE(iTable.SelectRelative(20));
	}


	TEST_F(TableTest, SelectPrev)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		std::string idName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open(TableOpenFlag::TOF_SCROLLABLE_CURSORS));

		LongColumnBufferPtr pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pIntCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
		
		// We expect some Records
		std::string sqlWhere = boost::str(boost::format(u8"%s >= 2 ORDER BY %s ASC") % idName % idName);
		ASSERT_NO_THROW(iTable.Select(sqlWhere));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(2, *pIdCol);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(3, *pIdCol);
		EXPECT_EQ((-2147483647 - 1), *pIntCol);

		// now Select the prev record - we have the first again
		EXPECT_TRUE(iTable.SelectPrev());
		EXPECT_EQ(2, *pIdCol);
		EXPECT_TRUE(pIntCol->IsNull());

		// no more prev records available:
		EXPECT_FALSE(iTable.SelectPrev());
	}


	TEST_F(TableTest, SelectNext)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName);
		ASSERT_NO_THROW(iTable.Open());

		// We expect 7 Records
		iTable.Select(u8"");
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_FALSE(iTable.SelectNext());

		iTable.SelectClose();
	}


	TEST_F(TableTest, SelectNextFromPkValue)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_SELECT_PK, tableName);
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		
		iTable.Open();
		auto pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pIntCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
		pIdCol->SetValue(2);
		iTable.SelectByPkValues();
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(2, pIdCol->GetValue());
		EXPECT_TRUE(pIntCol->IsNull());
		pIdCol->SetValue(7);
		iTable.SelectByPkValues();
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(7, pIdCol->GetValue());
		EXPECT_EQ(26, pIntCol->GetValue());
	}


	TEST_F(TableTest, SelectClose)
	{
		std::string tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName);
		ASSERT_NO_THROW(iTable.Open());
		
		// Do something that opens a transaction
		iTable.Select(u8"");
		EXPECT_NO_THROW(iTable.SelectClose());
	}


	// Count
	// -----
	TEST_F(TableTest, Count)
	{
		Table table(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, GetTableName(TableId::FLOATTYPES));
		ASSERT_NO_THROW(table.Open());

		SQLUBIGINT all;
		EXPECT_NO_THROW(all = table.Count(u8""));
		EXPECT_EQ(6, all);

		// \todo: Check some really big table, with billions of rows
		SQLUBIGINT some;
		std::string whereStmt = u8"tdouble > 0";
		if (m_pDb->GetDbms() == DatabaseProduct::DB2)
		{
			whereStmt = u8"TDOUBLE > 0";
		}
		EXPECT_NO_THROW(some = table.Count(whereStmt));
		EXPECT_EQ(1, some);
		whereStmt = u8"tdouble > 0 OR tfloat > 0";
		if (m_pDb->GetDbms() == DatabaseProduct::DB2)
		{
			whereStmt = u8"TDOUBLE > 0 OR TFLOAT > 0";
		}
		EXPECT_NO_THROW(some = table.Count(whereStmt));
		EXPECT_EQ(2, some);

		// we should fail with an exception if we query using a non-sense where-stmt
		EXPECT_THROW(table.Count(u8"a query that is invalid for sure"), Exception);
	}


	// Insert rows
	// ---------
	TEST_F(TableTest, Insert)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		ClearTmpTable(TableId::INTEGERTYPES_TMP);

		{
			// First insert where all columns are bound
			Table iTable(m_pDb, TableAccessFlag::AF_INSERT, tableName);
			iTable.Open();

			// If we do not set a value for the primary key, fail
			EXPECT_THROW(iTable.Insert(), SqlResultException);

			// Set some values
			auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
			pId->SetValue(300);
			pInt->SetValue(400);

			EXPECT_NO_THROW(iTable.Insert());
		}
		{
			// Now insert with only id-column bound for inserting
			Table iTable(m_pDb, TableAccessFlag::AF_INSERT, tableName);
			iTable.CreateAutoColumnBufferPtrs(false, true, false);
			LongColumnBufferPtr pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			LongColumnBufferPtr pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
			pInt->Clear(ColumnFlag::CF_INSERT);
			iTable.Open();
			pId->SetValue(301);
			pInt->SetValue(401);
			iTable.Insert();
		}
		m_pDb->CommitTrans();

		// Read back values
		Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName);
		iTable.Open();
		auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		string idColName = GetIdColumnName(TableId::INTEGERTYPES_TMP);
		string sqlWhere = boost::str(boost::format(u8"%s = 300 OR %s = 301 ORDER by %s") % idColName %idColName %idColName);
		iTable.Select(sqlWhere);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(300, *pId);
		EXPECT_EQ(400, *pInt);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(301, *pId);
		EXPECT_TRUE(pInt->IsNull());
	}


	TEST_F(TableTest, InsertFlag)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		ClearTmpTable(TableId::INTEGERTYPES_TMP);

		{
			// Now insert with only id-column bound for inserting
			Table iTable(m_pDb, TableAccessFlag::AF_INSERT, tableName);
			iTable.CreateAutoColumnBufferPtrs(false, true, false);
			LongColumnBufferPtr pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			LongColumnBufferPtr pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
			pInt->Clear(ColumnFlag::CF_INSERT);
			iTable.Open();
			pId->SetValue(301);
			pInt->SetValue(401);
			iTable.Insert();
		}
		m_pDb->CommitTrans();

		// Read back values
		Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName);
		iTable.Open();
		auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		string idColName = GetIdColumnName(TableId::INTEGERTYPES_TMP);
		string sqlWhere = boost::str(boost::format(u8"%s = 301 ORDER by %s") %idColName %idColName);
		iTable.Select(sqlWhere);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(301, *pId);
		EXPECT_TRUE(pInt->IsNull());
	}


	// Update rows
	// ---------
	TEST_F(TableTest, UpdatePk)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		ClearTmpTable(TableId::INTEGERTYPES_TMP);

		{
			// Insert some rows
			Table iTable(m_pDb, TableAccessFlag::AF_INSERT, tableName);
			iTable.Open();

			// Set some values
			auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
			pId->SetValue(300);
			pInt->SetValue(400);
			iTable.Insert();

			pId->SetValue(301);
			pInt->SetValue(401);
			iTable.Insert();

			m_pDb->CommitTrans();
		}
		{
			// And update
			Table iTable(m_pDb, TableAccessFlag::AF_UPDATE_PK, tableName);
			iTable.Open();

			// Set some values
			auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
			
			pId->SetValue(300);
			pInt->SetValue(500);
			iTable.UpdateByPkValues();

			pId->SetValue(301);
			pInt->SetValue(501);
			iTable.UpdateByPkValues();
			
			m_pDb->CommitTrans();
		}

		// Read back values
		Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		iTable.Open();
		auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		string idColName = GetIdColumnName(TableId::INTEGERTYPES_TMP);
		string sqlWhere = boost::str(boost::format(u8"%s = 300 OR %s = 301 ORDER by %s") % idColName %idColName %idColName);
		iTable.Select(sqlWhere);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(300, *pId);
		EXPECT_EQ(500, *pInt);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(301, *pId);
		EXPECT_EQ(501, *pInt);
	}


	TEST_F(TableTest, UpdateFlag)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		ClearTmpTable(TableId::INTEGERTYPES_TMP);

		{
			// Insert some rows
			Table iTable(m_pDb, TableAccessFlag::AF_INSERT, tableName);
			iTable.Open();

			// Set some values
			auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
			pId->SetValue(300);
			pInt->SetValue(400);
			iTable.Insert();

			pId->SetValue(301);
			pInt->SetNull();
			iTable.Insert();

			m_pDb->CommitTrans();
		}
		{
			// And update, but remove the UPDATE_FLAG from the value we are trying to change
			Table iTable(m_pDb, TableAccessFlag::AF_UPDATE_PK, tableName);
			auto columns = iTable.CreateAutoColumnBufferPtrs(false, true, false);
			auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
			pInt->Clear(ColumnFlag::CF_UPDATE);
			iTable.Open();

			pId->SetValue(300);
			pInt->SetValue(500);
			iTable.UpdateByPkValues();

			pId->SetValue(301);
			pInt->SetValue(501);
			iTable.UpdateByPkValues();

			m_pDb->CommitTrans();
		}

		// Read back values - int col was not updated, because flag was removed
		Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		iTable.Open();
		auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		string idColName = GetIdColumnName(TableId::INTEGERTYPES_TMP);
		string sqlWhere = boost::str(boost::format(u8"%s = 300 OR %s = 301 ORDER by %s") % idColName %idColName %idColName);
		iTable.Select(sqlWhere);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(300, *pId);
		EXPECT_EQ(400, *pInt);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(301, *pId);
		EXPECT_TRUE(pInt->IsNull());
	}


	TEST_F(TableTest, UpdateWhere)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		string idColName = GetIdColumnName(TableId::INTEGERTYPES_TMP);
		ClearTmpTable(TableId::INTEGERTYPES_TMP);

		{
			// Insert some rows
			Table iTable(m_pDb, TableAccessFlag::AF_INSERT, tableName);
			iTable.Open();

			// Set some values
			auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
			pId->SetValue(300);
			pInt->SetValue(400);
			iTable.Insert();

			pId->SetValue(301);
			pInt->SetValue(401);
			iTable.Insert();

			m_pDb->CommitTrans();
		}
		{
			// And update
			Table iTable(m_pDb, TableAccessFlag::AF_UPDATE_WHERE, tableName);
			iTable.Open();

			// Set some values - this time we also update the primary key columns
			auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

			pId->SetValue(3000);
			pInt->SetValue(5000);
			string where = boost::str(boost::format(u8"%s = 300") % idColName);
			iTable.Update(where);

			pId->SetValue(3001);
			pInt->SetValue(5001);
			where = boost::str(boost::format(u8"%s = 301") % idColName);
			iTable.Update(where);

			m_pDb->CommitTrans();
		}

		// Read back values
		Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName);
		iTable.Open();
		auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		string sqlWhere = boost::str(boost::format(u8"%s = 3000 OR %s = 3001 ORDER by %s") % idColName %idColName %idColName);
		iTable.Select(sqlWhere);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(3000, *pId);
		EXPECT_EQ(5000, *pInt);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(3001, *pId);
		EXPECT_EQ(5001, *pInt);
	}


	// Delete rows
	// ---------
	TEST_F(TableTest, DeletePk)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		ClearTmpTable(TableId::INTEGERTYPES_TMP);

		{
			// Insert some rows
			Table iTable(m_pDb, TableAccessFlag::AF_INSERT, tableName);
			iTable.Open();

			// Set some values
			auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
			pId->SetValue(600);
			pInt->SetValue(700);
			iTable.Insert();

			pId->SetValue(601);
			pInt->SetValue(701);
			iTable.Insert();

			pId->SetValue(602);
			pInt->SetValue(702);
			iTable.Insert();

			m_pDb->CommitTrans();
		}
		{
			// And delete
			Table iTable(m_pDb, TableAccessFlag::AF_DELETE_PK, tableName);
			iTable.Open();

			auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

			pId->SetValue(601);
			iTable.DeleteByPkValues();

			pId->SetValue(602);
			iTable.DeleteByPkValues();

			m_pDb->CommitTrans();
		}

		// Read back values
		Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		iTable.Open();
		auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		string idColName = GetIdColumnName(TableId::INTEGERTYPES_TMP);
		string sqlWhere = boost::str(boost::format(u8"%s >= 600 ORDER BY %s") % idColName %idColName);
		iTable.Select(sqlWhere);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(600, *pId);
		EXPECT_EQ(700, *pInt);
		EXPECT_FALSE(iTable.SelectNext());
	}


	TEST_F(TableTest, DeleteFailOnNoData)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		ClearTmpTable(TableId::INTEGERTYPES_TMP);

		// try to delete from an empty table
		Table iTable(m_pDb, TableAccessFlag::AF_DELETE_PK, tableName);
		iTable.Open();

		auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		// Expect to fail, except we pass the flag not to fail on SQL_NO_DATA
		pId->SetValue(601);
		EXPECT_THROW(iTable.DeleteByPkValues(true), SqlResultException);

		pId->SetValue(601);
		EXPECT_NO_THROW(iTable.DeleteByPkValues(false));
	}


	TEST_F(TableTest, DeleteWhere)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		string idColName = GetIdColumnName(TableId::INTEGERTYPES_TMP);
		ClearTmpTable(TableId::INTEGERTYPES_TMP);

		{
			// Insert some rows
			Table iTable(m_pDb, TableAccessFlag::AF_INSERT, tableName);
			iTable.Open();

			// Set some values
			auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
			auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
			pId->SetValue(6000);
			pInt->SetValue(7000);
			iTable.Insert();

			pId->SetValue(6001);
			pInt->SetValue(7001);
			iTable.Insert();

			m_pDb->CommitTrans();
		}
		{
			// And delete
			Table iTable(m_pDb, TableAccessFlag::AF_DELETE_WHERE, tableName);
			iTable.Open();

			string where = boost::str(boost::format(u8"%s >= 6000") % idColName);
			iTable.Delete(where);

			m_pDb->CommitTrans();
		}

		// Read back values - no more rows
		Table iTable(m_pDb, TableAccessFlag::AF_READ_WITHOUT_PK, tableName);
		iTable.Open();
		auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		string sqlWhere = boost::str(boost::format(u8"%s >= 0 ORDER by %s") % idColName %idColName);
		iTable.Select(sqlWhere);
		EXPECT_FALSE(iTable.SelectNext());
	}


	TEST_F(TableTest, GetColumnBufferIndex)
	{
		std::string intTypesTableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, intTypesTableName);
		ASSERT_NO_THROW(iTable.Open());

		EXPECT_EQ(0, iTable.GetColumnBufferIndex(u8"idintegertypes", false));
		EXPECT_EQ(1, iTable.GetColumnBufferIndex(u8"tsmallint", false));
		EXPECT_EQ(2, iTable.GetColumnBufferIndex(u8"tint", false));
		EXPECT_EQ(3, iTable.GetColumnBufferIndex(u8"tbigint", false));
	}


	TEST_F(TableTest, CreateAutoIntBuffers)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, tableName);
		vector<ColumnBufferPtrVariant> columns = iTable.CreateAutoColumnBufferPtrs(false, false, false);
		
		EXPECT_EQ(4, columns.size());
		// Access reports everything as int columns
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			EXPECT_NO_THROW(boost::get<LongColumnBufferPtr>(columns[0]));
			EXPECT_NO_THROW(boost::get<LongColumnBufferPtr>(columns[1]));
			EXPECT_NO_THROW(boost::get<LongColumnBufferPtr>(columns[2]));
			EXPECT_NO_THROW(boost::get<LongColumnBufferPtr>(columns[3]));
		}
		else
		{
			EXPECT_NO_THROW(boost::get<LongColumnBufferPtr>(columns[0]));
			EXPECT_NO_THROW(boost::get<ShortColumnBufferPtr>(columns[1]));
			EXPECT_NO_THROW(boost::get<LongColumnBufferPtr>(columns[2]));
			EXPECT_NO_THROW(boost::get<BigIntColumnBufferPtr>(columns[3]));
		}
	}


	TEST_F(TableTest, CreateAutoBlobBuffers)
	{
		string tableName = GetTableName(TableId::BLOBTYPES);
		Table bTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, tableName);
		vector<ColumnBufferPtrVariant> columns = bTable.CreateAutoColumnBufferPtrs(false, false, false);

		EXPECT_EQ(3, columns.size());

		EXPECT_NO_THROW(boost::get<LongColumnBufferPtr>(columns[0]));
		EXPECT_NO_THROW(boost::get<BinaryColumnBufferPtr>(columns[1]));
		EXPECT_NO_THROW(boost::get<BinaryColumnBufferPtr>(columns[2]));

		BinaryColumnBufferPtr pBlob16 = boost::get<BinaryColumnBufferPtr>(columns[1]);
		BinaryColumnBufferPtr pVarblob20 = boost::get<BinaryColumnBufferPtr>(columns[2]);

		if (m_pDb->GetDbms() == DatabaseProduct::POSTGRESQL)
		{
			EXPECT_EQ(BinaryColumnBuffer::SQL_NO_TOTAL_BUFFER_LENGTH, pBlob16->GetBufferLength());
			EXPECT_EQ(BinaryColumnBuffer::SQL_NO_TOTAL_BUFFER_LENGTH, pVarblob20->GetBufferLength());
			EXPECT_EQ(BinaryColumnBuffer::SQL_NO_TOTAL_BUFFER_LENGTH, pBlob16->GetNrOfElements());
			EXPECT_EQ(BinaryColumnBuffer::SQL_NO_TOTAL_BUFFER_LENGTH, pVarblob20->GetNrOfElements());
		}
		else
		{
			EXPECT_EQ(16, pBlob16->GetBufferLength());
			EXPECT_EQ(20, pVarblob20->GetBufferLength());
			EXPECT_EQ(16, pBlob16->GetNrOfElements());
			EXPECT_EQ(20, pVarblob20->GetNrOfElements());
		}
	}


	TEST_F(TableTest, CreateAutoCharBuffers)
	{
		string tableName = GetTableName(TableId::CHARTYPES);
		Table cTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, tableName);

		// Depending on the server config, the driver might report SQL_CHAR or 
		// SQL_WCHAR as column type. Simply accept any of those to pass the test
		vector<ColumnBufferPtrVariant> columns = cTable.CreateAutoColumnBufferPtrs(false, false, false);

		ASSERT_EQ(3, columns.size());

		SqlCTypeVisitor cTypeV;

		SQLSMALLINT columnType1 = boost::apply_visitor(cTypeV, columns[1]);
		SQLSMALLINT columnType2 = boost::apply_visitor(cTypeV, columns[2]);

		EXPECT_TRUE(columnType1 == SQL_C_CHAR || columnType1 == SQL_C_WCHAR);
		EXPECT_TRUE(columnType2 == SQL_C_CHAR || columnType2 == SQL_C_WCHAR);

		NrOfElementsVisitor elemCounter;
		SQLLEN nrOfElements1 = boost::apply_visitor(elemCounter, columns[1]);
		SQLLEN nrOfElements2 = boost::apply_visitor(elemCounter, columns[2]);

		EXPECT_EQ(128 + 1, nrOfElements1);
		EXPECT_EQ(128 + 1, nrOfElements2);
	}


	TEST_F(TableTest, CreateAutoDateTypeBuffers)
	{
		string tableName = GetTableName(TableId::DATETYPES);
		Table dTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, tableName);

		// Sql server reports TIME as TIME2 with SqlType -154 - skip that column
		bool skipUnsupportedCols = m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER;
		auto columns = dTable.CreateAutoColumnBufferPtrs(skipUnsupportedCols, false, false);
		
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			EXPECT_EQ(3, columns.size());
		}
		else
		{
			EXPECT_EQ(4, columns.size());
		}

		SqlCTypeVisitor cTypeV;

		EXPECT_EQ(SQL_C_SLONG, boost::apply_visitor(cTypeV, columns[0]));
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			// Access reports only Timestamps
			EXPECT_EQ(SQL_C_TYPE_TIMESTAMP, boost::apply_visitor(cTypeV, columns[1]));
			EXPECT_EQ(SQL_C_TYPE_TIMESTAMP, boost::apply_visitor(cTypeV, columns[2]));
		}
		else if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			EXPECT_EQ(SQL_C_TYPE_DATE, boost::apply_visitor(cTypeV, columns[1]));
		}
		else
		{
			EXPECT_EQ(SQL_C_TYPE_DATE, boost::apply_visitor(cTypeV, columns[1]));
			EXPECT_EQ(SQL_C_TYPE_TIME, boost::apply_visitor(cTypeV, columns[2]));
		}
		size_t tsIndex = 3;
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			tsIndex = 2;
		}

		EXPECT_EQ(SQL_C_TYPE_TIMESTAMP, boost::apply_visitor(cTypeV, columns[tsIndex]));

		TypeTimestampColumnBufferPtr pTs = boost::get<TypeTimestampColumnBufferPtr>(columns[tsIndex]);
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			EXPECT_EQ(3, pTs->GetDecimalDigits());
		}
		else if (m_pDb->GetDbms() == DatabaseProduct::DB2 || m_pDb->GetDbms() == DatabaseProduct::POSTGRESQL)
		{
			EXPECT_EQ(6, pTs->GetDecimalDigits());
		}
		else
		{
			EXPECT_EQ(0, pTs->GetDecimalDigits());
		}
	}


	TEST_F(TableTest, CreateAutoFloatTypeBuffers)
	{
		string tableName = GetTableName(TableId::FLOATTYPES);
		Table dTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, tableName);

		auto columns = dTable.CreateAutoColumnBufferPtrs(false, false, false);
		EXPECT_EQ(3, columns.size());

		SqlCTypeVisitor cTypeV;

		EXPECT_EQ(SQL_C_SLONG, boost::apply_visitor(cTypeV, columns[0]));
		EXPECT_EQ(SQL_C_DOUBLE, boost::apply_visitor(cTypeV, columns[1]));
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER
			|| m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			EXPECT_EQ(SQL_C_DOUBLE, boost::apply_visitor(cTypeV, columns[2]));
		}
		else
		{
			EXPECT_EQ(SQL_C_FLOAT, boost::apply_visitor(cTypeV, columns[2]));
		}
	}


	TEST_F(TableTest, CreateAutoNumericTypeBuffers)
	{
		string tableName = GetTableName(TableId::NUMERICTYPES);
		Table nTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, tableName);

		auto columns = nTable.CreateAutoColumnBufferPtrs(false, false, false);
		EXPECT_EQ(4, columns.size());
		SqlCTypeVisitor cTypeV;

		EXPECT_EQ(SQL_C_SLONG, boost::apply_visitor(cTypeV, columns[0]));
		EXPECT_EQ(SQL_C_NUMERIC, boost::apply_visitor(cTypeV, columns[1]));
		EXPECT_EQ(SQL_C_NUMERIC, boost::apply_visitor(cTypeV, columns[2]));
		EXPECT_EQ(SQL_C_NUMERIC, boost::apply_visitor(cTypeV, columns[3]));

		auto pNum18_0 = boost::get<NumericColumnBufferPtr>(columns[1]);
		auto pNum18_10 = boost::get<NumericColumnBufferPtr>(columns[2]);
		auto pNum5_3 = boost::get<NumericColumnBufferPtr>(columns[3]);

		EXPECT_EQ(18, pNum18_0->GetColumnSize());
		EXPECT_EQ(0, pNum18_0->GetDecimalDigits());

		EXPECT_EQ(18, pNum18_10->GetColumnSize());
		EXPECT_EQ(10, pNum18_10->GetDecimalDigits());

		EXPECT_EQ(5, pNum5_3->GetColumnSize());
		EXPECT_EQ(3, pNum5_3->GetDecimalDigits());
	}


	TEST_F(TableTest, CreateAutoSkipUnsupported)
	{
		// Disable the support for one type.
		// Disable support for Integer by removing it from the typeMap
		auto pTypeMap = std::make_shared<DefaultSql2BufferMap>(OdbcVersion::V_3);
		pTypeMap->ClearType(SQL_INTEGER);

		string tableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, tableName);
		iTable.SetSql2BufferTypeMap(pTypeMap);
		auto columns = iTable.CreateAutoColumnBufferPtrs(true, false, false);

		// The id col and the tint column should be missing
		// except for access, there everything should be missing
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			EXPECT_EQ(0, columns.size());
		}
		else
		{
			EXPECT_EQ(2, columns.size());
		}
	}


	TEST_F(TableTest, CreateAutoFailOnUnsupported)
	{
		// Disable the support for one type.
		// Disable support for Integer by removing it from the typeMap
		auto pTypeMap = std::make_shared<DefaultSql2BufferMap>(OdbcVersion::V_3);
		pTypeMap->ClearType(SQL_INTEGER);

		string tableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, tableName);
		iTable.SetSql2BufferTypeMap(pTypeMap);

		EXPECT_THROW(iTable.CreateAutoColumnBufferPtrs(false, false, false), NotSupportedException);
	}


	TEST_F(TableTest, QueryPrimaryKeysAndUpdateColumns)
	{
		string tableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_SELECT_WHERE, tableName);
		auto columns = iTable.CreateAutoColumnBufferPtrs(false, true, false);
		// no flags activated so far
		for (size_t i = 0; i < columns.size(); ++i)
		{
			auto col = columns[i];
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), col);
			EXPECT_FALSE(pFlags->Test(ColumnFlag::CF_PRIMARY_KEY));
		}
		// activate flags, must select first column
		iTable.QueryPrimaryKeysAndUpdateColumns(iTable.m_columns);
		for (size_t i = 0; i < columns.size(); ++i)
		{
			auto col = columns[i];
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), col);
			if (i == 0)
			{
				EXPECT_TRUE(pFlags->Test(ColumnFlag::CF_PRIMARY_KEY));
			}
			else
			{
				EXPECT_FALSE(pFlags->Test(ColumnFlag::CF_PRIMARY_KEY));
			}
		}


	}

} // namespace exodbctest
