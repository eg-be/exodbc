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

	// NotExistingTable
	// ----------------
	MNotExistingTable::MNotExistingTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 1, TestTables::GetTableName(L"notexisting", namesCase))
	{
		m_idNotExisting = 0;

		SetColumn(0, TestTables::GetColName(L"idnotexisting", namesCase), &m_idNotExisting, SQL_C_SLONG, sizeof(m_idNotExisting));
	}


	// IntTypesTable
	// ---------------
	MIntTypesTable::MIntTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 4, TestTables::GetTableName(L"integertypes", namesCase), L"", L"", L"", Table::READ_ONLY)
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
	MFloatTypesTable::MFloatTypesTable(Database* pDb, TestTables::NameCase namesCase /* = TestTables::NC_LOWER */)
		: Table(pDb, 3, TestTables::GetTableName(L"floattypes", namesCase), L"", L"", L"", Table::READ_ONLY)
	{
		m_idFloatTypes = 0;
		m_double = 0;
		m_float = 0;

		// Note: When binding a column of type FLOAT (database-type), you still need to use SQL_C_DOUBLE
		// SQL_C_FLOAT is for REAL (database-type), which I will not test here, mysql doesnt know about it
		// TODO: Test it for db-2 specific test (REAL-type? what did I mean?). But do that once we can determine the db-type from the wxDb object itself 
		SetColumn(0, TestTables::GetColName(L"idfloattypes", namesCase), &m_idFloatTypes, SQL_C_SLONG, sizeof(m_idFloatTypes));
		SetColumn(1, TestTables::GetColName(L"tdouble", namesCase), &m_double, SQL_C_DOUBLE, sizeof(m_double));
		SetColumn(2, TestTables::GetColName(L"tfloat", namesCase), &m_float, SQL_C_DOUBLE, sizeof(m_float));
	}
}