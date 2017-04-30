/*!
* \file InfoObject.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2015
* \brief Source file for info objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "TableInfo.h"

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
	// Class TableInfo
	// ===============

	TableInfo::TableInfo()
		: m_isCatalogNull(false)
		, m_isSchemaNull(false)
		, m_dbms(DatabaseProduct::UNKNOWN)
	{ }


	TableInfo::TableInfo(const std::string& tableName, const std::string& tableType, const std::string& tableRemarks, const std::string& catalogName, const std::string schemaName, DatabaseProduct dbms /* = DatabaseProduct::UNKNOWN */)
		: m_tableName(tableName)
		, m_tableType(tableType)
		, m_tableRemarks(tableRemarks)
		, m_catalogName(catalogName)
		, m_schemaName(schemaName)
		, m_dbms(dbms)
		, m_isCatalogNull(catalogName.empty())
		, m_isSchemaNull(schemaName.empty())
	{}


	TableInfo::TableInfo(const std::string& tableName, const std::string& tableType, const std::string& tableRemarks, const std::string& catalogName, const std::string schemaName, bool isCatalogNull, bool isSchemaNull, DatabaseProduct dbms /* = DatabaseProduct::UNKNOWN */)
		: m_tableName(tableName)
		, m_tableType(tableType)
		, m_tableRemarks(tableRemarks)
		, m_catalogName(catalogName)
		, m_schemaName(schemaName)
		, m_dbms(dbms)
		, m_isCatalogNull(isCatalogNull)
		, m_isSchemaNull(isSchemaNull)
	{}


	std::string TableInfo::GetQueryName() const
	{
		exASSERT( ! m_tableName.empty());

		switch (m_dbms)
		{
		case DatabaseProduct::ACCESS:
			// For access, return only the pure table name.
			return m_tableName;
			break;
		case DatabaseProduct::EXCEL:
			// For excel, add '[' and ']' around the pure table name.
			return u8"[" + m_tableName + u8"]";
			break;
		}

		// As default, include everything we have
		std::stringstream ss;
		if (HasCatalog())
		{
			ss << m_catalogName << u8".";
		}
		if (HasSchema())
		{
			ss << m_schemaName << u8".";
		}
		ss << m_tableName;

		std::string str = ss.str();
		return str;
	}


	std::string TableInfo::GetPureName() const
	{
		exASSERT(! m_tableName.empty());

		return m_tableName;
	}


	std::string TableInfo::ToString() const noexcept
	{
		stringstream ss;
		ss << u8"Name: '" << m_tableName << u8"'; ";
		ss << u8"Schema: '" << m_schemaName << u8"'; ";
		ss << u8"Catalog: '" << m_catalogName << u8"'; ";
		ss << u8"Type: '" << m_tableType << u8"'";
		return ss.str();
	}


	bool TableInfo::operator==(const TableInfo& other) const noexcept
	{
		return m_tableName == other.m_tableName
			&& m_schemaName == other.m_schemaName
			&& m_catalogName == other.m_catalogName
			&& m_tableType == other.m_tableType
			&& m_tableRemarks == other.m_tableRemarks;
	}


	bool TableInfo::operator!=(const TableInfo& other) const noexcept
	{
		return !(*this == other);
	}

}
