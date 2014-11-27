/*!
 * \file DbTableTest.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "TableTest.h"

// Same component headers
#include "exOdbcGTest.h"
#include "GenericTestTables.h"

// Other headers
#include "Environment.h"
#include "Database.h"
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
//using namespace exodbc;

namespace exodbc
{

	void TableTest::SetUp()
	{
		// Called for every unit-test
		m_pIntTypesAutoTable = NULL;

		m_odbcInfo = GetParam();

		// Set up Env
		ASSERT_TRUE(m_env.AllocHenv());
		ASSERT_TRUE(m_env.SetOdbcVersion(OV_3));

		// And database
		ASSERT_TRUE(m_db.AllocateHdbc(m_env));
		ASSERT_TRUE(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		// And an Auto-table
		std::wstring intTypesTableName = TestTables::GetTableName(L"integertypes", m_odbcInfo.m_namesCase);
		m_pIntTypesAutoTable = new Table(&m_db, intTypesTableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(m_pIntTypesAutoTable->Open(false, true));
	}


	void TableTest::TearDown()
	{
		if (m_pIntTypesAutoTable && m_pIntTypesAutoTable->IsOpen())
		{
			delete m_pIntTypesAutoTable;
			m_pIntTypesAutoTable = NULL;
		}
		if (m_db.IsOpen())
		{
			// Microsoft Sql Server needs a CommitTrans()
			if (m_db.Dbms() == dbmsMS_SQL_SERVER)
			{
				EXPECT_TRUE(m_db.CommitTrans());
			}
			EXPECT_TRUE(m_db.Close());
		}
	}

	// Open
	// ----
	TEST_P(TableTest, OpenManualWithoutCheck)
	{
		// Open an existing table without checking for privileges or existence
		IntTypesTable table(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(table.Open(false, false));

		// If we pass in the STableInfo directly we should also be able to "open"
		// a totally non-sense table:
		STableInfo neTableInfo;
		neTableInfo.m_tableName = L"NotExisting";
		Table neTable(&m_db, 2, neTableInfo, Table::READ_ONLY);
		// TODO: Here we must set some columns
		EXPECT_TRUE(neTable.Open(false, false));
		// TODO: So we can prove in the test that we will fail doing a SELECT later
	}

	TEST_P(TableTest, OpenManualCheckExistence)
	{
		// Open a table with checking for existence
		IntTypesTable table(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(table.Open(false, true));

		// Open a non-existing table
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			NotExistingTable neTable(&m_db, m_odbcInfo.m_namesCase);
			EXPECT_FALSE(neTable.Open(false, true));
		}
	}

	TEST_P(TableTest, OpenAutoWithoutCheck)
	{
		// Open an auto-table without checking for privileges or existence
		// This makes only sense if we've already determined the correct STableInfo structure
		std::wstring tableName = TestTables::GetTableName(L"integertypes", m_odbcInfo.m_namesCase);
		STableInfo tableInfo;
		EXPECT_TRUE(m_db.FindOneTable(tableName, L"", L"", L"", tableInfo));

		exodbc::Table table(&m_db, tableInfo, Table::READ_ONLY);
		EXPECT_TRUE(table.Open(false, false));

		// If we try to open an auto-table this will never work if you've passed invalid information:
		// As soon as the columns are searched, we expect to fail
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			STableInfo neTableInfo;
			neTableInfo.m_tableName = L"NotExisting";
			Table neTable(&m_db, neTableInfo, Table::READ_ONLY);
			EXPECT_FALSE(neTable.Open(false, false));
		}
	}

	TEST_P(TableTest, OpenAutoCheckExistence)
	{
		std::wstring tableName = TestTables::GetTableName(L"integertypes", m_odbcInfo.m_namesCase);
		exodbc::Table table(&m_db, tableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(table.Open(false, true));

		// Open a non-existing table
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			std::wstring neName = TestTables::GetTableName(L"notexistring", m_odbcInfo.m_namesCase);
			exodbc::Table neTable(&m_db, neName, L"", L"", L"", Table::READ_ONLY);
			EXPECT_FALSE(neTable.Open(false, true));
		}
	}

	// Select / GetNext
	// ----------------
	TEST_P(TableTest, Select)
	{
		ASSERT_TRUE(m_pIntTypesAutoTable != NULL);
		ASSERT_TRUE(m_pIntTypesAutoTable->IsOpen());

		EXPECT_TRUE(m_pIntTypesAutoTable->Select(L""));

		EXPECT_TRUE(m_pIntTypesAutoTable->SelectClose());

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}

	TEST_P(TableTest, SelectNext)
	{
		ASSERT_TRUE(m_pIntTypesAutoTable != NULL);
		ASSERT_TRUE(m_pIntTypesAutoTable->IsOpen());

		// We expect 6 Records
		EXPECT_TRUE(m_pIntTypesAutoTable->Select(L""));
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_FALSE(m_pIntTypesAutoTable->SelectNext());

		EXPECT_TRUE(m_pIntTypesAutoTable->SelectClose());

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}

	TEST_P(TableTest, SelectClose)
	{
		ASSERT_TRUE(m_pIntTypesAutoTable != NULL);
		ASSERT_TRUE(m_pIntTypesAutoTable->IsOpen());
		
		// Do something that opens a transaction
		EXPECT_TRUE(m_pIntTypesAutoTable->Select(L""));
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectClose());
		// We should be closed now
		EXPECT_FALSE(m_pIntTypesAutoTable->IsSelectOpen());
		// On MySql, closing works always
		if (m_db.Dbms() != dbmsMY_SQL)
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			EXPECT_FALSE(m_pIntTypesAutoTable->SelectClose());
		}

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}

	// Count
	// -----
	TEST_P(TableTest, Count)
	{
		FloatTypesTable table(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(table.Open(false, true));

		size_t all;
		EXPECT_TRUE(table.Count(L"", all));
		EXPECT_EQ(6, all);

		// TODO: We need some GetColumnName function for things like that
		size_t some;
		std::wstring whereStmt = L"tdouble > 0";
		if (m_db.Dbms() == dbmsDB2)
		{
			whereStmt = L"TDOUBLE > 0";
		}
		EXPECT_TRUE(table.Count(whereStmt, some));
		EXPECT_EQ(1, some);
		whereStmt = L"tdouble > 0 OR tfloat > 0";
		if (m_db.Dbms() == dbmsDB2)
		{
			whereStmt = L"TDOUBLE > 0 OR TFLOAT > 0";
		}
		EXPECT_TRUE(table.Count(whereStmt, some));
		EXPECT_EQ(2, some);
	}

	// GetValues
	// ---------
	TEST_P(TableTest, GetIntValues)
	{
		ASSERT_TRUE(m_pIntTypesAutoTable != NULL);
		ASSERT_TRUE(m_pIntTypesAutoTable->IsOpen());

		// Test values
		SQLSMALLINT s = 0;
		SQLINTEGER i = 0;
		SQLBIGINT b = 0;
		std::wstring str;

		// We expect 6 Records
		std::wstring idName = TestTables::GetColName(L"idintegertypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(m_pIntTypesAutoTable->Select( (boost::wformat(L"%d = 1") %idName).str()));
		// The first column has a smallint set, we can read that as any int value -32768
		SQLBIGINT colVal = -32768;
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(1, s));
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(1, i));
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(1, b));
		EXPECT_EQ(colVal, s);
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		// Read as str
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(1, str));
		EXPECT_EQ(L"-32768", str);
		EXPECT_FALSE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectClose());

		EXPECT_TRUE(m_pIntTypesAutoTable->Select((boost::wformat(L"%d = 2") % idName).str()));
		colVal = 32767;
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(1, s));
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(1, i));
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(1, b));
		EXPECT_EQ(colVal, s);
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		// Read as str
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(1, str));
		EXPECT_EQ(L"32767", str);
		EXPECT_FALSE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectClose());

		EXPECT_TRUE(m_pIntTypesAutoTable->Select((boost::wformat(L"%d = 3") % idName).str()));
		// The 2nd column has a int set, we can read that as int or bigint value -2147483648
		colVal = INT_MIN;
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_FALSE(m_pIntTypesAutoTable->GetColumnValue(2, s));
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(2, i));
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(2, b));
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		// Read as str
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(2, str));
		EXPECT_EQ(L"-2147483648", str);
		EXPECT_FALSE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectClose());

		EXPECT_TRUE(m_pIntTypesAutoTable->Select((boost::wformat(L"%d = 4") % idName).str()));
		colVal = 2147483647;
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_FALSE(m_pIntTypesAutoTable->GetColumnValue(2, s));
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(2, i));
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(2, b));
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(2, str));
		EXPECT_EQ(L"2147483647", str);
		EXPECT_FALSE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectClose());

		EXPECT_TRUE(m_pIntTypesAutoTable->Select((boost::wformat(L"%d = 5") % idName).str()));
		// The 3rd column has a bigint set, we can read that as bigint value -9223372036854775808
		colVal = (-9223372036854775807 - 1);
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_FALSE(m_pIntTypesAutoTable->GetColumnValue(3, s));
		EXPECT_FALSE(m_pIntTypesAutoTable->GetColumnValue(3, i));
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(3, b));
		EXPECT_EQ(colVal, b);
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(3, str));
		EXPECT_EQ(L"-9223372036854775808", str);
		EXPECT_FALSE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectClose());

		EXPECT_TRUE(m_pIntTypesAutoTable->Select((boost::wformat(L"%d = 6") % idName).str()));
		colVal = 9223372036854775807;
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_FALSE(m_pIntTypesAutoTable->GetColumnValue(3, s));
		EXPECT_FALSE(m_pIntTypesAutoTable->GetColumnValue(3, i));
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(3, b));
		EXPECT_EQ(colVal, b);
		EXPECT_TRUE(m_pIntTypesAutoTable->GetColumnValue(3, str));
		EXPECT_EQ(L"9223372036854775807", str);
		EXPECT_FALSE(m_pIntTypesAutoTable->SelectNext());
		EXPECT_TRUE(m_pIntTypesAutoTable->SelectClose());

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}

	TEST_P(TableTest, GetWCharValues)
	{
		std::wstring charTypesTableName = TestTables::GetTableName(L"chartypes", m_odbcInfo.m_namesCase);
		Table charTypesAutoTable(&m_db, charTypesTableName, L"", L"", L"", Table::READ_ONLY);
		charTypesAutoTable.SetCharBindingMode(Table::BIND_AS_WCHAR);
		EXPECT_TRUE(charTypesAutoTable.Open(false, true));
		// We want to trim on the right side for DB2 and sql server
		charTypesAutoTable.SetCharTrimOption(Table::TRIM_RIGHT);

		std::wstring str;

		// We expect 6 Records
		std::wstring idName = TestTables::GetColName(L"idchartypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 1") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(1, str));
		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);
		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 2") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(2, str));
		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 3") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(1, str));
		EXPECT_EQ(std::wstring(L"הצüאיט"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 4") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(2, str));
		EXPECT_EQ(std::wstring(L"הצüאיט"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}


	TEST_P(TableTest, GetCharValues)
	{
		std::wstring charTypesTableName = TestTables::GetTableName(L"chartypes", m_odbcInfo.m_namesCase);
		Table charTypesAutoTable(&m_db, charTypesTableName, L"", L"", L"", Table::READ_ONLY);
		charTypesAutoTable.SetCharBindingMode(Table::BIND_AS_CHAR);
		EXPECT_TRUE(charTypesAutoTable.Open(false, true));
		// We want to trim on the right side for DB2 and also sql server
		charTypesAutoTable.SetCharTrimOption(Table::TRIM_RIGHT);

		std::string str;

		// We expect 6 Records
		std::wstring idName = TestTables::GetColName(L"idchartypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 1") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(1, str));
		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);
		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 2") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(2, str));
		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 3") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(1, str));
		EXPECT_EQ(std::string("הצüאיט"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 4") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(2, str));
		EXPECT_EQ(std::string("הצüאיט"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}


// Interfaces
// ----------

} // namespace exodbc
