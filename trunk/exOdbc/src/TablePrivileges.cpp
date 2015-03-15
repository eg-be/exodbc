/*!
* \file TablePrivileges.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 31.12.2014
* \brief Source file for the TablePrivileges class.
* \copyright wxWindows Library Licence, Version 3.1
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
	TablePrivileges::TablePrivileges()
		: m_initialized(false)
	{

	}


	TablePrivileges::TablePrivileges(const Database& db, const STableInfo& tableInfo)
		: m_initialized(false)
	{
		Initialize(db, tableInfo);
		m_initialized = true;
	}

	// Destruction
	// -----------
	

	// Implementation
	// --------------
	void TablePrivileges::Initialize(const Database& db, const STableInfo& tableInfo)
	{
		exASSERT(db.IsOpen());

		m_privileges = 0;
		m_initialized = false;
		TablePrivilegesVector tablePrivs;

		tablePrivs = db.ReadTablePrivileges(tableInfo);
		Parse(tablePrivs);
		m_initialized = true;
	}


	void TablePrivileges::Parse(const TablePrivilegesVector& tablePrivs)
	{
		// Very simple so far
		for (TablePrivilegesVector::const_iterator it = tablePrivs.begin(); it != tablePrivs.end(); it++)
		{
			if (boost::iequals(L"SELECT", it->m_privilege))
			{
				m_privileges |= TP_SELECT;
			}
			else if (boost::iequals(L"INSERT", it->m_privilege))
			{
				m_privileges |= TP_INSERT;
			}
			else if (boost::iequals(L"UPDATE", it->m_privilege))
			{
				m_privileges |= TP_UPDATE;
			}
			else if (boost::iequals(L"DELETE", it->m_privilege))
			{
				m_privileges |= TP_DELETE;
			}
		}
	}

} // namespace exodbc