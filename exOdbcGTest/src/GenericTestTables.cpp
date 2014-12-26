/*!
 * \file GenericTestTables.cpp
 * \author Elias Gerber <eg@zame.ch>
 * \date 23.02.2014
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "GenericTestTables.h"

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
	// NotExistingTable
	// ----------------
	NotExistingTable::NotExistingTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 1, TestTables::GetTableName(L"notexisting", namesCase))
	{
		SetColDefs(0, TestTables::GetColName(L"idnotexisting", namesCase), DB_DATA_TYPE_INTEGER, &m_idNotExisting, SQL_C_LONG, sizeof(m_idNotExisting), true);

	}

	// CharTypesTable
	// --------------
	CharTypesTable::CharTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 3, TestTables::GetTableName(L"chartypes", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idCharTypes	= 0;
		m_varchar[0]	= 0;
		m_char[0]		= 0;

		SetColDefs(0, TestTables::GetColName(L"idchartypes", namesCase), DB_DATA_TYPE_INTEGER, &m_idCharTypes, SQL_C_SLONG, sizeof(m_idCharTypes), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"tvarchar", namesCase), DB_DATA_TYPE_VARCHAR, m_varchar, SQL_C_WCHAR, sizeof(m_varchar), false, false, false, false);
		SetColDefs(2, TestTables::GetColName(L"tchar", namesCase), DB_DATA_TYPE_VARCHAR, m_char, SQL_C_WCHAR, sizeof(m_char), false, false, false, false);
	}

	// CharTypesTmpTable
	// --------------
	CharTypesTmpTable::CharTypesTmpTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 3, TestTables::GetTableName(L"chartypes_tmp", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idCharTypes	= 0;
		m_varchar[0]	= 0;
		m_char[0]		= 0;

		SetColDefs(0, TestTables::GetColName(L"idchartypes_tmp", namesCase), DB_DATA_TYPE_INTEGER, &m_idCharTypes, SQL_C_SLONG, sizeof(m_idCharTypes), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"tvarchar", namesCase), DB_DATA_TYPE_VARCHAR, m_varchar, SQL_C_WCHAR, sizeof(m_varchar), false, false, false, false);
		SetColDefs(2, TestTables::GetColName(L"tchar", namesCase), DB_DATA_TYPE_VARCHAR, m_char, SQL_C_WCHAR, sizeof(m_char), false, false, false, false);
	}

	// IntTypesTable
	// ---------------
	IntTypesTable::IntTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */ )
		: Table(pDb, 4, TestTables::GetTableName(L"integertypes", namesCase), L"", L"", L"", Table::READ_ONLY)
	{	
		m_idIntegerTypes = 0;
		m_smallInt = 0;
		m_int = 0;
		m_bigInt = 0;

		SetColDefs(0, TestTables::GetColName(L"idintegertypes", namesCase), DB_DATA_TYPE_INTEGER, &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"tsmallint", namesCase), DB_DATA_TYPE_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), false, false, false, false);
		SetColDefs(2, TestTables::GetColName(L"tint", namesCase), DB_DATA_TYPE_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), false, false, false, false);
		SetColDefs(3, TestTables::GetColName(L"tbigint", namesCase), DB_DATA_TYPE_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), false, false, false, false);
	}

	// IntTypesTmpTable
	// ---------------
	IntTypesTmpTable::IntTypesTmpTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 4, TestTables::GetTableName(L"integertypes_tmp", namesCase), L"", L"", L"", Table::READ_ONLY)
	{	
		m_idIntegerTypes = 0;
		m_smallInt = 0;
		m_int = 0;
		m_bigInt = 0;

		SetColDefs(0, TestTables::GetColName(L"idintegertypes_tmp", namesCase), DB_DATA_TYPE_INTEGER, &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"tsmallint", namesCase), DB_DATA_TYPE_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), false, false, false, false);
		SetColDefs(2, TestTables::GetColName(L"tint", namesCase), DB_DATA_TYPE_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), false, false, false, false);
		SetColDefs(3, TestTables::GetColName(L"tbigint", namesCase), DB_DATA_TYPE_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), false, false, false, false);
	}

	// DateTypesTable
	// ---------------
	DateTypesTable::DateTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 4, TestTables::GetTableName(L"datetypes", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idDateTypes		= 0;
		ZeroMemory(&m_date, sizeof(m_date));
		ZeroMemory(&m_time, sizeof(m_time));
		ZeroMemory(&m_timestamp, sizeof(m_timestamp));

		// Note: We are odbc v. 2, therefore use the old s-type (without type: SQL_C_DATE instead of SQL_C_TYPE_DATE). See Ticket # 17
		SetColDefs(0, TestTables::GetColName(L"iddatetypes", namesCase), DB_DATA_TYPE_INTEGER, &m_idDateTypes, SQL_C_SLONG, sizeof(m_idDateTypes), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"tdate", namesCase), DB_DATA_TYPE_DATE, &m_date, SQL_C_DATE, sizeof(m_date), false, false, false, false);
		SetColDefs(2, TestTables::GetColName(L"ttime", namesCase), DB_DATA_TYPE_DATE, &m_time, SQL_C_TIME, sizeof(m_time), false, false, false, false);
		SetColDefs(3, TestTables::GetColName(L"ttimestamp", namesCase), DB_DATA_TYPE_DATE, &m_timestamp, SQL_C_TIMESTAMP, sizeof(m_timestamp), false, false, false, false);
	}

	// DateTypesTmpTable
	// ---------------
	DateTypesTmpTable::DateTypesTmpTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 4, TestTables::GetTableName(L"datetypes_tmp", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idDateTypes		= 0;
		ZeroMemory(&m_date, sizeof(m_date));
		ZeroMemory(&m_time, sizeof(m_time));
		ZeroMemory(&m_timestamp, sizeof(m_timestamp));

		// Note: We are odbc v. 2, therefore use the old s-type (without type: SQL_C_DATE instead of SQL_C_TYPE_DATE). See Ticket # 17
		SetColDefs(0, TestTables::GetColName(L"iddatetypes_tmp", namesCase), DB_DATA_TYPE_INTEGER, &m_idDateTypes, SQL_C_SLONG, sizeof(m_idDateTypes), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"tdate", namesCase), DB_DATA_TYPE_DATE, &m_date, SQL_C_DATE, sizeof(m_date), false, false, false, false);
		SetColDefs(2, TestTables::GetColName(L"ttime", namesCase), DB_DATA_TYPE_DATE, &m_time, SQL_C_TIME, sizeof(m_time), false, false, false, false);
		SetColDefs(3, TestTables::GetColName(L"ttimestamp", namesCase), DB_DATA_TYPE_DATE, &m_timestamp, SQL_C_TIMESTAMP, sizeof(m_timestamp), false, false, false, false);
	}

	// FloatTypesTable
	// ---------------
	FloatTypesTable::FloatTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 3, TestTables::GetTableName(L"floattypes", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idFloatTypes	= 0;
		m_double		= 0;
		m_float			= 0;

		// Note: When binding a column of type FLOAT (database-type), you still need to use SQL_C_DOUBLE
		// SQL_C_FLOAT is for REAL (database-type), which I will not test here, mysql doesnt know about it
		// TODO: Test it for db-2 specific test (REAL-type? what did I mean?). But do that once we can determine the db-type from the wxDb object itself 
		SetColDefs(0, TestTables::GetColName(L"idfloattypes", namesCase), DB_DATA_TYPE_INTEGER, &m_idFloatTypes, SQL_C_SLONG, sizeof(m_idFloatTypes), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"tdouble", namesCase), DB_DATA_TYPE_FLOAT, &m_double, SQL_C_DOUBLE, sizeof(m_double), false, false, false, false);
		SetColDefs(2, TestTables::GetColName(L"tfloat", namesCase), DB_DATA_TYPE_FLOAT, &m_float, SQL_C_DOUBLE, sizeof(m_float), false, false, false, false);
	}

	// FloatTypesTable
	// ---------------
	FloatTypesTmpTable::FloatTypesTmpTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 3, TestTables::GetTableName(L"floattypes_tmp", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idFloatTypes	= 0;
		m_double		= 0;
		m_float			= 0;

		// Note: When binding a column of type FLOAT (database-type), you still need to use SQL_C_DOUBLE
		// SQL_C_FLOAT is for REAL (database-type), which I will not test here, mysql doesnt know about it
		SetColDefs(0, TestTables::GetColName(L"idfloattypes_tmp", namesCase), DB_DATA_TYPE_INTEGER, &m_idFloatTypes, SQL_C_SLONG, sizeof(m_idFloatTypes), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"tdouble", namesCase), DB_DATA_TYPE_FLOAT, &m_double, SQL_C_DOUBLE, sizeof(m_double), false, false, false, false);
		SetColDefs(2, TestTables::GetColName(L"tfloat", namesCase), DB_DATA_TYPE_FLOAT, &m_float, SQL_C_DOUBLE, sizeof(m_float), false, false, false, false);
	}

	// NumericTypesTable
	// -----------------
	NumericTypesTable::NumericTypesTable(Database* pDb, ReadMode readMode, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 3, TestTables::GetTableName(L"numerictypes", namesCase), L"", L"", L"", Table::READ_ONLY)
		, m_readMode(readMode)
	{
		m_idNumericTypes	= 0;
		ZeroMemory(&m_decimal_18_0, sizeof(m_decimal_18_0));
		ZeroMemory(&m_decimal_18_10, sizeof(m_decimal_18_10));
		m_wcdecimal_18_0[0] = 0;
		m_wcdecimal_18_10[0] = 0;

		// TODO: This DB_DATA_TYPE * stuff is probably useless, or must be extended to support more types. 
		// maybe used for formating to sql?
		SetColDefs(0, TestTables::GetColName(L"idnumerictypes", namesCase), DB_DATA_TYPE_INTEGER, &m_idNumericTypes, SQL_C_SLONG, sizeof(m_idNumericTypes), true, false, false, false);
		if(m_readMode == ReasAsNumeric)
		{
			SetColDefs(1, TestTables::GetColName(L"tdecimal_18_0", namesCase), DB_DATA_TYPE_FLOAT, &m_decimal_18_0, SQL_C_NUMERIC, sizeof(m_decimal_18_0), false, false, false, false);
			SetColDefs(2, TestTables::GetColName(L"tdecimal_18_10", namesCase), DB_DATA_TYPE_FLOAT, &m_decimal_18_10, SQL_C_NUMERIC, sizeof(m_decimal_18_0), false, false, false, false);
		}
		else if(m_readMode == ReadAsChar)
		{
			SetColDefs(1, TestTables::GetColName(L"tdecimal_18_0", namesCase), DB_DATA_TYPE_VARCHAR, m_wcdecimal_18_0, SQL_C_WCHAR, sizeof(m_wcdecimal_18_0), false, false, false, false);
			SetColDefs(2, TestTables::GetColName(L"tdecimal_18_10", namesCase), DB_DATA_TYPE_VARCHAR, m_wcdecimal_18_10, SQL_C_WCHAR, sizeof(m_wcdecimal_18_10), false, false, false, false);
		}
	};

	// NumericTypesTable
	// -----------------
	NumericTypesTmpTable::NumericTypesTmpTable(Database* pDb, ReadMode readMode, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 3, TestTables::GetTableName(L"numerictypes_tmp", namesCase), L"", L"", L"", Table::READ_ONLY)
		, m_readMode(readMode)
	{
		m_idNumericTypes	= 0;
		ZeroMemory(&m_decimal_18_0, sizeof(m_decimal_18_0));
		ZeroMemory(&m_decimal_18_10, sizeof(m_decimal_18_10));
		m_wcdecimal_18_0[0] = 0;
		m_wcdecimal_18_10[0] = 0;

		// TODO: This DB_DATA_TYPE * stuff is probably useless, or must be extended to support more types. 
		// maybe used for formating to sql?
		SetColDefs(0, TestTables::GetColName(L"idnumerictypes_tmp", namesCase), DB_DATA_TYPE_INTEGER, &m_idNumericTypes, SQL_C_SLONG, sizeof(m_idNumericTypes), true, false, false, false);
		if(m_readMode == ReasAsNumeric)
		{
			SetColDefs(1, TestTables::GetColName(L"tdecimal_18_0", namesCase), DB_DATA_TYPE_FLOAT, &m_decimal_18_0, SQL_C_NUMERIC, sizeof(m_decimal_18_0), false, false, false, false);
			SetColDefs(2, TestTables::GetColName(L"tdecimal_18_10", namesCase), DB_DATA_TYPE_FLOAT, &m_decimal_18_10, SQL_C_NUMERIC, sizeof(m_decimal_18_0), false, false, false, false);
		}
		else if(m_readMode == ReadAsChar)
		{
			SetColDefs(1, TestTables::GetColName(L"tdecimal_18_0", namesCase), DB_DATA_TYPE_VARCHAR, m_wcdecimal_18_0, SQL_C_WCHAR, sizeof(m_wcdecimal_18_0), false, false, false, false);
			SetColDefs(2, TestTables::GetColName(L"tdecimal_18_10", namesCase), DB_DATA_TYPE_VARCHAR, m_wcdecimal_18_10, SQL_C_WCHAR, sizeof(m_wcdecimal_18_10), false, false, false, false);
		}
	};

	// BlobTypesTable
	// --------------
	BlobTypesTable::BlobTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 2, TestTables::GetTableName(L"blobtypes", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		ZeroMemory(m_blob, sizeof(m_blob));

		SetColDefs(0, TestTables::GetColName(L"idblobtypes", namesCase), DB_DATA_TYPE_INTEGER, &m_idBlobTypes, SQL_C_SLONG, sizeof(m_idBlobTypes), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"tblob", namesCase), DB_DATA_TYPE_VARCHAR, m_blob, SQL_C_BINARY, sizeof(m_blob), false, false, false, false);
	}

	// BlobTypesTmpTable
	// --------------
	BlobTypesTmpTable::BlobTypesTmpTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 2, TestTables::GetTableName(L"blobtypes_tmp", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		ZeroMemory(m_blob, sizeof(m_blob));

		SetColDefs(0, TestTables::GetColName(L"idblobtypes_tmp", namesCase), DB_DATA_TYPE_INTEGER, &m_idBlobTypes, SQL_C_SLONG, sizeof(m_idBlobTypes), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"tblob", namesCase), DB_DATA_TYPE_VARCHAR, m_blob, SQL_C_BINARY, sizeof(m_blob), false, false, false, false);
	}

	// CharTable
	// ---------
	CharTable::CharTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 4, TestTables::GetTableName(L"chartable", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idCharTable	= 0;
		m_col2[0] = 0;
		m_col3[0] = 0;
		m_col4[0] = 0;

		SetColDefs(0, TestTables::GetColName(L"idchartable", namesCase), DB_DATA_TYPE_INTEGER, &m_idCharTable, SQL_C_SLONG, sizeof(m_idCharTable), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"col2", namesCase), DB_DATA_TYPE_VARCHAR, m_col2, SQL_C_WCHAR, sizeof(m_col2), false, false, false, false);
		SetColDefs(2, TestTables::GetColName(L"col3", namesCase), DB_DATA_TYPE_VARCHAR, m_col3, SQL_C_WCHAR, sizeof(m_col3), false, false, false, false);
		SetColDefs(3, TestTables::GetColName(L"col4", namesCase), DB_DATA_TYPE_VARCHAR, m_col4, SQL_C_WCHAR, sizeof(m_col4), false, false, false, false);
	}

	// IncompleteCharTable
	// -------------------
	IncompleteCharTable::IncompleteCharTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 4, TestTables::GetTableName(L"chartable", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idCharTable	= 0;
		m_col2[0] = 0;
		//m_col3[0] = 0;
		m_col4[0] = 0;

		SetColDefs(0, TestTables::GetColName(L"idchartable", namesCase), DB_DATA_TYPE_INTEGER, &m_idCharTable, SQL_C_SLONG, sizeof(m_idCharTable), true, false, false, false);
		SetColDefs(1, TestTables::GetColName(L"col2", namesCase), DB_DATA_TYPE_VARCHAR, m_col2, SQL_C_WCHAR, sizeof(m_col2), false, false, false, false);
		//SetColDefs(2, L"col3", DB_DATA_TYPE_VARCHAR, m_col3, SQL_C_WCHAR, sizeof(m_col3), false, false, false, false);
		SetColDefs(3, TestTables::GetColName(L"col4", namesCase), DB_DATA_TYPE_VARCHAR, m_col4, SQL_C_WCHAR, sizeof(m_col4), false, false, false, false);
	}
} // namespace exodbc