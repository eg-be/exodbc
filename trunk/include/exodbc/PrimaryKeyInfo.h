/*!
* \file PrimaryKeyInfo.h
* \author Elias Gerber <eg@elisium.ch>
* \date 01.05.2017
* \brief Header file for PrimaryKeyInfo.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "AssertionException.h"

// Other headers
// System headers
#include <string>
#include <vector>

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class	PrimaryKeyInfo
	* \brief	Primary Keys of a table as fetched using SQLPrimaryKeys
	*/
	class EXODBCAPI PrimaryKeyInfo
	{
	public:
		PrimaryKeyInfo();
		PrimaryKeyInfo(const std::string& tableName, const std::string& columnName, SQLSMALLINT keySequence);
		PrimaryKeyInfo(const std::string& catalogName, const std::string& schemaName, const std::string& tableName, const std::string& columnName,
			SQLSMALLINT keySequence, const std::string& keyName, bool isCatalogNull, bool isSchemaNull, bool isPrimaryKeyNameNull);

		std::string GetQueryName() const;
		std::string GetPureName() const;

		std::string GetCatalogName() const { exASSERT(!IsCatalogNull()); return m_catalogName; };
		std::string GetSchemaName() const { exASSERT(!IsSchemaNull()); return m_schemaName; };
		std::string GetTableName() const { return m_tableName; };
		std::string GetColumnName() const { return m_columnName; };

		SQLSMALLINT GetKeySequence() const { return m_keySequence; };

		std::string GetKeyName() const { exASSERT(!IsKeyNameNull()); return m_primaryKeyName; };

		bool IsCatalogNull() const { return m_isCatalogNull; };
		bool IsSchemaNull() const { return m_isSchemaNull; };
		bool IsKeyNameNull() const { return m_isPrimaryKeyNameNull; };

	private:
		std::string	m_catalogName;	///< TABLE_CAT [Nullable]. Primary key table catalog name.
		std::string	m_schemaName;	///< TABLE_SCHEM [Nullable]. Primary key table schema name.
		std::string	m_tableName;	///< TABLE_NAME. Primary key table name.
		std::string	m_columnName;	///< COLUMN_NAME. Primary key column name.

		SQLSMALLINT		m_keySequence;	///< KEY_SEQ. Column sequence number in key (starting with 1).
		std::string	m_primaryKeyName;	///< PK_NAME [Nullable]. Column sequence number in key (starting with 1).

		bool			m_isCatalogNull;		///< True if TABLE_CAT is Null.
		bool			m_isSchemaNull;			///< True if TABLE_SCHEM is Null.
		bool			m_isPrimaryKeyNameNull;	///< True if PK_NAME is Null.
	};

	/*!
	* \typedef PrimaryKeyInfoVector
	* \brief std::vector of PrimaryKeyInfo objects.
	*/
	typedef std::vector<PrimaryKeyInfo> PrimaryKeyInfoVector;
}
