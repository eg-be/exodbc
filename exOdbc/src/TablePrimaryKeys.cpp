/*!
* \file TablePrimaryKeys.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 02.01.2015
* \brief Source file for the TablePrimaryKeys class.
*/

#include "stdafx.h"

// Own header
#include "TablePrimaryKeys.h"

// Same component headers
#include "Database.h"

// Other headers

// Debug
#include "DebugNew.h"

namespace exodbc
{
	// Construction
	// ------------
	TablePrimaryKeys::TablePrimaryKeys()
		: m_initialized(false)
	{

	}


	TablePrimaryKeys::TablePrimaryKeys(Database* pDb, const STableInfo& tableInfo)
		: m_initialized(false)
	{
		m_initialized = Initialize(pDb, tableInfo);
	}

	// Destruction
	// -----------


	// Implementation
	// --------------
	bool TablePrimaryKeys::Initialize(Database* pDb, const STableInfo& tableInfo)
	{
		exASSERT(pDb);

		m_initialized = false;
		TablePrimaryKeysVector tablePks;

		if (pDb->ReadTablePrimaryKeys(tableInfo, tablePks))
		{
			m_initialized = Parse(tablePks);
		}

		return m_initialized;
	}


	bool TablePrimaryKeys::Parse(const TablePrimaryKeysVector& tablePks)
	{
		// Very simple so far
		for (TablePrimaryKeysVector::const_iterator it = tablePks.begin(); it != tablePks.end(); it++)
		{

		}
		return true;
	}

} // namespace exodbc