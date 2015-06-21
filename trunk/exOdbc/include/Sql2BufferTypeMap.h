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
		* \throw Exception		If SQL Type is not registered and no Default Type is set.
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

	
	class EXODBCAPI DefaultSql2BufferMap
		: public Sql2BufferTypeMap
	{
	public:
		DefaultSql2BufferMap();
	};
}

#endif // SQL2BUFFERMAP_H