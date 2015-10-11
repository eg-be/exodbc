/*!
* \file Sql2BufferTypeMap.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 21.06.2015
* \brief Source file for name objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
#include "Sql2BufferTypeMap.h"

// Same component headers
#include "Exception.h"
#include "GenericCBufferType.h"

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
	{}


	void Sql2BufferTypeMap::RegisterType(SQLSMALLINT sqlType, SQLSMALLINT sqlCType, CBufferTypeConstPtr pBufferType) throw()
	{
		exASSERT(pBufferType);
		exASSERT(sqlCType != 0);
		exASSERT(pBufferType->IsSqlCTypeSupported(sqlCType));

		Sql2BufferTypeMapEntry entry(sqlCType, pBufferType);

		m_typeMap[sqlType] = entry;
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
			Sql2BufferTypeMapEntry entry = it->second;
			return entry.m_sqlCType;
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


	void Sql2BufferTypeMap::SetDefault(SQLSMALLINT defaultBufferType, CBufferTypeConstPtr pCBufferType) throw()
	{
		m_hasDefault = true;
		m_defaultBufferType = Sql2BufferTypeMapEntry(defaultBufferType, pCBufferType);
	}


	SQLSMALLINT Sql2BufferTypeMap::GetDefault() const
	{
		exASSERT(HasDefault());
		return m_defaultBufferType.m_sqlCType;
	}


	DefaultSql2BufferMap::DefaultSql2BufferMap(OdbcVersion odbcVersion)
	{
		CBufferTypeConstPtr genericBufferCreator = std::make_shared<GenericCBufferType>();
		RegisterType(SQL_SMALLINT, SQL_C_SSHORT, genericBufferCreator);
		RegisterType(SQL_INTEGER, SQL_C_SLONG, genericBufferCreator);
		RegisterType(SQL_BIGINT, SQL_C_SBIGINT, genericBufferCreator);
		RegisterType(SQL_CHAR, SQL_C_CHAR, genericBufferCreator);
		RegisterType(SQL_VARCHAR, SQL_C_CHAR, genericBufferCreator);
		RegisterType(SQL_WCHAR, SQL_C_WCHAR, genericBufferCreator);
		RegisterType(SQL_WVARCHAR, SQL_C_WCHAR, genericBufferCreator);
		RegisterType(SQL_DOUBLE, SQL_C_DOUBLE, genericBufferCreator);
		RegisterType(SQL_FLOAT, SQL_C_DOUBLE, genericBufferCreator);
		RegisterType(SQL_REAL, SQL_C_FLOAT, genericBufferCreator);
		if (odbcVersion >= OdbcVersion::V_3)
		{
			// Register the Odbc Version 3.x Types only if the version is >= 3.x
			RegisterType(SQL_TYPE_DATE, SQL_C_TYPE_DATE, genericBufferCreator);
			RegisterType(SQL_TYPE_TIME, SQL_C_TYPE_TIME, genericBufferCreator);
			RegisterType(SQL_TYPE_TIMESTAMP, SQL_C_TYPE_TIMESTAMP, genericBufferCreator);
			// But always register the old 2.x types. Some DBs still report them when working with 3.x
			// But map them to the new 3.x types if we have >= 3.x
			RegisterType(SQL_DATE, SQL_C_TYPE_DATE, genericBufferCreator);
			RegisterType(SQL_TIME, SQL_C_TYPE_TIME, genericBufferCreator);
			RegisterType(SQL_TIMESTAMP, SQL_C_TYPE_TIMESTAMP, genericBufferCreator);
		}
		else
		{
			// register old 2.x types and map them to the old types
			RegisterType(SQL_DATE, SQL_C_DATE, genericBufferCreator);
			RegisterType(SQL_TIME, SQL_C_TIME, genericBufferCreator);
			RegisterType(SQL_TIMESTAMP, SQL_C_TIMESTAMP, genericBufferCreator);
		}
#ifdef HAVE_MSODBCSQL_H
		if (odbcVersion >= OdbcVersion::V_3_8)
		{
			RegisterType(SQL_SS_TIME2, SQL_C_SS_TIME2, genericBufferCreator);
		}
		else if (odbcVersion >= OdbcVersion::V_3)
		{
			RegisterType(SQL_SS_TIME2, SQL_C_TYPE_TIME, genericBufferCreator);
		}
		else
		{
			RegisterType(SQL_SS_TIME2, SQL_C_TIME, genericBufferCreator);
		}
#endif

		RegisterType(SQL_BINARY, SQL_C_BINARY, genericBufferCreator);
		RegisterType(SQL_VARBINARY, SQL_C_BINARY, genericBufferCreator);
		RegisterType(SQL_LONGVARBINARY, SQL_C_BINARY, genericBufferCreator);
		RegisterType(SQL_NUMERIC, SQL_C_NUMERIC, genericBufferCreator);
		RegisterType(SQL_DECIMAL, SQL_C_NUMERIC, genericBufferCreator);
	}


	WCharSql2BufferMap::WCharSql2BufferMap()
	{
		CBufferTypeConstPtr genericBufferCreator = std::make_shared<GenericCBufferType>();

		SetDefault(SQL_C_WCHAR, genericBufferCreator);
	}


	CharSql2BufferMap::CharSql2BufferMap()
	{
		CBufferTypeConstPtr genericBufferCreator = std::make_shared<GenericCBufferType>();

		SetDefault(SQL_C_CHAR, genericBufferCreator);
	}


	CharAsWCharSql2BufferMap::CharAsWCharSql2BufferMap(OdbcVersion odbcVersion)
		: DefaultSql2BufferMap(odbcVersion)
	{
		CBufferTypeConstPtr genericBufferCreator = std::make_shared<GenericCBufferType>();

		RegisterType(SQL_CHAR, SQL_C_WCHAR, genericBufferCreator);
		RegisterType(SQL_VARCHAR, SQL_C_WCHAR, genericBufferCreator);
	}


	WCharAsCharSql2BufferMap::WCharAsCharSql2BufferMap(OdbcVersion odbcVersion)
		: DefaultSql2BufferMap(odbcVersion)
	{
		CBufferTypeConstPtr genericBufferCreator = std::make_shared<GenericCBufferType>();

		RegisterType(SQL_WCHAR, SQL_C_CHAR, genericBufferCreator);
		RegisterType(SQL_WVARCHAR, SQL_C_CHAR, genericBufferCreator);
	}
}
