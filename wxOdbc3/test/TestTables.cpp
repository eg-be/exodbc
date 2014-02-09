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

// QueryTypesTable
// ---------------
QueryTypesTable::QueryTypesTable(wxDb* pDb)
	: wxDbTable(pDb, L"querytypes", 1)
{
	SetColDefs(0, L"idquerytypes", DB_DATA_TYPE_INTEGER, &m_idQueryTypes, SQL_C_LONG, sizeof(m_idQueryTypes), true);
}

QueryTypesTable::~QueryTypesTable()
{

}



