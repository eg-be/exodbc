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
	*
	* \brief Information about a Table
	*
	* \details Holds information about a table found using the SQLTables catalog function.
	*/
	class EXODBCAPI TableInfo
	{	
	public:
		TableInfo();
		TableInfo(const std::string& tableName, const std::string& tableType, const std::string& tableRemarks, 
			const std::string& catalogName, const std::string schemaName, DatabaseProduct dbms = DatabaseProduct::UNKNOWN);
		TableInfo(const std::string& tableName, const std::string& tableType, const std::string& tableRemarks, 
			const std::string& catalogName, const std::string schemaName, bool isCatalogNull, bool isSchemaNull, DatabaseProduct dbms = DatabaseProduct::UNKNOWN);

		std::string GetQueryName() const;
		std::string GetPureName() const;

		std::string		GetType() const { return m_tableType; };
		std::string		GetCatalog() const { return m_catalogName;};
		std::string		GetSchema() const {	return m_schemaName; };

		bool				HasSchema() const { return !m_isSchemaNull && m_schemaName.length() > 0; };
		bool				HasCatalog() const { return !m_isCatalogNull && m_catalogName.length() > 0; };

		bool operator==(const TableInfo& other) const noexcept;
		bool operator!=(const TableInfo& other) const noexcept;

		std::string ToString() const noexcept;

	private:
		DatabaseProduct		m_dbms;

		std::string		m_tableName;		///< Table name.
		std::string		m_tableType;        ///< "TABLE" or "SYSTEM TABLE" etc.
		std::string		m_tableRemarks;		///< Remarks
		std::string		m_catalogName;		///< Catalog name
		std::string		m_schemaName;		///< Schema name.
		bool				m_isCatalogNull;	///< True if NULL was returned for catalog name.
		bool				m_isSchemaNull;		///< True if NULL was returned for schema name.
	};

	/*!
	* \typedef TableInfoVector
	* \brief std::vector of TableInfo objects.
	*/
	typedef std::vector<TableInfo> TableInfoVector;

}
