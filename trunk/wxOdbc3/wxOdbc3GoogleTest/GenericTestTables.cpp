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
}