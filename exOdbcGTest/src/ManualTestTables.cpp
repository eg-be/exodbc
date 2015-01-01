/*!
* \file ManualTestTables.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 26.12.2014
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
	MIntTypesTable::MIntTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"IntegerTypes" */)
		: Table(pDb, 4, TestTables::GetTableName(name, namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idIntegerTypes = 0;
		m_smallInt = 0;
		m_int = 0;
		m_bigInt = 0;

		SetColumn(0, TestTables::GetColName(L"idintegertypes", namesCase), &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes));
		SetColumn(1, TestTables::GetColName(L"tsmallint", namesCase), &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt));
		SetColumn(2, TestTables::GetColName(L"tint", namesCase), &m_int, SQL_C_SLONG, sizeof(m_int));
		SetColumn(3, TestTables::GetColName(L"tbigint", namesCase), &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt));
	}


	// FloatTypesTable
	// ---------------
	MFloatTypesTable::MFloatTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"FloatTypes" */)
		: Table(pDb, 3, TestTables::GetTableName(name, namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idFloatTypes = 0;
		m_double = 0;
		m_float = 0;

		// Note: When binding a column of type FLOAT (database-type), you still need to use SQL_C_DOUBLE
		// SQL_C_FLOAT is for REAL (database-type), which I will not test here, mysql doesnt know about it ? but still reports float as real?
		// TODO: Test it for db-2 specific test (REAL-type? what did I mean?). But do that once we can determine the db-type from the wxDb object itself 
		SetColumn(0, TestTables::GetColName(L"idfloattypes", namesCase), &m_idFloatTypes, SQL_C_SLONG, sizeof(m_idFloatTypes));
		SetColumn(1, TestTables::GetColName(L"tdouble", namesCase), &m_double, SQL_C_DOUBLE, sizeof(m_double));
		SetColumn(2, TestTables::GetColName(L"tfloat", namesCase), &m_float, SQL_C_DOUBLE, sizeof(m_float));
	}


	// CharTypesTable
	// --------------
	MCharTypesTable::MCharTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"CharTypes" */)
		: Table(pDb, 3, TestTables::GetTableName(name, namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idCharTypes = 0;
		m_varchar[0] = 0;
		m_char[0] = 0;

		SetColumn(0, TestTables::GetColName(L"idchartypes", namesCase), &m_idCharTypes, SQL_C_SLONG, sizeof(m_idCharTypes));
		SetColumn(1, TestTables::GetColName(L"tvarchar", namesCase), m_varchar, SQL_C_CHAR, sizeof(m_varchar));
		SetColumn(2, TestTables::GetColName(L"tchar", namesCase), m_char, SQL_C_CHAR, sizeof(m_char));
	}


	// WCharTypesTable
	// --------------
	MWCharTypesTable::MWCharTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"CharTypes" */)
		: Table(pDb, 3, TestTables::GetTableName(name, namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idCharTypes = 0;
		m_varchar[0] = 0;
		m_char[0] = 0;

		SetColumn(0, TestTables::GetColName(L"idchartypes", namesCase), &m_idCharTypes, SQL_C_SLONG, sizeof(m_idCharTypes));
		SetColumn(1, TestTables::GetColName(L"tvarchar", namesCase), m_varchar, SQL_C_WCHAR, sizeof(m_varchar));
		SetColumn(2, TestTables::GetColName(L"tchar", namesCase), m_char, SQL_C_WCHAR, sizeof(m_char));
	}


	// DateTypesTable
	// ---------------
	MDateTypesTable::MDateTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"DateTypes" */)
		: Table(pDb, 4, TestTables::GetTableName(name, namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idDateTypes = 0;
		ZeroMemory(&m_date, sizeof(m_date));
		ZeroMemory(&m_time, sizeof(m_time));
		ZeroMemory(&m_timestamp, sizeof(m_timestamp));

		// Note: We are odbc 3, therefore use the new c-type (with type: SQL_C_TYPE_DATE instead of SQL_C_DATE).
		SetColumn(0, TestTables::GetColName(L"iddatetypes", namesCase), &m_idDateTypes, SQL_C_SLONG, sizeof(m_idDateTypes));
		SetColumn(1, TestTables::GetColName(L"tdate", namesCase), &m_date, SQL_C_TYPE_DATE, sizeof(m_date));
		SetColumn(2, TestTables::GetColName(L"ttime", namesCase), &m_time, SQL_C_TYPE_TIME, sizeof(m_time));
		SetColumn(3, TestTables::GetColName(L"ttimestamp", namesCase), &m_timestamp, SQL_C_TYPE_TIMESTAMP, sizeof(m_timestamp));
	}


	// BlobTypesTable
	// --------------
	MBlobTypesTable::MBlobTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"BlobTypes" */)
		: Table(pDb, 2, TestTables::GetTableName(name, namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		ZeroMemory(m_blob, sizeof(m_blob));

		SetColumn(0, TestTables::GetColName(L"idblobtypes", namesCase),  &m_idBlobTypes, SQL_C_SLONG, sizeof(m_idBlobTypes));
		SetColumn(1, TestTables::GetColName(L"tblob", namesCase), m_blob, SQL_C_BINARY, sizeof(m_blob));
	}


	// MNumericTypesTable
	// -----------------
	MNumericTypesTable::MNumericTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"NumericTypes" */)
		: Table(pDb, 4, TestTables::GetTableName(name, namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idNumericTypes = 0;

		::ZeroMemory(&m_decimal_18_0, sizeof(m_decimal_18_0));
		::ZeroMemory(&m_decimal_18_10, sizeof(m_decimal_18_10));

		SetColumn(0, TestTables::GetColName(L"idnumerictypes", namesCase), &m_idNumericTypes, SQL_C_SLONG, sizeof(m_idNumericTypes));
		SetColumn(1, TestTables::GetColName(L"tdecimal_18_0", namesCase), &m_decimal_18_0, SQL_C_NUMERIC, sizeof(m_decimal_18_0), 18, 0);
		SetColumn(2, TestTables::GetColName(L"tdecimal_18_10", namesCase), &m_decimal_18_10, SQL_C_NUMERIC, sizeof(m_decimal_18_10), 18, 10);
		SetColumn(3, TestTables::GetColName(L"tdecimal_5_3", namesCase), &m_decimal_5_3, SQL_C_NUMERIC, sizeof(m_decimal_5_3), 5, 3);
	};


	// MNumericTypesAsCharTable
	// -----------------
	MNumericTypesAsCharTable::MNumericTypesAsCharTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */, const std::wstring& name /* = L"NumericTypes" */)
		: Table(pDb, 3, TestTables::GetTableName(name, namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idNumericTypes = 0;
		m_wcdecimal_18_0[0] = 0;
		m_wcdecimal_18_10[0] = 0;

		SetColumn(0, TestTables::GetColName(L"idnumerictypes", namesCase), &m_idNumericTypes, SQL_C_SLONG, sizeof(m_idNumericTypes));
		SetColumn(1, TestTables::GetColName(L"tdecimal_18_0", namesCase), m_wcdecimal_18_0, SQL_C_WCHAR, sizeof(m_wcdecimal_18_0));
		SetColumn(2, TestTables::GetColName(L"tdecimal_18_10", namesCase), m_wcdecimal_18_10, SQL_C_WCHAR, sizeof(m_wcdecimal_18_10));
	};


	// CharTable
	// ---------
	MCharTable::MCharTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 4, TestTables::GetTableName(L"chartable", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idCharTable = 0;
		m_col2[0] = 0;
		m_col3[0] = 0;
		m_col4[0] = 0;

		SetColumn(0, TestTables::GetColName(L"idchartable", namesCase), &m_idCharTable, SQL_C_SLONG, sizeof(m_idCharTable));
		SetColumn(1, TestTables::GetColName(L"col2", namesCase), m_col2, SQL_C_WCHAR, sizeof(m_col2));
		SetColumn(2, TestTables::GetColName(L"col3", namesCase), m_col3, SQL_C_WCHAR, sizeof(m_col3));
		SetColumn(3, TestTables::GetColName(L"col4", namesCase), m_col4, SQL_C_WCHAR, sizeof(m_col4));
	}


	// IncompleteCharTable
	// -------------------
	MIncompleteCharTable::MIncompleteCharTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 4, TestTables::GetTableName(L"chartable", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idCharTable = 0;
		m_col2[0] = 0;
		//m_col3[0] = 0;
		m_col4[0] = 0;

		SetColumn(0, TestTables::GetColName(L"idchartable", namesCase), &m_idCharTable, SQL_C_SLONG, sizeof(m_idCharTable));
		SetColumn(1, TestTables::GetColName(L"col2", namesCase), m_col2, SQL_C_WCHAR, sizeof(m_col2));
		//SetColDefs(2, L"col3", DB_DATA_TYPE_VARCHAR, m_col3, SQL_C_WCHAR, sizeof(m_col3), false, false, false, false);
		SetColumn(3, TestTables::GetColName(L"col4", namesCase), m_col4, SQL_C_WCHAR, sizeof(m_col4));
	}


	// NotExistingTable
	// ----------------
	MNotExistingTable::MNotExistingTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 1, TestTables::GetTableName(L"notexisting", namesCase))
	{
		m_idNotExisting = 0;

		SetColumn(0, TestTables::GetColName(L"idnotexisting", namesCase), &m_idNotExisting, SQL_C_SLONG, sizeof(m_idNotExisting));
	}
}