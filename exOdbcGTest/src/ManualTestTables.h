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

		MIntTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER, const std::wstring& name = L"IntegerTypes");
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
		MFloatTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER, const std::wstring& name = L"FloatTypes");
		virtual ~MFloatTypesTable() {};

		SQLINTEGER				m_idFloatTypes;
		SQLDOUBLE				m_double;
		SQLFLOAT				m_float;
	};


	// CharTypesTable
	// --------------
	class MCharTypesTable : public exodbc::Table
	{
	public:

		MCharTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER, const std::wstring& name = L"CharTypes");
		virtual ~MCharTypesTable() {};

		SQLINTEGER	m_idCharTypes;
		SQLCHAR	m_varchar[128 + 1];
		SQLCHAR	m_char[128 + 1];
	};



	// WCharTypesTable
	// --------------
	class MWCharTypesTable : public exodbc::Table
	{
	public:

		MWCharTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER, const std::wstring& name = L"CharTypes");
		virtual ~MWCharTypesTable() {};

		SQLINTEGER	m_idCharTypes;
		SQLWCHAR	m_varchar[128 + 1];
		SQLWCHAR	m_char[128 + 1];
	};


	// DateTypesTable
	// --------------
	class MDateTypesTable : public exodbc::Table
	{
	public:
		MDateTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER, const std::wstring& name = L"DateTypes");
		virtual ~MDateTypesTable() {};

		SQLINTEGER				m_idDateTypes;
		SQL_DATE_STRUCT			m_date;
		SQL_TIME_STRUCT			m_time;
		SQL_TIMESTAMP_STRUCT	m_timestamp;
	};

	
	// BlobTypesTable
	// --------------
	class MBlobTypesTable : public exodbc::Table
	{
	public:
		MBlobTypesTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER, const std::wstring& name = L"BlobTypes");
		virtual ~MBlobTypesTable() {};

		SQLINTEGER		m_idBlobTypes;
		SQLCHAR			m_blob[16];
	};


	// MNumbericTypesAsCharTable
	// ------------------
	class MNumericTypesAsCharTable : public exodbc::Table
	{
	public:
		MNumericTypesAsCharTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER, const std::wstring& name = L"NumericTypes");
		virtual ~MNumericTypesAsCharTable() {};

		SQLINTEGER			m_idNumericTypes;

		SQLWCHAR			m_wcdecimal_18_0[20 + 1];
		SQLWCHAR			m_wcdecimal_18_10[20 + 1];
	};


	// CharTable
	// ---------
	class MCharTable : public exodbc::Table
	{
	public:
		MCharTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~MCharTable() {};

		SQLINTEGER	m_idCharTable;
		SQLWCHAR	m_col2[128 + 1];
		SQLWCHAR	m_col3[128 + 1];
		SQLWCHAR	m_col4[128 + 2];
	};


	// IncompleCharTable
	// -----------------
	class MIncompleteCharTable : public exodbc::Table
	{
	public:
		MIncompleteCharTable(exodbc::Database* pDb, TestTables::NameCase namesCase = TestTables::NC_LOWER);
		virtual ~MIncompleteCharTable() {};

		SQLINTEGER	m_idCharTable;
		SQLWCHAR	m_col2[128 + 1];
		// we do not bind col3 and have a gap
		//SQLWCHAR	m_col3[128 + 1];
		SQLWCHAR	m_col4[128 + 2];
	};
}