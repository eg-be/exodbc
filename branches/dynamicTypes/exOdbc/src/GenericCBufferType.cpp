/*!
* \file GenericCBufferType.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 01.10.2015
* \brief Header file for the Extended Types of Microsoft SQL Server.
* \copyright GNU Lesser General Public License Version 3
*/

#include "stdafx.h"

// Own header
#include "GenericCBufferType.h"

// Same component headers
// Other headers

// Debug
#include "DebugNew.h"

namespace exodbc
{
	const std::set<SQLSMALLINT> GenericCBufferType::s_supportedCBufferTypes
	{
		SQL_C_SSHORT, SQL_C_SLONG, SQL_C_SBIGINT,
		SQL_C_CHAR, SQL_C_WCHAR,
		SQL_C_DOUBLE, SQL_C_FLOAT,
		SQL_C_TYPE_DATE, SQL_C_TYPE_TIME, SQL_C_TYPE_TIMESTAMP,
		SQL_C_DATE, SQL_C_TIME, SQL_C_TIMESTAMP,
		SQL_C_NUMERIC,
		SQL_C_BINARY
	};


	// Construction
	// ------------
	GenericCBufferType::GenericCBufferType()
		: m_bufferSize(0)
		, m_pBuffer(NULL)
		, m_bufferSqlCType(0)
	{
	};


	GenericCBufferType::GenericCBufferType(SQLSMALLINT sqlCType, SQLLEN nrOfElements)
		: m_bufferSize(0)
		, m_pBuffer(NULL)
		, m_bufferSqlCType(0)
	{
		exASSERT(IsSqlCTypeSupported(sqlCType));
		exASSERT(m_pBuffer == NULL);
		exASSERT(m_bufferSize == 0);

		switch (sqlCType)
		{
		case SQL_C_SSHORT:
			m_pBuffer = new SQLSMALLINT(0);
			m_bufferSize = sizeof(SQLSMALLINT);
			break;
		case SQL_C_SLONG:
			m_pBuffer = new SQLINTEGER(0);
			m_bufferSize = sizeof(SQLINTEGER);
			break;
		case SQL_C_SBIGINT:
			m_pBuffer = new SQLBIGINT(0);
			m_bufferSize = sizeof(SQLBIGINT);
			break;
		case SQL_C_CHAR:
		case SQL_C_BINARY:
			m_pBuffer = new SQLCHAR[nrOfElements];
			m_bufferSize = nrOfElements * sizeof(SQLCHAR);
			ZeroMemory(m_pBuffer, m_bufferSize);
			break;
		case SQL_C_WCHAR:
			m_pBuffer = new SQLCHAR[nrOfElements];
			m_bufferSize = nrOfElements * sizeof(SQLWCHAR);
			ZeroMemory(m_pBuffer, m_bufferSize);
			break;
		case SQL_C_DOUBLE:
			m_pBuffer = new SQLDOUBLE(0.0);
			m_bufferSize = sizeof(SQLDOUBLE);
			break;
		case SQL_C_FLOAT:
			m_pBuffer = new SQLREAL(0.0);
			m_bufferSize = sizeof(SQLREAL);
			break;
		case SQL_C_TYPE_DATE:
		case SQL_C_DATE:
			m_pBuffer = new SQL_DATE_STRUCT();
			m_bufferSize = sizeof(SQL_DATE_STRUCT);
			ZeroMemory(m_pBuffer, m_bufferSize);
			break;
		case SQL_C_TYPE_TIME:
		case SQL_C_TIME:
			m_pBuffer = new SQL_TIME_STRUCT();
			m_bufferSize = sizeof(SQL_TIME_STRUCT);
			ZeroMemory(m_pBuffer, m_bufferSize);
			break;
		case SQL_C_TYPE_TIMESTAMP:
		case SQL_C_TIMESTAMP:
			m_pBuffer = new SQL_TIMESTAMP_STRUCT();
			m_bufferSize = sizeof(SQL_TIMESTAMP_STRUCT);
			ZeroMemory(m_pBuffer, m_bufferSize);
			break;
		case SQL_C_NUMERIC:
			m_pBuffer = new SQL_NUMERIC_STRUCT();
			m_bufferSize = sizeof(SQL_NUMERIC_STRUCT);
			ZeroMemory(m_pBuffer, m_bufferSize);
			break;

		default:
			exASSERT(false);
		}

		m_bufferSqlCType = sqlCType;
	}


	// Destruction
	// -----------
	GenericCBufferType::~GenericCBufferType()
	{
		if (m_pBuffer)
		{
			switch (m_bufferSqlCType)
			{
			case SQL_C_BINARY:
			case SQL_C_CHAR:
			case SQL_C_WCHAR:
				delete[] m_pBuffer;
				break;

			default:
				delete m_pBuffer;
				break;
			}

			m_pBuffer = NULL;
			m_bufferSize = 0;
			m_bufferSqlCType = 0;
		}
	};


	// Implementation
	// --------------
	CBufferType* GenericCBufferType::CreateInstance(SQLSMALLINT sqlCType, SQLLEN nrOfElements) const
	{
		exASSERT(IsSqlCTypeSupported(sqlCType));

		GenericCBufferType* pType = new GenericCBufferType(sqlCType, nrOfElements);
		return pType;
	}


	std::set<SQLSMALLINT> GenericCBufferType::GetSupportedSqlCTypes() const
	{
		return s_supportedCBufferTypes;
	}


	bool GenericCBufferType::IsSqlCTypeSupported(SQLSMALLINT sqlCType) const
	{
		return s_supportedCBufferTypes.find(sqlCType) != s_supportedCBufferTypes.end();
	}


	SQLSMALLINT GenericCBufferType::GetSmallInt() const
	{
		exASSERT(m_pBuffer);
		exASSERT(m_bufferSqlCType == SQL_C_SSHORT);

		SQLSMALLINT* pSi = static_cast<SQLSMALLINT*>(m_pBuffer);
		return *pSi;
	}


	void GenericCBufferType::SetSmallInt(SQLSMALLINT value)
	{
		exASSERT(m_pBuffer);
		exASSERT(m_bufferSqlCType == SQL_C_SHORT);

		SQLSMALLINT* pSi = static_cast<SQLSMALLINT*>(m_pBuffer);
		*pSi = value;
	}
} // namespace exodbc
