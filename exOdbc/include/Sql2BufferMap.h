/*!
* \file Sql2BufferMap.h
* \author Elias Gerber <eg@elisium.ch>
* \date 21.06.2014
* \brief Header file for Sql2BufferMap.
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
	* \class Sql2BufferMap
	*
	* \brief Name of an Object.
	*/
	class EXODBCAPI Sql2BufferMap
	{
	public:
		virtual ~Sql2BufferMap() {};

		virtual void RegisterType(SQLSMALLINT sqlType, SQLSMALLINT sqlCType);
		virtual void ClearType(SQLSMALLINT sqlType);
		virtual bool ContainsType(SQLSMALLINT sqlType) const throw();
		virtual SQLSMALLINT GetBufferType(SQLSMALLINT sqlType) const;

	protected:
		typedef std::map<SQLSMALLINT, SQLSMALLINT> TypeMap;
		TypeMap m_typeMap;
	};

	
	class EXODBCAPI DefaultSql2BufferMap
		: public Sql2BufferMap
	{
	public:
		DefaultSql2BufferMap();
	};
}

#endif // SQL2BUFFERMAP_H