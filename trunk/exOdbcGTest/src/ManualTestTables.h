/*!
* \file ManualTestTables.h
* \author Elias Gerber <eg@zame.ch>
* \date 26.12.2014
*
* Declares classes derived from exodbc::Table that define their columns manually.
*/

#pragma once

// Same component headers
#include "TestTables.h"

// Other headers
#include "Table.h"

// System headers

// Forward declarations
// --------------------
namespace exodbc
{
	class Database;
}

// Structs
// -------

// Classes
// -------
namespace exodbc
{
	// NotExistingTable
	// ----------------
	class MNotExistingTable : public exodbc::Table
	{
	public:
		MNotExistingTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		~MNotExistingTable() {};

		SQLINTEGER		m_idNotExisting;
	};


	// IntTypesTable
	// ----------
	class MIntTypesTable : public exodbc::Table
	{
	public:

		MIntTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~MIntTypesTable() {};

		// Size of Type							Bytes		Min						Max
		SQLINTEGER		m_idIntegerTypes;	//	4

		SQLSMALLINT		m_smallInt;			//	2			-32768 					32767
		SQLINTEGER		m_int;				//	4			-2147483648 			2147483647		
		SQLBIGINT		m_bigInt;			//	8 			-9223372036854775808 	9223372036854775807
	};


	// FloatTypesTable
	// --------------
	class MFloatTypesTable : public exodbc::Table
	{
	public:
		MFloatTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~MFloatTypesTable() {};

		SQLINTEGER				m_idFloatTypes;
		SQLDOUBLE				m_double;
		SQLFLOAT				m_float;
	};
}