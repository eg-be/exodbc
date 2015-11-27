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
} // namespace exodbc