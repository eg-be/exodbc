/*!
* \file TableInfo.h
* \author Elias Gerber <eg@elisium.ch>
* \date 30.04.2017
* \brief Header file for TableInfo.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "exOdbc.h"
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
	* \class TableInfo
	* \brief Information about a Table found using the SQLTables catalog function.
	* \see DatabaseCatalog::SearchTables()
	* \see DatabaseCatalog::FindOneTable()
	*/
	class EXODBCAPI TableInfo
	{	
	public:
		/*!
		* \brief Default constructor, all members are set to empty values, null flags are set to true.
		*/
		TableInfo();

		/*!
		* \brief Use passed values to init members. If schemaName or catalogName are empty, the corresponding
		*		null flags are set to true.
		*/
		TableInfo(const std::string& tableName, const std::string& tableType, const std::string& tableRemarks, 
			const std::string& catalogName, const std::string schemaName, DatabaseProduct dbms = DatabaseProduct::UNKNOWN);

		/*!
		* \brief Use passed values to init members.
		*/
		TableInfo(const std::string& tableName, const std::string& tableType, const std::string& tableRemarks, 
			const std::string& catalogName, const std::string schemaName, bool isCatalogNull, bool isSchemaNull, DatabaseProduct dbms = DatabaseProduct::UNKNOWN);

		/*!
		* \brief Create from a statement that is assumed to hold the results of SQLTables. The cursor must
		*		be positioned at the row and is not modified, but column values are read.
		* \throw Exception If reading any value fails, or if props does not hold all required properties.
		*/
		TableInfo(ConstSqlStmtHandlePtr pStmt, const SqlInfoProperties& props);


		/*!
		* \brief Return the non-empty name to be used in queries like SELECT, UPDATE, etc.
		* \throw AssertionException If no non-empty query name can be returned.
		*/
		std::string GetQueryName() const;

		/*!
		* \return Table name. Empty value might be returned.
		*/
		std::string GetName() const noexcept { return m_tableName; };

		/*!
		* \return Table type. Empty value might be returned.
		*/
		std::string		GetType() const noexcept { return m_tableType; };

		/*!
		* \return Catalog name. Empty value might be returned.
		* \see HasCatalog()
		*/
		std::string		GetCatalog() const noexcept { return m_catalogName;};

		/*!
		* \return Schema name. Empty value might be returned.
		* \see HasSchema()
		*/
		std::string		GetSchema() const noexcept {	return m_schemaName; };

		/*!
		* \return True if null flag for Schema is not set and Schema Name is not empty.
		*/
		bool			HasSchema() const noexcept  { return !m_isSchemaNull && !m_schemaName.empty(); };

		/*!
		* \return True if null flag for Catalog is not set and Catalog Name is not empty.
		*/
		bool			HasCatalog() const noexcept  { return !m_isCatalogNull && !m_catalogName.empty(); };

		/*!
		* \return True if null flag for Schema is set.
		*/
		bool			IsSchemaNull() const noexcept { return m_isSchemaNull; };

		/*!
		* \brief True if null flag for Catalog is set.
		*/
		bool			IsCatalogNull() const noexcept { return m_isCatalogNull; };

		/*!
		* \brief Compares all members for equality
		*/
		bool operator==(const TableInfo& other) const noexcept;
		bool operator!=(const TableInfo& other) const noexcept;

		/*!
		* \return One-line string with all properties of this TableInfo.
		*/
		std::string ToString() const noexcept;

	private:
		DatabaseProduct		m_dbms;

		std::string		m_tableName;		///< Table name.
		std::string		m_tableType;        ///< "TABLE" or "SYSTEM TABLE" etc.
		std::string		m_tableRemarks;		///< Remarks
		std::string		m_catalogName;		///< Catalog name
		std::string		m_schemaName;		///< Schema name.
		bool			m_isCatalogNull;	///< True if NULL was returned for catalog name.
		bool			m_isSchemaNull;		///< True if NULL was returned for schema name.
	};

	/*!
	* \typedef TableInfoVector
	* \brief std::vector of TableInfo objects.
	*/
	typedef std::vector<TableInfo> TableInfoVector;

}
