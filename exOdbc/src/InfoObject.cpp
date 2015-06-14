/*!
* \file InfoObject.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2015
* \brief Source file for info objects.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#include "stdafx.h"

// Own header
#include "InfoObject.h"

// Same component headers

// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	TableInfo::TableInfo()
		: m_isCatalogNull(false)
		, m_isSchemaNull(false)
		, m_queryNameHint(TableQueryNameHint::ALL)
	{

	}


	std::wstring TableInfo::GetPureTableName() const
	{
		exASSERT(!m_tableName.empty());
		return m_tableName;
	}


	std::wstring TableInfo::GetSqlName() const
	{
		exASSERT(!m_tableName.empty());

		bool includeCat = true;
		bool includeSchem = true;
		bool includeName = true;
		switch (m_queryNameHint)
		{
		case TableQueryNameHint::ALL:
			includeCat = true;
			includeSchem = true;
			includeName = true;
			break;
		case TableQueryNameHint::CATALOG_TABLE:
			includeCat = true;
			includeSchem = false;
			includeName = true;
			break;
		case TableQueryNameHint::SCHEMA_TABLE:
			includeCat = false;
			includeSchem = true;
			includeName = true;
			break;
		case TableQueryNameHint::TABLE_ONLY:
			includeCat = false;
			includeSchem = false;
			includeName = true;
			break;
		case TableQueryNameHint::EXCEL:
			return L"[" + m_tableName + L"]";
		}

		std::wstringstream ws;
		if (includeCat && HasCatalog())
		{
			ws << m_catalogName << L".";
		}
		if (includeSchem && HasSchema())
		{
			ws << m_schemaName << L".";
		}
		if (includeName)
		{
			ws << m_tableName << L".";
		}

		std::wstring str = ws.str();
		boost::erase_last(str, L".");
		return str;
	}

	// Interfaces
	// ----------
}
