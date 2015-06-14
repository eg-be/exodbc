/*!
* \file InfoObject.h
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2014
* \brief Header file for info objects.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#pragma once
#ifndef INFOOBJECT_H
#define INFOOBJECT_H

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
	* \enum		TableQueryNameHint
	* \brief	A helper to specify how to build the name of the table to be used in a SQL query.
	*/
	enum class TableQueryNameHint
	{
		ALL,			///< Use Catalog, Schema and TableName, resulting in 'CatalogName.SchemaName.TableName'
		TABLE_ONLY,		///< Use only TableName
		CATALOG_TABLE,	///< Use Catalog and TableName, resulting in 'CatalogName.TableName'
		SCHEMA_TABLE,	///< Use Schema and TableName, resulting in 'SchemaName.TableName'
		EXCEL			///< For Excel use only the TableName$ and wrap it inside [], so it becomes '[TableName$]'. \note: The '$' is not added automatically.
	};

	/*!
	* \class TableInfo
	*
	* \brief Information about a Table
	*/
	class EXODBCAPI TableInfo
	{
	public:
		TableInfo();

		std::wstring		m_tableName;		///< Name
		std::wstring		m_tableType;        ///< "TABLE" or "SYSTEM TABLE" etc.
		std::wstring		m_tableRemarks;		///< Remarks
		std::wstring		m_catalogName;		///< catalog
		std::wstring		m_schemaName;		///< schema
		bool				m_isCatalogNull;	///< True if m_catalogName is null.
		bool				m_isSchemaNull;		///< True if m_schemaName is null.

		bool				HasSchema() const { return !m_isSchemaNull && m_schemaName.length() > 0; };
		bool				HasCatalog() const { return !m_isCatalogNull && m_catalogName.length() > 0; };

		void				SetSqlNameHint(TableQueryNameHint hint) { m_queryNameHint = hint; };
		std::wstring		GetSqlName() const;
		std::wstring		GetPureTableName() const;

	private:
		TableQueryNameHint	m_queryNameHint;
	};

	/*!
	* \typedef TableInfosVector
	* \brief std::vector of TableInfo objects.
	*/
	typedef std::vector<TableInfo> TableInfosVector;
}

#endif // INFOOBJECT_H