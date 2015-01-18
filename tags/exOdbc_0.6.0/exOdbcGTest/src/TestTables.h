/*!
* \file TestTables.h
* \author Elias Gerber <eg@zame.ch>
* \date 26.12.2014
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
		enum NameCase
		{
			NC_UPPER,	///< Tables will be created using all UPPERCASE letters for table- and column-names
			NC_LOWER	///< Tables will be created using all lowercase letters for table- and column-names
		};

		std::wstring GetTableName(const std::wstring& tableName, NameCase nameCase);
		std::wstring GetColName(const std::wstring& columnName, NameCase nameCase);
	}
}