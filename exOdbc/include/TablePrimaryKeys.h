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
		* \details	Tries to Initialize() this TablePrimaryKeys automatically.
		* \see		IsInitialized()
		* \throw	Exception If querying primary keys fails.
		*/
		TablePrimaryKeys(const Database& db, const STableInfo& tableInfo);


		~TablePrimaryKeys() {};


		/*!
		* \brief	Query database about the Primary Keys of the passed Table.
		*			Marks object as initialized on success.
		* \throw	If querying fails or parsing fails.
		*/
		void Initialize(const Database& db, const STableInfo& tableInfo);


		/*!
		* \brief	Initialize object from passed data. Marks as uninitialized first.
		*			Marks object as initialized on success.
		*			Note that this will not fail if tablePks is empty.
		* \param	tablePks table primary keys
		* \throw	Exception If parsing fails.
		*/
		void Initialize(const TablePrimaryKeysVector& tablePks) { m_initialized = false; Parse(tablePks); m_initialized = true; };


		/*!
		* \brief	Initialize object from passed data. Marks as uninitialized first.
		*			Marks object as initialized on success.
		*			An entry is created for every ColumnBuffer that has the flag CF_PRIMARY_KEY set.
		*			The ColunmnBuffers query name is used as column-name, but only the part after the last '.'.
		*			The pure table-name from the passed STableInfo is used as tablename for the entries created.
		*			Note that this will not fail if columnBuffers is empty or has no columns marked as primary keys.
		* \param	tablePks table primary keys
		* \throw	Exception If parsing fails.
		*/
		void Initialize(const STableInfo& tableInfo, const ColumnBufferPtrMap& columnBuffers) { m_initialized = false; Parse(tableInfo, columnBuffers); m_initialized = true; };


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
		/*!
		* \brief	Parses - will just fill the map and vector of primary keys with the passed values.
		* \throw	Exception
		*/
		void Parse(const TablePrimaryKeysVector& tablePks);

		/*!
		* \brief	Fills the map and vector of primary keys using the passed ColumnBufferPtrMap.
		* \details	An entry is created for every ColumnBuffer that has the flag CF_PRIMARY_KEY set.
		*			The ColunmnBuffers query name is used as column-name, but only the part after the last '.'.
		*			The pure table-name from the passed STableInfo is used as tablename for the entries created.
		* \throw	Exception
		*/
		void TablePrimaryKeys::Parse(const STableInfo& tableInfo, const ColumnBufferPtrMap& columnBuffers);

		bool m_initialized;

		typedef std::map<std::wstring, STablePrimaryKeyInfo> PrimaryKeysMap;
		PrimaryKeysMap m_pksMap;
		TablePrimaryKeysVector m_pksVector;
	};

} // namesapce exodbc

#endif // TABLE_PRIMARY_KEYS_H