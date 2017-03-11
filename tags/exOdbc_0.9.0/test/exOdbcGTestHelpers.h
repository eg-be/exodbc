#pragma once
/*!
* \file exOdbcGTestHelpers.h
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2015
* \copyright GNU Lesser General Public License Version 3
*
* A file for helpers common to all tests. But not as complicated
* as the last stuff, that was pretty useless.
*/

#pragma once

// Same component headers
#include "exOdbcGTest.h"

// Other headers
#include "exodbc/Environment.h"
#include "exodbc/Database.h"

// System headers
#include <map>

// Forward declarations
// --------------------

namespace exodbctest
{
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

	extern const std::map<TableId, std::string> g_TableNames;
	extern const std::map<TableId, std::string> g_IdColumnNames;

	extern exodbc::EnvironmentPtr CreateEnv(exodbc::OdbcVersion odbcVersion = exodbc::OdbcVersion::V_3);
	extern exodbc::DatabasePtr OpenTestDb(exodbc::ConstEnvironmentPtr pEnv);
	extern exodbc::DatabasePtr OpenTestDb(exodbc::OdbcVersion odbcVersion = exodbc::OdbcVersion::V_3);

	extern std::string PrependSchemaOrCatalogName(exodbc::DatabaseProduct dbms, const std::string& name);
	extern std::string GetTableName(TableId tableId);
	extern std::string GetIdColumnName(TableId tableId);
	extern std::string ToDbCase(const std::string& str);

	extern void ClearTmpTable(TableId tmpTableId);

	// Structs
	// -------

	// Classes
	// -------

} // namespace exodbc

