/*!
* \file ObjectName.h
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2014
* \brief Header file for name objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once
#ifndef OBJECTNAME_H
#define OBJECTNAME_H

// Same component headers
#include "exOdbc.h"

// Other headers

// System headers
#include <string>

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class ObjectName
	*
	* \brief Name of a Database Object like a Table, Column, etc.
	*/
	class EXODBCAPI ObjectName
	{
	public:
		virtual ~ObjectName();

		/*!
		* \brief Get the name of the Object to be used in Queries.
		*/
		virtual std::wstring GetQueryName() const = 0;
		
		
		/*!
		* \brief Get the pure name of the Object. Like only the
		*		 table name for a table (without schema or catalog), etc.
		*/
		virtual std::wstring GetPureName() const = 0;


		/*!
		* \brief Compare two ObjectName for equality. Default implementation
		*		 will compare using GetQueryName().
		*/
		bool operator==(const ObjectName& other) const;
	};


	class EXODBCAPI BindInfo
	{
	public:
		virtual ~BindInfo();


	};
}

#endif // OBJECTNAME_H