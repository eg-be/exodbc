/*!
* \file TablePrivileges.h
* \author Elias Gerber <eg@zame.ch>
* \date 31.12.2014
* \brief Header file for the TablePrivileges class.
*/

#pragma once
#ifndef TABLE_PRIVILEGES_H
#define TABLE_PRIVILEGES_H

// Same component headers
#include "exOdbc.h"

// Other headers
// System headers

// Forward declarations
// --------------------
namespace exodbc
{
	class Database;
}

namespace exodbc
{
	// Consts
	// ------

	// Structs
	// -------

	// Classes
	// -------

	/*!
	* \class TablePrivileges
	*
	* \brief Parses Table Privileges read from the database and caches them for later use.
	*
	*/
	class EXODBCAPI TablePrivileges
	{
	public:
		TablePrivileges();
		TablePrivileges(Database* pDb, const STableInfo& tableInfo);
		~TablePrivileges() {};

	private:
		bool Initialize(Database* pDb, const STableInfo& tableInfo);

		bool Initialize(const TablePrivilegesVector& tablePrivs);
	};

} // namesapce exodbc

#endif // TABLE_PRIVILEGES_H