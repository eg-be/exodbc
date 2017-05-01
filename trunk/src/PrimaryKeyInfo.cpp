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
		, m_isPrimaryKeyNameNull(true)
		, m_isCatalogNull(true)
		, m_isSchemaNull(true)
	{}


	PrimaryKeyInfo::PrimaryKeyInfo(const std::string& tableName, const std::string& columnName, SQLSMALLINT keySequence)
		: m_tableName(tableName)
		, m_columnName(columnName)
		, m_keySequence(keySequence)
		, m_isCatalogNull(true)
		, m_isSchemaNull(true)
		, m_isPrimaryKeyNameNull(true)
	{}


	PrimaryKeyInfo::PrimaryKeyInfo(const std::string& catalogName, const std::string& schemaName, const std::string& tableName, const std::string& columnName,
		SQLSMALLINT keySequence, const std::string& keyName, bool isCatalogNull, bool isSchemaNull, bool isPrimaryKeyNameNull)
		: m_catalogName(catalogName)
		, m_schemaName(schemaName)
		, m_tableName(tableName)
		, m_columnName(columnName)
		, m_keySequence(keySequence)
		, m_primaryKeyName(keyName)
		, m_isCatalogNull(isCatalogNull)
		, m_isSchemaNull(isSchemaNull)
		, m_isPrimaryKeyNameNull(isPrimaryKeyNameNull)
	{}


	std::string PrimaryKeyInfo::GetQueryName() const
	{
		return GetPureName();
	}


	std::string PrimaryKeyInfo::GetPureName() const
	{
		exASSERT(!m_columnName.empty());
		return m_columnName;
	}
}
