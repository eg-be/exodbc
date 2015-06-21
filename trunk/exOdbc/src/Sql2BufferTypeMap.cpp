/*!
* \file Sql2BufferTypeMap.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 21.06.2015
* \brief Source file for name objects.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#include "stdafx.h"

// Own header
#include "Sql2BufferTypeMap.h"

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
	Sql2BufferTypeMap::Sql2BufferTypeMap() throw()
		: m_hasDefault(false)
		, m_defaultBufferType(0)
	{}


	void Sql2BufferTypeMap::RegisterType(SQLSMALLINT sqlType, SQLSMALLINT sqlCType) throw()
	{
		m_typeMap[sqlType] = sqlCType;
	}


	void Sql2BufferTypeMap::ClearType(SQLSMALLINT sqlType) throw()
	{
		TypeMap::const_iterator it = m_typeMap.find(sqlType);
		if (it != m_typeMap.end())
		{
			m_typeMap.erase(it);
		}
	}


	bool Sql2BufferTypeMap::ContainsType(SQLSMALLINT sqlType) const
	{
		return m_typeMap.find(sqlType) != m_typeMap.end();
	}


	SQLSMALLINT Sql2BufferTypeMap::GetBufferType(SQLSMALLINT sqlType) const
	{
		TypeMap::const_iterator it = m_typeMap.find(sqlType);
		if (it != m_typeMap.end())
		{
			return it->second;
		}

		if (HasDefault())
		{
			return GetDefault();
		}

		NotSupportedException nse(NotSupportedException::Type::SQL_TYPE, sqlType, L"SQL Type is not registered in Sql2BufferTypeMap and no default Type is set: ");
		SET_EXCEPTION_SOURCE(nse);
		throw nse;
	}


	bool Sql2BufferTypeMap::HasDefault() const throw()
	{
		return m_hasDefault;
	}


	void Sql2BufferTypeMap::SetDefault(SQLSMALLINT defaultBufferType) throw()
	{
		m_hasDefault = true;
		m_defaultBufferType = defaultBufferType;
	}


	SQLSMALLINT Sql2BufferTypeMap::GetDefault() const
	{
		exASSERT(HasDefault());
		return m_defaultBufferType;
	}


	DefaultSql2BufferMap::DefaultSql2BufferMap(OdbcVersion odbcVersion)
	{
		RegisterType(SQL_SMALLINT, SQL_C_SSHORT);
		RegisterType(SQL_INTEGER, SQL_C_SLONG);
		RegisterType(SQL_BIGINT, SQL_C_SBIGINT);
		RegisterType(SQL_CHAR, SQL_C_CHAR);
		RegisterType(SQL_VARCHAR, SQL_C_CHAR);
		RegisterType(SQL_WCHAR, SQL_C_CHAR);
		RegisterType(SQL_WVARCHAR, SQL_C_CHAR);
		RegisterType(SQL_DOUBLE, SQL_C_DOUBLE);
		RegisterType(SQL_FLOAT, SQL_C_DOUBLE);
		RegisterType(SQL_REAL, SQL_C_DOUBLE);
		if (odbcVersion >= OdbcVersion::V_3)
		{
			RegisterType(SQL_DATE, SQL_C_TYPE_DATE);
			RegisterType(SQL_TIME, SQL_C_TYPE_TIME);
			RegisterType(SQL_TIMESTAMP, SQL_C_TYPE_TIMESTAMP);
		}
		else
		{
			RegisterType(SQL_DATE, SQL_C_DATE);
			RegisterType(SQL_TIME, SQL_C_TIME);
			RegisterType(SQL_TIMESTAMP, SQL_C_TIMESTAMP);
		}
#ifdef HAVE_MSODBCSQL_H
		if (odbcVersion >= OdbcVersion::V_3_8)
		{
			RegisterType(SQL_SS_TIME2, SQL_C_SS_TIME2);
		}
		else if (odbcVersion >= OdbcVersion::V_3)
		{
			RegisterType(SQL_SS_TIME2, SQL_C_TYPE_TIME);
		}
		else
		{
			RegisterType(SQL_SS_TIME2, SQL_C_TIME);
		}
#endif

		RegisterType(SQL_BINARY, SQL_C_BINARY);
		RegisterType(SQL_VARBINARY, SQL_C_BINARY);
		RegisterType(SQL_LONGVARBINARY, SQL_C_BINARY);
		RegisterType(SQL_NUMERIC, SQL_C_NUMERIC);
		RegisterType(SQL_DECIMAL, SQL_C_NUMERIC);
	}
}
