/*!
* \file ObjectName.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2015
* \brief Source file for name objects.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#include "stdafx.h"

// Own header
#include "ObjectName.h"

// Same component headers

// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{


	// Interfaces
	// ----------
	TableName::TableName(const STableInfo& tableInfo, DatabaseProduct dbms /* = DatabaseProduct::UNKNOWN */)
		: m_dbms(dbms)
		, m_tableInfo(tableInfo)
	{ }


	std::wstring TableName::GetQueryName() const
	{
		switch (m_dbms)
		{
		case DatabaseProduct::ACCESS:
			// For access, return only the pure table name.
			return m_tableInfo.m_tableName;
			break;
		case DatabaseProduct::EXCEL:
			// For excel, add '[' and ']' around the pure table name.
			return L"[" + m_tableInfo.m_tableName + L"]";
			break;
		}

		// As default, include everything we have
		std::wstringstream ws;
		if (m_tableInfo.HasCatalog())
		{
			ws << m_tableInfo.m_catalogName << L".";
		}
		if (m_tableInfo.HasSchema())
		{
			ws << m_tableInfo.m_schemaName << L".";
		}
		ws << m_tableInfo.m_tableName;

		std::wstring str = ws.str();
		return str;
	}


	ColumnName::ColumnName(const SColumnInfo& columnInfo)
		: m_columnInfo(columnInfo)
		, m_haveColumnInfo(true)
	{ }


	ColumnName::ColumnName(const std::wstring& queryName)
		: m_haveColumnInfo(false)
		, m_queryName(queryName)
	{ }


	std::wstring ColumnName::GetQueryName() const
	{
		if (m_haveColumnInfo)
		{
			return m_columnInfo.m_columnName;
		}
		else
		{
			return m_queryName;
		}
	}
}
