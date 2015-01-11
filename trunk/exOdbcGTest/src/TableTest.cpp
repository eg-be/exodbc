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
		m_odbcInfo = GetParam();

		// Set up Env
		ASSERT_TRUE(m_env.AllocHenv());
		// Try to set to the highest version available: We need that for the tests to run correct
		ASSERT_TRUE(m_env.SetOdbcVersion(OV_3));

		// And database
		ASSERT_TRUE(m_db.AllocateHdbc(m_env));
		ASSERT_TRUE(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

	}


	void TableTest::TearDown()
	{
		if (m_db.IsOpen())
		{
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
			MNotExistingTable neTable(&m_db, m_odbcInfo.m_namesCase);
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


	TEST_P(TableTest, DISABLED_OpenAutoCheckPrivs)
	{
		// Test to open read-only:
		// MySQL fails totally with the privileges stuff
		std::wstring tableName = TestTables::GetTableName(L"integertypes", m_odbcInfo.m_namesCase);
		exodbc::Table rTable(&m_db, tableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(rTable.Open(true, true));

		// Test a table that is read only: we can only open it read-only:
		std::wstring so1Name = TestTables::GetTableName(L"selectonly", m_odbcInfo.m_namesCase);
		exodbc::Table so1Table(&m_db, so1Name, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(so1Table.Open(true, true));

		std::wstring so2Name = TestTables::GetTableName(L"selectonly", m_odbcInfo.m_namesCase);
		exodbc::Table so2Table(&m_db, so2Name, L"", L"", L"", Table::READ_WRITE);
		// DB2 fails here, it still reports we have insert permissions. but doing an insert later fails

		EXPECT_FALSE(so2Table.Open(true, true));
		EXPECT_FALSE(m_db.ExecSql(L"insert into exodbc.dbo.selectonly (idselectonly) values (3)"));
		int p = 3;
	}


	// Select / GetNext
	// ----------------
	TEST_P(TableTest, Select)
	{
		std::wstring tableName = TestTables::GetTableName(L"integertypes", m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", Table::READ_ONLY);
		ASSERT_TRUE(iTable.Open(false, true));

		EXPECT_TRUE(iTable.Select(L""));

		EXPECT_TRUE(iTable.SelectClose());
	}


	TEST_P(TableTest, SelectNext)
	{
		std::wstring tableName = TestTables::GetTableName(L"integertypes", m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", Table::READ_ONLY);
		ASSERT_TRUE(iTable.Open(false, true));

		// We expect 6 Records
		EXPECT_TRUE(iTable.Select(L""));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_FALSE(iTable.SelectNext());

		EXPECT_TRUE(iTable.SelectClose());
	}


	TEST_P(TableTest, SelectClose)
	{
		std::wstring tableName = TestTables::GetTableName(L"integertypes", m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", Table::READ_ONLY);
		ASSERT_TRUE(iTable.Open(false, true));
		
		// Do something that opens a transaction
		EXPECT_TRUE(iTable.Select(L""));
		EXPECT_TRUE(iTable.SelectClose());
		// We should be closed now
		EXPECT_FALSE(iTable.IsSelectOpen());
		// On MySql, closing works always
		if (m_db.Dbms() != dbmsMY_SQL)
		{
			LogLevelFatal llFatal;
			LOG_ERROR(L"Warning: This test is supposed to spit errors");
			EXPECT_FALSE(iTable.SelectClose());
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
		std::wstring tableName = TestTables::GetTableName(L"integertypes", m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", Table::READ_ONLY);
		ASSERT_TRUE(iTable.Open(false, true));

		// Test values
		SQLSMALLINT s = 0;
		SQLINTEGER i = 0;
		SQLBIGINT b = 0;
		std::wstring str;

		// We expect 6 Records
		std::wstring idName = TestTables::GetColName(L"idintegertypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		// The first column has a smallint set, we can read that as any int value -32768
		SQLBIGINT colVal = -32768;
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.GetColumnValue(1, s));
		EXPECT_TRUE(iTable.GetColumnValue(1, i));
		EXPECT_TRUE(iTable.GetColumnValue(1, b));
		EXPECT_EQ(colVal, s);
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		// Read as str
		EXPECT_TRUE(iTable.GetColumnValue(1, str));
		EXPECT_EQ(L"-32768", str);
		EXPECT_FALSE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectClose());

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		colVal = 32767;
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.GetColumnValue(1, s));
		EXPECT_TRUE(iTable.GetColumnValue(1, i));
		EXPECT_TRUE(iTable.GetColumnValue(1, b));
		EXPECT_EQ(colVal, s);
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		// Read as str
		EXPECT_TRUE(iTable.GetColumnValue(1, str));
		EXPECT_EQ(L"32767", str);
		EXPECT_FALSE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectClose());

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		// The 2nd column has a int set, we can read that as int or bigint value -2147483648
		colVal = INT_MIN;
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_FALSE(iTable.GetColumnValue(2, s));
		EXPECT_TRUE(iTable.GetColumnValue(2, i));
		EXPECT_TRUE(iTable.GetColumnValue(2, b));
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		// Read as str
		EXPECT_TRUE(iTable.GetColumnValue(2, str));
		EXPECT_EQ(L"-2147483648", str);
		EXPECT_FALSE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectClose());

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		colVal = 2147483647;
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_FALSE(iTable.GetColumnValue(2, s));
		EXPECT_TRUE(iTable.GetColumnValue(2, i));
		EXPECT_TRUE(iTable.GetColumnValue(2, b));
		EXPECT_EQ(colVal, i);
		EXPECT_EQ(colVal, b);
		EXPECT_TRUE(iTable.GetColumnValue(2, str));
		EXPECT_EQ(L"2147483647", str);
		EXPECT_FALSE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectClose());

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		// The 3rd column has a bigint set, we can read that as bigint value -9223372036854775808
		colVal = (-9223372036854775807 - 1);
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_FALSE(iTable.GetColumnValue(3, s));
		EXPECT_FALSE(iTable.GetColumnValue(3, i));
		EXPECT_TRUE(iTable.GetColumnValue(3, b));
		EXPECT_EQ(colVal, b);
		EXPECT_TRUE(iTable.GetColumnValue(3, str));
		EXPECT_EQ(L"-9223372036854775808", str);
		EXPECT_FALSE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectClose());

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 6") % idName).str()));
		colVal = 9223372036854775807;
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_FALSE(iTable.GetColumnValue(3, s));
		EXPECT_FALSE(iTable.GetColumnValue(3, i));
		EXPECT_TRUE(iTable.GetColumnValue(3, b));
		EXPECT_EQ(colVal, b);
		EXPECT_TRUE(iTable.GetColumnValue(3, str));
		EXPECT_EQ(L"9223372036854775807", str);
		EXPECT_FALSE(iTable.SelectNext());
		EXPECT_TRUE(iTable.SelectClose());
	}


	TEST_P(TableTest, GetManualIntValues)
	{
		MIntTypesTable iTable(&m_db, m_odbcInfo.m_namesCase);
		ASSERT_TRUE(iTable.Open(false, true));

		// Just check that the buffers are correct
		std::wstring idName = TestTables::GetColName(L"idintegertypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(-32768, iTable.m_smallInt);

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(32767, iTable.m_smallInt);

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(INT_MIN, iTable.m_int);

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(2147483647, iTable.m_int);

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ((-9223372036854775807 - 1), iTable.m_bigInt);

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 6") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_EQ(9223372036854775807, iTable.m_bigInt);
	}


	TEST_P(TableTest, GetWCharIntValues)
	{
		// And some tables with auto-columns
		std::wstring intTypesTableName = TestTables::GetTableName(L"integertypes", m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", Table::READ_ONLY);
		iTable.SetAutoBindingMode(AutoBindingMode::BIND_ALL_AS_WCHAR);
		EXPECT_TRUE(iTable.Open(false, true));

		std::wstring id, smallInt, i, bigInt;
		std::wstring idName = TestTables::GetColName(L"idintegertypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.GetColumnValue(1, smallInt));
		EXPECT_EQ(L"-32768", smallInt);

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.GetColumnValue(1, smallInt));
		EXPECT_EQ(L"32767", smallInt);

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.GetColumnValue(2, i));
		EXPECT_EQ(L"-2147483648", i);

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.GetColumnValue(2, i));
		EXPECT_EQ(L"2147483647", i);

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.GetColumnValue(3, bigInt));
		EXPECT_EQ(L"-9223372036854775808", bigInt);

		EXPECT_TRUE(iTable.Select((boost::wformat(L"%s = 6") % idName).str()));
		EXPECT_TRUE(iTable.SelectNext());
		EXPECT_TRUE(iTable.GetColumnValue(3, bigInt));
		EXPECT_EQ(L"9223372036854775807", bigInt);
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
	}


	TEST_P(TableTest, GetWCharFloatValues)
	{
		wstring floatTypesTableName = TestTables::GetTableName(L"floattypes", m_odbcInfo.m_namesCase);
		Table fTable(&m_db, floatTypesTableName, L"", L"", L"", Table::READ_ONLY);
		fTable.SetAutoBindingMode(AutoBindingMode::BIND_ALL_AS_WCHAR);
		EXPECT_TRUE(fTable.Open(false, true));

		wstring sVal;
		wstring idName = TestTables::GetColName(L"idfloattypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(2, sVal));
		//EXPECT_EQ(0.0, sVal);
		
		// TODO: Find a way to compare without having to diff for every db-type

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(2, sVal));

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(2, sVal));

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(1, sVal));

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(1, sVal));

		EXPECT_TRUE(fTable.Select((boost::wformat(L"%s = 6") % idName).str()));
		EXPECT_TRUE(fTable.SelectNext());
		EXPECT_TRUE(fTable.GetColumnValue(1, sVal));

	}


	TEST_P(TableTest, GetAutoWCharValues)
	{
		std::wstring charTypesTableName = TestTables::GetTableName(L"chartypes", m_odbcInfo.m_namesCase);
		Table charTypesAutoTable(&m_db, charTypesTableName, L"", L"", L"", Table::READ_ONLY);
		charTypesAutoTable.SetAutoBindingMode(AutoBindingMode::BIND_CHAR_AS_WCHAR);
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

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 5") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(3, str));
		EXPECT_EQ(std::wstring(L"abc"), str);
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(4, str));
		EXPECT_EQ(std::wstring(L"abc"), str);

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 6") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(3, str));
		EXPECT_EQ(std::wstring(L"abcde12345"), str);
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(4, str));
		EXPECT_EQ(std::wstring(L"abcde12345"), str);

		EXPECT_TRUE(charTypesAutoTable.SelectClose());
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
		EXPECT_EQ(std::wstring(L"הצüאיט"), trim_right_copy(str));
		EXPECT_FALSE(wTable.SelectNext());
		EXPECT_TRUE(wTable.SelectClose());

		EXPECT_TRUE(wTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*)&wTable.m_char);
		EXPECT_EQ(std::wstring(L"הצüאיט"), trim_right_copy(str));
		EXPECT_FALSE(wTable.SelectNext());
		EXPECT_TRUE(wTable.SelectClose());

		EXPECT_TRUE(wTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*)&wTable.m_char_10);
		EXPECT_EQ(std::wstring(L"abc"), trim_right_copy(str));
		str.assign((wchar_t*)&wTable.m_varchar_10);
		EXPECT_EQ(std::wstring(L"abc"), trim_right_copy(str));
		EXPECT_TRUE(wTable.SelectClose());

		EXPECT_TRUE(wTable.Select((boost::wformat(L"%s = 6") % idName).str()));
		EXPECT_TRUE(wTable.SelectNext());
		str.assign((wchar_t*)&wTable.m_char_10);
		EXPECT_EQ(std::wstring(L"abcde12345"), trim_right_copy(str));
		str.assign((wchar_t*)&wTable.m_varchar_10);
		EXPECT_EQ(std::wstring(L"abcde12345"), trim_right_copy(str));
		EXPECT_TRUE(wTable.SelectClose());

	}


	TEST_P(TableTest, GetAutoCharValues)
	{
		std::wstring charTypesTableName = TestTables::GetTableName(L"chartypes", m_odbcInfo.m_namesCase);
		Table charTypesAutoTable(&m_db, charTypesTableName, L"", L"", L"", Table::READ_ONLY);
		charTypesAutoTable.SetAutoBindingMode(AutoBindingMode::BIND_WCHAR_AS_CHAR);
		EXPECT_TRUE(charTypesAutoTable.Open(false, true));
		// We want to trim on the right side for DB2 and also sql server
		charTypesAutoTable.SetCharTrimOption(Table::TRIM_RIGHT);

		std::string str;

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

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 5") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(3, str));
		EXPECT_EQ(std::string("abc"), str);
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(4, str));
		EXPECT_EQ(std::string("abc"), str);

		EXPECT_TRUE(charTypesAutoTable.Select((boost::wformat(L"%d = 6") % idName).str()));
		EXPECT_TRUE(charTypesAutoTable.SelectNext());
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(3, str));
		EXPECT_EQ(std::string("abcde12345"), str);
		EXPECT_TRUE(charTypesAutoTable.GetColumnValue(4, str));
		EXPECT_EQ(std::string("abcde12345"), str);

		EXPECT_TRUE(charTypesAutoTable.SelectClose());
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
		EXPECT_EQ(std::string("הצüאיט"), trim_right_copy(str));
		EXPECT_FALSE(cTable.SelectNext());
		EXPECT_TRUE(cTable.SelectClose());

		EXPECT_TRUE(cTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_char);
		EXPECT_EQ(std::string("הצüאיט"), trim_right_copy(str));
		EXPECT_FALSE(cTable.SelectNext());
		EXPECT_TRUE(cTable.SelectClose());

		EXPECT_TRUE(cTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_char_10);
		EXPECT_EQ(std::string("abc"), trim_right_copy(str));
		str.assign((char*)&cTable.m_varchar_10);
		EXPECT_EQ(std::string("abc"), trim_right_copy(str));
		EXPECT_TRUE(cTable.SelectClose());

		EXPECT_TRUE(cTable.Select((boost::wformat(L"%s = 6") % idName).str()));
		EXPECT_TRUE(cTable.SelectNext());
		str.assign((char*)&cTable.m_char_10);
		EXPECT_EQ(std::string("abcde12345"), trim_right_copy(str));
		str.assign((char*)&cTable.m_varchar_10);
		EXPECT_EQ(std::string("abcde12345"), trim_right_copy(str));
		EXPECT_TRUE(cTable.SelectClose());
	}


	TEST_P(TableTest, GetAutoDateValues)
	{
		// Note how to read fractions:
		// [b]   The value of the fraction field is the number of billionths of a second and ranges from 0 through 999,999,999 (1 less than 1 billion). 
		// For example, the value of the fraction field for a half-second is 500,000,000, for a thousandth of a second (one millisecond) is 1,000,000, 
		// for a millionth of a second (one microsecond) is 1,000, and for a billionth of a second (one nanosecond) is 1.

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
		{
			// Microsoft will info here (which we report as warning) that the time field has been truncated (as we do not use the fancy TIME2 struct)
			LogLevelError llE;
			EXPECT_TRUE(dTable.SelectNext());
		}
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

		// Test some digits stuff
		EXPECT_TRUE(dTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(dTable.SelectNext());
		EXPECT_TRUE(dTable.GetColumnValue(3, timestamp));
		// In IBM DB2 we have 6 digits for the fractions of a timestamp 123456 turns into 123'456'000
		if (m_db.Dbms() == dbmsDB2)
		{
			EXPECT_EQ(123456000, timestamp.fraction);
		}
		else if (m_db.Dbms() == dbmsMS_SQL_SERVER)
		{
			// ms has 3 digits 123 turns into 123'000'000
			EXPECT_EQ(123000000, timestamp.fraction);
		}
		else
		{
			EXPECT_EQ(0, timestamp.fraction);
		}

		// MS SQL Server has a new SQL_TIME2_STRUCT if ODBC version is >= 3.8
#if HAVE_MSODBCSQL_H
		Environment env38(OV_3_8);
		EXPECT_TRUE(env38.HasHenv());
		Database db38(env38);
		EXPECT_TRUE(db38.HasHdbc());
		if (m_db.Dbms() == dbmsMY_SQL)
		{
			// My SQL does not support 3.8, the database will warn about a version-mismatch and fall back to 3.0. we know about that.
			// TODO: Open a ticket
			LogLevelError llE;
			EXPECT_TRUE(db38.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
		else
		{
			EXPECT_TRUE(db38.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
		Table dTable38(&db38, dateTypesTableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(dTable38.Open(false, true));
		EXPECT_TRUE(dTable38.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(dTable38.SelectNext());
		EXPECT_TRUE(dTable38.GetColumnValue(2, time));
		EXPECT_EQ(13, time.hour);
		EXPECT_EQ(55, time.minute);
		EXPECT_EQ(56, time.second);

		// We should be able to read the value as TIME2 struct
		SQL_SS_TIME2_STRUCT t2;
		EXPECT_TRUE(dTable38.GetColumnValue(2, t2));
		EXPECT_EQ(13, t2.hour);
		EXPECT_EQ(55, t2.minute);
		EXPECT_EQ(56, t2.second);
		if (db38.Dbms() == dbmsMS_SQL_SERVER)
		{
			// MS should also have fractions, configurable, here set to 7: 1234567 -> 123'456'700
			EXPECT_TRUE(db38.GetMaxSupportedOdbcVersion() >= OV_3_8);
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


	TEST_P(TableTest, GetManualDateValues)
	{
		MDateTypesTable cTable(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(cTable.Open(false, true));

		// Just test the values in the buffer - trimming has no effect here
		using namespace boost::algorithm;
		std::string str;
		std::wstring idName = TestTables::GetColName(L"iddatetypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(cTable.Select((boost::wformat(L"%s = 1") % idName).str()));
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


	TEST_P(TableTest, GetWCharDateValues)
	{
		std::wstring dateTypesTableName = TestTables::GetTableName(L"datetypes", m_odbcInfo.m_namesCase);
		Table dTable(&m_db, dateTypesTableName, L"", L"", L"", Table::READ_ONLY);
		dTable.SetAutoBindingMode(AutoBindingMode::BIND_ALL_AS_WCHAR);
		EXPECT_TRUE(dTable.Open(false, true));

		wstring sDate, sTime, sTimestamp;
		wstring idName = TestTables::GetColName(L"iddatetypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(dTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(dTable.SelectNext());
		EXPECT_TRUE(dTable.GetColumnValue(1, sDate));
		EXPECT_EQ(L"1983-01-26", sDate);

		EXPECT_TRUE(dTable.GetColumnValue(2, sTime));
		EXPECT_TRUE(dTable.GetColumnValue(3, sTimestamp));

		if (m_db.Dbms() == dbmsDB2)
		{
			EXPECT_EQ(L"13:55:56", sTime);
			EXPECT_EQ(L"1983-01-26 13:55:56.000000", sTimestamp);
		}
		else if (m_db.Dbms() == dbmsMS_SQL_SERVER)
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


	TEST_P(TableTest, GetAutoBlobValues)
	{
		std::wstring blobTypesTableName = TestTables::GetTableName(L"blobtypes", m_odbcInfo.m_namesCase);
		Table bTable(&m_db, blobTypesTableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(bTable.Open(false, true));

		wstring idName = TestTables::GetColName(L"idblobtypes", m_odbcInfo.m_namesCase);

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
		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_TRUE(bTable.GetBuffer(1, pBlob, size, length));
		EXPECT_EQ(0, memcmp(empty, pBlob, length));
		EXPECT_EQ(16, size);
		EXPECT_EQ(sizeof(empty), length);
		EXPECT_TRUE(bTable.IsColumnNull(2));

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_TRUE(bTable.GetBuffer(1, pBlob, size, length));
		EXPECT_EQ(0, memcmp(ff, pBlob, length));
		EXPECT_EQ(16, size);
		EXPECT_EQ(sizeof(ff), length);
		EXPECT_TRUE(bTable.IsColumnNull(2));

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_TRUE(bTable.GetBuffer(1, pBlob, size, length));
		EXPECT_EQ(0, memcmp(abc, pBlob, length));
		EXPECT_EQ(16, size);
		EXPECT_EQ(sizeof(abc), length);
		EXPECT_TRUE(bTable.IsColumnNull(2));

		// Read Varbins
		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_TRUE(bTable.GetBuffer(2, pBlob, size, length));
		EXPECT_EQ(0, memcmp(abc, pBlob, length));
		EXPECT_EQ(20, size);
		EXPECT_EQ(sizeof(abc), length);
		EXPECT_TRUE(bTable.IsColumnNull(1));

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_TRUE(bTable.GetBuffer(2, pBlob, size, length));
		EXPECT_EQ(0, memcmp(abc_ff, pBlob, length));
		EXPECT_EQ(20, size);
		EXPECT_EQ(sizeof(abc_ff), length);
		EXPECT_TRUE(bTable.IsColumnNull(1));
	}


	TEST_P(TableTest, GetManualBlobValues)
	{
		MBlobTypesTable bTable(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(bTable.Open(false, true));

		wstring idName = TestTables::GetColName(L"idblobtypes", m_odbcInfo.m_namesCase);

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

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(empty, bTable.m_blob, sizeof(bTable.m_blob)));

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(ff, bTable.m_blob, sizeof(bTable.m_blob)));

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(abc, bTable.m_blob, sizeof(bTable.m_blob)));

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(abc, bTable.m_varblob_20, sizeof(abc)));

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(abc_ff, bTable.m_varblob_20, sizeof(abc_ff)));

	}


	TEST_P(TableTest, IsNullAutoNumericValue)
	{
		std::wstring numericTypesTableName = TestTables::GetTableName(L"numerictypes", m_odbcInfo.m_namesCase);
		Table nTable(&m_db, numericTypesTableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(nTable.Open(false, true));

		wstring idName = TestTables::GetColName(L"idnumerictypes", m_odbcInfo.m_namesCase);
		const ColumnBuffer* pColId = nTable.GetColumnBuffer(0);
		const ColumnBuffer* pColBuff18_00 = nTable.GetColumnBuffer(1);
		const ColumnBuffer* pColBuff18_10 = nTable.GetColumnBuffer(2);
		const ColumnBuffer* pColBuff5_3 = nTable.GetColumnBuffer(3);
		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_FALSE(pColBuff5_3->IsNull());

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_TRUE(pColBuff18_00->IsNull());
		EXPECT_FALSE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_TRUE(pColBuff18_00->IsNull());
		EXPECT_FALSE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 6") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_TRUE(pColBuff18_00->IsNull());
		EXPECT_FALSE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());
	}


	TEST_P(TableTest, GetAutoNumericValue)
	{
		// \note: There is a special Tests for NULL values, its complicated enough.
		std::wstring numericTypesTableName = TestTables::GetTableName(L"numerictypes", m_odbcInfo.m_namesCase);
		Table nTable(&m_db, numericTypesTableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(nTable.Open(false, true));

		wstring idName = TestTables::GetColName(L"idnumerictypes", m_odbcInfo.m_namesCase);
		SQL_NUMERIC_STRUCT numStr;
		const ColumnBuffer* pColBuff18_00 = nTable.GetColumnBuffer(1);
		const ColumnBuffer* pColBuff18_10 = nTable.GetColumnBuffer(2);
		const ColumnBuffer* pColBuff5_3 = nTable.GetColumnBuffer(3);
		SQLBIGINT ex;
		SQLBIGINT* p;

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_TRUE(nTable.GetColumnValue(1, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(0, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 0;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_TRUE(nTable.GetColumnValue(1, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(0, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);
		// Col 3 has a value here too.
		EXPECT_TRUE(nTable.GetColumnValue(3, numStr));
		EXPECT_EQ(5, numStr.precision);
		EXPECT_EQ(3, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 12345;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_TRUE(nTable.GetColumnValue(1, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(0, numStr.scale);
		EXPECT_EQ(0, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_TRUE(nTable.GetColumnValue(2, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(10, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 0;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_TRUE(nTable.GetColumnValue(2, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(10, numStr.scale);
		EXPECT_EQ(1, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 6") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_TRUE(nTable.GetColumnValue(2, numStr));
		EXPECT_EQ(18, numStr.precision);
		EXPECT_EQ(10, numStr.scale);
		EXPECT_EQ(0, numStr.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&numStr.val;
		EXPECT_EQ(ex, *p);
	}


	TEST_P(TableTest, GetManualNumericValue)
	{
		MNumericTypesTable nTable(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(nTable.Open(false, true));

		SQLBIGINT ex;
		SQLBIGINT* p;
		wstring idName = TestTables::GetColName(L"idNumericTypes", m_odbcInfo.m_namesCase); 
		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_EQ(18, nTable.m_decimal_18_0.precision);
		EXPECT_EQ(0, nTable.m_decimal_18_0.scale);
		EXPECT_EQ(1, nTable.m_decimal_18_0.sign);
		ex = 0;
		p = (SQLBIGINT*)&nTable.m_decimal_18_0.val;
		EXPECT_EQ(ex, *p);

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 2") % idName).str()));
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

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_EQ(18, nTable.m_decimal_18_0.precision);
		EXPECT_EQ(0, nTable.m_decimal_18_0.scale);
		EXPECT_EQ(0, nTable.m_decimal_18_0.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&nTable.m_decimal_18_0.val;
		EXPECT_EQ(ex, *p);

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_EQ(18, nTable.m_decimal_18_10.precision);
		EXPECT_EQ(10, nTable.m_decimal_18_10.scale);
		EXPECT_EQ(1, nTable.m_decimal_18_10.sign);
		ex = 0;
		p = (SQLBIGINT*)&nTable.m_decimal_18_10.val;
		EXPECT_EQ(ex, *p);

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_EQ(18, nTable.m_decimal_18_10.precision);
		EXPECT_EQ(10, nTable.m_decimal_18_10.scale);
		EXPECT_EQ(1, nTable.m_decimal_18_10.sign);
		ex = 123456789012345678;
		p = (SQLBIGINT*)&nTable.m_decimal_18_10.val;
		EXPECT_EQ(ex, *p);

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 6") % idName).str()));
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
		std::wstring numericTypesTableName = TestTables::GetTableName(L"numerictypes", m_odbcInfo.m_namesCase);
		Table nTable(&m_db, numericTypesTableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(nTable.Open(false, true));

		wstring idName = TestTables::GetColName(L"idnumerictypes", m_odbcInfo.m_namesCase);
		const ColumnBuffer* pColId = nTable.GetColumnBuffer(0);
		const ColumnBuffer* pColBuff18_00 = nTable.GetColumnBuffer(1);
		const ColumnBuffer* pColBuff18_10 = nTable.GetColumnBuffer(2);
		const ColumnBuffer* pColBuff5_3 = nTable.GetColumnBuffer(3);
		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_FALSE(pColBuff5_3->IsNull());

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_FALSE(pColBuff18_00->IsNull());
		EXPECT_TRUE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 4") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_TRUE(pColBuff18_00->IsNull());
		EXPECT_FALSE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		EXPECT_FALSE(pColId->IsNull());
		EXPECT_TRUE(pColBuff18_00->IsNull());
		EXPECT_FALSE(pColBuff18_10->IsNull());
		EXPECT_TRUE(pColBuff5_3->IsNull());

		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 6") % idName).str()));
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
		std::wstring intTypesTableName = TestTables::GetTableName(L"integertypes_tmp", m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", Table::READ_WRITE);
		ASSERT_TRUE(iTable.Open(false, true));

		wstring idName = TestTables::GetColName(L"idintegertypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Note: We could also do this in one transaction.
		// \todo: Write a separate transaction test about this, to check transaction-visiblity
		// Try to delete eventually available leftovers, ignore if none exists
		EXPECT_TRUE(iTable.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

		// Now lets insert some data:
		ColumnBuffer* pId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt = iTable.GetColumnBuffer(1);
		ColumnBuffer* pInt = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBigInt = iTable.GetColumnBuffer(3);

		*pId = (SQLINTEGER)103;
		pSmallInt->SetNull();
		pInt->SetNull();
		pBigInt->SetNull();
		EXPECT_TRUE(iTable.Insert());
		EXPECT_TRUE(m_db.CommitTrans());

		// Now we must have something to delete
		EXPECT_TRUE(iTable.Delete(sqlstmt, true));
		EXPECT_TRUE(m_db.CommitTrans());

		// And fetching shall return no results at all
		EXPECT_TRUE(iTable.Select());
		EXPECT_FALSE(iTable.SelectNext());
	}


	TEST_P(TableTest, DeleteWhereShouldReturnSQL_NO_DATA)
	{
		std::wstring intTypesTableName = TestTables::GetTableName(L"integertypes_tmp", m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", Table::READ_WRITE);
		ASSERT_TRUE(iTable.Open(false, true));

		wstring idName = TestTables::GetColName(L"idintegertypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Try to delete eventually available leftovers, ignore if none exists
		EXPECT_TRUE(iTable.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());
		// We can be sure now that nothing exists, and we will fail if we try to delete
		EXPECT_FALSE(iTable.Delete(sqlstmt, true));
		EXPECT_TRUE(m_db.CommitTrans());

		// And fetching shall return no results at all
		EXPECT_TRUE(iTable.Select());
		EXPECT_FALSE(iTable.SelectNext());
	}


	TEST_P(TableTest, Delete)
	{
		std::wstring intTypesTableName = TestTables::GetTableName(L"integertypes_tmp", m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", Table::READ_WRITE);
		ASSERT_TRUE(iTable.Open(false, true));

		wstring idName = TestTables::GetColName(L"idintegertypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		EXPECT_TRUE(iTable.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

		// Insert a row that we want to delete
		ColumnBuffer* pId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt = iTable.GetColumnBuffer(1);
		ColumnBuffer* pInt = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBigInt = iTable.GetColumnBuffer(3);
		pSmallInt->SetNull();
		pInt->SetNull();
		pBigInt->SetNull();
		*pId = (SQLINTEGER)99;
		EXPECT_TRUE(iTable.Insert());
		EXPECT_TRUE(m_db.CommitTrans());

		// Now lets delete that row. our pId is still set to the key-value 99
		EXPECT_TRUE(iTable.Delete());

		// And fetching shall return no results at all
		EXPECT_TRUE(iTable.Select());
		EXPECT_FALSE(iTable.SelectNext());
	}


	// Insert rows
	// -----------
	TEST_P(TableTest, InsertIntTypes)
	{
		std::wstring intTypesTableName = TestTables::GetTableName(L"integertypes_tmp", m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", Table::READ_WRITE);
		ASSERT_TRUE(iTable.Open(false, true));
		ColumnBuffer* pId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt = iTable.GetColumnBuffer(1);
		ColumnBuffer* pInt = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBigInt = iTable.GetColumnBuffer(3);

		wstring idName = TestTables::GetColName(L"idintegertypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		EXPECT_TRUE(iTable.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

		// Set some silly values to insert
		*pId = (SQLINTEGER)101;
		*pSmallInt = (SQLSMALLINT)102;
		*pInt = (SQLINTEGER)103;
		*pBigInt = (SQLBIGINT)104;

		EXPECT_TRUE(iTable.Insert());
		EXPECT_TRUE(m_db.CommitTrans());

		// Open another table and read the values from there
		Table iTable2(&m_db, intTypesTableName, L"", L"", L"", Table::READ_WRITE);
		ASSERT_TRUE(iTable2.Open(false, true));
		ColumnBuffer* pId2 = iTable2.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt2 = iTable2.GetColumnBuffer(1);
		ColumnBuffer* pInt2 = iTable2.GetColumnBuffer(2);
		ColumnBuffer* pBigInt2 = iTable2.GetColumnBuffer(3);

		EXPECT_TRUE(iTable2.Select());
		EXPECT_TRUE(iTable2.SelectNext());

		EXPECT_EQ(101, (SQLINTEGER)*pId2);
		EXPECT_EQ(102, (SQLSMALLINT)*pSmallInt2);
		EXPECT_EQ(103, (SQLINTEGER)*pInt2);
		EXPECT_EQ(104, (SQLBIGINT)*pBigInt2);
	}


	TEST_P(TableTest, InsertDateTypes)
	{
		std::wstring dateTypesTableName = TestTables::GetTableName(L"datetypes_tmp", m_odbcInfo.m_namesCase);
		Table dTable(&m_db, dateTypesTableName, L"", L"", L"", Table::READ_WRITE);
		ASSERT_TRUE(dTable.Open(false, true));
		ColumnBuffer* pId = dTable.GetColumnBuffer(0);
		ColumnBuffer* pDate = dTable.GetColumnBuffer(1);
		ColumnBuffer* pTime = dTable.GetColumnBuffer(2);
		ColumnBuffer* pTimestamp = dTable.GetColumnBuffer(3);

		wstring idName = TestTables::GetColName(L"iddatetypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		EXPECT_TRUE(dTable.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

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
		if (m_db.Dbms() != dbmsMY_SQL)
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
		EXPECT_TRUE(dTable.Insert());
		EXPECT_TRUE(m_db.CommitTrans());

		// Open another table and read the values from there
		Table dTable2(&m_db, dateTypesTableName, L"", L"", L"", Table::READ_WRITE);
		ASSERT_TRUE(dTable2.Open(false, true));
		ColumnBuffer* pId2 = dTable2.GetColumnBuffer(0);
		ColumnBuffer* pDate2 = dTable2.GetColumnBuffer(1);
		ColumnBuffer* pTime2 = dTable2.GetColumnBuffer(2);
		ColumnBuffer* pTimestamp2 = dTable2.GetColumnBuffer(3);

		EXPECT_TRUE(dTable2.Select());
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
		std::wstring floatTypesTableName = TestTables::GetTableName(L"floattypes_tmp", m_odbcInfo.m_namesCase);
		Table fTable(&m_db, floatTypesTableName, L"", L"", L"", Table::READ_WRITE);
		ASSERT_TRUE(fTable.Open(false, true));
		ColumnBuffer* pId = fTable.GetColumnBuffer(0);
		ColumnBuffer* pDouble = fTable.GetColumnBuffer(1);
		ColumnBuffer* pFloat = fTable.GetColumnBuffer(2);

		wstring idName = TestTables::GetColName(L"idfloattypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Remove everything, ignoring if there was any data:
		EXPECT_TRUE(fTable.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

		// Insert some values
		*pId = (SQLINTEGER)101;
		*pDouble = 3.14159265359;
		*pFloat = -3.14159;
		EXPECT_TRUE(fTable.Insert());
		EXPECT_TRUE(m_db.CommitTrans());

		// Open another table and read the values from there
		Table fTable2(&m_db, floatTypesTableName, L"", L"", L"", Table::READ_WRITE);
		ASSERT_TRUE(fTable2.Open(false, true));
		ColumnBuffer* pId2 = fTable2.GetColumnBuffer(0);
		ColumnBuffer* pDouble2 = fTable2.GetColumnBuffer(1);
		ColumnBuffer* pFloat2 = fTable2.GetColumnBuffer(2);

		EXPECT_TRUE(fTable2.Select());
		EXPECT_TRUE(fTable2.SelectNext());

		EXPECT_EQ(101, (SQLINTEGER)*pId2);
		EXPECT_EQ((SQLDOUBLE)*pDouble, (SQLDOUBLE)*pDouble2);
		EXPECT_EQ((SQLDOUBLE)*pFloat, (SQLDOUBLE)*pFloat2);
	}


	TEST_P(TableTest, InsertNumericTypes)
	{
		std::wstring numericTypesTmpTableName = TestTables::GetTableName(L"numerictypes_tmp", m_odbcInfo.m_namesCase);
		Table nTable(&m_db, numericTypesTmpTableName, L"", L"", L"", Table::READ_WRITE);
		ASSERT_TRUE(nTable.Open(false, true));
		ColumnBuffer* pId = nTable.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = nTable.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = nTable.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = nTable.GetColumnBuffer(3);

		wstring idName = TestTables::GetColName(L"idnumerictypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		// Select a valid record from the non-tmp table
		std::wstring numericTypesTableName = TestTables::GetTableName(L"numerictypes", m_odbcInfo.m_namesCase);
		Table nnTable(&m_db, numericTypesTableName, L"", L"", L"", Table::READ_WRITE);
		EXPECT_TRUE(nnTable.Open(false, true));
		SQL_NUMERIC_STRUCT numStr18_0, numStr18_10, numStr5_3;
		EXPECT_TRUE(nnTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(nnTable.SelectNext());
		EXPECT_TRUE(nnTable.GetColumnValue(1, numStr18_0));
		EXPECT_TRUE(nnTable.GetColumnValue(3, numStr5_3));
		EXPECT_TRUE(nnTable.Select((boost::wformat(L"%s = 5") % idName).str()));
		EXPECT_TRUE(nnTable.SelectNext());
		EXPECT_TRUE(nnTable.GetColumnValue(2, numStr18_10));
		EXPECT_TRUE(nnTable.SelectClose());

		// Remove everything, ignoring if there was any data:
		EXPECT_TRUE(nTable.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

		// Insert the just read values
		*pId = (SQLINTEGER)101;
		*pNumeric_18_0 = numStr18_0;
		*pNumeric_18_10 = numStr18_10;
		*pNumeric_5_3 = numStr5_3;
		EXPECT_TRUE(nTable.Insert());
		EXPECT_TRUE(m_db.CommitTrans());

		// And read them again from another table and compare them
		SQL_NUMERIC_STRUCT numStr18_0t, numStr18_10t, numStr5_3t;
		Table nntTable(&m_db, numericTypesTmpTableName, L"", L"", L"", Table::READ_WRITE);
		EXPECT_TRUE(nntTable.Open(false, true));
		sqlstmt = (boost::wformat(L"%s = %d") % idName % (SQLINTEGER)*pId).str();
		EXPECT_TRUE(nntTable.Select(sqlstmt));
		EXPECT_TRUE(nntTable.SelectNext());
		EXPECT_TRUE(nntTable.GetColumnValue(1, numStr18_0t));
		EXPECT_TRUE(nntTable.GetColumnValue(2, numStr18_10t));
		EXPECT_TRUE(nntTable.GetColumnValue(3, numStr5_3t));
		EXPECT_TRUE(nntTable.SelectClose());
		EXPECT_EQ(0, memcmp(&numStr18_0, &numStr18_0t, sizeof(numStr18_0)));
		EXPECT_EQ(0, memcmp(&numStr18_10, &numStr18_10t, sizeof(numStr18_10)));
		EXPECT_EQ(0, memcmp(&numStr5_3, &numStr5_3t, sizeof(numStr5_3)));
	}


	TEST_P(TableTest, InsertNumericTypes_5_3)
	{
		std::wstring numericTypesTmpTableName = TestTables::GetTableName(L"numerictypes_tmp", m_odbcInfo.m_namesCase);
		Table t(&m_db, numericTypesTmpTableName, L"", L"", L"", Table::READ_WRITE);

		ASSERT_TRUE(t.Open(false, true));
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetColName(L"idnumerictypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") %idName).str();
		EXPECT_TRUE(t.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

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
		EXPECT_TRUE(t.Insert());
		EXPECT_TRUE(m_db.CommitTrans());
	}


	TEST_P(TableTest, InsertNumericTypes_18_0)
	{
		std::wstring numericTypesTmpTableName = TestTables::GetTableName(L"numerictypes_tmp", m_odbcInfo.m_namesCase);
		Table t(&m_db, numericTypesTmpTableName, L"", L"", L"", Table::READ_WRITE);

		ASSERT_TRUE(t.Open(false, true));
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetColName(L"idnumerictypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		EXPECT_TRUE(t.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

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
		EXPECT_TRUE(t.Insert());
		EXPECT_TRUE(m_db.CommitTrans());
	}


	TEST_P(TableTest, InsertNumericTypes_18_10)
	{
		std::wstring numericTypesTmpTableName = TestTables::GetTableName(L"numerictypes_tmp", m_odbcInfo.m_namesCase);
		Table t(&m_db, numericTypesTmpTableName, L"", L"", L"", Table::READ_WRITE);

		ASSERT_TRUE(t.Open(false, true));
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetColName(L"idnumerictypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();

		EXPECT_TRUE(t.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

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
		EXPECT_TRUE(t.Insert());
		EXPECT_TRUE(m_db.CommitTrans());
	}


	TEST_P(TableTest, InsertNumericTypes_All)
	{
		std::wstring numericTypesTmpTableName = TestTables::GetTableName(L"numerictypes_tmp", m_odbcInfo.m_namesCase);
		Table t(&m_db, numericTypesTmpTableName, L"", L"", L"", Table::READ_WRITE);

		ASSERT_TRUE(t.Open(false, true));
		ColumnBuffer* pId = t.GetColumnBuffer(0);
		ColumnBuffer* pNumeric_18_0 = t.GetColumnBuffer(1);
		ColumnBuffer* pNumeric_18_10 = t.GetColumnBuffer(2);
		ColumnBuffer* pNumeric_5_3 = t.GetColumnBuffer(3);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetColName(L"idnumerictypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		EXPECT_TRUE(t.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

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
		EXPECT_TRUE(t.Insert());
		EXPECT_TRUE(m_db.CommitTrans());
	}


	TEST_P(TableTest, InsertBlobTypes)
	{
		std::wstring blobTypesTmpTableName = TestTables::GetTableName(L"blobtypes_tmp", m_odbcInfo.m_namesCase);
		Table bTable(&m_db, blobTypesTmpTableName, L"", L"", L"", Table::READ_WRITE);
		EXPECT_TRUE(bTable.Open(false, true));

		ColumnBuffer* pId = bTable.GetColumnBuffer(0);
		ColumnBuffer* pBlob = bTable.GetColumnBuffer(1);
		ColumnBuffer* pVarBlob_20 = bTable.GetColumnBuffer(2);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetColName(L"idblobtypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		EXPECT_TRUE(bTable.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

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
		EXPECT_TRUE(bTable.Insert());
		*pId = (SQLINTEGER)101;
		pBlob->SetBinaryValue(ff, sizeof(ff));
		EXPECT_TRUE(bTable.Insert());
		*pId = (SQLINTEGER)102;
		pBlob->SetBinaryValue(abc, sizeof(abc));
		EXPECT_TRUE(bTable.Insert());
		*pId = (SQLINTEGER)103;
		pBlob->SetNull();
		pVarBlob_20->SetBinaryValue(abc, sizeof(abc));
		EXPECT_TRUE(bTable.Insert());
		*pId = (SQLINTEGER)104;
		pVarBlob_20->SetBinaryValue(abc_ff, sizeof(abc_ff));
		EXPECT_TRUE(bTable.Insert());
		EXPECT_TRUE(m_db.CommitTrans());

		// Now read the inserted values
		sqlstmt = (boost::wformat(L"%s = 100") % idName).str();
		EXPECT_TRUE(bTable.Select(sqlstmt));
		EXPECT_TRUE(bTable.SelectNext());
		const SQLCHAR* pEmpty = *pBlob;
		EXPECT_EQ(0, memcmp(empty, pEmpty, sizeof(empty)));

		sqlstmt = (boost::wformat(L"%s = 101") % idName).str();
		EXPECT_TRUE(bTable.Select(sqlstmt));
		EXPECT_TRUE(bTable.SelectNext());
		const SQLCHAR* pFf = *pBlob;
		EXPECT_EQ(0, memcmp(ff, pFf, sizeof(ff)));

		sqlstmt = (boost::wformat(L"%s = 102") % idName).str();
		EXPECT_TRUE(bTable.Select(sqlstmt));
		EXPECT_TRUE(bTable.SelectNext());
		const SQLCHAR* pAbc = *pBlob;
		EXPECT_EQ(0, memcmp(abc, pAbc, sizeof(abc)));

		sqlstmt = (boost::wformat(L"%s = 103") % idName).str();
		EXPECT_TRUE(bTable.Select(sqlstmt));
		EXPECT_TRUE(bTable.SelectNext());
		pAbc = *pVarBlob_20;
		EXPECT_EQ(0, memcmp(abc, pAbc, sizeof(abc)));
		EXPECT_EQ(sizeof(abc), pVarBlob_20->GetCb());

		sqlstmt = (boost::wformat(L"%s = 104") % idName).str();
		EXPECT_TRUE(bTable.Select(sqlstmt));
		EXPECT_TRUE(bTable.SelectNext());
		const SQLCHAR* pAbc_ff = *pVarBlob_20;
		EXPECT_EQ(0, memcmp(abc_ff, pAbc_ff, sizeof(abc_ff)));
		EXPECT_EQ(sizeof(abc_ff), pVarBlob_20->GetCb());
	}


	TEST_P(TableTest, InsertCharTypes)
	{
		std::wstring charTypesTmpTableName = TestTables::GetTableName(L"chartypes_tmp", m_odbcInfo.m_namesCase);
		Table cTable(&m_db, charTypesTmpTableName, L"", L"", L"", Table::READ_WRITE);
		cTable.SetAutoBindingMode(AutoBindingMode::BIND_WCHAR_AS_CHAR);
		EXPECT_TRUE(cTable.Open(false, true));

		ColumnBuffer* pId = cTable.GetColumnBuffer(0);
		ColumnBuffer* pVarchar = cTable.GetColumnBuffer(1);
		ColumnBuffer* pChar = cTable.GetColumnBuffer(2);
		ColumnBuffer* pVarchar_10 = cTable.GetColumnBuffer(3);
		ColumnBuffer* pChar_10 = cTable.GetColumnBuffer(4);

		// Remove everything, ignoring if there was any data:
		wstring idName = TestTables::GetColName(L"idchartypes", m_odbcInfo.m_namesCase);
		wstring sqlstmt = (boost::wformat(L"%s > 0") % idName).str();
		EXPECT_TRUE(cTable.Delete(sqlstmt, false));
		EXPECT_TRUE(m_db.CommitTrans());

		// Insert some values:
		std::string s = "Hello World!";
		*pId = (SQLINTEGER)100;
		*pVarchar = s;
		*pChar = s;
		pVarchar_10->SetNull();
		pChar_10->SetNull();
		EXPECT_TRUE(cTable.Insert());
		EXPECT_TRUE(m_db.CommitTrans());

		// Insert one value that uses all space
		s = "abcde12345";
		*pId = (SQLINTEGER)101;
		pVarchar->SetNull();
		pChar->SetNull();
		*pVarchar_10 = s;
		*pChar_10 = s;
		EXPECT_TRUE(cTable.Insert());
		EXPECT_TRUE(m_db.CommitTrans());


		// Read them back from another table
		s = "Hello World!";
		Table t2(&m_db, charTypesTmpTableName, L"", L"", L"", Table::READ_WRITE);
		t2.SetAutoBindingMode(AutoBindingMode::BIND_WCHAR_AS_CHAR);
		t2.SetCharTrimOption(Table::TRIM_RIGHT);
		EXPECT_TRUE(t2.Open(false, true));

		std::string varchar2, char2;
		EXPECT_TRUE(t2.Select((boost::wformat(L"%s = 100") % idName).str()));
		EXPECT_TRUE(t2.SelectNext());
		EXPECT_TRUE(t2.GetColumnValue(1, varchar2));
		EXPECT_TRUE(t2.GetColumnValue(2, char2));
		EXPECT_TRUE(t2.IsColumnNull(3));
		EXPECT_TRUE(t2.IsColumnNull(4));
		EXPECT_EQ(s, varchar2);
		EXPECT_EQ(s, char2);

		s = "abcde12345";
		EXPECT_TRUE(t2.Select((boost::wformat(L"%s = 101") % idName).str()));
		EXPECT_TRUE(t2.SelectNext());
		EXPECT_TRUE(t2.IsColumnNull(1));
		EXPECT_TRUE(t2.IsColumnNull(2));
		EXPECT_TRUE(t2.GetColumnValue(3, varchar2));
		EXPECT_TRUE(t2.GetColumnValue(4, char2));
		EXPECT_EQ(s, varchar2);
		EXPECT_EQ(s, char2);
	}


	TEST_P(TableTest, DISABLED_InsertWCharTypes)
	{

	}

	// Update rows
	// -----------

	TEST_P(TableTest, DISABLED_UpdateIntTypes)
	{
		// Clear tmp-table
		// hm.. we need to do that first.. if not we have to type sql-syntax over and over..
		// \todo: We need to test to insert NULL values, that is not handled at all so far.

		std::wstring intTypesTableName = TestTables::GetTableName(L"integertypes_tmp", m_odbcInfo.m_namesCase);
		Table iTable(&m_db, intTypesTableName, L"", L"", L"", Table::READ_WRITE);
		ASSERT_TRUE(iTable.Open(false, true));

		// Set some silly values to update, but dont touch the id
		ColumnBuffer* pId = iTable.GetColumnBuffer(0);
		ColumnBuffer* pSmallInt = iTable.GetColumnBuffer(1);
		ColumnBuffer* pInt = iTable.GetColumnBuffer(2);
		ColumnBuffer* pBigInt = iTable.GetColumnBuffer(3);
		*pId = (SQLINTEGER)1;
		*pSmallInt = (SQLSMALLINT)100;
		*pInt = (SQLINTEGER)101;
		*pBigInt = (SQLBIGINT)102;

		EXPECT_TRUE(iTable.Update());
		EXPECT_TRUE(m_db.CommitTrans());
	}



// Interfaces
// ----------

} // namespace exodbc
