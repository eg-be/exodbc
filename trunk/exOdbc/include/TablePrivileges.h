/*!
* \file TablePrivileges.h
* \author Elias Gerber <eg@elisium.ch>
* \date 31.12.2014
* \brief Header file for the TablePrivileges class.
* \copyright wxWindows Library Licence, Version 3.1
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
		* \details	Tries to Initialize() this TablePrivileges automatically.
		* \throw	Exception If Initialization fails.
		*/
		TablePrivileges(const Database* pDb, const STableInfo& tableInfo);
		
		
		~TablePrivileges() {};


		/*!
		* \brief	Query database about the Privileges of the passed Table.
		*			Marks object as initialized on success.
		* \throw	Exception If querying or parsing fails.
		*/
		void Initialize(const Database* pDb, const STableInfo& tableInfo);


		/*!
		* \brief	Initialize object from passed data. Marks as uninitialized first.
		*			Marks object as initialized on success.
		* \param	tablePrivs TablePrivilige s
		* \throw	Exception if Parsing passed data fails.
		*/
		void Initialize(const TablePrivilegesVector& tablePrivs) { return Parse(tablePrivs); };


		/*!
		* \brief	Returns true if Privileges have been parsed and can be queried.
		*/
		bool IsInitialized() const { return m_initialized; };


		/*!
		* \brief	Test if multiple TablePrivileges are set.
		* \throw	Exception If not initialized.
		*/
		bool AreSet(unsigned int priv) const { exASSERT(IsInitialized());  return (m_privileges & priv) != 0; };
		

		/*!
		* \brief	Test if a TablePrivilege is set.
		* \throw	Exception If not initialized.
		*/
		bool IsSet(TablePrivilege priv) const { exASSERT(IsInitialized());  return (m_privileges & priv) != 0; };

	private:
		/*!
		* \brief	Parses to TablePrivilege.
		* \throw	Exception
		*/
		void Parse(const TablePrivilegesVector& tablePrivs);

		bool m_initialized;

		unsigned int m_privileges;
	};

} // namesapce exodbc

#endif // TABLE_PRIVILEGES_H