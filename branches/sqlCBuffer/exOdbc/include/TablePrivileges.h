/*!
* \file TablePrivileges.h
* \author Elias Gerber <eg@elisium.ch>
* \date 31.12.2014
* \brief Header file for the TablePrivileges class.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once
#ifndef TABLE_PRIVILEGES_H
#define TABLE_PRIVILEGES_H

// Same component headers
#include "exOdbc.h"
#include "InfoObject.h"
#include "EnumFlags.h"
#include "Database.h"

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

	enum TablePrivilegeOld
	{
		TP_SELECT = 0x1,
		TP_INSERT = 0x2,
		TP_UPDATE = 0x4,
		TP_DELETE = 0x8,
	};
	// Classes
	// -------
	enum class TablePrivilege
	{
		NONE = 0x0,

		SELECT = 0x1,
		UPDATE = 0x2,
		INSERT = 0x4,
		DEL = 0x8
	};
	template<>
	struct enable_bitmask_operators<TablePrivilege> {
		static const bool enable = true;
	};


	/*!
	* \class TablePrivileges
	*
	* \brief Parses Table Privileges read from the database and caches them for later use.
	*
	*/
	class TablePrivileges
		: public EnumFlags<TablePrivilege>
	{
	public:
		/*!
		* \brief	Create an empty TablePrivileges with no TablePrivilege set.
		*/
		TablePrivileges()
			: EnumFlags()
		{};

		/*!
		* \brief	Query database about the Privileges of the passed Table.
		*			Overrides any privileges set with the values read from database.
		* \throw	Exception If querying or parsing fails.
		*/
		void Init(ConstDatabasePtr pDb, const TableInfo& tableInfo);


		/*!
		* \brief	Overrides privileges with passed values.
		* \param	tablePrivs
		* \throw	Exception if Parsing passed data fails.
		*/
		void Init(const TablePrivilegesVector& tablePrivs);
	};

} // namesapce exodbc

#endif // TABLE_PRIVILEGES_H