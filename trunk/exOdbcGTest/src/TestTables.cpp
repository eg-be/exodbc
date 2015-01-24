/*!
* \file TestTables.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 26.12.2014
* \copyright wxWindows Library Licence, Version 3.1
*
*  Defines some helpers for TestTables, like converting names to upper/lower case, etc.
*/

#include "stdafx.h"

// Own header
#include "TestTables.h"

// Same component headers
// Other headers

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
	namespace TestTables
	{
		std::wstring GetTableName(const std::wstring& tableName, NameCase nameCase)
		{
			return (nameCase == NC_UPPER ? boost::algorithm::to_upper_copy(tableName) : boost::algorithm::to_lower_copy(tableName));
		}

		std::wstring GetColName(const std::wstring& columnName, NameCase nameCase)
		{
			return GetTableName(columnName, nameCase);
		}
	}
}