/*!
* \file TablePrimaryKeys.cpp
* \author Elias Gerber <eg@elisium.ch>
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


	TablePrimaryKeys::TablePrimaryKeys(const Database* pDb, const STableInfo& tableInfo)
		: m_initialized(false)
	{
		Initialize(pDb, tableInfo);
		m_initialized = true;
	}

	// Destruction
	// -----------


	// Implementation
	// --------------
	void TablePrimaryKeys::Initialize(const Database* pDb, const STableInfo& tableInfo)
	{
		exASSERT(pDb->IsOpen());

		m_initialized = false;
		TablePrimaryKeysVector tablePks;

		tablePks = pDb->ReadTablePrimaryKeys(tableInfo);
		Parse(tablePks);
		m_initialized = true;
	}


	void TablePrimaryKeys::Parse(const TablePrimaryKeysVector& tablePks)
	{
		// Very simple so far
		// \todo: Ensure we have unique values for m_keySequence
		m_pksVector = tablePks;
	}


	bool TablePrimaryKeys::WeakCompare(std::wstring name1, std::wstring name2) const
	{
		size_t d1 = name1.find_last_of(L".");
		size_t d2 = name2.find_last_of(L".");
		if (d1 != wstring::npos)
		{
			name1 = name1.substr(d1 + 1);
		}
		if (d2 != wstring::npos)
		{
			name2 = name2.substr(d2 + 1);
		}
		return name1 == name2;
	}


	void TablePrimaryKeys::Parse(const STableInfo& tableInfo, const ColumnBufferPtrMap& columnBuffers)
	{
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
				keyInfo.m_tableName = tableInfo.GetPureTableName();
				keyInfo.m_columnName = columnName;
				keyInfo.m_keySequence = keySequence;

				m_pksVector.push_back(keyInfo);

				++keySequence;
			}
		}
	}


	bool TablePrimaryKeys::IsPrimaryKey(const std::wstring queryName) const
	{
		exASSERT(m_initialized);

		for (TablePrimaryKeysVector::const_iterator it = m_pksVector.begin(); it != m_pksVector.end(); ++it)
		{
			const wstring& keyName = it->GetSqlName();
			if (WeakCompare(queryName, keyName))
			{
				return true;
			}
		}
		return false;
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
				if (WeakCompare(pBuff->GetQueryName(), pkQueryName) && pBuff->IsBound())
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


	bool TablePrimaryKeys::AreAllPrimaryKeysInMap(const ColumnBufferPtrMap& columnBuffers) const
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
				if (WeakCompare(pBuff->GetQueryName(), pkQueryName) && pBuff->IsBound())
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


	void TablePrimaryKeys::SetPrimaryKeyFlags(const ColumnBufferPtrMap& columnBuffers) const
	{
		exASSERT(m_initialized);

		ColumnBufferPtrMap::const_iterator itBuffs;
		for (itBuffs = columnBuffers.begin(); itBuffs != columnBuffers.end(); ++itBuffs)
		{
			const wstring& queryName = itBuffs->second->GetQueryName();
			if (IsPrimaryKey(queryName))
			{
				itBuffs->second->SetPrimaryKey(true);
			}
		}
	}


} // namespace exodbc