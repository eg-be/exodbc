/*!
* \file GenericCBufferType.h
* \author Elias Gerber <eg@elisium.ch>
* \date 01.10.2015
* \brief Header file for the GenericCBufferType class.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "CBufferType.h"

// Other headers
// System headers

// Forward declarations
// --------------------

namespace exodbc
{
	// Consts
	// ------

	// Structs
	// -------

	// Classes
	// -------

	class EXODBCAPI GenericCBufferType
		: public CBufferType
	{
	public:
		GenericCBufferType();

		/*!
		* param nrOfElements	If sqlCType is SQL_C_CHAR or SQL_C_BINARY, nrOfElements is
		*						the number of SQLCHAR elements to allocate. If sqlCType 
		*						is SQL_C_WCHAR, nrOfElements is the number of SQLWCHAR elements
		*						to allocate.
		*						In all other cases nrOfElements is ignored.
		*/
		GenericCBufferType(SQLSMALLINT sqlCType, SQLLEN nrOfElements);

	public:
		const static std::set<SQLSMALLINT> s_supportedCBufferTypes;

		virtual ~GenericCBufferType();

		virtual CBufferType* CreateInstance(SQLSMALLINT sqlCType, SQLLEN nrOfElements) const;

		virtual std::set<SQLSMALLINT> GetSupportedSqlCTypes() const;

		virtual bool IsSqlCTypeSupported(SQLSMALLINT sqlCType) const;

		virtual SQLSMALLINT GetSmallInt() const;

		virtual void SetSmallInt(SQLSMALLINT value);

	private:
		SQLPOINTER m_pBuffer;
		SQLLEN m_bufferSize;
		SQLSMALLINT m_bufferSqlCType;
	};

} // namespace exodbc
