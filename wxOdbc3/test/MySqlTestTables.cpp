/*!
* \file MySqlTestTables.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 09.02.2014
* 
* [Brief CPP-file description]
*/ 


// Own header
#include "MySqlTestTables.h"
// Same component headers
// Other headers
#include "db.h"

// Static consts
// -------------

namespace MySql
{


	// IntTypesTable
	// ---------------
	IntTypesTable::IntTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"integertypes", 11, L"", wxDB_QUERY_ONLY)
		//: wxDbTable(pDb, L"wxodbc3.integertypes", 1, L"", wxDB_QUERY_ONLY)
		//: wxDbTable(pDb, L"TEST.T1", 1, L"", wxDB_QUERY_ONLY)
	{	
		m_idIntegerTypes = 13;
		m_tinyInt = 0;
		m_smallInt = 0;
		m_mediumInt = 0;
		m_int = 0;
		m_bigInt = 0;

		m_utinyInt = 0;
		m_usmallInt = 0;
		m_umediumInt = 0;
		m_uint = 0;
		m_ubigInt = 0;

		//	SetColDefs(0, L"IDT1", DB_DATA_TYPE_INTEGER, &m_idTest, SQL_C_SLONG, sizeof(m_idTest), true, false, false, false);

		//SetColDefs(0, L"idintegertypes", DB_DATA_TYPE_INTEGER, &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes), true, false, false, false);
		//SetColDefs(1, L"smallint", DB_DATA_TYPE_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), true, false, false, false);
		//SetColDefs(2, L"int", DB_DATA_TYPE_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), true, false, false, false);
		//SetColDefs(3, L"bigint", DB_DATA_TYPE_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), true, false, false, false);


		SetColDefs(0, L"idintegertypes", DB_DATA_TYPE_INTEGER, &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes), true, false, false, false);
		SetColDefs(1, L"tinyint", DB_DATA_TYPE_INTEGER, &m_tinyInt, SQL_C_STINYINT, sizeof(m_tinyInt), true, false, false, false);
		SetColDefs(2, L"smallint", DB_DATA_TYPE_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), true, false, false, false);
		SetColDefs(3, L"mediumint", DB_DATA_TYPE_INTEGER, &m_mediumInt, SQL_C_SLONG, sizeof(m_mediumInt), true, false, false, false);
		SetColDefs(4, L"int", DB_DATA_TYPE_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), true, false, false, false);
		SetColDefs(5, L"bigint", DB_DATA_TYPE_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), true, false, false, false);

		SetColDefs(6, L"utinyint", DB_DATA_TYPE_INTEGER, &m_utinyInt, SQL_C_UTINYINT, sizeof(m_utinyInt), true, false, false, false);
		SetColDefs(7, L"usmallint", DB_DATA_TYPE_INTEGER, &m_usmallInt, SQL_C_USHORT, sizeof(m_usmallInt), true, false, false, false);
		SetColDefs(8, L"umediumint", DB_DATA_TYPE_INTEGER, &m_umediumInt, SQL_C_ULONG, sizeof(m_umediumInt), true, false, false, false);
		SetColDefs(9, L"uint", DB_DATA_TYPE_INTEGER, &m_uint, SQL_C_ULONG, sizeof(m_uint), true, false, false, false);
		SetColDefs(10, L"ubigint", DB_DATA_TYPE_INTEGER, &m_ubigInt, SQL_C_UBIGINT, sizeof(m_ubigInt), true, false, false, false);


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
		SetColDefs(1, L"varchar", DB_DATA_TYPE_VARCHAR, m_varchar, SQL_C_WCHAR, sizeof(m_varchar), false, false, false, false);
		SetColDefs(2, L"char", DB_DATA_TYPE_VARCHAR, m_char, SQL_C_WCHAR, sizeof(m_char), false, false, false, false);
	}


	// FloatTypesTable
	// ---------------
	FloatTypesTable::FloatTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"floattypes", 5, L"", wxDB_QUERY_ONLY)
	{
		m_idFloatTypes	= 0;
		m_float			= 0;
		m_double		= 0;
		m_decimal_10_0[0] = 0;
		m_decimal_15_10[0] = 0;
		//ZeroMemory(&m_decimal, sizeof(m_decimal));

		SetColDefs(0, L"idfloattypes", DB_DATA_TYPE_INTEGER, &m_idFloatTypes, SQL_C_SLONG, sizeof(m_idFloatTypes), true, false, false, false);
		SetColDefs(1, L"float", DB_DATA_TYPE_FLOAT, &m_float, SQL_C_DOUBLE, sizeof(m_float), false, false, false, false);
		SetColDefs(2, L"double", DB_DATA_TYPE_FLOAT, &m_double, SQL_C_DOUBLE, sizeof(m_double), false, false, false, false);
		SetColDefs(3, L"decimal_15_10", DB_DATA_TYPE_VARCHAR, m_decimal_15_10, SQL_C_WCHAR, sizeof(m_decimal_15_10), false, false, false, false);
		SetColDefs(4, L"decimal_10_0", DB_DATA_TYPE_VARCHAR, m_decimal_10_0, SQL_C_WCHAR, sizeof(m_decimal_10_0), false, false, false, false);

		// TODO: Fails with MySql ODBC 3.51 with a restricted attribute error (not supported by driver maybe?)
		// Its an odbc 3 feature
		//	SetColDefs(3, L"decimal", DB_DATA_TYPE_FLOAT, &m_decimal, SQL_C_NUMERIC, sizeof(m_decimal), false, false, false, false);

	}


	// DateTypesTable
	// ---------------
	DateTypesTable::DateTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"datetypes", 6, L"", wxDB_QUERY_ONLY)
	{
		m_idDateTypes		= 0;
		ZeroMemory(&m_date, sizeof(m_date));
		ZeroMemory(&m_datetime, sizeof(m_datetime));
		ZeroMemory(&m_time, sizeof(m_time));
		ZeroMemory(&m_timestamp, sizeof(m_timestamp));

		// Note: We are odbc v. 2, therefore use the old s-type (without type: SQL_C_DATE instead of SQL_C_TYPE_DATE). See Ticket # 17
		SetColDefs(0, L"iddatetypes", DB_DATA_TYPE_INTEGER, &m_idDateTypes, SQL_C_SLONG, sizeof(m_idDateTypes), true, false, false, false);
		SetColDefs(1, L"date", DB_DATA_TYPE_DATE, &m_date, SQL_C_DATE, sizeof(m_date), false, false, false, false);
		SetColDefs(2, L"datetime", DB_DATA_TYPE_DATE, &m_datetime, SQL_C_TIMESTAMP, sizeof(m_datetime), false, false, false, false);
		SetColDefs(3, L"time", DB_DATA_TYPE_DATE, &m_time, SQL_C_TIME, sizeof(m_time), false, false, false, false);
		SetColDefs(4, L"timestamp", DB_DATA_TYPE_DATE, &m_timestamp, SQL_C_TIMESTAMP, sizeof(m_timestamp), false, false, false, false);
		SetColDefs(5, L"year", DB_DATA_TYPE_INTEGER, &m_year, SQL_C_SSHORT, sizeof(m_year), false, false, false, false);
	}

}