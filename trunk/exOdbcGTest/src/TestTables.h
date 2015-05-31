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
	namespace test
	{

		enum class Case
		{
			UPPER,	///< Tables will be created using all UPPERCASE letters for table- and column-names
			LOWER	///< Tables will be created using all lowercase letters for table- and column-names
		};

		std::wstring ConvertNameCase(const std::wstring& columnOrTableName, Case nameCase);

		enum class TableId
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


		enum class ValueIndicator
		{
			NO_INDICATOR,	///< Compare the value normally - it must be equal
			IS_NULL,		///< Expect the value to be NULL
			IGNORE_VAL		///< Ignore this value while comparing
		};

		typedef boost::variant<ValueIndicator, SQLSMALLINT> SmallInt;
		typedef boost::variant<ValueIndicator, SQLINTEGER > Int;
		typedef boost::variant<ValueIndicator, SQLBIGINT> BigInt;

		/*!
		* \brief	Test if the currently Select() ed record of the passed Table matches the passed expected values.
		* \details	If the passed db is of DatabaseType::ACCESS, the BigInt column is ignored and the SmallInt column
		*			is compared using SQLINTEGER (as access has no SQLSMALLINT, nor SQLBIGINT).
		* \return	AssertionSuccess or AssertionFailure.
		*/
		::testing::AssertionResult IsIntRecordEqual(const exodbc::Database& db, const exodbc::Table& iTable, Int expId, SmallInt expSmallInt, Int expInt, BigInt expBigInt);

		extern const std::map<test::TableId, std::wstring> TableNames;
		extern const std::map<test::TableId, std::wstring> IdColumnNames;

		std::wstring GetTableName(test::TableId table, test::Case nameCase);

		std::wstring GetIdColumnName(test::TableId table, test::Case nameCase);
		
		// \todo: See ticket #82
		// exodbc::Table GetEmptyTestTable(TestTables::Table table, TestTables::NameCase nameCase, exodbc::Database& db);

		void ClearTestTable(test::TableId table, test::Case nameCase, exodbc::Table& testTable, exodbc::Database& db);

		void ClearIntTable(const exodbc::Database& db, test::Case nameCase);

		static const SQLSMALLINT NULL_INT_VALUE = -666;

		// \todo: Replace by some methods that use the typedef of SmallInt, Int, etc.
		// \deprecated
		void InsertIntTypes(test::Case nameCase, const exodbc::Database& db, SQLINTEGER id, SQLSMALLINT smallInt, SQLINTEGER i, SQLBIGINT bigInt, bool commit = true);
		// \deprecated
		void InsertIntTypes(const exodbc::Table& insertableTable, const exodbc::Database& db, SQLINTEGER id, SQLSMALLINT smallInt, SQLINTEGER i, SQLBIGINT bigInt, bool commit = true);
	}


}