/*!
* \file ObjectName.h
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2014
* \brief Header file for name objects.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#pragma once
#ifndef OBJECTNAME_H
#define OBJECTNAME_H

// Same component headers
#include "exOdbc.h"

// Other headers

// System headers
#include <string>

// Forward declarations
// --------------------

namespace exodbc
{
	///*!
	//* \enum		TableQueryNameHint
	//* \brief	A helper to specify how to build the name of the table to be used in a SQL query.
	//*/
	//enum class TableQueryNameHint
	//{
	//	ALL,			///< Use Catalog, Schema and TableName, resulting in 'CatalogName.SchemaName.TableName'
	//	TABLE_ONLY,		///< Use only TableName
	//	CATALOG_TABLE,	///< Use Catalog and TableName, resulting in 'CatalogName.TableName'
	//	SCHEMA_TABLE,	///< Use Schema and TableName, resulting in 'SchemaName.TableName'
	//	EXCEL			///< For Excel use only the TableName$ and wrap it inside [], so it becomes '[TableName$]'. \note: The '$' is not added automatically.
	//};


	/*!
	* \class ObjectName
	*
	* \brief Name of an Object.
	*/
	class ObjectName
	{
	public:
		virtual std::wstring GetQueryName() const = 0;
	};


	/*!
	* \class TableName
	*
	* \brief Name of a Table.
	*/
	class TableName
		: public ObjectName
	{
	public:
		TableName(const STableInfo& tableInfo, DatabaseProduct dbms = DatabaseProduct::UNKNOWN);

		virtual std::wstring GetQueryName() const;

	private:
		STableInfo m_tableInfo;
		DatabaseProduct m_dbms;
	};


	/*!
	* \class ColumnName
	*
	* \brief Name of a column.
	*/
	class ColumnName
		: public ObjectName
	{
	public:

		ColumnName(const std::wstring& queryName);
		ColumnName(const SColumnInfo& columnInfo);

		virtual std::wstring GetQueryName() const;

	private:
		bool m_haveColumnInfo;
		SColumnInfo m_columnInfo;
		std::wstring m_queryName;
	};
}

#endif // OBJECTNAME_H