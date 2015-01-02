/*!
* \file TablePrimaryKeys.h
* \author Elias Gerber <eg@zame.ch>
* \date 02.01.2015
* \brief Header file for the TablePrimaryKeys class.
*/

#pragma once
#ifndef TABLE_PRIMARY_KEYS_H
#define TABLE_PRIMARY_KEYS_H

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
	* \class TablePrimaryKeys
	*
	* \brief Parses PrimaryKeys read from the database and caches them for later use.
	*
	*/
	class EXODBCAPI TablePrimaryKeys
	{
	public:
		/*!
		* \brief	Create an empty TablePrimaryKeys.
		*/
		TablePrimaryKeys();


		/*!
		* \brief	Create a TablePrimaryKeys for the Table given by tableInfo.
		* \detailed	Tries to Initialize() this TablePrimaryKeys automatically.
		* \see		IsInitialized()
		*/
		TablePrimaryKeys(Database* pDb, const STableInfo& tableInfo);


		~TablePrimaryKeys() {};


		/*!
		* \brief	Query database about the Primary Keys of the passed Table.
		*			Marks object as initialized on success.
		* \return	True on success.
		*/
		bool Initialize(Database* pDb, const STableInfo& tableInfo);


		/*!
		* \brief	Initialize object from passed data. Marks as uninitialized first.
		*			Marks object as initialized on success.
		* \param	tablePks table primary keys
		* \return	True on success.
		*/
		bool Initialize(const TablePrimaryKeysVector& tablePks) { return Parse(tablePks); };


		/*!
		* \brief	Returns true if Primary Keys have been parsed and can be queried.
		*/
		bool IsInitialized() const { return m_initialized; };


	private:

		bool Parse(const TablePrimaryKeysVector& tablePks);

		bool m_initialized;
	};

} // namesapce exodbc

#endif // TABLE_PRIMARY_KEYS_H