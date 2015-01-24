/*!
* \file TablePrimaryKeys.h
* \author Elias Gerber <eg@zame.ch>
* \date 02.01.2015
* \brief Header file for the TablePrimaryKeys class.
* \copyright wxWindows Library Licence, Version 3.1
*/

#pragma once
#ifndef TABLE_PRIMARY_KEYS_H
#define TABLE_PRIMARY_KEYS_H

// Same component headers
#include "exOdbc.h"
#include "ColumnBuffer.h"

// Other headers
// System headers
#include <map>

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
		*			Note that this will not fail if tablePks is empty.
		* \param	tablePks table primary keys
		* \return	True on success.
		*/
		bool Initialize(const TablePrimaryKeysVector& tablePks) { return Parse(tablePks); };


		/*!
		* \brief	Returns true if Primary Keys have been parsed and can be queried.
		*/
		bool IsInitialized() const { return m_initialized; };


		bool IsPrimaryKey(const std::wstring queryName) const;


		bool AreAllPrimaryKeysBound(const ColumnBufferPtrMap& columnBuffers) const;


		bool SetPrimaryKeyFlag(const ColumnBufferPtrMap& columnBuffers) const;


		size_t GetPrimaryKeysCount() const { exASSERT(m_initialized); return m_pksVector.size(); };


		const TablePrimaryKeysVector& GetPrimaryKeysVector() const { exASSERT(m_initialized); return m_pksVector; };

	private:

		bool Parse(const TablePrimaryKeysVector& tablePks);

		bool m_initialized;

		typedef std::map<std::wstring, STablePrimaryKeyInfo> PrimaryKeysMap;
		PrimaryKeysMap m_pksMap;
		TablePrimaryKeysVector m_pksVector;
	};

} // namesapce exodbc

#endif // TABLE_PRIMARY_KEYS_H