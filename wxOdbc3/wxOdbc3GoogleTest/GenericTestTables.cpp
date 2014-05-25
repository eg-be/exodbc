/*!
 * \file GenericTestTables.cpp
 * \author Elias Gerber <eg@zame.ch>
 * \date 23.02.2014
 * 
 * [Brief CPP-file description]
 */ 


// Own header
#include "GenericTestTables.h"
// Same component headers
// Other headers
#include "db.h"

// Static consts
// -------------

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
		SetColDefs(1, L"varchar", DB_DATA_TYPE_VARCHAR, m_varchar, SQL_C_WCHAR, sizeof(m_varchar), false, false, false, false);
		SetColDefs(2, L"char", DB_DATA_TYPE_VARCHAR, m_char, SQL_C_WCHAR, sizeof(m_char), false, false, false, false);
	}

	// IntTypesTable
	// ---------------
	IntTypesTable::IntTypesTable(wxDb* pDb)
		: wxDbTable(pDb, L"INTEGERTYPES", 7, L"", wxDB_QUERY_ONLY)
	{	
		m_idIntegerTypes = 0;
		m_smallInt = 0;
		m_int = 0;
		m_bigInt = 0;
		m_usmallInt = 0;
		m_uint = 0;
		m_ubigInt = 0;

		SetColDefs(0, L"IDINTEGERTYPES", DB_DATA_TYPE_INTEGER, &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes), true, false, false, false);
		SetColDefs(1, L"SMALLINT", DB_DATA_TYPE_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), false, false, false, false);
		SetColDefs(2, L"INT", DB_DATA_TYPE_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), false, false, false, false);
		SetColDefs(3, L"BIGINT", DB_DATA_TYPE_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), false, false, false, false);	
		SetColDefs(4, L"USMALLINT", DB_DATA_TYPE_INTEGER, &m_usmallInt, SQL_C_USHORT, sizeof(m_usmallInt), false, false, false, false);	
		SetColDefs(5, L"UINT", DB_DATA_TYPE_INTEGER, &m_uint, SQL_C_ULONG, sizeof(m_usmallInt), false, false, false, false);	
		SetColDefs(6, L"UBIGINT", DB_DATA_TYPE_INTEGER, &m_ubigInt, SQL_C_UBIGINT, sizeof(m_usmallInt), false, false, false, false);	
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
		SetColDefs(1, L"date", DB_DATA_TYPE_DATE, &m_date, SQL_C_DATE, sizeof(m_date), false, false, false, false);
		SetColDefs(2, L"time", DB_DATA_TYPE_DATE, &m_time, SQL_C_TIME, sizeof(m_time), false, false, false, false);
		SetColDefs(3, L"timestamp", DB_DATA_TYPE_DATE, &m_timestamp, SQL_C_TIMESTAMP, sizeof(m_timestamp), false, false, false, false);
	}
}