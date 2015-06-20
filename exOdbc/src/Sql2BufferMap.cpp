/*!
* \file Sql2BufferMap.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 21.06.2015
* \brief Source file for name objects.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#include "stdafx.h"

// Own header
#include "Sql2BufferMap.h"

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

	void Sql2BufferMap::RegisterType(SQLSMALLINT sqlType, SQLSMALLINT sqlCType)
	{
		m_typeMap[sqlType] = sqlCType;
	}


	void Sql2BufferMap::ClearType(SQLSMALLINT sqlType)
	{
		TypeMap::const_iterator it = m_typeMap.find(sqlType);
		if (it != m_typeMap.end())
		{
			m_typeMap.erase(it);
		}
	}


	bool Sql2BufferMap::ContainsType(SQLSMALLINT sqlType) const
	{
		return m_typeMap.find(sqlType) != m_typeMap.end();
	}


	SQLSMALLINT Sql2BufferMap::GetBufferType(SQLSMALLINT sqlType) const
	{
		TypeMap::const_iterator it = m_typeMap.find(sqlType);
		if (it != m_typeMap.end())
		{
			return it->second;
		}

		NotFoundException nfe(boost::str(boost::wformat(L"SQL Type %s (%d) is not Registered in Sql2BufferMap") % SqlType2s(sqlType) %sqlType));
		SET_EXCEPTION_SOURCE(nfe);
		throw nfe;
	}


	DefaultSql2BufferMap::DefaultSql2BufferMap()
	{
		RegisterType(SQL_SMALLINT, SQL_C_SSHORT);
		RegisterType(SQL_INTEGER, SQL_C_SLONG);
		RegisterType(SQL_BIGINT, SQL_C_SBIGINT);
		RegisterType(SQL_CHAR, SQL_C_CHAR);
		RegisterType(SQL_VARCHAR, SQL_C_CHAR);

			/*
			* SQL_SMALLINT				| SQLSMALLINT*
			* SQL_INTEGER				| SQLINTEGER*
			* SQL_BIGINT				| SQLBIGINT*
			* SQL_CHAR / SQL_VARCHAR	| SQLCHAR* [1]
			* SQL_WCHAR / SQL_WVARCHAR	| SQLWCHAR* [1]
			* SQL_DOUBLE				| SQLDOUBLE*
			* SQL_FLOAT					| SQLDOUBLE*
			* SQL_REAL					| SQLDOUBLE*
			* SQL_DATE					| SQL_DATE_STRUCT*
			* SQL_TIME					| SQL_TIME_STRUCT*
			* SQL_TIME2					| SQL_TIME2_STRUCT* / SQL_TIME_STRUCT* [2]
			* SQL_TIMESTAMP				| SQL_TIMESTAMP_STRUCT*
			* SQL_BINARY				| SQL_CHAR*
			* SQL_VARBINARY				| SQL_CHAR*
			* SQL_LONGVARBINARY			| SQL_CHAR*
			* SQL_NUMERIC				| SQL_NUMERIC_STRUCT*
			* SQL_DECIMAL				| SQL_NUMERIC_STRUCT*
			*/
	}
}
