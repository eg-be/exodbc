/*!
* \file TableInfo.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 30.04.2017
* \brief Source file for TableInfo.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "TableInfo.h"

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
	// Class TableInfo
	// ===============

	TableInfo::TableInfo()
		: m_isCatalogNull(true)
		, m_isSchemaNull(true)
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
	{ }


	TableInfo::TableInfo(const std::string& tableName, const std::string& tableType, const std::string& tableRemarks, const std::string& catalogName, const std::string schemaName, bool isCatalogNull, bool isSchemaNull, DatabaseProduct dbms /* = DatabaseProduct::UNKNOWN */)
		: m_tableName(tableName)
		, m_tableType(tableType)
		, m_tableRemarks(tableRemarks)
		, m_catalogName(catalogName)
		, m_schemaName(schemaName)
		, m_dbms(dbms)
		, m_isCatalogNull(isCatalogNull)
		, m_isSchemaNull(isSchemaNull)
	{ }


	TableInfo::TableInfo(ConstSqlStmtHandlePtr pStmt, const SqlInfoProperties& props)
	{
		exASSERT(pStmt);
		exASSERT(pStmt->IsAllocated());

		m_dbms = props.DetectDbms();

		SQLLEN cb = 0;
		GetData(pStmt, 1, props.GetMaxCatalogNameLen(), m_catalogName, &m_isCatalogNull);
		GetData(pStmt, 2, props.GetMaxSchemaNameLen(), m_schemaName, &m_isSchemaNull);
		GetData(pStmt, 3, props.GetMaxTableNameLen(), m_tableName);
		GetData(pStmt, 4, DB_MAX_TABLE_TYPE_LEN, m_tableType);
		GetData(pStmt, 5, DB_MAX_TABLE_REMARKS_LEN, m_tableRemarks);
	}


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


	std::string TableInfo::ToString() const noexcept
	{
		stringstream ss;
		ss << u8"Name: '" << m_tableName << u8"'; ";
		ss << u8"Schema: '";
		if (m_isSchemaNull)
			ss << u8"NULL; ";
		else
			ss << m_schemaName << u8"'; ";
		ss << u8"Catalog: '";
		if (m_isCatalogNull)
			ss << u8"NULL; ";
		else
			ss << m_catalogName << u8"; ";
		ss << u8"Type: '" << m_tableType << u8"'";
		return ss.str();
	}


	bool TableInfo::operator==(const TableInfo& other) const noexcept
	{
		return m_tableName == other.m_tableName
			&& m_schemaName == other.m_schemaName
			&& m_catalogName == other.m_catalogName
			&& m_tableType == other.m_tableType
			&& m_tableRemarks == other.m_tableRemarks
			&& m_isSchemaNull == other.m_isSchemaNull
			&& m_isCatalogNull == other.m_isCatalogNull;
	}


	bool TableInfo::operator!=(const TableInfo& other) const noexcept
	{
		return !(*this == other);
	}

}
