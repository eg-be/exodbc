/*!
* \file PrimaryKeyInfo.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 01.05.2017
* \brief Source file for PrimaryKeyInfo.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "PrimaryKeyInfo.h"

// Same component headers
#include "AssertionException.h"
#include "Helpers.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Class TablePrimaryKeyInfo
	// =========================
	PrimaryKeyInfo::PrimaryKeyInfo()
		: m_keySequence(0)
		, m_isKeyNameNull(true)
		, m_isCatalogNull(true)
		, m_isSchemaNull(true)
	{}


	PrimaryKeyInfo::PrimaryKeyInfo(const std::string& tableName, const std::string& columnName, SQLSMALLINT keySequence)
		: m_tableName(tableName)
		, m_columnName(columnName)
		, m_keySequence(keySequence)
		, m_isCatalogNull(true)
		, m_isSchemaNull(true)
		, m_isKeyNameNull(true)
	{ }


	PrimaryKeyInfo::PrimaryKeyInfo(const std::string& catalogName, const std::string& schemaName, const std::string& tableName, const std::string& columnName,
		SQLSMALLINT keySequence, const std::string& keyName, bool isCatalogNull, bool isSchemaNull, bool isPrimaryKeyNameNull)
		: m_catalogName(catalogName)
		, m_schemaName(schemaName)
		, m_tableName(tableName)
		, m_columnName(columnName)
		, m_keySequence(keySequence)
		, m_keyName(keyName)
		, m_isCatalogNull(isCatalogNull)
		, m_isSchemaNull(isSchemaNull)
		, m_isKeyNameNull(isPrimaryKeyNameNull)
	{ }


	PrimaryKeyInfo::PrimaryKeyInfo(ConstSqlStmtHandlePtr pStmt, const SqlInfoProperties& props)
	{
		exASSERT(pStmt);
		exASSERT(pStmt->IsAllocated());

		SQLLEN cb = 0;
		GetData(pStmt, 1, props.GetMaxCatalogNameLen(), m_catalogName, &m_isCatalogNull);
		GetData(pStmt, 2, props.GetMaxSchemaNameLen(), m_schemaName, &m_isSchemaNull);
		GetData(pStmt, 3, props.GetMaxTableNameLen(), m_tableName);
		GetData(pStmt, 4, props.GetMaxColumnNameLen(), m_columnName);
		GetData(pStmt, 5, SQL_C_SHORT, &m_keySequence, sizeof(m_keySequence), &cb, NULL);
		GetData(pStmt, 6, DB_MAX_PRIMARY_KEY_NAME_LEN, m_keyName, &m_isKeyNameNull);
	}


	std::string PrimaryKeyInfo::GetQueryName() const
	{
		exASSERT(!m_columnName.empty());
		return m_columnName;
	}


	std::string PrimaryKeyInfo::GetColumnName() const
	{
		exASSERT(!m_columnName.empty());
		return m_columnName;
	}
}
