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
#include "db.h"

using namespace exodbc;
// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
namespace wxOdbc3Test
{
	// NotExistingTable
	// ----------------
	NotExistingTable::NotExistingTable(wxDb* pDb)
		: wxDbTable(pDb, L"notexisting", 1)
	{
		SetColDefs(0, L"idnotexisting", DB_DATA_TYPE_INTEGER, &m_idNotExisting, SQL_C_LONG, sizeof(m_idNotExisting), true);

	}

	// CharTypesTable
	// --------------
	CharTypesTable::CharTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"chartypes", 3, L"", wxDB_QUERY_ONLY)
	{
		m_idCharTypes	= 0;
		m_varchar[0]	= 0;
		m_char[0]		= 0;

		SetColDefs(0, L"idchartypes", DB_DATA_TYPE_INTEGER, &m_idCharTypes, SQL_C_SLONG, sizeof(m_idCharTypes), true, false, false, false);
		SetColDefs(1, L"tvarchar", DB_DATA_TYPE_VARCHAR, m_varchar, SQL_C_WCHAR, sizeof(m_varchar), false, false, false, false);
		SetColDefs(2, L"tchar", DB_DATA_TYPE_VARCHAR, m_char, SQL_C_WCHAR, sizeof(m_char), false, false, false, false);
	}

	// CharTypesTmpTable
	// --------------
	CharTypesTmpTable::CharTypesTmpTable(wxDb* pDb)
		: wxDbTable(pDb, L"chartypes_tmp", 3, L"", wxDB_QUERY_ONLY)
	{
		m_idCharTypes	= 0;
		m_varchar[0]	= 0;
		m_char[0]		= 0;

		SetColDefs(0, L"idchartypes_tmp", DB_DATA_TYPE_INTEGER, &m_idCharTypes, SQL_C_SLONG, sizeof(m_idCharTypes), true, false, false, false);
		SetColDefs(1, L"tvarchar", DB_DATA_TYPE_VARCHAR, m_varchar, SQL_C_WCHAR, sizeof(m_varchar), false, false, false, false);
		SetColDefs(2, L"tchar", DB_DATA_TYPE_VARCHAR, m_char, SQL_C_WCHAR, sizeof(m_char), false, false, false, false);
	}

	// IntTypesTable
	// ---------------
	IntTypesTable::IntTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"integertypes", 7, L"", wxDB_QUERY_ONLY)
	{	
		m_idIntegerTypes = 0;
		m_smallInt = 0;
		m_int = 0;
		m_bigInt = 0;
		m_usmallInt = 0;
		m_uint = 0;
		m_ubigInt = 0;

		// TODO: Note: IBM DB2 has no unsigned-cols, but we get no error and no warning?
		// TODO: Should the error be spit when the table is opened?

		SetColDefs(0, L"idintegertypes", DB_DATA_TYPE_INTEGER, &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes), true, false, false, false);
		SetColDefs(1, L"tsmallint", DB_DATA_TYPE_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), false, false, false, false);
		SetColDefs(2, L"tint", DB_DATA_TYPE_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), false, false, false, false);
		SetColDefs(3, L"tbigint", DB_DATA_TYPE_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), false, false, false, false);	
		SetColDefs(4, L"tusmallint", DB_DATA_TYPE_INTEGER, &m_usmallInt, SQL_C_USHORT, sizeof(m_usmallInt), false, false, false, false);	
		SetColDefs(5, L"tuint", DB_DATA_TYPE_INTEGER, &m_uint, SQL_C_ULONG, sizeof(m_usmallInt), false, false, false, false);	
		SetColDefs(6, L"tubigint", DB_DATA_TYPE_INTEGER, &m_ubigInt, SQL_C_UBIGINT, sizeof(m_usmallInt), false, false, false, false);	
	}

	// IntTypesTmpTable
	// ---------------
	IntTypesTmpTable::IntTypesTmpTable(wxDb* pDb)
		: wxDbTable(pDb, L"integertypes_tmp", 7, L"", wxDB_QUERY_ONLY)
	{	
		m_idIntegerTypes = 0;
		m_smallInt = 0;
		m_int = 0;
		m_bigInt = 0;
		m_usmallInt = 0;
		m_uint = 0;
		m_ubigInt = 0;

		SetColDefs(0, L"idintegertypes_tmp", DB_DATA_TYPE_INTEGER, &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes), true, false, false, false);
		SetColDefs(1, L"tsmallint", DB_DATA_TYPE_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), false, false, false, false);
		SetColDefs(2, L"tint", DB_DATA_TYPE_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), false, false, false, false);
		SetColDefs(3, L"tbigint", DB_DATA_TYPE_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), false, false, false, false);	
		SetColDefs(4, L"tusmallint", DB_DATA_TYPE_INTEGER, &m_usmallInt, SQL_C_USHORT, sizeof(m_usmallInt), false, false, false, false);	
		SetColDefs(5, L"tuint", DB_DATA_TYPE_INTEGER, &m_uint, SQL_C_ULONG, sizeof(m_usmallInt), false, false, false, false);	
		SetColDefs(6, L"tubigint", DB_DATA_TYPE_INTEGER, &m_ubigInt, SQL_C_UBIGINT, sizeof(m_usmallInt), false, false, false, false);	
	}

	// DateTypesTable
	// ---------------
	DateTypesTable::DateTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"datetypes", 4, L"", wxDB_QUERY_ONLY)
	{
		m_idDateTypes		= 0;
		ZeroMemory(&m_date, sizeof(m_date));
		ZeroMemory(&m_time, sizeof(m_time));
		ZeroMemory(&m_timestamp, sizeof(m_timestamp));

		// Note: We are odbc v. 2, therefore use the old s-type (without type: SQL_C_DATE instead of SQL_C_TYPE_DATE). See Ticket # 17
		SetColDefs(0, L"iddatetypes", DB_DATA_TYPE_INTEGER, &m_idDateTypes, SQL_C_SLONG, sizeof(m_idDateTypes), true, false, false, false);
		SetColDefs(1, L"tdate", DB_DATA_TYPE_DATE, &m_date, SQL_C_DATE, sizeof(m_date), false, false, false, false);
		SetColDefs(2, L"ttime", DB_DATA_TYPE_DATE, &m_time, SQL_C_TIME, sizeof(m_time), false, false, false, false);
		SetColDefs(3, L"ttimestamp", DB_DATA_TYPE_DATE, &m_timestamp, SQL_C_TIMESTAMP, sizeof(m_timestamp), false, false, false, false);
	}

	// DateTypesTmpTable
	// ---------------
	DateTypesTmpTable::DateTypesTmpTable(wxDb* pDb)
		: wxDbTable(pDb, L"datetypes_tmp", 4, L"", wxDB_QUERY_ONLY)
	{
		m_idDateTypes		= 0;
		ZeroMemory(&m_date, sizeof(m_date));
		ZeroMemory(&m_time, sizeof(m_time));
		ZeroMemory(&m_timestamp, sizeof(m_timestamp));

		// Note: We are odbc v. 2, therefore use the old s-type (without type: SQL_C_DATE instead of SQL_C_TYPE_DATE). See Ticket # 17
		SetColDefs(0, L"iddatetypes_tmp", DB_DATA_TYPE_INTEGER, &m_idDateTypes, SQL_C_SLONG, sizeof(m_idDateTypes), true, false, false, false);
		SetColDefs(1, L"tdate", DB_DATA_TYPE_DATE, &m_date, SQL_C_DATE, sizeof(m_date), false, false, false, false);
		SetColDefs(2, L"ttime", DB_DATA_TYPE_DATE, &m_time, SQL_C_TIME, sizeof(m_time), false, false, false, false);
		SetColDefs(3, L"ttimestamp", DB_DATA_TYPE_DATE, &m_timestamp, SQL_C_TIMESTAMP, sizeof(m_timestamp), false, false, false, false);
	}

	// FloatTypesTable
	// ---------------
	FloatTypesTable::FloatTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"floattypes", 3, L"", wxDB_QUERY_ONLY)
	{
		m_idFloatTypes	= 0;
		m_double		= 0;
		m_float			= 0;

		// Note: When binding a column of type FLOAT (database-type), you still need to use SQL_C_DOUBLE
		// SQL_C_FLOAT is for REAL (database-type), which I will not test here, mysql doesnt know about it
		// TODO: Test it for db-2 specific test (REAL-type? what did I mean?). But do that once we can determine the db-type from the wxDb object itself 
		SetColDefs(0, L"idfloattypes", DB_DATA_TYPE_INTEGER, &m_idFloatTypes, SQL_C_SLONG, sizeof(m_idFloatTypes), true, false, false, false);
		SetColDefs(1, L"tdouble", DB_DATA_TYPE_FLOAT, &m_double, SQL_C_DOUBLE, sizeof(m_double), false, false, false, false);
		SetColDefs(2, L"tfloat", DB_DATA_TYPE_FLOAT, &m_float, SQL_C_DOUBLE, sizeof(m_float), false, false, false, false);
	}

	// FloatTypesTable
	// ---------------
	FloatTypesTmpTable::FloatTypesTmpTable(wxDb* pDb)
		: wxDbTable(pDb, L"floattypes_tmp", 3, L"", wxDB_QUERY_ONLY)
	{
		m_idFloatTypes	= 0;
		m_double		= 0;
		m_float			= 0;

		// Note: When binding a column of type FLOAT (database-type), you still need to use SQL_C_DOUBLE
		// SQL_C_FLOAT is for REAL (database-type), which I will not test here, mysql doesnt know about it
		SetColDefs(0, L"idfloattypes_tmp", DB_DATA_TYPE_INTEGER, &m_idFloatTypes, SQL_C_SLONG, sizeof(m_idFloatTypes), true, false, false, false);
		SetColDefs(1, L"tdouble", DB_DATA_TYPE_FLOAT, &m_double, SQL_C_DOUBLE, sizeof(m_double), false, false, false, false);
		SetColDefs(2, L"tfloat", DB_DATA_TYPE_FLOAT, &m_float, SQL_C_DOUBLE, sizeof(m_float), false, false, false, false);
	}

	// NumericTypesTable
	// -----------------
	NumericTypesTable::NumericTypesTable(wxDb* pDb, ReadMode readMode)
		: wxDbTable(pDb, L"numerictypes", 3, L"", wxDB_QUERY_ONLY)
		, m_readMode(readMode)
	{
		m_idNumericTypes	= 0;
		ZeroMemory(&m_decimal_18_0, sizeof(m_decimal_18_0));
		ZeroMemory(&m_decimal_18_10, sizeof(m_decimal_18_10));
		m_wcdecimal_18_0[0] = 0;
		m_wcdecimal_18_10[0] = 0;

		// TODO: This DB_DATA_TYPE * stuff is probably useless, or must be extended to support more types. 
		// maybe used for formating to sql?
		SetColDefs(0, L"idnumerictypes", DB_DATA_TYPE_INTEGER, &m_idNumericTypes, SQL_C_SLONG, sizeof(m_idNumericTypes), true, false, false, false);
		if(m_readMode == ReasAsNumeric)
		{
			SetColDefs(1, L"tdecimal_18_0", DB_DATA_TYPE_FLOAT, &m_decimal_18_0, SQL_C_NUMERIC, sizeof(m_decimal_18_0), false, false, false, false);
			SetColDefs(2, L"tdecimal_18_10", DB_DATA_TYPE_FLOAT, &m_decimal_18_10, SQL_C_NUMERIC, sizeof(m_decimal_18_0), false, false, false, false);
		}
		else if(m_readMode == ReadAsChar)
		{
			SetColDefs(1, L"tdecimal_18_0", DB_DATA_TYPE_VARCHAR, m_wcdecimal_18_0, SQL_C_WCHAR, sizeof(m_wcdecimal_18_0), false, false, false, false);
			SetColDefs(2, L"tdecimal_18_10", DB_DATA_TYPE_VARCHAR, m_wcdecimal_18_10, SQL_C_WCHAR, sizeof(m_wcdecimal_18_10), false, false, false, false);
		}
	};

	// NumericTypesTable
	// -----------------
	NumericTypesTmpTable::NumericTypesTmpTable(wxDb* pDb, ReadMode readMode)
		: wxDbTable(pDb, L"numerictypes_tmp", 3, L"", wxDB_QUERY_ONLY)
		, m_readMode(readMode)
	{
		m_idNumericTypes	= 0;
		ZeroMemory(&m_decimal_18_0, sizeof(m_decimal_18_0));
		ZeroMemory(&m_decimal_18_10, sizeof(m_decimal_18_10));
		m_wcdecimal_18_0[0] = 0;
		m_wcdecimal_18_10[0] = 0;

		// TODO: This DB_DATA_TYPE * stuff is probably useless, or must be extended to support more types. 
		// maybe used for formating to sql?
		SetColDefs(0, L"idnumerictypes_tmp", DB_DATA_TYPE_INTEGER, &m_idNumericTypes, SQL_C_SLONG, sizeof(m_idNumericTypes), true, false, false, false);
		if(m_readMode == ReasAsNumeric)
		{
			SetColDefs(1, L"tdecimal_18_0", DB_DATA_TYPE_FLOAT, &m_decimal_18_0, SQL_C_NUMERIC, sizeof(m_decimal_18_0), false, false, false, false);
			SetColDefs(2, L"tdecimal_18_10", DB_DATA_TYPE_FLOAT, &m_decimal_18_10, SQL_C_NUMERIC, sizeof(m_decimal_18_0), false, false, false, false);
		}
		else if(m_readMode == ReadAsChar)
		{
			SetColDefs(1, L"tdecimal_18_0", DB_DATA_TYPE_VARCHAR, m_wcdecimal_18_0, SQL_C_WCHAR, sizeof(m_wcdecimal_18_0), false, false, false, false);
			SetColDefs(2, L"tdecimal_18_10", DB_DATA_TYPE_VARCHAR, m_wcdecimal_18_10, SQL_C_WCHAR, sizeof(m_wcdecimal_18_10), false, false, false, false);
		}
	};

	// BlobTypesTable
	// --------------
	BlobTypesTable::BlobTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"blobtypes", 2, L"", wxDB_QUERY_ONLY)
	{
		ZeroMemory(m_blob, sizeof(m_blob));

		SetColDefs(0, L"idblobtypes", DB_DATA_TYPE_INTEGER, &m_idBlobTypes, SQL_C_SLONG, sizeof(m_idBlobTypes), true, false, false, false);
		SetColDefs(1, L"tblob", DB_DATA_TYPE_VARCHAR, m_blob, SQL_C_BINARY, sizeof(m_blob), false, false, false, false);
	}

	// BlobTypesTmpTable
	// --------------
	BlobTypesTmpTable::BlobTypesTmpTable(wxDb* pDb)
		: wxDbTable(pDb, L"blobtypes_tmp", 2, L"", wxDB_QUERY_ONLY)
	{
		ZeroMemory(m_blob, sizeof(m_blob));

		SetColDefs(0, L"idblobtypes_tmp", DB_DATA_TYPE_INTEGER, &m_idBlobTypes, SQL_C_SLONG, sizeof(m_idBlobTypes), true, false, false, false);
		SetColDefs(1, L"tblob", DB_DATA_TYPE_VARCHAR, m_blob, SQL_C_BINARY, sizeof(m_blob), false, false, false, false);
	}

	// CharTable
	// ---------
	CharTable::CharTable(wxDb* pDb)
		: wxDbTable(pDb, L"chartable", 4, L"", wxDB_QUERY_ONLY)
	{
		m_idCharTable	= 0;
		m_col2[0] = 0;
		m_col3[0] = 0;
		m_col4[0] = 0;

		SetColDefs(0, L"idchartable", DB_DATA_TYPE_INTEGER, &m_idCharTable, SQL_C_SLONG, sizeof(m_idCharTable), true, false, false, false);
		SetColDefs(1, L"col2", DB_DATA_TYPE_VARCHAR, m_col2, SQL_C_WCHAR, sizeof(m_col2), false, false, false, false);
		SetColDefs(2, L"col3", DB_DATA_TYPE_VARCHAR, m_col3, SQL_C_WCHAR, sizeof(m_col3), false, false, false, false);
		SetColDefs(3, L"col4", DB_DATA_TYPE_VARCHAR, m_col4, SQL_C_WCHAR, sizeof(m_col4), false, false, false, false);
	}

	// IncompleteCharTable
	// -------------------
	IncompleteCharTable::IncompleteCharTable(wxDb* pDb)
		: wxDbTable(pDb, L"chartable", 4, L"", wxDB_QUERY_ONLY)
	{
		m_idCharTable	= 0;
		m_col2[0] = 0;
		//m_col3[0] = 0;
		m_col4[0] = 0;

		SetColDefs(0, L"idchartable", DB_DATA_TYPE_INTEGER, &m_idCharTable, SQL_C_SLONG, sizeof(m_idCharTable), true, false, false, false);
		SetColDefs(1, L"col2", DB_DATA_TYPE_VARCHAR, m_col2, SQL_C_WCHAR, sizeof(m_col2), false, false, false, false);
		//SetColDefs(2, L"col3", DB_DATA_TYPE_VARCHAR, m_col3, SQL_C_WCHAR, sizeof(m_col3), false, false, false, false);
		SetColDefs(3, L"col4", DB_DATA_TYPE_VARCHAR, m_col4, SQL_C_WCHAR, sizeof(m_col4), false, false, false, false);
	}
}