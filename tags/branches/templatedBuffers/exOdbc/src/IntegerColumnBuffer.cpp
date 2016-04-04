/*!
* \file IntegerColumnBuffer.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 18.10.2015
* \brief Source file for info objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
#include "IntegerColumnBuffer.h"

// Same component headers
#include "Exception.h"

// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	IntegerColumnBuffer::IntegerColumnBuffer(SQLSMALLINT sqlCType)
	{
		switch (sqlCType)
		{
		case SQL_C_USHORT:
			m_intVariant = SqlUShortBuffer();
			break;
		case SQL_C_ULONG:
			m_intVariant = SqlULongBuffer();
			break;
		case SQL_C_UBIGINT:
			m_intVariant = SqlUBigIntBuffer();
			break;
		case SQL_C_SSHORT:
			m_intVariant = SqlShortBuffer();
			break;
		case SQL_C_SLONG:
			m_intVariant = SqlLongBuffer();
			break;
		case SQL_C_SBIGINT:
			m_intVariant = SqlBigIntBuffer();
			break;
		default:
			NotSupportedException nse(NotSupportedException::Type::SQL_C_TYPE, sqlCType);
			SET_EXCEPTION_SOURCE(nse);
			throw nse;
			break;
		}
	}

	// Implementation
	// --------------
	const std::set<SQLSMALLINT> IntegerColumnBuffer::s_supportedCTypes = { 
		SQL_C_USHORT, SQL_C_ULONG, SQL_C_UBIGINT, 
		SQL_C_SSHORT, SQL_C_SLONG, SQL_C_SBIGINT };

	IColumnBufferPtr IntegerColumnBuffer::CreateBuffer(SQLSMALLINT sqlCType)
	{
		IColumnBufferPtr pBuffer = std::make_shared<IntegerColumnBuffer>(sqlCType);
		return pBuffer;
	}


	void IntegerColumnBuffer::BindSelect(SQLHSTMT hStmt, SQLSMALLINT columnNr) const
	{
		SQLPOINTER pBuffer = NULL;

		try
		{
			switch (m_intVariant.which())
			{
			case VAR_USHORT:
			{
				const SqlUShortBuffer& si = boost::get<SqlUShortBuffer>(m_intVariant);
				pBuffer = (SQLPOINTER)si.GetBuffer().get();
			}
			break;
			case VAR_ULONG:
			{
				const SqlULongBuffer& i = boost::get<SqlULongBuffer>(m_intVariant);
				pBuffer = (SQLPOINTER)i.GetBuffer().get();

			}
			break;
			case VAR_UBIGINT:
			{
				const SqlUBigIntBuffer& bi = boost::get<SqlUBigIntBuffer>(m_intVariant);
				pBuffer = (SQLPOINTER)bi.GetBuffer().get();

			}
			break;
			case VAR_SSHORT:
			{
				const SqlShortBuffer& si = boost::get<SqlShortBuffer>(m_intVariant);
				pBuffer = (SQLPOINTER)si.GetBuffer().get();
			}
			break;
			case VAR_SLONG:
			{
				const SqlLongBuffer& i = boost::get<SqlLongBuffer>(m_intVariant);
				pBuffer = (SQLPOINTER)i.GetBuffer().get();

			}
			break;
			case VAR_SBIGINT:
			{
				const SqlBigIntBuffer& bi = boost::get<SqlBigIntBuffer>(m_intVariant);
				pBuffer = (SQLPOINTER)bi.GetBuffer().get();

			}
			break;
			default:
				exASSERT(false);
			}
		}
		catch (const boost::bad_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw;
		}
	}


	SQLSMALLINT IntegerColumnBuffer::GetSqlCType() const
	{
		try
		{
			switch (m_intVariant.which())
			{
			case VAR_USHORT:
				return SqlUShortBuffer::GetSqlCType();
			case VAR_ULONG:
				return SqlULongBuffer::GetSqlCType();
			case VAR_UBIGINT:
				return SqlUBigIntBuffer::GetSqlCType();
			case VAR_SSHORT:
				return SqlShortBuffer::GetSqlCType();
			case VAR_SLONG:
				return SqlLongBuffer::GetSqlCType();
			case VAR_SBIGINT:
				return SqlBigIntBuffer::GetSqlCType();
			default:
				exASSERT(false);
			}
		}
		catch (const boost::bad_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw we;
		}
		// make compiler happy
		return SQL_UNKNOWN_TYPE;
	}


	bool IntegerColumnBuffer::IsNull() const
	{
		try
		{
			switch (m_intVariant.which())
			{
			case VAR_USHORT:
				return boost::get<SqlUShortBuffer>(m_intVariant).IsNull();
			case VAR_ULONG:
				return boost::get<SqlULongBuffer>(m_intVariant).IsNull();
			case VAR_UBIGINT:
				return boost::get<SqlUBigIntBuffer>(m_intVariant).IsNull();
			case VAR_SSHORT:
				return boost::get<SqlShortBuffer>(m_intVariant).IsNull();
			case VAR_SLONG:
				return boost::get<SqlLongBuffer>(m_intVariant).IsNull();
			case VAR_SBIGINT:
				return boost::get<SqlBigIntBuffer>(m_intVariant).IsNull();
			default:
				exASSERT(false);
			}
		}
		catch (const boost::bad_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw;
		}
		// make compiler happy
		return false;
	}
	
	
	void IntegerColumnBuffer::SetNull()
	{
		try
		{
			switch (m_intVariant.which())
			{
			case VAR_USHORT:
				boost::get<SqlUShortBuffer>(m_intVariant).SetNull();
			case VAR_ULONG:
				boost::get<SqlULongBuffer>(m_intVariant).SetNull();
			case VAR_UBIGINT:
				boost::get<SqlUBigIntBuffer>(m_intVariant).SetNull();
			case VAR_SSHORT:
				boost::get<SqlShortBuffer>(m_intVariant).SetNull();
			case VAR_SLONG:
				boost::get<SqlLongBuffer>(m_intVariant).SetNull();
			case VAR_SBIGINT:
				boost::get<SqlBigIntBuffer>(m_intVariant).SetNull();
			default:
				exASSERT(false);
			}
		}
		catch (const boost::bad_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw;
		}
	}


	// Interfaces
	// ----------
}
