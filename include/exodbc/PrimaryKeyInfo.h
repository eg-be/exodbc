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
#include "SqlHandle.h"
#include "SqlInfoProperty.h"

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
		/*!
		* \brief Default constructor, all members are set to empty values, null flags are set to true.
		*/
		PrimaryKeyInfo();


		/*!
		* \brief Use passed values to init members. Null flags for catalog, schema and key name are set,
		*		non passed values are empty.
		*/
		PrimaryKeyInfo(const std::string& tableName, const std::string& columnName, SQLSMALLINT keySequence);


		/*!
		* \brief Use passed values to init members.
		*/
		PrimaryKeyInfo(const std::string& catalogName, const std::string& schemaName, const std::string& tableName, const std::string& columnName,
			SQLSMALLINT keySequence, const std::string& keyName, bool isCatalogNull, bool isSchemaNull, bool isPrimaryKeyNameNull);


		/*!
		* \brief Create from a statement that is assumed to hold the results of SQLPrimaryKeys. The cursor must
		*		be positioned at the row and is not modified, but column values are read.
		* \throw Exception If reading any value fails, or if props does not hold all required properties.
		*/
		PrimaryKeyInfo(ConstSqlStmtHandlePtr pStmt, const SqlInfoProperties& props);


		/*!
		* \brief Return the non-empty name to be used in queries like SELECT, UPDATE, etc.
		* \throw AssertionException If no non-empty query name can be returned.
		*/
		std::string GetQueryName() const;


		/*!
		* \return Key column name. Empty value might be returned.
		*/
		std::string GetColumnName() const;


		/*!
		* \return Catalog name. Empty value might be returned.
		* \see HasCatalog()
		*/
		std::string GetCatalog() const noexcept { return m_catalogName; };


		/*!
		* \return Schema name. Empty value might be returned.
		* \see HasSchema()
		*/
		std::string GetSchema() const noexcept { return m_schemaName; };


		/*!
		* \return Table name. Empty value might be returned.
		*/
		std::string GetTable() const noexcept { return m_tableName; };


		/*!
		* \return Key Sequence Number, starting with 1 (0 if not set).
		*/
		SQLSMALLINT GetKeySequence() const noexcept { return m_keySequence; };


		/*!
		* \return Key Name. Empty value might be returned.
		*/
		std::string GetKeyName() const { exASSERT(!IsKeyNameNull()); return m_keyName; };


		/*!
		* \return True if null flag for Schema is not set and Schema Name is not empty.
		*/
		bool			HasSchema() const noexcept { return !m_isSchemaNull && !m_schemaName.empty(); };


		/*!
		* \return True if null flag for Catalog is not set and Catalog Name is not empty.
		*/
		bool			HasCatalog() const noexcept { return !m_isCatalogNull && !m_catalogName.empty(); };


		/*!
		* \return True if null flag for Schema is set.
		*/
		bool IsCatalogNull() const noexcept  { return m_isCatalogNull; };


		/*!
		* \brief True if null flag for Catalog is set.
		*/
		bool IsSchemaNull() const { return m_isSchemaNull; };


		/*!
		* \brief True if null flag for Key Name is set.
		*/
		bool IsKeyNameNull() const { return m_isKeyNameNull; };

	private:
		std::string	m_catalogName;	///< TABLE_CAT [Nullable]. Primary key table catalog name.
		std::string	m_schemaName;	///< TABLE_SCHEM [Nullable]. Primary key table schema name.
		std::string	m_tableName;	///< TABLE_NAME. Primary key table name.
		std::string	m_columnName;	///< COLUMN_NAME. Primary key column name.

		SQLSMALLINT		m_keySequence;	///< KEY_SEQ. Column sequence number in key (starting with 1).
		std::string		m_keyName;	///< PK_NAME [Nullable]. Column sequence number in key (starting with 1).

		bool			m_isCatalogNull;		///< True if TABLE_CAT is Null.
		bool			m_isSchemaNull;			///< True if TABLE_SCHEM is Null.
		bool			m_isKeyNameNull;	///< True if PK_NAME is Null.
	};

	/*!
	* \typedef PrimaryKeyInfoVector
	* \brief std::vector of PrimaryKeyInfo objects.
	*/
	typedef std::vector<PrimaryKeyInfo> PrimaryKeyInfoVector;
}
