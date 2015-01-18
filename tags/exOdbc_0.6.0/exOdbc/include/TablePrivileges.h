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

	enum TablePrivilege
	{
		TP_SELECT = 0x1,
		TP_INSERT = 0x2,
		TP_UPDATE = 0x4,
		TP_DELETE = 0x8,
	};
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
		/*!
		* \brief	Create an empty TablePrivileges with no TablePrivilege set.
		*/
		TablePrivileges();


		/*!
		* \brief	Create a TablePrivilege for the Table given by tableInfo.
		* \detailed	Tries to Initialize() this TablePrivileges automatically.
		* \see		IsInitialized()
		*/
		TablePrivileges(Database* pDb, const STableInfo& tableInfo);
		
		
		~TablePrivileges() {};


		/*!
		* \brief	Query database about the Privileges of the passed Table.
		*			Marks object as initialized on success.
		* \return	True on success.
		*/
		bool Initialize(Database* pDb, const STableInfo& tableInfo);


		/*!
		* \brief	Initialize object from passed data. Marks as uninitialized first.
		*			Marks object as initialized on success.
		* \param	tablePrivs TablePrivilige s
		* \return	True on success.
		*/
		bool Initialize(const TablePrivilegesVector& tablePrivs) { return Parse(tablePrivs); };


		/*!
		* \brief	Returns true if Privileges have been parsed and can be queried.
		*/
		bool IsInitialized() const { return m_initialized; };


		/*!
		* \brief	Test if multiple TablePrivileges are set.
		*/
		bool AreSet(unsigned int priv) const { return (m_privileges & priv) != 0; };
		

		/*!
		* \brief	Test if a TablePrivilege is set.
		*/
		bool IsSet(TablePrivilege priv) const { return (m_privileges & priv) != 0; };

	private:

		bool Parse(const TablePrivilegesVector& tablePrivs);

		bool m_initialized;

		unsigned int m_privileges;
	};

} // namesapce exodbc

#endif // TABLE_PRIVILEGES_H