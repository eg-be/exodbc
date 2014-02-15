/*!
 * \file TestTables.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 09.02.2014
 * 
 * [Brief CPP-file description]
 */ 


// Own header
#include "TestTables.h"
// Same component headers
// Other headers
#include "db.h"

// Static consts
// -------------

// NotExistingTable
// ----------------
NotExistingTable::NotExistingTable(wxDb* pDb)
	: wxDbTable(pDb, L"notexisting", 1)
{
	SetColDefs(0, L"idnotexisting", DB_DATA_TYPE_INTEGER, &m_idNotExisting, SQL_C_LONG, sizeof(m_idNotExisting), true);

}

// QueryTypesTable
// ---------------
IntTypesTable::IntTypesTable(wxDb* pDb)
	: wxDbTable(pDb, L"integertypes", 4, L"", wxDB_QUERY_ONLY)
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

	int32_t* pAddr = &m_idIntegerTypes;

//	SetColDefs(0, L"IDT1", DB_DATA_TYPE_INTEGER, &m_idTest, SQL_C_SLONG, sizeof(m_idTest), true, false, false, false);

	SetColDefs(0, L"idintegertypes", DB_DATA_TYPE_INTEGER, &m_idIntegerTypes, SQL_C_SLONG, sizeof(m_idIntegerTypes), true, false, false, false);
	//SetColDefs(1, L"smallint", DB_DATA_TYPE_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), true, false, false, false);
	//SetColDefs(2, L"int", DB_DATA_TYPE_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), true, false, false, false);
	//SetColDefs(3, L"bigint", DB_DATA_TYPE_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), true, false, false, false);


	//SetColDefs(0, L"idintegertypes", DB_DATA_TYPE_INTEGER, &m_idQueryTypes, SQL_C_SLONG, sizeof(m_idQueryTypes), true, false, false, false);
	//SetColDefs(1, L"tinyint", DB_DATA_TYPE_INTEGER, &m_tinyInt, SQL_C_STINYINT, sizeof(m_tinyInt), true, false, false, false);
	//SetColDefs(2, L"smallint", DB_DATA_TYPE_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), true, false, false, false);
	//SetColDefs(3, L"mediumint", DB_DATA_TYPE_INTEGER, &m_mediumInt, SQL_C_SLONG, sizeof(m_mediumInt), true, false, false, false);
	//SetColDefs(4, L"int", DB_DATA_TYPE_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), true, false, false, false);
	//SetColDefs(5, L"bigint", DB_DATA_TYPE_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), true, false, false, false);

	//SetColDefs(6, L"utinyint", DB_DATA_TYPE_INTEGER, &m_utinyInt, SQL_C_UTINYINT, sizeof(m_utinyInt), true, false, false, false);
	//SetColDefs(7, L"usmallint", DB_DATA_TYPE_INTEGER, &m_usmallInt, SQL_C_USHORT, sizeof(m_usmallInt), true, false, false, false);
	//SetColDefs(8, L"umediumint", DB_DATA_TYPE_INTEGER, &m_umediumInt, SQL_C_ULONG, sizeof(m_umediumInt), true, false, false, false);
	//SetColDefs(9, L"uint", DB_DATA_TYPE_INTEGER, &m_uint, SQL_C_ULONG, sizeof(m_uint), true, false, false, false);
	//SetColDefs(10, L"ubigint", DB_DATA_TYPE_INTEGER, &m_ubigInt, SQL_C_UBIGINT, sizeof(m_ubigInt), true, false, false, false);


}



