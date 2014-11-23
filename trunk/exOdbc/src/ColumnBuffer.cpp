/*!
* \file ColumnBuffer.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 23.11.2014
* \brief Source file for the ColumnBuffer class and its helpers.
*
*/

#include "stdafx.h"

// Own header
#include "ColumnBuffer.h"

// Same component headers
// Other headers

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	ColumnBuffer::ColumnBuffer(const SColumnInfo& columnInfo)
		: m_columnInfo(columnInfo)
		, m_allocatedBuffer(false)
	{
		m_allocatedBuffer = AllocateBuffer(m_columnInfo);
		//m_pBuffer = new boost::any();
		//m_createdBuffer = true;
	}

	ColumnBuffer::ColumnBuffer(const SColumnInfo& columnInfo, boost::any* pBuffer)
		: m_columnInfo(columnInfo)
		, m_allocatedBuffer(false)
	{
		//m_pBuffer = pBuffer;
		//m_createdBuffer = false;
	}

	ColumnBuffer::ColumnBuffer(const ColumnBuffer& other)
		: m_columnInfo(other.m_columnInfo)
		, m_allocatedBuffer(false)
	{
		m_allocatedBuffer = AllocateBuffer(m_columnInfo);
	}

	//ColumnBuffer::ColumnBuffer(const STableColumnInfo& columnInfo, void* pBuffer)
	//{
	//	m_createdBuffer = false;
	//}

	// Destructor
	// -----------
	ColumnBuffer::~ColumnBuffer()
	{
		if (m_allocatedBuffer)
		{
			LOG_ERROR(L"Deleting buffer");
			try
			{
				SQLSMALLINT* pInt = NULL;
				switch (m_columnInfo.m_sqlDataType)
				{
				case SQL_SMALLINT:
					pInt = boost::get<SQLSMALLINT*>(m_buffer);
					break;
					//case SQL_INTEGER:
					//	delete (boost::get<SQLINTEGER*>(m_buffer));
					//	break;
					//case SQL_BIGINT:
					//	delete (boost::get<SQLBIGINT*>(m_buffer));
					//	break;
				}
			}
			catch (boost::bad_get exception)
			{
				int p = 3;
			}
			m_allocatedBuffer = false;
		}
	}

	// Implementation
	// --------------
	bool ColumnBuffer::AllocateBuffer(const SColumnInfo& columnInfo)
	{
		LOG_ERROR(L"Alloc");
		wstring s = SqlType2s(columnInfo.m_sqlDataType);
		//		boost::variant<boost::variant<SQLUSMALLINT, SQLSMALLINT>, boost::variant<>> numeric;

		bool allocated = false;
		SQLSMALLINT* pInt = 0;
		SQLSMALLINT a = 0;
		switch (columnInfo.m_sqlDataType)
		{
		case SQL_SMALLINT:
			m_intVar = SQLSMALLINT(0);
			break;
		case SQL_INTEGER:
			m_intVar = SQLINTEGER(0);
			break;
		case SQL_BIGINT:
			m_intVar = SQLBIGINT(0);
			break;
		default:
			LOG_ERROR((boost::wformat(L"Not implemented SqlDataType '%s' (%d)") % SqlType2s(columnInfo.m_sqlDataType) % columnInfo.m_sqlDataType).str());
			allocated = false;
		}
		if (m_columnInfo.m_sqlDataType == SQL_SMALLINT)
		{
			SQLSMALLINT* bc = &(boost::get<SQLSMALLINT>(m_intVar));
			*bc = 35;
			int p = 3;
		}
		if (m_columnInfo.m_sqlDataType == SQL_SMALLINT)
		{
			SQLSMALLINT zz = boost::get<SQLSMALLINT>(m_intVar);
			int o = 3;
		}
		return allocated;
	}

	// Interfaces
	// ----------

}
