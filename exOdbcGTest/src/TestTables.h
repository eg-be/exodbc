/*!
* \file TestTables.h
* \author Elias Gerber <eg@elisium.ch>
* \date 26.12.2014
* \copyright wxWindows Library Licence, Version 3.1
*
* Declares some helpers for TestTables, like converting names to upper/lower case, etc.
*/

#pragma once

// Same component headers
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
	namespace TestTables
	{
		enum class NameCase
		{
			UPPER,	///< Tables will be created using all UPPERCASE letters for table- and column-names
			LOWER	///< Tables will be created using all lowercase letters for table- and column-names
		};

		std::wstring ConvertNameCase(const std::wstring& columnOrTableName, NameCase nameCase);

		enum class Table
		{
			BLOBTYPES,
			BLOBTYPES_TMP,
			CHARTABLE,
			CHARTYPES,
			CHARTYPES_TMP,
			DATETYPES,
			DATETYPES_TMP,
			FLOATTYPES,
			FLOATTYPES_TMP,
			INTEGERTYPES,
			INTEGERTYPES_TMP,
			MULTIKEY,
			NUMERICTYPES,
			NUMERICTYPES_TMP,
			SELECTONLY,
			NOT_EXISTING,
			NOT_SUPPORTED,
			NOT_SUPPORTED_TMP
		};

		extern const std::map<TestTables::Table, std::wstring> TableNames;
		extern const std::map<TestTables::Table, std::wstring> IdColumnNames;

		std::wstring GetTableName(TestTables::Table table, TestTables::NameCase nameCase);

		std::wstring GetIdColumnName(TestTables::Table table, TestTables::NameCase nameCase);
		
		// \todo: See ticket #82
		// exodbc::Table GetEmptyTestTable(TestTables::Table table, TestTables::NameCase nameCase, exodbc::Database& db);

		void ClearTestTable(TestTables::Table table, TestTables::NameCase nameCase, exodbc::Table& testTable, exodbc::Database& db);
		void ClearTestTable(const exodbc::Table& deletableTable, const exodbc::Database& db);
		void ClearTestTable(TestTables::Table table, TestTables::NameCase nameCase, const exodbc::Database& db);

		static const SQLBIGINT NULL_INT_VALUE = -13141315;
		void InsertIntTypes(TestTables::NameCase nameCase, const exodbc::Database& db, SQLINTEGER id, SQLSMALLINT smallInt, SQLINTEGER i, SQLBIGINT bigInt, bool commit = true);
		void InsertIntTypes(const exodbc::Table& insertableTable, const exodbc::Database& db, SQLINTEGER id, SQLSMALLINT smallInt, SQLINTEGER i, SQLBIGINT bigInt, bool commit = true);
	}


}