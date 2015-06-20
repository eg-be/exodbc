/*!
 * \file DbTableTest.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright wxWindows Library Licence, Version 3.1
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "TableTest.h"

// Same component headers
#include "exOdbcGTest.h"
#include "ManualTestTables.h"

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

namespace exodbc
{
	void TableTest::SetUpTestCase()
	{

	}


	void TableTest::SetUp()
	{
		m_odbcInfo = GetParam();

		// Set up Env
		m_env.AllocateEnvironmentHandle();
		// Try to set to the ODBC v3 : We need that for the tests to run correct. 3.8 is not supported by all databases and we dont use specific stuff from it.
		// except for the TIME2 things, sql server specific. those tests can create their own env.
		m_env.SetOdbcVersion(OdbcVersion::V_3);

		// And database
		ASSERT_NO_THROW(m_db.AllocateConnectionHandle(&m_env));
		if (m_odbcInfo.HasConnectionString())
		{
			ASSERT_NO_THROW(m_db.Open(m_odbcInfo.m_connectionString));
		}
		else
		{
			ASSERT_NO_THROW(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
	}


	void TableTest::TearDown()
	{
		if (m_db.IsOpen())
		{
			EXPECT_NO_THROW(m_db.Close());
		}
	}
	

	// Open / Close
	// ------------
	TEST_P(TableTest, OpenManualReadOnlyWithoutCheck)
	{
		// Open an existing table without checking for privileges or existence
		MIntTypesTable table(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_NO_THROW(table.Open(TOF_NONE));

		// If we pass in the TableInfo directly we should also be able to "open"
		// a totally non-sense table:
		TableInfo neTableInfo(L"NotExisting", L"", L"", L"", L"");
		Table neTable(&m_db, 2, neTableInfo, AF_READ);
		SQLINTEGER idNotExisting = 0;
		neTable.SetColumn(0, L"idNotExistring", &idNotExisting, SQL_C_SLONG, sizeof(idNotExisting));
		EXPECT_NO_THROW(neTable.Open(TOF_NONE));
		// \todo So we can prove in the test that we will fail doing a SELECT later
	}


	TEST_P(TableTest, OpenManualWritableWithoutCheckPrivOrExist)
	{
		// Open an existing table without checking for privileges or existence
		Table iTable(&m_db, 4, test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase), L"", L"", L"", AF_READ_WRITE);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		SQLINTEGER bii = 0;
		int type = SQL_C_SLONG;
		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT | CF_PRIMARY_KEY);
		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_INSERT | CF_UPDATE);
		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			// access has no bigint
			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bii, SQL_C_SLONG, sizeof(bii), CF_SELECT | CF_INSERT);
		}
		else
		{
			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
		}
		EXPECT_NO_THROW(iTable.Open(TOF_NONE));
	}


	TEST_P(TableTest, OpenManualCheckExistence)
	{
		// Open a table with checking for existence
		MIntTypesTable table(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_NO_THROW(table.Open(TOF_CHECK_EXISTANCE));

		// Open a non-existing table
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			MNotExistingTable neTable(&m_db, m_odbcInfo.m_namesCase);
			EXPECT_THROW(neTable.Open(TOF_CHECK_EXISTANCE), exodbc::Exception);
		}
	}


	TEST_P(TableTest, OpenManualCheckColumnFlagSelect)
	{
		// Open a table manually but do not set the Select flag for all columns
		Table iTable(&m_db, 4, test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), &id, SQL_C_SLONG, sizeof(id), CF_SELECT);
		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_NULLABLE);
		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), &i, SQL_C_SLONG, sizeof(i), CF_NONE | CF_NULLABLE);
		iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_NULLABLE);

		ASSERT_NO_THROW(iTable.Open());
		// We expect all columnBuffers to be bound, except nr 2
		ColumnBuffer* pBuffId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pBuffsi = iTable.GetColumnBuffer(1);
		ColumnBuffer* pBuffi = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBuffbi = iTable.GetColumnBuffer(3);
		EXPECT_TRUE(pBuffId->IsBound());
		EXPECT_TRUE(pBuffsi->IsBound());
		EXPECT_FALSE(pBuffi->IsBound());
		EXPECT_TRUE(pBuffbi->IsBound());

		// And we should be able to select a row
		wstring sqlstmt = boost::str(boost::wformat(L"%s = 7") % test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase));
		iTable.Select(sqlstmt);
		EXPECT_TRUE(iTable.SelectNext());
		// and have all values
		EXPECT_EQ(7, id);
		EXPECT_EQ(-13, si);
		EXPECT_EQ(0, i);
		EXPECT_EQ(10502, bi);
		{
			LogLevelFatal llf;
			DontDebugBreak ddb;
			// except the not bound column, we are unable to get its value, but its buffer has not changed
			EXPECT_THROW(boost::get<SQLINTEGER>(iTable.GetColumnValue(2)), AssertionException);
		}
	}


	TEST_P(TableTest, OpenManualCheckColumnFlagInsert)
	{
		// Open a table manually but do not set the Insert flag for all columns
		Table iTable(&m_db, 4, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT | AF_INSERT | AF_DELETE);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		int type = SQL_C_SLONG;

		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT | CF_PRIMARY_KEY);
		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_INSERT);
		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
		}
		else
		{
			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
		}

		// Open and remove all data from the table
		iTable.Open();
		ASSERT_NO_THROW(test::ClearTestTable(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase, iTable, m_db));

		// Insert a value by using our primary key - columnIndex 2 will not get inserted but the default NULL value will be set by the db
		iTable.SetColumnValue(0, (SQLINTEGER)11);
		iTable.SetColumnValue(1, (SQLSMALLINT)202);
		iTable.SetColumnValue(2, (SQLINTEGER)303);
		iTable.SetColumnValue(3, (SQLBIGINT)-404);
		EXPECT_NO_THROW(iTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Read back from another table
		// note that when opening an Access Table Access automatically, the second column will not get bound to a SMALLINT, as
		// Access reports it as SQL_INTEGER (Db-Type) which corresponds to C-Type of SQL_C_SLONG.
		// The other databases report it as SQL_SMALLINT which we bind to SQL_C_SSHORT
		Table iTable2(&m_db, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_READ);
		iTable2.Open();
		iTable2.Select();
		ASSERT_TRUE(iTable2.SelectNext());
		EXPECT_EQ(11, boost::get<SQLINTEGER>(iTable2.GetColumnValue(0)));
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			EXPECT_EQ(202, boost::get<SQLINTEGER>(iTable2.GetColumnValue(1)));
			EXPECT_EQ(-404, boost::get<SQLINTEGER>(iTable2.GetColumnValue(3)));
		}
		else
		{
			EXPECT_EQ(202, boost::get<SQLSMALLINT>(iTable2.GetColumnValue(1)));
			EXPECT_EQ(-404, boost::get<SQLBIGINT>(iTable2.GetColumnValue(3)));
		}
		EXPECT_TRUE(iTable2.IsColumnNull(2));
	}


	TEST_P(TableTest, OpenManualCheckColumnFlagUpdate)
	{
		// Open a table manually but do not set the Update flag for all columns
		Table iTable(&m_db, 4, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT | AF_UPDATE | AF_DELETE | AF_INSERT);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		int type = SQL_C_SLONG;

		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_UPDATE | CF_INSERT | CF_PRIMARY_KEY);
		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_UPDATE | CF_INSERT);
		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT | CF_INSERT);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT | CF_UPDATE);
		}
		else
		{
			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT | CF_UPDATE);
		}

		// Open and remove all data from the table
		iTable.Open();
		ASSERT_NO_THROW(test::ClearTestTable(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase, iTable, m_db));

		// Insert some value to update later
		iTable.SetColumnValue(0, (SQLINTEGER)11);
		iTable.SetColumnValue(1, (SQLSMALLINT)202);
		iTable.SetColumnValue(2, (SQLINTEGER)303);
		iTable.SetColumnValue(3, (SQLBIGINT)-404);
		EXPECT_NO_THROW(iTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Select to update
		iTable.Select();
		ASSERT_TRUE(iTable.SelectNext());

		// Update - note column 2 will not get updated, flag not set
		// iTable.SetColumnValue(0, (SQLINTEGER) 11); //pk
		iTable.SetColumnValue(1, (SQLSMALLINT) 880);
		iTable.SetColumnValue(2, (SQLINTEGER) 990);
		iTable.SetColumnValue(3, (SQLBIGINT) 1001);
		EXPECT_NO_THROW(iTable.Update());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Read back from another table - column 2 still has the originally inserted value
		Table iTable2(&m_db, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_READ);
		iTable2.Open();
		iTable2.Select();
		ASSERT_TRUE(iTable2.SelectNext());
		EXPECT_EQ(11, boost::get<SQLINTEGER>(iTable2.GetColumnValue(0)));
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			EXPECT_EQ(880, boost::get<SQLINTEGER>(iTable2.GetColumnValue(1)));
			EXPECT_EQ(1001, boost::get<SQLINTEGER>(iTable2.GetColumnValue(3)));
		}
		else
		{
			EXPECT_EQ(880, boost::get<SQLSMALLINT>(iTable2.GetColumnValue(1)));
			EXPECT_EQ(1001, boost::get<SQLBIGINT>(iTable2.GetColumnValue(3)));
		}
		EXPECT_EQ(303, boost::get<SQLINTEGER>(iTable2.GetColumnValue(2)));
	}
	

	TEST_P(TableTest, OpenManualPrimaryKeys)
	{
		// Open a table by defining primary keys manually
		// Open a table manually but do not set the Select flag for all columns
		Table iTable(&m_db, 4, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT | AF_DELETE | AF_INSERT);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT |  CF_INSERT | CF_PRIMARY_KEY);
		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_INSERT);
		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT | CF_INSERT);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
		}
		else
		{
			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
		}

		// Opening must work
		EXPECT_NO_THROW(iTable.Open(TOF_DO_NOT_QUERY_PRIMARY_KEYS));

		// But opening if primary keys are not defined must fail
		Table iTable2(&m_db, 4, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT | AF_DELETE | AF_INSERT);
		SQLINTEGER id2 = 0;
		SQLSMALLINT si2 = 0;
		SQLINTEGER i2 = 0;
		SQLBIGINT bi2 = 0;
		iTable2.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id2, SQL_C_SLONG, sizeof(id2), CF_SELECT | CF_INSERT);
		iTable2.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si2, SQL_C_SSHORT, sizeof(si2), CF_SELECT | CF_INSERT);
		iTable2.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i2, SQL_C_SLONG, sizeof(i2), CF_SELECT | CF_INSERT);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable2.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
		}
		else
		{
			iTable2.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
		}
		EXPECT_THROW(iTable2.Open(TOF_DO_NOT_QUERY_PRIMARY_KEYS), Exception);

		// But if we open for select only, we do not care about the primary keys
		Table iTable3(&m_db, 4, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_READ);
		SQLINTEGER id3 = 0;
		SQLSMALLINT si3 = 0;
		SQLINTEGER i3 = 0;
		SQLBIGINT bi3 = 0;
		iTable3.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id3, SQL_C_SLONG, sizeof(id3), CF_SELECT);
		iTable3.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si3, SQL_C_SSHORT, sizeof(si3), CF_SELECT);
		iTable3.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i3, SQL_C_SLONG, sizeof(i3), CF_SELECT);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable3.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT);
		}
		else
		{
			iTable3.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT);
		}
		EXPECT_NO_THROW(iTable3.Open(TOF_DO_NOT_QUERY_PRIMARY_KEYS));
	}


	TEST_P(TableTest, OpenAutoWithoutCheck)
	{
		// Open an auto-table without checking for privileges or existence
		// This makes only sense if we've already determined the correct TableInfo structure
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		TableInfo tableInfo;
		EXPECT_NO_THROW(tableInfo = m_db.FindOneTable(tableName, L"", L"", L""));

		exodbc::Table table(&m_db, tableInfo, AF_READ);
		EXPECT_NO_THROW(table.Open(TOF_NONE));

		// If we try to open an auto-table this will never work if you've passed invalid information:
		// As soon as the columns are searched, we expect to fail
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			TableInfo neTableInfo(L"NotExisting", L"", L"", L"", L"");
			Table neTable(&m_db, neTableInfo, AF_READ);
			EXPECT_THROW(neTable.Open(TOF_NONE), Exception);
		}
	}


	TEST_P(TableTest, OpenAutoCheckExistence)
	{
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table table(&m_db, tableName, L"", L"", L"", AF_READ);
		EXPECT_NO_THROW(table.Open(TOF_CHECK_EXISTANCE));

		// Open a non-existing table
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			std::wstring neName = test::GetTableName(test::TableId::NOT_EXISTING, m_odbcInfo.m_namesCase);
			exodbc::Table neTable(&m_db, neName, L"", L"", L"", AF_READ);
			EXPECT_THROW(neTable.Open(TOF_CHECK_EXISTANCE), Exception);
		}
	}


	TEST_P(TableTest, OpenAutoCheckPrivs)
	{


		// Test to open read-only a table we know we have all rights:
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table rTable(&m_db, tableName, L"", L"", L"", AF_READ);
		EXPECT_NO_THROW(rTable.Open(TOF_CHECK_PRIVILEGES));

		// Test to open read-write a table we know we have all rights:
		exodbc::Table rTable2(&m_db, tableName, L"", L"", L"", AF_READ_WRITE);
		EXPECT_NO_THROW(rTable2.Open(TOF_CHECK_PRIVILEGES));

		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			// We only have a Read-only user for ms sql server
			Database db(&m_env);
			if (m_odbcInfo.HasConnectionString())
			{
				return;
			}
			else
			{
				ASSERT_NO_THROW(db.Open(m_odbcInfo.m_dsn, L"exReadOnly", L"exReadOnly"));
			}

			// Test to open a table read-only
			// Note that here in ms server we have given the user no rights except the select for this table
			// If you add him to some role, things will get messed up: No privs are reported, but the user can
			// still access the table for selecting
			exodbc::Table table2(&db, tableName, L"", L"", L"", AF_READ);
			EXPECT_NO_THROW(table2.Open(TOF_CHECK_PRIVILEGES));

			// We expect to fail if trying to open for writing
			table2.Close();
			table2.SetAccessFlags(AF_READ_WRITE);
			EXPECT_THROW(table2.Open(TOF_CHECK_PRIVILEGES), Exception);

			// But we do not fail if we do not check the privs
			EXPECT_NO_THROW(table2.Open());

			// Try to open one we do not even have the rights for
			std::wstring table3Name = test::GetTableName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
			Table table3(&db, table3Name, L"", L"", L"", AF_READ);
			EXPECT_THROW(table3.Open(), Exception);
		}
	}


	TEST_P(TableTest, OpenAutoWithUnsupportedColumn)
	{
		std::wstring tableName = test::GetTableName(test::TableId::NOT_SUPPORTED, m_odbcInfo.m_namesCase);
		exodbc::Table nst(&m_db, tableName, L"", L"", L"", AF_READ_WRITE);

		// Expect to fail if we open with default flags
		EXPECT_THROW(nst.Open(), NotSupportedException);

		// But not if we pass the flag to skip
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// We should now be able to select from column indexed 0 (id), 1 (int1) and 3 (int2) - 2 (xml) should be missing
		nst.Select();
		EXPECT_TRUE(nst.SelectNext());
		SQLINTEGER id, int1, int2;
		EXPECT_NO_THROW(id = nst.GetInt(0));
		EXPECT_NO_THROW(int1 = nst.GetInt(1));
		EXPECT_FALSE(nst.ColumnBufferExists(2));
		EXPECT_THROW(nst.GetColumnBuffer(2), IllegalArgumentException);
		EXPECT_NO_THROW(int2 = nst.GetInt(3));
		EXPECT_EQ(1, id);
		EXPECT_EQ(10, int1);
		EXPECT_EQ(12, int2);
	}


	TEST_P(TableTest, SelectFromAutoWithUnsupportedColumn)
	{


		std::wstring tableName = test::GetTableName(test::TableId::NOT_SUPPORTED, m_odbcInfo.m_namesCase);
		exodbc::Table nst(&m_db, tableName, L"", L"", L"", AF_READ_WRITE);

		// we do not if we pass the flag to skip
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// We should now be able to select from column indexed 0 (id), 1 (int1) and 3 (int2) - 2 (xml) should be missing
		nst.Select();
		EXPECT_TRUE(nst.SelectNext());
		SQLINTEGER id, int1, int2;
		EXPECT_NO_THROW(id = nst.GetInt(0));
		EXPECT_NO_THROW(int1 = nst.GetInt(1));
		EXPECT_THROW(nst.GetColumnBuffer(2), IllegalArgumentException);
		EXPECT_NO_THROW(int2 = nst.GetInt(3));
		EXPECT_EQ(1, id);
		EXPECT_EQ(10, int1);
		EXPECT_EQ(12, int2);
	}


	TEST_P(TableTest, InsertIntoAutoWithUnsupportedColumn)
	{


		std::wstring tableName = test::GetTableName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table nst(&m_db, tableName, L"", L"", L"", AF_READ_WRITE);

		// we do not if we pass the flag to skip
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// Remove everything, ignoring if there was any data:
		wstring idName = test::GetIdColumnName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(nst.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert some data
		EXPECT_FALSE(nst.ColumnBufferExists(2));
		nst.SetColumnValue(0, (SQLINTEGER)2);
		nst.SetColumnValue(1, (SQLINTEGER)20);
		nst.SetColumnValue(3, (SQLINTEGER)22);
		EXPECT_NO_THROW(nst.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// We should now be able to select. As a test, use a different table object
		exodbc::Table nst2(&m_db, tableName, L"", L"", L"", AF_READ_WRITE);
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst2.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		nst2.Select();
		EXPECT_TRUE(nst2.SelectNext());
		SQLINTEGER id, int1, int2;
		EXPECT_NO_THROW(id = nst2.GetInt(0));
		EXPECT_NO_THROW(int1 = nst2.GetInt(1));
		EXPECT_NO_THROW(int2 = nst2.GetInt(3));
		EXPECT_EQ(2, id);
		EXPECT_EQ(20, int1);
		EXPECT_EQ(22, int2);
	}


	TEST_P(TableTest, UpdateIntoAutoWithUnsupportedColumn)
	{


		std::wstring tableName = test::GetTableName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table nst(&m_db, tableName, L"", L"", L"", AF_READ_WRITE);

		// we do not if we pass the flag to skip
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// Remove everything, ignoring if there was any data:
		wstring idName = test::GetIdColumnName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(nst.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert some data
		nst.SetColumnValue(0, (SQLINTEGER)2);
		nst.SetColumnValue(1, (SQLINTEGER)20);
		nst.SetColumnValue(3, (SQLINTEGER)22);
		EXPECT_NO_THROW(nst.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Now update it
		nst.SetColumnValue(1, (SQLINTEGER)30);
		nst.SetColumnValue(3, (SQLINTEGER)32);
		EXPECT_NO_THROW(nst.Update());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// We should now be able to select. As a test, use a different table object
		exodbc::Table nst2(&m_db, tableName, L"", L"", L"", AF_READ_WRITE);
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst2.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		nst2.Select();
		EXPECT_TRUE(nst2.SelectNext());
		SQLINTEGER id, int1, int2;
		EXPECT_NO_THROW(id = nst2.GetInt(0));
		EXPECT_NO_THROW(int1 = nst2.GetInt(1));
		EXPECT_NO_THROW(int2 = nst2.GetInt(3));
		EXPECT_EQ(2, id);
		EXPECT_EQ(30, int1);
		EXPECT_EQ(32, int2);
	}


	TEST_P(TableTest, DeleteFromAutoWithUnsupportedColumn)
	{


		std::wstring tableName = test::GetTableName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table nst(&m_db, tableName, L"", L"", L"", AF_READ_WRITE);

		// we do not if we pass the flag to skip
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// Remove everything, ignoring if there was any data:
		wstring idName = test::GetIdColumnName(test::TableId::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(nst.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert some data
		nst.SetColumnValue(0, (SQLINTEGER)2);
		nst.SetColumnValue(1, (SQLINTEGER)20);
		nst.SetColumnValue(3, (SQLINTEGER)22);
		EXPECT_NO_THROW(nst.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Check its there
		ASSERT_NO_THROW(nst.Select());
		ASSERT_TRUE(nst.SelectNext());

		// Now delete it
		nst.SetColumnValue(0, (SQLINTEGER)2);
		EXPECT_NO_THROW(nst.Delete());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// We should now longer be able to select it. As a test, use a different table object
		exodbc::Table nst2(&m_db, tableName, L"", L"", L"", AF_READ_WRITE);
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst2.Open(TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		nst2.Select();
		EXPECT_FALSE(nst2.SelectNext());
	}


	TEST_P(TableTest, Close)
	{
		// Create table
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", AF_READ);

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


	TEST_P(TableTest, OpenAndCloseAndOpenAndClose)
	{
		// Create a Table, open for reading
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(iTable.Open());

		// Close Table
		EXPECT_NO_THROW(iTable.Close());

		// Open again for reading
		EXPECT_NO_THROW(iTable.Open());

		// And close again
		EXPECT_NO_THROW(iTable.Close());
	}


	TEST_P(TableTest, SetAccessFlags)
	{
		// Create a Table, for reading only
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", AF_READ);

		// Before opening it, change the mode, then open
		ASSERT_NO_THROW(iTable.SetAccessFlags(AF_READ_WRITE));
		// Do not forget to set the primary keys if this is an Access database
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		ASSERT_NO_THROW(iTable.Open());

		// We cannot change the mode if the table is open
		{
			DontDebugBreak ddb;
			LogLevelFatal llf;
			EXPECT_THROW(iTable.SetAccessFlags(AF_READ), AssertionException);
		}

		// But when we close first we can
		EXPECT_NO_THROW(iTable.Close());
		EXPECT_NO_THROW(iTable.SetAccessFlags(AF_READ));
		// And we can open it again
		EXPECT_NO_THROW(iTable.Open());
	}


	TEST_P(TableTest, IsQueryOnly)
	{
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", AF_READ);
		
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(AF_UPDATE);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(AF_UPDATE);
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(AF_INSERT);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(AF_INSERT);
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(AF_DELETE);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(AF_DELETE);
		EXPECT_TRUE(iTable.IsQueryOnly());

		// and one that is initially rw
		exodbc::Table iTable2(&m_db, tableName, L"", L"", L"", AF_READ_WRITE);
		EXPECT_FALSE(iTable2.IsQueryOnly());
		iTable2.ClearAccessFlag(AF_INSERT);
		EXPECT_FALSE(iTable2.IsQueryOnly());

		iTable2.ClearAccessFlag(AF_DELETE);
		EXPECT_FALSE(iTable2.IsQueryOnly());

		iTable2.ClearAccessFlag(AF_UPDATE);
		EXPECT_TRUE(iTable2.IsQueryOnly());

		// remove read, we are no longer query only
		iTable2.ClearAccessFlag(AF_SELECT);
		EXPECT_FALSE(iTable2.IsQueryOnly());
	}


	TEST_P(TableTest, MissingAccessFlagsThrowOnWrite)
	{
		// Open a read-only table
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", AF_READ);

		ASSERT_NO_THROW(iTable.Open());

		// we should be able to set some silly value..
		iTable.SetColumnValue(0, (SQLINTEGER)333);
		// .. but not insert, delete or update it
		{
			LogLevelFatal llf;
			DontDebugBreak ddb;
			EXPECT_THROW(iTable.Insert(), AssertionException);
			EXPECT_THROW(iTable.Delete(), AssertionException);
			EXPECT_THROW(iTable.Update(), AssertionException);
		}

		// we test that writing works in all those cases where we open RW
	}


	// Select / GetNext
	// ----------------
	TEST_P(TableTest, Select)
	{
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(iTable.Open());

		EXPECT_NO_THROW(iTable.Select(L""));

		iTable.SelectClose();
	}


	TEST_P(TableTest, SelectNext)
	{
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", AF_READ);
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


	TEST_P(TableTest, SelectClose)
	{
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(iTable.Open());
		
		// Do something that opens a transaction
		iTable.Select(L"");
		EXPECT_NO_THROW(iTable.SelectClose());
		// We should be closed now
		EXPECT_FALSE(iTable.IsSelectOpen());
	}


	TEST_P(TableTest, SelectHasMASEnabled)
	{
		// Test if we can have multiple active statements.
		// For MS SQL Server, this is disabled by default and must be activated manually
		// It must be activated before opening the database

		// Use our already open database to determine the server type
		Database db(&m_env);
		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			// It is disabled by default on MS and must be enabled before opening the db:
			EXPECT_FALSE(m_db.IsMarsEnabled());
			ASSERT_NO_THROW(db.SetMarsEnabled(true));
		}

		if (m_odbcInfo.HasConnectionString())
		{
			db.Open(m_odbcInfo.m_connectionString);
		}
		else
		{
			db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password);
		}
		EXPECT_TRUE(db.IsMarsEnabled());

		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);

		// Clear Table
		test::ClearIntTypesTmpTable(db, m_odbcInfo.m_namesCase);

		// Insert some values
		test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, db, 1, 10, 100, 1000);
		test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, db, 2, 20, 200, 2000);
		test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, db, 3, 30, 300, 3000);

		// Open one statement, select first record, but more records would be available
		Table iTable(&db, intTypesTableName, L"", L"", L"", AF_READ);
		iTable.Open();
		ASSERT_NO_THROW(iTable.Select());
		ASSERT_TRUE(iTable.SelectNext());

		// Open another statement, where we also select some record. If MARS is not enabled, we would have 
		// to close the other result-set first: MS Sql Server fails with a "busy for other result" during Open(),
		// when it queries the Database to find the table.
		// While Access just fails on the SelectNext()
		Table iTable2(&db, intTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(iTable2.Open());
		EXPECT_NO_THROW(iTable2.Select());
		EXPECT_TRUE(iTable2.SelectNext());
	}


	// Count
	// -----
	TEST_P(TableTest, Count)
	{
		MFloatTypesTable table(&m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(table.Open());

		SQLUBIGINT all;
		EXPECT_NO_THROW(all = table.Count(L""));
		EXPECT_EQ(6, all);

		// \todo: Check some really big table, with billions of rows
		SQLUBIGINT some;
		std::wstring whereStmt = L"tdouble > 0";
		if (m_db.GetDbms() == DatabaseProduct::DB2)
		{
			whereStmt = L"TDOUBLE > 0";
		}
		EXPECT_NO_THROW(some = table.Count(whereStmt));
		EXPECT_EQ(1, some);
		whereStmt = L"tdouble > 0 OR tfloat > 0";
		if (m_db.GetDbms() == DatabaseProduct::DB2)
		{
			whereStmt = L"TDOUBLE > 0 OR TFLOAT > 0";
		}
		EXPECT_NO_THROW(some = table.Count(whereStmt));
		EXPECT_EQ(2, some);

		// we should fail with an exception if we query using a non-sense where-stmt
		EXPECT_THROW(table.Count(L"a query that is invalid for sure"), Exception);
	}


	// GetValues
	// ---------
	TEST_P(TableTest, SelectAutoIntValues)
	{
		namespace tt = test;
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(iTable.Open());

		std::wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);

		// The first column has a smallint set, we can read that as any int value -32768
		iTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 1, -32768, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL));
		iTable.SelectClose();

		iTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 2, 32767, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL));
		iTable.SelectClose();

		// The 2nd column has a int set, we can read that as int or bigint value -2147483648
		iTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 3, test::ValueIndicator::IS_NULL, (-2147483647 - 1), test::ValueIndicator::IS_NULL));
		iTable.SelectClose();

		iTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 4, test::ValueIndicator::IS_NULL, 2147483647, test::ValueIndicator::IS_NULL));
		iTable.SelectClose();

		// The 3rd column has a bigint set, we can read that as bigint value -9223372036854775808
		iTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 5, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL, (-9223372036854775807 - 1)));
		iTable.SelectClose();

		iTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 6, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL, 9223372036854775807));
		iTable.SelectClose();
	}


	TEST_P(TableTest, SelectManualIntValues)
	{
		MIntTypesTable iTable(&m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(iTable.Open());

		// Just check that the buffers are correct
		// do not check using the IsIntRecordEqual: Here we bind the smallint column to a SQLSMALLINT with Access, 
		// this works as the driver can convert it. But in auto we bind as the reported SQLINTEGER type, and our
		// test-helpers have some problems as they assume its reported as SQLINTEGER..
		std::wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		iTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(-32768, iTable.m_smallInt);

		iTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(32767, iTable.m_smallInt);

		iTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(INT_MIN, iTable.m_int);

		iTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(2147483647, iTable.m_int);

		// note that Access has no BigInt and those values are set to NULL
		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
		{
			iTable.Select((boost::wformat(L"%s = 5") % idName).str());
			EXPECT_TRUE(iTable.SelectNext());
			EXPECT_EQ((-9223372036854775807 - 1), iTable.m_bigInt);

			iTable.Select((boost::wformat(L"%s = 6") % idName).str());
			EXPECT_TRUE(iTable.SelectNext());
			EXPECT_EQ(9223372036854775807, iTable.m_bigInt);
		}
	}


	TEST_P(TableTest, SelectWCharIntValues)
	{
		// And some tables with auto-columns
		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", AF_READ);
		iTable.SetAutoBindingMode(AutoBindingMode::BIND_ALL_AS_WCHAR);
		ASSERT_NO_THROW(iTable.Open());

		std::wstring id, smallInt, i, bigInt;
		std::wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		iTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(smallInt = iTable.GetWString(1));
		EXPECT_EQ(L"-32768", smallInt);

		iTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(smallInt = iTable.GetWString(1));
		EXPECT_EQ(L"32767", smallInt);

		iTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(i = iTable.GetWString(2));
		EXPECT_EQ(L"-2147483648", i);

		iTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(i = iTable.GetWString(2));
		EXPECT_EQ(L"2147483647", i);

		// No bigints on access
		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
		{
			iTable.Select((boost::wformat(L"%s = 5") % idName).str());
			EXPECT_TRUE(iTable.SelectNext());
			EXPECT_NO_THROW(bigInt = iTable.GetWString(3));
			EXPECT_EQ(L"-9223372036854775808", bigInt);

			iTable.Select((boost::wformat(L"%s = 6") % idName).str());
			EXPECT_TRUE(iTable.SelectNext());
			EXPECT_NO_THROW(bigInt = iTable.GetWString(3));
			EXPECT_EQ(L"9223372036854775807", bigInt);
		}
	}


	TEST_P(TableTest, SelectAutoFloatValues)
	{
		wstring floatTypesTableName = test::GetTableName(test::TableId::FLOATTYPES, m_odbcInfo.m_namesCase);
		Table fTable(&m_db, floatTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(fTable.Open());

		SQLDOUBLE val;
		wstring str;
		std::wstring idName = test::GetIdColumnName(test::TableId::FLOATTYPES, m_odbcInfo.m_namesCase);
		fTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(val = fTable.GetDouble(2));
		EXPECT_EQ(0.0, val);
		// Read as str
		EXPECT_NO_THROW(str = fTable.GetWString(2));
		EXPECT_EQ(L"0", str);

		fTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(val = fTable.GetDouble(2));
		EXPECT_EQ(3.141, val);
		// Read as str
		EXPECT_NO_THROW(str = fTable.GetWString(2));
		EXPECT_EQ(L"3.141", str);

		fTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(val = fTable.GetDouble(2));
		EXPECT_EQ(-3.141, val);
		// Read as str
		EXPECT_NO_THROW(str = fTable.GetWString(2));
		EXPECT_EQ(L"-3.141", str);

		fTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(val = fTable.GetDouble(1));
		EXPECT_EQ(0.0, val);
		// Read as str
		EXPECT_NO_THROW(str = fTable.GetWString(1));
		EXPECT_EQ(L"0", str);

		fTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(val = fTable.GetDouble(1));
		EXPECT_EQ(3.141592, val);
		// Read as str
		EXPECT_NO_THROW(str = fTable.GetWString(1));
		EXPECT_EQ(L"3.1415920000000002", str);

		fTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(val = fTable.GetDouble(1));
		EXPECT_EQ(-3.141592, val);
		// Read as str
		EXPECT_NO_THROW(str = fTable.GetWString(1));
		EXPECT_EQ(L"-3.1415920000000002", str);
	}


	TEST_P(TableTest, SelectManualFloatValues)
	{
		MFloatTypesTable fTable(&m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(fTable.Open());

		std::wstring idName = test::GetIdColumnName(test::TableId::FLOATTYPES, m_odbcInfo.m_namesCase);
		fTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(0.0, fTable.m_float);

		fTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(3.141, fTable.m_float);

		fTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(-3.141, fTable.m_float);

		fTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(0.0, fTable.m_double);

		fTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(3.141592, fTable.m_double);

		fTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(-3.141592, fTable.m_double);
	}


	TEST_P(TableTest, SelectWCharFloatValues)
	{
		wstring floatTypesTableName = test::GetTableName(test::TableId::FLOATTYPES, m_odbcInfo.m_namesCase);
		Table fTable(&m_db, floatTypesTableName, L"", L"", L"", AF_READ);
		fTable.SetAutoBindingMode(AutoBindingMode::BIND_ALL_AS_WCHAR);
		ASSERT_NO_THROW(fTable.Open());

		wstring sVal;
		wstring idName = test::GetIdColumnName(test::TableId::FLOATTYPES, m_odbcInfo.m_namesCase);
		fTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(sVal = fTable.GetWString(2));
		//EXPECT_EQ(0.0, sVal);
		
		// TODO: Find a way to compare without having to diff for every db-type

		fTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(sVal = fTable.GetWString(2));

		fTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(sVal = fTable.GetWString(2));

		fTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(sVal = fTable.GetWString(1));
		
		fTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(sVal = fTable.GetWString(1));

		fTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(sVal = fTable.GetWString(1));
	}


	TEST_P(TableTest, SelectAutoWCharValues)
	{
		std::wstring charTypesTableName = test::GetTableName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
		Table charTypesAutoTable(&m_db, charTypesTableName, L"", L"", L"", AF_READ);
		charTypesAutoTable.SetAutoBindingMode(AutoBindingMode::BIND_CHAR_AS_WCHAR);
		ASSERT_NO_THROW(charTypesAutoTable.Open());
		// We want to trim on the right side for DB2 and sql server
		charTypesAutoTable.SetCharTrimRight(true);

		std::wstring str;

		// We expect 6 Records
		std::wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
		charTypesAutoTable.Select((boost::wformat(L"%d = 1") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(1));
		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);
		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 2") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(2));
		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 3") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(1));
		EXPECT_EQ(std::wstring(L"הצüאיט"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 4") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(2));
		EXPECT_EQ(std::wstring(L"הצüאיט"), str);

		charTypesAutoTable.Select((boost::wformat(L"%d = 5") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(3));
		EXPECT_EQ(std::wstring(L"abc"), str);
		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(4));
		EXPECT_EQ(std::wstring(L"abc"), str);

		charTypesAutoTable.Select((boost::wformat(L"%d = 6") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(3));
		EXPECT_EQ(std::wstring(L"abcde12345"), str);
		EXPECT_NO_THROW(str = charTypesAutoTable.GetWString(4));
		EXPECT_EQ(std::wstring(L"abcde12345"), str);

		charTypesAutoTable.SelectClose();
	}


	TEST_P(TableTest, SelectManualWCharValues)
	{
		MWCharTypesTable wTable(&m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(wTable.Open());

		// Just test the values in the buffer - trimming has no effect here
		using namespace boost::algorithm;
		std::wstring str;
		std::wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
		wTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*) &wTable.m_varchar);
		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
		EXPECT_FALSE(wTable.SelectNext());
		wTable.SelectClose();

		wTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*)&wTable.m_char);
		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
		EXPECT_FALSE(wTable.SelectNext());
		wTable.SelectClose();

		wTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*)&wTable.m_varchar);
		EXPECT_EQ(std::wstring(L"הצüאיט"), trim_right_copy(str));
		EXPECT_FALSE(wTable.SelectNext());
		wTable.SelectClose();

		wTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*)&wTable.m_char);
		EXPECT_EQ(std::wstring(L"הצüאיט"), trim_right_copy(str));
		EXPECT_FALSE(wTable.SelectNext());
		wTable.SelectClose();

		wTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*)&wTable.m_char_10);
		EXPECT_EQ(std::wstring(L"abc"), trim_right_copy(str));
		str.assign((wchar_t*)&wTable.m_varchar_10);
		EXPECT_EQ(std::wstring(L"abc"), trim_right_copy(str));
		wTable.SelectClose();

		wTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*)&wTable.m_char_10);
		EXPECT_EQ(std::wstring(L"abcde12345"), trim_right_copy(str));
		str.assign((wchar_t*)&wTable.m_varchar_10);
		EXPECT_EQ(std::wstring(L"abcde12345"), trim_right_copy(str));
		wTable.SelectClose();

	}

	
	TEST_P(TableTest, SelectAutoCharValues)
	{
		std::wstring charTypesTableName = test::GetTableName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
		Table charTypesAutoTable(&m_db, charTypesTableName, L"", L"", L"", AF_READ);
		charTypesAutoTable.SetAutoBindingMode(AutoBindingMode::BIND_WCHAR_AS_CHAR);
		ASSERT_NO_THROW(charTypesAutoTable.Open());
		// We want to trim on the right side for DB2 and also sql server
		charTypesAutoTable.SetCharTrimRight(true);

		std::string str;

		std::wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
		charTypesAutoTable.Select((boost::wformat(L"%d = 1") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(1));
		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);
		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 2") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(2));
		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);
		
		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 3") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(1));
		EXPECT_EQ(std::string("הצüאיט"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 4") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(2));
		EXPECT_EQ(std::string("הצüאיט"), str);

		charTypesAutoTable.Select((boost::wformat(L"%d = 5") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(3));
		EXPECT_EQ(std::string("abc"), str);
		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(4));
		EXPECT_EQ(std::string("abc"), str);

		charTypesAutoTable.Select((boost::wformat(L"%d = 6") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(3));
		EXPECT_EQ(std::string("abcde12345"), str);
		EXPECT_NO_THROW(str = charTypesAutoTable.GetString(4));
		EXPECT_EQ(std::string("abcde12345"), str);

		charTypesAutoTable.SelectClose();
	}


	TEST_P(TableTest, SelectManualCharValues)
	{
		MCharTypesTable cTable(&m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(cTable.Open());

		// Just test the values in the buffer - trimming has no effect here
		using namespace boost::algorithm;
		std::string str;
		std::wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase);
		cTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_varchar);
		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
		EXPECT_FALSE(cTable.SelectNext());
		cTable.SelectClose();

		cTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_char);
		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
		EXPECT_FALSE(cTable.SelectNext());
		cTable.SelectClose();

		cTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_varchar);
		EXPECT_EQ(std::string("הצüאיט"), trim_right_copy(str));
		EXPECT_FALSE(cTable.SelectNext());
		cTable.SelectClose();

		cTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_char);
		EXPECT_EQ(std::string("הצüאיט"), trim_right_copy(str));
		EXPECT_FALSE(cTable.SelectNext());
		cTable.SelectClose();

		cTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_char_10);
		EXPECT_EQ(std::string("abc"), trim_right_copy(str));
		str.assign((char*)&cTable.m_varchar_10);
		EXPECT_EQ(std::string("abc"), trim_right_copy(str));
		cTable.SelectClose();

		cTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_char_10);
		EXPECT_EQ(std::string("abcde12345"), trim_right_copy(str));
		str.assign((char*)&cTable.m_varchar_10);
		EXPECT_EQ(std::string("abcde12345"), trim_right_copy(str));
		cTable.SelectClose();
	}
	

	TEST_P(TableTest, SelectAutoDateValues)
	{
		// Note how to read fractions:
		// [b]   The value of the fraction field is the number of billionths of a second and ranges from 0 through 999,999,999 (1 less than 1 billion). 
		// For example, the value of the fraction field for a half-second is 500,000,000, for a thousandth of a second (one millisecond) is 1,000,000, 
		// for a millionth of a second (one microsecond) is 1,000, and for a billionth of a second (one nanosecond) is 1.

		std::wstring dateTypesTableName = test::GetTableName(test::TableId::DATETYPES, m_odbcInfo.m_namesCase);
		Table dTable(&m_db, dateTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(dTable.Open());

		using namespace boost::algorithm;
		std::wstring str;
		SQL_DATE_STRUCT date;
		SQL_TIME_STRUCT time;
		SQL_TIMESTAMP_STRUCT timestamp;
		std::wstring idName = test::GetIdColumnName(test::TableId::DATETYPES, m_odbcInfo.m_namesCase);
		dTable.Select((boost::wformat(L"%s = 1") % idName).str());
		{
			// Microsoft will info here (which we report as warning) that the time field has been truncated (as we do not use the fancy TIME2 struct)
			LogLevelError llE;
			EXPECT_TRUE(dTable.SelectNext());
		}
		EXPECT_NO_THROW(date = dTable.GetDate(1));
		EXPECT_NO_THROW(time = dTable.GetTime(2));
		EXPECT_NO_THROW(timestamp = dTable.GetTimeStamp(3));

		EXPECT_EQ(26, date.day);
		EXPECT_EQ(1, date.month);
		EXPECT_EQ(1983, date.year);
		
		EXPECT_EQ(13, time.hour);
		EXPECT_EQ(55, time.minute);
		EXPECT_EQ(56, time.second);

		EXPECT_EQ(26, timestamp.day);
		EXPECT_EQ(1, timestamp.month);
		EXPECT_EQ(1983, timestamp.year);
		EXPECT_EQ(13, timestamp.hour);
		EXPECT_EQ(55, timestamp.minute);
		EXPECT_EQ(56, timestamp.second);
		
		// Test some digits stuff
		dTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(dTable.SelectNext());
		EXPECT_NO_THROW(timestamp = dTable.GetTimeStamp(3));
		// In IBM DB2 we have 6 digits for the fractions of a timestamp 123456 turns into 123'456'000
		if (m_db.GetDbms() == DatabaseProduct::DB2)
		{
			EXPECT_EQ(123456000, timestamp.fraction);
		}
		else if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			// ms has 3 digits 123 turns into 123'000'000
			EXPECT_EQ(123000000, timestamp.fraction);
		}
		else
		{
			EXPECT_EQ(0, timestamp.fraction);
		}

		dTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(dTable.SelectNext());
		EXPECT_NO_THROW(timestamp = dTable.GetTimeStamp(3));
		if ( ! (m_db.GetDbms() == DatabaseProduct::MY_SQL || m_db.GetDbms() == DatabaseProduct::ACCESS))
		{
			EXPECT_EQ(10000000, timestamp.fraction);
		}


		// MS SQL Server has a new SQL_TIME2_STRUCT if ODBC version is >= 3.8
#if HAVE_MSODBCSQL_H
		Environment env38(OdbcVersion::V_3_8);
		EXPECT_TRUE(env38.HasEnvironmentHandle());
		Database db38(&env38);
		EXPECT_TRUE(db38.HasConnectionHandle());
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			// My SQL does not support 3.8, the database will warn about a version-mismatch and fall back to 3.0. we know about that.
			// \todo: Open a ticket
			LogLevelError llE;
			if (m_odbcInfo.HasConnectionString())
			{
				EXPECT_NO_THROW(db38.Open(m_odbcInfo.m_connectionString));
			}
			else
			{
				EXPECT_NO_THROW(db38.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
			}
		}
		else
		{
			if (m_odbcInfo.HasConnectionString())
			{
				EXPECT_NO_THROW(db38.Open(m_odbcInfo.m_connectionString));
			}
			else
			{
				EXPECT_NO_THROW(db38.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
			}
		}
		Table dTable38(&db38, dateTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(dTable38.Open());
		dTable38.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(dTable38.SelectNext());
		EXPECT_NO_THROW(time = dTable38.GetTime(2));
		EXPECT_EQ(13, time.hour);
		EXPECT_EQ(55, time.minute);
		EXPECT_EQ(56, time.second);

		// We should be able to read the value as TIME2 struct
		SQL_SS_TIME2_STRUCT t2;
		EXPECT_NO_THROW(t2 = dTable38.GetTime2(2));
		EXPECT_EQ(13, t2.hour);
		EXPECT_EQ(55, t2.minute);
		EXPECT_EQ(56, t2.second);
		if (db38.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			// MS should also have fractions, configurable, here set to 7: 1234567 -> 123'456'700
			EXPECT_TRUE(db38.GetMaxSupportedOdbcVersion() >= OdbcVersion::V_3_8);
			EXPECT_EQ(123456700, t2.fraction);
		}
		else
		{
			// For the other DBs we expect the fraction part to be 0, as we did not read it
			// from a TIME2 struct, but from a normal TIME struct, without any fractions
			EXPECT_EQ(0, t2.fraction);
		}
#endif
	}


	TEST_P(TableTest, SelectManualDateValues)
	{
		MDateTypesTable cTable(&m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(cTable.Open());

		// Just test the values in the buffer - trimming has no effect here
		using namespace boost::algorithm;
		std::string str;
		std::wstring idName = test::GetIdColumnName(test::TableId::DATETYPES, m_odbcInfo.m_namesCase);
		cTable.Select((boost::wformat(L"%s = 1") % idName).str());
		{
			// Microsoft will info here (which we report as warning) that the time field has been truncated (as we do not use the fancy TIME2 struct)
			LogLevelError llE;
			EXPECT_TRUE(cTable.SelectNext());
		}
		EXPECT_EQ(26, cTable.m_date.day);
		EXPECT_EQ(1, cTable.m_date.month);
		EXPECT_EQ(1983, cTable.m_date.year);

		EXPECT_EQ(13, cTable.m_time.hour);
		EXPECT_EQ(55, cTable.m_time.minute);
		EXPECT_EQ(56, cTable.m_time.second);

		EXPECT_EQ(26, cTable.m_timestamp.day);
		EXPECT_EQ(1, cTable.m_timestamp.month);
		EXPECT_EQ(1983, cTable.m_timestamp.year);
		EXPECT_EQ(13, cTable.m_timestamp.hour);
		EXPECT_EQ(55, cTable.m_timestamp.minute);
		EXPECT_EQ(56, cTable.m_timestamp.second);
	}


	TEST_P(TableTest, SelectWCharDateValues)
	{
		std::wstring dateTypesTableName = test::GetTableName(test::TableId::DATETYPES, m_odbcInfo.m_namesCase);
		Table dTable(&m_db, dateTypesTableName, L"", L"", L"", AF_READ);
		dTable.SetAutoBindingMode(AutoBindingMode::BIND_ALL_AS_WCHAR);
		ASSERT_NO_THROW(dTable.Open());

		wstring sDate, sTime, sTimestamp;
		wstring idName = test::GetIdColumnName(test::TableId::DATETYPES, m_odbcInfo.m_namesCase);
		dTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(dTable.SelectNext());
		EXPECT_NO_THROW(sDate = dTable.GetWString(1));
		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
		{
			EXPECT_EQ(L"1983-01-26", sDate);
		}

		EXPECT_NO_THROW(sTime = dTable.GetWString(2));
		EXPECT_NO_THROW(sTimestamp = dTable.GetWString(3));

		if (m_db.GetDbms() == DatabaseProduct::DB2)
		{
			EXPECT_EQ(L"13:55:56", sTime);
			EXPECT_EQ(L"1983-01-26 13:55:56.000000", sTimestamp);
		}
		else if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			EXPECT_EQ(L"13:55:56.1234567", sTime);
			EXPECT_EQ(L"1983-01-26 13:55:56.000", sTimestamp);
		}
		else
		{
			if (m_db.GetDbms() != DatabaseProduct::ACCESS)
			{
				EXPECT_EQ(L"13:55:56", sTime);
			}
			EXPECT_EQ(L"1983-01-26 13:55:56", sTimestamp);
		}

	}

	
	TEST_P(TableTest, SelectAutoBlobValues)
	{
		std::wstring blobTypesTableName = test::GetTableName(test::TableId::BLOBTYPES, m_odbcInfo.m_namesCase);
		Table bTable(&m_db, blobTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(bTable.Open());

		wstring idName = test::GetIdColumnName(test::TableId::BLOBTYPES, m_odbcInfo.m_namesCase);

		SQLCHAR empty[] = { 0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

		SQLCHAR ff[] = { 255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255
		};

		SQLCHAR abc[] = { 0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67
		};

		SQLCHAR abc_ff[] = { 0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67,
			0xff, 0xff, 0xff, 0xff
		};

		const SQLCHAR* pBlob = NULL;
		SQLLEN size, length = 0;

		// Fixed size bins
		bTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_NO_THROW(pBlob = bTable.GetBinaryValue(1, size, length));
		EXPECT_EQ(0, memcmp(empty, pBlob, length));
		EXPECT_EQ(16, size);
		EXPECT_EQ(sizeof(empty), length);
		EXPECT_TRUE(bTable.IsColumnNull(2));

		bTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_NO_THROW(pBlob = bTable.GetBinaryValue(1, size, length));
		EXPECT_EQ(0, memcmp(ff, pBlob, length));
		EXPECT_EQ(16, size);
		EXPECT_EQ(sizeof(ff), length);
		EXPECT_TRUE(bTable.IsColumnNull(2));

		bTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_NO_THROW(pBlob = bTable.GetBinaryValue(1, size, length));
		EXPECT_EQ(0, memcmp(abc, pBlob, length));
		EXPECT_EQ(16, size);
		EXPECT_EQ(sizeof(abc), length);
		EXPECT_TRUE(bTable.IsColumnNull(2));

		// Read Varbins
		bTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_NO_THROW(pBlob = bTable.GetBinaryValue(2, size, length));
		EXPECT_EQ(0, memcmp(abc, pBlob, length));
		EXPECT_EQ(20, size);
		EXPECT_EQ(sizeof(abc), length);
		EXPECT_TRUE(bTable.IsColumnNull(1));

		bTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_NO_THROW(pBlob = bTable.GetBinaryValue(2, size, length));
		EXPECT_EQ(0, memcmp(abc_ff, pBlob, length));
		EXPECT_EQ(20, size);
		EXPECT_EQ(sizeof(abc_ff), length);
		EXPECT_TRUE(bTable.IsColumnNull(1));
	}


	TEST_P(TableTest, SelectManualBlobValues)
	{
		MBlobTypesTable bTable(&m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(bTable.Open());

		wstring idName = test::GetIdColumnName(test::TableId::BLOBTYPES, m_odbcInfo.m_namesCase);

		SQLCHAR empty[] = { 0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

		SQLCHAR ff[] = { 255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255
		};

		SQLCHAR abc[] = { 0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67
		};

		SQLCHAR abc_ff[] = { 0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67,
			0xff, 0xff, 0xff, 0xff
		};

		bTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(empty, bTable.m_blob, sizeof(bTable.m_blob)));

		bTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(ff, bTable.m_blob, sizeof(bTable.m_blob)));

		bTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(abc, bTable.m_blob, sizeof(bTable.m_blob)));

		bTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(abc, bTable.m_varblob_20, sizeof(abc)));

		bTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(abc_ff, bTable.m_varblob_20, sizeof(abc_ff)));

	}


	TEST_P(TableTest, IsNullAutoNumericValue)
	{
		std::wstring numericTypesTableName = test::GetTableName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
		Table nTable(&m_db, numericTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(nTable.Open());

		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
		const ColumnBuffer* pColId = nTable.GetColumnBuffer(0);
		const ColumnBuffer* pColBuff18_00 = nTable.GetColumnBuffer(1);
		const ColumnBuffer* pColBuff18_10 = nTable.GetColumnBuffer(2);
		const ColumnBuffer* pColBuff5_3 = nTable.GetColumnBuffer(3);
		nTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		nTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_FALSE(pColBuff5_3->IsNull());

		nTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		nTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_TRUE(pColBuff18_00->IsNull());
		EXPECT_FALSE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		nTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_TRUE(pColBuff18_00->IsNull());
		EXPECT_FALSE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		nTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_TRUE(pColBuff18_00->IsNull());
		EXPECT_FALSE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());
	}
	
	
	TEST_P(TableTest, SelectAutoNumericValue)
	{


		// \note: There is a special Tests for NULL values, its complicated enough.
		std::wstring numericTypesTableName = test::GetTableName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
		Table nTable(&m_db, numericTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(nTable.Open());
		
		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
		SQL_NUMERIC_STRUCT numStr;
		const ColumnBuffer* pColBuff18_00 = nTable.GetColumnBuffer(1);
		const ColumnBuffer* pColBuff18_10 = nTable.GetColumnBuffer(2);
		const ColumnBuffer* pColBuff5_3 = nTable.GetColumnBuffer(3);
		SQLBIGINT ex;
		SQLBIGINT* p;

		nTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(numStr = nTable.GetNumeric(1));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(0, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 0;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(numStr = nTable.GetNumeric(1));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(0, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);
		// Col 3 has a value here too.
		EXPECT_NO_THROW(numStr = nTable.GetNumeric(3));
		EXPECT_EQ(5, numStr.precision);
		EXPECT_EQ(3, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 12345;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(numStr = nTable.GetNumeric(1));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(0, numStr.scale);
		EXPECT_EQ(0, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(numStr = nTable.GetNumeric(2));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(10, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 0;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(numStr = nTable.GetNumeric(2));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(10, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(numStr = nTable.GetNumeric(2));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(10, numStr.scale);
		EXPECT_EQ(0, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);
	}


	TEST_P(TableTest, SelectManualNumericValue)
	{


		MNumericTypesTable nTable(&m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(nTable.Open());

		SQLBIGINT ex;
		SQLBIGINT* p;
		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
		nTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_EQ(18, nTable.m_decimal_18_0.precision);
		EXPECT_EQ(0, nTable.m_decimal_18_0.scale);
		EXPECT_EQ(1, nTable.m_decimal_18_0.sign);
		ex = 0;
		p = (SQLBIGINT*)&nTable.m_decimal_18_0.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_EQ(18, nTable.m_decimal_18_0.precision);
		EXPECT_EQ(0, nTable.m_decimal_18_0.scale);
		EXPECT_EQ(1, nTable.m_decimal_18_0.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&nTable.m_decimal_18_0.val;
		EXPECT_EQ(ex, *p);
		EXPECT_EQ(5, nTable.m_decimal_5_3.precision);
		EXPECT_EQ(3, nTable.m_decimal_5_3.scale);
		EXPECT_EQ(1, nTable.m_decimal_5_3.sign);
		ex = 12345;
		p = (SQLBIGINT*)&nTable.m_decimal_5_3.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_EQ(18, nTable.m_decimal_18_0.precision);
		EXPECT_EQ(0, nTable.m_decimal_18_0.scale);
		EXPECT_EQ(0, nTable.m_decimal_18_0.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&nTable.m_decimal_18_0.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_EQ(18, nTable.m_decimal_18_10.precision);
		EXPECT_EQ(10, nTable.m_decimal_18_10.scale);
		EXPECT_EQ(1, nTable.m_decimal_18_10.sign);
		ex = 0;
		p = (SQLBIGINT*)&nTable.m_decimal_18_10.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_EQ(18, nTable.m_decimal_18_10.precision);
		EXPECT_EQ(10, nTable.m_decimal_18_10.scale);
		EXPECT_EQ(1, nTable.m_decimal_18_10.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&nTable.m_decimal_18_10.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_EQ(18, nTable.m_decimal_18_10.precision);
		EXPECT_EQ(10, nTable.m_decimal_18_10.scale);
		EXPECT_EQ(0, nTable.m_decimal_18_10.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&nTable.m_decimal_18_10.val;
		EXPECT_EQ(ex, *p);
	}


	TEST_P(TableTest, IsNullManualNumericValue)
	{


		std::wstring numericTypesTableName = test::GetTableName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
		Table nTable(&m_db, numericTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(nTable.Open());

		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
		const ColumnBuffer* pColId = nTable.GetColumnBuffer(0);
		const ColumnBuffer* pColBuff18_00 = nTable.GetColumnBuffer(1);
		const ColumnBuffer* pColBuff18_10 = nTable.GetColumnBuffer(2);
		const ColumnBuffer* pColBuff5_3 = nTable.GetColumnBuffer(3);
		nTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		nTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_FALSE(pColBuff5_3->IsNull());

		nTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		nTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_TRUE(pColBuff18_00->IsNull());
		EXPECT_FALSE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		nTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_TRUE(pColBuff18_00->IsNull());
		EXPECT_FALSE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		nTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_TRUE(pColBuff18_00->IsNull());
		EXPECT_FALSE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());
	}


	// Delete rows: We do not test that for the different data-types, its just deleting a row.
	// -----------
	TEST_P(TableTest, DeleteWhere)
	{
		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", AF_SELECT | AF_DELETE_WHERE | AF_INSERT);
		ASSERT_NO_THROW(iTable.Open());

		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Note: We could also do this in one transaction.
		// \todo: Write a separate transaction test about this, to check transaction-visibility
		// Try to delete eventually available leftovers, ignore if none exists
		test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase);

		// Now lets insert some data:
		test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, m_db, 103, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL);

		// Now we must have something to delete
		EXPECT_NO_THROW(iTable.Delete(sqlstmt, true));
		EXPECT_NO_THROW(m_db.CommitTrans());

		// And fetching shall return no results at all
		iTable.Select();
		EXPECT_FALSE(iTable.SelectNext());
	}


	TEST_P(TableTest, DeleteWhereShouldReturnSQL_NO_DATA)
	{
		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", AF_SELECT | AF_DELETE_WHERE);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		ASSERT_NO_THROW(iTable.Open());

		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #77");
		}

		// Remove everything, ignoring if there was any data:
		test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase);

		// We can be sure now that nothing exists, and we will fail if we try to delete
		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		EXPECT_THROW(iTable.Delete(sqlstmt, true), SqlResultException);
		EXPECT_NO_THROW(m_db.CommitTrans());

		// And fetching shall return no results at all
		iTable.Select();
		EXPECT_FALSE(iTable.SelectNext());
	}


	TEST_P(TableTest, Delete)
	{
		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", AF_SELECT | AF_DELETE_PK);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		ASSERT_NO_THROW(iTable.Open());

		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase);

		// Insert a row that we want to delete
		test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, m_db, 99, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL);

		// Now lets delete that row by pointing the primary key column to it
		iTable.SetColumnValue(0, (SQLINTEGER) 99);
		EXPECT_NO_THROW(iTable.Delete());

		// And fetching shall return no results at all
		iTable.Select();
		EXPECT_FALSE(iTable.SelectNext());
	}


	// Insert rows
	// -----------
	TEST_P(TableTest, InsertIntTypes)
	{
		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", AF_SELECT | AF_INSERT);
		ASSERT_NO_THROW(iTable.Open());
		ColumnBuffer* pId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt = iTable.GetColumnBuffer(1);
		ColumnBuffer* pInt = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBigInt = iTable.GetColumnBuffer(3);

		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase));

		// Set some silly values to insert
		*pId = (SQLINTEGER)101;
		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
		{
			*pSmallInt = (SQLSMALLINT)102;
		}
		else
		{
			*pSmallInt = (SQLINTEGER)102;
		}
		*pInt = (SQLINTEGER)103;
		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
		{
			*pBigInt = (SQLBIGINT)104;
		}

		EXPECT_NO_THROW(iTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Open another table and read the values from there
		Table iTable2(&m_db, intTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(iTable2.Open());

		iTable2.Select();
		EXPECT_TRUE(iTable2.SelectNext());
		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 101, 102, 103, 104));
	}


	// just do only one test to insert the manual types, its all the same, except that opening is a little bit different
	TEST_P(TableTest, InsertManualIntTypes)
	{
		// Open an existing table without checking for privileges or existence
		Table iTable(&m_db, 4, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT | AF_INSERT);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		SQLINTEGER biAccess = 0;
		int type = SQL_C_SLONG;
		iTable.SetColumn(0, test::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT);
		iTable.SetColumn(1, test::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_INSERT);
		iTable.SetColumn(2, test::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT | CF_INSERT);
		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
		{
			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
		}
		else
		{
			// access has no bigint, col is int in test-db
			iTable.SetColumn(3, test::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_INTEGER, &biAccess, SQL_C_SLONG, sizeof(bi), CF_SELECT | CF_INSERT | CF_NULLABLE);
		}

		iTable.Open();
		
		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase));

		// Set some silly values to insert
		// note: no need to set to not-null, the buffers are created with null set to false
		id = 101;
		si = 102;
		i = 103;
		bi = 104;
		biAccess = 104;

		EXPECT_NO_THROW(iTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Open another table and read the values from there
		Table iTable2(&m_db, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_READ);
		iTable2.Open();
		iTable2.Select();
		ASSERT_TRUE(iTable2.SelectNext());

		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable2, 101, 102, 103, 104));
	}


	TEST_P(TableTest, InsertDateTypes)
	{
		std::wstring dateTypesTableName = test::GetTableName(test::TableId::DATETYPES_TMP, m_odbcInfo.m_namesCase);
		Table dTable(&m_db, dateTypesTableName, L"", L"", L"", AF_SELECT | AF_INSERT);
		ASSERT_NO_THROW(dTable.Open());
		//ColumnBuffer* pId = dTable.GetColumnBuffer(0);
		//ColumnBuffer* pDate = dTable.GetColumnBuffer(1);
		//ColumnBuffer* pTime = dTable.GetColumnBuffer(2);
		//ColumnBuffer* pTimestamp = dTable.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		test::ClearDateTypesTmpTable(m_db, m_odbcInfo.m_namesCase);

		// Set some silly values
		SQL_DATE_STRUCT date;
		date.day = 26;
		date.year = 1983;
		date.month = 1;
		
		SQL_TIME_STRUCT time;
		time.hour = 13;
		time.minute = 55;
		time.second = 03;
		
		SQL_TIMESTAMP_STRUCT timestamp;
		timestamp.day = 26;
		timestamp.year = 1983;
		timestamp.month = 1;
		timestamp.hour = 13;
		timestamp.minute = 55;
		timestamp.second = 03;
		if ( ! (m_db.GetDbms() == DatabaseProduct::MY_SQL || m_db.GetDbms() == DatabaseProduct::ACCESS))
		{
			timestamp.fraction = 123000000;
		}
		else
		{
			timestamp.fraction = 0;
		}

		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			// Access has only timestamps ??
			dTable.SetColumnValue(0, (SQLINTEGER)101);
			dTable.SetColumnValue(1, timestamp);
			dTable.SetColumnValue(2, timestamp);
			dTable.SetColumnValue(3, timestamp);
		}
		else
		{
			dTable.SetColumnValue(0, (SQLINTEGER)101);
			dTable.SetColumnValue(1, date);
			dTable.SetColumnValue(2, time);
			dTable.SetColumnValue(3, timestamp);
		}

		EXPECT_NO_THROW(dTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Open another table and read the values from there
		Table dTable2(&m_db, dateTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(dTable2.Open());

		dTable2.Select();
		EXPECT_TRUE(dTable2.SelectNext());

		SQL_DATE_STRUCT date2;
		SQL_TIME_STRUCT time2;
		SQL_TIMESTAMP_STRUCT timestamp2 = dTable.GetTimeStamp(3);
		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
		{
			date2 = dTable.GetDate(1);
			time2 = dTable.GetTime(2);
		}

		EXPECT_EQ(101, dTable2.GetInt(0));
		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
		{
			EXPECT_EQ(date.year, date2.year);
			EXPECT_EQ(date.month, date2.month);
			EXPECT_EQ(date.day, date2.day);
			EXPECT_EQ(time.hour, time2.hour);
			EXPECT_EQ(time.minute, time2.minute);
			EXPECT_EQ(time.second, time2.second);
		}

		EXPECT_EQ(timestamp.year, timestamp2.year);
		EXPECT_EQ(timestamp.month, timestamp2.month);
		EXPECT_EQ(timestamp.day, timestamp2.day);

		EXPECT_EQ(timestamp.hour, timestamp2.hour);
		EXPECT_EQ(timestamp.minute, timestamp2.minute);
		EXPECT_EQ(timestamp.second, timestamp2.second);
		EXPECT_EQ(timestamp.fraction, timestamp2.fraction);
	}


	TEST_P(TableTest, InsertFloatTypes)
	{
		std::wstring floatTypesTableName = test::GetTableName(test::TableId::FLOATTYPES_TMP, m_odbcInfo.m_namesCase);
		Table fTable(&m_db, floatTypesTableName, L"", L"", L"", AF_READ_WRITE);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			fTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		ASSERT_NO_THROW(fTable.Open());
		ColumnBuffer* pId = fTable.GetColumnBuffer(0);
		ColumnBuffer* pDouble = fTable.GetColumnBuffer(1);
		ColumnBuffer* pFloat = fTable.GetColumnBuffer(2);

		wstring idName = test::GetIdColumnName(test::TableId::FLOATTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(fTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert some values
		*pId = (SQLINTEGER)101;
		*pDouble = 3.14159265359;
		*pFloat = -3.14159;
		EXPECT_NO_THROW(fTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Open another table and read the values from there
		Table fTable2(&m_db, floatTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(fTable2.Open());
		ColumnBuffer* pId2 = fTable2.GetColumnBuffer(0);
		ColumnBuffer* pDouble2 = fTable2.GetColumnBuffer(1);
		ColumnBuffer* pFloat2 = fTable2.GetColumnBuffer(2);

		fTable2.Select();
		EXPECT_TRUE(fTable2.SelectNext());

		EXPECT_EQ(101, (SQLINTEGER)*pId2);
		EXPECT_EQ((SQLDOUBLE)*pDouble, (SQLDOUBLE)*pDouble2);
		EXPECT_EQ((SQLDOUBLE)*pFloat, (SQLDOUBLE)*pFloat2);
	}


	TEST_P(TableTest, InsertNumericTypes)
	{


		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table nTable(&m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(nTable.Open());
		ColumnBuffer* pId = nTable.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = nTable.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = nTable.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = nTable.GetColumnBuffer(3);

		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Select a valid record from the non-tmp table
		std::wstring numericTypesTableName = test::GetTableName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase);
		Table nnTable(&m_db, numericTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(nnTable.Open());
		SQL_NUMERIC_STRUCT numStr18_0, numStr18_10, numStr5_3;
		nnTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(nnTable.SelectNext());
		EXPECT_NO_THROW(numStr18_0 = nnTable.GetNumeric(1));
		EXPECT_NO_THROW(numStr5_3 = nnTable.GetNumeric(3));
		nnTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(nnTable.SelectNext());
		EXPECT_NO_THROW(numStr18_10 = nnTable.GetNumeric(2));
		nnTable.SelectClose();

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(nTable.Delete(sqlstmt, false));
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Insert the just read values
		*pId = (SQLINTEGER)101;
		*pNumeric_18_0 = numStr18_0;
		*pNumeric_18_10 = numStr18_10;
		*pNumeric_5_3 = numStr5_3;
		EXPECT_NO_THROW(nTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// And read them again from another table and compare them
		SQL_NUMERIC_STRUCT numStr18_0t, numStr18_10t, numStr5_3t;
		Table nntTable(&m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(nntTable.Open());
		sqlstmt = (boost::wformat(L"%s = %d") % idName % (SQLINTEGER)*pId).str();
		nntTable.Select(sqlstmt);
		EXPECT_TRUE(nntTable.SelectNext());
		EXPECT_NO_THROW(numStr18_0t = nntTable.GetNumeric(1));
		EXPECT_NO_THROW(numStr18_10t = nntTable.GetNumeric(2));
		EXPECT_NO_THROW(numStr5_3t = nntTable.GetNumeric(3));
		nntTable.SelectClose();
		EXPECT_EQ(0, memcmp(&numStr18_0, &numStr18_0t, sizeof(numStr18_0)));
		EXPECT_EQ(0, memcmp(&numStr18_10, &numStr18_10t, sizeof(numStr18_10)));
		EXPECT_EQ(0, memcmp(&numStr5_3, &numStr5_3t, sizeof(numStr5_3)));
	}


	TEST_P(TableTest, InsertNumericTypes_5_3)
	{


		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table t(&m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);

		ASSERT_NO_THROW(t.Open());
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") %idName).str();
		ASSERT_NO_THROW(t.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		SQL_NUMERIC_STRUCT numStr;
		ZeroMemory(&numStr, sizeof(numStr));
		numStr.val[0] = 57;
		numStr.val[1] = 48;
		numStr.precision = 5;
		numStr.scale = 3;
		numStr.sign = 1;

		pNumeric_18_0->SetNull();
		pNumeric_18_10->SetNull();
		*pNumeric_5_3 = numStr;
		*pId = (SQLINTEGER)300;
		EXPECT_NO_THROW(t.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());
	}


	TEST_P(TableTest, InsertNumericTypes_18_0)
	{


		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table t(&m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);

		ASSERT_NO_THROW(t.Open());
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		ASSERT_NO_THROW(t.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		SQL_NUMERIC_STRUCT numStr;
		ZeroMemory(&numStr, sizeof(numStr));
		numStr.val[0] = 78;
		numStr.val[1] = 243;
		numStr.val[2] = 48;
		numStr.val[3] = 166;
		numStr.val[4] = 75;
		numStr.val[5] = 155;
		numStr.val[6] = 182;
		numStr.val[7] = 1;

		numStr.precision = 18;
		numStr.scale = 0;
		numStr.sign = 1;

		*pNumeric_18_0 = numStr;
		pNumeric_18_10->SetNull();
		pNumeric_5_3->SetNull();
		*pId = (SQLINTEGER)300;
		EXPECT_NO_THROW(t.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());
	}


	TEST_P(TableTest, InsertNumericTypes_18_10)
	{


		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table t(&m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);

		ASSERT_NO_THROW(t.Open());
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		ASSERT_NO_THROW(t.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		SQL_NUMERIC_STRUCT numStr;
		ZeroMemory(&numStr, sizeof(numStr));
		numStr.val[0] = 78;
		numStr.val[1] = 243;
		numStr.val[2] = 48;
		numStr.val[3] = 166;
		numStr.val[4] = 75;
		numStr.val[5] = 155;
		numStr.val[6] = 182;
		numStr.val[7] = 1;

		numStr.precision = 18;
		numStr.scale = 10;
		numStr.sign = 1;

		pNumeric_18_0->SetNull();
		*pNumeric_18_10 = numStr;
		pNumeric_5_3->SetNull();
		*pId = (SQLINTEGER)300;
		EXPECT_NO_THROW(t.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());
	}


	TEST_P(TableTest, InsertNumericTypes_All)
	{


		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table t(&m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);

		ASSERT_NO_THROW(t.Open());
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(t.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		SQL_NUMERIC_STRUCT numStr18_0;
		ZeroMemory(&numStr18_0, sizeof(numStr18_0));
		numStr18_0.val[0] = 78;
		numStr18_0.val[1] = 243;
		numStr18_0.val[2] = 48;
		numStr18_0.val[3] = 166;
		numStr18_0.val[4] = 75;
		numStr18_0.val[5] = 155;
		numStr18_0.val[6] = 182;
		numStr18_0.val[7] = 1;
		numStr18_0.precision = 18;
		numStr18_0.scale = 0;
		numStr18_0.sign = 1;

		SQL_NUMERIC_STRUCT numStr18_10;
		ZeroMemory(&numStr18_10, sizeof(numStr18_10));
		numStr18_10.val[0] = 78;
		numStr18_10.val[1] = 243;
		numStr18_10.val[2] = 48;
		numStr18_10.val[3] = 166;
		numStr18_10.val[4] = 75;
		numStr18_10.val[5] = 155;
		numStr18_10.val[6] = 182;
		numStr18_10.val[7] = 1;
		numStr18_10.precision = 18;
		numStr18_10.scale = 10;
		numStr18_10.sign = 1;

		SQL_NUMERIC_STRUCT numStr5_3;
		ZeroMemory(&numStr5_3, sizeof(numStr5_3));
		numStr5_3.val[0] = 57;
		numStr5_3.val[1] = 48;
		numStr5_3.precision = 5;
		numStr5_3.scale = 3;
		numStr5_3.sign = 1;

		*pId = (SQLINTEGER)300;
		*pNumeric_18_0 = numStr18_0;
		*pNumeric_18_10 = numStr18_10;
		*pNumeric_5_3 = numStr5_3;
		EXPECT_NO_THROW(t.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());
	}

	
	TEST_P(TableTest, InsertBlobTypes)
	{
		std::wstring blobTypesTmpTableName = test::GetTableName(test::TableId::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
		Table bTable(&m_db, blobTypesTmpTableName, L"", L"", L"", AF_SELECT | AF_INSERT);
		ASSERT_NO_THROW(bTable.Open());

		ColumnBuffer* pId = bTable.GetColumnBuffer(0);
		ColumnBuffer* pBlob = bTable.GetColumnBuffer(1);
		ColumnBuffer* pVarBlob_20 = bTable.GetColumnBuffer(2);

		// Remove everything, ignoring if there was any data:
		test::ClearBlobTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
		wstring idName = test::GetIdColumnName(test::TableId::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt;

		SQLCHAR empty[] = { 0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

		SQLCHAR ff[] = { 255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255
		};

		SQLCHAR abc[] = { 0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67
		};

		SQLCHAR abc_ff[] = { 0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67,
			0xff, 0xff, 0xff, 0xff
		};

		// Insert some values
		*pId = (SQLINTEGER)100;
		pBlob->SetBinaryValue(empty, sizeof(empty));
		pVarBlob_20->SetNull();
		EXPECT_NO_THROW(bTable.Insert());
		*pId = (SQLINTEGER)101;
		pBlob->SetBinaryValue(ff, sizeof(ff));
		EXPECT_NO_THROW(bTable.Insert());
		*pId = (SQLINTEGER)102;
		pBlob->SetBinaryValue(abc, sizeof(abc));
		EXPECT_NO_THROW(bTable.Insert());
		*pId = (SQLINTEGER)103;
		pBlob->SetNull();
		pVarBlob_20->SetBinaryValue(abc, sizeof(abc));
		EXPECT_NO_THROW(bTable.Insert());
		*pId = (SQLINTEGER)104;
		pVarBlob_20->SetBinaryValue(abc_ff, sizeof(abc_ff));
		EXPECT_NO_THROW(bTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Now read the inserted values
		sqlstmt = (boost::wformat(L"%s = 100") % idName).str();
		bTable.Select(sqlstmt);
		EXPECT_TRUE(bTable.SelectNext());
		const SQLCHAR* pEmpty = *pBlob;
		EXPECT_EQ(0, memcmp(empty, pEmpty, sizeof(empty)));

		sqlstmt = (boost::wformat(L"%s = 101") % idName).str();
		bTable.Select(sqlstmt);
		EXPECT_TRUE(bTable.SelectNext());
		const SQLCHAR* pFf = *pBlob;
		EXPECT_EQ(0, memcmp(ff, pFf, sizeof(ff)));

		sqlstmt = (boost::wformat(L"%s = 102") % idName).str();
		bTable.Select(sqlstmt);
		EXPECT_TRUE(bTable.SelectNext());
		const SQLCHAR* pAbc = *pBlob;
		EXPECT_EQ(0, memcmp(abc, pAbc, sizeof(abc)));

		sqlstmt = (boost::wformat(L"%s = 103") % idName).str();
		bTable.Select(sqlstmt);
		EXPECT_TRUE(bTable.SelectNext());
		pAbc = *pVarBlob_20;
		EXPECT_EQ(0, memcmp(abc, pAbc, sizeof(abc)));
		EXPECT_EQ(sizeof(abc), pVarBlob_20->GetCb());

		sqlstmt = (boost::wformat(L"%s = 104") % idName).str();
		bTable.Select(sqlstmt);
		EXPECT_TRUE(bTable.SelectNext());
		const SQLCHAR* pAbc_ff = *pVarBlob_20;
		EXPECT_EQ(0, memcmp(abc_ff, pAbc_ff, sizeof(abc_ff)));
		EXPECT_EQ(sizeof(abc_ff), pVarBlob_20->GetCb());
	}


	TEST_P(TableTest, InsertCharTypes)
	{
		std::wstring charTypesTmpTableName = test::GetTableName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		Table cTable(&m_db, charTypesTmpTableName, L"", L"", L"", AF_SELECT | AF_INSERT);
		cTable.SetAutoBindingMode(AutoBindingMode::BIND_WCHAR_AS_CHAR);
		ASSERT_NO_THROW(cTable.Open());

		ColumnBuffer* pId = cTable.GetColumnBuffer(0);
		ColumnBuffer* pVarchar = cTable.GetColumnBuffer(1);
		ColumnBuffer* pChar = cTable.GetColumnBuffer(2);
		ColumnBuffer* pVarchar_10 = cTable.GetColumnBuffer(3);
		ColumnBuffer* pChar_10 = cTable.GetColumnBuffer(4);

		// Remove everything, ignoring if there was any data:
		test::ClearCharTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
		wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);

		// Insert some values:
		std::string s = "Hello World!";
		*pId = (SQLINTEGER)100;
		*pVarchar = s;
		*pChar = s;
		pVarchar_10->SetNull();
		pChar_10->SetNull();
		EXPECT_NO_THROW(cTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Insert one value that uses all space
		s = "abcde12345";
		*pId = (SQLINTEGER)101;
		pVarchar->SetNull();
		pChar->SetNull();
		*pVarchar_10 = s;
		*pChar_10 = s;
		EXPECT_NO_THROW(cTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Read them back from another table
		s = "Hello World!";
		Table t2(&m_db, charTypesTmpTableName, L"", L"", L"", AF_READ);
		t2.SetAutoBindingMode(AutoBindingMode::BIND_WCHAR_AS_CHAR);
		ASSERT_NO_THROW(t2.Open());
		t2.SetCharTrimRight(true);

		std::string varchar2, char2;
		t2.Select((boost::wformat(L"%s = 100") % idName).str());
		EXPECT_TRUE(t2.SelectNext());
		EXPECT_NO_THROW(varchar2 = t2.GetString(1));
		EXPECT_NO_THROW(char2 = t2.GetString(2));
		EXPECT_TRUE(t2.IsColumnNull(3));
		EXPECT_TRUE(t2.IsColumnNull(4));
		EXPECT_EQ(s, varchar2);
		EXPECT_EQ(s, char2);

		s = "abcde12345";
		t2.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(t2.SelectNext());
		EXPECT_TRUE(t2.IsColumnNull(1));
		EXPECT_TRUE(t2.IsColumnNull(2));
		EXPECT_NO_THROW(varchar2 = t2.GetString(3));
		EXPECT_NO_THROW(char2 = t2.GetString(4));
		EXPECT_EQ(s, varchar2);
		EXPECT_EQ(s, char2);
	}


	TEST_P(TableTest, InsertWCharTypes)
	{
		std::wstring charTypesTmpTableName = test::GetTableName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		Table cTable(&m_db, charTypesTmpTableName, L"", L"", L"", AF_SELECT | AF_INSERT);
		cTable.SetAutoBindingMode(AutoBindingMode::BIND_CHAR_AS_WCHAR);
		ASSERT_NO_THROW(cTable.Open());

		ColumnBuffer* pId = cTable.GetColumnBuffer(0);
		ColumnBuffer* pVarchar = cTable.GetColumnBuffer(1);
		ColumnBuffer* pChar = cTable.GetColumnBuffer(2);
		ColumnBuffer* pVarchar_10 = cTable.GetColumnBuffer(3);
		ColumnBuffer* pChar_10 = cTable.GetColumnBuffer(4);

		// Remove everything, ignoring if there was any data:
		test::ClearCharTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
		wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);

		// Insert some values:
		std::wstring s = L"Hello World!";
		*pId = (SQLINTEGER)100;
		*pVarchar = s;
		*pChar = s;
		pVarchar_10->SetNull();
		pChar_10->SetNull();
		EXPECT_NO_THROW(cTable.Insert());
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert one value that uses all space
		s = L"abcde12345";
		*pId = (SQLINTEGER)101;
		pVarchar->SetNull();
		pChar->SetNull();
		*pVarchar_10 = s;
		*pChar_10 = s;
		EXPECT_NO_THROW(cTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Read them back from another table
		s = L"Hello World!";
		Table t2(&m_db, charTypesTmpTableName, L"", L"", L"", AF_READ);
		t2.SetAutoBindingMode(AutoBindingMode::BIND_CHAR_AS_WCHAR);
		ASSERT_NO_THROW(t2.Open());
		t2.SetCharTrimRight(true);

		std::wstring varchar2, char2;
		t2.Select((boost::wformat(L"%s = 100") % idName).str());
		EXPECT_TRUE(t2.SelectNext());
		EXPECT_NO_THROW(varchar2 = t2.GetWString(1));
		EXPECT_NO_THROW(char2 = t2.GetWString(2));
		EXPECT_TRUE(t2.IsColumnNull(3));
		EXPECT_TRUE(t2.IsColumnNull(4));
		EXPECT_EQ(s, varchar2);
		EXPECT_EQ(s, char2);

		s = L"abcde12345";
		t2.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(t2.SelectNext());
		EXPECT_TRUE(t2.IsColumnNull(1));
		EXPECT_TRUE(t2.IsColumnNull(2));
		EXPECT_NO_THROW(varchar2 = t2.GetWString(3));
		EXPECT_NO_THROW(char2 = t2.GetWString(4));
		EXPECT_EQ(s, varchar2);
		EXPECT_EQ(s, char2);
	}


	// Update rows
	// -----------

	TEST_P(TableTest, UpdateIntTypes)
	{
		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", AF_SELECT | AF_INSERT | AF_UPDATE_PK);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			// Manually set the PKs on access
			iTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		ASSERT_NO_THROW(iTable.Open());
		
		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase));

		// Insert some values
		for (int i = 1; i < 10; i++)
		{
			test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, m_db, i, i, i, i);
		}
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Update single rows using key-values
		iTable.SetColumnValue(0, (SQLINTEGER)3);
		iTable.SetColumnValue(2, (SQLINTEGER)102);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumnValue(1, (SQLINTEGER)101);
			iTable.SetColumnNull(3);
		}
		else
		{
			iTable.SetColumnValue(1, (SQLSMALLINT)101);
			iTable.SetColumnValue(3, (SQLBIGINT)103);
		}
		EXPECT_NO_THROW(iTable.Update());

		iTable.SetColumnValue(0, (SQLINTEGER)5);
		iTable.SetColumnValue(2, (SQLINTEGER)1002);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumnValue(1, (SQLINTEGER)1001);
			iTable.SetColumnNull(3);
		}
		else
		{
			iTable.SetColumnValue(1, (SQLSMALLINT)1001);
			iTable.SetColumnValue(3, (SQLBIGINT)1003);
		}
		EXPECT_NO_THROW(iTable.Update());

		// And set some values to null
		iTable.SetColumnValue(0, (SQLINTEGER)7);
		iTable.SetColumnNull(1);
		iTable.SetColumnValue(2, (SQLINTEGER)99);
		iTable.SetColumnNull(3);
		EXPECT_NO_THROW(iTable.Update());

		// And set all to null
		iTable.SetColumnValue(0, (SQLINTEGER)9);
		iTable.SetColumnNull(1);
		iTable.SetColumnNull(2);
		iTable.SetColumnNull(3);
		EXPECT_NO_THROW(iTable.Update());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Read back the just updated values
		iTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 3, 101, 102, 103));

		iTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 5, 1001, 1002, 1003));

		iTable.Select((boost::wformat(L"%s = 7") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 7, test::ValueIndicator::IS_NULL, 99, test::ValueIndicator::IS_NULL));

		iTable.Select((boost::wformat(L"%s = 9") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, 9, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL, test::ValueIndicator::IS_NULL));
	}


	TEST_P(TableTest, UpdateDateTypes)
	{
		std::wstring dateTypesTableName = test::GetTableName(test::TableId::DATETYPES_TMP, m_odbcInfo.m_namesCase);
		Table dTable(&m_db, dateTypesTableName, L"", L"", L"", AF_SELECT | AF_INSERT | AF_UPDATE_PK);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			dTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		ASSERT_NO_THROW(dTable.Open());

		wstring idName = test::GetIdColumnName(test::TableId::DATETYPES_TMP, m_odbcInfo.m_namesCase);

		// Remove everything, ignoring if there was any data:
		test::ClearDateTypesTmpTable(m_db, m_odbcInfo.m_namesCase);

		// Insert some values
		SQL_TIME_STRUCT time = InitTime(13, 55, 56);
		SQL_DATE_STRUCT date = InitDate(26, 1, 2983);
		SQL_TIMESTAMP_STRUCT timestamp = InitTimestamp(13, 55, 56, 0, 26, 01, 1983);

		dTable.SetColumnValue(0, (SQLINTEGER)101);
		dTable.SetColumnValue(3, timestamp);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			dTable.SetColumnNull(1);
			dTable.SetColumnNull(2);
		}
		else
		{
			dTable.SetColumnValue(1, date);
			dTable.SetColumnValue(2, time);
		}
		dTable.Insert();
		// and a null row
		dTable.SetColumnValue(0, (SQLINTEGER)102);
		dTable.SetColumnNull(1);
		dTable.SetColumnNull(2);
		dTable.SetColumnNull(3);
		ASSERT_NO_THROW(dTable.Insert());
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Now update the values
		// \todo: We do not test the Fractions here. Lets fix Ticket #70 first
		date = InitDate(20, 9, 1985);
		time = InitTime(16, 31, 49);
		timestamp = InitTimestamp(16, 31, 49, 0, 20, 9, 1985);
		dTable.SetColumnValue(0, (SQLINTEGER)101);
		dTable.SetColumnNull(1);
		dTable.SetColumnNull(2);
		dTable.SetColumnNull(3);
		EXPECT_NO_THROW(dTable.Update());

		dTable.SetColumnValue(0, (SQLINTEGER)102);
		dTable.SetColumnValue(3, timestamp);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			dTable.SetColumnNull(1);
			dTable.SetColumnNull(2);
		}
		else
		{
			dTable.SetColumnValue(1, date);
			dTable.SetColumnValue(2, time);
		}
		EXPECT_NO_THROW(dTable.Update());
		ASSERT_NO_THROW(m_db.CommitTrans());

		// And read back
		dTable.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(dTable.SelectNext());
		EXPECT_TRUE(dTable.IsColumnNull(1));
		EXPECT_TRUE(dTable.IsColumnNull(2));
		EXPECT_TRUE(dTable.IsColumnNull(3));

		dTable.Select((boost::wformat(L"%s = 102") % idName).str());
		EXPECT_TRUE(dTable.SelectNext());
		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
		{
			EXPECT_FALSE(dTable.IsColumnNull(1));
			EXPECT_FALSE(dTable.IsColumnNull(2));
			EXPECT_TRUE(IsDateEqual(date, dTable.GetDate(1)));
			EXPECT_TRUE(IsTimeEqual(time, dTable.GetTime(2)));
		}
		EXPECT_FALSE(dTable.IsColumnNull(3));
		EXPECT_TRUE(IsTimestampEqual(timestamp, dTable.GetTimeStamp(3)));
	}

	
	TEST_P(TableTest, UpdateNumericTypes)
	{
		std::wstring numericTypesTmpTableName = test::GetTableName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table nTable(&m_db, numericTypesTmpTableName, L"", L"", L"", AF_SELECT | AF_INSERT | AF_UPDATE_PK);
		ASSERT_NO_THROW(nTable.Open());
		ColumnBuffer* pId = nTable.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = nTable.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = nTable.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = nTable.GetColumnBuffer(3);

		wstring idName = test::GetIdColumnName(test::TableId::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		test::ClearNumericTypesTmpTable(m_db, m_odbcInfo.m_namesCase);

		// Insert some boring 0 entries
		SQL_NUMERIC_STRUCT num = InitNullNumeric();
		*pId = (SQLINTEGER)100;
		*pNumeric_18_0 = num;
		*pNumeric_18_10 = num;
		*pNumeric_5_3 = num;
		nTable.Insert();
		*pId = (SQLINTEGER)101;
		pNumeric_18_0->SetNull();
		pNumeric_18_10->SetNull();
		pNumeric_5_3->SetNull();
		nTable.Insert();
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Now update that with our well known entries
		// Note: We MUST set the correct precision and scale, at least for ms!
		SQL_NUMERIC_STRUCT num18_0;
		SQL_NUMERIC_STRUCT num18_10;
		SQL_NUMERIC_STRUCT num5_3;
		ZeroMemory(&num18_0, sizeof(num18_0));
		ZeroMemory(&num18_10, sizeof(num18_10));
		ZeroMemory(&num5_3, sizeof(num5_3));
		num18_0.sign = 1;
		num18_0.scale = 0;
		num18_0.precision = 18;
		num18_0.val[0] = 78;
		num18_0.val[1] = 243;
		num18_0.val[2] = 48;
		num18_0.val[3] = 166;
		num18_0.val[4] = 75;
		num18_0.val[5] = 155;
		num18_0.val[6] = 182;
		num18_0.val[7] = 1;

		num18_10 = num18_0;
		num18_10.scale = 10;

		num5_3.sign = 1;
		num5_3.scale = 3;
		num5_3.precision = 5;
		num5_3.val[0] = 57;
		num5_3.val[1] = 48;

		*pId = (SQLINTEGER)101;
		*pNumeric_18_0 = num18_0;
		*pNumeric_18_10 = num18_10;
		*pNumeric_5_3 = num5_3;
		EXPECT_NO_THROW(nTable.Update());

		*pId = (SQLINTEGER)100;
		pNumeric_18_0->SetNull();
		pNumeric_18_10->SetNull();
		pNumeric_5_3->SetNull();
		EXPECT_NO_THROW(nTable.Update());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// And read back the values
		nTable.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		SQL_NUMERIC_STRUCT n18_0, n18_10, n5_3;
		n18_0 = *pNumeric_18_0;
		n18_10 = *pNumeric_18_10;
		n5_3 = *pNumeric_5_3;
		EXPECT_EQ(0, memcmp(&num18_0, &n18_0, sizeof(n18_0)));
		EXPECT_EQ(0, memcmp(&num18_10, &n18_10, sizeof(n18_10)));
		EXPECT_EQ(0, memcmp(&num5_3, &n5_3, sizeof(n5_3)));

		nTable.Select((boost::wformat(L"%s = 100") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_TRUE(pNumeric_18_0->IsNull());
		EXPECT_TRUE(pNumeric_18_10->IsNull());
		EXPECT_TRUE(pNumeric_5_3->IsNull());
	}


	TEST_P(TableTest, UpdateFloatTypes)
	{
		std::wstring floatTypesTableName = test::GetTableName(test::TableId::FLOATTYPES_TMP, m_odbcInfo.m_namesCase);
		Table fTable(&m_db, floatTypesTableName, L"", L"", L"", AF_SELECT | AF_INSERT | AF_UPDATE_PK);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			fTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		ASSERT_NO_THROW(fTable.Open());
		ColumnBuffer* pId = fTable.GetColumnBuffer(0);
		ColumnBuffer* pDouble = fTable.GetColumnBuffer(1);
		ColumnBuffer* pFloat = fTable.GetColumnBuffer(2);

		wstring idName = test::GetIdColumnName(test::TableId::FLOATTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		test::ClearFloatTypesTmpTable(m_db, m_odbcInfo.m_namesCase);

		// Insert some values
		*pId = (SQLINTEGER)101;
		*pDouble = 3.14159265359;
		*pFloat = -3.14159;
		fTable.Insert();
		ASSERT_NO_THROW(m_db.CommitTrans());

		// And update them using the key fields
		*pDouble = -6.2343354;
		*pFloat = 989.213;
		EXPECT_NO_THROW(fTable.Update());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Open another table and read the values from there
		Table fTable2(&m_db, floatTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(fTable2.Open());
		ColumnBuffer* pId2 = fTable2.GetColumnBuffer(0);
		ColumnBuffer* pDouble2 = fTable2.GetColumnBuffer(1);
		ColumnBuffer* pFloat2 = fTable2.GetColumnBuffer(2);

		fTable2.Select();
		EXPECT_TRUE(fTable2.SelectNext());

		EXPECT_EQ(101, (SQLINTEGER)*pId2);
		EXPECT_EQ((SQLDOUBLE)*pDouble, (SQLDOUBLE)*pDouble2);
		EXPECT_EQ((SQLDOUBLE)*pFloat, (SQLDOUBLE)*pFloat2);
	}


	TEST_P(TableTest, UpdateWCharTypes)
	{
		std::wstring charTypesTmpTableName = test::GetTableName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		Table cTable(&m_db, charTypesTmpTableName, L"", L"", L"", AF_SELECT | AF_UPDATE_PK | AF_INSERT);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			cTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		cTable.SetAutoBindingMode(AutoBindingMode::BIND_CHAR_AS_WCHAR);
		ASSERT_NO_THROW(cTable.Open());
		cTable.SetCharTrimRight(true);

		ColumnBuffer* pId = cTable.GetColumnBuffer(0);
		ColumnBuffer* pVarchar = cTable.GetColumnBuffer(1);
		ColumnBuffer* pChar = cTable.GetColumnBuffer(2);
		ColumnBuffer* pVarchar_10 = cTable.GetColumnBuffer(3);
		ColumnBuffer* pChar_10 = cTable.GetColumnBuffer(4);
		
		// Remove everything, ignoring if there was any data:
		test::ClearCharTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
		wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);

		// Insert some values:
		// \todo: Note, in IBM DB2 special chars seem to occupy more space (two bytes`?). We cannot have more than 5 special chars if the size of the field is 10..
		// but this might be because we bind a char to wchar or so.. hm..
		std::wstring s100 = L"הצאטי";
		*pId = (SQLINTEGER)100;
		*pVarchar = s100;
		*pChar = s100;
		*pVarchar_10 = s100;
		*pChar_10 = s100;
		cTable.Insert();
		std::wstring s101 = L"abcde12345";
		*pId = (SQLINTEGER)101;
		*pVarchar = s101;
		*pChar = s101;
		*pVarchar_10 = s101;
		*pChar_10 = s101;
		cTable.Insert();
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Select and check
		cTable.Select((boost::wformat(L"%s = 100") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		std::wstring s2;
		EXPECT_NO_THROW(s2 = cTable.GetWString(1));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(s2 = cTable.GetWString(2));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(s2 = cTable.GetWString(3));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(s2 = cTable.GetWString(4));
		EXPECT_EQ(s100, s2);

		cTable.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		EXPECT_NO_THROW(s2 = cTable.GetWString(1));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(s2 = cTable.GetWString(2));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(s2 = cTable.GetWString(3));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(s2 = cTable.GetWString(4));
		EXPECT_EQ(s101, s2);
	}


	TEST_P(TableTest, UpdateCharTypes)
	{
		std::wstring charTypesTmpTableName = test::GetTableName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		Table cTable(&m_db, charTypesTmpTableName, L"", L"", L"", AF_SELECT | AF_INSERT | AF_UPDATE);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			cTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		cTable.SetAutoBindingMode(AutoBindingMode::BIND_WCHAR_AS_CHAR);
		ASSERT_NO_THROW(cTable.Open());
		cTable.SetCharTrimRight(true);

		ColumnBuffer* pId = cTable.GetColumnBuffer(0);
		ColumnBuffer* pVarchar = cTable.GetColumnBuffer(1);
		ColumnBuffer* pChar = cTable.GetColumnBuffer(2);
		ColumnBuffer* pVarchar_10 = cTable.GetColumnBuffer(3);
		ColumnBuffer* pChar_10 = cTable.GetColumnBuffer(4);

		// Remove everything, ignoring if there was any data:
		test::ClearCharTypesTmpTable(m_db, m_odbcInfo.m_namesCase);
		wstring idName = test::GetIdColumnName(test::TableId::CHARTYPES_TMP, m_odbcInfo.m_namesCase);

		// Insert some values:
		// \todo: Note, in IBM DB2 special chars seem to occupy more space (two bytes`?). We cannot have more than 5 special chars if the size of the field is 10..
		// but this might be because we bind a char to wchar or so.. hm..
		std::string s100 = "abcd";
		*pId = (SQLINTEGER)100;
		*pVarchar = s100;
		*pChar = s100;
		*pVarchar_10 = s100;
		*pChar_10 = s100;
		cTable.Insert();
		std::string s101 = "abcde12345";
		*pId = (SQLINTEGER)101;
		*pVarchar = s101;
		*pChar = s101;
		*pVarchar_10 = s101;
		*pChar_10 = s101;
		cTable.Insert();
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Select and check
		cTable.Select((boost::wformat(L"%s = 100") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		std::string s2;
		EXPECT_NO_THROW(s2 = cTable.GetString(1));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(s2 = cTable.GetString(2));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(s2 = cTable.GetString(3));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(s2 = cTable.GetString(4));
		EXPECT_EQ(s100, s2);

		cTable.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		EXPECT_NO_THROW(s2 = cTable.GetString(1));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(s2 = cTable.GetString(2));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(s2 = cTable.GetString(3));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(s2 = cTable.GetString(4));
		EXPECT_EQ(s101, s2);
	}


	TEST_P(TableTest, UpdateBlobTypes)
	{
		std::wstring blobTypesTmpTableName = test::GetTableName(test::TableId::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
		Table bTable(&m_db, blobTypesTmpTableName, L"", L"", L"", AF_SELECT | AF_INSERT | AF_UPDATE);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			bTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		ASSERT_NO_THROW(bTable.Open());

		// Remove everything, ignoring if there was any data:
		wstring idName = test::GetIdColumnName(test::TableId::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt;
		test::ClearBlobTypesTmpTable(m_db, m_odbcInfo.m_namesCase);

		SQLCHAR empty[] = { 0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

		SQLCHAR ff[] = { 255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255
		};

		SQLCHAR abc[] = { 0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67
		};

		SQLCHAR abc_ff[] = { 0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67,
			0xff, 0xff, 0xff, 0xff
		};

		// Insert some values
		bTable.SetColumnValue(0, (SQLINTEGER)100);
		bTable.SetBinaryValue(1, abc, sizeof(abc));
		bTable.SetColumnNull(2);
		bTable.Insert();
		bTable.SetColumnValue(0, (SQLINTEGER)101);
		bTable.SetColumnNull(1);
		bTable.SetBinaryValue(2, abc_ff, sizeof(abc_ff));
		bTable.Insert();
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Update 
		bTable.SetColumnValue(0, (SQLINTEGER)100);
		bTable.SetColumnNull(1);
		bTable.SetBinaryValue(2, abc, sizeof(abc));
		EXPECT_NO_THROW(bTable.Update());
		bTable.SetColumnValue(0, (SQLINTEGER)101);
		bTable.SetBinaryValue(1, empty, sizeof(empty));
		bTable.SetColumnNull(2);
		EXPECT_NO_THROW(bTable.Update());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Re-Fetch and compare
		bTable.Select((boost::wformat(L"%s = 100") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_TRUE(bTable.IsColumnNull(1));
		EXPECT_FALSE(bTable.IsColumnNull(2));
		const SQLCHAR* pBlobBuff = NULL;
		SQLLEN size, length = 0;
		EXPECT_NO_THROW(pBlobBuff = bTable.GetBinaryValue(2, size, length));
		EXPECT_EQ(0, memcmp(pBlobBuff, abc, sizeof(abc)));
		EXPECT_EQ(20, size);
		EXPECT_EQ(16, length);
		bTable.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_FALSE(bTable.IsColumnNull(1));
		EXPECT_TRUE(bTable.IsColumnNull(2));
		EXPECT_NO_THROW(pBlobBuff = bTable.GetBinaryValue(1, size, length));
		EXPECT_EQ(0, memcmp(pBlobBuff, empty, sizeof(empty)));
		EXPECT_EQ(16, size);
		EXPECT_EQ(16, length);
	}


	TEST_P(TableTest, UpdateWhere)
	{
		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", AF_SELECT | AF_INSERT | AF_UPDATE_WHERE);
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		ASSERT_NO_THROW(iTable.Open());

		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);

		// Remove everything, ignoring if there was any data:
		test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase);

		// Insert some values
		for (int i = 1; i < 10; i++)
		{
			test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, m_db, i, i, i, i);
		}

		// Now update using our WHERE statement. This allows us to update also key rows. Shift all values *(1000)
		int shift = 1000;
		for (int i = 1; i < 10; i++)
		{
			iTable.SetColumnValue(0, (SQLINTEGER)i * shift);
			iTable.SetColumnValue(2, (SQLINTEGER)(i * shift));
			if (m_db.GetDbms() == DatabaseProduct::ACCESS)
			{
				iTable.SetColumnValue(1, (SQLINTEGER)(i * shift));
				iTable.SetColumnNull(3);
			}
			else
			{
				iTable.SetColumnValue(1, (SQLSMALLINT)(i * shift));
				iTable.SetColumnValue(3, (SQLBIGINT)(i * shift));
			}

			wstring sqlstmt = (boost::wformat(L"%s = %d") % idName %i).str();
			EXPECT_NO_THROW(iTable.Update(sqlstmt));
		}
		EXPECT_NO_THROW(m_db.CommitTrans());

		// And select them and compare
		wstring sqlstmt = (boost::wformat(L"%s > 0 ORDER BY %s") % idName %idName).str();
		iTable.Select(sqlstmt);
		for (int i = 1; i < 10; i++)
		{
			EXPECT_TRUE(iTable.SelectNext());
			EXPECT_TRUE(test::IsIntRecordEqual(m_db, iTable, i * shift, i * shift, i * shift, i * shift));
		}
		EXPECT_FALSE(iTable.SelectNext());
	}


	TEST_P(TableTest, GetColumnBufferIndex)
	{
		std::wstring intTypesTableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", AF_SELECT);
		ASSERT_NO_THROW(iTable.Open());

		EXPECT_EQ(0, iTable.GetColumnBufferIndex(L"idintegertypes", false));
		EXPECT_EQ(1, iTable.GetColumnBufferIndex(L"tsmallint", false));
		EXPECT_EQ(2, iTable.GetColumnBufferIndex(L"tint", false));
		EXPECT_EQ(3, iTable.GetColumnBufferIndex(L"tbigint", false));
	}


// Interfaces
// ----------

} // namespace exodbc
