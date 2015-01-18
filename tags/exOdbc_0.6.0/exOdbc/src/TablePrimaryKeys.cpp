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

using namespace std;

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
		m_pksMap.clear();

		// Very simple so far
		// \todo: Ensure we have unique values for m_keySequence
		m_pksVector = tablePks;
		for (TablePrimaryKeysVector::const_iterator it = tablePks.begin(); it != tablePks.end(); it++)
		{
			m_pksMap[it->GetSqlName()] = *it;
		}

		return true;
	}


	bool TablePrimaryKeys::IsPrimaryKey(const std::wstring queryName) const
	{
		exASSERT(m_initialized);

		PrimaryKeysMap::const_iterator it = m_pksMap.find(queryName);

		return it != m_pksMap.end();
	}


	bool TablePrimaryKeys::AreAllPrimaryKeysBound(const ColumnBufferPtrMap& columnBuffers) const
	{
		exASSERT(m_initialized);

		bool allBound = true;
		TablePrimaryKeysVector::const_iterator it;
		for (it = m_pksVector.begin(); it != m_pksVector.end(); it++)
		{
			wstring pkQueryName = it->GetSqlName();
			bool bound = false;
			ColumnBufferPtrMap::const_iterator itBuffs;
			for (itBuffs = columnBuffers.begin(); itBuffs != columnBuffers.end() && !bound; itBuffs++)
			{
				const ColumnBuffer* pBuff = itBuffs->second;
				if (pBuff->GetQueryName() == pkQueryName && pBuff->IsBound())
				{
					bound = true;
				}
			}
			if (!bound)
			{
				allBound = false;
				break;
			}
		}

		return allBound;
	}


	bool TablePrimaryKeys::SetPrimaryKeyFlag(const ColumnBufferPtrMap& columnBuffers) const
	{
		exASSERT(m_initialized);
		// \todo: Check that in Parse. Remember if we've setted all primary-keys, we just assume the query name is unique
		set<wstring> keys;
		for (TablePrimaryKeysVector::const_iterator it = m_pksVector.begin(); it != m_pksVector.end(); ++it)
		{
			keys.insert(it->GetSqlName());
		}

		ColumnBufferPtrMap::const_iterator itBuffs;
		for (itBuffs = columnBuffers.begin(); itBuffs != columnBuffers.end() && !keys.empty(); ++itBuffs)
		{
			const wstring& queryName = itBuffs->second->GetQueryName();
			if (IsPrimaryKey(queryName))
			{
				itBuffs->second->SetPrimaryKey(true);
				set<wstring>::iterator itK = keys.find(queryName);
				exASSERT(itK != keys.end());
				keys.erase(itK);
			}
		}

		return keys.empty();
	}


} // namespace exodbc