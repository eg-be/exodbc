/*!
* \file TablePrivileges.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 31.12.2014
* \brief Source file for the TablePrivileges class.
* \copyright GNU Lesser General Public License Version 3
*/

#include "stdafx.h"

// Own header
#include "TablePrivileges.h"

// Same component headers
#include "Database.h"
#include "Exception.h"

// Other headers
// Debug
#include "DebugNew.h"

namespace exodbc
{
	// Construction
	// ------------

	// Destruction
	// -----------

	// Implementation
	// --------------
	std::wstring TablePrivileges::ToString(TablePrivilege priv)
	{
		switch (priv)
		{
		case TablePrivilege::SELECT:
			return L"SELECT";
		case TablePrivilege::INSERT:
			return L"INSERT";
		case TablePrivilege::UPDATE:
			return L"UPDATE";
		case TablePrivilege::DEL:
			return L"DELETE";
		case TablePrivilege::NONE:
			return L"NONE";
		default:
			return L"???";
		}
	}

	void TablePrivileges::Init(ConstDatabasePtr pDb, const TableInfo& tableInfo)
	{
		exASSERT(pDb);
		exASSERT(pDb->IsOpen());
		TablePrivilegesVector tablePrivs = pDb->ReadTablePrivileges(tableInfo);
		Init(tablePrivs);
	}


	void TablePrivileges::Init(const TablePrivilegesVector& tablePrivs)
	{
		for (TablePrivilegesVector::const_iterator it = tablePrivs.begin(); it != tablePrivs.end(); it++)
		{
			if (boost::iequals(L"SELECT", it->m_privilege))
			{
				Set(TablePrivilege::SELECT);
			}
			else if (boost::iequals(L"INSERT", it->m_privilege))
			{
				Set(TablePrivilege::INSERT);
			}
			else if (boost::iequals(L"UPDATE", it->m_privilege))
			{
				Set(TablePrivilege::UPDATE);
			}
			else if (boost::iequals(L"DELETE", it->m_privilege))
			{
				Set(TablePrivilege::DEL);
			}
		}
	}


	std::wstring MissingTablePrivilegeException::ToString() const noexcept
	{
		std::wstringstream ws;
		ws << Exception::ToString();
		ws << L"Missing Privilege '" << TablePrivileges::ToString(m_missingPriv) << L"' on Table '" << m_tableInfo.GetQueryName() << L"'";
		return ws.str();
	}
} // namespace exodbc