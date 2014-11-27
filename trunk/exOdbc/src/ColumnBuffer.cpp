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
	ColumnBuffer::ColumnBuffer(const SColumnInfo& columnInfo, Table::CharBindingMode mode)
		: m_columnInfo(columnInfo)
		, m_isBound(false)
		, m_allocatedBuffer(false)
		, m_charBindingMode(mode)
	{
		m_allocatedBuffer = AllocateBuffer(m_columnInfo);
	}

	//ColumnBuffer::ColumnBuffer(const SColumnInfo& columnInfo, boost::any* pBuffer)
	//	: m_columnInfo(columnInfo)
	//	, m_allocatedBuffer(false)
	//{
	//	//m_pBuffer = pBuffer;
	//	//m_createdBuffer = false;
	//}


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
			try
			{
				switch (m_columnInfo.m_sqlDataType)
				{
				case SQL_SMALLINT:
					delete GetSmallIntPtr();
					break;
				case SQL_INTEGER:
					delete GetIntPtr();
					break;
				case SQL_BIGINT:
					delete GetBigIntPtr();
					break;
				case SQL_CHAR:
				case SQL_VARCHAR:
					if (m_charBindingMode == Table::BIND_AS_CHAR || Table::BIND_AS_REPORTED)
					{
						delete[] GetCharPtr();
					}
					else
					{
						delete[] GetWCharPtr();
					}
					break;
				case SQL_WCHAR:
				case SQL_WVARCHAR:
					if (m_charBindingMode == Table::BIND_AS_WCHAR || Table::BIND_AS_REPORTED)
					{
						delete[] GetWCharPtr();
					}
					else
					{
						delete[] GetCharPtr();
					}
					break;
				}
			}
			catch (boost::bad_get ex)
			{
				LOG_ERROR(ex.what());
			}
		}

	}

	// Implementation
	// --------------
	bool ColumnBuffer::AllocateBuffer(const SColumnInfo& columnInfo)
	{
		exASSERT(!m_allocatedBuffer);
		exASSERT(!m_isBound);

		bool failed = false;
		switch (columnInfo.m_sqlDataType)
		{
		case SQL_SMALLINT:
			m_intPtrVar = new SQLSMALLINT(0);
			break;
		case SQL_INTEGER:
			m_intPtrVar = new SQLINTEGER(0);
			break;
		case SQL_BIGINT:
			m_intPtrVar = new SQLBIGINT(0);
			break;
		case SQL_CHAR:
		case SQL_VARCHAR:
			if (m_charBindingMode == Table::BIND_AS_CHAR || Table::BIND_AS_REPORTED)
			{
				m_intPtrVar = new SQLCHAR[GetBufferSize() + 1];
			}
			else
			{
				m_intPtrVar = new SQLWCHAR[GetBufferSize() + 1];
			}
			break;
		case SQL_WCHAR:
		case SQL_WVARCHAR:
			if (m_charBindingMode == Table::BIND_AS_WCHAR || Table::BIND_AS_REPORTED)
			{
				m_intPtrVar = new SQLWCHAR[GetBufferSize() + 1];
			}
			else
			{
				m_intPtrVar = new SQLCHAR[GetBufferSize() + 1];
			}
			break;
		default:
			LOG_ERROR((boost::wformat(L"Not implemented SqlDataType '%s' (%d)") % SqlType2s(columnInfo.m_sqlDataType) % columnInfo.m_sqlDataType).str());
			failed = true;
		}

		if (!failed)
		{
			m_allocatedBuffer = true;
		}
		return m_allocatedBuffer;
	}

	void* ColumnBuffer::GetBuffer()
	{
		exASSERT(m_allocatedBuffer);

		void* pBuffer = NULL;
		switch (m_columnInfo.m_sqlDataType)
		{
		case SQL_SMALLINT:
			return static_cast<void*>(boost::get<SQLSMALLINT*>(m_intPtrVar));
		case SQL_INTEGER:
			return static_cast<void*>(boost::get<SQLINTEGER*>(m_intPtrVar));
		case SQL_BIGINT:
			return static_cast<void*>(boost::get<SQLBIGINT*>(m_intPtrVar));
		case SQL_CHAR:
		case SQL_VARCHAR:
			if (m_charBindingMode == Table::BIND_AS_CHAR || Table::BIND_AS_REPORTED)
			{
				return static_cast<void*>(boost::get<SQLCHAR*>(m_intPtrVar));
			}
			else
			{
				return static_cast<void*>(boost::get<SQLWCHAR*>(m_intPtrVar));
			}
		case SQL_WCHAR:
		case SQL_WVARCHAR:
			if (m_charBindingMode == Table::BIND_AS_WCHAR || Table::BIND_AS_REPORTED)
			{
				return static_cast<void*>(boost::get<SQLWCHAR*>(m_intPtrVar));
			}
			else
			{
				return static_cast<void*>(boost::get<SQLCHAR*>(m_intPtrVar));
			}
		default:
			LOG_ERROR((boost::wformat(L"Not implemented SqlDataType '%s' (%d)") % SqlType2s(m_columnInfo.m_sqlDataType) % m_columnInfo.m_sqlDataType).str());
		}
		return pBuffer;
	}


	size_t ColumnBuffer::GetBufferSize() const
	{
		switch (m_columnInfo.m_sqlDataType)
		{
		case SQL_SMALLINT:
			return sizeof(SQLSMALLINT);
		case SQL_INTEGER:
			return sizeof(SQLINTEGER);
		case SQL_BIGINT:
			return sizeof(SQLBIGINT);
		case SQL_CHAR:
		case SQL_VARCHAR:
			if (m_charBindingMode == Table::BIND_AS_CHAR || Table::BIND_AS_REPORTED)
			{
				return (m_columnInfo.m_columnSize + 1) * sizeof(SQLCHAR);
			}
			else
			{
				return (m_columnInfo.m_columnSize + 1) * sizeof(SQLWCHAR);
			}
		case SQL_WCHAR:
		case SQL_WVARCHAR:
			if (m_charBindingMode == Table::BIND_AS_WCHAR || Table::BIND_AS_REPORTED)
			{
				return (m_columnInfo.m_columnSize + 1) * sizeof(SQLWCHAR);
			}
			else
			{
				return (m_columnInfo.m_columnSize + 1) * sizeof(SQLCHAR);
			}
		default:
			LOG_ERROR((boost::wformat(L"Not implemented SqlDataType '%s' (%d)") % SqlType2s(m_columnInfo.m_sqlDataType) % m_columnInfo.m_sqlDataType).str());
		}
		return 0;
	}


	ColumnBuffer::operator SQLSMALLINT() const
	{
		exASSERT(m_allocatedBuffer);
		exASSERT(m_isBound);

		// We use the BigIntVisitor here. It will always succeed to convert if 
		// the underlying Value is an int-value or throw otherwise
		SQLBIGINT bigVal = boost::apply_visitor(BigintVisitor(m_columnInfo.m_sqlDataType), m_intPtrVar);
		// But we are only allowed to Downcast this to a Smallint if the original value was a smallint
		if (!(m_columnInfo.m_sqlDataType == SQL_SMALLINT))
		{
			throw CastException(m_columnInfo.m_sqlDataType, SQL_C_SSHORT);
		}
		return (SQLSMALLINT)bigVal;
	}


	ColumnBuffer::operator SQLINTEGER() const
	{
		exASSERT(m_allocatedBuffer);
		exASSERT(m_isBound);

		// We use the BigIntVisitor here. It will always succeed to convert if 
		// the underlying Value is an int-value or throw otherwise
		SQLBIGINT bigVal = boost::apply_visitor(BigintVisitor(m_columnInfo.m_sqlDataType), m_intPtrVar);
		// But we are only allowed to downcast this to an Int if we are not loosing information
		if (!(m_columnInfo.m_sqlDataType == SQL_SMALLINT || m_columnInfo.m_sqlDataType == SQL_INTEGER))
		{
			throw CastException(m_columnInfo.m_sqlDataType, SQL_C_SLONG);
		}
		return (SQLINTEGER)bigVal;
	}


	ColumnBuffer::operator SQLBIGINT() const
	{
		exASSERT(m_allocatedBuffer);
		exASSERT(m_isBound);

		return boost::apply_visitor(BigintVisitor(m_columnInfo.m_sqlDataType), m_intPtrVar);
	}


	ColumnBuffer::operator std::wstring() const
	{
		exASSERT(m_allocatedBuffer);
		exASSERT(m_isBound);

		return boost::apply_visitor(WStringVisitor(m_columnInfo.m_sqlDataType), m_intPtrVar);
	}


	ColumnBuffer::operator std::string() const
	{
		exASSERT(m_allocatedBuffer);
		exASSERT(m_isBound);

		return boost::apply_visitor(StringVisitor(m_columnInfo.m_sqlDataType), m_intPtrVar);
	}


	SQLSMALLINT* ColumnBuffer::GetSmallIntPtr() const
	{
		exASSERT(m_allocatedBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLSMALLINT*>(m_intPtrVar);
	}

	SQLINTEGER* ColumnBuffer::GetIntPtr() const
	{
		exASSERT(m_allocatedBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLINTEGER*>(m_intPtrVar);
	}

	SQLBIGINT* ColumnBuffer::GetBigIntPtr() const
	{
		exASSERT(m_allocatedBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLBIGINT*>(m_intPtrVar);
	}

	SQLCHAR* ColumnBuffer::GetCharPtr() const
	{
		exASSERT(m_allocatedBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLCHAR*>(m_intPtrVar);
	}

	SQLWCHAR* ColumnBuffer::GetWCharPtr() const
	{
		exASSERT(m_allocatedBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLWCHAR*>(m_intPtrVar);
	}


	bool ColumnBuffer::BindColumnBuffer(HSTMT hStmt)
	{
		exASSERT(m_allocatedBuffer);
		exASSERT(!m_isBound);

		void* pBuffer = GetBuffer();
		size_t buffSize = GetBufferSize();
		SQLRETURN ret = 0;
		bool notBound = false;
		m_cb = 0;
		switch (m_columnInfo.m_sqlDataType)
		{
		case SQL_SMALLINT:
			ret = SQLBindCol(hStmt, (SQLUSMALLINT)m_columnInfo.m_ordinalPosition, SQL_C_SSHORT, (SQLPOINTER*)pBuffer, buffSize, &m_cb);
			break;
		case SQL_INTEGER:
			ret = SQLBindCol(hStmt, (SQLUSMALLINT)m_columnInfo.m_ordinalPosition, SQL_C_SLONG, (SQLPOINTER*)pBuffer, buffSize, &m_cb);
			break;
		case SQL_BIGINT:
			ret = SQLBindCol(hStmt, (SQLUSMALLINT)m_columnInfo.m_ordinalPosition, SQL_C_SBIGINT, (SQLPOINTER*)pBuffer, buffSize, &m_cb);
			break;
		case SQL_CHAR:
		case SQL_VARCHAR:
			if (m_charBindingMode == Table::BIND_AS_CHAR || Table::BIND_AS_REPORTED)
			{
				ret = SQLBindCol(hStmt, (SQLUSMALLINT)m_columnInfo.m_ordinalPosition, SQL_C_CHAR, (SQLPOINTER*)pBuffer, buffSize, &m_cb);
			}
			else
			{
				ret = SQLBindCol(hStmt, (SQLUSMALLINT)m_columnInfo.m_ordinalPosition, SQL_C_WCHAR, (SQLPOINTER*)pBuffer, buffSize, &m_cb);
			}
			break;
		case SQL_WCHAR:
		case SQL_WVARCHAR:
			if (m_charBindingMode == Table::BIND_AS_CHAR || Table::BIND_AS_REPORTED)
			{
				ret = SQLBindCol(hStmt, (SQLUSMALLINT)m_columnInfo.m_ordinalPosition, SQL_C_CHAR, (SQLPOINTER*)pBuffer, buffSize, &m_cb);
			}
			else
			{
				ret = SQLBindCol(hStmt, (SQLUSMALLINT)m_columnInfo.m_ordinalPosition, SQL_C_WCHAR, (SQLPOINTER*)pBuffer, buffSize, &m_cb);
			}
			break;
		default:
			notBound = true;
			LOG_ERROR((boost::wformat(L"Not implemented SqlDataType '%s' (%d)") % SqlType2s(m_columnInfo.m_sqlDataType) % m_columnInfo.m_sqlDataType).str());
		}
		if (!notBound && ret != SQL_SUCCESS)
		{
			LOG_ERROR_STMT(hStmt, ret, SQLBindCol);
			notBound = true;
		}

		m_isBound = !notBound;
		return m_isBound;
	}


	//SQLBIGINT BigintVisitor::operator()(SQLINTEGER i) const
	//{
	//	if (!(m_sqlDataType == SQL_SMALLINT || m_sqlDataType == SQL_INTEGER))
	//	{
	//		
	//	}
	//	return i;
	//}

	// Interfaces
	// ----------

}
