/*!
* \file Sql2BufferTypeMap.h
* \author Elias Gerber <eg@elisium.ch>
* \date 21.06.2014
* \brief Header file for Sql2BufferTypeMap.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#pragma once
#ifndef SQL2BUFFERMAP_H
#define SQL2BUFFERMAP_H

// Same component headers
#include "exOdbc.h"

// Other headers
// System headers
#include <map>

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class Sql2BufferTypeMap
	*
	* \brief Name of an Object.
	*/
	class EXODBCAPI Sql2BufferTypeMap
	{
	public:
		Sql2BufferTypeMap();
		virtual ~Sql2BufferTypeMap() {};

		virtual void RegisterType(SQLSMALLINT sqlType, SQLSMALLINT sqlCType);
		virtual void ClearType(SQLSMALLINT sqlType);
		virtual bool ContainsType(SQLSMALLINT sqlType) const throw();
		virtual SQLSMALLINT GetBufferType(SQLSMALLINT sqlType) const;
		virtual bool HasDefault() const throw();
		virtual void SetDefault(SQLSMALLINT defaultBufferType) throw();
		virtual SQLSMALLINT GetDefault() const;

	protected:
		bool		m_hasDefault;
		SQLSMALLINT	m_defaultBufferType;

		typedef std::map<SQLSMALLINT, SQLSMALLINT> TypeMap;
		TypeMap m_typeMap;
	};

	
	class EXODBCAPI DefaultSql2BufferMap
		: public Sql2BufferTypeMap
	{
	public:
		DefaultSql2BufferMap();
	};
}

#endif // SQL2BUFFERMAP_H