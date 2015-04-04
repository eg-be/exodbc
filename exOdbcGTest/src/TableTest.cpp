/*!
 * \file DbTableTest.cpp
 * \author Elias Gerber <egerber@gmx.net>
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

	void TableTest::SetUp()
	{
		m_odbcInfo = GetParam();

		// Set up Env
		m_env.AllocateEnvironmentHandle();
		// Try to set to the ODBC v3 : We need that for the tests to run correct. 3.8 is not supported by all databases and we dont use specific stuff from it.
		// except for the TIME2 things, sql server specific. those tests can create their own env.
		m_env.SetOdbcVersion(OdbcVersion::V_3);

		// And database
		ASSERT_NO_THROW(m_db.AllocateConnectionHandle(m_env));
		ASSERT_NO_THROW(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
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
		MIntTypesTable table(m_db, m_odbcInfo.m_namesCase);
		EXPECT_NO_THROW(table.Open(m_db, TOF_NONE));

		// If we pass in the STableInfo directly we should also be able to "open"
		// a totally non-sense table:
		STableInfo neTableInfo;
		neTableInfo.m_tableName = L"NotExisting";
		Table neTable(m_db, 2, neTableInfo, AF_READ);
		SQLINTEGER idNotExisting = 0;
		neTable.SetColumn(0, L"idNotExistring", &idNotExisting, SQL_C_SLONG, sizeof(idNotExisting));
		EXPECT_NO_THROW(neTable.Open(m_db, TOF_NONE));
		// \todo So we can prove in the test that we will fail doing a SELECT later
	}


	TEST_P(TableTest, OpenManualWritableWithoutCheck)
	{
		// Open an existing table without checking for privileges or existence
		Table iTable(m_db, 4, TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase), L"", L"", L"", AF_READ_WRITE);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		int type = SQL_C_SLONG;
		iTable.SetColumn(0, TestTables::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT);
		iTable.SetColumn(1, TestTables::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_INSERT);
		iTable.SetColumn(2, TestTables::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT);
		iTable.SetColumn(3, TestTables::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);
	}


	TEST_P(TableTest, OpenManualCheckExistence)
	{
		// Open a table with checking for existence
		MIntTypesTable table(m_db, m_odbcInfo.m_namesCase);
		EXPECT_NO_THROW(table.Open(m_db, TOF_CHECK_EXISTANCE));

		// Open a non-existing table
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			MNotExistingTable neTable(m_db, m_odbcInfo.m_namesCase);
			EXPECT_THROW(neTable.Open(m_db, TOF_CHECK_EXISTANCE), exodbc::Exception);
		}
	}


	TEST_P(TableTest, OpenManualCheckColumnFlagSelect)
	{
		bool c1 = CF_SELECT & CF_INSERT;
		bool c2 = CF_SELECT & CF_INSERT;
		bool b = (!(0 == SQL_UNKNOWN_TYPE) && ((CF_SELECT & CF_INSERT) || (CF_SELECT & CF_UPDATE)));
		bool b2 = (!(0 == SQL_UNKNOWN_TYPE) && (c1 || c2));

		// Open a table manually but do not set the Select flag for all columns
		Table iTable(m_db, 4, TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		iTable.SetColumn(0, TestTables::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), &id, SQL_C_SLONG, sizeof(id), CF_SELECT);
		iTable.SetColumn(1, TestTables::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), &si, SQL_C_SSHORT, sizeof(si), CF_SELECT);
		iTable.SetColumn(2, TestTables::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), &i, SQL_C_SLONG, sizeof(i), CF_NONE);
		iTable.SetColumn(3, TestTables::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT);

		ASSERT_NO_THROW(iTable.Open(m_db));
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
		wstring sqlstmt = boost::str(boost::wformat(L"%s = 7") % TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase));
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
		// Open a table manually but do not set the Select flag for all columns
		Table iTable(m_db, 4, TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT | AF_INSERT | AF_DELETE);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		int type = SQL_C_SLONG;
		// \todo
		iTable.SetColumn(0, TestTables::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT);
		iTable.SetColumn(1, TestTables::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_INSERT);
		iTable.SetColumn(2, TestTables::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT);
		iTable.SetColumn(3, TestTables::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);

		// Open and remove all data from the table
		iTable.Open(m_db);
		ASSERT_NO_THROW(TestTables::ClearTestTable(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase, iTable, m_db));

		// Insert a value by using our primary key - columnIndex 2 will not get inserted but the default NULL value will be set by the db
		iTable.SetColumnValue(0, (SQLINTEGER)11);
		iTable.SetColumnValue(1, (SQLSMALLINT)202);
		iTable.SetColumnValue(2, (SQLINTEGER)303);
		iTable.SetColumnValue(3, (SQLBIGINT)-404);
		EXPECT_NO_THROW(iTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Read back from another table
		Table iTable2(m_db, TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_READ);
		iTable2.Open(m_db);
		iTable2.Select();
		ASSERT_TRUE(iTable2.SelectNext());
		EXPECT_EQ(11, boost::get<SQLINTEGER>(iTable2.GetColumnValue(0)));
		EXPECT_EQ(202, boost::get<SQLSMALLINT>(iTable2.GetColumnValue(1)));
		EXPECT_EQ(-404, boost::get<SQLBIGINT>(iTable2.GetColumnValue(3)));
		EXPECT_TRUE(iTable2.IsColumnNull(2));
	}


	TEST_P(TableTest, OpenManualCheckColumnFlagUpdate)
	{
		// Open a table manually but do not set the Select flag for all columns
		Table iTable(m_db, 4, TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT | AF_UPDATE | AF_DELETE | AF_INSERT);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		int type = SQL_C_SLONG;
		// \todo
		iTable.SetColumn(0, TestTables::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_UPDATE | CF_INSERT);
		iTable.SetColumn(1, TestTables::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_UPDATE | CF_INSERT);
		iTable.SetColumn(2, TestTables::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT | CF_INSERT);
		iTable.SetColumn(3, TestTables::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_UPDATE | CF_INSERT);

		// Open and remove all data from the table
		iTable.Open(m_db);
		ASSERT_NO_THROW(TestTables::ClearTestTable(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase, iTable, m_db));

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
		Table iTable2(m_db, TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_READ);
		iTable2.Open(m_db);
		iTable2.Select();
		ASSERT_TRUE(iTable2.SelectNext());
		EXPECT_EQ(11, boost::get<SQLINTEGER>(iTable2.GetColumnValue(0)));
		EXPECT_EQ(880, boost::get<SQLSMALLINT>(iTable2.GetColumnValue(1)));
		EXPECT_EQ(303, boost::get<SQLINTEGER>(iTable2.GetColumnValue(2)));
		EXPECT_EQ(1001, boost::get<SQLBIGINT>(iTable2.GetColumnValue(3)));
	}
	

	TEST_P(TableTest, OpenManualPrimaryKeys)
	{
		// Open a table by defining primary keys manually
		// Open a table manually but do not set the Select flag for all columns
		Table iTable(m_db, 4, TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT | AF_DELETE | AF_INSERT);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		iTable.SetColumn(0, TestTables::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT |  CF_INSERT | CF_PRIMARY_KEY);
		iTable.SetColumn(1, TestTables::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_INSERT);
		iTable.SetColumn(2, TestTables::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT | CF_INSERT);
		iTable.SetColumn(3, TestTables::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);

		// Opening must work
		EXPECT_NO_THROW(iTable.Open(m_db, TOF_DO_NOT_QUERY_PRIMARY_KEYS));

		// But opening if primary keys are not defined must fail
		Table iTable2(m_db, 4, TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT | AF_DELETE | AF_INSERT);
		SQLINTEGER id2 = 0;
		SQLSMALLINT si2 = 0;
		SQLINTEGER i2 = 0;
		SQLBIGINT bi2 = 0;
		iTable2.SetColumn(0, TestTables::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id2, SQL_C_SLONG, sizeof(id2), CF_SELECT | CF_INSERT);
		iTable2.SetColumn(1, TestTables::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si2, SQL_C_SSHORT, sizeof(si2), CF_SELECT | CF_INSERT);
		iTable2.SetColumn(2, TestTables::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i2, SQL_C_SLONG, sizeof(i2), CF_SELECT | CF_INSERT);
		iTable2.SetColumn(3, TestTables::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi2, SQL_C_SBIGINT, sizeof(bi2), CF_SELECT | CF_INSERT);
		EXPECT_THROW(iTable2.Open(m_db, TOF_DO_NOT_QUERY_PRIMARY_KEYS), Exception);

		// But if we open for select only, we do not care about the primary keys
		Table iTable3(m_db, 4, TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_READ);
		SQLINTEGER id3 = 0;
		SQLSMALLINT si3 = 0;
		SQLINTEGER i3 = 0;
		SQLBIGINT bi3 = 0;
		iTable3.SetColumn(0, TestTables::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id3, SQL_C_SLONG, sizeof(id3), CF_SELECT);
		iTable3.SetColumn(1, TestTables::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si3, SQL_C_SSHORT, sizeof(si3), CF_SELECT);
		iTable3.SetColumn(2, TestTables::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i3, SQL_C_SLONG, sizeof(i3), CF_SELECT);
		iTable3.SetColumn(3, TestTables::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi3, SQL_C_SBIGINT, sizeof(bi3), CF_SELECT);
		EXPECT_NO_THROW(iTable3.Open(m_db, TOF_DO_NOT_QUERY_PRIMARY_KEYS));
	}


	TEST_P(TableTest, OpenAutoWithoutCheck)
	{
		// Open an auto-table without checking for privileges or existence
		// This makes only sense if we've already determined the correct STableInfo structure
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		STableInfo tableInfo;
		EXPECT_NO_THROW(tableInfo = m_db.FindOneTable(tableName, L"", L"", L""));

		exodbc::Table table(m_db, tableInfo, AF_READ);
		EXPECT_NO_THROW(table.Open(m_db, TOF_NONE));

		// If we try to open an auto-table this will never work if you've passed invalid information:
		// As soon as the columns are searched, we expect to fail
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			STableInfo neTableInfo;
			neTableInfo.m_tableName = L"NotExisting";
			Table neTable(m_db, neTableInfo, AF_READ);
			EXPECT_THROW(neTable.Open(m_db, TOF_NONE), Exception);
		}
	}


	TEST_P(TableTest, OpenAutoCheckExistence)
	{
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table table(m_db, tableName, L"", L"", L"", AF_READ);
		EXPECT_NO_THROW(table.Open(m_db, TOF_CHECK_EXISTANCE));

		// Open a non-existing table
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			std::wstring neName = TestTables::GetTableName(TestTables::Table::NOT_EXISTING, m_odbcInfo.m_namesCase);
			exodbc::Table neTable(m_db, neName, L"", L"", L"", AF_READ);
			EXPECT_THROW(neTable.Open(m_db, TOF_CHECK_EXISTANCE), Exception);
		}
	}


	TEST_P(TableTest, OpenAutoCheckPrivs)
	{
		// See #134
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			// Not working on MySQL, see Ticket #134 and #76
			LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #134 and #76");
		}

		// Test to open read-only a table we know we have all rights:
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table rTable(m_db, tableName, L"", L"", L"", AF_READ);
		EXPECT_NO_THROW(rTable.Open(m_db, TOF_CHECK_PRIVILEGES));

		// Test to open read-write a table we know we have all rights:
		exodbc::Table rTable2(m_db, tableName, L"", L"", L"", AF_READ_WRITE);
		EXPECT_NO_THROW(rTable2.Open(m_db, TOF_CHECK_PRIVILEGES));

		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			// We only have a Read-only user for ms sql server
			Database db(m_env);
			ASSERT_NO_THROW(db.Open(m_odbcInfo.m_dsn, L"exOdbcReadOnly", L"exodbcReadOnly"));

			// Test to open a table read-only
			// Note that here in ms server we have given the user no rights except the select for this table
			// If you add him to some role, things will get messed up: No privs are reported, but the user can
			// still access the table for selecting
			exodbc::Table table2(db, tableName, L"", L"", L"", AF_READ);
			EXPECT_NO_THROW(table2.Open(db, TOF_CHECK_PRIVILEGES));

			// We expect to fail if trying to open for writing
			table2.Close();
			table2.SetAccessFlags(db, AF_READ_WRITE);
			EXPECT_THROW(table2.Open(db, TOF_CHECK_PRIVILEGES), Exception);

			// But we do not fail if we do not check the privs
			EXPECT_NO_THROW(table2.Open(db));

			// Try to open one we do not even have the rights for
			std::wstring table3Name = TestTables::GetTableName(TestTables::Table::NUMERICTYPES, m_odbcInfo.m_namesCase);
			Table table3(db, table3Name, L"", L"", L"", AF_READ);
			EXPECT_THROW(table3.Open(db), Exception);
		}
	}


	TEST_P(TableTest, OpenAutoWithUnsupportedColumn)
	{
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			// \note Not working for mysql so far because we have no unsupported column - altough we fail on geometry columns, see #121
			return;
		}

		std::wstring tableName = TestTables::GetTableName(TestTables::Table::NOT_SUPPORTED, m_odbcInfo.m_namesCase);
		exodbc::Table nst(m_db, tableName, L"", L"", L"", AF_READ_WRITE);

		// Expect to fail if we open with default flags
		EXPECT_THROW(nst.Open(m_db), NotSupportedException);

		// But not if we pass the flag to skip
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst.Open(m_db, TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// We should now be able to select from column indexed 0 (id), 1 (int1) and 3 (int2) - 2 (xml) should be missing
		nst.Select();
		EXPECT_TRUE(nst.SelectNext());
		SQLINTEGER id, int1, int2;
		EXPECT_NO_THROW(nst.GetColumnValue(0, id));
		EXPECT_NO_THROW(nst.GetColumnValue(1, int1));
		EXPECT_NO_THROW(nst.GetColumnValue(2, int2));
		EXPECT_THROW(nst.GetColumnBuffer(3), IllegalArgumentException);
		EXPECT_EQ(1, id);
		EXPECT_EQ(10, int1);
		EXPECT_EQ(12, int2);
	}


	TEST_P(TableTest, SelectFromAutoWithUnsupportedColumn)
	{
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			// \note Not working for mysql so far because we have no unsupported column - altough we fail on geometry columns, see #121
			return;
		}

		std::wstring tableName = TestTables::GetTableName(TestTables::Table::NOT_SUPPORTED, m_odbcInfo.m_namesCase);
		exodbc::Table nst(m_db, tableName, L"", L"", L"", AF_READ_WRITE);

		// we do not if we pass the flag to skip
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst.Open(m_db, TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// We should now be able to select from column indexed 0 (id), 1 (int1) and 3 (int2) - 2 (xml) should be missing
		nst.Select();
		EXPECT_TRUE(nst.SelectNext());
		SQLINTEGER id, int1, int2;
		EXPECT_NO_THROW(nst.GetColumnValue(0, id));
		EXPECT_NO_THROW(nst.GetColumnValue(1, int1));
		EXPECT_NO_THROW(nst.GetColumnValue(2, int2));
		EXPECT_THROW(nst.GetColumnBuffer(3), IllegalArgumentException);
		EXPECT_EQ(1, id);
		EXPECT_EQ(10, int1);
		EXPECT_EQ(12, int2);
	}


	TEST_P(TableTest, InsertIntoAutoWithUnsupportedColumn)
	{
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			// \note Not working for mysql so far because we have no unsupported column - altough we fail on geometry columns, see #121
			return;
		}

		std::wstring tableName = TestTables::GetTableName(TestTables::Table::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table nst(m_db, tableName, L"", L"", L"", AF_READ_WRITE);

		// we do not if we pass the flag to skip
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst.Open(m_db, TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(nst.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert some data
		// note the shifted columnbuffer-indexes, see #123
		ColumnBuffer* pId = nst.GetColumnBuffer(0);
		ColumnBuffer* pInt1 = nst.GetColumnBuffer(1);
		ColumnBuffer* pInt2 = nst.GetColumnBuffer(2);
		
		*pId = (SQLINTEGER)2;
		*pInt1 = (SQLINTEGER)20;
		*pInt2 = (SQLINTEGER)22;
		EXPECT_NO_THROW(nst.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// We should now be able to select. As a test, use a different table object
		exodbc::Table nst2(m_db, tableName, L"", L"", L"", AF_READ_WRITE);
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst2.Open(m_db, TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		nst2.Select();
		EXPECT_TRUE(nst2.SelectNext());
		SQLINTEGER id, int1, int2;
		EXPECT_NO_THROW(nst2.GetColumnValue(0, id));
		EXPECT_NO_THROW(nst2.GetColumnValue(1, int1));
		EXPECT_NO_THROW(nst2.GetColumnValue(2, int2));
		EXPECT_EQ(2, id);
		EXPECT_EQ(20, int1);
		EXPECT_EQ(22, int2);
	}


	TEST_P(TableTest, UpdateIntoAutoWithUnsupportedColumn)
	{
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			// \note Not working for mysql so far because we have no unsupported column - altough we fail on geometry columns, see #121
			return;
		}

		std::wstring tableName = TestTables::GetTableName(TestTables::Table::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table nst(m_db, tableName, L"", L"", L"", AF_READ_WRITE);

		// we do not if we pass the flag to skip
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst.Open(m_db, TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(nst.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert some data
		// note the shifted columnbuffer-indexes, see #123
		nst.SetColumnValue(0, (SQLINTEGER)2);
		nst.SetColumnValue(1, (SQLINTEGER)20);
		nst.SetColumnValue(2, (SQLINTEGER)22);
		EXPECT_NO_THROW(nst.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Now update it
		nst.SetColumnValue(1, (SQLINTEGER)30);
		nst.SetColumnValue(2, (SQLINTEGER)32);
		EXPECT_NO_THROW(nst.Update());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// We should now be able to select. As a test, use a different table object
		exodbc::Table nst2(m_db, tableName, L"", L"", L"", AF_READ_WRITE);
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst2.Open(m_db, TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		nst2.Select();
		EXPECT_TRUE(nst2.SelectNext());
		SQLINTEGER id, int1, int2;
		EXPECT_NO_THROW(nst2.GetColumnValue(0, id));
		EXPECT_NO_THROW(nst2.GetColumnValue(1, int1));
		EXPECT_NO_THROW(nst2.GetColumnValue(2, int2));
		EXPECT_EQ(2, id);
		EXPECT_EQ(30, int1);
		EXPECT_EQ(32, int2);
	}


	TEST_P(TableTest, DeleteFromAutoWithUnsupportedColumn)
	{
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			// \note Not working for mysql so far because we have no unsupported column - altough we fail on geometry columns, see #121
			return;
		}

		std::wstring tableName = TestTables::GetTableName(TestTables::Table::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table nst(m_db, tableName, L"", L"", L"", AF_READ_WRITE);

		// we do not if we pass the flag to skip
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst.Open(m_db, TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NOT_SUPPORTED_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(nst.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert some data
		// note the shifted columnbuffer-indexes, see #123
		ColumnBuffer* pId = nst.GetColumnBuffer(0);
		ColumnBuffer* pInt1 = nst.GetColumnBuffer(1);
		ColumnBuffer* pInt2 = nst.GetColumnBuffer(2);

		*pId = (SQLINTEGER)2;
		*pInt1 = (SQLINTEGER)20;
		*pInt2 = (SQLINTEGER)22;
		EXPECT_NO_THROW(nst.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Check its there
		ASSERT_NO_THROW(nst.Select());
		ASSERT_TRUE(nst.SelectNext());

		// Now delete it
		*pId = (SQLINTEGER)2;
		EXPECT_NO_THROW(nst.Delete());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// We should now longer be able to select it. As a test, use a different table object
		exodbc::Table nst2(m_db, tableName, L"", L"", L"", AF_READ_WRITE);
		{
			LogLevelFatal llf;
			EXPECT_NO_THROW(nst2.Open(m_db, TOF_SKIP_UNSUPPORTED_COLUMNS));
		}

		nst2.Select();
		EXPECT_FALSE(nst2.SelectNext());
	}


	TEST_P(TableTest, Close)
	{
		// Create table
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(m_db, tableName, L"", L"", L"", AF_READ);

		// Close when not open must fail
		{
			DontDebugBreak ddb;
			LogLevelFatal llf;
			EXPECT_THROW(iTable.Close(), AssertionException);
		}

		// Open a Table read-only
		ASSERT_NO_THROW(iTable.Open(m_db));

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
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(m_db, tableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(iTable.Open(m_db));

		// Close Table
		EXPECT_NO_THROW(iTable.Close());

		// Open again for reading
		EXPECT_NO_THROW(iTable.Open(m_db));

		// And close again
		EXPECT_NO_THROW(iTable.Close());
	}


	TEST_P(TableTest, SetAccessFlags)
	{
		// Create a Table, for reading only
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(m_db, tableName, L"", L"", L"", AF_READ);

		// Before opening it, change the mode, then open
		ASSERT_NO_THROW(iTable.SetAccessFlags(m_db, AF_READ_WRITE));
		ASSERT_NO_THROW(iTable.Open(m_db));

		// We cannot change the mode if the table is open
		{
			DontDebugBreak ddb;
			LogLevelFatal llf;
			EXPECT_THROW(iTable.SetAccessFlags(m_db, AF_READ), AssertionException);
		}

		// But when we close first we can
		EXPECT_NO_THROW(iTable.Close());
		EXPECT_NO_THROW(iTable.SetAccessFlags(m_db, AF_READ));
		// And we can open it again
		EXPECT_NO_THROW(iTable.Open(m_db));
	}


	TEST_P(TableTest, IsQueryOnly)
	{
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(m_db, tableName, L"", L"", L"", AF_READ);
		
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(m_db, AF_UPDATE);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(m_db, AF_UPDATE);
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(m_db, AF_INSERT);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(m_db, AF_INSERT);
		EXPECT_TRUE(iTable.IsQueryOnly());

		iTable.SetAccessFlag(m_db, AF_DELETE);
		EXPECT_FALSE(iTable.IsQueryOnly());
		iTable.ClearAccessFlag(m_db, AF_DELETE);
		EXPECT_TRUE(iTable.IsQueryOnly());

		// and one that is initially rw
		exodbc::Table iTable2(m_db, tableName, L"", L"", L"", AF_READ_WRITE);
		EXPECT_FALSE(iTable2.IsQueryOnly());
		iTable2.ClearAccessFlag(m_db, AF_INSERT);
		EXPECT_FALSE(iTable2.IsQueryOnly());

		iTable2.ClearAccessFlag(m_db, AF_DELETE);
		EXPECT_FALSE(iTable2.IsQueryOnly());

		iTable2.ClearAccessFlag(m_db, AF_UPDATE);
		EXPECT_TRUE(iTable2.IsQueryOnly());

		// remove read, we are no longer query only
		iTable2.ClearAccessFlag(m_db, AF_SELECT);
		EXPECT_FALSE(iTable2.IsQueryOnly());
	}


	TEST_P(TableTest, MissingAccessFlagsThrowOnWrite)
	{
		// Open a read-only table
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(m_db, tableName, L"", L"", L"", AF_READ);

		ASSERT_NO_THROW(iTable.Open(m_db));

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
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(m_db, tableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(iTable.Open(m_db));

		EXPECT_NO_THROW(iTable.Select(L""));

		iTable.SelectClose();
	}


	TEST_P(TableTest, SelectNext)
	{
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(m_db, tableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(iTable.Open(m_db));

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
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(m_db, tableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(iTable.Open(m_db));
		
		// Do something that opens a transaction
		iTable.Select(L"");
		EXPECT_NO_THROW(iTable.SelectClose());
		// We should be closed now
		EXPECT_FALSE(iTable.IsSelectOpen());
	}


	TEST_P(TableTest, SelectHasMASEnabled)
	{
		// Test what happens if we Select from a table 2 records, then select the first record,
		// then delete the second record
		// and then try to select the second record using the still open select.

		std::wstring intTypesTableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);

		Table iTable(m_db, intTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(iTable.Open(m_db));
		ColumnBuffer* pId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt = iTable.GetColumnBuffer(1);
		ColumnBuffer* pInt = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBigInt = iTable.GetColumnBuffer(3);

		// Clear Table
		ASSERT_NO_THROW(TestTables::ClearTestTable(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase, iTable, m_db));

		// Set some silly values to insert
		*pId = (SQLINTEGER)101;
		*pSmallInt = (SQLSMALLINT)1;
		*pInt = (SQLINTEGER)10;
		*pBigInt = (SQLBIGINT)100;
		iTable.Insert();
		*pId = (SQLINTEGER)102;
		*pSmallInt = (SQLSMALLINT)2;
		*pInt = (SQLINTEGER)20;
		*pBigInt = (SQLBIGINT)200;
		iTable.Insert();
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Now select those two records
		wstring sqlstmt = (boost::wformat(L"%s = 101 OR %s = 102 ORDER BY %s") % idName %idName %idName).str();
		ASSERT_NO_THROW(iTable.Select(sqlstmt));
		ASSERT_TRUE(iTable.SelectNext());
		ASSERT_EQ(101, (SQLINTEGER)*pId);
		ASSERT_EQ(1, (SQLSMALLINT)*pSmallInt);
		ASSERT_EQ(10, (SQLINTEGER)*pInt);
		ASSERT_EQ(100, (SQLBIGINT)*pBigInt);
		//ASSERT_TRUE(iTable.SelectClose());

		// Now delete the second record: We cannot do that if we do not have support for What are Multiple Active Statements (MAS)?
		// MS SQL Server does not have this enabled by default
		// See Ticket # 63 and # 75
		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			LOG_WARNING(L"This test is known to fail with Microsoft SQL Server 2014 (and probably others too), see Ticket #75");
		}
		sqlstmt = (boost::wformat(L"%s = 102") % idName).str();
		ASSERT_NO_THROW(iTable.Delete(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());

		// If MAS is enabled, we should be able to still see the result of the just deleted record, even if the result was committed inbetween
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(102, (SQLINTEGER)*pId);
		EXPECT_EQ(2, (SQLSMALLINT)*pSmallInt);
		EXPECT_EQ(20, (SQLINTEGER)*pInt);
		EXPECT_EQ(200, (SQLBIGINT)*pBigInt);

		// But if we try to select it again now, we will find only one result
		iTable.SelectClose();
		sqlstmt = (boost::wformat(L"%s = 101 OR %s = 102 ORDER BY %s") % idName %idName %idName).str();
		iTable.Select(sqlstmt);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_FALSE(iTable.SelectNext());
	}


	// Count
	// -----
	TEST_P(TableTest, Count)
	{
		MFloatTypesTable table(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(table.Open(m_db));

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
		std::wstring tableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(m_db, tableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(iTable.Open(m_db));

		// Test values
		SQLSMALLINT s = 0;
		SQLINTEGER i = 0;
		SQLBIGINT b = 0;
		std::wstring str;

		// We expect 6 Records
		std::wstring idName = TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		iTable.Select((boost::wformat(L"%s = 1") % idName).str());
		// The first column has a smallint set, we can read that as any int value -32768
		SQLBIGINT colVal = -32768;
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(iTable.GetColumnValue(1, s));
		EXPECT_NO_THROW(iTable.GetColumnValue(1, i));
		EXPECT_NO_THROW(iTable.GetColumnValue(1, b));
		EXPECT_EQ(colVal, s);
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		// Read as str
		EXPECT_NO_THROW(iTable.GetColumnValue(1, str));
		EXPECT_EQ(L"-32768", str);
		EXPECT_FALSE(iTable.SelectNext());
		iTable.SelectClose();

		iTable.Select((boost::wformat(L"%s = 2") % idName).str());
		colVal = 32767;
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(iTable.GetColumnValue(1, s));
		EXPECT_NO_THROW(iTable.GetColumnValue(1, i));
		EXPECT_NO_THROW(iTable.GetColumnValue(1, b));
		EXPECT_EQ(colVal, s);
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		// Read as str
		EXPECT_NO_THROW(iTable.GetColumnValue(1, str));
		EXPECT_EQ(L"32767", str);
		EXPECT_FALSE(iTable.SelectNext());
		iTable.SelectClose();

		iTable.Select((boost::wformat(L"%s = 3") % idName).str());
		// The 2nd column has a int set, we can read that as int or bigint value -2147483648
		colVal = INT_MIN;
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_THROW(iTable.GetColumnValue(2, s), CastException);
		EXPECT_NO_THROW(iTable.GetColumnValue(2, i));
		EXPECT_NO_THROW(iTable.GetColumnValue(2, b));
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		// Read as str
		EXPECT_NO_THROW(iTable.GetColumnValue(2, str));
		EXPECT_EQ(L"-2147483648", str);
		EXPECT_FALSE(iTable.SelectNext());
		iTable.SelectClose();

		iTable.Select((boost::wformat(L"%s = 4") % idName).str());
		colVal = 2147483647;
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_THROW(iTable.GetColumnValue(2, s), CastException);
		EXPECT_NO_THROW(iTable.GetColumnValue(2, i));
		EXPECT_NO_THROW(iTable.GetColumnValue(2, b));
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		EXPECT_NO_THROW(iTable.GetColumnValue(2, str));
		EXPECT_EQ(L"2147483647", str);
		EXPECT_FALSE(iTable.SelectNext());
		iTable.SelectClose();

		iTable.Select((boost::wformat(L"%s = 5") % idName).str());
		// The 3rd column has a bigint set, we can read that as bigint value -9223372036854775808
		colVal = (-9223372036854775807 - 1);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_THROW(iTable.GetColumnValue(3, s), CastException);
		EXPECT_THROW(iTable.GetColumnValue(3, i), CastException);
		EXPECT_NO_THROW(iTable.GetColumnValue(3, b));
		EXPECT_EQ(colVal, b);
		EXPECT_NO_THROW(iTable.GetColumnValue(3, str));
		EXPECT_EQ(L"-9223372036854775808", str);
		EXPECT_FALSE(iTable.SelectNext());
		iTable.SelectClose();

		iTable.Select((boost::wformat(L"%s = 6") % idName).str());
		colVal = 9223372036854775807;
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_THROW(iTable.GetColumnValue(3, s), CastException);
		EXPECT_THROW(iTable.GetColumnValue(3, i), CastException);
		EXPECT_NO_THROW(iTable.GetColumnValue(3, b));
		EXPECT_EQ(colVal, b);
		EXPECT_NO_THROW(iTable.GetColumnValue(3, str));
		EXPECT_EQ(L"9223372036854775807", str);
		EXPECT_FALSE(iTable.SelectNext());
		iTable.SelectClose();
	}


	TEST_P(TableTest, SelectManualIntValues)
	{
		MIntTypesTable iTable(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(iTable.Open(m_db));

		// Just check that the buffers are correct
		std::wstring idName = TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
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

		iTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ((-9223372036854775807 - 1), iTable.m_bigInt);

		iTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(9223372036854775807, iTable.m_bigInt);
	}


	TEST_P(TableTest, SelectWCharIntValues)
	{
		// And some tables with auto-columns
		std::wstring intTypesTableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		Table iTable(m_db, intTypesTableName, L"", L"", L"", AF_READ);
		iTable.SetAutoBindingMode(AutoBindingMode::BIND_ALL_AS_WCHAR);
		ASSERT_NO_THROW(iTable.Open(m_db));

		std::wstring id, smallInt, i, bigInt;
		std::wstring idName = TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase);
		iTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(iTable.GetColumnValue(1, smallInt));
		EXPECT_EQ(L"-32768", smallInt);

		iTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(iTable.GetColumnValue(1, smallInt));
		EXPECT_EQ(L"32767", smallInt);

		iTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(iTable.GetColumnValue(2, i));
		EXPECT_EQ(L"-2147483648", i);

		iTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(iTable.GetColumnValue(2, i));
		EXPECT_EQ(L"2147483647", i);

		iTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(iTable.GetColumnValue(3, bigInt));
		EXPECT_EQ(L"-9223372036854775808", bigInt);

		iTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_NO_THROW(iTable.GetColumnValue(3, bigInt));
		EXPECT_EQ(L"9223372036854775807", bigInt);
	}


	TEST_P(TableTest, SelectAutoFloatValues)
	{
		wstring floatTypesTableName = TestTables::GetTableName(TestTables::Table::FLOATTYPES, m_odbcInfo.m_namesCase);
		Table fTable(m_db, floatTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(fTable.Open(m_db));

		SQLDOUBLE val;
		wstring str;
		std::wstring idName = TestTables::GetIdColumnName(TestTables::Table::FLOATTYPES, m_odbcInfo.m_namesCase);
		fTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(2, val));
		EXPECT_EQ(0.0, val);
		// Read as str
		EXPECT_NO_THROW(fTable.GetColumnValue(2, str));
		EXPECT_EQ(L"0", str);

		fTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(2, val));
		EXPECT_EQ(3.141, val);
		// Read as str
		EXPECT_NO_THROW(fTable.GetColumnValue(2, str));
		EXPECT_EQ(L"3.141", str);

		fTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(2, val));
		EXPECT_EQ(-3.141, val);
		// Read as str
		EXPECT_NO_THROW(fTable.GetColumnValue(2, str));
		EXPECT_EQ(L"-3.141", str);

		fTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(1, val));
		EXPECT_EQ(0.0, val);
		// Read as str
		EXPECT_NO_THROW(fTable.GetColumnValue(1, str));
		EXPECT_EQ(L"0", str);

		fTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(1, val));
		EXPECT_EQ(3.141592, val);
		// Read as str
		EXPECT_NO_THROW(fTable.GetColumnValue(1, str));
		EXPECT_EQ(L"3.1415920000000002", str);

		fTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(1, val));
		EXPECT_EQ(-3.141592, val);
		// Read as str
		EXPECT_NO_THROW(fTable.GetColumnValue(1, str));
		EXPECT_EQ(L"-3.1415920000000002", str);
	}


	TEST_P(TableTest, SelectManualFloatValues)
	{
		MFloatTypesTable fTable(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(fTable.Open(m_db));

		std::wstring idName = TestTables::GetIdColumnName(TestTables::Table::FLOATTYPES, m_odbcInfo.m_namesCase);
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
		wstring floatTypesTableName = TestTables::GetTableName(TestTables::Table::FLOATTYPES, m_odbcInfo.m_namesCase);
		Table fTable(m_db, floatTypesTableName, L"", L"", L"", AF_READ);
		fTable.SetAutoBindingMode(AutoBindingMode::BIND_ALL_AS_WCHAR);
		ASSERT_NO_THROW(fTable.Open(m_db));

		wstring sVal;
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::FLOATTYPES, m_odbcInfo.m_namesCase);
		fTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(2, sVal));
		//EXPECT_EQ(0.0, sVal);
		
		// TODO: Find a way to compare without having to diff for every db-type

		fTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(2, sVal));

		fTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(2, sVal));

		fTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(1, sVal));
		
		fTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(1, sVal));

		fTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_NO_THROW(fTable.GetColumnValue(1, sVal));

	}


	TEST_P(TableTest, SelectAutoWCharValues)
	{
		std::wstring charTypesTableName = TestTables::GetTableName(TestTables::Table::CHARTYPES, m_odbcInfo.m_namesCase);
		Table charTypesAutoTable(m_db, charTypesTableName, L"", L"", L"", AF_READ);
		charTypesAutoTable.SetAutoBindingMode(AutoBindingMode::BIND_CHAR_AS_WCHAR);
		ASSERT_NO_THROW(charTypesAutoTable.Open(m_db));
		// We want to trim on the right side for DB2 and sql server
		charTypesAutoTable.SetCharTrimRight(true);

		std::wstring str;

		// We expect 6 Records
		std::wstring idName = TestTables::GetIdColumnName(TestTables::Table::CHARTYPES, m_odbcInfo.m_namesCase);
		charTypesAutoTable.Select((boost::wformat(L"%d = 1") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(1, str));
		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);
		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 2") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(2, str));
		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 3") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(1, str));
		EXPECT_EQ(std::wstring(L"הצüאיט"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 4") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(2, str));
		EXPECT_EQ(std::wstring(L"הצüאיט"), str);

		charTypesAutoTable.Select((boost::wformat(L"%d = 5") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(3, str));
		EXPECT_EQ(std::wstring(L"abc"), str);
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(4, str));
		EXPECT_EQ(std::wstring(L"abc"), str);

		charTypesAutoTable.Select((boost::wformat(L"%d = 6") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(3, str));
		EXPECT_EQ(std::wstring(L"abcde12345"), str);
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(4, str));
		EXPECT_EQ(std::wstring(L"abcde12345"), str);

		charTypesAutoTable.SelectClose();
	}


	TEST_P(TableTest, SelectManualWCharValues)
	{
		MWCharTypesTable wTable(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(wTable.Open(m_db));

		// Just test the values in the buffer - trimming has no effect here
		using namespace boost::algorithm;
		std::wstring str;
		std::wstring idName = TestTables::GetIdColumnName(TestTables::Table::CHARTYPES, m_odbcInfo.m_namesCase);
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
		std::wstring charTypesTableName = TestTables::GetTableName(TestTables::Table::CHARTYPES, m_odbcInfo.m_namesCase);
		Table charTypesAutoTable(m_db, charTypesTableName, L"", L"", L"", AF_READ);
		charTypesAutoTable.SetAutoBindingMode(AutoBindingMode::BIND_WCHAR_AS_CHAR);
		ASSERT_NO_THROW(charTypesAutoTable.Open(m_db));
		// We want to trim on the right side for DB2 and also sql server
		charTypesAutoTable.SetCharTrimRight(true);

		std::string str;

		std::wstring idName = TestTables::GetIdColumnName(TestTables::Table::CHARTYPES, m_odbcInfo.m_namesCase);
		charTypesAutoTable.Select((boost::wformat(L"%d = 1") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(1, str));
		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);
		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 2") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(2, str));
		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 3") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(1, str));
		EXPECT_EQ(std::string("הצüאיט"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		charTypesAutoTable.SelectClose();

		charTypesAutoTable.Select((boost::wformat(L"%d = 4") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(2, str));
		EXPECT_EQ(std::string("הצüאיט"), str);

		charTypesAutoTable.Select((boost::wformat(L"%d = 5") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(3, str));
		EXPECT_EQ(std::string("abc"), str);
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(4, str));
		EXPECT_EQ(std::string("abc"), str);

		charTypesAutoTable.Select((boost::wformat(L"%d = 6") % idName).str());
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(3, str));
		EXPECT_EQ(std::string("abcde12345"), str);
		EXPECT_NO_THROW(charTypesAutoTable.GetColumnValue(4, str));
		EXPECT_EQ(std::string("abcde12345"), str);

		charTypesAutoTable.SelectClose();
	}


	TEST_P(TableTest, SelectManualCharValues)
	{
		MCharTypesTable cTable(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(cTable.Open(m_db));

		// Just test the values in the buffer - trimming has no effect here
		using namespace boost::algorithm;
		std::string str;
		std::wstring idName = TestTables::GetIdColumnName(TestTables::Table::CHARTYPES, m_odbcInfo.m_namesCase);
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

		std::wstring dateTypesTableName = TestTables::GetTableName(TestTables::Table::DATETYPES, m_odbcInfo.m_namesCase);
		Table dTable(m_db, dateTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(dTable.Open(m_db));

		using namespace boost::algorithm;
		std::wstring str;
		SQL_DATE_STRUCT date;
		SQL_TIME_STRUCT time;
		SQL_TIMESTAMP_STRUCT timestamp;
		std::wstring idName = TestTables::GetIdColumnName(TestTables::Table::DATETYPES, m_odbcInfo.m_namesCase);
		dTable.Select((boost::wformat(L"%s = 1") % idName).str());
		{
			// Microsoft will info here (which we report as warning) that the time field has been truncated (as we do not use the fancy TIME2 struct)
			LogLevelError llE;
			EXPECT_TRUE(dTable.SelectNext());
		}
		EXPECT_NO_THROW(dTable.GetColumnValue(1, date));
		EXPECT_NO_THROW(dTable.GetColumnValue(2, time));
		EXPECT_NO_THROW(dTable.GetColumnValue(3, timestamp));

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
		EXPECT_NO_THROW(dTable.GetColumnValue(3, timestamp));
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
		EXPECT_NO_THROW(dTable.GetColumnValue(3, timestamp));
		if (m_db.GetDbms() != DatabaseProduct::MY_SQL)
		{
			EXPECT_EQ(10000000, timestamp.fraction);
		}


		// MS SQL Server has a new SQL_TIME2_STRUCT if ODBC version is >= 3.8
#if HAVE_MSODBCSQL_H
		Environment env38(OdbcVersion::V_3_8);
		EXPECT_TRUE(env38.HasEnvironmentHandle());
		Database db38(env38);
		EXPECT_TRUE(db38.HasConnectionHandle());
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			// My SQL does not support 3.8, the database will warn about a version-mismatch and fall back to 3.0. we know about that.
			// \todo: Open a ticket
			LogLevelError llE;
			EXPECT_NO_THROW(db38.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
		else
		{
			EXPECT_NO_THROW(db38.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
		Table dTable38(db38, dateTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(dTable38.Open(db38));
		dTable38.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(dTable38.SelectNext());
		EXPECT_NO_THROW(dTable38.GetColumnValue(2, time));
		EXPECT_EQ(13, time.hour);
		EXPECT_EQ(55, time.minute);
		EXPECT_EQ(56, time.second);

		// We should be able to read the value as TIME2 struct
		SQL_SS_TIME2_STRUCT t2;
		EXPECT_NO_THROW(dTable38.GetColumnValue(2, t2));
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
		MDateTypesTable cTable(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(cTable.Open(m_db));

		// Just test the values in the buffer - trimming has no effect here
		using namespace boost::algorithm;
		std::string str;
		std::wstring idName = TestTables::GetIdColumnName(TestTables::Table::DATETYPES, m_odbcInfo.m_namesCase);
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
		std::wstring dateTypesTableName = TestTables::GetTableName(TestTables::Table::DATETYPES, m_odbcInfo.m_namesCase);
		Table dTable(m_db, dateTypesTableName, L"", L"", L"", AF_READ);
		dTable.SetAutoBindingMode(AutoBindingMode::BIND_ALL_AS_WCHAR);
		ASSERT_NO_THROW(dTable.Open(m_db));

		wstring sDate, sTime, sTimestamp;
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::DATETYPES, m_odbcInfo.m_namesCase);
		dTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(dTable.SelectNext());
		EXPECT_NO_THROW(dTable.GetColumnValue(1, sDate));
		EXPECT_EQ(L"1983-01-26", sDate);

		EXPECT_NO_THROW(dTable.GetColumnValue(2, sTime));
		EXPECT_NO_THROW(dTable.GetColumnValue(3, sTimestamp));

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
			EXPECT_EQ(L"13:55:56", sTime);
			EXPECT_EQ(L"1983-01-26 13:55:56", sTimestamp);
		}

	}


	TEST_P(TableTest, SelectAutoBlobValues)
	{
		std::wstring blobTypesTableName = TestTables::GetTableName(TestTables::Table::BLOBTYPES, m_odbcInfo.m_namesCase);
		Table bTable(m_db, blobTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(bTable.Open(m_db));

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::BLOBTYPES, m_odbcInfo.m_namesCase);

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
		SQLINTEGER size, length = 0;

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
		MBlobTypesTable bTable(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(bTable.Open(m_db));

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::BLOBTYPES, m_odbcInfo.m_namesCase);

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
		std::wstring numericTypesTableName = TestTables::GetTableName(TestTables::Table::NUMERICTYPES, m_odbcInfo.m_namesCase);
		Table nTable(m_db, numericTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(nTable.Open(m_db));

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NUMERICTYPES, m_odbcInfo.m_namesCase);
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
		std::wstring numericTypesTableName = TestTables::GetTableName(TestTables::Table::NUMERICTYPES, m_odbcInfo.m_namesCase);
		Table nTable(m_db, numericTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(nTable.Open(m_db));

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NUMERICTYPES, m_odbcInfo.m_namesCase);
		SQL_NUMERIC_STRUCT numStr;
		const ColumnBuffer* pColBuff18_00 = nTable.GetColumnBuffer(1);
		const ColumnBuffer* pColBuff18_10 = nTable.GetColumnBuffer(2);
		const ColumnBuffer* pColBuff5_3 = nTable.GetColumnBuffer(3);
		SQLBIGINT ex;
		SQLBIGINT* p;

		nTable.Select((boost::wformat(L"%s = 1") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(nTable.GetColumnValue(1, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(0, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 0;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(nTable.GetColumnValue(1, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(0, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);
		// Col 3 has a value here too.
		EXPECT_NO_THROW(nTable.GetColumnValue(3, numStr));
		EXPECT_EQ(5, numStr.precision);
		EXPECT_EQ(3, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 12345;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(nTable.GetColumnValue(1, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(0, numStr.scale);
		EXPECT_EQ(0, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 4") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(nTable.GetColumnValue(2, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(10, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 0;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(nTable.GetColumnValue(2, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(10, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		nTable.Select((boost::wformat(L"%s = 6") % idName).str());
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_NO_THROW(nTable.GetColumnValue(2, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(10, numStr.scale);
		EXPECT_EQ(0, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);
	}


	TEST_P(TableTest, SelectManualNumericValue)
	{
		MNumericTypesTable nTable(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(nTable.Open(m_db));

		SQLBIGINT ex;
		SQLBIGINT* p;
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NUMERICTYPES, m_odbcInfo.m_namesCase);
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
		std::wstring numericTypesTableName = TestTables::GetTableName(TestTables::Table::NUMERICTYPES, m_odbcInfo.m_namesCase);
		Table nTable(m_db, numericTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(nTable.Open(m_db));

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NUMERICTYPES, m_odbcInfo.m_namesCase);
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
		std::wstring intTypesTableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(m_db, intTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(iTable.Open(m_db));

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Note: We could also do this in one transaction.
		// \todo: Write a separate transaction test about this, to check transaction-visiblity
		// Try to delete eventually available leftovers, ignore if none exists
		ASSERT_NO_THROW(iTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Now lets insert some data:
		ColumnBuffer* pId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt = iTable.GetColumnBuffer(1);
		ColumnBuffer* pInt = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBigInt = iTable.GetColumnBuffer(3);

		*pId = (SQLINTEGER)103;
		pSmallInt->SetNull();
		pInt->SetNull();
		pBigInt->SetNull();
		iTable.Insert();
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Now we must have something to delete
		EXPECT_NO_THROW(iTable.Delete(sqlstmt, true));
		EXPECT_NO_THROW(m_db.CommitTrans());

		// And fetching shall return no results at all
		iTable.Select();
		EXPECT_FALSE(iTable.SelectNext());
	}


	TEST_P(TableTest, DeleteWhereShouldReturnSQL_NO_DATA)
	{
		std::wstring intTypesTableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(m_db, intTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(iTable.Open(m_db));

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Try to delete eventually available leftovers, ignore if none exists
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #77");
		}
		ASSERT_NO_THROW(iTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());
		// We can be sure now that nothing exists, and we will fail if we try to delete
		EXPECT_THROW(iTable.Delete(sqlstmt, true), SqlResultException);
		EXPECT_NO_THROW(m_db.CommitTrans());

		// And fetching shall return no results at all
		iTable.Select();
		EXPECT_FALSE(iTable.SelectNext());
	}


	TEST_P(TableTest, Delete)
	{
		std::wstring intTypesTableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(m_db, intTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(iTable.Open(m_db));

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(iTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert a row that we want to delete
		ColumnBuffer* pId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt = iTable.GetColumnBuffer(1);
		ColumnBuffer* pInt = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBigInt = iTable.GetColumnBuffer(3);
		pSmallInt->SetNull();
		pInt->SetNull();
		pBigInt->SetNull();
		*pId = (SQLINTEGER)99;
		iTable.Insert();
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Now lets delete that row. our pId is still set to the key-value 99
		EXPECT_NO_THROW(iTable.Delete());

		// And fetching shall return no results at all
		iTable.Select();
		EXPECT_FALSE(iTable.SelectNext());
	}


	// Insert rows
	// -----------
	TEST_P(TableTest, InsertIntTypes)
	{
		std::wstring intTypesTableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(m_db, intTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(iTable.Open(m_db));
		ColumnBuffer* pId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt = iTable.GetColumnBuffer(1);
		ColumnBuffer* pInt = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBigInt = iTable.GetColumnBuffer(3);

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(iTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Set some silly values to insert
		*pId = (SQLINTEGER)101;
		*pSmallInt = (SQLSMALLINT)102;
		*pInt = (SQLINTEGER)103;
		*pBigInt = (SQLBIGINT)104;

		EXPECT_NO_THROW(iTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Open another table and read the values from there
		Table iTable2(m_db, intTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(iTable2.Open(m_db));
		ColumnBuffer* pId2 = iTable2.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt2 = iTable2.GetColumnBuffer(1);
		ColumnBuffer* pInt2 = iTable2.GetColumnBuffer(2);
		ColumnBuffer* pBigInt2 = iTable2.GetColumnBuffer(3);

		iTable2.Select();
		EXPECT_TRUE(iTable2.SelectNext());

		EXPECT_EQ(101, (SQLINTEGER)*pId2);
		EXPECT_EQ(102, (SQLSMALLINT)*pSmallInt2);
		EXPECT_EQ(103, (SQLINTEGER)*pInt2);
		EXPECT_EQ(104, (SQLBIGINT)*pBigInt2);
	}


	// just do only one test to insert the manual types, its all the same, except that opening is a little bit different
	TEST_P(TableTest, InsertManualIntTypes)
	{
		// Open an existing table without checking for privileges or existence
		Table iTable(m_db, 4, TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_SELECT | AF_DELETE | AF_INSERT);
		SQLINTEGER id = 0;
		SQLSMALLINT si = 0;
		SQLINTEGER i = 0;
		SQLBIGINT bi = 0;
		int type = SQL_C_SLONG;
		iTable.SetColumn(0, TestTables::ConvertNameCase(L"idintegertypes", m_odbcInfo.m_namesCase), SQL_INTEGER, &id, SQL_C_SLONG, sizeof(id), CF_SELECT | CF_INSERT);
		iTable.SetColumn(1, TestTables::ConvertNameCase(L"tsmallint", m_odbcInfo.m_namesCase), SQL_INTEGER, &si, SQL_C_SSHORT, sizeof(si), CF_SELECT | CF_INSERT);
		iTable.SetColumn(2, TestTables::ConvertNameCase(L"tint", m_odbcInfo.m_namesCase), SQL_INTEGER, &i, SQL_C_SLONG, sizeof(i), CF_SELECT | CF_INSERT);
		iTable.SetColumn(3, TestTables::ConvertNameCase(L"tbigint", m_odbcInfo.m_namesCase), SQL_BIGINT, &bi, SQL_C_SBIGINT, sizeof(bi), CF_SELECT | CF_INSERT);

		iTable.Open(m_db);
		TestTables::ClearTestTable(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase, iTable, m_db);
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		iTable.Delete(sqlstmt, false);
		m_db.CommitTrans();

		// Set some silly values to insert
		// note: no need to set to not-null, the buffers are created with null set to false
		id = 101;
		si = 102;
		i = 103;
		bi = 104;

		EXPECT_NO_THROW(iTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Open another table and read the values from there
		Table iTable2(m_db, TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"", AF_READ);
		iTable2.Open(m_db);
		iTable2.Select();
		ASSERT_TRUE(iTable2.SelectNext());

		EXPECT_EQ(101, boost::get<SQLINTEGER>(iTable2.GetColumnValue(0)));
		EXPECT_EQ(102, boost::get<SQLSMALLINT>(iTable2.GetColumnValue(1)));
		EXPECT_EQ(103, boost::get<SQLINTEGER>(iTable2.GetColumnValue(2)));
		EXPECT_EQ(104, boost::get<SQLBIGINT>(iTable2.GetColumnValue(3)));

	}


	TEST_P(TableTest, InsertDateTypes)
	{
		std::wstring dateTypesTableName = TestTables::GetTableName(TestTables::Table::DATETYPES_TMP, m_odbcInfo.m_namesCase);
		Table dTable(m_db, dateTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(dTable.Open(m_db));
		ColumnBuffer* pId = dTable.GetColumnBuffer(0);
		ColumnBuffer* pDate = dTable.GetColumnBuffer(1);
		ColumnBuffer* pTime = dTable.GetColumnBuffer(2);
		ColumnBuffer* pTimestamp = dTable.GetColumnBuffer(3);

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::DATETYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(dTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

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
		if (m_db.GetDbms() != DatabaseProduct::MY_SQL)
		{
			timestamp.fraction = 123000000;
		}
		else
		{
			timestamp.fraction = 0;
		}

		*pId = (SQLINTEGER)101;
		*pDate = date;
		*pTime = time;
		*pTimestamp = timestamp;
		EXPECT_NO_THROW(dTable.Insert());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Open another table and read the values from there
		Table dTable2(m_db, dateTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(dTable2.Open(m_db));
		ColumnBuffer* pId2 = dTable2.GetColumnBuffer(0);
		ColumnBuffer* pDate2 = dTable2.GetColumnBuffer(1);
		ColumnBuffer* pTime2 = dTable2.GetColumnBuffer(2);
		ColumnBuffer* pTimestamp2 = dTable2.GetColumnBuffer(3);

		dTable2.Select();
		EXPECT_TRUE(dTable2.SelectNext());
		SQL_DATE_STRUCT date2 = *pDate2;
		SQL_TIME_STRUCT time2 = *pTime2;
		SQL_TIMESTAMP_STRUCT timestamp2 = *pTimestamp2;

		EXPECT_EQ(101, (SQLINTEGER)*pId2);
		EXPECT_EQ(date.year, date2.year);
		EXPECT_EQ(date.month, date2.month);
		EXPECT_EQ(date.day, date2.day);

		EXPECT_EQ(time.hour, time2.hour);
		EXPECT_EQ(time.minute, time2.minute);
		EXPECT_EQ(time.second, time2.second);

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
		std::wstring floatTypesTableName = TestTables::GetTableName(TestTables::Table::FLOATTYPES_TMP, m_odbcInfo.m_namesCase);
		Table fTable(m_db, floatTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(fTable.Open(m_db));
		ColumnBuffer* pId = fTable.GetColumnBuffer(0);
		ColumnBuffer* pDouble = fTable.GetColumnBuffer(1);
		ColumnBuffer* pFloat = fTable.GetColumnBuffer(2);

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::FLOATTYPES_TMP, m_odbcInfo.m_namesCase);
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
		Table fTable2(m_db, floatTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(fTable2.Open(m_db));
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
		std::wstring numericTypesTmpTableName = TestTables::GetTableName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table nTable(m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(nTable.Open(m_db));
		ColumnBuffer* pId = nTable.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = nTable.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = nTable.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = nTable.GetColumnBuffer(3);

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Select a valid record from the non-tmp table
		std::wstring numericTypesTableName = TestTables::GetTableName(TestTables::Table::NUMERICTYPES, m_odbcInfo.m_namesCase);
		Table nnTable(m_db, numericTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(nnTable.Open(m_db));
		SQL_NUMERIC_STRUCT numStr18_0, numStr18_10, numStr5_3;
		nnTable.Select((boost::wformat(L"%s = 2") % idName).str());
		EXPECT_TRUE(nnTable.SelectNext());
		EXPECT_NO_THROW(nnTable.GetColumnValue(1, numStr18_0));
		EXPECT_NO_THROW(nnTable.GetColumnValue(3, numStr5_3));
		nnTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(nnTable.SelectNext());
		EXPECT_NO_THROW(nnTable.GetColumnValue(2, numStr18_10));
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
		Table nntTable(m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(nntTable.Open(m_db));
		sqlstmt = (boost::wformat(L"%s = %d") % idName % (SQLINTEGER)*pId).str();
		nntTable.Select(sqlstmt);
		EXPECT_TRUE(nntTable.SelectNext());
		EXPECT_NO_THROW(nntTable.GetColumnValue(1, numStr18_0t));
		EXPECT_NO_THROW(nntTable.GetColumnValue(2, numStr18_10t));
		EXPECT_NO_THROW(nntTable.GetColumnValue(3, numStr5_3t));
		nntTable.SelectClose();
		EXPECT_EQ(0, memcmp(&numStr18_0, &numStr18_0t, sizeof(numStr18_0)));
		EXPECT_EQ(0, memcmp(&numStr18_10, &numStr18_10t, sizeof(numStr18_10)));
		EXPECT_EQ(0, memcmp(&numStr5_3, &numStr5_3t, sizeof(numStr5_3)));
	}


	TEST_P(TableTest, InsertNumericTypes_5_3)
	{
		std::wstring numericTypesTmpTableName = TestTables::GetTableName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table t(m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);

		ASSERT_NO_THROW(t.Open(m_db));
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
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
		std::wstring numericTypesTmpTableName = TestTables::GetTableName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table t(m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);

		ASSERT_NO_THROW(t.Open(m_db));
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
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
		std::wstring numericTypesTmpTableName = TestTables::GetTableName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table t(m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);

		ASSERT_NO_THROW(t.Open(m_db));
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
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
		std::wstring numericTypesTmpTableName = TestTables::GetTableName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table t(m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);

		ASSERT_NO_THROW(t.Open(m_db));
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
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
		std::wstring blobTypesTmpTableName = TestTables::GetTableName(TestTables::Table::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
		Table bTable(m_db, blobTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(bTable.Open(m_db));

		ColumnBuffer* pId = bTable.GetColumnBuffer(0);
		ColumnBuffer* pBlob = bTable.GetColumnBuffer(1);
		ColumnBuffer* pVarBlob_20 = bTable.GetColumnBuffer(2);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(bTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

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
		std::wstring charTypesTmpTableName = TestTables::GetTableName(TestTables::Table::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		Table cTable(m_db, charTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		cTable.SetAutoBindingMode(AutoBindingMode::BIND_WCHAR_AS_CHAR);
		ASSERT_NO_THROW(cTable.Open(m_db));

		ColumnBuffer* pId = cTable.GetColumnBuffer(0);
		ColumnBuffer* pVarchar = cTable.GetColumnBuffer(1);
		ColumnBuffer* pChar = cTable.GetColumnBuffer(2);
		ColumnBuffer* pVarchar_10 = cTable.GetColumnBuffer(3);
		ColumnBuffer* pChar_10 = cTable.GetColumnBuffer(4);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(cTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

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
		Table t2(m_db, charTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		t2.SetAutoBindingMode(AutoBindingMode::BIND_WCHAR_AS_CHAR);
		ASSERT_NO_THROW(t2.Open(m_db));
		t2.SetCharTrimRight(true);

		std::string varchar2, char2;
		t2.Select((boost::wformat(L"%s = 100") % idName).str());
		EXPECT_TRUE(t2.SelectNext());
		EXPECT_NO_THROW(t2.GetColumnValue(1, varchar2));
		EXPECT_NO_THROW(t2.GetColumnValue(2, char2));
		EXPECT_TRUE(t2.IsColumnNull(3));
		EXPECT_TRUE(t2.IsColumnNull(4));
		EXPECT_EQ(s, varchar2);
		EXPECT_EQ(s, char2);

		s = "abcde12345";
		t2.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(t2.SelectNext());
		EXPECT_TRUE(t2.IsColumnNull(1));
		EXPECT_TRUE(t2.IsColumnNull(2));
		EXPECT_NO_THROW(t2.GetColumnValue(3, varchar2));
		EXPECT_NO_THROW(t2.GetColumnValue(4, char2));
		EXPECT_EQ(s, varchar2);
		EXPECT_EQ(s, char2);
	}


	TEST_P(TableTest, InsertWCharTypes)
	{
		std::wstring charTypesTmpTableName = TestTables::GetTableName(TestTables::Table::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		Table cTable(m_db, charTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		cTable.SetAutoBindingMode(AutoBindingMode::BIND_CHAR_AS_WCHAR);
		ASSERT_NO_THROW(cTable.Open(m_db));

		ColumnBuffer* pId = cTable.GetColumnBuffer(0);
		ColumnBuffer* pVarchar = cTable.GetColumnBuffer(1);
		ColumnBuffer* pChar = cTable.GetColumnBuffer(2);
		ColumnBuffer* pVarchar_10 = cTable.GetColumnBuffer(3);
		ColumnBuffer* pChar_10 = cTable.GetColumnBuffer(4);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(cTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

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
		Table t2(m_db, charTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		t2.SetAutoBindingMode(AutoBindingMode::BIND_CHAR_AS_WCHAR);
		ASSERT_NO_THROW(t2.Open(m_db));
		t2.SetCharTrimRight(true);

		std::wstring varchar2, char2;
		t2.Select((boost::wformat(L"%s = 100") % idName).str());
		EXPECT_TRUE(t2.SelectNext());
		EXPECT_NO_THROW(t2.GetColumnValue(1, varchar2));
		EXPECT_NO_THROW(t2.GetColumnValue(2, char2));
		EXPECT_TRUE(t2.IsColumnNull(3));
		EXPECT_TRUE(t2.IsColumnNull(4));
		EXPECT_EQ(s, varchar2);
		EXPECT_EQ(s, char2);

		s = L"abcde12345";
		t2.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(t2.SelectNext());
		EXPECT_TRUE(t2.IsColumnNull(1));
		EXPECT_TRUE(t2.IsColumnNull(2));
		EXPECT_NO_THROW(t2.GetColumnValue(3, varchar2));
		EXPECT_NO_THROW(t2.GetColumnValue(4, char2));
		EXPECT_EQ(s, varchar2);
		EXPECT_EQ(s, char2);
	}


	// Update rows
	// -----------

	TEST_P(TableTest, UpdateIntTypes)
	{
		std::wstring intTypesTableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(m_db, intTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(iTable.Open(m_db));
		ColumnBuffer* pId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt = iTable.GetColumnBuffer(1);
		ColumnBuffer* pInt = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBigInt = iTable.GetColumnBuffer(3);

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(iTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert some values
		for (int i = 1; i < 10; i++)
		{
			*pId = (SQLINTEGER)i;
			*pSmallInt = (SQLSMALLINT)i;
			*pInt = (SQLINTEGER)i;
			*pBigInt = (SQLBIGINT)i;
			ASSERT_NO_THROW(iTable.Insert());
		}
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Update single rows by using key-values
		*pId = (SQLINTEGER)3;
		*pSmallInt = (SQLSMALLINT) 101;
		*pInt = (SQLINTEGER) 102;
		*pBigInt = (SQLBIGINT) 103;
		EXPECT_NO_THROW(iTable.Update());
		
		*pId = (SQLINTEGER)5;
		*pSmallInt = (SQLSMALLINT)1001;
		*pInt = (SQLINTEGER)1002;
		*pBigInt = (SQLBIGINT)1003;
		EXPECT_NO_THROW(iTable.Update());

		// And set some values to null
		*pId = (SQLINTEGER)7;
		pSmallInt->SetNull();
		*pInt = (SQLINTEGER)99;
		*pBigInt = (SQLBIGINT)99;
		EXPECT_NO_THROW(iTable.Update());
		*pId = (SQLINTEGER)8;
		*pSmallInt = (SQLSMALLINT)99;
		pInt->SetNull();
		*pBigInt = (SQLBIGINT)99;
		EXPECT_NO_THROW(iTable.Update());
		*pId = (SQLINTEGER)9;
		*pSmallInt = (SQLSMALLINT) 99;
		*pInt = (SQLINTEGER)99;
		pBigInt->SetNull();
		EXPECT_NO_THROW(iTable.Update());
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Read back the just updated values
		iTable.Select((boost::wformat(L"%s = 3") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(3, (SQLINTEGER)*pId);
		EXPECT_EQ(101, (SQLSMALLINT)*pSmallInt);
		EXPECT_EQ(102, (SQLINTEGER)*pInt);
		EXPECT_EQ(103, (SQLBIGINT)*pBigInt);

		iTable.Select((boost::wformat(L"%s = 5") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(5, (SQLINTEGER)*pId);
		EXPECT_EQ(1001, (SQLSMALLINT)*pSmallInt);
		EXPECT_EQ(1002, (SQLINTEGER)*pInt);
		EXPECT_EQ(1003, (SQLBIGINT)*pBigInt);

		iTable.Select((boost::wformat(L"%s = 7") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(7, (SQLINTEGER)*pId);
		EXPECT_TRUE(pSmallInt->IsNull());
		EXPECT_EQ(99, (SQLINTEGER)*pInt);
		EXPECT_EQ(99, (SQLBIGINT)*pBigInt);

		iTable.Select((boost::wformat(L"%s = 8") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(8, (SQLINTEGER)*pId);
		EXPECT_EQ(99, (SQLSMALLINT)*pSmallInt);
		EXPECT_TRUE(pInt->IsNull());
		EXPECT_EQ(99, (SQLBIGINT)*pBigInt);

		iTable.Select((boost::wformat(L"%s = 9") % idName).str());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(9, (SQLINTEGER)*pId);
		EXPECT_EQ(99, (SQLSMALLINT)*pSmallInt);
		EXPECT_EQ(99, (SQLINTEGER)*pInt);
		EXPECT_TRUE(pBigInt->IsNull());
	}


	TEST_P(TableTest, UpdateDateTypes)
	{
		std::wstring dateTypesTableName = TestTables::GetTableName(TestTables::Table::DATETYPES_TMP, m_odbcInfo.m_namesCase);
		Table dTable(m_db, dateTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(dTable.Open(m_db));
		ColumnBuffer* pId = dTable.GetColumnBuffer(0);
		ColumnBuffer* pDate = dTable.GetColumnBuffer(1);
		ColumnBuffer* pTime = dTable.GetColumnBuffer(2);
		ColumnBuffer* pTimestamp = dTable.GetColumnBuffer(3);

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::DATETYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(dTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert some values
		SQL_TIME_STRUCT time = InitTime(13, 55, 56);
		SQL_DATE_STRUCT date = InitDate(26, 1, 2983);
		SQL_TIMESTAMP_STRUCT timestamp = InitTimestamp(13, 55, 56, 0, 26, 01, 1983);

		*pId = (SQLINTEGER)101;
		*pDate = date;
		*pTime = time;
		*pTimestamp = timestamp;
		dTable.Insert();
		*pId = (SQLINTEGER)102;
		pDate->SetNull();
		pTime->SetNull();
		pTimestamp->SetNull();
		dTable.Insert();
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Now update the values
		// \todo: We do not test the Fractions here. Lets fix Ticket #70 first
		date = InitDate(20, 9, 1985);
		time = InitTime(16, 31, 49);
		timestamp = InitTimestamp(16, 31, 49, 0, 20, 9, 1985);
		*pId = (SQLINTEGER)101;
		pDate->SetNull();
		pTime->SetNull();
		pTimestamp->SetNull();
		EXPECT_NO_THROW(dTable.Update());
		*pId = (SQLINTEGER)102;
		*pDate = date;
		*pTime = time;
		*pTimestamp = timestamp;
		EXPECT_NO_THROW(dTable.Update());
		ASSERT_NO_THROW(m_db.CommitTrans());

		// And read back
		dTable.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(dTable.SelectNext());
		EXPECT_TRUE(pDate->IsNull());
		EXPECT_TRUE(pTime->IsNull());
		EXPECT_TRUE(pTimestamp->IsNull());

		dTable.Select((boost::wformat(L"%s = 102") % idName).str());
		EXPECT_TRUE(dTable.SelectNext());
		EXPECT_FALSE(pDate->IsNull());
		EXPECT_FALSE(pTime->IsNull());
		EXPECT_FALSE(pTimestamp->IsNull());
		EXPECT_TRUE(IsDateEqual(date, *pDate));
		EXPECT_TRUE(IsTimeEqual(time, *pTime));
		EXPECT_TRUE(IsTimestampEqual(timestamp, *pTimestamp));
	}


	TEST_P(TableTest, UpdateNumericTypes)
	{
		std::wstring numericTypesTmpTableName = TestTables::GetTableName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		Table nTable(m_db, numericTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(nTable.Open(m_db));
		ColumnBuffer* pId = nTable.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = nTable.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = nTable.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = nTable.GetColumnBuffer(3);

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::NUMERICTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(nTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

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
		std::wstring floatTypesTableName = TestTables::GetTableName(TestTables::Table::FLOATTYPES_TMP, m_odbcInfo.m_namesCase);
		Table fTable(m_db, floatTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(fTable.Open(m_db));
		ColumnBuffer* pId = fTable.GetColumnBuffer(0);
		ColumnBuffer* pDouble = fTable.GetColumnBuffer(1);
		ColumnBuffer* pFloat = fTable.GetColumnBuffer(2);

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::FLOATTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(fTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

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
		Table fTable2(m_db, floatTypesTableName, L"", L"", L"", AF_READ);
		ASSERT_NO_THROW(fTable2.Open(m_db));
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
		std::wstring charTypesTmpTableName = TestTables::GetTableName(TestTables::Table::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		Table cTable(m_db, charTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		cTable.SetAutoBindingMode(AutoBindingMode::BIND_CHAR_AS_WCHAR);
		ASSERT_NO_THROW(cTable.Open(m_db));
		cTable.SetCharTrimRight(true);

		ColumnBuffer* pId = cTable.GetColumnBuffer(0);
		ColumnBuffer* pVarchar = cTable.GetColumnBuffer(1);
		ColumnBuffer* pChar = cTable.GetColumnBuffer(2);
		ColumnBuffer* pVarchar_10 = cTable.GetColumnBuffer(3);
		ColumnBuffer* pChar_10 = cTable.GetColumnBuffer(4);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(cTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

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
		EXPECT_NO_THROW(cTable.GetColumnValue(1, s2));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(2, s2));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(3, s2));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(4, s2));
		EXPECT_EQ(s100, s2);

		cTable.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		EXPECT_NO_THROW(cTable.GetColumnValue(1, s2));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(2, s2));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(3, s2));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(4, s2));
		EXPECT_EQ(s101, s2);
	}


	TEST_P(TableTest, UpdateCharTypes)
	{
		std::wstring charTypesTmpTableName = TestTables::GetTableName(TestTables::Table::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		Table cTable(m_db, charTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		cTable.SetAutoBindingMode(AutoBindingMode::BIND_WCHAR_AS_CHAR);
		ASSERT_NO_THROW(cTable.Open(m_db));
		cTable.SetCharTrimRight(true);

		ColumnBuffer* pId = cTable.GetColumnBuffer(0);
		ColumnBuffer* pVarchar = cTable.GetColumnBuffer(1);
		ColumnBuffer* pChar = cTable.GetColumnBuffer(2);
		ColumnBuffer* pVarchar_10 = cTable.GetColumnBuffer(3);
		ColumnBuffer* pChar_10 = cTable.GetColumnBuffer(4);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::CHARTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(cTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

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
		EXPECT_NO_THROW(cTable.GetColumnValue(1, s2));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(2, s2));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(3, s2));
		EXPECT_EQ(s100, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(4, s2));
		EXPECT_EQ(s100, s2);

		cTable.Select((boost::wformat(L"%s = 101") % idName).str());
		EXPECT_TRUE(cTable.SelectNext());
		EXPECT_NO_THROW(cTable.GetColumnValue(1, s2));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(2, s2));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(3, s2));
		EXPECT_EQ(s101, s2);
		EXPECT_NO_THROW(cTable.GetColumnValue(4, s2));
		EXPECT_EQ(s101, s2);
	}


	TEST_P(TableTest, UpdateBlobTypes)
	{
		std::wstring blobTypesTmpTableName = TestTables::GetTableName(TestTables::Table::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
		Table bTable(m_db, blobTypesTmpTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(bTable.Open(m_db));

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetIdColumnName(TestTables::Table::BLOBTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		ASSERT_NO_THROW(bTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

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
		SQLINTEGER size, length = 0;
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
		std::wstring intTypesTableName = TestTables::GetTableName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		Table iTable(m_db, intTypesTableName, L"", L"", L"", AF_READ_WRITE);
		ASSERT_NO_THROW(iTable.Open(m_db));
		ColumnBuffer* pId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt = iTable.GetColumnBuffer(1);
		ColumnBuffer* pInt = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBigInt = iTable.GetColumnBuffer(3);

		wstring idName = TestTables::GetIdColumnName(TestTables::Table::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		ASSERT_NO_THROW(iTable.Delete(sqlstmt, false));
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Insert some values
		for (int i = 1; i < 10; i++)
		{
			*pId = (SQLINTEGER)i;
			*pSmallInt = (SQLSMALLINT)i;
			*pInt = (SQLINTEGER)i;
			*pBigInt = (SQLBIGINT)i;
			ASSERT_NO_THROW(iTable.Insert());
		}
		ASSERT_NO_THROW(m_db.CommitTrans());

		// Now update using our WHERE statement. This allows us to update also key rows. Shift all values *(1000)
		int shift = 1000;
		for (int i = 1; i < 10; i++)
		{
			*pId = (SQLINTEGER) (i * shift);
			*pSmallInt = (SQLSMALLINT)(i * shift);
			*pInt = (SQLINTEGER)(i * shift);
			*pBigInt = (SQLBIGINT)(i * shift);
			sqlstmt = (boost::wformat(L"%s = %d") % idName %i).str();
			EXPECT_NO_THROW(iTable.Update(sqlstmt));
		}
		EXPECT_NO_THROW(m_db.CommitTrans());

		// And select them and compare
		sqlstmt = (boost::wformat(L"%s > 0 ORDER BY %s") % idName %idName).str();
		iTable.Select(sqlstmt);
		for (int i = 1; i < 10; i++)
		{
			EXPECT_TRUE(iTable.SelectNext());
			EXPECT_EQ((i * shift), (SQLINTEGER)*pId);
			EXPECT_EQ((i * shift), (SQLSMALLINT)*pSmallInt);
			EXPECT_EQ((i * shift), (SQLINTEGER)*pInt);
			EXPECT_EQ((i * shift), (SQLBIGINT)*pBigInt);
		}
		EXPECT_FALSE(iTable.SelectNext());
	}


// Interfaces
// ----------

} // namespace exodbc
