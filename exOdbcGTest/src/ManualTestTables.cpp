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
	MIntTypesTable::MIntTypesTable(const Database* pDb, test::Case namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"IntegerTypes" */)
		: Table(pDb, 4, test::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idIntegerTypes = 0;
		m_smallInt = 0;
		m_int = 0;
		m_bigInt = 0;

		SetColumn(0, test::ConvertNameCase(L"idintegertypes", namesCase), SQL_INTEGER, &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		SetColumn(1, test::ConvertNameCase(L"tsmallint", namesCase), SQL_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), ColumnFlag::CF_SELECT);
		SetColumn(2, test::ConvertNameCase(L"tint", namesCase), SQL_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), ColumnFlag::CF_SELECT);
		SetColumn(3, test::ConvertNameCase(L"tbigint", namesCase), SQL_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), ColumnFlag::CF_SELECT);
	}


	// FloatTypesTable
	// ---------------
	MFloatTypesTable::MFloatTypesTable(const Database* pDb, test::Case namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"FloatTypes" */)
		: Table(pDb, 3, test::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idFloatTypes = 0;
		m_double = 0;
		m_doubleAsFloat = 0;
		m_float = 0;

		// Note: When binding a column of type FLOAT (database-type), you still need to use SQL_C_DOUBLE
		// SQL_C_FLOAT is for REAL (database-type), which I will not test here, mysql doesnt know about it ? but still reports float as real?
		// TODO: Test it for db-2 specific test (REAL-type? what did I mean?). But do that once we can determine the db-type from the wxDb object itself 
		SetColumn(0, test::ConvertNameCase(L"idfloattypes", namesCase), SQL_INTEGER, &m_idFloatTypes, SQL_C_SLONG, sizeof(m_idFloatTypes), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);

		if (pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			// Sql Server has no double, its all float
			SetColumn(1, test::ConvertNameCase(L"tdouble", namesCase), SQL_FLOAT, &m_doubleAsFloat, SQL_C_FLOAT, sizeof(m_doubleAsFloat), ColumnFlag::CF_SELECT);
		}
		else
		{
			SetColumn(1, test::ConvertNameCase(L"tdouble", namesCase), SQL_DOUBLE, &m_double, SQL_C_DOUBLE, sizeof(m_double), ColumnFlag::CF_SELECT);
		}

		if (pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			// Access reports SQL_REAL as a supported DB-type
			SetColumn(2, test::ConvertNameCase(L"tfloat", namesCase), SQL_REAL, &m_float, SQL_C_FLOAT, sizeof(m_float), ColumnFlag::CF_SELECT);
		}
		else
		{
			SetColumn(2, test::ConvertNameCase(L"tfloat", namesCase), SQL_FLOAT, &m_float, SQL_C_FLOAT, sizeof(m_float), ColumnFlag::CF_SELECT);
		}
	}


	// CharTypesTable
	// --------------
	MCharTypesTable::MCharTypesTable(const Database* pDb, test::Case namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"CharTypes" */)
		: Table(pDb, 5, test::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idCharTypes = 0;
		m_varchar[0] = 0;
		m_char[0] = 0;
		m_varchar_10[0] = 0;
		m_char_10[0] = 0;

		SetColumn(0, test::ConvertNameCase(L"idchartypes", namesCase), SQL_INTEGER, &m_idCharTypes, SQL_C_SLONG, sizeof(m_idCharTypes), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		if (pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			// Access does not report SQL_VARCHAR as a supported type, it will report the WVARCHAR, lets try converting that to SQL_C_CHAR
			SetColumn(1, test::ConvertNameCase(L"tvarchar", namesCase), SQL_WVARCHAR, m_varchar, SQL_C_CHAR, sizeof(m_varchar), ColumnFlag::CF_SELECT);
			SetColumn(2, test::ConvertNameCase(L"tchar", namesCase), SQL_WVARCHAR, m_char, SQL_C_CHAR, sizeof(m_char), ColumnFlag::CF_SELECT);
			SetColumn(3, test::ConvertNameCase(L"tvarchar_10", namesCase), SQL_WVARCHAR, m_varchar_10, SQL_C_CHAR, sizeof(m_varchar_10), ColumnFlag::CF_SELECT);
			SetColumn(4, test::ConvertNameCase(L"tchar_10", namesCase), SQL_WVARCHAR, m_char_10, SQL_C_CHAR, sizeof(m_char_10), ColumnFlag::CF_SELECT);
		}
		else
		{
			SetColumn(1, test::ConvertNameCase(L"tvarchar", namesCase), SQL_VARCHAR, m_varchar, SQL_C_CHAR, sizeof(m_varchar), ColumnFlag::CF_SELECT);
			SetColumn(2, test::ConvertNameCase(L"tchar", namesCase), SQL_VARCHAR, m_char, SQL_C_CHAR, sizeof(m_char), ColumnFlag::CF_SELECT);
			SetColumn(3, test::ConvertNameCase(L"tvarchar_10", namesCase), SQL_VARCHAR, m_varchar_10, SQL_C_CHAR, sizeof(m_varchar_10), ColumnFlag::CF_SELECT);
			SetColumn(4, test::ConvertNameCase(L"tchar_10", namesCase), SQL_VARCHAR, m_char_10, SQL_C_CHAR, sizeof(m_char_10), ColumnFlag::CF_SELECT);
		}
	}


	// WCharTypesTable
	// --------------
	MWCharTypesTable::MWCharTypesTable(const Database* pDb, test::Case namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"CharTypes" */)
		: Table(pDb, 5, test::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idCharTypes = 0;
		m_varchar[0] = 0;
		m_char[0] = 0;
		m_varchar_10[0] = 0;
		m_char_10[0] = 0;

		SetColumn(0, test::ConvertNameCase(L"idchartypes", namesCase), SQL_INTEGER, &m_idCharTypes, SQL_C_SLONG, sizeof(m_idCharTypes), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		if (pDb->GetDbms() == DatabaseProduct::MY_SQL)
		{
			// MySql does not report SQL_WVARCHAR as a supported type, use the SQL_VARCHAR type as database-type
			SetColumn(1, test::ConvertNameCase(L"tvarchar", namesCase), SQL_VARCHAR, m_varchar, SQL_C_WCHAR, sizeof(m_varchar), ColumnFlag::CF_SELECT);
			SetColumn(2, test::ConvertNameCase(L"tchar", namesCase), SQL_VARCHAR, m_char, SQL_C_WCHAR, sizeof(m_char), ColumnFlag::CF_SELECT);
			SetColumn(3, test::ConvertNameCase(L"tvarchar_10", namesCase), SQL_VARCHAR, m_varchar_10, SQL_C_WCHAR, sizeof(m_varchar_10), ColumnFlag::CF_SELECT);
			SetColumn(4, test::ConvertNameCase(L"tchar_10", namesCase), SQL_VARCHAR, m_char_10, SQL_C_WCHAR, sizeof(m_char_10), ColumnFlag::CF_SELECT);
		}
		else
		{
			SetColumn(1, test::ConvertNameCase(L"tvarchar", namesCase), SQL_WVARCHAR, m_varchar, SQL_C_WCHAR, sizeof(m_varchar), ColumnFlag::CF_SELECT);
			SetColumn(2, test::ConvertNameCase(L"tchar", namesCase), SQL_WVARCHAR, m_char, SQL_C_WCHAR, sizeof(m_char), ColumnFlag::CF_SELECT);
			SetColumn(3, test::ConvertNameCase(L"tvarchar_10", namesCase), SQL_WVARCHAR, m_varchar_10, SQL_C_WCHAR, sizeof(m_varchar_10), ColumnFlag::CF_SELECT);
			SetColumn(4, test::ConvertNameCase(L"tchar_10", namesCase), SQL_WVARCHAR, m_char_10, SQL_C_WCHAR, sizeof(m_char_10), ColumnFlag::CF_SELECT);
		}
	}


	// DateTypesTable
	// ---------------
	MDateTypesTable::MDateTypesTable(const Database* pDb, test::Case namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"DateTypes" */)
		: Table(pDb, 4, test::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idDateTypes = 0;
		ZeroMemory(&m_date, sizeof(m_date));
		ZeroMemory(&m_time, sizeof(m_time));
		ZeroMemory(&m_timestamp, sizeof(m_timestamp));

		// Note: We are odbc 3, therefore use the new c-type (with type: SQL_C_TYPE_DATE instead of SQL_C_DATE).
		SetColumn(0, test::ConvertNameCase(L"iddatetypes", namesCase), SQL_INTEGER, &m_idDateTypes, SQL_C_SLONG, sizeof(m_idDateTypes), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		if (pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			// Access does not report SQL_DATE, SQL_TIME or SQL_TIMESTAMP as a reported type.
			SetColumn(1, test::ConvertNameCase(L"tdate", namesCase), SQL_UNKNOWN_TYPE, &m_date, SQL_C_TYPE_DATE, sizeof(m_date), ColumnFlag::CF_SELECT);
			SetColumn(2, test::ConvertNameCase(L"ttime", namesCase), SQL_UNKNOWN_TYPE, &m_time, SQL_C_TYPE_TIME, sizeof(m_time), ColumnFlag::CF_SELECT);
			SetColumn(3, test::ConvertNameCase(L"ttimestamp", namesCase), SQL_UNKNOWN_TYPE, &m_timestamp, SQL_C_TYPE_TIMESTAMP, sizeof(m_timestamp), ColumnFlag::CF_SELECT);
		}
		else
		{
			SetColumn(1, test::ConvertNameCase(L"tdate", namesCase), SQL_DATE, &m_date, SQL_C_TYPE_DATE, sizeof(m_date), ColumnFlag::CF_SELECT);
			SetColumn(2, test::ConvertNameCase(L"ttime", namesCase), SQL_TIME, &m_time, SQL_C_TYPE_TIME, sizeof(m_time), ColumnFlag::CF_SELECT);
			SetColumn(3, test::ConvertNameCase(L"ttimestamp", namesCase), SQL_TIMESTAMP, &m_timestamp, SQL_C_TYPE_TIMESTAMP, sizeof(m_timestamp), ColumnFlag::CF_SELECT);
		}
	}


	// BlobTypesTable
	// --------------
	MBlobTypesTable::MBlobTypesTable(const Database* pDb, test::Case namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"BlobTypes" */)
		: Table(pDb, 3, test::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		ZeroMemory(m_blob, sizeof(m_blob));
		ZeroMemory(m_varblob_20, sizeof(m_varblob_20));

		SetColumn(0, test::ConvertNameCase(L"idblobtypes", namesCase), SQL_INTEGER, &m_idBlobTypes, SQL_C_SLONG, sizeof(m_idBlobTypes), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		SetColumn(1, test::ConvertNameCase(L"tblob", namesCase), SQL_BINARY, m_blob, SQL_C_BINARY, sizeof(m_blob), ColumnFlag::CF_SELECT);
		SetColumn(2, test::ConvertNameCase(L"tvarblob_20", namesCase), SQL_BINARY, m_varblob_20, SQL_C_BINARY, sizeof(m_varblob_20), ColumnFlag::CF_SELECT);
	}


	// MNumericTypesTable
	// -----------------
	MNumericTypesTable::MNumericTypesTable(const Database* pDb, test::Case namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"NumericTypes" */)
		: Table(pDb, 4, test::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idNumericTypes = 0;

		::ZeroMemory(&m_decimal_18_0, sizeof(m_decimal_18_0));
		::ZeroMemory(&m_decimal_18_10, sizeof(m_decimal_18_10));
		::ZeroMemory(&m_decimal_5_3, sizeof(m_decimal_5_3));

		SetColumn(0, test::ConvertNameCase(L"idnumerictypes", namesCase), SQL_INTEGER, &m_idNumericTypes, SQL_C_SLONG, sizeof(m_idNumericTypes), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		SetColumn(1, test::ConvertNameCase(L"tdecimal_18_0", namesCase), SQL_NUMERIC, &m_decimal_18_0, SQL_C_NUMERIC, sizeof(m_decimal_18_0), CF_SELECT, 18, 0);
		SetColumn(2, test::ConvertNameCase(L"tdecimal_18_10", namesCase), SQL_NUMERIC, &m_decimal_18_10, SQL_C_NUMERIC, sizeof(m_decimal_18_10), CF_SELECT, 18, 10);
		SetColumn(3, test::ConvertNameCase(L"tdecimal_5_3", namesCase), SQL_NUMERIC, &m_decimal_5_3, SQL_C_NUMERIC, sizeof(m_decimal_5_3), CF_SELECT, 5, 3);
	};


	// MNumericTypesAsCharTable
	// -----------------
	MNumericTypesAsCharTable::MNumericTypesAsCharTable(const Database* pDb, test::Case namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"NumericTypes" */)
		: Table(pDb, 4, test::ConvertNameCase(name, namesCase), L"", L"", L"", AF_READ)
	{
		m_idNumericTypes = 0;
		m_wcdecimal_18_0[0] = 0;
		m_wcdecimal_18_10[0] = 0;
		m_wdecimal_5_3[0] = 0;

		SetColumn(0, test::ConvertNameCase(L"idnumerictypes", namesCase), SQL_INTEGER, &m_idNumericTypes, SQL_C_SLONG, sizeof(m_idNumericTypes), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		SetColumn(1, test::ConvertNameCase(L"tdecimal_18_0", namesCase), SQL_NUMERIC, m_wcdecimal_18_0, SQL_C_WCHAR, sizeof(m_wcdecimal_18_0), ColumnFlag::CF_SELECT);
		SetColumn(2, test::ConvertNameCase(L"tdecimal_18_10", namesCase), SQL_NUMERIC, m_wcdecimal_18_10, SQL_C_WCHAR, sizeof(m_wcdecimal_18_10), ColumnFlag::CF_SELECT);
		SetColumn(3, test::ConvertNameCase(L"tdecimal_5_3", namesCase), SQL_NUMERIC, m_wdecimal_5_3, SQL_C_WCHAR, sizeof(m_wdecimal_5_3), ColumnFlag::CF_SELECT);
	};


	// CharTable
	// ---------
	MCharTable::MCharTable(const Database* pDb, test::Case namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 4, test::ConvertNameCase(L"chartable", namesCase), L"", L"", L"", AF_READ)
	{
		m_idCharTable = 0;
		m_col2[0] = 0;
		m_col3[0] = 0;
		m_col4[0] = 0;

		SetColumn(0, test::ConvertNameCase(L"idchartable", namesCase), SQL_INTEGER, &m_idCharTable, SQL_C_SLONG, sizeof(m_idCharTable), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		SetColumn(1, test::ConvertNameCase(L"col2", namesCase), SQL_VARCHAR, m_col2, SQL_C_WCHAR, sizeof(m_col2), ColumnFlag::CF_SELECT);
		SetColumn(2, test::ConvertNameCase(L"col3", namesCase), SQL_VARCHAR, m_col3, SQL_C_WCHAR, sizeof(m_col3), ColumnFlag::CF_SELECT);
		SetColumn(3, test::ConvertNameCase(L"col4", namesCase), SQL_VARCHAR, m_col4, SQL_C_WCHAR, sizeof(m_col4), ColumnFlag::CF_SELECT);
	}


	// IncompleteCharTable
	// -------------------
	MIncompleteCharTable::MIncompleteCharTable(const Database* pDb, test::Case namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 4, test::ConvertNameCase(L"chartable", namesCase), L"", L"", L"", AF_READ)
	{
		m_idCharTable = 0;
		m_col2[0] = 0;
		//m_col3[0] = 0;
		m_col4[0] = 0;

		SetColumn(0, test::ConvertNameCase(L"idchartable", namesCase), SQL_INTEGER, &m_idCharTable, SQL_C_SLONG, sizeof(m_idCharTable), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		SetColumn(1, test::ConvertNameCase(L"col2", namesCase), SQL_VARCHAR, m_col2, SQL_C_WCHAR, sizeof(m_col2), ColumnFlag::CF_SELECT);
		//SetColDefs(2, L"col3", DB_DATA_TYPE_VARCHAR, m_col3, SQL_C_WCHAR, sizeof(m_col3), false, false, false, false);
		SetColumn(3, test::ConvertNameCase(L"col4", namesCase), SQL_VARCHAR, m_col4, SQL_C_WCHAR, sizeof(m_col4), ColumnFlag::CF_SELECT);
	}


	// NotExistingTable
	// ----------------
	MNotExistingTable::MNotExistingTable(const Database* pDb, test::Case namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 1, test::ConvertNameCase(L"notexisting", namesCase))
	{
		m_idNotExisting = 0;

		SetColumn(0, test::ConvertNameCase(L"idnotexisting", namesCase), SQL_INTEGER, &m_idNotExisting, SQL_C_SLONG, sizeof(m_idNotExisting), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
	}
}