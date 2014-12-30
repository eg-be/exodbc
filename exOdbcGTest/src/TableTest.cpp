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

		EXPECT_FALSE(charTypesAutoTable.SelectNext());
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

		const SQLCHAR* pBlob = NULL;
		SQLINTEGER size = 0;

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_TRUE(bTable.GetBuffer(1, pBlob, size));
		EXPECT_EQ(0, memcmp(empty, pBlob, size));

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_TRUE(bTable.GetBuffer(1, pBlob, size));
		EXPECT_EQ(0, memcmp(ff, pBlob, size));

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_TRUE(bTable.GetBuffer(1, pBlob, size));
		EXPECT_EQ(0, memcmp(abc, pBlob, size));
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

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(empty, bTable.m_blob, sizeof(bTable.m_blob)));

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(ff, bTable.m_blob, sizeof(bTable.m_blob)));

		EXPECT_TRUE(bTable.Select((boost::wformat(L"%s = 3") % idName).str()));
		EXPECT_TRUE(bTable.SelectNext());
		EXPECT_EQ(0, memcmp(abc, bTable.m_blob, sizeof(bTable.m_blob)));
	}


	TEST_P(TableTest, GetAutoNumericValue)
	{
		std::wstring numericTypesTableName = TestTables::GetTableName(L"numerictypes", m_odbcInfo.m_namesCase);
		Table nTable(&m_db, numericTypesTableName, L"", L"", L"", Table::READ_ONLY);
		EXPECT_TRUE(nTable.Open(false, true));

		wstring idName = TestTables::GetColName(L"idnumerictypes", m_odbcInfo.m_namesCase);
		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 1") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		const SQLCHAR* buff = NULL;
		SQLINTEGER buffSize = 0;
		EXPECT_TRUE(nTable.GetBuffer(1, buff, buffSize));
		SQL_NUMERIC_STRUCT* pNum = (SQL_NUMERIC_STRUCT*)buff;
		int p = 3;

	}


	TEST_P(TableTest, DISABLED_GetTestNumericValue)
	{
//		if (m_db.Dbms() != dbmsMS_SQL_SERVER)
		if (m_db.Dbms() != dbmsMY_SQL)
		{
			//return;
		}

		SQLINTEGER strlen1;
		SQLINTEGER a;

		SQLRETURN ret = 0;
		SQL_NUMERIC_STRUCT numStr;
		SQLHSTMT hstmt = SQL_NULL_HSTMT;
		SQLHDBC hdbc = m_db.GetHDBC();
		SQLHDESC hdesc = SQL_NULL_HDESC;
		ZeroMemory(&numStr, sizeof(numStr));
		ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
		ret = SQLExecDirect(hstmt, L"SELECT idnumerictypes, tdecimal_18_0, tdecimal_18_10  FROM exodbc.numerictypes WHERE idnumerictypes = 2", SQL_NTS);
//		ret = SQLExecDirect(hstmt, L"SELECT idnumerictypes, tdecimal_18_0, tdecimal_18_10  FROM exodbc.numerictypes WHERE idnumerictypes = 5", SQL_NTS);

		// Use SQLBindCol to bind the NumStr to the column that is being retrieved.

//		ret = SQLBindCol(hstmt, 2, SQL_C_NUMERIC, &numStr, 19, &strlen1);

		// Get the application row descriptor for the statement handle using
		//SQLGetStmtAttr.

		ret = SQLGetStmtAttr(hstmt, SQL_ATTR_APP_ROW_DESC, &hdesc, 0, NULL);

		//ret = SQLSetDescField(hdesc, 2, SQL_DESC_TYPE, (VOID*)SQL_C_NUMERIC, 0);
		ret = SQLSetDescField(hdesc, 2, SQL_DESC_PRECISION, (VOID*)18, 0);
		ret = SQLSetDescField(hdesc, 2, SQL_DESC_SCALE, (VOID*)10, 0);

		ret = SQLBindCol(hstmt, 2, SQL_C_NUMERIC, &numStr, 19, &strlen1);

		while ((ret = SQLFetch(hstmt)) != SQL_NO_DATA)
		{
			if (ret != SQL_SUCCESS)
			{
				std::vector<SErrorInfo> errs = GetAllErrors(NULL, NULL, hstmt);
				LOG_ERROR(L"hello");
				//LOG_ERROR_STMT(hstmt, ret, SQLFetch);
			}
			// Notice that the TargetType (3rd Parameter) is SQL_ARD_TYPE, which  
			//forces the driver to use the Application Row Descriptor with the 
			//specified scale and precision.

			ret = SQLGetData(hstmt, 2, SQL_ARD_TYPE, &numStr, 19, &a);

			// Check for null indicator.

			if (SQL_NULL_DATA == a)
			{
				int p = 3;
				printf("The final value: NULL\n");
				continue;
			}

			// Call to convert the little endian mode data into numeric data.

			//long myvalue = strtohextoval();

			//// The returned value in the above code is scaled to the value specified
			////in the scale field of the numeric structure. For example 25.212 would
			////be returned as 25212. The scale in this case is 3 hence the integer 
			////value needs to be divided by 1000.

			//divisor = 1;
			//if (NumStr.scale > 0)
			//{
			//	for (i = 0; i < NumStr.scale; i++)
			//		divisor = divisor * 10;
			//}
			//final_val = (float)myvalue / (float)divisor;

			//// Examine the sign value returned in the sign field for the numeric
			////structure.
			////NOTE: The ODBC 3.0 spec required drivers to return the sign as 
			////1 for positive numbers and 2 for negative number. This was changed in the
			////ODBC 3.5 spec to return 0 for negative instead of 2.

			//if (!NumStr.sign) sign = -1;
			//else sign = 1;

			//final_val *= sign;
			//printf("The final value: %f\n", final_val);
		}

	}

	TEST_P(TableTest, DISABLED_GetManualNumericValue)
	{
		MNumericTypesTable nTable(&m_db, m_odbcInfo.m_namesCase);
		EXPECT_TRUE(nTable.Open(false, true));

		wstring idName = TestTables::GetColName(L"idNumericTypes", m_odbcInfo.m_namesCase); 
		EXPECT_TRUE(nTable.Select((boost::wformat(L"%s = 2") % idName).str()));
		EXPECT_TRUE(nTable.SelectNext());
		char buff1[8];
		char buff2[8];
		ZeroMemory(&buff1, sizeof(buff1));
		ZeroMemory(&buff2, sizeof(buff2));
		memcpy(&buff1, &nTable.m_decimal_18_0.val, sizeof(buff1));
		memcpy(&buff2, &nTable.m_decimal_18_0.val[8], sizeof(buff2));
		SQLUBIGINT* i1 = (SQLUBIGINT*) &buff1;
		SQLUBIGINT* i2 = (SQLUBIGINT*) &buff2;
//		SQLUBIGINT i = (SQLUBIGINT)nTable.m_decimal_18_0.val;
		int p = 3;

	}


// Interfaces
// ----------

} // namespace exodbc
