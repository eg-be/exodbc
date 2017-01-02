/*!
* \file TablePrivileges.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 31.12.2014
* \brief Source file for the TablePrivileges class.
* \copyright GNU Lesser General Public License Version 3
*/

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
	std::string TablePrivileges::ToString(TablePrivilege priv)
	{
		switch (priv)
		{
		case TablePrivilege::SELECT:
			return u8"SELECT";
		case TablePrivilege::INSERT:
			return u8"INSERT";
		case TablePrivilege::UPDATE:
			return u8"UPDATE";
		case TablePrivilege::DEL:
			return u8"DELETE";
		case TablePrivilege::NONE:
			return u8"NONE";
		default:
			return u8"???";
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
			if (boost::iequals(u8"SELECT", it->m_privilege))
			{
				Set(TablePrivilege::SELECT);
			}
			else if (boost::iequals(u8"INSERT", it->m_privilege))
			{
				Set(TablePrivilege::INSERT);
			}
			else if (boost::iequals(u8"UPDATE", it->m_privilege))
			{
				Set(TablePrivilege::UPDATE);
			}
			else if (boost::iequals(u8"DELETE", it->m_privilege))
			{
				Set(TablePrivilege::DEL);
			}
		}
	}


	std::string MissingTablePrivilegeException::ToString() const noexcept
	{
		std::stringstream ss;
		ss << Exception::ToString();
		ss << u8"Missing Privilege '" << TablePrivileges::ToString(m_missingPriv) << u8"' on Table '" << m_tableInfo.GetQueryName() << u8"'";
		return ss.str();
	}
} // namespace exodbc