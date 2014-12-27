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
		, m_bound(false)
		, m_allocatedBuffer(false)
		, m_haveBuffer(false)
		, m_charBindingMode(mode)
		, m_bufferType(0)
		, m_bufferSize(0)
		, m_columnNr((SQLUSMALLINT) columnInfo.m_ordinalPosition)
		, m_haveColumnInfo(true)
	{
		exASSERT(m_columnNr > 0);
		exASSERT(columnInfo.m_sqlDataType != 0);

		m_queryName = m_columnInfo.GetSqlName();
		m_allocatedBuffer = AllocateBuffer(m_columnInfo);
		m_haveBuffer = m_allocatedBuffer;
	}


	ColumnBuffer::ColumnBuffer(SQLSMALLINT sqlCType, SQLUSMALLINT ordinalPosition, BufferPtrVariant bufferPtrVariant, SQLLEN bufferSize, const std::wstring& queryName)
		: m_bound(false)
		, m_allocatedBuffer(false)
		, m_haveBuffer(true)
		, m_charBindingMode(CharBindingMode::BIND_AS_REPORTED)
		, m_bufferType(sqlCType)
		, m_bufferSize(bufferSize)
		, m_columnNr(ordinalPosition)
		, m_bufferPtr(bufferPtrVariant)
		, m_haveColumnInfo(false)
		, m_queryName(queryName)
	{
		exASSERT(sqlCType != 0);
		exASSERT(ordinalPosition > 0);
		exASSERT(bufferSize > 0);
		exASSERT(!m_queryName.empty());
	}

	// Destructor
	// -----------
	ColumnBuffer::~ColumnBuffer()
	{
		if (m_allocatedBuffer)
		{
			try
			{
				switch (m_bufferType)
				{
				case SQL_C_SSHORT:
					delete GetSmallIntPtr();
					break;
				case SQL_C_SLONG:
					delete GetIntPtr();
					break;
				case SQL_C_SBIGINT:
					delete GetBigIntPtr();
					break;
				case SQL_C_CHAR:
					delete[] GetCharPtr();
					break;
				case SQL_C_WCHAR:
					delete[] GetWCharPtr();
					break;
				case SQL_C_DOUBLE:
					delete[] GetDoublePtr();
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
	bool ColumnBuffer::Bind(HSTMT hStmt)
	{
		exASSERT(m_haveBuffer);
		exASSERT(!m_bound);
		exASSERT(m_bufferType != 0);
		exASSERT(m_bufferSize > 0);

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

		SQLRETURN ret = SQLBindCol(hStmt, m_columnNr, m_bufferType, (SQLPOINTER*)pBuffer, m_bufferSize, &m_cb);
		if (ret != SQL_SUCCESS)
		{
			LOG_ERROR_STMT(hStmt, ret, SQLBindCol);
		};

		m_bound = ( ret == SQL_SUCCESS );
		return m_bound;
	}


	bool ColumnBuffer::AllocateBuffer(const SColumnInfo& columnInfo)
	{
		exASSERT(!m_allocatedBuffer);
		exASSERT(!m_bound);
		exASSERT(!m_haveBuffer);

		m_bufferType = DetermineBufferType();
		if (!m_bufferType)
		{
			return false;
		}
		m_bufferSize = DetermineBufferSize();
		if (!m_bufferSize)
		{
			return false;
		}

		bool failed = false;
		switch (m_bufferType)
		{
		case SQL_C_SSHORT:
			m_bufferPtr = new SQLSMALLINT(0);
			break;
		case SQL_C_SLONG:
			m_bufferPtr = new SQLINTEGER(0);
			break;
		case SQL_C_SBIGINT:
			m_bufferPtr = new SQLBIGINT(0);
			break;
		case SQL_C_CHAR:
			m_bufferPtr = new SQLCHAR[m_bufferSize];
			break;
		case SQL_C_WCHAR:
			m_bufferPtr = new SQLWCHAR[m_bufferSize];
			break;
		case SQL_C_DOUBLE:
			m_bufferPtr = new SQLDOUBLE(0.0);
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
		exASSERT(m_haveBuffer);

		void* pBuffer = NULL;
		switch (m_bufferType)
		{
		case SQL_C_SSHORT:
			return static_cast<void*>(boost::get<SQLSMALLINT*>(m_bufferPtr));
		case SQL_C_SLONG:
			return static_cast<void*>(boost::get<SQLINTEGER*>(m_bufferPtr));
		case SQL_C_SBIGINT:
			return static_cast<void*>(boost::get<SQLBIGINT*>(m_bufferPtr));
		case SQL_C_CHAR:
			return static_cast<void*>(boost::get<SQLCHAR*>(m_bufferPtr));
		case SQL_C_WCHAR:
			return static_cast<void*>(boost::get<SQLWCHAR*>(m_bufferPtr));
		case SQL_C_DOUBLE:
			return static_cast<void*>(boost::get<SQLDOUBLE*>(m_bufferPtr));
		case SQL_C_TYPE_DATE:
		case SQL_C_DATE:
			return static_cast<void*>(boost::get<SQL_DATE_STRUCT*>(m_bufferPtr));
		case SQL_C_TYPE_TIME:
		case SQL_C_TIME:
			return static_cast<void*>(boost::get<SQL_TIME_STRUCT*>(m_bufferPtr));
		case SQL_C_TYPE_TIMESTAMP:
		case SQL_C_TIMESTAMP:
			return static_cast<void*>(boost::get<SQL_TIMESTAMP_STRUCT*>(m_bufferPtr));
		default:
			exASSERT(false);
		}
		return pBuffer;
	}


	size_t ColumnBuffer::DetermineBufferSize() const
	{
		exASSERT(m_bufferType != 0);
		exASSERT(m_columnInfo.m_sqlDataType != SQL_UNKNOWN_TYPE);

		// if the determined buffer type is a simple type its just sizeof
		switch (m_bufferType)
		{
		case SQL_C_SSHORT:
			return sizeof(SQLSMALLINT);
		case SQL_C_SLONG:
			return sizeof(SQLINTEGER);
		case SQL_C_SBIGINT:
			return sizeof(SQLBIGINT);
		case SQL_C_CHAR:
			// else it determines on the ColumnInfo too:
			// If it is a char or (w)char or (w)varchar calculate the string length as if we've used SQL_C_CHAR
			if (m_columnInfo.m_sqlDataType == SQL_CHAR || m_columnInfo.m_sqlDataType == SQL_VARCHAR || 
				m_columnInfo.m_sqlDataType == SQL_WCHAR || m_columnInfo.m_sqlDataType == SQL_WVARCHAR)
			{
				// We could calculate the length using m_columnInfo.m_columnSize + 1) * sizeof(SQLCHAR)
				// TODO: or also use the char_octet_length. Maybe this would be cleaner - some dbs
				// report higher values there than we calculate (like sizeof(SQLWCHAR) would be 3)
				return (m_columnInfo.m_columnSize + 1) * sizeof(SQLCHAR);
			}
			else
			{
				// Should never happen. 
				exASSERT(false);
			}
		case SQL_C_WCHAR:
			if (m_columnInfo.m_sqlDataType == SQL_CHAR || m_columnInfo.m_sqlDataType == SQL_VARCHAR ||
				m_columnInfo.m_sqlDataType == SQL_WCHAR || m_columnInfo.m_sqlDataType == SQL_WVARCHAR)
			{
				return (m_columnInfo.m_columnSize + 1) * sizeof(SQLWCHAR);
			}
			else
			{
				exASSERT(false);
			}
		case SQL_C_DOUBLE:
			return sizeof(SQLDOUBLE);
		default:
			LOG_ERROR((boost::wformat(L"Not implemented SqlDataType '%s' (%d)") % SqlType2s(m_columnInfo.m_sqlDataType) % m_columnInfo.m_sqlDataType).str());
		}
		return 0;
	}


	SQLSMALLINT ColumnBuffer::DetermineBufferType() const
	{
		exASSERT(m_columnInfo.m_sqlDataType != SQL_UNKNOWN_TYPE);

		switch (m_columnInfo.m_sqlDataType)
		{
		case SQL_SMALLINT:
			return SQL_C_SSHORT;
		case SQL_INTEGER:
			return SQL_C_SLONG;
		case SQL_BIGINT:
			return SQL_C_SBIGINT;
		case SQL_CHAR:
		case SQL_VARCHAR:
			if (m_charBindingMode == CharBindingMode::BIND_AS_CHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
			{
				return SQL_C_CHAR;
			}
			else
			{
				return SQL_C_WCHAR;
			}
		case SQL_WCHAR:
		case SQL_WVARCHAR:
			if (m_charBindingMode == CharBindingMode::BIND_AS_WCHAR || m_charBindingMode == CharBindingMode::BIND_AS_REPORTED)
			{
				return SQL_C_WCHAR;
			}
			else
			{
				return SQL_C_CHAR;
			}
		case SQL_DOUBLE:
		case SQL_FLOAT:
		case SQL_REAL:
			return SQL_C_DOUBLE;
		default:
			LOG_ERROR((boost::wformat(L"Not implemented SqlDataType '%s' (%d)") % SqlType2s(m_columnInfo.m_sqlDataType) % m_columnInfo.m_sqlDataType).str());
		}
		return 0;
	}


	ColumnBuffer::operator SQLSMALLINT() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(m_bound);

		// We use the BigIntVisitor here. It will always succeed to convert if 
		// the underlying Value is an int-value or throw otherwise
		SQLBIGINT bigVal = boost::apply_visitor(BigintVisitor(), m_bufferPtr);
		
		// But we are only allowed to Downcast this to a Smallint if the original value was a smallint
		if ( m_bufferType != SQL_C_SSHORT)
		{
			throw CastException(m_bufferType, SQL_C_SSHORT);
		}
		return (SQLSMALLINT)bigVal;
	}


	ColumnBuffer::operator SQLINTEGER() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(m_bound);

		// We use the BigIntVisitor here. It will always succeed to convert if 
		// the underlying Value is an int-value or throw otherwise
		SQLBIGINT bigVal = boost::apply_visitor(BigintVisitor(), m_bufferPtr);
		// But we are only allowed to downcast this to an Int if we are not loosing information
		if (!( m_bufferType == SQL_C_SSHORT || m_bufferType == SQL_C_SLONG))
		{
			// TODO: Fix IT
			throw CastException(m_bufferType, SQL_C_SLONG);
		}
		return (SQLINTEGER)bigVal;
	}


	ColumnBuffer::operator SQLBIGINT() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(m_bound);

		return boost::apply_visitor(BigintVisitor(), m_bufferPtr);
	}


	ColumnBuffer::operator std::wstring() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(m_bound);

		return boost::apply_visitor(WStringVisitor(), m_bufferPtr);
	}


	ColumnBuffer::operator std::string() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(m_bound);

		return boost::apply_visitor(StringVisitor(), m_bufferPtr);
	}


	ColumnBuffer::operator SQLDOUBLE() const
	{
		exASSERT(m_haveBuffer);
		exASSERT(m_bound);

		return boost::apply_visitor(DoubleVisitor(), m_bufferPtr);
	}


	SQLSMALLINT* ColumnBuffer::GetSmallIntPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLSMALLINT*>(m_bufferPtr);
	}

	SQLINTEGER* ColumnBuffer::GetIntPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLINTEGER*>(m_bufferPtr);
	}

	SQLBIGINT* ColumnBuffer::GetBigIntPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLBIGINT*>(m_bufferPtr);
	}

	SQLCHAR* ColumnBuffer::GetCharPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLCHAR*>(m_bufferPtr);
	}

	SQLWCHAR* ColumnBuffer::GetWCharPtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLWCHAR*>(m_bufferPtr);
	}

	SQLDOUBLE* ColumnBuffer::GetDoublePtr() const
	{
		exASSERT(m_haveBuffer);

		// Could throw boost::bad_get
		return boost::get<SQLDOUBLE*>(m_bufferPtr);
	}





	// Interfaces
	// ----------

}
