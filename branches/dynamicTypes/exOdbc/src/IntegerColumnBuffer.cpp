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
		case SQL_C_SSHORT:
		{
			m_intVariant = SqlSmallIntBuffer();
		}
			break;
		case SQL_C_SLONG:
		{
			m_intVariant = SqlIntBuffer();
		}
			break;
		case SQL_C_SBIGINT:
		{
			m_intVariant = SqlBigIntBuffer();
		}
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
	const std::set<SQLSMALLINT> IntegerColumnBuffer::s_supportedCTypes = { SQL_C_SHORT, SQL_C_SLONG, SQL_C_SBIGINT };

	std::shared_ptr<IColumnBuffer> IntegerColumnBuffer::CreateBuffer(SQLSMALLINT sqlCType)
	{
		std::shared_ptr<IColumnBuffer> pBuffer = std::make_shared<IntegerColumnBuffer>(sqlCType);
		return pBuffer;
	}


	void IntegerColumnBuffer::BindSelect(SQLHSTMT hStmt, SQLSMALLINT columnNr) const
	{
		SQLPOINTER pBuffer = NULL;

		try
		{
			switch (m_intVariant.which())
			{
			case VAR_SMALLINT:
			{
				const SqlSmallIntBuffer& si = boost::get<SqlSmallIntBuffer>(m_intVariant);
				pBuffer = (SQLPOINTER)si.GetBuffer().get();
			}
			break;
			case VAR_INT:
			{
				const SqlIntBuffer& i = boost::get<SqlIntBuffer>(m_intVariant);
				pBuffer = (SQLPOINTER)i.GetBuffer().get();

			}
			break;
			case VAR_BIGINT:
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
		switch (m_intVariant.which())
		{
		case VAR_SMALLINT:
			return SqlSmallIntBuffer::GetSqlCType();
		case VAR_INT:
			return SqlIntBuffer::GetSqlCType();
		case VAR_BIGINT:
			return SqlBigIntBuffer::GetSqlCType();
		default:
			exASSERT(false);
		}
		// make compiler happy
		return SQL_UNKNOWN_TYPE;
	}

	// Interfaces
	// ----------
}
