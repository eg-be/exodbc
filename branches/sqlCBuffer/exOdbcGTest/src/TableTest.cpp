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


//	TEST_F(TableTest, OpenManualCheckColumnFlagSelect)
//	{
//		// Open a table manually but do not set the Select flag for all columns
//		Table iTable(&m_db, AF_SELECT, test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase), L"", L"", L"");
//		SQLINTEGER id = 0;
//		SQLSMALLINT si = 0;
//		SQLINTEGER i = 0;
//		SQLBIGINT bi = 0;
//		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT);
//		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_NULLABLE);
//		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_NONE | CF_NULLABLE);
//		iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_NULLABLE);
//
//		ASSERT_NO_THROW(iTable.Open());
//		// We expect all columnBuffers to be bound, except nr 2
//		ColumnBuffer* pBuffId = iTable.GetColumnVariant(0);
//		ColumnBuffer* pBuffsi = iTable.GetColumnVariant(1);
//		ColumnBuffer* pBuffi = iTable.GetColumnVariant(2);
//		ColumnBuffer* pBuffbi = iTable.GetColumnVariant(3);
//		EXPECT_TRUE(pBuffId->IsBound());
//		EXPECT_TRUE(pBuffsi->IsBound());
//		EXPECT_FALSE(pBuffi->IsBound());
//		EXPECT_TRUE(pBuffbi->IsBound());
//
//		// And we should be able to select a row
//		wstring sqlstmt = boost::str(boost::wformat(L"%s = 7") % test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase));
//		iTable.Select(sqlstmt);
//		EXPECT_TRUE(iTable.SelectNext());
//		// and have all values
//		EXPECT_EQ(7, id);
//		EXPECT_EQ(-13, si);
//		EXPECT_EQ(0, i);
//		EXPECT_EQ(10502, bi);
//		{
//			LogLevelFatal llf;
//			DontDebugBreak ddb;
//			// except the not bound column, we are unable to get its value, but its buffer has not changed
//			EXPECT_THROW(boost::get<SQLINTEGER>(iTable.GetColumnValue(2)), AssertionException);
//		}
//	}
//
//
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


	TEST_F(TableTest, OpenAutoCheckPrivs)
	{
		// Test to open read-only a table we know we have all rights:
		std::wstring tableName = GetTableName(TableId::INTEGERTYPES);
		exodbc::Table rTable(m_pDb, TableAccessFlag::AF_READ, tableName, L"", L"", L"");
		EXPECT_NO_THROW(rTable.Open(TableOpenFlag::TOF_CHECK_PRIVILEGES));

		// Test to open read-write a table we know we have all rights:
		exodbc::Table rTable2(m_pDb, TableAccessFlag::AF_READ_WRITE, tableName, L"", L"", L"");
		EXPECT_NO_THROW(rTable2.Open(TableOpenFlag::TOF_CHECK_PRIVILEGES));

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
			EXPECT_NO_THROW(table2.Open(TableOpenFlag::TOF_CHECK_PRIVILEGES));

			// We expect to fail if trying to open for writing
			table2.Close();
			table2.SetAccessFlags(TableAccessFlag::AF_READ_WRITE);
			EXPECT_THROW(table2.Open(TableOpenFlag::TOF_CHECK_PRIVILEGES), Exception);

			// But we do not fail if we do not check the privs
			EXPECT_NO_THROW(table2.Open());

			// Try to open one we do not even have the rights for
			std::wstring table3Name = GetTableName(TableId::NUMERICTYPES);
			Table table3(pDb, TableAccessFlag::AF_READ, table3Name, L"", L"", L"");
			EXPECT_THROW(table3.Open(), Exception);
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


	//TEST_F(TableTest, OpenAutoWithUnsupportedColumn)
	//{
	//	std::wstring tableName = GetTableName(TableId::NOT_SUPPORTED);
	//	exodbc::Table nst(m_pDb, TableAccessFlag::AF_READ_WRITE, tableName, L"", L"", L"");

	//	// Expect to fail if we open with default flags
	//	ASSERT_THROW(nst.Open(), NotSupportedException);
	//	// But not if we pass the flag to skip
	//	{
	//		LogLevelFatal llf;
	//		ASSERT_NO_THROW(nst.Open(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS));
	//	}

	//	// We should now be able to select from column indexed 0 (id), 1 (int1) and 3 (int2) - 2 (xml) should be missing
	//	EXPECT_NO_THROW(nst.Select());
	//	EXPECT_TRUE(nst.SelectNext());
	//	SQLINTEGER id, int1, int2;
	//	EXPECT_NO_THROW(id = nst.GetInt(0));
	//	EXPECT_NO_THROW(int1 = nst.GetInt(1));
	//	EXPECT_FALSE(nst.ColumnBufferExists(2));
	//	EXPECT_THROW(nst.GetColumnVariant(2), IllegalArgumentException);
	//	EXPECT_NO_THROW(int2 = nst.GetInt(3));
	//	EXPECT_EQ(1, id);
	//	EXPECT_EQ(10, int1);
	//	EXPECT_EQ(12, int2);
	//}


//	TEST_F(TableTest, SelectFromAutoWithUnsupportedColumn)
//	{
//
//
//		std::wstring tableName = test::GetTableName(test::TableId::NOT_SUPPORTED, m_odbcInfo.m_namesCase);
//		exodbc::Table nst(&m_db, AF_READ_WRITE, tableName, L"", L"", L"");
//
//		// we do not if we pass the flag to skip
//		{
//			LogLevelFatal llf;
//			EXPECT_NO_THROW(nst.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
//		}
//
//		// We should now be able to select from column indexed 0 (id), 1 (int1) and 3 (int2) - 2 (xml) should be missing
//		EXPECT_NO_THROW(nst.Select());
//		EXPECT_TRUE(nst.SelectNext());
//		SQLINTEGER id, int1, int2;
//		EXPECT_NO_THROW(id = nst.GetInt(0));
//		EXPECT_NO_THROW(int1 = nst.GetInt(1));
//		EXPECT_THROW(nst.GetColumnVariant(2), IllegalArgumentException);
//		EXPECT_NO_THROW(int2 = nst.GetInt(3));
//		EXPECT_EQ(1, id);
//		EXPECT_EQ(10, int1);
//		EXPECT_EQ(12, int2);
//	}
//
//
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
			DontDebugBreak ddb;
			LogLevelFatal llf;
			EXPECT_THROW(iTable.Close(), AssertionException);
		}

		// Open a Table read-only
		ASSERT_NO_THROW(iTable.Open());

		// Close must work
		EXPECT_NO_THROW(iTable.Close());

		// Close an already closed table
		{
			DontDebugBreak ddb;
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
			DontDebugBreak ddb;
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
//	// Select / GetNext
//	// ----------------
//	TEST_F(TableTest, Select)
//	{
//		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//
//		EXPECT_NO_THROW(iTable.Select(L""));
//
//		iTable.SelectClose();
//	}
//
//
//	TEST_F(TableTest, SelectFirst)
//	{
//		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		std::wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//		// We expect some Records
//		std::wstring sqlWhere = boost::str(boost::wformat(L"%s >= 2 ORDER BY %s ASC") % idName % idName);
//		ASSERT_NO_THROW(iTable.Select(sqlWhere));
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 4, test::ValueIndicator::IS_NULL, 2147483647, test::ValueIndicator::IS_NULL));
//
//		// now Select the first again
//		EXPECT_TRUE(iTable.SelectFirst());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 2, 32767, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL));
//	}
//
//
//	TEST_F(TableTest, SelectLast)
//	{
//		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		std::wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//		// We expect some Records
//		std::wstring sqlWhere = boost::str(boost::wformat(L"%s >= 2 ORDER BY %s ASC") % idName % idName);
//		ASSERT_NO_THROW(iTable.Select(sqlWhere));
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 4, test::ValueIndicator::IS_NULL, 2147483647, test::ValueIndicator::IS_NULL));
//
//		// now Select the last record
//		EXPECT_TRUE(iTable.SelectLast());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 7, -13, 26, 10502));
//	}
//
//	TEST_F(TableTest, SelectAbsolute)
//	{
//		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		std::wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//		// We expect some Records
//		std::wstring sqlWhere = boost::str(boost::wformat(L"%s >= 2 ORDER BY %s ASC") % idName % idName);
//		ASSERT_NO_THROW(iTable.Select(sqlWhere));
//		EXPECT_TRUE(iTable.SelectAbsolute(3));
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 4, test::ValueIndicator::IS_NULL, 2147483647, test::ValueIndicator::IS_NULL));
//
//		// Select something not in result set
//		EXPECT_FALSE(iTable.SelectAbsolute(20));
//	}
//
//
//	TEST_F(TableTest, SelectRelative)
//	{
//		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		std::wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//		// We expect some Records
//		std::wstring sqlWhere = boost::str(boost::wformat(L"%s >= 2 ORDER BY %s ASC") % idName % idName);
//		ASSERT_NO_THROW(iTable.Select(sqlWhere));
//		// Note: For MySQL to be able to select relative, a record must be selected first. Else, SelectRelative will choose wrong offset
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 2, 32767, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL));
//		EXPECT_TRUE(iTable.SelectRelative(1));
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 3, test::ValueIndicator::IS_NULL, (-2147483647 - 1), test::ValueIndicator::IS_NULL));
//		// Move by one forward
//		EXPECT_TRUE(iTable.SelectRelative(1));
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 4, test::ValueIndicator::IS_NULL, 2147483647, test::ValueIndicator::IS_NULL));
//		// And one back again
//		EXPECT_TRUE(iTable.SelectRelative(-1));
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 3, test::ValueIndicator::IS_NULL, (-2147483647 - 1), test::ValueIndicator::IS_NULL));
//
//		// Select something not in result set
//		EXPECT_FALSE(iTable.SelectRelative(20));
//	}
//
//
//	TEST_F(TableTest, SelectPrev)
//	{
//		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		std::wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//		
//		// We expect some Records
//		std::wstring sqlWhere = boost::str(boost::wformat(L"%s >= 2 ORDER BY %s ASC") % idName % idName);
//		ASSERT_NO_THROW(iTable.Select(sqlWhere));
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 2, 32767, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL));
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 3, test::ValueIndicator::IS_NULL, (-2147483647 - 1), test::ValueIndicator::IS_NULL));
//
//		// now Select the prev record - we have the first again
//		EXPECT_TRUE(iTable.SelectPrev());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 2, 32767, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL));
//		// no more prev records available:
//		EXPECT_FALSE(iTable.SelectPrev());
//	}
//
//
//	TEST_F(TableTest, SelectNext)
//	{
//		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//
//		// We expect 7 Records
//		iTable.Select(L"");
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_FALSE(iTable.SelectNext());
//
//		iTable.SelectClose();
//	}
//
//
//	TEST_F(TableTest, SelectClose)
//	{
//		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//		
//		// Do something that opens a transaction
//		iTable.Select(L"");
//		EXPECT_NO_THROW(iTable.SelectClose());
//		// We should be closed now
//		EXPECT_FALSE(iTable.IsSelectOpen());
//	}
//
//
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
//	// Count
//	// -----
//	TEST_F(TableTest, Count)
//	{
//		Table table(&m_db, AF_READ, test::GetTableName(test::TableId::FLOATTYPES, m_odbcInfo.m_namesCase), L"", L"", L"");
//		ASSERT_NO_THROW(table.Open());
//
//		SQLUBIGINT all;
//		EXPECT_NO_THROW(all = table.Count(L""));
//		EXPECT_EQ(6, all);
//
//		// \todo: Check some really big table, with billions of rows
//		SQLUBIGINT some;
//		std::wstring whereStmt = L"tdouble > 0";
//		if (m_db.GetDbms() == DatabaseProduct::DB2)
//		{
//			whereStmt = L"TDOUBLE > 0";
//		}
//		EXPECT_NO_THROW(some = table.Count(whereStmt));
//		EXPECT_EQ(1, some);
//		whereStmt = L"tdouble > 0 OR tfloat > 0";
//		if (m_db.GetDbms() == DatabaseProduct::DB2)
//		{
//			whereStmt = L"TDOUBLE > 0 OR TFLOAT > 0";
//		}
//		EXPECT_NO_THROW(some = table.Count(whereStmt));
//		EXPECT_EQ(2, some);
//
//		// we should fail with an exception if we query using a non-sense where-stmt
//		EXPECT_THROW(table.Count(L"a query that is invalid for sure"), Exception);
//	}
//
//
//	// GetValues
//	// ---------
//	TEST_F(TableTest, SelectAutoIntValues)
//	{
//		namespace tt = test;
//		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//
//		std::wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//
//		// The first column has a smallint set, we can read that as any int value -32768
//		iTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 1, -32768, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL));
//		iTable.SelectClose();
//
//		iTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 2, 32767, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL));
//		iTable.SelectClose();
//
//		// The 2nd column has a int set, we can read that as int or bigint value -2147483648
//		iTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 3, test::ValueIndicator::IS_NULL, (-2147483647 - 1), test::ValueIndicator::IS_NULL));
//		iTable.SelectClose();
//
//		iTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 4, test::ValueIndicator::IS_NULL, 2147483647, test::ValueIndicator::IS_NULL));
//		iTable.SelectClose();
//
//		// The 3rd column has a bigint set, we can read that as bigint value -9223372036854775808
//		iTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 5, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL, (-9223372036854775807 - 1)));
//		iTable.SelectClose();
//
//		iTable.Select((boost::wformat(L"%s = 6") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 6, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL, 9223372036854775807));
//		iTable.SelectClose();
//	}
//
//
//	TEST_F(TableTest, SelectManualIntValues)
//	{
//		MIntTypesTable iTable(&m_db, m_odbcInfo.m_namesCase);
//		ASSERT_NO_THROW(iTable.Open());
//
//		// Just check that the buffers are correct
//		// do not check using the IsIntRecordEqual: Here we bind the smallint column to a SQLSMALLINT with Access, 
//		// this works as the driver can convert it. But in auto we bind as the reported SQLINTEGER type, and our
//		// test-helpers have some problems as they assume its reported as SQLINTEGER..
//		std::wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		iTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_EQ(-32768, iTable.m_smallInt);
//
//		iTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_EQ(32767, iTable.m_smallInt);
//
//		iTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_EQ(INT_MIN, iTable.m_int);
//
//		iTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_EQ(2147483647, iTable.m_int);
//
//		// note that Access has no BigInt and those values are set to NULL
//		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
//		{
//			iTable.Select((boost::wformat(L"%s = 5") % idName).str());
//			EXPECT_TRUE(iTable.SelectNext());
//			EXPECT_EQ((-9223372036854775807 - 1), iTable.m_bigInt);
//
//			iTable.Select((boost::wformat(L"%s = 6") % idName).str());
//			EXPECT_TRUE(iTable.SelectNext());
//			EXPECT_EQ(9223372036854775807, iTable.m_bigInt);
//		}
//	}
//
//
//	TEST_F(TableTest, SelectWCharIntValues)
//	{
//		// And some tables with auto-columns
//		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		Table iTable(&m_db, AF_READ, intTypesTableName, L"", L"", L"");
//		iTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharSql2BufferMap()));
//		ASSERT_NO_THROW(iTable.Open());
//
//		std::wstring id, smallInt, i, bigInt;
//		std::wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		iTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_NO_THROW(smallInt = iTable.GetWString(1));
//		EXPECT_EQ(L"-32768", smallInt);
//
//		iTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_NO_THROW(smallInt = iTable.GetWString(1));
//		EXPECT_EQ(L"32767", smallInt);
//
//		iTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_NO_THROW(i = iTable.GetWString(2));
//		EXPECT_EQ(L"-2147483648", i);
//
//		iTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_NO_THROW(i = iTable.GetWString(2));
//		EXPECT_EQ(L"2147483647", i);
//
//		// No bigints on access
//		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
//		{
//			iTable.Select((boost::wformat(L"%s = 5") % idName).str());
//			EXPECT_TRUE(iTable.SelectNext());
//			EXPECT_NO_THROW(bigInt = iTable.GetWString(3));
//			EXPECT_EQ(L"-9223372036854775808", bigInt);
//
//			iTable.Select((boost::wformat(L"%s = 6") % idName).str());
//			EXPECT_TRUE(iTable.SelectNext());
//			EXPECT_NO_THROW(bigInt = iTable.GetWString(3));
//			EXPECT_EQ(L"9223372036854775807", bigInt);
//		}
//	}
//
//
//	TEST_F(TableTest, SelectAutoFloatValues)
//	{
//		wstring floatTypesTableName = test::GetTableName(test::TableId::FLOATTYPES, m_odbcInfo.m_namesCase);
//		Table fTable(&m_db, AF_READ, floatTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(fTable.Open());
//
//		SQLDOUBLE val;
//		wstring str;
//		std::wstring idName = test::GetIdColumnName(test::TableId::FLOATTYPES, m_odbcInfo.m_namesCase);
//		fTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(val = fTable.GetDouble(2));
//		EXPECT_EQ(0.0, val);
//
//		fTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(val = fTable.GetDouble(2));
//		EXPECT_EQ((int)(1e3 * 3.141), (int)(1e3 * val));
//
//		fTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(val = fTable.GetDouble(2));
//		EXPECT_EQ((int)(1e3 * -3.141), (int)(1e3 * val));
//
//		fTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(val = fTable.GetDouble(1));
//		EXPECT_EQ(0.0, val);
//
//		fTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(val = fTable.GetDouble(1));
//		EXPECT_EQ((int)(1e6 * 3.141592), (int)(1e6 * val));
//
//		fTable.Select((boost::wformat(L"%s = 6") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(val = fTable.GetDouble(1));
//		EXPECT_EQ((int)(1e6 * -3.141592), (int)(1e6 * val));
//	}
//
//
//	TEST_F(TableTest, SelectManualFloatValues)
//	{
//		MFloatTypesTable fTable(&m_db, m_odbcInfo.m_namesCase);
//		ASSERT_NO_THROW(fTable.Open());
//
//		std::wstring idName = test::GetIdColumnName(test::TableId::FLOATTYPES, m_odbcInfo.m_namesCase);
//		fTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_EQ(0.0, fTable.m_float);
//
//		fTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_EQ((int)(1e3 * 3.141), (int)(1e3 * fTable.m_float));
//
//		fTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_EQ((int)(1e3 * -3.141), (int)(1e3 * fTable.m_float));
//
//		fTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			EXPECT_EQ(0.0, fTable.m_doubleAsFloat);
//		}
//		else
//		{
//			EXPECT_EQ(0.0, fTable.m_double);
//		}
//
//		fTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			EXPECT_EQ(int(1e6 * 3.141592), int(1e6 * fTable.m_doubleAsFloat));
//		}
//		else
//		{
//			EXPECT_EQ((int)(1e6 * 3.141592), int(1e6 * fTable.m_double));
//		}
//
//		fTable.Select((boost::wformat(L"%s = 6") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			EXPECT_EQ((int)(1e6 * -3.141592), (int)(1e6 * fTable.m_doubleAsFloat));
//		}
//		else
//		{
//			EXPECT_EQ((int)(1e6 * -3.141592), (int)(1e6 * fTable.m_double));
//		}
//	}
//
//
//	TEST_F(TableTest, SelectWCharFloatValues)
//	{
//		wstring floatTypesTableName = test::GetTableName(test::TableId::FLOATTYPES, m_odbcInfo.m_namesCase);
//		Table fTable(&m_db, AF_READ, floatTypesTableName, L"", L"", L"");
//		fTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharSql2BufferMap()));
//		ASSERT_NO_THROW(fTable.Open());
//
//		wstring sVal;
//		wstring idName = test::GetIdColumnName(test::TableId::FLOATTYPES, m_odbcInfo.m_namesCase);
//		fTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(sVal = fTable.GetWString(2));
//		//EXPECT_EQ(0.0, sVal);
//		
//		// TODO: Find a way to compare without having to diff for every db-type
//
//		fTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(sVal = fTable.GetWString(2));
//
//		fTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(sVal = fTable.GetWString(2));
//
//		fTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(sVal = fTable.GetWString(1));
//		
//		fTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(sVal = fTable.GetWString(1));
//
//		fTable.Select((boost::wformat(L"%s = 6") % idName).str());
//		EXPECT_TRUE(fTable.SelectNext());
//		EXPECT_NO_THROW(sVal = fTable.GetWString(1));
//	}
//
//
//	TEST_F(TableTest, SelectAutoWCharValues)
//	{
//		std::wstring charTypesTableName = test::GetTableName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
//		Table charTypesAutoTable(&m_db, AF_READ, charTypesTableName, L"", L"", L"");
//		charTypesAutoTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new CharAsWCharSql2BufferMap(m_db.GetMaxSupportedOdbcVersion())));
//		ASSERT_NO_THROW(charTypesAutoTable.Open());
//		// We want to trim on the right side for DB2 and sql server
//		charTypesAutoTable.SetCharTrimRight(true);
//
//		std::wstring str;
//
//		// We expect 6 Records
//		std::wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
//		charTypesAutoTable.Select((boost::wformat(L"%d = 1") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(1));
//		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);
//		EXPECT_FALSE(charTypesAutoTable.SelectNext());
//		charTypesAutoTable.SelectClose();
//
//		charTypesAutoTable.Select((boost::wformat(L"%d = 2") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(2));
//		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);
//
//		EXPECT_FALSE(charTypesAutoTable.SelectNext());
//		charTypesAutoTable.SelectClose();
//
//		charTypesAutoTable.Select((boost::wformat(L"%d = 3") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(1));
//		EXPECT_EQ(std::wstring(L"������"), str);
//
//		EXPECT_FALSE(charTypesAutoTable.SelectNext());
//		charTypesAutoTable.SelectClose();
//
//		charTypesAutoTable.Select((boost::wformat(L"%d = 4") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(2));
//		EXPECT_EQ(std::wstring(L"������"), str);
//
//		charTypesAutoTable.Select((boost::wformat(L"%d = 5") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(3));
//		EXPECT_EQ(std::wstring(L"abc"), str);
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(4));
//		EXPECT_EQ(std::wstring(L"abc"), str);
//
//		charTypesAutoTable.Select((boost::wformat(L"%d = 6") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(3));
//		EXPECT_EQ(std::wstring(L"abcde12345"), str);
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(4));
//		EXPECT_EQ(std::wstring(L"abcde12345"), str);
//
//		charTypesAutoTable.SelectClose();
//	}
//
//
//	TEST_F(TableTest, SelectManualWCharValues)
//	{
//		MWCharTypesTable wTable(&m_db, m_odbcInfo.m_namesCase);
//		ASSERT_NO_THROW(wTable.Open());
//
//		// Just test the values in the buffer - trimming has no effect here
//		using namespace boost::algorithm;
//		std::wstring str;
//		std::wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
//		wTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(wTable.SelectNext());
//		str.assign((wchar_t*) &wTable.m_varchar);
//		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
//		EXPECT_FALSE(wTable.SelectNext());
//		wTable.SelectClose();
//
//		wTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(wTable.SelectNext());
//		str.assign((wchar_t*)&wTable.m_char);
//		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
//		EXPECT_FALSE(wTable.SelectNext());
//		wTable.SelectClose();
//
//		wTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(wTable.SelectNext());
//		str.assign((wchar_t*)&wTable.m_varchar);
//		EXPECT_EQ(std::wstring(L"������"), trim_right_copy(str));
//		EXPECT_FALSE(wTable.SelectNext());
//		wTable.SelectClose();
//
//		wTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(wTable.SelectNext());
//		str.assign((wchar_t*)&wTable.m_char);
//		EXPECT_EQ(std::wstring(L"������"), trim_right_copy(str));
//		EXPECT_FALSE(wTable.SelectNext());
//		wTable.SelectClose();
//
//		wTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(wTable.SelectNext());
//		str.assign((wchar_t*)&wTable.m_char_10);
//		EXPECT_EQ(std::wstring(L"abc"), trim_right_copy(str));
//		str.assign((wchar_t*)&wTable.m_varchar_10);
//		EXPECT_EQ(std::wstring(L"abc"), trim_right_copy(str));
//		wTable.SelectClose();
//
//		wTable.Select((boost::wformat(L"%s = 6") % idName).str());
//		EXPECT_TRUE(wTable.SelectNext());
//		str.assign((wchar_t*)&wTable.m_char_10);
//		EXPECT_EQ(std::wstring(L"abcde12345"), trim_right_copy(str));
//		str.assign((wchar_t*)&wTable.m_varchar_10);
//		EXPECT_EQ(std::wstring(L"abcde12345"), trim_right_copy(str));
//		wTable.SelectClose();
//
//	}
//
//	
//	TEST_F(TableTest, SelectAutoCharValues)
//	{
//		std::wstring charTypesTableName = test::GetTableName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
//		Table charTypesAutoTable(&m_db, AF_READ, charTypesTableName, L"", L"", L"");
//		charTypesAutoTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharAsCharSql2BufferMap(m_db.GetMaxSupportedOdbcVersion())));
//		ASSERT_NO_THROW(charTypesAutoTable.Open());
//		// We want to trim on the right side for DB2 and also sql server
//		charTypesAutoTable.SetCharTrimRight(true);
//
//		std::string str;
//
//		std::wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
//		charTypesAutoTable.Select((boost::wformat(L"%d = 1") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(1));
//		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);
//		EXPECT_FALSE(charTypesAutoTable.SelectNext());
//		charTypesAutoTable.SelectClose();
//
//		charTypesAutoTable.Select((boost::wformat(L"%d = 2") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(2));
//		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);
//		
//		EXPECT_FALSE(charTypesAutoTable.SelectNext());
//		charTypesAutoTable.SelectClose();
//
//		charTypesAutoTable.Select((boost::wformat(L"%d = 3") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(1));
//		EXPECT_EQ(std::string("������"), str);
//
//		EXPECT_FALSE(charTypesAutoTable.SelectNext());
//		charTypesAutoTable.SelectClose();
//
//		charTypesAutoTable.Select((boost::wformat(L"%d = 4") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(2));
//		EXPECT_EQ(std::string("������"), str);
//
//		charTypesAutoTable.Select((boost::wformat(L"%d = 5") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(3));
//		EXPECT_EQ(std::string("abc"), str);
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(4));
//		EXPECT_EQ(std::string("abc"), str);
//
//		charTypesAutoTable.Select((boost::wformat(L"%d = 6") % idName).str());
//		EXPECT_TRUE(charTypesAutoTable.SelectNext());
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(3));
//		EXPECT_EQ(std::string("abcde12345"), str);
//		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(4));
//		EXPECT_EQ(std::string("abcde12345"), str);
//
//		charTypesAutoTable.SelectClose();
//	}
//
//
//	TEST_F(TableTest, SelectManualCharValues)
//	{
//		MCharTypesTable cTable(&m_db, m_odbcInfo.m_namesCase);
//		ASSERT_NO_THROW(cTable.Open());
//
//		// Just test the values in the buffer - trimming has no effect here
//		using namespace boost::algorithm;
//		std::string str;
//		std::wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
//		cTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(cTable.SelectNext());
//		str.assign((char*)&cTable.m_varchar);
//		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
//		EXPECT_FALSE(cTable.SelectNext());
//		cTable.SelectClose();
//
//		cTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(cTable.SelectNext());
//		str.assign((char*)&cTable.m_char);
//		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
//		EXPECT_FALSE(cTable.SelectNext());
//		cTable.SelectClose();
//
//		cTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(cTable.SelectNext());
//		str.assign((char*)&cTable.m_varchar);
//		EXPECT_EQ(std::string("������"), trim_right_copy(str));
//		EXPECT_FALSE(cTable.SelectNext());
//		cTable.SelectClose();
//
//		cTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(cTable.SelectNext());
//		str.assign((char*)&cTable.m_char);
//		EXPECT_EQ(std::string("������"), trim_right_copy(str));
//		EXPECT_FALSE(cTable.SelectNext());
//		cTable.SelectClose();
//
//		cTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(cTable.SelectNext());
//		str.assign((char*)&cTable.m_char_10);
//		EXPECT_EQ(std::string("abc"), trim_right_copy(str));
//		str.assign((char*)&cTable.m_varchar_10);
//		EXPECT_EQ(std::string("abc"), trim_right_copy(str));
//		cTable.SelectClose();
//
//		cTable.Select((boost::wformat(L"%s = 6") % idName).str());
//		EXPECT_TRUE(cTable.SelectNext());
//		str.assign((char*)&cTable.m_char_10);
//		EXPECT_EQ(std::string("abcde12345"), trim_right_copy(str));
//		str.assign((char*)&cTable.m_varchar_10);
//		EXPECT_EQ(std::string("abcde12345"), trim_right_copy(str));
//		cTable.SelectClose();
//	}
//	
//
//	TEST_F(TableTest, SelectAutoDateValues)
//	{
//		// Note how to read fractions:
//		// [b]   The value of the fraction field is the number of billionths of a second and ranges from 0 through 999,999,999 (1 less than 1 billion). 
//		// For example, the value of the fraction field for a half-second is 500,000,000, for a thousandth of a second (one millisecond) is 1,000,000, 
//		// for a millionth of a second (one microsecond) is 1,000, and for a billionth of a second (one nanosecond) is 1.
//
//		std::wstring dateTypesTableName = test::GetTableName(test::TableId::DATETYPES, m_odbcInfo.m_namesCase);
//		Table dTable(&m_db, AF_READ, dateTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(dTable.Open());
//
//		using namespace boost::algorithm;
//		std::wstring str;
//		SQL_DATE_STRUCT date;
//		SQL_TIME_STRUCT time;
//		SQL_TIMESTAMP_STRUCT timestamp;
//		std::wstring idName = test::GetIdColumnName(test::TableId::DATETYPES, m_odbcInfo.m_namesCase);
//		dTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		{
//			// Microsoft will info here (which we report as warning) that the time field has been truncated (as we do not use the fancy TIME2 struct)
//			LogLevelError llE;
//			EXPECT_TRUE(dTable.SelectNext());
//		}
//		EXPECT_NO_THROW(date = dTable.GetDate(1));
//		EXPECT_NO_THROW(time = dTable.GetTime(2));
//		EXPECT_NO_THROW(timestamp = dTable.GetTimeStamp(3));
//
//		EXPECT_EQ(26, date.day);
//		EXPECT_EQ(1, date.month);
//		EXPECT_EQ(1983, date.year);
//		
//		EXPECT_EQ(13, time.hour);
//		EXPECT_EQ(55, time.minute);
//		EXPECT_EQ(56, time.second);
//
//		EXPECT_EQ(26, timestamp.day);
//		EXPECT_EQ(1, timestamp.month);
//		EXPECT_EQ(1983, timestamp.year);
//		EXPECT_EQ(13, timestamp.hour);
//		EXPECT_EQ(55, timestamp.minute);
//		EXPECT_EQ(56, timestamp.second);
//		
//		// Test some digits stuff
//		dTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(dTable.SelectNext());
//		EXPECT_NO_THROW(timestamp = dTable.GetTimeStamp(3));
//		// In IBM DB2 we have 6 digits for the fractions of a timestamp 123456 turns into 123'456'000
//		if (m_db.GetDbms() == DatabaseProduct::DB2)
//		{
//			EXPECT_EQ(123456000, timestamp.fraction);
//		}
//		else if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			// ms has 3 digits 123 turns into 123'000'000
//			EXPECT_EQ(123000000, timestamp.fraction);
//		}
//		else
//		{
//			EXPECT_EQ(0, timestamp.fraction);
//		}
//
//		dTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(dTable.SelectNext());
//		EXPECT_NO_THROW(timestamp = dTable.GetTimeStamp(3));
//		if ( ! (m_db.GetDbms() == DatabaseProduct::MY_SQL || m_db.GetDbms() == DatabaseProduct::ACCESS))
//		{
//			EXPECT_EQ(10000000, timestamp.fraction);
//		}
//
//
//		// MS SQL Server has a new SQL_TIME2_STRUCT if ODBC version is >= 3.8
//#if HAVE_MSODBCSQL_H
//		Environment env38(OdbcVersion::V_3_8);
//		EXPECT_TRUE(env38.HasEnvironmentHandle());
//		Database db38(&env38);
//		EXPECT_TRUE(db38.HasConnectionHandle());
//		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
//		{
//			// My SQL does not support 3.8, the database will warn about a version-mismatch and fall back to 3.0. we know about that.
//			// \todo: Open a ticket
//			LogLevelError llE;
//			if (m_odbcInfo.HasConnectionString())
//			{
//				EXPECT_NO_THROW(db38.Open(m_odbcInfo.m_connectionString));
//			}
//			else
//			{
//				EXPECT_NO_THROW(db38.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
//			}
//		}
//		else
//		{
//			if (m_odbcInfo.HasConnectionString())
//			{
//				EXPECT_NO_THROW(db38.Open(m_odbcInfo.m_connectionString));
//			}
//			else
//			{
//				EXPECT_NO_THROW(db38.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
//			}
//		}
//		Table dTable38(&db38, AF_READ, dateTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(dTable38.Open());
//		dTable38.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(dTable38.SelectNext());
//		EXPECT_NO_THROW(time = dTable38.GetTime(2));
//		EXPECT_EQ(13, time.hour);
//		EXPECT_EQ(55, time.minute);
//		EXPECT_EQ(56, time.second);
//
//		// We should be able to read the value as TIME2 struct
//		SQL_SS_TIME2_STRUCT t2;
//		EXPECT_NO_THROW(t2 = dTable38.GetTime2(2));
//		EXPECT_EQ(13, t2.hour);
//		EXPECT_EQ(55, t2.minute);
//		EXPECT_EQ(56, t2.second);
//		if (db38.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			// MS should also have fractions, configurable, here set to 7: 1234567 -> 123'456'700
//			EXPECT_TRUE(db38.GetMaxSupportedOdbcVersion() >= OdbcVersion::V_3_8);
//			EXPECT_EQ(123456700, t2.fraction);
//		}
//		else
//		{
//			// For the other DBs we expect the fraction part to be 0, as we did not read it
//			// from a TIME2 struct, but from a normal TIME struct, without any fractions
//			EXPECT_EQ(0, t2.fraction);
//		}
//#endif
//	}
//
//
//	TEST_F(TableTest, SelectManualDateValues)
//	{
//		MDateTypesTable cTable(&m_db, m_odbcInfo.m_namesCase);
//		// MS SQL Server does not report time as supported, but their extension TIME2 - but this will work only if odbc-version is >= 3.8
//		// So, if we have a sql-server, we must ignore the supported-db-types
//		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			ASSERT_NO_THROW(cTable.Open(TOF_IGNORE_DB_TYPE_INFOS));
//		}
//		else
//		{
//			ASSERT_NO_THROW(cTable.Open());
//		}
//		// Just test the values in the buffer - trimming has no effect here
//		using namespace boost::algorithm;
//		std::string str;
//		std::wstring idName = test::GetIdColumnName(test::TableId::DATETYPES, m_odbcInfo.m_namesCase);
//		cTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		{
//			// Microsoft will info here (which we report as warning) that the time field has been truncated (as we do not use the fancy TIME2 struct)
//			LogLevelError llE;
//			EXPECT_TRUE(cTable.SelectNext());
//		}
//		EXPECT_EQ(26, cTable.m_date.day);
//		EXPECT_EQ(1, cTable.m_date.month);
//		EXPECT_EQ(1983, cTable.m_date.year);
//
//		EXPECT_EQ(13, cTable.m_time.hour);
//		EXPECT_EQ(55, cTable.m_time.minute);
//		EXPECT_EQ(56, cTable.m_time.second);
//
//		EXPECT_EQ(26, cTable.m_timestamp.day);
//		EXPECT_EQ(1, cTable.m_timestamp.month);
//		EXPECT_EQ(1983, cTable.m_timestamp.year);
//		EXPECT_EQ(13, cTable.m_timestamp.hour);
//		EXPECT_EQ(55, cTable.m_timestamp.minute);
//		EXPECT_EQ(56, cTable.m_timestamp.second);
//	}
//
//
//	TEST_F(TableTest, SelectWCharDateValues)
//	{
//		std::wstring dateTypesTableName = test::GetTableName(test::TableId::DATETYPES, m_odbcInfo.m_namesCase);
//		Table dTable(&m_db, AF_READ, dateTypesTableName, L"", L"", L"");
//		dTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharSql2BufferMap()));
//		ASSERT_NO_THROW(dTable.Open());
//
//		wstring sDate, sTime, sTimestamp;
//		wstring idName = test::GetIdColumnName(test::TableId::DATETYPES, m_odbcInfo.m_namesCase);
//		dTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(dTable.SelectNext());
//		EXPECT_NO_THROW(sDate = dTable.GetWString(1));
//		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
//		{
//			EXPECT_EQ(L"1983-01-26", sDate);
//		}
//
//		EXPECT_NO_THROW(sTime = dTable.GetWString(2));
//		EXPECT_NO_THROW(sTimestamp = dTable.GetWString(3));
//
//		if (m_db.GetDbms() == DatabaseProduct::DB2)
//		{
//			EXPECT_EQ(L"13:55:56", sTime);
//			EXPECT_EQ(L"1983-01-26 13:55:56.000000", sTimestamp);
//		}
//		else if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			EXPECT_EQ(L"13:55:56.1234567", sTime);
//			EXPECT_EQ(L"1983-01-26 13:55:56.000", sTimestamp);
//		}
//		else
//		{
//			if (m_db.GetDbms() != DatabaseProduct::ACCESS)
//			{
//				EXPECT_EQ(L"13:55:56", sTime);
//			}
//			EXPECT_EQ(L"1983-01-26 13:55:56", sTimestamp);
//		}
//
//	}
//
//	
//	TEST_F(TableTest, SelectAutoBlobValues)
//	{
//		std::wstring blobTypesTableName = test::GetTableName(test::TableId::BLOBTYPES, m_odbcInfo.m_namesCase);
//		Table bTable(&m_db, AF_READ, blobTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(bTable.Open());
//
//		wstring idName = test::GetIdColumnName(test::TableId::BLOBTYPES, m_odbcInfo.m_namesCase);
//
//		SQLCHAR empty[] = { 0, 0, 0, 0,
//			0, 0, 0, 0,
//			0, 0, 0, 0,
//			0, 0, 0, 0
//		};
//
//		SQLCHAR ff[] = { 255, 255, 255, 255,
//			255, 255, 255, 255,
//			255, 255, 255, 255,
//			255, 255, 255, 255
//		};
//
//		SQLCHAR abc[] = { 0xab, 0xcd, 0xef, 0xf0,
//			0x12, 0x34, 0x56, 0x78,
//			0x90, 0xab, 0xcd, 0xef,
//			0x01, 0x23, 0x45, 0x67
//		};
//
//		SQLCHAR abc_ff[] = { 0xab, 0xcd, 0xef, 0xf0,
//			0x12, 0x34, 0x56, 0x78,
//			0x90, 0xab, 0xcd, 0xef,
//			0x01, 0x23, 0x45, 0x67,
//			0xff, 0xff, 0xff, 0xff
//		};
//
//		const SQLCHAR* pBlob = NULL;
//		SQLLEN size, length = 0;
//
//		// Fixed size bins
//		bTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_NO_THROW(pBlob = bTable.GetBinaryValue(1, size, length));
//		EXPECT_EQ(0, memcmp(empty, pBlob, length));
//		EXPECT_EQ(16, size);
//		EXPECT_EQ(sizeof(empty), length);
//		EXPECT_TRUE(bTable.IsColumnNull(2));
//
//		bTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_NO_THROW(pBlob = bTable.GetBinaryValue(1, size, length));
//		EXPECT_EQ(0, memcmp(ff, pBlob, length));
//		EXPECT_EQ(16, size);
//		EXPECT_EQ(sizeof(ff), length);
//		EXPECT_TRUE(bTable.IsColumnNull(2));
//
//		bTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_NO_THROW(pBlob = bTable.GetBinaryValue(1, size, length));
//		EXPECT_EQ(0, memcmp(abc, pBlob, length));
//		EXPECT_EQ(16, size);
//		EXPECT_EQ(sizeof(abc), length);
//		EXPECT_TRUE(bTable.IsColumnNull(2));
//
//		// Read Varbins
//		bTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_NO_THROW(pBlob = bTable.GetBinaryValue(2, size, length));
//		EXPECT_EQ(0, memcmp(abc, pBlob, length));
//		EXPECT_EQ(20, size);
//		EXPECT_EQ(sizeof(abc), length);
//		EXPECT_TRUE(bTable.IsColumnNull(1));
//
//		bTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_NO_THROW(pBlob = bTable.GetBinaryValue(2, size, length));
//		EXPECT_EQ(0, memcmp(abc_ff, pBlob, length));
//		EXPECT_EQ(20, size);
//		EXPECT_EQ(sizeof(abc_ff), length);
//		EXPECT_TRUE(bTable.IsColumnNull(1));
//	}
//
//
//	TEST_F(TableTest, SelectManualBlobValues)
//	{
//		MBlobTypesTable bTable(&m_db, m_odbcInfo.m_namesCase);
//		ASSERT_NO_THROW(bTable.Open());
//
//		wstring idName = test::GetIdColumnName(test::TableId::BLOBTYPES, m_odbcInfo.m_namesCase);
//
//		SQLCHAR empty[] = { 0, 0, 0, 0,
//			0, 0, 0, 0,
//			0, 0, 0, 0,
//			0, 0, 0, 0
//		};
//
//		SQLCHAR ff[] = { 255, 255, 255, 255,
//			255, 255, 255, 255,
//			255, 255, 255, 255,
//			255, 255, 255, 255
//		};
//
//		SQLCHAR abc[] = { 0xab, 0xcd, 0xef, 0xf0,
//			0x12, 0x34, 0x56, 0x78,
//			0x90, 0xab, 0xcd, 0xef,
//			0x01, 0x23, 0x45, 0x67
//		};
//
//		SQLCHAR abc_ff[] = { 0xab, 0xcd, 0xef, 0xf0,
//			0x12, 0x34, 0x56, 0x78,
//			0x90, 0xab, 0xcd, 0xef,
//			0x01, 0x23, 0x45, 0x67,
//			0xff, 0xff, 0xff, 0xff
//		};
//
//		bTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_EQ(0, memcmp(empty, bTable.m_blob, sizeof(bTable.m_blob)));
//
//		bTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_EQ(0, memcmp(ff, bTable.m_blob, sizeof(bTable.m_blob)));
//
//		bTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_EQ(0, memcmp(abc, bTable.m_blob, sizeof(bTable.m_blob)));
//
//		bTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_EQ(0, memcmp(abc, bTable.m_varblob_20, sizeof(abc)));
//
//		bTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_EQ(0, memcmp(abc_ff, bTable.m_varblob_20, sizeof(abc_ff)));
//
//	}
//
//
//	TEST_F(TableTest, IsNullAutoNumericValue)
//	{
//		std::wstring numericTypesTableName = test::GetTableName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
//		Table nTable(&m_db, AF_READ, numericTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(nTable.Open());
//
//		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
//		
//		// col indexes:
//		SQLSMALLINT iid = 0;
//		SQLSMALLINT i18_00 = 1;
//		SQLSMALLINT i18_10 = 2;
//		SQLSMALLINT i5_3 = 3;
//
//		nTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_00));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_10));
//		EXPECT_TRUE(nTable.IsColumnNull(i5_3));
//
//		nTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_00));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_10));
//		EXPECT_FALSE(nTable.IsColumnNull(i5_3));
//
//		nTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_00));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_10));
//		EXPECT_TRUE(nTable.IsColumnNull(i5_3));
//
//		nTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_00));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_10));
//		EXPECT_TRUE(nTable.IsColumnNull(i5_3));
//
//		nTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_00));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_10));
//		EXPECT_TRUE(nTable.IsColumnNull(i5_3));
//
//		nTable.Select((boost::wformat(L"%s = 6") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_00));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_10));
//		EXPECT_TRUE(nTable.IsColumnNull(i5_3));
//	}
//	
//	
//	TEST_F(TableTest, SelectAutoNumericValue)
//	{
//		// \note: There is a special Tests for NULL values, its complicated enough.
//		std::wstring numericTypesTableName = test::GetTableName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
//		Table nTable(&m_db, AF_READ, numericTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(nTable.Open());
//		
//		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
//		SQL_NUMERIC_STRUCT numStr;
//		SQLBIGINT ex;
//		SQLBIGINT* p;
//
//		nTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_NO_THROW(numStr = nTable.GetNumeric(1));
//		EXPECT_EQ(18, numStr.precision);
//		EXPECT_EQ(0, numStr.scale);
//		EXPECT_EQ(1, numStr.sign);
//		ex = 0;
//		p = (SQLBIGINT*)&numStr.val;
//		EXPECT_EQ(ex, *p);
//
//		nTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_NO_THROW(numStr = nTable.GetNumeric(1));
//		EXPECT_EQ(18, numStr.precision);
//		EXPECT_EQ(0, numStr.scale);
//		EXPECT_EQ(1, numStr.sign);
//		ex = 123456789012345678;
//		p = (SQLBIGINT*)&numStr.val;
//		EXPECT_EQ(ex, *p);
//		// Col 3 has a value here too.
//		EXPECT_NO_THROW(numStr = nTable.GetNumeric(3));
//		EXPECT_EQ(5, numStr.precision);
//		EXPECT_EQ(3, numStr.scale);
//		EXPECT_EQ(1, numStr.sign);
//		ex = 12345;
//		p = (SQLBIGINT*)&numStr.val;
//		EXPECT_EQ(ex, *p);
//
//		nTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_NO_THROW(numStr = nTable.GetNumeric(1));
//		EXPECT_EQ(18, numStr.precision);
//		EXPECT_EQ(0, numStr.scale);
//		EXPECT_EQ(0, numStr.sign);
//		ex = 123456789012345678;
//		p = (SQLBIGINT*)&numStr.val;
//		EXPECT_EQ(ex, *p);
//
//		nTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_NO_THROW(numStr = nTable.GetNumeric(2));
//		EXPECT_EQ(18, numStr.precision);
//		EXPECT_EQ(10, numStr.scale);
//		EXPECT_EQ(1, numStr.sign);
//		ex = 0;
//		p = (SQLBIGINT*)&numStr.val;
//		EXPECT_EQ(ex, *p);
//
//		nTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_NO_THROW(numStr = nTable.GetNumeric(2));
//		EXPECT_EQ(18, numStr.precision);
//		EXPECT_EQ(10, numStr.scale);
//		EXPECT_EQ(1, numStr.sign);
//		ex = 123456789012345678;
//		p = (SQLBIGINT*)&numStr.val;
//		EXPECT_EQ(ex, *p);
//
//		nTable.Select((boost::wformat(L"%s = 6") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_NO_THROW(numStr = nTable.GetNumeric(2));
//		EXPECT_EQ(18, numStr.precision);
//		EXPECT_EQ(10, numStr.scale);
//		EXPECT_EQ(0, numStr.sign);
//		ex = 123456789012345678;
//		p = (SQLBIGINT*)&numStr.val;
//		EXPECT_EQ(ex, *p);
//	}
//
//
//	TEST_F(TableTest, SelectManualNumericValue)
//	{
//		MNumericTypesTable nTable(&m_db, m_odbcInfo.m_namesCase);
//		ASSERT_NO_THROW(nTable.Open());
//
//		SQLBIGINT ex;
//		SQLBIGINT* p;
//		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
//		nTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_EQ(18, nTable.m_decimal_18_0.precision);
//		EXPECT_EQ(0, nTable.m_decimal_18_0.scale);
//		EXPECT_EQ(1, nTable.m_decimal_18_0.sign);
//		ex = 0;
//		p = (SQLBIGINT*)&nTable.m_decimal_18_0.val;
//		EXPECT_EQ(ex, *p);
//
//		nTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_EQ(18, nTable.m_decimal_18_0.precision);
//		EXPECT_EQ(0, nTable.m_decimal_18_0.scale);
//		EXPECT_EQ(1, nTable.m_decimal_18_0.sign);
//		ex = 123456789012345678;
//		p = (SQLBIGINT*)&nTable.m_decimal_18_0.val;
//		EXPECT_EQ(ex, *p);
//		EXPECT_EQ(5, nTable.m_decimal_5_3.precision);
//		EXPECT_EQ(3, nTable.m_decimal_5_3.scale);
//		EXPECT_EQ(1, nTable.m_decimal_5_3.sign);
//		ex = 12345;
//		p = (SQLBIGINT*)&nTable.m_decimal_5_3.val;
//		EXPECT_EQ(ex, *p);
//
//		nTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_EQ(18, nTable.m_decimal_18_0.precision);
//		EXPECT_EQ(0, nTable.m_decimal_18_0.scale);
//		EXPECT_EQ(0, nTable.m_decimal_18_0.sign);
//		ex = 123456789012345678;
//		p = (SQLBIGINT*)&nTable.m_decimal_18_0.val;
//		EXPECT_EQ(ex, *p);
//
//		nTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_EQ(18, nTable.m_decimal_18_10.precision);
//		EXPECT_EQ(10, nTable.m_decimal_18_10.scale);
//		EXPECT_EQ(1, nTable.m_decimal_18_10.sign);
//		ex = 0;
//		p = (SQLBIGINT*)&nTable.m_decimal_18_10.val;
//		EXPECT_EQ(ex, *p);
//
//		nTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_EQ(18, nTable.m_decimal_18_10.precision);
//		EXPECT_EQ(10, nTable.m_decimal_18_10.scale);
//		EXPECT_EQ(1, nTable.m_decimal_18_10.sign);
//		ex = 123456789012345678;
//		p = (SQLBIGINT*)&nTable.m_decimal_18_10.val;
//		EXPECT_EQ(ex, *p);
//
//		nTable.Select((boost::wformat(L"%s = 6") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_EQ(18, nTable.m_decimal_18_10.precision);
//		EXPECT_EQ(10, nTable.m_decimal_18_10.scale);
//		EXPECT_EQ(0, nTable.m_decimal_18_10.sign);
//		ex = 123456789012345678;
//		p = (SQLBIGINT*)&nTable.m_decimal_18_10.val;
//		EXPECT_EQ(ex, *p);
//	}
//
//
//	TEST_F(TableTest, IsNullManualNumericValue)
//	{
//		std::wstring numericTypesTableName = test::GetTableName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
//		Table nTable(&m_db, AF_READ, numericTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(nTable.Open());
//
//		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
//		
//		// col indexes
//		SQLSMALLINT iid = 0;
//		SQLSMALLINT i18_00 = 1;
//		SQLSMALLINT i18_10 = 2;
//		SQLSMALLINT i5_3 = 3;
//
//		nTable.Select((boost::wformat(L"%s = 1") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_00));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_10));
//		EXPECT_TRUE(nTable.IsColumnNull(i5_3));
//
//		nTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_00));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_10));
//		EXPECT_FALSE(nTable.IsColumnNull(i5_3));
//
//		nTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_00));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_10));
//		EXPECT_TRUE(nTable.IsColumnNull(i5_3));
//
//		nTable.Select((boost::wformat(L"%s = 4") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_00));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_10));
//		EXPECT_TRUE(nTable.IsColumnNull(i5_3));
//
//		nTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_00));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_10));
//		EXPECT_TRUE(nTable.IsColumnNull(i5_3));
//
//		nTable.Select((boost::wformat(L"%s = 6") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_FALSE(nTable.IsColumnNull(iid));
//		EXPECT_TRUE(nTable.IsColumnNull(i18_00));
//		EXPECT_FALSE(nTable.IsColumnNull(i18_10));
//		EXPECT_TRUE(nTable.IsColumnNull(i5_3));
//	}
//
//
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
//	// Insert rows
//	// -----------
//	TEST_F(TableTest, InsertIntTypes)
//	{
//		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table iTable(&m_db, AF_SELECT | AF_INSERT, intTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//
//		// Remove everything, ignoring if there was any data:
//		ASSERT_NO_THROW(test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase));
//
//		// Set some silly values to insert
//		iTable.SetColumnValue(0, (SQLINTEGER)101);
//		iTable.SetColumnValue(2, (SQLINTEGER)103);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			iTable.SetColumnValue(1, (SQLINTEGER)102);
//			iTable.SetColumnNull(3);
//		}
//		else
//		{
//			iTable.SetColumnValue(1, (SQLSMALLINT)102);
//			iTable.SetColumnValue(3, (SQLBIGINT)104);
//		}
//		EXPECT_NO_THROW(iTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Open another table and read the values from there
//		Table iTable2(&m_db, AF_READ, intTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable2.Open());
//
//		iTable2.Select();
//		EXPECT_TRUE(iTable2.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 101, 102, 103, 104));
//	}
//
//
//	// just do only one test to insert the manual types, its all the same, except that opening is a little bit different
//	TEST_F(TableTest, InsertManualIntTypes)
//	{
//		// Open an existing table without checking for privileges or existence
//		Table iTable(&m_db, AF_SELECT | AF_INSERT, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"");
//		SQLINTEGER id = 0;
//		SQLSMALLINT si = 0;
//		SQLINTEGER i = 0;
//		SQLBIGINT bi = 0;
//		SQLINTEGER biAccess = 0;
//		int type = SQL_C_SLONG;
//		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT);
//		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_INSERT);
//		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT | CF_INSERT);
//		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
//		{
//			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
//		}
//		else
//		{
//			// access has no bigint, col is int in test-db
//			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &biAccess, SQL_C_SLONG, sizeof(bi), CF_SELECT | CF_INSERT | CF_NULLABLE);
//		}
//
//		iTable.Open();
//		
//		// Remove everything, ignoring if there was any data:
//		ASSERT_NO_THROW(test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase));
//
//		// Set some silly values to insert
//		// note: no need to set to not-null, the buffers are created with null set to false
//		id = 101;
//		si = 102;
//		i = 103;
//		bi = 104;
//		biAccess = 104;
//
//		EXPECT_NO_THROW(iTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Open another table and read the values from there
//		Table iTable2(&m_db, AF_READ, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"");
//		iTable2.Open();
//		iTable2.Select();
//		ASSERT_TRUE(iTable2.SelectNext());
//
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable2, 101, 102, 103, 104));
//	}
//
//
//	TEST_F(TableTest, InsertDateTypes)
//	{
//		std::wstring dateTypesTableName = test::GetTableName(test::TableId::DATETYPES_TMP, m_odbcInfo.m_namesCase);
//		Table dTable(&m_db, AF_SELECT | AF_INSERT, dateTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(dTable.Open());
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearDateTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		// Set some silly values
//		SQL_DATE_STRUCT date;
//		date.day = 26;
//		date.year = 1983;
//		date.month = 1;
//		
//		SQL_TIME_STRUCT time;
//		time.hour = 13;
//		time.minute = 55;
//		time.second = 03;
//		
//		SQL_TIMESTAMP_STRUCT timestamp;
//		timestamp.day = 26;
//		timestamp.year = 1983;
//		timestamp.month = 1;
//		timestamp.hour = 13;
//		timestamp.minute = 55;
//		timestamp.second = 03;
//		if ( ! (m_db.GetDbms() == DatabaseProduct::MY_SQL || m_db.GetDbms() == DatabaseProduct::ACCESS))
//		{
//			timestamp.fraction = 123000000;
//		}
//		else
//		{
//			timestamp.fraction = 0;
//		}
//
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			// Access has only timestamps ??
//			dTable.SetColumnValue(0, (SQLINTEGER)101);
//			dTable.SetColumnValue(1, timestamp);
//			dTable.SetColumnValue(2, timestamp);
//			dTable.SetColumnValue(3, timestamp);
//		}
//		else
//		{
//			dTable.SetColumnValue(0, (SQLINTEGER)101);
//			dTable.SetColumnValue(1, date);
//			dTable.SetColumnValue(2, time);
//			dTable.SetColumnValue(3, timestamp);
//		}
//
//		EXPECT_NO_THROW(dTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Open another table and read the values from there
//		Table dTable2(&m_db, AF_READ, dateTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(dTable2.Open());
//
//		dTable2.Select();
//		EXPECT_TRUE(dTable2.SelectNext());
//
//		SQL_DATE_STRUCT date2;
//		SQL_TIME_STRUCT time2;
//		SQL_TIMESTAMP_STRUCT timestamp2 = dTable.GetTimeStamp(3);
//		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
//		{
//			date2 = dTable.GetDate(1);
//			time2 = dTable.GetTime(2);
//		}
//
//		EXPECT_EQ(101, dTable2.GetInt(0));
//		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
//		{
//			EXPECT_EQ(date.year, date2.year);
//			EXPECT_EQ(date.month, date2.month);
//			EXPECT_EQ(date.day, date2.day);
//			EXPECT_EQ(time.hour, time2.hour);
//			EXPECT_EQ(time.minute, time2.minute);
//			EXPECT_EQ(time.second, time2.second);
//		}
//
//		EXPECT_EQ(timestamp.year, timestamp2.year);
//		EXPECT_EQ(timestamp.month, timestamp2.month);
//		EXPECT_EQ(timestamp.day, timestamp2.day);
//
//		EXPECT_EQ(timestamp.hour, timestamp2.hour);
//		EXPECT_EQ(timestamp.minute, timestamp2.minute);
//		EXPECT_EQ(timestamp.second, timestamp2.second);
//		EXPECT_EQ(timestamp.fraction, timestamp2.fraction);
//	}
//
//
//	TEST_F(TableTest, InsertFloatTypes)
//	{
//		std::wstring floatTypesTableName = test::GetTableName(test::TableId::FLOATTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table fTable(&m_db, AF_READ_WRITE, floatTypesTableName, L"", L"", L"");
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			fTable.SetColumnPrimaryKeyIndexes({ 0 });
//		}
//		ASSERT_NO_THROW(fTable.Open());
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearFloatTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		// Insert some values
//		fTable.SetColumnValue(0, (SQLINTEGER)101);
//		fTable.SetColumnValue(1, (SQLDOUBLE)3.14159265359);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS || m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			// Access binds as double
//			// also ms sql server, it seems to report SQLFLOAT which we map to SQL_C_FLOAT, which is a DOUBLE
//			fTable.SetColumnValue(2, (SQLDOUBLE)-3.14159);
//		}
//		else
//		{
//			fTable.SetColumnValue(2, (SQLREAL)-3.14159);
//		}
//		EXPECT_NO_THROW(fTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Open another table and read the values from there
//		Table fTable2(&m_db, AF_READ, floatTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(fTable2.Open());
//
//		fTable2.Select();
//		EXPECT_TRUE(fTable2.SelectNext());
//
//		EXPECT_EQ(101, fTable2.GetInt(0));
//		SQLDOUBLE doubleVal = fTable2.GetDouble(1);
//		EXPECT_EQ((SQLBIGINT)(1e11 * 3.14159265359), (SQLBIGINT)(1e11 * doubleVal));
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS || m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			SQLDOUBLE realVal = fTable2.GetDouble(2);
//			EXPECT_EQ((int)(1e5 * -3.14159), (int)(1e5 * realVal));
//		}
//		else
//		{
//			SQLREAL realVal = fTable2.GetReal(2);
//			EXPECT_EQ((int)(1e5 * -3.14159), (int)(1e5 * realVal));
//		}
//	}
//
//
//	TEST_F(TableTest, InsertNumericTypes)
//	{
//		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table nTable(&m_db, AF_READ_WRITE, numericTypesTmpTableName, L"", L"", L"");
//		ASSERT_NO_THROW(nTable.Open());
//
//		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Select a valid record from the non-tmp table
//		std::wstring numericTypesTableName = test::GetTableName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
//		Table nnTable(&m_db, AF_READ_WRITE, numericTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(nnTable.Open());
//		SQL_NUMERIC_STRUCT numStr18_0, numStr18_10, numStr5_3;
//		nnTable.Select((boost::wformat(L"%s = 2") % idName).str());
//		EXPECT_TRUE(nnTable.SelectNext());
//		EXPECT_NO_THROW(numStr18_0 = nnTable.GetNumeric(1));
//		EXPECT_NO_THROW(numStr5_3 = nnTable.GetNumeric(3));
//		nnTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(nnTable.SelectNext());
//		EXPECT_NO_THROW(numStr18_10 = nnTable.GetNumeric(2));
//		nnTable.SelectClose();
//
//		// Remove everything from the tmp-table
//		test::ClearNumericTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		// Insert the just read value into the tmp-table
//		nTable.SetColumnValue(0, (SQLINTEGER)101);
//		nTable.SetColumnValue(1, numStr18_0);
//		nTable.SetColumnValue(2, numStr18_10);
//		nTable.SetColumnValue(3, numStr5_3);
//		EXPECT_NO_THROW(nTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// And read them again from another table and compare them
//		SQL_NUMERIC_STRUCT numStr18_0t, numStr18_10t, numStr5_3t;
//		Table nntTable(&m_db, AF_READ_WRITE, numericTypesTmpTableName, L"", L"", L"");
//		ASSERT_NO_THROW(nntTable.Open());
//		nntTable.Select();
//		EXPECT_TRUE(nntTable.SelectNext());
//		EXPECT_NO_THROW(numStr18_0t = nntTable.GetNumeric(1));
//		EXPECT_NO_THROW(numStr18_10t = nntTable.GetNumeric(2));
//		EXPECT_NO_THROW(numStr5_3t = nntTable.GetNumeric(3));
//		nntTable.SelectClose();
//		EXPECT_EQ(0, memcmp(&numStr18_0, &numStr18_0t, sizeof(numStr18_0)));
//		EXPECT_EQ(0, memcmp(&numStr18_10, &numStr18_10t, sizeof(numStr18_10)));
//		EXPECT_EQ(0, memcmp(&numStr5_3, &numStr5_3t, sizeof(numStr5_3)));
//	}
//
//
//	TEST_F(TableTest, InsertNumericTypes_5_3)
//	{
//		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table t(&m_db, AF_READ_WRITE, numericTypesTmpTableName, L"", L"", L"");
//
//		ASSERT_NO_THROW(t.Open());
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearNumericTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		SQL_NUMERIC_STRUCT numStr;
//		ZeroMemory(&numStr, sizeof(numStr));
//		numStr.val[0] = 57;
//		numStr.val[1] = 48;
//		numStr.precision = 5;
//		numStr.scale = 3;
//		numStr.sign = 1;
//
//		t.SetColumnValue(0, (SQLINTEGER)300);
//		t.SetColumnNull(1);
//		t.SetColumnNull(2);
//		t.SetColumnValue(3, numStr);
//
//		EXPECT_NO_THROW(t.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//	}
//
//
//	TEST_F(TableTest, InsertNumericTypes_18_0)
//	{
//		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table t(&m_db, AF_READ_WRITE, numericTypesTmpTableName, L"", L"", L"");
//
//		ASSERT_NO_THROW(t.Open());
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearNumericTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		SQL_NUMERIC_STRUCT numStr;
//		ZeroMemory(&numStr, sizeof(numStr));
//		numStr.val[0] = 78;
//		numStr.val[1] = 243;
//		numStr.val[2] = 48;
//		numStr.val[3] = 166;
//		numStr.val[4] = 75;
//		numStr.val[5] = 155;
//		numStr.val[6] = 182;
//		numStr.val[7] = 1;
//
//		numStr.precision = 18;
//		numStr.scale = 0;
//		numStr.sign = 1;
//
//		t.SetColumnValue(0, (SQLINTEGER)300);
//		t.SetColumnValue(1, numStr);
//		t.SetColumnNull(2);
//		t.SetColumnNull(3);
//
//		EXPECT_NO_THROW(t.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//	}
//
//
//	TEST_F(TableTest, InsertNumericTypes_18_10)
//	{
//		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table t(&m_db, AF_READ_WRITE, numericTypesTmpTableName, L"", L"", L"");
//
//		ASSERT_NO_THROW(t.Open());
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearNumericTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		SQL_NUMERIC_STRUCT numStr;
//		ZeroMemory(&numStr, sizeof(numStr));
//		numStr.val[0] = 78;
//		numStr.val[1] = 243;
//		numStr.val[2] = 48;
//		numStr.val[3] = 166;
//		numStr.val[4] = 75;
//		numStr.val[5] = 155;
//		numStr.val[6] = 182;
//		numStr.val[7] = 1;
//
//		numStr.precision = 18;
//		numStr.scale = 10;
//		numStr.sign = 1;
//
//		t.SetColumnValue(0, (SQLINTEGER)300);
//		t.SetColumnNull(1);
//		t.SetColumnValue(2, numStr);
//		t.SetColumnNull(3);
//		EXPECT_NO_THROW(t.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//	}
//
//
//	TEST_F(TableTest, InsertNumericTypes_All)
//	{
//		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table t(&m_db, AF_READ_WRITE, numericTypesTmpTableName, L"", L"", L"");
//
//		ASSERT_NO_THROW(t.Open());
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearNumericTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		SQL_NUMERIC_STRUCT numStr18_0;
//		ZeroMemory(&numStr18_0, sizeof(numStr18_0));
//		numStr18_0.val[0] = 78;
//		numStr18_0.val[1] = 243;
//		numStr18_0.val[2] = 48;
//		numStr18_0.val[3] = 166;
//		numStr18_0.val[4] = 75;
//		numStr18_0.val[5] = 155;
//		numStr18_0.val[6] = 182;
//		numStr18_0.val[7] = 1;
//		numStr18_0.precision = 18;
//		numStr18_0.scale = 0;
//		numStr18_0.sign = 1;
//
//		SQL_NUMERIC_STRUCT numStr18_10;
//		ZeroMemory(&numStr18_10, sizeof(numStr18_10));
//		numStr18_10.val[0] = 78;
//		numStr18_10.val[1] = 243;
//		numStr18_10.val[2] = 48;
//		numStr18_10.val[3] = 166;
//		numStr18_10.val[4] = 75;
//		numStr18_10.val[5] = 155;
//		numStr18_10.val[6] = 182;
//		numStr18_10.val[7] = 1;
//		numStr18_10.precision = 18;
//		numStr18_10.scale = 10;
//		numStr18_10.sign = 1;
//
//		SQL_NUMERIC_STRUCT numStr5_3;
//		ZeroMemory(&numStr5_3, sizeof(numStr5_3));
//		numStr5_3.val[0] = 57;
//		numStr5_3.val[1] = 48;
//		numStr5_3.precision = 5;
//		numStr5_3.scale = 3;
//		numStr5_3.sign = 1;
//
//		t.SetColumnValue(0, (SQLINTEGER)300);
//		t.SetColumnValue(1, numStr18_0);
//		t.SetColumnValue(2, numStr18_10);
//		t.SetColumnValue(3, numStr5_3);
//		EXPECT_NO_THROW(t.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//	}
//
//	
//	TEST_F(TableTest, InsertBlobTypes)
//	{
//		std::wstring blobTypesTmpTableName = test::GetTableName(test::TableId::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table bTable(&m_db, AF_SELECT | AF_INSERT, blobTypesTmpTableName, L"", L"", L"");
//		ASSERT_NO_THROW(bTable.Open());
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearBlobTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//		wstring idName = test::GetIdColumnName(test::TableId::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
//		wstring sqlstmt;
//
//		SQLCHAR empty[] = { 0, 0, 0, 0,
//			0, 0, 0, 0,
//			0, 0, 0, 0,
//			0, 0, 0, 0
//		};
//
//		SQLCHAR ff[] = { 255, 255, 255, 255,
//			255, 255, 255, 255,
//			255, 255, 255, 255,
//			255, 255, 255, 255
//		};
//
//		SQLCHAR abc[] = { 0xab, 0xcd, 0xef, 0xf0,
//			0x12, 0x34, 0x56, 0x78,
//			0x90, 0xab, 0xcd, 0xef,
//			0x01, 0x23, 0x45, 0x67
//		};
//
//		SQLCHAR abc_ff[] = { 0xab, 0xcd, 0xef, 0xf0,
//			0x12, 0x34, 0x56, 0x78,
//			0x90, 0xab, 0xcd, 0xef,
//			0x01, 0x23, 0x45, 0x67,
//			0xff, 0xff, 0xff, 0xff
//		};
//
//		// Insert some values
//		bTable.SetColumnValue(0, (SQLINTEGER)100);
//		bTable.SetBinaryValue(1, empty, sizeof(empty));
//		bTable.SetColumnNull(2);
//		EXPECT_NO_THROW(bTable.Insert());
//		bTable.SetColumnValue(0, (SQLINTEGER)101);
//		bTable.SetBinaryValue(1, ff, sizeof(ff));
//		bTable.SetColumnNull(2);
//		EXPECT_NO_THROW(bTable.Insert());
//		bTable.SetColumnValue(0, (SQLINTEGER)102);
//		bTable.SetBinaryValue(1, abc, sizeof(abc));
//		bTable.SetColumnNull(2);
//		EXPECT_NO_THROW(bTable.Insert());
//		bTable.SetColumnValue(0, (SQLINTEGER)103);
//		bTable.SetColumnNull(1);
//		bTable.SetBinaryValue(2, abc, sizeof(abc));
//		EXPECT_NO_THROW(bTable.Insert());
//		bTable.SetColumnValue(0, (SQLINTEGER)104);
//		bTable.SetColumnNull(1);
//		bTable.SetBinaryValue(2, abc_ff, sizeof(abc_ff));
//		EXPECT_NO_THROW(bTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Now read the inserted values
//		sqlstmt = (boost::wformat(L"%s = 100") % idName).str();
//		bTable.Select(sqlstmt);
//		EXPECT_TRUE(bTable.SelectNext());
//		SQLLEN bufferSize = 0;
//		SQLLEN cb = 0;
//		const SQLCHAR* pEmpty = bTable.GetBinaryValue(1, bufferSize, cb);
//		EXPECT_EQ(0, memcmp(empty, pEmpty, sizeof(empty)));
//
//		sqlstmt = (boost::wformat(L"%s = 101") % idName).str();
//		bTable.Select(sqlstmt);
//		EXPECT_TRUE(bTable.SelectNext());
//		const SQLCHAR* pFf = bTable.GetBinaryValue(1, bufferSize, cb);
//		EXPECT_EQ(0, memcmp(ff, pFf, sizeof(ff)));
//
//		sqlstmt = (boost::wformat(L"%s = 102") % idName).str();
//		bTable.Select(sqlstmt);
//		EXPECT_TRUE(bTable.SelectNext());
//		const SQLCHAR* pAbc = bTable.GetBinaryValue(1, bufferSize, cb);
//		EXPECT_EQ(0, memcmp(abc, pAbc, sizeof(abc)));
//
//		sqlstmt = (boost::wformat(L"%s = 103") % idName).str();
//		bTable.Select(sqlstmt);
//		EXPECT_TRUE(bTable.SelectNext());
//		pAbc = bTable.GetBinaryValue(2, bufferSize, cb);
//		EXPECT_EQ(0, memcmp(abc, pAbc, sizeof(abc)));
//		EXPECT_EQ(sizeof(abc), cb);
//
//		sqlstmt = (boost::wformat(L"%s = 104") % idName).str();
//		bTable.Select(sqlstmt);
//		EXPECT_TRUE(bTable.SelectNext());
//		const SQLCHAR* pAbc_ff = bTable.GetBinaryValue(2, bufferSize, cb);
//		EXPECT_EQ(0, memcmp(abc_ff, pAbc_ff, sizeof(abc_ff)));
//		EXPECT_EQ(sizeof(abc_ff), cb);
//	}
//
//
//	TEST_F(TableTest, InsertCharTypes)
//	{
//		std::wstring charTypesTmpTableName = test::GetTableName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table cTable(&m_db, AF_SELECT | AF_INSERT, charTypesTmpTableName, L"", L"", L"");
//		cTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharAsCharSql2BufferMap(m_db.GetMaxSupportedOdbcVersion())));
//		ASSERT_NO_THROW(cTable.Open());
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearCharTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//		wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Insert some values:
//		std::string s = "Hello World!";
//		cTable.SetColumnValue(0, (SQLINTEGER)100);
//		cTable.SetColumnValue(1, s);
//		cTable.SetColumnValue(2, s);
//		cTable.SetColumnNull(3);
//		cTable.SetColumnNull(4);
//
//		EXPECT_NO_THROW(cTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Insert one value that uses all space
//		s = "abcde12345";
//		cTable.SetColumnValue(0, (SQLINTEGER)101);
//		cTable.SetColumnNull(1);
//		cTable.SetColumnNull(2);
//		cTable.SetColumnValue(3, s);
//		cTable.SetColumnValue(4, s);
//
//		EXPECT_NO_THROW(cTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Read them back from another table
//		s = "Hello World!";
//		Table t2(&m_db, AF_READ, charTypesTmpTableName, L"", L"", L"");
//		t2.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharAsCharSql2BufferMap(m_db.GetMaxSupportedOdbcVersion())));
//		ASSERT_NO_THROW(t2.Open());
//		t2.SetCharTrimRight(true);
//
//		std::string varchar2, char2;
//		t2.Select((boost::wformat(L"%s = 100") % idName).str());
//		EXPECT_TRUE(t2.SelectNext());
//		EXPECT_NO_THROW(varchar2 = t2.GetString(1));
//		EXPECT_NO_THROW(char2 = t2.GetString(2));
//		EXPECT_TRUE(t2.IsColumnNull(3));
//		EXPECT_TRUE(t2.IsColumnNull(4));
//		EXPECT_EQ(s, varchar2);
//		EXPECT_EQ(s, char2);
//
//		s = "abcde12345";
//		t2.Select((boost::wformat(L"%s = 101") % idName).str());
//		EXPECT_TRUE(t2.SelectNext());
//		EXPECT_TRUE(t2.IsColumnNull(1));
//		EXPECT_TRUE(t2.IsColumnNull(2));
//		EXPECT_NO_THROW(varchar2 = t2.GetString(3));
//		EXPECT_NO_THROW(char2 = t2.GetString(4));
//		EXPECT_EQ(s, varchar2);
//		EXPECT_EQ(s, char2);
//	}
//
//
//	TEST_F(TableTest, InsertWCharTypes)
//	{
//		std::wstring charTypesTmpTableName = test::GetTableName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table cTable(&m_db, AF_SELECT | AF_INSERT, charTypesTmpTableName, L"", L"", L"");
//		cTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new CharAsWCharSql2BufferMap(m_db.GetMaxSupportedOdbcVersion())));
//		ASSERT_NO_THROW(cTable.Open());
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearCharTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//		wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Insert some values:
//		std::wstring s = L"Hello World!";
//		cTable.SetColumnValue(0, (SQLINTEGER)100);
//		cTable.SetColumnValue(1, s);
//		cTable.SetColumnValue(2, s);
//		cTable.SetColumnNull(3);
//		cTable.SetColumnNull(4);
//		EXPECT_NO_THROW(cTable.Insert());
//		ASSERT_NO_THROW(m_db.CommitTrans());
//
//		// Insert one value that uses all space
//		s = L"abcde12345";
//		cTable.SetColumnValue(0, (SQLINTEGER)101);
//		cTable.SetColumnNull(1);
//		cTable.SetColumnNull(2);
//		cTable.SetColumnValue(3, s);
//		cTable.SetColumnValue(4, s);
//		EXPECT_NO_THROW(cTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Read them back from another table
//		s = L"Hello World!";
//		Table t2(&m_db, AF_READ, charTypesTmpTableName, L"", L"", L"");
//		t2.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new CharAsWCharSql2BufferMap(m_db.GetMaxSupportedOdbcVersion())));
//		ASSERT_NO_THROW(t2.Open());
//		t2.SetCharTrimRight(true);
//
//		std::wstring varchar2, char2;
//		t2.Select((boost::wformat(L"%s = 100") % idName).str());
//		EXPECT_TRUE(t2.SelectNext());
//		EXPECT_NO_THROW(varchar2 = t2.GetWString(1));
//		EXPECT_NO_THROW(char2 = t2.GetWString(2));
//		EXPECT_TRUE(t2.IsColumnNull(3));
//		EXPECT_TRUE(t2.IsColumnNull(4));
//		EXPECT_EQ(s, varchar2);
//		EXPECT_EQ(s, char2);
//
//		s = L"abcde12345";
//		t2.Select((boost::wformat(L"%s = 101") % idName).str());
//		EXPECT_TRUE(t2.SelectNext());
//		EXPECT_TRUE(t2.IsColumnNull(1));
//		EXPECT_TRUE(t2.IsColumnNull(2));
//		EXPECT_NO_THROW(varchar2 = t2.GetWString(3));
//		EXPECT_NO_THROW(char2 = t2.GetWString(4));
//		EXPECT_EQ(s, varchar2);
//		EXPECT_EQ(s, char2);
//	}
//
//
//	// Manual Table Inserts and Update rows
//	// -----------
//	TEST_F(TableTest, InsertAndUpdateManualIntTable)
//	{
//		std::wstring intTypesTmpTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Clear the tmp-table
//		test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		Table iTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE_PK, intTypesTmpTableName, L"", L"", L"");
//		// Set Columns
//		SQLINTEGER id;
//		SQLSMALLINT tSmallint;
//		SQLINTEGER tInt;
//		SQLBIGINT tBigint;
//
//		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT | CF_PRIMARY_KEY);
//		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &tSmallint, SQL_C_SSHORT, sizeof(tSmallint), CF_SELECT | CF_INSERT | CF_UPDATE);
//		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &tInt, SQL_C_SLONG, sizeof(tInt), CF_SELECT | CF_INSERT | CF_UPDATE);
//		iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &tBigint, SQL_C_SBIGINT, sizeof(tBigint), CF_SELECT | CF_INSERT | CF_UPDATE);
//
//		ASSERT_NO_THROW(iTable.Open(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS));
//
//		// Insert some values
//		id = 199;
//		tSmallint = -13;
//		tInt = 83912;
//		tBigint = -1516;
//		ASSERT_NO_THROW(iTable.Insert());
//		ASSERT_NO_THROW(m_db.CommitTrans());
//
//		// and read them back - note, we read back all as bigints on purpose, to avoid making a diff (access no smallint, etc.)
//		Table iTable2(&m_db, AF_READ, intTypesTmpTableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable2.Open());
//		EXPECT_NO_THROW(iTable2.Select());
//		EXPECT_TRUE(iTable2.SelectNext());
//		EXPECT_EQ(id, iTable2.GetInt(0));
//		EXPECT_EQ((SQLBIGINT)tSmallint, iTable2.GetBigInt(1));
//		EXPECT_EQ((SQLBIGINT)tInt, iTable2.GetBigInt(2));
//		EXPECT_EQ(tBigint, iTable2.GetBigInt(3));
//
//		// Now update those values
//		tSmallint = 13;
//		tInt = -93912;
//		tBigint = 1516;
//		EXPECT_NO_THROW(iTable.Update());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// and read them back - note, we read back all as bigints on purpose, to avoid making a diff (access no smallint, etc.)
//		EXPECT_NO_THROW(iTable2.Select());
//		EXPECT_TRUE(iTable2.SelectNext());
//		EXPECT_EQ(id, iTable2.GetInt(0));
//		EXPECT_EQ((SQLBIGINT)tSmallint, iTable2.GetBigInt(1));
//		EXPECT_EQ((SQLBIGINT)tInt, iTable2.GetBigInt(2));
//		EXPECT_EQ(tBigint, iTable2.GetBigInt(3));
//	}
//
//
//	TEST_F(TableTest, InsertAndUpdateManualCharTable)
//	{
//		std::wstring charTypesTmpTableName = test::GetTableName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Clear the tmp-table
//		test::ClearCharTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		Table charTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE_PK, charTypesTmpTableName, L"", L"", L"");
//		// Set Columns
//		SQLINTEGER id;
//		SQLWCHAR varchar[128 + 1];
//		SQLWCHAR char_128[128 + 1];
//		SQLWCHAR varchar_10[10 + 1];
//		SQLWCHAR char_10[10 + 1];
//
//		SQLWCHAR* p1 = varchar_10;
//		size_t addr = (size_t) varchar_10;
//
//		charTable.SetColumn(0, test::ConvertNameCase(L"idchartypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT | CF_PRIMARY_KEY);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			// Access does not report SQL_VARCHAR as a supported type, but it seems to work with SQL_WVARCHAR. Access also absolutely needs a columnSize if we wish to insert.
//			charTable.SetColumn(1, test::ConvertNameCase(L"tvarchar", m_odbcInfo.m_namesCase), SQL_WVARCHAR, varchar, SQL_C_WCHAR, sizeof(varchar), CF_SELECT | CF_INSERT | CF_UPDATE, 128, 0);
//			charTable.SetColumn(2, test::ConvertNameCase(L"tchar", m_odbcInfo.m_namesCase), SQL_WVARCHAR, char_128, SQL_C_WCHAR, sizeof(char_128), CF_SELECT | CF_INSERT | CF_UPDATE, 128, 0);
//			charTable.SetColumn(3, test::ConvertNameCase(L"tvarchar_10", m_odbcInfo.m_namesCase), SQL_WVARCHAR, varchar_10, SQL_C_WCHAR, sizeof(varchar_10), CF_SELECT | CF_INSERT | CF_UPDATE, 10, 0);
//			charTable.SetColumn(4, test::ConvertNameCase(L"tchar_10", m_odbcInfo.m_namesCase), SQL_WVARCHAR, char_10, SQL_C_WCHAR, sizeof(char_10), CF_SELECT | CF_INSERT | CF_UPDATE, 10, 0);
//		}
//		else
//		{
//			charTable.SetColumn(1, test::ConvertNameCase(L"tvarchar", m_odbcInfo.m_namesCase), SQL_VARCHAR, varchar, SQL_C_WCHAR, sizeof(varchar), CF_SELECT | CF_INSERT | CF_UPDATE, 128, 0);
//			charTable.SetColumn(2, test::ConvertNameCase(L"tchar", m_odbcInfo.m_namesCase), SQL_VARCHAR, char_128, SQL_C_WCHAR, sizeof(char_128), CF_SELECT | CF_INSERT | CF_UPDATE, 128, 0);
//			charTable.SetColumn(3, test::ConvertNameCase(L"tvarchar_10", m_odbcInfo.m_namesCase), SQL_VARCHAR, varchar_10, SQL_C_WCHAR, sizeof(varchar_10), CF_SELECT | CF_INSERT | CF_UPDATE, 10, 0);
//			charTable.SetColumn(4, test::ConvertNameCase(L"tchar_10", m_odbcInfo.m_namesCase), SQL_VARCHAR, char_10, SQL_C_WCHAR, sizeof(char_10), CF_SELECT | CF_INSERT | CF_UPDATE, 10, 0);
//		}
//
//		ASSERT_NO_THROW(charTable.Open(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS));
//		
//		// Insert some values
//		wstring s1 = L"Hello World";
//		wstring s2 = L"HelloWorld";
//		wcsncpy(varchar, s1.c_str(), 129);
//		wcsncpy(char_128, s1.c_str(), 129);
//		wcsncpy(varchar_10, s2.c_str(), 11);
//		wcsncpy(char_10, s2.c_str(), 11);
//		id = 199;
//		charTable.SetColumnNTS(1);
//		charTable.SetColumnNTS(2);
//		charTable.SetColumnNTS(3);
//		charTable.SetColumnNTS(4);
//
//		EXPECT_NO_THROW(charTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// And read them back
//		Table charTable2(&m_db, AF_READ, charTypesTmpTableName, L"", L"", L"");
//		charTable2.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharSql2BufferMap()));
//		EXPECT_NO_THROW(charTable2.Open(TOF_CHAR_TRIM_LEFT | TOF_CHAR_TRIM_RIGHT));
//		EXPECT_NO_THROW(charTable2.Select());
//		EXPECT_TRUE(charTable2.SelectNext());
//		EXPECT_EQ(s1, charTable2.GetWString(1));
//		EXPECT_EQ(s1, charTable2.GetWString(2));
//		EXPECT_EQ(s2, charTable2.GetWString(3));
//		EXPECT_EQ(s2, charTable2.GetWString(4));
//
//		// Update some values
//		wstring s3 = L"Hello Moon";
//		wcsncpy(varchar, s3.c_str(), 129);
//		wcsncpy(char_128, s3.c_str(), 129);
//		wcsncpy(varchar_10, s3.c_str(), 11);
//		wcsncpy(char_10, s3.c_str(), 11);
//		EXPECT_NO_THROW(charTable.Update());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// and read back
//		EXPECT_NO_THROW(charTable2.Select());
//		EXPECT_TRUE(charTable2.SelectNext());
//		EXPECT_EQ(s3, charTable2.GetWString(1));
//		EXPECT_EQ(s3, charTable2.GetWString(2));
//		EXPECT_EQ(s3, charTable2.GetWString(3));
//		EXPECT_EQ(s3, charTable2.GetWString(4));
//	}
//
//
//	TEST_F(TableTest, InsertAndUpdateManualFloatTypes)
//	{
//		std::wstring floatTypesTmpTableName = test::GetTableName(test::TableId::FLOATTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Clear the tmp-table
//		test::ClearFloatTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		Table fTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE_PK, floatTypesTmpTableName, L"", L"", L"");
//		
//		// Set Columns
//		SQLINTEGER id = 0;
//		SQLDOUBLE tDouble = 0.0;
//		SQLREAL tReal = 0.0f;
//		
//		fTable.SetColumn(0, test::ConvertNameCase(L"idfloattypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT | CF_PRIMARY_KEY);
//		// MS Sql Server does not report SQL_DOUBLE as a supported datatype, it is defined in the db as Float too.
//		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			fTable.SetColumn(1, test::ConvertNameCase(L"tdouble", m_odbcInfo.m_namesCase), SQL_REAL, &tDouble, SQL_C_DOUBLE, sizeof(tDouble), CF_SELECT | CF_INSERT | CF_UPDATE);
//		}
//		else
//		{
//			fTable.SetColumn(1, test::ConvertNameCase(L"tdouble", m_odbcInfo.m_namesCase), SQL_DOUBLE, &tDouble, SQL_C_DOUBLE, sizeof(tDouble), CF_SELECT | CF_INSERT | CF_UPDATE);
//		}
//		fTable.SetColumn(2, test::ConvertNameCase(L"tfloat", m_odbcInfo.m_namesCase), SQL_REAL, &tReal, SQL_C_FLOAT, sizeof(tReal), CF_SELECT | CF_INSERT | CF_UPDATE);
//
//		ASSERT_NO_THROW(fTable.Open(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS));
//
//		// Insert some data
//		id = 199;
//		tDouble = 3.141592;
//		tReal = 3.141f;
//		EXPECT_NO_THROW(fTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Read back - read it back as Double, that will hold a real without data loss, and 
//		// some dbs like sql server report the database-type in a way we map it to a double, not a real.
//		Table fTable2(&m_db, AF_READ, floatTypesTmpTableName, L"", L"", L"");
//		EXPECT_NO_THROW(fTable2.Open());
//		EXPECT_NO_THROW(fTable2.Select());
//		EXPECT_TRUE(fTable2.SelectNext());
//		EXPECT_EQ((int)(1e6 * 3.141592), (int)(1e6 * fTable2.GetDouble(1)));
//		EXPECT_EQ((int)(1e3 * 3.141), (int)(1e3 * fTable2.GetDouble(2)));
//
//		// Update the row
//		tDouble = -3.141592;
//		tReal = -3.141f;
//		EXPECT_NO_THROW(fTable.Update());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Read back
//		EXPECT_NO_THROW(fTable2.Select());
//		EXPECT_TRUE(fTable2.SelectNext());
//		EXPECT_EQ((int)(1e6 * -3.141592), (int)(1e6 * fTable2.GetDouble(1)));
//		EXPECT_EQ((int)(1e3 * -3.141), (int)(1e3 * fTable2.GetDouble(2)));
//	}
//
//
//	TEST_F(TableTest, InsertAndUpdateManualDateTypes)
//	{
//		std::wstring dateTypesTmpTableName = test::GetTableName(test::TableId::DATETYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Clear the tmp-table
//		test::ClearDateTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		Table dTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE_PK, dateTypesTmpTableName, L"", L"", L"");
//
//		// Set Columns
//		SQLINTEGER id = 0;
//		SQL_DATE_STRUCT date;
//		SQL_TIME_STRUCT time;
//		SQL_TIMESTAMP_STRUCT timestamp;
//
//		dTable.SetColumn(0, test::ConvertNameCase(L"iddatetypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT | CF_PRIMARY_KEY);
//		dTable.SetColumn(1, test::ConvertNameCase(L"tdate", m_odbcInfo.m_namesCase), SQL_TYPE_DATE, &date, SQL_C_TYPE_DATE, sizeof(date), CF_SELECT | CF_INSERT | CF_UPDATE);
//		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			// ms does not report SQL_TYPE_TIME as a supported time, they have their own thing
//			dTable.SetColumn(2, test::ConvertNameCase(L"ttime", m_odbcInfo.m_namesCase), SQL_SS_TIME2, &time, SQL_C_TYPE_TIME, sizeof(time), CF_SELECT | CF_INSERT | CF_UPDATE);
//		}
//		else
//		{
//			dTable.SetColumn(2, test::ConvertNameCase(L"ttime", m_odbcInfo.m_namesCase), SQL_TYPE_TIME, &time, SQL_C_TYPE_TIME, sizeof(time), CF_SELECT | CF_INSERT | CF_UPDATE);
//		}
//		dTable.SetColumn(3, test::ConvertNameCase(L"ttimestamp", m_odbcInfo.m_namesCase), SQL_TYPE_TIMESTAMP, &timestamp, SQL_C_TYPE_TIMESTAMP, sizeof(timestamp), CF_SELECT | CF_INSERT | CF_UPDATE);
//
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			// Access does not report any of the types as a supported type, but if we just ignore what it reports, things (seem to) work fine.
//			ASSERT_NO_THROW(dTable.Open(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS | TOF_IGNORE_DB_TYPE_INFOS));
//		}
//		else
//		{
//			ASSERT_NO_THROW(dTable.Open(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS));
//		}
//
//		// Insert some data
//		id = 199;
//		date.day = 26;
//		date.month = 1;
//		date.year = 1983;
//
//		time.hour = 13;
//		time.minute = 14;
//		time.second = 15;
//
//		timestamp.day = 26;
//		timestamp.month = 1;
//		timestamp.year = 1983;
//		timestamp.hour = 13;
//		timestamp.minute = 14;
//		timestamp.second = 15;
//		timestamp.fraction = 0;
//
//		EXPECT_NO_THROW(dTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Read back
//		Table dTable2(&m_db, AF_READ, dateTypesTmpTableName, L"", L"", L"");
//		EXPECT_NO_THROW(dTable2.Open());
//		EXPECT_NO_THROW(dTable2.Select());
//		EXPECT_TRUE(dTable2.SelectNext());
//		DATE_STRUCT date2 = dTable2.GetDate(1);
//		TIME_STRUCT time2 = dTable2.GetTime(2);
//		TIMESTAMP_STRUCT timestamp2 = dTable2.GetTimeStamp(3);
//
//		EXPECT_EQ(date.day, date2.day);
//		EXPECT_EQ(date.month, date2.month);
//		EXPECT_EQ(date.year, date2.year);
//
//		EXPECT_EQ(time.hour, time2.hour);
//		EXPECT_EQ(time.minute, time2.minute);
//		EXPECT_EQ(time.second, time2.second);
//
//		EXPECT_EQ(timestamp.day, timestamp2.day);
//		EXPECT_EQ(timestamp.month, timestamp2.month);
//		EXPECT_EQ(timestamp.year, timestamp2.year);
//		EXPECT_EQ(timestamp.hour, timestamp2.hour);
//		EXPECT_EQ(timestamp.minute, timestamp2.minute);
//		EXPECT_EQ(timestamp.second, timestamp2.second);
//		EXPECT_EQ(timestamp.fraction, timestamp2.fraction);
//
//		// Update values
//		date.day = 9;
//		date.month = 2;
//		date.year = 1982;
//
//		time.hour = 3;
//		time.minute = 4;
//		time.second = 5;
//
//		timestamp.day = 9;
//		timestamp.month = 2;
//		timestamp.year = 1982;
//		timestamp.hour = 3;
//		timestamp.minute = 4;
//		timestamp.second = 5;
//		timestamp.fraction = 0;
//
//		EXPECT_NO_THROW(dTable.Update());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// and read back
//		EXPECT_NO_THROW(dTable2.Select());
//		EXPECT_TRUE(dTable2.SelectNext());
//		date2 = dTable2.GetDate(1);
//		time2 = dTable2.GetTime(2);
//		timestamp2 = dTable2.GetTimeStamp(3);
//
//		EXPECT_EQ(date.day, date2.day);
//		EXPECT_EQ(date.month, date2.month);
//		EXPECT_EQ(date.year, date2.year);
//
//		EXPECT_EQ(time.hour, time2.hour);
//		EXPECT_EQ(time.minute, time2.minute);
//		EXPECT_EQ(time.second, time2.second);
//
//		EXPECT_EQ(timestamp.day, timestamp2.day);
//		EXPECT_EQ(timestamp.month, timestamp2.month);
//		EXPECT_EQ(timestamp.year, timestamp2.year);
//		EXPECT_EQ(timestamp.hour, timestamp2.hour);
//		EXPECT_EQ(timestamp.minute, timestamp2.minute);
//		EXPECT_EQ(timestamp.second, timestamp2.second);
//		EXPECT_EQ(timestamp.fraction, timestamp2.fraction);
//	}
//
//
//	TEST_F(TableTest, InsertAndUpdateManualNumericTypes)
//	{
//		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Clear the tmp-table
//		test::ClearNumericTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		Table nTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE_PK, numericTypesTmpTableName, L"", L"", L"");
//
//		// Set Columns
//		SQLINTEGER id = 0;
//		SQL_NUMERIC_STRUCT tdecimal_18_0;
//		SQL_NUMERIC_STRUCT tdecimal_18_10;
//		SQL_NUMERIC_STRUCT tdecimal_5_3;
//
//		nTable.SetColumn(0, test::ConvertNameCase(L"idnumerictypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT | CF_PRIMARY_KEY);
//		nTable.SetColumn(1, test::ConvertNameCase(L"tdecimal_18_0", m_odbcInfo.m_namesCase), SQL_NUMERIC, &tdecimal_18_0, SQL_C_NUMERIC, sizeof(tdecimal_18_0), CF_SELECT | CF_INSERT | CF_UPDATE | CF_NULLABLE, 18, 0);
//		nTable.SetColumn(2, test::ConvertNameCase(L"tdecimal_18_10", m_odbcInfo.m_namesCase), SQL_NUMERIC, &tdecimal_18_10, SQL_C_NUMERIC, sizeof(tdecimal_18_10), CF_SELECT | CF_INSERT | CF_UPDATE, 18, 10);
//		nTable.SetColumn(3, test::ConvertNameCase(L"tdecimal_5_3", m_odbcInfo.m_namesCase), SQL_NUMERIC, &tdecimal_5_3, SQL_C_NUMERIC, sizeof(tdecimal_5_3), CF_SELECT | CF_INSERT | CF_UPDATE | CF_NULLABLE, 5, 3);
//
//		ASSERT_NO_THROW(nTable.Open(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS));
//
//		// Insert some value by populating the struct of the column 18_10
//		ZeroMemory(&tdecimal_18_10, sizeof(tdecimal_18_10));
//		tdecimal_18_10.val[0] = 78;
//		tdecimal_18_10.val[1] = 243;
//		tdecimal_18_10.val[2] = 48;
//		tdecimal_18_10.val[3] = 166;
//		tdecimal_18_10.val[4] = 75;
//		tdecimal_18_10.val[5] = 155;
//		tdecimal_18_10.val[6] = 182;
//		tdecimal_18_10.val[7] = 1;
//
//		tdecimal_18_10.precision = 18;
//		tdecimal_18_10.scale = 10;
//		tdecimal_18_10.sign = 1; // insert positive
//
//		id = 199;
//		nTable.SetColumnNull(1);
//		nTable.SetColumnNull(3);
//		EXPECT_NO_THROW(nTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// And read back
//		Table nTable2(&m_db, AF_READ, numericTypesTmpTableName, L"", L"", L"");
//		EXPECT_NO_THROW(nTable2.Open());
//		EXPECT_NO_THROW(nTable2.Select());
//		EXPECT_TRUE(nTable2.SelectNext());
//		SQL_NUMERIC_STRUCT numStr2 = nTable2.GetNumeric(2);
//		EXPECT_EQ(0, memcmp(&tdecimal_18_10, &numStr2, sizeof(numStr2)));
//
//		// Update value, just change sign
//		tdecimal_18_10.sign = 0;
//		EXPECT_NO_THROW(nTable.Update());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Read back
//		EXPECT_NO_THROW(nTable2.Select());
//		EXPECT_TRUE(nTable2.SelectNext());
//		numStr2 = nTable2.GetNumeric(2);
//		EXPECT_EQ(0, memcmp(&tdecimal_18_10, &numStr2, sizeof(numStr2)));
//	}
//
//
//	TEST_F(TableTest, InsertAndUpdateManualBlobTypes)
//	{
//		std::wstring blobTypesTmpTableName = test::GetTableName(test::TableId::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Clear the tmp-table
//		test::ClearBlobTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		Table bTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE_PK, blobTypesTmpTableName, L"", L"", L"");
//
//		// Set Columns
//		SQLINTEGER id = 0;
//		SQLCHAR blob[16];
//		SQLCHAR varblob_20[20];
//
//		bTable.SetColumn(0, test::ConvertNameCase(L"idblobtypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT | CF_PRIMARY_KEY);
//		bTable.SetColumn(1, test::ConvertNameCase(L"tblob", m_odbcInfo.m_namesCase), SQL_BINARY, blob, SQL_C_BINARY, sizeof(blob), CF_SELECT | CF_INSERT | CF_UPDATE, 16);
//		bTable.SetColumn(2, test::ConvertNameCase(L"tvarblob_20", m_odbcInfo.m_namesCase), SQL_VARBINARY, varblob_20, SQL_C_BINARY, sizeof(varblob_20), CF_SELECT | CF_INSERT | CF_UPDATE, 20);
//
//		ASSERT_NO_THROW(bTable.Open(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS));
//
//		// Insert values, insert into the varblob only 16 bytes
//		SQLCHAR abc[] = { 0xab, 0xcd, 0xef, 0xf0,
//			0x12, 0x34, 0x56, 0x78,
//			0x90, 0xab, 0xcd, 0xef,
//			0x01, 0x23, 0x45, 0x67
//		};
//
//		id = 199;
//		memcpy(blob, abc, sizeof(abc));
//		memcpy(varblob_20, abc, sizeof(abc));
//		
//		// Set the length indicator
//		bTable.SetColumnLengthIndicator(1, 16);
//		bTable.SetColumnLengthIndicator(2, 16);
//		EXPECT_NO_THROW(bTable.Insert());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Read back values
//		Table bTable2(&m_db, AF_READ, blobTypesTmpTableName, L"", L"", L"");
//		EXPECT_NO_THROW(bTable2.Open());
//		EXPECT_NO_THROW(bTable2.Select());
//		EXPECT_NO_THROW(bTable2.SelectNext());
//
//		SQLLEN cb = 0;
//		SQLLEN buffSize = 0;
//		const SQLCHAR* pBuff = NULL;
//		pBuff = bTable2.GetBinaryValue(1, buffSize, cb);
//		EXPECT_EQ(sizeof(blob), buffSize);
//		EXPECT_EQ(sizeof(abc), cb);
//		EXPECT_EQ(0, memcmp(abc, blob, sizeof(abc)));
//
//		pBuff = bTable2.GetBinaryValue(2, buffSize, cb);
//		EXPECT_EQ(sizeof(varblob_20), buffSize);
//		EXPECT_EQ(sizeof(abc), cb);
//		EXPECT_EQ(0, memcmp(abc, varblob_20, sizeof(abc)));
//	}
//
//
//	// Update rows
//	// -----------
//	TEST_F(TableTest, UpdateIntTypes)
//	{
//		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table iTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE_PK, intTypesTableName, L"", L"", L"");
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			// Manually set the PKs on access
//			iTable.SetColumnPrimaryKeyIndexes({ 0 });
//		}
//		ASSERT_NO_THROW(iTable.Open());
//		
//		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Remove everything, ignoring if there was any data:
//		ASSERT_NO_THROW(test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase));
//
//		// Insert some values
//		for (int i = 1; i < 10; i++)
//		{
//			test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, m_db, i, i, i, i);
//		}
//		ASSERT_NO_THROW(m_db.CommitTrans());
//
//		// Update single rows using key-values
//		iTable.SetColumnValue(0, (SQLINTEGER)3);
//		iTable.SetColumnValue(2, (SQLINTEGER)102);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			iTable.SetColumnValue(1, (SQLINTEGER)101);
//			iTable.SetColumnNull(3);
//		}
//		else
//		{
//			iTable.SetColumnValue(1, (SQLSMALLINT)101);
//			iTable.SetColumnValue(3, (SQLBIGINT)103);
//		}
//		EXPECT_NO_THROW(iTable.Update());
//
//		iTable.SetColumnValue(0, (SQLINTEGER)5);
//		iTable.SetColumnValue(2, (SQLINTEGER)1002);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			iTable.SetColumnValue(1, (SQLINTEGER)1001);
//			iTable.SetColumnNull(3);
//		}
//		else
//		{
//			iTable.SetColumnValue(1, (SQLSMALLINT)1001);
//			iTable.SetColumnValue(3, (SQLBIGINT)1003);
//		}
//		EXPECT_NO_THROW(iTable.Update());
//
//		// And set some values to null
//		iTable.SetColumnValue(0, (SQLINTEGER)7);
//		iTable.SetColumnNull(1);
//		iTable.SetColumnValue(2, (SQLINTEGER)99);
//		iTable.SetColumnNull(3);
//		EXPECT_NO_THROW(iTable.Update());
//
//		// And set all to null
//		iTable.SetColumnValue(0, (SQLINTEGER)9);
//		iTable.SetColumnNull(1);
//		iTable.SetColumnNull(2);
//		iTable.SetColumnNull(3);
//		EXPECT_NO_THROW(iTable.Update());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Read back the just updated values
//		iTable.Select((boost::wformat(L"%s = 3") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 3, 101, 102, 103));
//
//		iTable.Select((boost::wformat(L"%s = 5") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 5, 1001, 1002, 1003));
//
//		iTable.Select((boost::wformat(L"%s = 7") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 7, test::ValueIndicator::IS_NULL, 99, test::ValueIndicator::IS_NULL));
//
//		iTable.Select((boost::wformat(L"%s = 9") % idName).str());
//		EXPECT_TRUE(iTable.SelectNext());
//		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 9, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL));
//	}
//
//
//	TEST_F(TableTest, UpdateDateTypes)
//	{
//		std::wstring dateTypesTableName = test::GetTableName(test::TableId::DATETYPES_TMP, m_odbcInfo.m_namesCase);
//		Table dTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE_PK, dateTypesTableName, L"", L"", L"");
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			dTable.SetColumnPrimaryKeyIndexes({ 0 });
//		}
//		ASSERT_NO_THROW(dTable.Open());
//
//		wstring idName = test::GetIdColumnName(test::TableId::DATETYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearDateTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		// Insert some values
//		SQL_TIME_STRUCT time = InitTime(13, 55, 56);
//		SQL_DATE_STRUCT date = InitDate(26, 1, 2983);
//		SQL_TIMESTAMP_STRUCT timestamp = InitTimestamp(13, 55, 56, 0, 26, 01, 1983);
//
//		dTable.SetColumnValue(0, (SQLINTEGER)101);
//		dTable.SetColumnValue(3, timestamp);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			dTable.SetColumnNull(1);
//			dTable.SetColumnNull(2);
//		}
//		else
//		{
//			dTable.SetColumnValue(1, date);
//			dTable.SetColumnValue(2, time);
//		}
//		dTable.Insert();
//		// and a null row
//		dTable.SetColumnValue(0, (SQLINTEGER)102);
//		dTable.SetColumnNull(1);
//		dTable.SetColumnNull(2);
//		dTable.SetColumnNull(3);
//		ASSERT_NO_THROW(dTable.Insert());
//		ASSERT_NO_THROW(m_db.CommitTrans());
//
//		// Now update the values
//		// \todo: We do not test the Fractions here. Lets fix Ticket #70 first
//		date = InitDate(20, 9, 1985);
//		time = InitTime(16, 31, 49);
//		timestamp = InitTimestamp(16, 31, 49, 0, 20, 9, 1985);
//		dTable.SetColumnValue(0, (SQLINTEGER)101);
//		dTable.SetColumnNull(1);
//		dTable.SetColumnNull(2);
//		dTable.SetColumnNull(3);
//		EXPECT_NO_THROW(dTable.Update());
//
//		dTable.SetColumnValue(0, (SQLINTEGER)102);
//		dTable.SetColumnValue(3, timestamp);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			dTable.SetColumnNull(1);
//			dTable.SetColumnNull(2);
//		}
//		else
//		{
//			dTable.SetColumnValue(1, date);
//			dTable.SetColumnValue(2, time);
//		}
//		EXPECT_NO_THROW(dTable.Update());
//		ASSERT_NO_THROW(m_db.CommitTrans());
//
//		// And read back
//		dTable.Select((boost::wformat(L"%s = 101") % idName).str());
//		EXPECT_TRUE(dTable.SelectNext());
//		EXPECT_TRUE(dTable.IsColumnNull(1));
//		EXPECT_TRUE(dTable.IsColumnNull(2));
//		EXPECT_TRUE(dTable.IsColumnNull(3));
//
//		dTable.Select((boost::wformat(L"%s = 102") % idName).str());
//		EXPECT_TRUE(dTable.SelectNext());
//		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
//		{
//			EXPECT_FALSE(dTable.IsColumnNull(1));
//			EXPECT_FALSE(dTable.IsColumnNull(2));
//			EXPECT_TRUE(IsDateEqual(date, dTable.GetDate(1)));
//			EXPECT_TRUE(IsTimeEqual(time, dTable.GetTime(2)));
//		}
//		EXPECT_FALSE(dTable.IsColumnNull(3));
//		EXPECT_TRUE(IsTimestampEqual(timestamp, dTable.GetTimeStamp(3)));
//	}
//
//	
//	TEST_F(TableTest, UpdateNumericTypes)
//	{
//		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table nTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE_PK, numericTypesTmpTableName, L"", L"", L"");
//		ASSERT_NO_THROW(nTable.Open());
//
//		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearNumericTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		// Insert some boring 0 entries
//		SQL_NUMERIC_STRUCT num = InitNullNumeric();
//		nTable.SetColumnValue(0, (SQLINTEGER)100);
//		nTable.SetColumnValue(1, num);
//		nTable.SetColumnValue(2, num);
//		nTable.SetColumnValue(3, num);
//
//		nTable.Insert();
//		nTable.SetColumnValue(0, (SQLINTEGER)101);
//		nTable.SetColumnNull(1);
//		nTable.SetColumnNull(2);
//		nTable.SetColumnNull(3);
//		nTable.Insert();
//		ASSERT_NO_THROW(m_db.CommitTrans());
//
//		// Now update that with our well known entries
//		// Note: We MUST set the correct precision and scale, at least for ms!
//		SQL_NUMERIC_STRUCT num18_0;
//		SQL_NUMERIC_STRUCT num18_10;
//		SQL_NUMERIC_STRUCT num5_3;
//		ZeroMemory(&num18_0, sizeof(num18_0));
//		ZeroMemory(&num18_10, sizeof(num18_10));
//		ZeroMemory(&num5_3, sizeof(num5_3));
//		num18_0.sign = 1;
//		num18_0.scale = 0;
//		num18_0.precision = 18;
//		num18_0.val[0] = 78;
//		num18_0.val[1] = 243;
//		num18_0.val[2] = 48;
//		num18_0.val[3] = 166;
//		num18_0.val[4] = 75;
//		num18_0.val[5] = 155;
//		num18_0.val[6] = 182;
//		num18_0.val[7] = 1;
//
//		num18_10 = num18_0;
//		num18_10.scale = 10;
//
//		num5_3.sign = 1;
//		num5_3.scale = 3;
//		num5_3.precision = 5;
//		num5_3.val[0] = 57;
//		num5_3.val[1] = 48;
//
//		nTable.SetColumnValue(0, (SQLINTEGER)101);
//		nTable.SetColumnValue(1, num18_0);
//		nTable.SetColumnValue(2, num18_10);
//		nTable.SetColumnValue(3, num5_3);
//		EXPECT_NO_THROW(nTable.Update());
//
//		nTable.SetColumnValue(0, (SQLINTEGER)100);
//		nTable.SetColumnNull(1);
//		nTable.SetColumnNull(2);
//		nTable.SetColumnNull(3);
//		EXPECT_NO_THROW(nTable.Update());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// And read back the values
//		nTable.Select((boost::wformat(L"%s = 101") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		SQL_NUMERIC_STRUCT n18_0, n18_10, n5_3;
//		n18_0 = nTable.GetNumeric(1);
//		n18_10 = nTable.GetNumeric(2);
//		n5_3 = nTable.GetNumeric(3);
//
//		EXPECT_EQ(0, memcmp(&num18_0, &n18_0, sizeof(n18_0)));
//		EXPECT_EQ(0, memcmp(&num18_10, &n18_10, sizeof(n18_10)));
//		EXPECT_EQ(0, memcmp(&num5_3, &n5_3, sizeof(n5_3)));
//
//		nTable.Select((boost::wformat(L"%s = 100") % idName).str());
//		EXPECT_TRUE(nTable.SelectNext());
//		EXPECT_TRUE(nTable.IsColumnNull(1));
//		EXPECT_TRUE(nTable.IsColumnNull(2));
//		EXPECT_TRUE(nTable.IsColumnNull(3));
//	}
//
//
//	TEST_F(TableTest, UpdateFloatTypes)
//	{
//		std::wstring floatTypesTableName = test::GetTableName(test::TableId::FLOATTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table fTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE_PK, floatTypesTableName, L"", L"", L"");
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			fTable.SetColumnPrimaryKeyIndexes({ 0 });
//		}
//		ASSERT_NO_THROW(fTable.Open());
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearFloatTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		// Insert some values
//		fTable.SetColumnValue(0, (SQLINTEGER)101);
//		fTable.SetColumnValue(1, (SQLDOUBLE)3.14159265359);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS || m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			// Access binds as double
//			// also ms sql server, it seems to report SQLFLOAT which we map to SQL_C_FLOAT, which is a DOUBLE
//			fTable.SetColumnValue(2, (SQLDOUBLE)-3.14159);
//		}
//		else
//		{
//			fTable.SetColumnValue(2, (SQLREAL)-3.14159);
//		}
//		fTable.Insert();
//		ASSERT_NO_THROW(m_db.CommitTrans());
//
//		// And update them using the key fields
//		fTable.SetColumnValue(1, (SQLDOUBLE)-6.2343354);
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS || m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			fTable.SetColumnValue(2, (SQLDOUBLE)989.213);
//		}
//		else
//		{
//			fTable.SetColumnValue(2, (SQLREAL)989.213);
//		}
//		EXPECT_NO_THROW(fTable.Update());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Open another table and read the values from there
//		Table fTable2(&m_db, AF_READ, floatTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(fTable2.Open());
//
//		fTable2.Select();
//		EXPECT_TRUE(fTable2.SelectNext());
//
//		EXPECT_EQ(101, fTable2.GetInt(0));
//		EXPECT_EQ((int)(1e7 * -6.2343354), (int)(1e7 * fTable2.GetDouble(1)));
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS || m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
//		{
//			EXPECT_EQ((int)(1e3 * 989.213), (int)(1e3 * fTable2.GetDouble(2)));
//		}
//		else
//		{
//			EXPECT_EQ((int)(1e3 * 989.213), (int)(1e3 * fTable2.GetReal(2)));
//		}
//	}
//
//
//	TEST_F(TableTest, UpdateWCharTypes)
//	{
//		std::wstring charTypesTmpTableName = test::GetTableName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table cTable(&m_db, AF_SELECT | AF_UPDATE_PK | AF_INSERT, charTypesTmpTableName, L"", L"", L"");
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			cTable.SetColumnPrimaryKeyIndexes({ 0 });
//		}
//		cTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new CharAsWCharSql2BufferMap(m_db.GetMaxSupportedOdbcVersion())));
//		ASSERT_NO_THROW(cTable.Open());
//		cTable.SetCharTrimRight(true);
//		
//		// Remove everything, ignoring if there was any data:
//		test::ClearCharTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//		wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Insert some values:
//		// \todo: Note, in IBM DB2 special chars seem to occupy more space (two bytes`?). We cannot have more than 5 special chars if the size of the field is 10..
//		// but this might be because we bind a char to wchar or so.. hm..
//		std::wstring s100 = L"�����";
//		cTable.SetColumnValue(0, (SQLINTEGER)100);
//		cTable.SetColumnValue(1, s100);
//		cTable.SetColumnValue(2, s100);
//		cTable.SetColumnValue(3, s100);
//		cTable.SetColumnValue(4, s100);
//
//		cTable.Insert();
//		std::wstring s101 = L"abcde12345";
//		cTable.SetColumnValue(0, (SQLINTEGER)101);
//		cTable.SetColumnValue(1, s101);
//		cTable.SetColumnValue(2, s101);
//		cTable.SetColumnValue(3, s101);
//		cTable.SetColumnValue(4, s101);
//
//		cTable.Insert();
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Select and check
//		cTable.Select((boost::wformat(L"%s = 100") % idName).str());
//		EXPECT_TRUE(cTable.SelectNext());
//		std::wstring s2;
//		EXPECT_NO_THROW(s2 = cTable.GetWString(1));
//		EXPECT_EQ(s100, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetWString(2));
//		EXPECT_EQ(s100, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetWString(3));
//		EXPECT_EQ(s100, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetWString(4));
//		EXPECT_EQ(s100, s2);
//
//		cTable.Select((boost::wformat(L"%s = 101") % idName).str());
//		EXPECT_TRUE(cTable.SelectNext());
//		EXPECT_NO_THROW(s2 = cTable.GetWString(1));
//		EXPECT_EQ(s101, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetWString(2));
//		EXPECT_EQ(s101, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetWString(3));
//		EXPECT_EQ(s101, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetWString(4));
//		EXPECT_EQ(s101, s2);
//	}
//
//
//	TEST_F(TableTest, UpdateCharTypes)
//	{
//		std::wstring charTypesTmpTableName = test::GetTableName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table cTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE, charTypesTmpTableName, L"", L"", L"");
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			cTable.SetColumnPrimaryKeyIndexes({ 0 });
//		}
//		cTable.SetSql2BufferTypeMap(Sql2BufferTypeMapPtr(new WCharAsCharSql2BufferMap(m_db.GetMaxSupportedOdbcVersion())));
//		ASSERT_NO_THROW(cTable.Open());
//		cTable.SetCharTrimRight(true);
//
//		// Remove everything, ignoring if there was any data:
//		test::ClearCharTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//		wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
//
//		// Insert some values:
//		// \todo: Note, in IBM DB2 special chars seem to occupy more space (two bytes`?). We cannot have more than 5 special chars if the size of the field is 10..
//		// but this might be because we bind a char to wchar or so.. hm..
//		std::string s100 = "abcd";
//		cTable.SetColumnValue(0, (SQLINTEGER)100);
//		cTable.SetColumnValue(1, s100);
//		cTable.SetColumnValue(2, s100);
//		cTable.SetColumnValue(3, s100);
//		cTable.SetColumnValue(4, s100);
//		cTable.Insert();
//		std::string s101 = "abcde12345";
//		cTable.SetColumnValue(0, (SQLINTEGER)101);
//		cTable.SetColumnValue(1, s101);
//		cTable.SetColumnValue(2, s101);
//		cTable.SetColumnValue(3, s101);
//		cTable.SetColumnValue(4, s101);
//		cTable.Insert();
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Select and check
//		cTable.Select((boost::wformat(L"%s = 100") % idName).str());
//		EXPECT_TRUE(cTable.SelectNext());
//		std::string s2;
//		EXPECT_NO_THROW(s2 = cTable.GetString(1));
//		EXPECT_EQ(s100, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetString(2));
//		EXPECT_EQ(s100, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetString(3));
//		EXPECT_EQ(s100, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetString(4));
//		EXPECT_EQ(s100, s2);
//
//		cTable.Select((boost::wformat(L"%s = 101") % idName).str());
//		EXPECT_TRUE(cTable.SelectNext());
//		EXPECT_NO_THROW(s2 = cTable.GetString(1));
//		EXPECT_EQ(s101, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetString(2));
//		EXPECT_EQ(s101, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetString(3));
//		EXPECT_EQ(s101, s2);
//		EXPECT_NO_THROW(s2 = cTable.GetString(4));
//		EXPECT_EQ(s101, s2);
//	}
//
//
//	TEST_F(TableTest, UpdateBlobTypes)
//	{
//		std::wstring blobTypesTmpTableName = test::GetTableName(test::TableId::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
//		Table bTable(&m_db, AF_SELECT | AF_INSERT | AF_UPDATE, blobTypesTmpTableName, L"", L"", L"");
//		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
//		{
//			bTable.SetColumnPrimaryKeyIndexes({ 0 });
//		}
//		ASSERT_NO_THROW(bTable.Open());
//
//		// Remove everything, ignoring if there was any data:
//		wstring idName = test::GetIdColumnName(test::TableId::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
//		wstring sqlstmt;
//		test::ClearBlobTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
//
//		SQLCHAR empty[] = { 0, 0, 0, 0,
//			0, 0, 0, 0,
//			0, 0, 0, 0,
//			0, 0, 0, 0
//		};
//
//		SQLCHAR ff[] = { 255, 255, 255, 255,
//			255, 255, 255, 255,
//			255, 255, 255, 255,
//			255, 255, 255, 255
//		};
//
//		SQLCHAR abc[] = { 0xab, 0xcd, 0xef, 0xf0,
//			0x12, 0x34, 0x56, 0x78,
//			0x90, 0xab, 0xcd, 0xef,
//			0x01, 0x23, 0x45, 0x67
//		};
//
//		SQLCHAR abc_ff[] = { 0xab, 0xcd, 0xef, 0xf0,
//			0x12, 0x34, 0x56, 0x78,
//			0x90, 0xab, 0xcd, 0xef,
//			0x01, 0x23, 0x45, 0x67,
//			0xff, 0xff, 0xff, 0xff
//		};
//
//		// Insert some values
//		bTable.SetColumnValue(0, (SQLINTEGER)100);
//		bTable.SetBinaryValue(1, abc, sizeof(abc));
//		bTable.SetColumnNull(2);
//		bTable.Insert();
//		bTable.SetColumnValue(0, (SQLINTEGER)101);
//		bTable.SetColumnNull(1);
//		bTable.SetBinaryValue(2, abc_ff, sizeof(abc_ff));
//		bTable.Insert();
//		ASSERT_NO_THROW(m_db.CommitTrans());
//
//		// Update 
//		bTable.SetColumnValue(0, (SQLINTEGER)100);
//		bTable.SetColumnNull(1);
//		bTable.SetBinaryValue(2, abc, sizeof(abc));
//		EXPECT_NO_THROW(bTable.Update());
//		bTable.SetColumnValue(0, (SQLINTEGER)101);
//		bTable.SetBinaryValue(1, empty, sizeof(empty));
//		bTable.SetColumnNull(2);
//		EXPECT_NO_THROW(bTable.Update());
//		EXPECT_NO_THROW(m_db.CommitTrans());
//
//		// Re-Fetch and compare
//		bTable.Select((boost::wformat(L"%s = 100") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_TRUE(bTable.IsColumnNull(1));
//		EXPECT_FALSE(bTable.IsColumnNull(2));
//		const SQLCHAR* pBlobBuff = NULL;
//		SQLLEN size, length = 0;
//		EXPECT_NO_THROW(pBlobBuff = bTable.GetBinaryValue(2, size, length));
//		EXPECT_EQ(0, memcmp(pBlobBuff, abc, sizeof(abc)));
//		EXPECT_EQ(20, size);
//		EXPECT_EQ(16, length);
//		bTable.Select((boost::wformat(L"%s = 101") % idName).str());
//		EXPECT_TRUE(bTable.SelectNext());
//		EXPECT_FALSE(bTable.IsColumnNull(1));
//		EXPECT_TRUE(bTable.IsColumnNull(2));
//		EXPECT_NO_THROW(pBlobBuff = bTable.GetBinaryValue(1, size, length));
//		EXPECT_EQ(0, memcmp(pBlobBuff, empty, sizeof(empty)));
//		EXPECT_EQ(16, size);
//		EXPECT_EQ(16, length);
//	}
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
//
//
//	TEST_F(TableTest, GetColumnBufferIndex)
//	{
//		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
//		Table iTable(&m_db, AF_SELECT, intTypesTableName, L"", L"", L"");
//		ASSERT_NO_THROW(iTable.Open());
//
//		EXPECT_EQ(0, iTable.GetColumnBufferIndex(L"idintegertypes", false));
//		EXPECT_EQ(1, iTable.GetColumnBufferIndex(L"tsmallint", false));
//		EXPECT_EQ(2, iTable.GetColumnBufferIndex(L"tint", false));
//		EXPECT_EQ(3, iTable.GetColumnBufferIndex(L"tbigint", false));
//	}
//

// Interfaces
// ----------

} // namespace exodbc
