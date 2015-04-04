/*!
* \file ManualTestTables.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 26.12.2014
* \copyright wxWindows Library Licence, Version 3.1
*
* Declares classes derived from exodbc::Table that define their columns manually.
*/

#include "stdafx.h"

// Own header
#include "ManualTestTables.h"

// Same component headers
// Other headers
#include "Database.h"

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
namespace exodbc
{
	// IntTypesTable
	// ---------------
	MIntTypesTable::MIntTypesTable(const Database& db, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"IntegerTypes" */)
		: Table(db, 4, TestTables::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idIntegerTypes = 0;
		m_smallInt = 0;
		m_int = 0;
		m_bigInt = 0;

		SetColumn(0, TestTables::ConvertNameCase(L"idintegertypes", namesCase), &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes));
		SetColumn(1, TestTables::ConvertNameCase(L"tsmallint", namesCase), &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt));
		SetColumn(2, TestTables::ConvertNameCase(L"tint", namesCase), &m_int, SQL_C_SLONG, sizeof(m_int));
		SetColumn(3, TestTables::ConvertNameCase(L"tbigint", namesCase), &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt));
	}


	// FloatTypesTable
	// ---------------
	MFloatTypesTable::MFloatTypesTable(const Database& db, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"FloatTypes" */)
		: Table(db, 3, TestTables::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idFloatTypes = 0;
		m_double = 0;
		m_float = 0;

		// Note: When binding a column of type FLOAT (database-type), you still need to use SQL_C_DOUBLE
		// SQL_C_FLOAT is for REAL (database-type), which I will not test here, mysql doesnt know about it ? but still reports float as real?
		// TODO: Test it for db-2 specific test (REAL-type? what did I mean?). But do that once we can determine the db-type from the wxDb object itself 
		SetColumn(0, TestTables::ConvertNameCase(L"idfloattypes", namesCase), &m_idFloatTypes, SQL_C_SLONG, sizeof(m_idFloatTypes));
		SetColumn(1, TestTables::ConvertNameCase(L"tdouble", namesCase), &m_double, SQL_C_DOUBLE, sizeof(m_double));
		SetColumn(2, TestTables::ConvertNameCase(L"tfloat", namesCase), &m_float, SQL_C_DOUBLE, sizeof(m_float));
	}


	// CharTypesTable
	// --------------
	MCharTypesTable::MCharTypesTable(const Database& db, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"CharTypes" */)
		: Table(db, 5, TestTables::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idCharTypes = 0;
		m_varchar[0] = 0;
		m_char[0] = 0;
		m_varchar_10[0] = 0;
		m_char_10[0] = 0;

		SetColumn(0, TestTables::ConvertNameCase(L"idchartypes", namesCase), &m_idCharTypes, SQL_C_SLONG, sizeof(m_idCharTypes));
		SetColumn(1, TestTables::ConvertNameCase(L"tvarchar", namesCase), m_varchar, SQL_C_CHAR, sizeof(m_varchar));
		SetColumn(2, TestTables::ConvertNameCase(L"tchar", namesCase), m_char, SQL_C_CHAR, sizeof(m_char));
		SetColumn(3, TestTables::ConvertNameCase(L"tvarchar_10", namesCase), m_varchar_10, SQL_C_CHAR, sizeof(m_varchar_10));
		SetColumn(4, TestTables::ConvertNameCase(L"tchar_10", namesCase), m_char_10, SQL_C_CHAR, sizeof(m_char_10));

	}


	// WCharTypesTable
	// --------------
	MWCharTypesTable::MWCharTypesTable(const Database& db, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"CharTypes" */)
		: Table(db, 5, TestTables::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idCharTypes = 0;
		m_varchar[0] = 0;
		m_char[0] = 0;
		m_varchar_10[0] = 0;
		m_char_10[0] = 0;

		SetColumn(0, TestTables::ConvertNameCase(L"idchartypes", namesCase), &m_idCharTypes, SQL_C_SLONG, sizeof(m_idCharTypes));
		SetColumn(1, TestTables::ConvertNameCase(L"tvarchar", namesCase), m_varchar, SQL_C_WCHAR, sizeof(m_varchar));
		SetColumn(2, TestTables::ConvertNameCase(L"tchar", namesCase), m_char, SQL_C_WCHAR, sizeof(m_char));
		SetColumn(3, TestTables::ConvertNameCase(L"tvarchar_10", namesCase), m_varchar_10, SQL_C_WCHAR, sizeof(m_varchar_10));
		SetColumn(4, TestTables::ConvertNameCase(L"tchar_10", namesCase), m_char_10, SQL_C_WCHAR, sizeof(m_char_10));
	}


	// DateTypesTable
	// ---------------
	MDateTypesTable::MDateTypesTable(const Database& db, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"DateTypes" */)
		: Table(db, 4, TestTables::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idDateTypes = 0;
		ZeroMemory(&m_date, sizeof(m_date));
		ZeroMemory(&m_time, sizeof(m_time));
		ZeroMemory(&m_timestamp, sizeof(m_timestamp));

		// Note: We are odbc 3, therefore use the new c-type (with type: SQL_C_TYPE_DATE instead of SQL_C_DATE).
		SetColumn(0, TestTables::ConvertNameCase(L"iddatetypes", namesCase), &m_idDateTypes, SQL_C_SLONG, sizeof(m_idDateTypes));
		SetColumn(1, TestTables::ConvertNameCase(L"tdate", namesCase), &m_date, SQL_C_TYPE_DATE, sizeof(m_date));
		SetColumn(2, TestTables::ConvertNameCase(L"ttime", namesCase), &m_time, SQL_C_TYPE_TIME, sizeof(m_time));
		SetColumn(3, TestTables::ConvertNameCase(L"ttimestamp", namesCase), &m_timestamp, SQL_C_TYPE_TIMESTAMP, sizeof(m_timestamp));
	}


	// BlobTypesTable
	// --------------
	MBlobTypesTable::MBlobTypesTable(const Database& db, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"BlobTypes" */)
		: Table(db, 3, TestTables::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		ZeroMemory(m_blob, sizeof(m_blob));
		ZeroMemory(m_varblob_20, sizeof(m_varblob_20));

		SetColumn(0, TestTables::ConvertNameCase(L"idblobtypes", namesCase),  &m_idBlobTypes, SQL_C_SLONG, sizeof(m_idBlobTypes));
		SetColumn(1, TestTables::ConvertNameCase(L"tblob", namesCase), m_blob, SQL_C_BINARY, sizeof(m_blob));
		SetColumn(2, TestTables::ConvertNameCase(L"tvarblob_20", namesCase), m_varblob_20, SQL_C_BINARY, sizeof(m_varblob_20));
	}


	// MNumericTypesTable
	// -----------------
	MNumericTypesTable::MNumericTypesTable(const Database& db, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"NumericTypes" */)
		: Table(db, 4, TestTables::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idNumericTypes = 0;

		::ZeroMemory(&m_decimal_18_0, sizeof(m_decimal_18_0));
		::ZeroMemory(&m_decimal_18_10, sizeof(m_decimal_18_10));
		::ZeroMemory(&m_decimal_5_3, sizeof(m_decimal_5_3));

		SetColumn(0, TestTables::ConvertNameCase(L"idnumerictypes", namesCase), &m_idNumericTypes, SQL_C_SLONG, sizeof(m_idNumericTypes));
		SetColumn(1, TestTables::ConvertNameCase(L"tdecimal_18_0", namesCase), &m_decimal_18_0, SQL_C_NUMERIC, sizeof(m_decimal_18_0), CF_SELECT, 18, 0);
		SetColumn(2, TestTables::ConvertNameCase(L"tdecimal_18_10", namesCase), &m_decimal_18_10, SQL_C_NUMERIC, sizeof(m_decimal_18_10), CF_SELECT, 18, 10);
		SetColumn(3, TestTables::ConvertNameCase(L"tdecimal_5_3", namesCase), &m_decimal_5_3, SQL_C_NUMERIC, sizeof(m_decimal_5_3), CF_SELECT, 5, 3);
	};


	// MNumericTypesAsCharTable
	// -----------------
	MNumericTypesAsCharTable::MNumericTypesAsCharTable(const Database& db, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"NumericTypes" */)
		: Table(db, 4, TestTables::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idNumericTypes = 0;
		m_wcdecimal_18_0[0] = 0;
		m_wcdecimal_18_10[0] = 0;
		m_wdecimal_5_3[0] = 0;

		SetColumn(0, TestTables::ConvertNameCase(L"idnumerictypes", namesCase), &m_idNumericTypes, SQL_C_SLONG, sizeof(m_idNumericTypes));
		SetColumn(1, TestTables::ConvertNameCase(L"tdecimal_18_0", namesCase), m_wcdecimal_18_0, SQL_C_WCHAR, sizeof(m_wcdecimal_18_0));
		SetColumn(2, TestTables::ConvertNameCase(L"tdecimal_18_10", namesCase), m_wcdecimal_18_10, SQL_C_WCHAR, sizeof(m_wcdecimal_18_10));
		SetColumn(3, TestTables::ConvertNameCase(L"tdecimal_5_3", namesCase), m_wdecimal_5_3, SQL_C_WCHAR, sizeof(m_wdecimal_5_3));
	};


	// CharTable
	// ---------
	MCharTable::MCharTable(const Database& db, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(db, 4, TestTables::ConvertNameCase(L"chartable", namesCase), L"", L"", L"", AF_READ)
	{
		m_idCharTable = 0;
		m_col2[0] = 0;
		m_col3[0] = 0;
		m_col4[0] = 0;

		SetColumn(0, TestTables::ConvertNameCase(L"idchartable", namesCase), &m_idCharTable, SQL_C_SLONG, sizeof(m_idCharTable));
		SetColumn(1, TestTables::ConvertNameCase(L"col2", namesCase), m_col2, SQL_C_WCHAR, sizeof(m_col2));
		SetColumn(2, TestTables::ConvertNameCase(L"col3", namesCase), m_col3, SQL_C_WCHAR, sizeof(m_col3));
		SetColumn(3, TestTables::ConvertNameCase(L"col4", namesCase), m_col4, SQL_C_WCHAR, sizeof(m_col4));
	}


	// IncompleteCharTable
	// -------------------
	MIncompleteCharTable::MIncompleteCharTable(const Database& db, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(db, 4, TestTables::ConvertNameCase(L"chartable", namesCase), L"", L"", L"", AF_READ)
	{
		m_idCharTable = 0;
		m_col2[0] = 0;
		//m_col3[0] = 0;
		m_col4[0] = 0;

		SetColumn(0, TestTables::ConvertNameCase(L"idchartable", namesCase), &m_idCharTable, SQL_C_SLONG, sizeof(m_idCharTable));
		SetColumn(1, TestTables::ConvertNameCase(L"col2", namesCase), m_col2, SQL_C_WCHAR, sizeof(m_col2));
		//SetColDefs(2, L"col3", DB_DATA_TYPE_VARCHAR, m_col3, SQL_C_WCHAR, sizeof(m_col3), false, false, false, false);
		SetColumn(3, TestTables::ConvertNameCase(L"col4", namesCase), m_col4, SQL_C_WCHAR, sizeof(m_col4));
	}


	// NotExistingTable
	// ----------------
	MNotExistingTable::MNotExistingTable(const Database& db, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(db, 1, TestTables::ConvertNameCase(L"notexisting", namesCase))
	{
		m_idNotExisting = 0;

		SetColumn(0, TestTables::ConvertNameCase(L"idnotexisting", namesCase), &m_idNotExisting, SQL_C_SLONG, sizeof(m_idNotExisting));
	}
}