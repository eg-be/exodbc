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
	ColumnBuffer::ColumnBuffer(const SColumnInfo& columnInfo, CharBindingMode mode)
		: m_columnInfo(columnInfo)
		, m_isBound(false)
		, m_allocatedBuffer(false)
		, m_charBindingMode(mode)
		, m_bufferType(0)
	{
		m_allocatedBuffer = AllocateBuffer(m_columnInfo);
	}


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
					if (m_charBindingMode == CharBindingMode::BIND_AS_CHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
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
					if (m_charBindingMode == CharBindingMode::BIND_AS_WCHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
					{
						delete[] GetWCharPtr();
					}
					else
					{
						delete[] GetCharPtr();
					}
					break;
				default:
					exASSERT(false);
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
	bool ColumnBuffer::BindColumnBuffer(HSTMT hStmt)
	{
		exASSERT(m_allocatedBuffer);
		exASSERT(!m_isBound);

		void* pBuffer = NULL;
		try
		{
			pBuffer = GetBuffer();
		}
		catch (boost::bad_get ex)
		{
			LOG_ERROR(L"Failed in GetBuffer() - probably allocated buffer type does not match sql type in SColumnInfo.");
			exASSERT(false);
			return false;
		}
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
			if (m_charBindingMode == CharBindingMode::BIND_AS_CHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
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
			if (m_charBindingMode == CharBindingMode::BIND_AS_CHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
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


	bool ColumnBuffer::AllocateBuffer(const SColumnInfo& columnInfo)
	{
		exASSERT(!m_allocatedBuffer);
		exASSERT(!m_isBound);

		bool failed = false;
		switch (columnInfo.m_sqlDataType)
		{
		case SQL_SMALLINT:
			m_bufferPtr = new SQLSMALLINT(0);
			m_bufferType = SQL_C_SSHORT;
			break;
		case SQL_INTEGER:
			m_bufferPtr = new SQLINTEGER(0);
			m_bufferType = SQL_C_SLONG;
			break;
		case SQL_BIGINT:
			m_bufferPtr = new SQLBIGINT(0);
			m_bufferType = SQL_C_SBIGINT;
			break;
		case SQL_CHAR:
		case SQL_VARCHAR:
			if (m_charBindingMode == CharBindingMode::BIND_AS_CHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
			{
				m_bufferPtr = new SQLCHAR[GetBufferSize() + 1];
				m_bufferType = SQL_C_CHAR;
			}
			else
			{
				m_bufferPtr = new SQLWCHAR[GetBufferSize() + 1];
				m_bufferType = SQL_C_WCHAR;
			}
			break;
		case SQL_WCHAR:
		case SQL_WVARCHAR:
			if (m_charBindingMode == CharBindingMode::BIND_AS_WCHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
			{
				m_bufferPtr = new SQLWCHAR[GetBufferSize() + 1];
				m_bufferType = SQL_C_WCHAR;
			}
			else
			{
				m_bufferPtr = new SQLCHAR[GetBufferSize() + 1];
				m_bufferType = SQL_C_CHAR;
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
			return static_cast<void*>(boost::get<SQLSMALLINT*>(m_bufferPtr));
		case SQL_INTEGER:
			return static_cast<void*>(boost::get<SQLINTEGER*>(m_bufferPtr));
		case SQL_BIGINT:
			return static_cast<void*>(boost::get<SQLBIGINT*>(m_bufferPtr));
		case SQL_CHAR:
		case SQL_VARCHAR:
			if (m_charBindingMode == CharBindingMode::BIND_AS_CHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
			{
				return static_cast<void*>(boost::get<SQLCHAR*>(m_bufferPtr));
			}
			else
			{
				return static_cast<void*>(boost::get<SQLWCHAR*>(m_bufferPtr));
			}
		case SQL_WCHAR:
		case SQL_WVARCHAR:
			if (m_charBindingMode == CharBindingMode::BIND_AS_WCHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
			{
				return static_cast<void*>(boost::get<SQLWCHAR*>(m_bufferPtr));
			}
			else
			{
				return static_cast<void*>(boost::get<SQLCHAR*>(m_bufferPtr));
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
			// We could calculate the length using m_columnInfo.m_columnSize + 1) * sizeof(SQLCHAR)
			// TODO: or also use the char_octet_length. Maybe this would be cleaner - some dbs
			// report higher values there than we calculate (like sizeof(SQLWCHAR) would be 3)
			if (m_charBindingMode == CharBindingMode::BIND_AS_CHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
			{
				return (m_columnInfo.m_columnSize + 1) * sizeof(SQLCHAR);
			}
			else
			{
				return (m_columnInfo.m_columnSize + 1) * sizeof(SQLWCHAR);
			}
		case SQL_WCHAR:
		case SQL_WVARCHAR:
			if (m_charBindingMode == CharBindingMode::BIND_AS_WCHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
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

		// TODO: We should not use the SQL-Data type here for the visitor. In there its only
		// used for the exception-message. The visitor is trying to convert from an ODBC-C-TYPE!!

		// We use the BigIntVisitor here. It will always succeed to convert if 
		// the underlying Value is an int-value or throw otherwise
		SQLBIGINT bigVal = boost::apply_visitor(BigintVisitor(m_columnInfo.m_sqlDataType), m_bufferPtr);
		// But we are only allowed to Downcast this to a Smallint if the original value was a smallint
		
		// TODO: We should do this by determining the type of the Variant
		// Then we would be independent of sqlDataType, except during binding

		if (!(m_columnInfo.m_sqlDataType == SQL_SMALLINT))
		{
			// TODO: FIx IT
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
		SQLBIGINT bigVal = boost::apply_visitor(BigintVisitor(m_columnInfo.m_sqlDataType), m_bufferPtr);
		// But we are only allowed to downcast this to an Int if we are not loosing information
		if (!(m_columnInfo.m_sqlDataType == SQL_SMALLINT || m_columnInfo.m_sqlDataType == SQL_INTEGER))
		{
			// TODO: Fix IT
			throw CastException(m_columnInfo.m_sqlDataType, SQL_C_SLONG);
		}
		return (SQLINTEGER)bigVal;
	}


	ColumnBuffer::operator SQLBIGINT() const
	{
		exASSERT(m_allocatedBuffer);
		exASSERT(m_isBound);

		return boost::apply_visitor(BigintVisitor(m_columnInfo.m_sqlDataType), m_bufferPtr);
	}


	ColumnBuffer::operator std::wstring() const
	{
		exASSERT(m_allocatedBuffer);
		exASSERT(m_isBound);

		return boost::apply_visitor(WStringVisitor(m_columnInfo.m_sqlDataType), m_bufferPtr);
	}


	ColumnBuffer::operator std::string() const
	{
		exASSERT(m_allocatedBuffer);
		exASSERT(m_isBound);

		return boost::apply_visitor(StringVisitor(m_columnInfo.m_sqlDataType), m_bufferPtr);
	}


	SQLSMALLINT* ColumnBuffer::GetSmallIntPtr() const
	{
		exASSERT(m_allocatedBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLSMALLINT*>(m_bufferPtr);
	}

	SQLINTEGER* ColumnBuffer::GetIntPtr() const
	{
		exASSERT(m_allocatedBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLINTEGER*>(m_bufferPtr);
	}

	SQLBIGINT* ColumnBuffer::GetBigIntPtr() const
	{
		exASSERT(m_allocatedBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLBIGINT*>(m_bufferPtr);
	}

	SQLCHAR* ColumnBuffer::GetCharPtr() const
	{
		exASSERT(m_allocatedBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLCHAR*>(m_bufferPtr);
	}

	SQLWCHAR* ColumnBuffer::GetWCharPtr() const
	{
		exASSERT(m_allocatedBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLWCHAR*>(m_bufferPtr);
	}





	// Interfaces
	// ----------

}
