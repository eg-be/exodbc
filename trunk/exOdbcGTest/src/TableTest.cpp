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
#include "ManualTestTables.h"

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
		m_pIntTypesManualTable = NULL;

		m_odbcInfo = GetParam();

		// Set up Env
		ASSERT_TRUE(m_env.AllocHenv());
		// Try to set to the highest version available: We need that for the tests to run correct
		ASSERT_TRUE(m_env.SetOdbcVersion(OV_3));

		// And database
		ASSERT_TRUE(m_db.AllocateHdbc(m_env));
		ASSERT_TRUE(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		// And some tables with auto-columns
		std::wstring intTypesTableName = TestTables::GetTableName(L"integertypes", m_odbcInfo.m_namesCase);
		m_pIntTypesAutoTable = new Table(&m_db, intTypesTableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(m_pIntTypesAutoTable->Open(false, true));

		// And some tables with manual-columns
		m_pIntTypesManualTable = new MIntTypesTable(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(m_pIntTypesManualTable->Open(false, true));
	}


	void TableTest::TearDown()
	{
		if (m_pIntTypesAutoTable)
		{
			delete m_pIntTypesAutoTable;
			m_pIntTypesAutoTable = NULL;
		}
		if (m_pIntTypesManualTable)
		{
			delete m_pIntTypesManualTable;
			m_pIntTypesManualTable = NULL;
		}
		if (m_db.IsOpen())
		{
			//// Microsoft Sql Server needs a CommitTrans()
			//if (m_db.Dbms() == dbmsMS_SQL_SERVER)
			//{
			//	EXPECT_TRUE(m_db.CommitTrans());
			//}
			EXPECT_TRUE(m_db.Close());
		}
	}


	// Open
	// ----
	TEST_P(TableTest, OpenManualWithoutCheck)
	{
		// Open an existing table without checking for privileges or existence
		MIntTypesTable table(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(table.Open(false, false));

		// If we pass in the STableInfo directly we should also be able to "open"
		// a totally non-sense table:
		STableInfo neTableInfo;
		neTableInfo.m_tableName = L"NotExisting";
		Table neTable(&m_db, 2, neTableInfo, Table::READ_ONLY);
		SQLINTEGER idNotExisting = 0;
		neTable.SetColumn(0, L"idNotExistring", &idNotExisting, SQL_C_SLONG, sizeof(idNotExisting));
		EXPECT_TRUE(neTable.Open(false, false));
		// TODO: So we can prove in the test that we will fail doing a SELECT later
	}


	TEST_P(TableTest, OpenManualCheckExistence)
	{
		// Open a table with checking for existence
		MIntTypesTable table(&m_db, m_odbcInfo.m_namesCase);
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
		MFloatTypesTable table(&m_db, m_odbcInfo.m_namesCase);
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
	TEST_P(TableTest, GetAutoIntValues)
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
		EXPECT_TRUE(m_pIntTypesAutoTable->Select( (boost::wformat(L"%s = 1") %idName).str()));
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

		EXPECT_TRUE(m_pIntTypesAutoTable->Select((boost::wformat(L"%s = 2") % idName).str()));
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

		EXPECT_TRUE(m_pIntTypesAutoTable->Select((boost::wformat(L"%s = 3") % idName).str()));
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

		EXPECT_TRUE(m_pIntTypesAutoTable->Select((boost::wformat(L"%s = 4") % idName).str()));
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

		EXPECT_TRUE(m_pIntTypesAutoTable->Select((boost::wformat(L"%s = 5") % idName).str()));
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

		EXPECT_TRUE(m_pIntTypesAutoTable->Select((boost::wformat(L"%s = 6") % idName).str()));
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


	TEST_P(TableTest, GetManualIntValues)
	{
		ASSERT_TRUE(m_pIntTypesManualTable != NULL);
		ASSERT_TRUE(m_pIntTypesManualTable->IsOpen());

		// Just check that the buffers are correct
		std::wstring idName = TestTables::GetColName(L"idintegertypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(m_pIntTypesManualTable->Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(m_pIntTypesManualTable->SelectNext());
		EXPECT_EQ(-32768, m_pIntTypesManualTable->m_smallInt);

		EXPECT_TRUE(m_pIntTypesManualTable->Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(m_pIntTypesManualTable->SelectNext());
		EXPECT_EQ(32767, m_pIntTypesManualTable->m_smallInt);

		EXPECT_TRUE(m_pIntTypesManualTable->Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(m_pIntTypesManualTable->SelectNext());
		EXPECT_EQ(INT_MIN, m_pIntTypesManualTable->m_int);

		EXPECT_TRUE(m_pIntTypesManualTable->Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(m_pIntTypesManualTable->SelectNext());
		EXPECT_EQ(2147483647, m_pIntTypesManualTable->m_int);

		EXPECT_TRUE(m_pIntTypesManualTable->Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(m_pIntTypesManualTable->SelectNext());
		EXPECT_EQ((-9223372036854775807 - 1), m_pIntTypesManualTable->m_bigInt);

		EXPECT_TRUE(m_pIntTypesManualTable->Select((boost::wformat(L"%s = 6") % idName).str()));
		EXPECT_TRUE(m_pIntTypesManualTable->SelectNext());
		EXPECT_EQ(9223372036854775807, m_pIntTypesManualTable->m_bigInt);

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}


	TEST_P(TableTest, GetAutoFloatValues)
	{
		wstring floatTypesTableName = TestTables::GetTableName(L"floattypes", m_odbcInfo.m_namesCase);
		Table fTable(&m_db, floatTypesTableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(fTable.Open(false, true));

		SQLDOUBLE val;
		wstring str;
		std::wstring idName = TestTables::GetColName(L"idfloattypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(2, val));
		EXPECT_EQ(0.0, val);
		// Read as str
		EXPECT_TRUE(fTable.GetColumnValue(2, str));
		EXPECT_EQ(L"0.000000", str);

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(2, val));
		EXPECT_EQ(3.141, val);
		// Read as str
		EXPECT_TRUE(fTable.GetColumnValue(2, str));
		EXPECT_EQ(L"3.141000", str);

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(2, val));
		EXPECT_EQ(-3.141, val);
		// Read as str
		EXPECT_TRUE(fTable.GetColumnValue(2, str));
		EXPECT_EQ(L"-3.141000", str);

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(1, val));
		EXPECT_EQ(0.0, val);
		// Read as str
		EXPECT_TRUE(fTable.GetColumnValue(1, str));
		EXPECT_EQ(L"0.000000", str);

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(1, val));
		EXPECT_EQ(3.141592, val);
		// Read as str
		EXPECT_TRUE(fTable.GetColumnValue(1, str));
		EXPECT_EQ(L"3.141592", str);

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 6") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(1, val));
		EXPECT_EQ(-3.141592, val);
		// Read as str
		EXPECT_TRUE(fTable.GetColumnValue(1, str));
		EXPECT_EQ(L"-3.141592", str);

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}


	TEST_P(TableTest, GetManualFloatValues)
	{
		MFloatTypesTable fTable(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(fTable.Open(false, true));

		std::wstring idName = TestTables::GetColName(L"idfloattypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(0.0, fTable.m_float);

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(3.141, fTable.m_float);

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(-3.141, fTable.m_float);

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(0.0, fTable.m_double);

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(3.141592, fTable.m_double);

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 6") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_EQ(-3.141592, fTable.m_double);

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}


	TEST_P(TableTest, GetAutoWCharValues)
	{
		std::wstring charTypesTableName = TestTables::GetTableName(L"chartypes", m_odbcInfo.m_namesCase);
		Table charTypesAutoTable(&m_db, charTypesTableName, L"", L"", L"", Table::READ_ONLY);
		charTypesAutoTable.SetCharBindingMode(CharBindingMode::BIND_AS_WCHAR);
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
		EXPECT_EQ(std::wstring(L"������"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 4") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(2, str));
		EXPECT_EQ(std::wstring(L"������"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}


	TEST_P(TableTest, GetManualWCharValues)
	{
		MWCharTypesTable wTable(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(wTable.Open(false, true));

		// Just test the values in the buffer - trimming has no effect here
		using namespace boost::algorithm;
		std::wstring str;
		std::wstring idName = TestTables::GetColName(L"idchartypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(wTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*) &wTable.m_varchar);
		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
		EXPECT_FALSE(wTable.SelectNext());
		EXPECT_TRUE(wTable.SelectClose());

		EXPECT_TRUE(wTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*)&wTable.m_char);
		EXPECT_EQ(std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
		EXPECT_FALSE(wTable.SelectNext());
		EXPECT_TRUE(wTable.SelectClose());

		EXPECT_TRUE(wTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*)&wTable.m_varchar);
		EXPECT_EQ(std::wstring(L"������"), trim_right_copy(str));
		EXPECT_FALSE(wTable.SelectNext());
		EXPECT_TRUE(wTable.SelectClose());

		EXPECT_TRUE(wTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*)&wTable.m_char);
		EXPECT_EQ(std::wstring(L"������"), trim_right_copy(str));
		EXPECT_FALSE(wTable.SelectNext());
		EXPECT_TRUE(wTable.SelectClose());

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}


	TEST_P(TableTest, GetAutoCharValues)
	{
		std::wstring charTypesTableName = TestTables::GetTableName(L"chartypes", m_odbcInfo.m_namesCase);
		Table charTypesAutoTable(&m_db, charTypesTableName, L"", L"", L"", Table::READ_ONLY);
		charTypesAutoTable.SetCharBindingMode(CharBindingMode::BIND_AS_CHAR);
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
		EXPECT_EQ(std::string("������"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 4") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(2, str));
		EXPECT_EQ(std::string("������"), str);

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.SelectClose());

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}


	TEST_P(TableTest, GetManualCharValues)
	{
		MCharTypesTable cTable(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(cTable.Open(false, true));

		// Just test the values in the buffer - trimming has no effect here
		using namespace boost::algorithm;
		std::string str;
		std::wstring idName = TestTables::GetColName(L"idchartypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(cTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_varchar);
		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
		EXPECT_FALSE(cTable.SelectNext());
		EXPECT_TRUE(cTable.SelectClose());

		EXPECT_TRUE(cTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_char);
		EXPECT_EQ(std::string(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), trim_right_copy(str));
		EXPECT_FALSE(cTable.SelectNext());
		EXPECT_TRUE(cTable.SelectClose());

		EXPECT_TRUE(cTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_varchar);
		EXPECT_EQ(std::string("������"), trim_right_copy(str));
		EXPECT_FALSE(cTable.SelectNext());
		EXPECT_TRUE(cTable.SelectClose());

		EXPECT_TRUE(cTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_char);
		EXPECT_EQ(std::string("������"), trim_right_copy(str));
		EXPECT_FALSE(cTable.SelectNext());
		EXPECT_TRUE(cTable.SelectClose());

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}


	TEST_P(TableTest, GetAutoDateValues)
	{
		std::wstring dateTypesTableName = TestTables::GetTableName(L"datetypes", m_odbcInfo.m_namesCase);
		Table dTable(&m_db, dateTypesTableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(dTable.Open(false, true));

		using namespace boost::algorithm;
		std::wstring str;
		SQL_DATE_STRUCT date;
		SQL_TIME_STRUCT time;
		SQL_TIMESTAMP_STRUCT timestamp;
		std::wstring idName = TestTables::GetColName(L"iddatetypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(dTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(dTable.SelectNext());
		EXPECT_TRUE(dTable.GetColumnValue(1, date));
		EXPECT_TRUE(dTable.GetColumnValue(2, time));
		EXPECT_TRUE(dTable.GetColumnValue(3, timestamp));

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

		// MS SQL Server has a new SQL_TIME2_STRUCT if ODBC version is >= 3.8
#if HAVE_MSODBCSQL_H
		Environment env38(OV_3_8);
		EXPECT_TRUE(env38.HasHenv());
		Database db38(env38);
		EXPECT_TRUE(db38.HasHdbc());
		EXPECT_TRUE(db38.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		Table dTable38(&db38, dateTypesTableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(dTable38.Open(false, true));
		EXPECT_TRUE(dTable38.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(dTable38.SelectNext());
		EXPECT_TRUE(dTable38.GetColumnValue(2, time));
		EXPECT_EQ(13, time.hour);
		EXPECT_EQ(55, time.minute);
		EXPECT_EQ(56, time.second);
#endif

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		//if (db38.GetCommitMode() != CM_AUTO_COMMIT && (db38.Dbms() == dbmsDB2 || db38.Dbms() == dbmsMS_SQL_SERVER))
		//{
		//	EXPECT_TRUE(db38.CommitTrans());
		//}
		//if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		//{
		//	EXPECT_TRUE(m_db.CommitTrans());
		//}
	}


	TEST_P(TableTest, GetManualDateValues)
	{
		MDateTypesTable cTable(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(cTable.Open(false, true));

		// Just test the values in the buffer - trimming has no effect here
		using namespace boost::algorithm;
		std::string str;
		std::wstring idName = TestTables::GetColName(L"iddatetypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(cTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(cTable.SelectNext());
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

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_db.GetCommitMode() != CM_AUTO_COMMIT && m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(m_db.CommitTrans());
		}
	}
// Interfaces
// ----------

} // namespace exodbc
