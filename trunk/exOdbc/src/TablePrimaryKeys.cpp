/*!
* \file TablePrimaryKeys.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 02.01.2015
* \brief Source file for the TablePrimaryKeys class.
* \copyright wxWindows Library Licence, Version 3.1
*/

#include "stdafx.h"

// Own header
#include "TablePrimaryKeys.h"

// Same component headers
#include "Database.h"
#include "Exception.h"

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


	TablePrimaryKeys::TablePrimaryKeys(const Database& db, const STableInfo& tableInfo)
		: m_initialized(false)
	{
		Initialize(db, tableInfo);
		m_initialized = true;
	}

	// Destruction
	// -----------


	// Implementation
	// --------------
	void TablePrimaryKeys::Initialize(const Database& db, const STableInfo& tableInfo)
	{
		exASSERT(db.IsOpen());

		m_initialized = false;
		TablePrimaryKeysVector tablePks;

		tablePks = db.ReadTablePrimaryKeys(tableInfo);
		Parse(tablePks);
		m_initialized = true;
	}


	void TablePrimaryKeys::Parse(const TablePrimaryKeysVector& tablePks)
	{
		m_pksMap.clear();

		// Very simple so far
		// \todo: Ensure we have unique values for m_keySequence
		m_pksVector = tablePks;
		for (TablePrimaryKeysVector::const_iterator it = tablePks.begin(); it != tablePks.end(); ++it)
		{
			m_pksMap[it->GetSqlName()] = *it;
		}
	}


	void TablePrimaryKeys::Parse(const STableInfo& tableInfo, const ColumnBufferPtrMap& columnBuffers)
	{
		m_pksMap.clear();
		m_pksVector.clear();

		// We read the info from the column-buffers. Use only the query-name of the columnbuffers.
		SQLSMALLINT keySequence = 1;
		for (ColumnBufferPtrMap::const_iterator it = columnBuffers.begin(); it != columnBuffers.end(); ++it)
		{
			ColumnBuffer* pBuffer = it->second;
			if (pBuffer->IsColumnFlagSet(CF_PRIMARY_KEY))
			{
				std::wstring columnName = pBuffer->GetQueryName();
				// keep only the last part and assume its the column-name only
				size_t lastDot = columnName.find_last_of(L".");
				if (lastDot != std::wstring::npos)
				{
					columnName = columnName.substr(lastDot + 1);
				}
				STablePrimaryKeyInfo keyInfo;
				// use only the pure table-name and the query-name from the column-buffer
				keyInfo.m_tableName = tableInfo.GetSqlName(QNF_TABLE);
				keyInfo.m_columnName = columnName;
				keyInfo.m_keySequence = keySequence;

				m_pksVector.push_back(keyInfo);
				m_pksMap[keyInfo.GetSqlName()] = keyInfo;

				++keySequence;
			}
		}
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