/*!
* \file ObjectName.h
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2014
* \brief Header file for name objects.
* \copyright wxWindows Library Licence, Version 3.1
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
	* \brief Name of an Object.
	*/
	class EXODBCAPI ObjectName
	{
	public:
		virtual ~ObjectName();

		virtual std::wstring GetQueryName() const = 0;
		virtual std::wstring GetPureName() const = 0;

		bool operator==(const ObjectName& other) const;
	};
}

#endif // OBJECTNAME_H