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
	: wxDbTable(pDb, L"IntegerTypes", 6)
{
	SetColDefs(0, L"idintegertypes", DB_DATA_TYPE_INTEGER, &m_idQueryTypes, SQL_C_SLONG, sizeof(m_idQueryTypes), true);
	SetColDefs(1, L"tinyint", DB_DATA_TYPE_INTEGER, &m_tinyInt, SQL_C_STINYINT, sizeof(m_tinyInt), true);
	SetColDefs(2, L"smallint", DB_DATA_TYPE_INTEGER, &m_smallInt, SQL_C_SSHORT, sizeof(m_smallInt), true);
	SetColDefs(3, L"mediumint", DB_DATA_TYPE_INTEGER, &m_mediumInt, SQL_C_SLONG, sizeof(m_mediumInt), true);
	SetColDefs(4, L"int", DB_DATA_TYPE_INTEGER, &m_int, SQL_C_SLONG, sizeof(m_int), true);
	SetColDefs(5, L"bigint", DB_DATA_TYPE_INTEGER, &m_bigInt, SQL_C_SBIGINT, sizeof(m_bigInt), true);

}



