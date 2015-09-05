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
#include "boost/smart_ptr.hpp"

// System headers
#include <map>

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class Sql2BufferTypeMap
	*
	* \brief Map defining the which SQL C Types SQL Types are mapped. Used by the ColumnBuffer
	*		to determine the type of the Buffer to allocated, if ColumnBuffers are created
	*		automatically.
	*/
	class EXODBCAPI Sql2BufferTypeMap
	{
	public:
		/*!
		* \brief
		*/
		Sql2BufferTypeMap() throw();
		virtual ~Sql2BufferTypeMap() {};

		/*!
		* \brief	Register a Type. Overwrites an already registered sqlType.
		* \param sqlType	The SQL Type.
		* \param sqlCType	The SQL C Type to map this SQL Type to.
		*/
		virtual void RegisterType(SQLSMALLINT sqlType, SQLSMALLINT sqlCType) throw();


		/*!
		* \brief	Clear a Type. Does nothing if given SQL Type not registered.
		* \param sqlType	The SQL Type to clear from the map.
		*/
		virtual void ClearType(SQLSMALLINT sqlType) throw();


		/*!
		* \brief	Test if a SQL Type is registered.
		* \param sqlType	The SQL Type to test for.
		* \return bool		True if the passed SQL Type is registered.
		*/
		virtual bool ContainsType(SQLSMALLINT sqlType) const throw();


		/*!
		* \brief	Get the SQL C Type for a given SQL Type.
		* \details	If no such SQL Type is registered but a Default Type is set, the
		*			Default Type is returned. Else throws.
		* \param sqlType		The SQL Type to get the SQL C Type for.
		* \return SQLSMALLINT	The SQL C Type to use for the given SQL Type.
		* \throw NotSupportedException		If SQL Type is not registered and no Default Type is set.
		*/
		virtual SQLSMALLINT GetBufferType(SQLSMALLINT sqlType) const;
		
		
		/*!
		* \brief	Test if a Default Type is set.
		* \return bool	True if a Default has been set.
		*/
		virtual bool HasDefault() const throw();
		
		
		/*!
		* \brief	Set the Default Type.
		* \param defaultBufferType	Default SQL C Type.
		*/
		virtual void SetDefault(SQLSMALLINT defaultBufferType) throw();


		/*!
		* \brief	Get the Default Type.
		* \return SQLSMALLINT	Default SQL C Type.
		* \throw Exception		If no Default SQL C Type has been set.
		*/
		virtual SQLSMALLINT GetDefault() const;

	protected:
		bool		m_hasDefault;
		SQLSMALLINT	m_defaultBufferType;

		typedef std::map<SQLSMALLINT, SQLSMALLINT> TypeMap;
		TypeMap m_typeMap;
	};
	typedef boost::shared_ptr<Sql2BufferTypeMap> Sql2BufferTypeMapPtr;
	typedef boost::shared_ptr<const Sql2BufferTypeMap> ConstSql2BufferTypeMapPtr;
	
	/*!
	 * \class DefaultSql2BufferMap
	 *
	 * \brief The Default used Sql2BufferMap. Does not set a Default Binding.
	 * \details The following Bindings are registered upon construction:
	 *
	 * SQL-Type					| Buffer-Type
	 * --------------------------|------------
	 * SQL_SMALLINT				| SQL_C_SSHORT
	 * SQL_INTEGER				| SQL_C_SLONG
	 * SQL_BIGINT				| SQL_C_SBIGINT
	 * SQL_CHAR / SQL_VARCHAR	| SQL_C_CHAR
	 * SQL_WCHAR / SQL_WVARCHAR	| SQL_C_WCHAR
	 * SQL_DOUBLE				| SQL_C_DOBULE
	 * SQL_FLOAT				| SQL_C_DOBULE
	 * SQL_REAL					| SQL_C_DOUBLE
	 * SQL_DATE	/ SQL_TYPE_DATE	| SQL_C_TYPE_DATE / SQL_C_DATE [1]
	 * SQL_TIME	/ SQL_TYPE_TIME | SQL_C_TYPE_TIME / SQL_C_TIME [1]
	 * SQL_SS_TIME2				| SQL_C_SS_TIME2 / SQL_C_TIME [1] [2]
	 * SQL_TIMESTAMP / SQL_TYPE_TIMESTAMP | SQL_C_TYPE_TIMESTAMP / SQL_C_TIMESTAMP [1]
	 * SQL_BINARY				| SQL_C_BINARY
	 * SQL_VARBINARY			| SQL_C_BINARY
	 * SQL_LONGVARBINARY		| SQL_C_BINARY
	 * SQL_NUMERIC				| SQL_C_NUMERIC
	 * SQL_DECIMAL				| SQL_C_NUMERIC
	 *
	 * [1] If the ODBC-Version is <= 2.x, only the SQL_C_DATE, SQL_C_TIME and SQL_C_TIMESTAMP types are used.
	 *	   If the ODBC-Version is >= 3.x, the old (like SQL_C_DATE) and the new (like SQL_C_TYPE_DATE) types
	 *	   are mapped to the new ODBC 3.x types.
	 * 
	 * [2] The SQL_TIME2 is a Microsoft SQL Server specific extension. It is only available
	 * if HAVE_MSODBCSQL_H is defined to 1. If HAVE_MSODBCSQL_H
	 * is not set to 1, the SQL-Type SQL_TIME2 is not supported. If HAVE_MSODBC_SQL_H is defined to 1 and
	 * the ODBC version is >= 3.8, SQL_SS_TIME2 is mapped to a SQL_C_SS_TIME2, else it is mapped
	 * to an SQL_C_TYPE_TIME.
	 */
	class EXODBCAPI DefaultSql2BufferMap
		: public Sql2BufferTypeMap
	{
	public:
		/*!
		* \brief		Create new Default Mapping.
		* \param odbcVersion Depending on OdbcVersion passed different bindings are registered, see DefaultSql2BufferMap.
		*/
		DefaultSql2BufferMap(OdbcVersion odbcVersion);
	};


	/*!
	* \class WCharSql2BufferMap
	*
	* \brief Registers no types but sets a Default to SQL_C_WCHAR.
	* \details All SQL Types will try to bind to a SQL_C_WCHAR.
	*
	*/
	class EXODBCAPI WCharSql2BufferMap
		: public Sql2BufferTypeMap
	{
	public:

		/*!
		* \brief Create new WCharSql2BufferMap.
		*/
		WCharSql2BufferMap();
	};


	/*!
	* \class CharSql2BufferMap
	*
	* \brief Registers no types but sets a Default to SQL_C_CHAR.
	* \details All SQL Types will try to bind to a SQL_C_CHAR.
	*
	*/
	class EXODBCAPI CharSql2BufferMap
		: public Sql2BufferTypeMap
	{
	public:

		/*!
		* \brief Create new CharSql2BufferMap.
		*/
		CharSql2BufferMap();
	};


	/*!
	* \class CharAsWCharSql2BufferMap
	*
	* \brief Extends the DefaultSql2BufferMap and binds CHAR fields as SQL_C_WCHAR too.
	* \details As DefaultSql2BufferMap, except SQL_CHAR and SQL_VARCHAR are bound to SQL_C_WCHAR.
	*
	*/
	class EXODBCAPI CharAsWCharSql2BufferMap
		: public DefaultSql2BufferMap
	{
	public:
		
		/*!
		* \brief Create new CharAsWCharSql2BufferMap.
		*/
		CharAsWCharSql2BufferMap(OdbcVersion ocbcVersion);
	};


	/*!
	* \class WCharAsCharSql2BufferMap
	*
	* \brief Extends the DefaultSql2BufferMap and binds WCHAR fields as SQL_C_CHAR too.
	* \details As DefaultSql2BufferMap, except SQL_WCHAR and SQL_WVARCHAR are bound to SQL_C_CHAR.
	*
	*/
	class EXODBCAPI WCharAsCharSql2BufferMap
		: public DefaultSql2BufferMap
	{
	public:

		/*!
		* \brief Create new WCharAsCharSql2BufferMap.
		*/
		WCharAsCharSql2BufferMap(OdbcVersion ocbcVersion);
	};
}

#endif // SQL2BUFFERMAP_H