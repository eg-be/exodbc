#if ENABLE_SQL_SERVER_TYPES

/*!
* \file SqlServerTypes.h
* \author Elias Gerber <eg@elisium.ch>
* \date 01.10.2015
* \brief Header file for the Extended Types of Microsoft SQL Server.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once
#ifndef SQL_SERVER_TYPES_H
#define SQL_SERVER_TYPES_H

// Same component headers
#include "exOdbc.h"
#include "ExtendedType.h"

// Other headers
// System headers
#include <set>

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

	/*!
	* \class SqlServerTypes
	*
	* \brief Adds support for types special to Microsoft SQL Server.
	* \details This is for the moment only the TIME2 type.
	*
	*/
	class EXODBCAPI SqlServerTypes
		: public ExtendedType
	{

	};

} // namespace exodbc

#endif // SQL_SERVER_TYPES_H

#endif