/*!
* \file Db2TestTables.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 23.02.2014
* 
* [Brief CPP-file description]
*/ 


// Own header
#include "Db2TestTables.h"
// Same component headers
// Other headers
#include "db.h"

// Static consts
// -------------

namespace DB2
{

	// IntTypesTable
	// ---------------
	IntTypesTable::IntTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"INTEGERTYPES", 4, L"", wxDB_QUERY_ONLY)
	{	
		m_idIntegerTypes = 13;
		m_smallInt = 0;
		m_int = 0;
		m_bigInt = 0;
	
		SetColDefs(0, L"IDINTEGERTYPES", DB_DATA_TYPE_INTEGER, &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes), true, false, false, false);
		SetColDefs(1, L"SMALLINT", DB_DATA_TYPE_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), true, false, false, false);
		SetColDefs(2, L"INT", DB_DATA_TYPE_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), true, false, false, false);
		SetColDefs(3, L"BIGINT", DB_DATA_TYPE_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), true, false, false, false);	
	}
	
	
	// CharTypesTable
	// --------------
	CharTypesTable::CharTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"CHARTYPES", 3, L"", wxDB_QUERY_ONLY)
	{
		m_idCharTypes	= 0;
		m_varchar[0]	= 0;
		m_char[0]		= 0;
	
		SetColDefs(0, L"IDCHARTYPES", DB_DATA_TYPE_INTEGER, &m_idCharTypes, SQL_C_SLONG, sizeof(m_idCharTypes), true, false, false, false);
		SetColDefs(1, L"VARCHAR", DB_DATA_TYPE_VARCHAR, m_varchar, SQL_C_WCHAR, sizeof(m_varchar), false, false, false, false);
		SetColDefs(2, L"CHAR", DB_DATA_TYPE_VARCHAR, m_char, SQL_C_WCHAR, sizeof(m_char), false, false, false, false);
	}
	
	
	// FloatTypesTable
	// ---------------
	FloatTypesTable::FloatTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"FLOATTYPES", 6, L"", wxDB_QUERY_ONLY)
	{
		m_idFloatTypes	= 0;
		m_float			= 0.0;
		m_double		= 0.0;
		m_real			= 0.0f;
		m_decimal_15_10[0] = 0;
		m_decimal_10_0[0] = 0;
	
		SetColDefs(0, L"IDFLOATTYPES", DB_DATA_TYPE_INTEGER, &m_idFloatTypes, SQL_C_SLONG, sizeof(m_idFloatTypes), true, false, false, false);
		SetColDefs(1, L"DOUBLE", DB_DATA_TYPE_FLOAT, &m_double, SQL_C_DOUBLE, sizeof(m_double), false, false, false, false);
		SetColDefs(2, L"FLOAT", DB_DATA_TYPE_FLOAT, &m_float, SQL_C_DOUBLE, sizeof(m_float), false, false, false, false);
		SetColDefs(3, L"REAL", DB_DATA_TYPE_FLOAT, &m_real, SQL_C_FLOAT, sizeof(m_real), false, false, false, false);
		SetColDefs(4, L"DECIMAL_15_10", DB_DATA_TYPE_VARCHAR, &m_decimal_15_10, SQL_C_WCHAR, sizeof(m_decimal_15_10), false, false, false, false);
		SetColDefs(5, L"DECIMAL_10_0", DB_DATA_TYPE_VARCHAR, &m_decimal_10_0, SQL_C_WCHAR, sizeof(m_decimal_10_0), false, false, false, false);
		
	}
	
	
	//// DateTypesTable
	//// ---------------
	//DateTypesTable::DateTypesTable(wxDb* pDb)
	//	: wxDbTable(pDb, L"datetypes", 2, L"", wxDB_QUERY_ONLY)
	//{
	//	m_idDateTypes		= 0;
	//	ZeroMemory(&m_date, sizeof(m_date));
	//	ZeroMemory(&m_datetime, sizeof(m_datetime));
	//	ZeroMemory(&m_time, sizeof(m_time));
	//	ZeroMemory(&m_timestamp, sizeof(m_timestamp));
	//
	//	SetColDefs(0, L"iddatetypes", DB_DATA_TYPE_INTEGER, &m_idDateTypes, SQL_C_SLONG, sizeof(m_idDateTypes), true, false, false, false);
	////	SetColDefs(1, L"date", DB_DATA_TYPE_DATE, &m_date, SQL_C_TYPE_DATE, sizeof(m_date), false, false, false, false);
	////	SetColDefs(1, L"datetime", DB_DATA_TYPE_DATE, &m_datetime, SQL_C_TYPE_TIMESTAMP, sizeof(m_datetime), false, false, false, false);
	////	SetColDefs(1, L"time", DB_DATA_TYPE_DATE, &m_time, SQL_C_TYPE_TIME, sizeof(m_time), false, false, false, false);
	////	SetColDefs(1, L"timestamp", DB_DATA_TYPE_DATE, &m_timestamp, SQL_C_TYPE_TIMESTAMP, sizeof(m_timestamp), false, false, false, false);
	//
	//}

}