/*!
 * \file DbTableTest.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "TableTest.h"

// Same component headers
#include "exOdbcGTest.h"
#include "ManualTestTables.h"
#include "exOdbcGTestHelpers.h"

// Other headers
#include "Environment.h"
#include "Database.h"
#include "Exception.h"
#include "SqlCBufferVisitors.h"
#include "boost/log/trivial.hpp"
#include "boost/log/core.hpp"
#include "boost/log/expressions.hpp"

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
	

	// Open / Close
	// ------------
	TEST_F(TableTest, OpenManualReadOnlyWithoutCheck)
	{
		// Open an existing table without checking for privileges or existence
		MIntTypesTable table(m_pDb);
		EXPECT_NO_THROW(table.Open(TableOpenFlag::TOF_NONE));

		// If we pass in the TableInfo directly we should also be able to "open"
		// a totally non-sense table:
		TableInfo neTableInfo(L"NotExisting", L"", L"", L"", L"");
		Table neTable(m_pDb, TableAccessFlag::AF_READ, neTableInfo);
		SQLINTEGER idNotExisting = 0;
		neTable.SetColumn(0, L"idNotExistring", SQL_INTEGER, &idNotExisting, SQL_C_SLONG, sizeof(idNotExisting), ColumnFlag::CF_SELECT);
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
		iTable.SetColumn(0, ToDbCase(L"idintegertypes"), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), ColumnFlag::CF_SELECT | ColumnFlag::CF_INSERT | ColumnFlag::CF_PRIMARY_KEY);
		iTable.SetColumn(1, ToDbCase(L"tsmallint"), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), ColumnFlag::CF_SELECT | ColumnFlag::CF_INSERT | ColumnFlag::CF_UPDATE);
		iTable.SetColumn(2, ToDbCase(L"tint"), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), ColumnFlag::CF_SELECT);
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			// access has no bigint
			iTable.SetColumn(3, ToDbCase(L"tbigint"), SQL_INTEGER, &bii, SQL_C_SLONG, sizeof(bii), ColumnFlag::CF_SELECT | ColumnFlag::CF_INSERT);
		}
		else
		{
			iTable.SetColumn(3, ToDbCase(L"tbigint"), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), ColumnFlag::CF_SELECT | ColumnFlag::CF_INSERT);
		}
		EXPECT_NO_THROW(iTable.Open(TableOpenFlag::TOF_NONE));
	}


	TEST_F(TableTest, OpenAutoDefaultCtr)
	{
		wstring tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table table;
		EXPECT_NO_THROW(table.Init(m_pDb, TableAccessFlag::AF_READ, tableName, L"", L"", L""));
		EXPECT_NO_THROW(table.Open());
	}


	TEST_F(TableTest, OpenManualDefaultCtr)
	{
		// Open an existing manual table for reading
		std::wstring intTypesTableName = GetTableName(TableId::INTEGERTYPES);

		Table iTable;
		iTable.Init(m_pDb, TableAccessFlag::AF_READ, intTypesTableName);
		// Set Columns
		SQLINTEGER id;
		SQLSMALLINT tSmallint;
		SQLINTEGER tInt;
		SQLBIGINT tBigint;

		iTable.SetColumn(0, ToDbCase(L"idintegertypes"), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		iTable.SetColumn(1, ToDbCase(L"tsmallint"), SQL_INTEGER, &tSmallint, SQL_C_SSHORT, sizeof(tSmallint), ColumnFlag::CF_SELECT);
		iTable.SetColumn(2, ToDbCase(L"tint"), SQL_INTEGER, &tInt, SQL_C_SLONG, sizeof(tInt), ColumnFlag::CF_SELECT);
		iTable.SetColumn(3, ToDbCase(L"tbigint"), SQL_INTEGER, &tBigint, SQL_C_SBIGINT, sizeof(tBigint), ColumnFlag::CF_SELECT);
		
		EXPECT_NO_THROW(iTable.Open());
	}


	TEST_F(TableTest, CopyCtr)
	{
		// Create a table
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table t1(m_pDb, TableAccessFlag::AF_READ, tableName);

		// Create a copy of that table
		Table c1(t1);
		EXPECT_EQ(t1.m_initialTableName, c1.m_initialTableName);
		EXPECT_EQ(t1.m_initialSchemaName, c1.m_initialSchemaName);
		EXPECT_EQ(t1.m_initialCatalogName, c1.m_initialCatalogName);
		EXPECT_EQ(t1.m_initialTypeName, c1.m_initialTypeName);
		EXPECT_EQ(t1.GetAccessFlags(), c1.GetAccessFlags());

		// If we open the table..
		EXPECT_NO_THROW(c1.Open());

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
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			MNotExistingTable neTable(m_pDb);
			EXPECT_THROW(neTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE), exodbc::Exception);
		}
	}


	//TEST_F(TableTest, OpenManualCheckColumnFlagSelect)
	//{
	//	// Open a table manually but do not set the Select flag for all columns
	//	Table iTable(m_pDb, TableAccessFlag::AF_SELECT, GetTableName(TableId::INTEGERTYPES));

	//	SqlSLongBuffer idCol;
	//	SqlSShortBuffer shortCol;
	//	SqlSLongBuffer intCol;
	//	SqlSBigIntBuffer bigIntCol;

	//	SQLINTEGER id = 0;
	//	SQLSMALLINT si = 0;
	//	SQLINTEGER i = 0;
	//	SQLBIGINT bi = 0;
	//	iTable.SetColumn(0, ToDbCase(L"idintegertypes"), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), TableOpenFlag::CF_SELECT);
	//	iTable.SetColumn(1, ToDbCase(L"tsmallint"), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), TableOpenFlag::CF_SELECT | TableOpenFlag::CF_NULLABLE);
	//	iTable.SetColumn(2, ToDbCase(L"tint"), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), TableOpenFlag::CF_NONE);
	//	iTable.SetColumn(3, ToDbCase(L"tbigint"), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), TableOpenFlag::CF_SELECT | TableOpenFlag::CF_NULLABLE);

	//	ASSERT_NO_THROW(iTable.Open());
	//	// We expect all columnBuffers to be bound, except nr 2
	//	ColumnBuffer* pBuffId = iTable.GetColumnVariant(0);
	//	ColumnBuffer* pBuffsi = iTable.GetColumnVariant(1);
	//	ColumnBuffer* pBuffi = iTable.GetColumnVariant(2);
	//	ColumnBuffer* pBuffbi = iTable.GetColumnVariant(3);
	//	EXPECT_TRUE(pBuffId->IsBound());
	//	EXPECT_TRUE(pBuffsi->IsBound());
	//	EXPECT_FALSE(pBuffi->IsBound());
	//	EXPECT_TRUE(pBuffbi->IsBound());

	//	// And we should be able to select a row
	//	wstring sqlstmt = boost::str(boost::wformat(L"%s = 7") % test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase));
	//	iTable.Select(sqlstmt);
	//	EXPECT_TRUE(iTable.SelectNext());
	//	// and have all values
	//	EXPECT_EQ(7, id);
	//	EXPECT_EQ(-13, si);
	//	EXPECT_EQ(0, i);
	//	EXPECT_EQ(10502, bi);
	//	{
	//		LogLevelFatal llf;
	//		DontDebugBreak ddb;
	//		// except the not bound column, we are unable to get its value, but its buffer has not changed
	//		EXPECT_THROW(boost::get<SQLINTEGER>(iTable.GetColumnValue(2)), AssertionException);
	//	}
	//}


//	TEST_F(TableTest, OpenManualCheckColumnFlagInsert)
//	{
//		// Open a table manually but do not set the Insert flag for all columns
//		Table iTable(&m_db, AF_SELECT | AF_INSERT | AF_DELETE, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"");
//		SQLINTEGER id = 0;
//		SQLSMALLINT si = 0;
//		SQLINTEGER i = 0;
//		SQLBIGINT bi = 0;
//		int type = SQL_C_SLONG;
//
//		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT | CF_PRIMARY_KEY);
//		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_INSERT);
//		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
//		}
//		else
//		{
//			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
//		}
//
//		// Open and remove all data from the table
//		iTable.Open();
//		ASSERT_NO_THROW(test::ClearTestTable(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase, iTable, m_db));
//
//		// Insert a value by using our primary key - columnIndex 2 will not get inserted but the default NULL value will be set by the db
//		iTable.SetColumnValue(0, (SQLINTEGER)11);
//		iTable.SetColumnValue(1, (SQLSMALLINT)202);
//		iTable.SetColumnValue(2, (SQLINTEGER)303);
//		iTable.SetColumnValue(3, (SQLBIGINT)-404);
//		EXPECT_NO_THROW(iTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Read back from another table
//		// note that when opening an Access Table Access automatically, the second column will not get bound to a SMALLINT, as
//		// Access reports it as SQL_INTEGER (Db-Type) which corresponds to C-Type of SQL_C_SLONG.
//		// The other databases report it as SQL_SMALLINT which we bind to SQL_C_SSHORT
//		Table iTable2(&m_db, AF_READ, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"");
//		iTable2.Open();
//		iTable2.Select();
//		ASSERT_TRUE(iTable2.SelectNext());
//		EXPECT_EQ(11, boost::get<SQLINTEGER>(iTable2.GetColumnValue(0)));
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			EXPECT_EQ(202, boost::get<SQLINTEGER>(iTable2.GetColumnValue(1)));
//			EXPECT_EQ(-404, boost::get<SQLINTEGER>(iTable2.GetColumnValue(3)));
//		}
//		else
//		{
//			EXPECT_EQ(202, boost::get<SQLSMALLINT>(iTable2.GetColumnValue(1)));
//			EXPECT_EQ(-404, boost::get<SQLBIGINT>(iTable2.GetColumnValue(3)));
//		}
//		EXPECT_TRUE(iTable2.IsColumnNull(2));
//	}
//
//
//	TEST_F(TableTest, OpenManualCheckColumnFlagUpdate)
//	{
//		// Open a table manually but do not set the Update flag for all columns
//		Table iTable(&m_db, AF_SELECT | AF_UPDATE | AF_DELETE | AF_INSERT, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"");
//		SQLINTEGER id = 0;
//		SQLSMALLINT si = 0;
//		SQLINTEGER i = 0;
//		SQLBIGINT bi = 0;
//		int type = SQL_C_SLONG;
//
//		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_UPDATE | CF_INSERT | CF_PRIMARY_KEY);
//		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_UPDATE | CF_INSERT);
//		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT | CF_INSERT);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT | CF_UPDATE);
//		}
//		else
//		{
//			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT | CF_UPDATE);
//		}
//
//		// Open and remove all data from the table
//		iTable.Open();
//		ASSERT_NO_THROW(test::ClearTestTable(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase, iTable, m_db));
//
//		// Insert some value to update later
//		iTable.SetColumnValue(0, (SQLINTEGER)11);
//		iTable.SetColumnValue(1, (SQLSMALLINT)202);
//		iTable.SetColumnValue(2, (SQLINTEGER)303);
//		iTable.SetColumnValue(3, (SQLBIGINT)-404);
//		EXPECT_NO_THROW(iTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Select to update
//		iTable.Select();
//		ASSERT_TRUE(iTable.SelectNext());
//
//		// Update - note column 2 will not get updated, flag not set
//		// iTable.SetColumnValue(0, (SQLINTEGER) 11); //pk
//		iTable.SetColumnValue(1, (SQLSMALLINT) 880);
//		iTable.SetColumnValue(2, (SQLINTEGER) 990);
//		iTable.SetColumnValue(3, (SQLBIGINT) 1001);
//		EXPECT_NO_THROW(iTable.Update());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Read back from another table - column 2 still has the originally inserted value
//		Table iTable2(&m_db, AF_READ, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"");
//		iTable2.Open();
//		iTable2.Select();
//		ASSERT_TRUE(iTable2.SelectNext());
//		EXPECT_EQ(11, boost::get<SQLINTEGER>(iTable2.GetColumnValue(0)));
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			EXPECT_EQ(880, boost::get<SQLINTEGER>(iTable2.GetColumnValue(1)));
//			EXPECT_EQ(1001, boost::get<SQLINTEGER>(iTable2.GetColumnValue(3)));
//		}
//		else
//		{
//			EXPECT_EQ(880, boost::get<SQLSMALLINT>(iTable2.GetColumnValue(1)));
//			EXPECT_EQ(1001, boost::get<SQLBIGINT>(iTable2.GetColumnValue(3)));
//		}
//		EXPECT_EQ(303, boost::get<SQLINTEGER>(iTable2.GetColumnValue(2)));
//	}
//	
//
//	TEST_F(TableTest, OpenManualPrimaryKeys)
//	{
//		// Open a table by defining primary keys manually
//		// Open a table manually but do not set the Select flag for all columns
//		Table iTable(&m_db, AF_SELECT | AF_DELETE | AF_INSERT, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"");
//		SQLINTEGER id = 0;
//		SQLSMALLINT si = 0;
//		SQLINTEGER i = 0;
//		SQLBIGINT bi = 0;
//		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT |  CF_INSERT | CF_PRIMARY_KEY);
//		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_INSERT);
//		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT | CF_INSERT);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
//		}
//		else
//		{
//			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
//		}
//
//		// Opening must work
//		EXPECT_NO_THROW(iTable.Open(TOF_DO_NOT_QUERY_PRIMARY_KEYS));
//
//		// But opening if primary keys are not defined must fail
//		Table iTable2(&m_db, AF_SELECT | AF_DELETE | AF_INSERT, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"");
//		SQLINTEGER id2 = 0;
//		SQLSMALLINT si2 = 0;
//		SQLINTEGER i2 = 0;
//		SQLBIGINT bi2 = 0;
//		iTable2.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id2, SQL_C_SLONG, sizeof(id2), CF_SELECT | CF_INSERT);
//		iTable2.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si2, SQL_C_SSHORT, sizeof(si2), CF_SELECT | CF_INSERT);
//		iTable2.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i2, SQL_C_SLONG, sizeof(i2), CF_SELECT | CF_INSERT);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			iTable2.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
//		}
//		else
//		{
//			iTable2.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
//		}
//		EXPECT_THROW(iTable2.Open(TOF_DO_NOT_QUERY_PRIMARY_KEYS), Exception);
//
//		// But if we open for select only, we do not care about the primary keys
//		Table iTable3(&m_db, AF_READ, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"");
//		SQLINTEGER id3 = 0;
//		SQLSMALLINT si3 = 0;
//		SQLINTEGER i3 = 0;
//		SQLBIGINT bi3 = 0;
//		iTable3.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id3, SQL_C_SLONG, sizeof(id3), CF_SELECT);
//		iTable3.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si3, SQL_C_SSHORT, sizeof(si3), CF_SELECT);
//		iTable3.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i3, SQL_C_SLONG, sizeof(i3), CF_SELECT);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			iTable3.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT);
//		}
//		else
//		{
//			iTable3.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT);
//		}
//		EXPECT_NO_THROW(iTable3.Open(TOF_DO_NOT_QUERY_PRIMARY_KEYS));
//	}


	TEST_F(TableTest, OpenAutoWithoutCheck)
	{
		// Open an auto-table without checking for privileges or existence
		// This makes only sense if we've already determined the correct TableInfo structure
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		TableInfo tableInfo;
		ASSERT_NO_THROW(tableInfo = m_pDb->FindOneTable(tableName, L"", L"", L""));

		exodbc::Table table(m_pDb, TableAccessFlag::AF_READ, tableInfo);
		EXPECT_NO_THROW(table.Open(TableOpenFlag::TOF_NONE));

		// If we try to open an auto-table this will never work if you've passed invalid information:
		// As soon as the columns are searched, we expect to fail
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			TableInfo neTableInfo(L"NotExisting", L"", L"", L"", L"");
			Table neTable(m_pDb, TableAccessFlag::AF_READ, neTableInfo);
			EXPECT_THROW(neTable.Open(TableOpenFlag::TOF_NONE), Exception);
		}
	}


	TEST_F(TableTest, OpenAutoCheckExistence)
	{
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table table(m_pDb, TableAccessFlag::AF_READ, tableName, L"", L"", L"");
		EXPECT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		// Open a non-existing table
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			std::wstring neName = GetTableName(TableId::NOT_EXISTING);
			exodbc::Table neTable(m_pDb, TableAccessFlag::AF_READ, neName, L"", L"", L"");
			EXPECT_THROW(neTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE), Exception);
		}
	}


	TEST_F(TableTest, CheckPrivileges)
	{
		// Test to open read-only a table we know we have all rights:
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table rTable(m_pDb, TableAccessFlag::AF_READ, tableName, L"", L"", L"");
		EXPECT_NO_THROW(rTable.CheckPrivileges());

		// Test to open read-write a table we know we have all rights:
		exodbc::Table rTable2(m_pDb, TableAccessFlag::AF_READ_WRITE, tableName, L"", L"", L"");
		EXPECT_NO_THROW(rTable2.CheckPrivileges());

		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			// We only have a Read-only user for ms sql server
			DatabasePtr pDb = std::make_shared<Database>(m_pEnv);
			if (g_odbcInfo.HasConnectionString())
			{
				return;
			}
			else
			{
				ASSERT_NO_THROW(pDb->Open(g_odbcInfo.m_dsn, L"exReadOnly", L"exReadOnly"));
			}

			// Test to open a table read-only
			// Note that here in ms server we have given the user no rights except the select for this table
			// If you add him to some role, things will get messed up: No privs are reported, but the user can
			// still access the table for selecting
			exodbc::Table table2(pDb, TableAccessFlag::AF_READ, tableName, L"", L"", L"");
			EXPECT_NO_THROW(table2.CheckPrivileges());

			// We expect to fail if trying to open for writing
			table2.SetAccessFlags(TableAccessFlag::AF_READ_WRITE);
			EXPECT_THROW(table2.CheckPrivileges(), MissingTablePrivilegeException);

			// If we open the Table and do not check privileges, we fail later when trying to prepare a write-statement
			EXPECT_THROW(table2.Open(), SqlResultException);
		}
	}


//	TEST_F(TableTest, OpenManualWithUnsupportedColumn)
//	{
//		Table iTable(&m_db, AF_SELECT, test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase), L"", L"", L"");
//		SQLINTEGER id = 0;
//		SQLSMALLINT si = 0;
//		SQLINTEGER i = 0;
//		SQLBIGINT bi = 0;
//		int type = SQL_C_SLONG;
//		
//		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//
//		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT);
//		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT);
//		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT);
//		// Set some silly type 30666 for the last column
//		iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), 30666, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT);
//		// Expect to fail if Opening without flag
//		EXPECT_THROW(iTable.Open(), Exception);
//		// But not if we skip
//		{
//			LogLevelError lle;
//			EXPECT_NO_THROW(iTable.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
//		}
//		// We should be able to select now
//		EXPECT_NO_THROW(iTable.Select(boost::str(boost::wformat(L"%s = 4") %idName)));
//		EXPECT_TRUE(iTable.SelectNext());
//		// just the last row doesnt exist
//		EXPECT_EQ(4, iTable.GetInt(0));
//		EXPECT_TRUE(iTable.IsColumnNull(1));
//		EXPECT_EQ(2147483647, iTable.GetInt(2));
//		EXPECT_FALSE(iTable.ColumnBufferExists(3));
//	}


	TEST_F(TableTest, OpenAutoSkipUnsupportedColumn)
	{
		std::wstring tableName = GetTableName(TableId::NOT_SUPPORTED);
		exodbc::Table nst(m_pDb, TableAccessFlag::AF_READ_WRITE, tableName);

		// Expect to fail if we open with default flags
		ASSERT_THROW(nst.Open(), NotSupportedException);
		// But not if we pass the flag to skip
		{
			LogLevelFatal llf;
			ASSERT_NO_THROW(nst.Open(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// The table is: id, int1, xml, int2
		// but we fail to read xml, so we have only indexes
		// 0, 1, 2 which map to
		// id, int1, int2

		EXPECT_NO_THROW(nst.GetColumnBufferPtr<LongColumnBufferPtr>(0));
		EXPECT_NO_THROW(nst.GetColumnBufferPtr<LongColumnBufferPtr>(1));
		EXPECT_NO_THROW(nst.GetColumnBufferPtr<LongColumnBufferPtr>(2));

		EXPECT_THROW(nst.GetColumnBufferPtr<LongColumnBufferPtr>(3), IllegalArgumentException);
	}


	TEST_F(TableTest, SelectFromAutoWithSkippedUnsupportedColumn)
	{
		std::wstring tableName = GetTableName(TableId::NOT_SUPPORTED);
		exodbc::Table nst(m_pDb, TableAccessFlag::AF_READ_WRITE, tableName);

		// we do not fail to open if we pass the flag to skip
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst.Open(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// The table is: id, int1, xml, int2
		// but we fail to read xml, so we have only indexes
		// 0, 1, 2 which map to
		// id, int1, int2
		LongColumnBufferPtr pId = nst.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pInt1 = nst.GetColumnBufferPtr<LongColumnBufferPtr>(1);
		LongColumnBufferPtr pInt2 = nst.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		EXPECT_THROW(nst.GetColumnBufferPtr<LongColumnBufferPtr>(3), IllegalArgumentException);

		EXPECT_NO_THROW(nst.Select());
		EXPECT_TRUE(nst.SelectNext());
		EXPECT_EQ(1, pId->GetValue());
		EXPECT_EQ(10, pInt1->GetValue());
		EXPECT_EQ(12, pInt2->GetValue());
	}


//	TEST_F(TableTest, InsertIntoAutoWithUnsupportedColumn)
//	{
//
//
//		std::wstring tableName = test::GetTableName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
//		exodbc::Table nst(&m_db, AF_READ_WRITE, tableName, L"", L"", L"");
//
//		// we do not if we pass the flag to skip
//		{
//			LogLevelFatal llf;
//			EXPECT_NO_THROW(nst.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
//		}
//
//		// Remove everything, ignoring if there was any data:
//		wstring idName = test::GetIdColumnName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
//		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
//		ASSERT_NO_THROW(nst.Delete(sqlstmt, false));
//		ASSERT_NO_THROW(m_db.CommitTrans());
//
//		// Insert some data
//		EXPECT_FALSE(nst.ColumnBufferExists(2));
//		nst.SetColumnValue(0, (SQLINTEGER)2);
//		nst.SetColumnValue(1, (SQLINTEGER)20);
//		nst.SetColumnValue(3, (SQLINTEGER)22);
//		EXPECT_NO_THROW(nst.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// We should now be able to select. As a test, use a different table object
//		exodbc::Table nst2(&m_db, AF_READ_WRITE, tableName, L"", L"", L"");
//		{
//			LogLevelFatal llf;
//			EXPECT_NO_THROW(nst2.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
//		}
//
//		nst2.Select();
//		EXPECT_TRUE(nst2.SelectNext());
//		SQLINTEGER id, int1, int2;
//		EXPECT_NO_THROW(id = nst2.GetInt(0));
//		EXPECT_NO_THROW(int1 = nst2.GetInt(1));
//		EXPECT_NO_THROW(int2 = nst2.GetInt(3));
//		EXPECT_EQ(2, id);
//		EXPECT_EQ(20, int1);
//		EXPECT_EQ(22, int2);
//	}
//
//
//	TEST_F(TableTest, UpdateIntoAutoWithUnsupportedColumn)
//	{
//
//
//		std::wstring tableName = test::GetTableName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
//		exodbc::Table nst(&m_db, AF_READ_WRITE, tableName, L"", L"", L"");
//
//		// we do not if we pass the flag to skip
//		{
//			LogLevelFatal llf;
//			EXPECT_NO_THROW(nst.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
//		}
//
//		// Remove everything, ignoring if there was any data:
//		wstring idName = test::GetIdColumnName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
//		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
//		ASSERT_NO_THROW(nst.Delete(sqlstmt, false));
//		ASSERT_NO_THROW(m_db.CommitTrans());
//
//		// Insert some data
//		nst.SetColumnValue(0, (SQLINTEGER)2);
//		nst.SetColumnValue(1, (SQLINTEGER)20);
//		nst.SetColumnValue(3, (SQLINTEGER)22);
//		EXPECT_NO_THROW(nst.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Now update it
//		nst.SetColumnValue(1, (SQLINTEGER)30);
//		nst.SetColumnValue(3, (SQLINTEGER)32);
//		EXPECT_NO_THROW(nst.Update());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// We should now be able to select. As a test, use a different table object
//		exodbc::Table nst2(&m_db, AF_READ_WRITE, tableName, L"", L"", L"");
//		{
//			LogLevelFatal llf;
//			EXPECT_NO_THROW(nst2.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
//		}
//
//		nst2.Select();
//		EXPECT_TRUE(nst2.SelectNext());
//		SQLINTEGER id, int1, int2;
//		EXPECT_NO_THROW(id = nst2.GetInt(0));
//		EXPECT_NO_THROW(int1 = nst2.GetInt(1));
//		EXPECT_NO_THROW(int2 = nst2.GetInt(3));
//		EXPECT_EQ(2, id);
//		EXPECT_EQ(30, int1);
//		EXPECT_EQ(32, int2);
//	}
//
//
//	TEST_F(TableTest, DeleteFromAutoWithUnsupportedColumn)
//	{
//
//
//		std::wstring tableName = test::GetTableName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
//		exodbc::Table nst(&m_db, AF_READ_WRITE, tableName, L"", L"", L"");
//
//		// we do not if we pass the flag to skip
//		{
//			LogLevelFatal llf;
//			EXPECT_NO_THROW(nst.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
//		}
//
//		// Remove everything, ignoring if there was any data:
//		wstring idName = test::GetIdColumnName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
//		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
//		ASSERT_NO_THROW(nst.Delete(sqlstmt, false));
//		ASSERT_NO_THROW(m_db.CommitTrans());
//
//		// Insert some data
//		nst.SetColumnValue(0, (SQLINTEGER)2);
//		nst.SetColumnValue(1, (SQLINTEGER)20);
//		nst.SetColumnValue(3, (SQLINTEGER)22);
//		EXPECT_NO_THROW(nst.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Check its there
//		ASSERT_NO_THROW(nst.Select());
//		ASSERT_TRUE(nst.SelectNext());
//
//		// Now delete it
//		nst.SetColumnValue(0, (SQLINTEGER)2);
//		EXPECT_NO_THROW(nst.Delete());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// We should now longer be able to select it. As a test, use a different table object
//		exodbc::Table nst2(&m_db, AF_READ_WRITE, tableName, L"", L"", L"");
//		{
//			LogLevelFatal llf;
//			EXPECT_NO_THROW(nst2.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
//		}
//
//		nst2.Select();
//		EXPECT_FALSE(nst2.SelectNext());
//	}
//
//
	TEST_F(TableTest, Close)
	{
		// Create table
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName, L"", L"", L"");

		// Close when not open must fail
		{
			LogLevelFatal llf;
			EXPECT_THROW(iTable.Close(), AssertionException);
		}

		// Open a Table read-only
		ASSERT_NO_THROW(iTable.Open());

		// Close must work
		EXPECT_NO_THROW(iTable.Close());

		// Close an already closed table
		{
			LogLevelFatal llf;
			EXPECT_THROW(iTable.Close(), AssertionException);
		}
	}


	TEST_F(TableTest, OpenAndCloseAndOpenAndClose)
	{
		// Create a Table, open for reading
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName, L"", L"", L"");
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
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName, L"", L"", L"");

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
			LogLevelFatal llf;
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
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName, L"", L"", L"");
		
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(TableAccessFlag::AF_UPDATE);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(TableAccessFlag::AF_UPDATE);
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(TableAccessFlag::AF_INSERT);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(TableAccessFlag::AF_INSERT);
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(TableAccessFlag::AF_DELETE);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(TableAccessFlag::AF_DELETE);
		EXPECT_TRUE(iTable.IsQueryOnly());

		// and one that is initially rw
		exodbc::Table iTable2(m_pDb, TableAccessFlag::AF_READ_WRITE, tableName, L"", L"", L"");
		EXPECT_FALSE(iTable2.IsQueryOnly());
		iTable2.ClearAccessFlag(TableAccessFlag::AF_INSERT);
		EXPECT_FALSE(iTable2.IsQueryOnly());

		iTable2.ClearAccessFlag(TableAccessFlag::AF_DELETE);
		EXPECT_FALSE(iTable2.IsQueryOnly());

		iTable2.ClearAccessFlag(TableAccessFlag::AF_UPDATE);
		EXPECT_TRUE(iTable2.IsQueryOnly());

		// remove read, we are no longer query only
		iTable2.ClearAccessFlag(TableAccessFlag::AF_SELECT);
		EXPECT_FALSE(iTable2.IsQueryOnly());
	}


//	TEST_F(TableTest, MissingAccessFlagsThrowOnWrite)
//	{
//		// Open a read-only table
//		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
//
//		ASSERT_NO_THROW(iTable.Open());
//
//		// we should be able to set some silly value..
//		iTable.SetColumnValue(0, (SQLINTEGER)333);
//		// .. but not insert, delete or update it
//		{
//			LogLevelFatal llf;
//			DontDebugBreak ddb;
//			EXPECT_THROW(iTable.Insert(), AssertionException);
//			EXPECT_THROW(iTable.Delete(), AssertionException);
//			EXPECT_THROW(iTable.Update(), AssertionException);
//		}
//
//		// we test that writing works in all those cases where we open RW
//	}
//
//
	// Select / GetNext
	// ----------------
	TEST_F(TableTest, Select)
	{
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open());

		EXPECT_NO_THROW(iTable.Select(L""));

		iTable.SelectClose();
	}


	TEST_F(TableTest, SelectFirst)
	{
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		std::wstring idName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open());

		LongColumnBufferPtr pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pIntCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		// We expect some Record that is not the one with id 2 if we move forward three times
		std::wstring sqlWhere = boost::str(boost::wformat(L"%s >= 2 ORDER BY %s ASC") % idName % idName);
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
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		std::wstring idName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open());

		LongColumnBufferPtr pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pIntCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		// We expect some Record that is not the one with id 2 if we move away from that one
		std::wstring sqlWhere = boost::str(boost::wformat(L"%s >= 2 ORDER BY %s ASC") % idName % idName);
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
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		std::wstring idName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open());

		LongColumnBufferPtr pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pIntCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		// Move the 3rd record in the result-set. 
		std::wstring sqlWhere = boost::str(boost::wformat(L"%s >= 2 ORDER BY %s ASC") % idName % idName);
		ASSERT_NO_THROW(iTable.Select(sqlWhere));
		EXPECT_TRUE(iTable.SelectAbsolute(3));
		EXPECT_EQ(4, *pIdCol);
		EXPECT_EQ(2147483647, *pIntCol);

		// Select something not in result set
		EXPECT_FALSE(iTable.SelectAbsolute(20));
	}


	TEST_F(TableTest, SelectRelative)
	{
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		std::wstring idName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open());

		LongColumnBufferPtr pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pIntCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		// We expect some Records
		std::wstring sqlWhere = boost::str(boost::wformat(L"%s >= 2 ORDER BY %s ASC") % idName % idName);
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
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		std::wstring idName = GetIdColumnName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open());

		LongColumnBufferPtr pIdCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		LongColumnBufferPtr pIntCol = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);
		
		// We expect some Records
		std::wstring sqlWhere = boost::str(boost::wformat(L"%s >= 2 ORDER BY %s ASC") % idName % idName);
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
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open());

		// We expect 7 Records
		iTable.Select(L"");
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


	TEST_F(TableTest, SelectClose)
	{
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		ASSERT_NO_THROW(iTable.Open());
		
		// Do something that opens a transaction
		iTable.Select(L"");
		EXPECT_NO_THROW(iTable.SelectClose());
	}


//	TEST_F(TableTest, SelectHasMASEnabled)
//	{
//		// Test if we can have multiple active statements.
//		// For MS SQL Server, this is disabled by default and must be activated manually
//		// It must be activated before opening the database
//
//		// Use our already open database to determine the server type
//		Database db(&m_env);
//		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			// It is disabled by default on MS and must be enabled before opening the db:
//			EXPECT_FALSE(m_db.IsMarsEnabled());
//			ASSERT_NO_THROW(db.SetMarsEnabled(true));
//		}
//
//		if (m_odbcInfo.HasConnectionString())
//		{
//			db.Open(m_odbcInfo.m_connectionString);
//		}
//		else
//		{
//			db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password);
//		}
//		EXPECT_TRUE(db.IsMarsEnabled());
//
//		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Clear Table
//		test::ClearIntTypesTmpTable(db, m_odbcInfo.m_namesCase);
//
//		// Insert some values
//		test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, db, 1, 10, 100, 1000);
//		test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, db, 2, 20, 200, 2000);
//		test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, db, 3, 30, 300, 3000);
//
//		// Open one statement, select first record, but more records would be available
//		Table iTable(&db, AF_READ, intTypesTableName, L"", L"", L"");
//		iTable.Open();
//		ASSERT_NO_THROW(iTable.Select());
//		ASSERT_TRUE(iTable.SelectNext());
//
//		// Open another statement, where we also select some record. If MARS is not enabled, we would have 
//		// to close the other result-set first: MS Sql Server fails with a "busy for other result" during Open(),
//		// when it queries the Database to find the table.
//		// While Access just fails on the SelectNext()
//		Table iTable2(&db, AF_READ, intTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable2.Open());
//		EXPECT_NO_THROW(iTable2.Select());
//		EXPECT_TRUE(iTable2.SelectNext());
//	}
//
//
//	TEST_F(TableTest, SelectMoreColumnsThanBound)
//	{
//		// What happens if we query more columns than actually bound?
//		// See ticket #65
//		wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		wstring schemaOrCatalogName = test::ConvertNameCase(L"exodbc", m_odbcInfo.m_namesCase);
//		wstring idColName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
//		// We do not bind the bigint-column
//		SQLINTEGER id = 0;
//		SQLSMALLINT tSmallInt = 0;
//		SQLINTEGER tInt = 0;
//		SQLBIGINT tBigInt = 0;
//		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_PRIMARY_KEY | CF_SELECT);
//		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &tSmallInt, SQL_C_SSHORT, sizeof(tSmallInt), CF_SELECT);
//		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &tInt, SQL_C_SLONG, sizeof(tInt), CF_SELECT);
//		//iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &tBigInt, SQL_C_SBIGINT, sizeof(tBigInt), CF_SELECT);
//
//		ASSERT_NO_THROW(iTable.Open());
//		wstring sqlstmt;
//		wstringstream ws;
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			// access has no schema / catalog name
//			ws << L"SELECT * FROM " << tableName << L" WHERE " << idColName << L" = 7";
//		}
//		else
//		{
//			ws << L"SELECT * FROM " << schemaOrCatalogName << L"." << tableName << L" WHERE " << idColName << L" = 7";
//		}
//		ASSERT_NO_THROW(iTable.SelectBySqlStmt(ws.str()));
//		ASSERT_TRUE(iTable.SelectNext());
//		EXPECT_EQ(7, id);
//		EXPECT_EQ(-13, tSmallInt);
//		EXPECT_EQ(26, tInt);
//		EXPECT_EQ(0, tBigInt);
//		
//		// Now execute some query on the database
//		EXPECT_NO_THROW(m_db.ExecSql(ws.str()));
//
//		// .. works as expected.. what whas the issue with #65 ?
//	}
//
//
	// Count
	// -----
	TEST_F(TableTest, Count)
	{
		Table table(m_pDb, TableAccessFlag::AF_READ, GetTableName(TableId::FLOATTYPES));
		ASSERT_NO_THROW(table.Open());

		SQLUBIGINT all;
		EXPECT_NO_THROW(all = table.Count(L""));
		EXPECT_EQ(6, all);

		// \todo: Check some really big table, with billions of rows
		SQLUBIGINT some;
		std::wstring whereStmt = L"tdouble > 0";
		if (m_pDb->GetDbms() == DatabaseProduct::DB2)
		{
			whereStmt = L"TDOUBLE > 0";
		}
		EXPECT_NO_THROW(some = table.Count(whereStmt));
		EXPECT_EQ(1, some);
		whereStmt = L"tdouble > 0 OR tfloat > 0";
		if (m_pDb->GetDbms() == DatabaseProduct::DB2)
		{
			whereStmt = L"TDOUBLE > 0 OR TFLOAT > 0";
		}
		EXPECT_NO_THROW(some = table.Count(whereStmt));
		EXPECT_EQ(2, some);

		// we should fail with an exception if we query using a non-sense where-stmt
		EXPECT_THROW(table.Count(L"a query that is invalid for sure"), Exception);
	}


	// Insert rows
	// ---------
	TEST_F(TableTest, Insert)
	{
		wstring tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		ClearTmpTable(TableId::INTEGERTYPES_TMP);

		{
			// First insert where all columns are bound
			Table iTable(m_pDb, TableAccessFlag::AF_READ | TableAccessFlag::AF_INSERT, tableName);
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
			// Now insert with only id-column bound for inserting, and the int column only bound for selecting
			Table iTable(m_pDb, TableAccessFlag::AF_READ | TableAccessFlag::AF_INSERT, tableName);
			auto columns = iTable.CreateAutoColumnBufferPtrs(false);
			LongColumnBufferPtr pId = boost::get<LongColumnBufferPtr>(columns[0]);
			LongColumnBufferPtr pInt = boost::get<LongColumnBufferPtr>(columns[2]);
			pInt->Clear(ColumnFlag::CF_INSERT);
			iTable.SetColumn(0, columns[0]);
			iTable.SetColumn(1, columns[2]);
			iTable.Open();
			pId->SetValue(301);
			pInt->SetValue(401);
			iTable.Insert();
		}
		m_pDb->CommitTrans();

		// Read back values
		Table iTable(m_pDb, TableAccessFlag::AF_READ, tableName);
		iTable.Open();
		auto pId = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pInt = iTable.GetColumnBufferPtr<LongColumnBufferPtr>(2);

		wstring idColName = GetIdColumnName(TableId::INTEGERTYPES_TMP);
		wstring sqlWhere = boost::str(boost::wformat(L"%s = 300 OR %s = 301 ORDER by %s") % idColName %idColName %idColName);
		iTable.Select(sqlWhere);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(300, *pId);
		EXPECT_EQ(400, *pInt);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(301, *pId);
		EXPECT_TRUE(pInt->IsNull());
	}


//	// Delete rows: We do not test that for the different data-types, its just deleting a row.
//	// -----------
//	TEST_F(TableTest, DeleteWhere)
//	{
//		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table iTable(&m_db, AF_SELECT | AF_DELETE_WHERE | AF_INSERT, intTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//
//		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
//
//		// Note: We could also do this in one transaction.
//		// \todo: Write a separate transaction test about this, to check transaction-visibility
//		// Try to delete eventually available leftovers, ignore if none exists
//		test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		// Now lets insert some data:
//		test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, m_db, 103, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL);
//
//		// Now we must have something to delete
//		EXPECT_NO_THROW(iTable.Delete(sqlstmt, true));
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// And fetching shall return no results at all
//		iTable.Select();
//		EXPECT_FALSE(iTable.SelectNext());
//	}
//
//
//	TEST_F(TableTest, DeleteWhereShouldReturnSQL_NO_DATA)
//	{
//		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table iTable(&m_db, AF_SELECT | AF_DELETE_WHERE, intTypesTableName, L"", L"", L"");
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			iTable.SetColumnPrimaryKeyIndexes({ 0 });
//		}
//		ASSERT_NO_THROW(iTable.Open());
//
//		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
//		{
//			LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #77");
//		}
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		// We can be sure now that nothing exists, and we will fail if we try to delete
//		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
//		EXPECT_THROW(iTable.Delete(sqlstmt, true), SqlResultException);
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// And fetching shall return no results at all
//		iTable.Select();
//		EXPECT_FALSE(iTable.SelectNext());
//	}
//
//
//	TEST_F(TableTest, Delete)
//	{
//		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table iTable(&m_db, AF_SELECT | AF_DELETE_PK, intTypesTableName, L"", L"", L"");
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			iTable.SetColumnPrimaryKeyIndexes({ 0 });
//		}
//		ASSERT_NO_THROW(iTable.Open());
//
//		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		// Insert a row that we want to delete
//		test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, m_db, 99, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL);
//
//		// Now lets delete that row by pointing the primary key column to it
//		iTable.SetColumnValue(0, (SQLINTEGER) 99);
//		EXPECT_NO_THROW(iTable.Delete());
//
//		// And fetching shall return no results at all
//		iTable.Select();
//		EXPECT_FALSE(iTable.SelectNext());
//	}
//
//


//
//	TEST_F(TableTest, UpdateWhere)
//	{
//		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table iTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE_WHERE, intTypesTableName, L"", L"", L"");
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			iTable.SetColumnPrimaryKeyIndexes({ 0 });
//		}
//		ASSERT_NO_THROW(iTable.Open());
//
//		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		// Insert some values
//		for (int i = 1; i < 10; i++)
//		{
//			test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, m_db, i, i, i, i);
//		}
//
//		// Now update using our WHERE statement. This allows us to update also key rows. Shift all values *(1000)
//		int shift = 1000;
//		for (int i = 1; i < 10; i++)
//		{
//			iTable.SetColumnValue(0, (SQLINTEGER)i * shift);
//			iTable.SetColumnValue(2, (SQLINTEGER)(i * shift));
//			if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//			{
//				iTable.SetColumnValue(1, (SQLINTEGER)(i * shift));
//				iTable.SetColumnNull(3);
//			}
//			else
//			{
//				iTable.SetColumnValue(1, (SQLSMALLINT)(i * shift));
//				iTable.SetColumnValue(3, (SQLBIGINT)(i * shift));
//			}
//
//			wstring sqlstmt = (boost::wformat(L"%s = %d") % idName %i).str();
//			EXPECT_NO_THROW(iTable.Update(sqlstmt));
//		}
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// And select them and compare
//		wstring sqlstmt = (boost::wformat(L"%s > 0 ORDER BY %s") % idName %idName).str();
//		iTable.Select(sqlstmt);
//		for (int i = 1; i < 10; i++)
//		{
//			EXPECT_TRUE(iTable.SelectNext());
//			EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, i * shift, i * shift, i * shift, i * shift));
//		}
//		EXPECT_FALSE(iTable.SelectNext());
//	}


	TEST_F(TableTest, GetColumnBufferIndex)
	{
		std::wstring intTypesTableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_SELECT, intTypesTableName);
		ASSERT_NO_THROW(iTable.Open());

		EXPECT_EQ(0, iTable.GetColumnBufferIndex(L"idintegertypes", false));
		EXPECT_EQ(1, iTable.GetColumnBufferIndex(L"tsmallint", false));
		EXPECT_EQ(2, iTable.GetColumnBufferIndex(L"tint", false));
		EXPECT_EQ(3, iTable.GetColumnBufferIndex(L"tbigint", false));
	}


	TEST_F(TableTest, CreateAutoIntBuffers)
	{
		wstring tableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_SELECT, tableName);
		vector<ColumnBufferPtrVariant> columns = iTable.CreateAutoColumnBufferPtrs(false);
		
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
		wstring tableName = GetTableName(TableId::BLOBTYPES);
		Table bTable(m_pDb, TableAccessFlag::AF_SELECT, tableName);
		vector<ColumnBufferPtrVariant> columns = bTable.CreateAutoColumnBufferPtrs(false);

		EXPECT_EQ(3, columns.size());

		EXPECT_NO_THROW(boost::get<LongColumnBufferPtr>(columns[0]));
		EXPECT_NO_THROW(boost::get<BinaryColumnBufferPtr>(columns[1]));
		EXPECT_NO_THROW(boost::get<BinaryColumnBufferPtr>(columns[2]));

		BinaryColumnBufferPtr pBlob16 = boost::get<BinaryColumnBufferPtr>(columns[1]);
		BinaryColumnBufferPtr pVarblob20 = boost::get<BinaryColumnBufferPtr>(columns[2]);

		EXPECT_EQ(16, pBlob16->GetBufferLength());
		EXPECT_EQ(20, pVarblob20->GetBufferLength());
		EXPECT_EQ(16, pBlob16->GetNrOfElements());
		EXPECT_EQ(20, pVarblob20->GetNrOfElements());
	}


	TEST_F(TableTest, CreateAutoCharBuffers)
	{
		wstring tableName = GetTableName(TableId::CHARTYPES);
		Table cTable(m_pDb, TableAccessFlag::AF_SELECT, tableName);

		// Enforce binding to SqlWChar - in sql server we have nvarchar and varchar,
		// db2 seems to default to cchar - we simplfiy the test by binding those to wchar too.
		auto pTypeMap = std::make_shared<DefaultSql2BufferMap>(OdbcVersion::V_3);
		pTypeMap->RegisterType(SQL_VARCHAR, SQL_C_WCHAR);
		pTypeMap->RegisterType(SQL_CHAR, SQL_C_WCHAR);
		cTable.SetSql2BufferTypeMap(pTypeMap);
		vector<ColumnBufferPtrVariant> columns = cTable.CreateAutoColumnBufferPtrs(false);

		EXPECT_EQ(5, columns.size());

		SqlCTypeVisitor cTypeV;

		EXPECT_EQ(SQL_C_SLONG, boost::apply_visitor(cTypeV, columns[0]));
		EXPECT_EQ(SQL_C_WCHAR, boost::apply_visitor(cTypeV, columns[1]));
		EXPECT_EQ(SQL_C_WCHAR, boost::apply_visitor(cTypeV, columns[2]));
		EXPECT_EQ(SQL_C_WCHAR, boost::apply_visitor(cTypeV, columns[3]));
		EXPECT_EQ(SQL_C_WCHAR, boost::apply_visitor(cTypeV, columns[4]));

		WCharColumnBufferPtr pVarchar128 = boost::get<WCharColumnBufferPtr>(columns[1]);
		WCharColumnBufferPtr pChar128 = boost::get<WCharColumnBufferPtr>(columns[2]);
		WCharColumnBufferPtr pVarchar10 = boost::get<WCharColumnBufferPtr>(columns[3]);
		WCharColumnBufferPtr pChar10 = boost::get<WCharColumnBufferPtr>(columns[4]);

		EXPECT_EQ(128 + 1, pVarchar128->GetNrOfElements());
		EXPECT_EQ(128 + 1, pChar128->GetNrOfElements());
		EXPECT_EQ(10 + 1, pVarchar10->GetNrOfElements());
		EXPECT_EQ(10 + 1, pChar10->GetNrOfElements());
	}


	TEST_F(TableTest, CreateAutoDateTypeBuffers)
	{
		wstring tableName = GetTableName(TableId::DATETYPES);
		Table dTable(m_pDb, TableAccessFlag::AF_SELECT, tableName);

		auto columns = dTable.CreateAutoColumnBufferPtrs(false);
		EXPECT_EQ(4, columns.size());

		SqlCTypeVisitor cTypeV;

		EXPECT_EQ(SQL_C_SLONG, boost::apply_visitor(cTypeV, columns[0]));
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			// Access reports only Timestamps
			EXPECT_EQ(SQL_C_TYPE_TIMESTAMP, boost::apply_visitor(cTypeV, columns[1]));
			EXPECT_EQ(SQL_C_TYPE_TIMESTAMP, boost::apply_visitor(cTypeV, columns[2]));
		}
		else
		{
			EXPECT_EQ(SQL_C_TYPE_DATE, boost::apply_visitor(cTypeV, columns[1]));
			EXPECT_EQ(SQL_C_TYPE_TIME, boost::apply_visitor(cTypeV, columns[2]));
		}
		EXPECT_EQ(SQL_C_TYPE_TIMESTAMP, boost::apply_visitor(cTypeV, columns[3]));

		TypeTimestampColumnBufferPtr pTs = boost::get<TypeTimestampColumnBufferPtr>(columns[3]);
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			EXPECT_EQ(3, pTs->GetDecimalDigits());
		}
		else if (m_pDb->GetDbms() == DatabaseProduct::DB2)
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
		wstring tableName = GetTableName(TableId::FLOATTYPES);
		Table dTable(m_pDb, TableAccessFlag::AF_SELECT, tableName);

		auto columns = dTable.CreateAutoColumnBufferPtrs(false);
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
		wstring tableName = GetTableName(TableId::NUMERICTYPES);
		Table nTable(m_pDb, TableAccessFlag::AF_SELECT, tableName);

		auto columns = nTable.CreateAutoColumnBufferPtrs(false);
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

		wstring tableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_SELECT, tableName);
		iTable.SetSql2BufferTypeMap(pTypeMap);
		auto columns = iTable.CreateAutoColumnBufferPtrs(true);

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

		wstring tableName = GetTableName(TableId::INTEGERTYPES);
		Table iTable(m_pDb, TableAccessFlag::AF_SELECT, tableName);
		iTable.SetSql2BufferTypeMap(pTypeMap);

		EXPECT_THROW(iTable.CreateAutoColumnBufferPtrs(false), NotSupportedException);
	}

// Interfaces
// ----------

} // namespace exodbc
