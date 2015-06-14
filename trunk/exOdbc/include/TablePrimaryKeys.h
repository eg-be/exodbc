/*!
* \file TablePrimaryKeys.h
* \author Elias Gerber <eg@elisium.ch>
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
		TablePrimaryKeys(const Database* pDb, const TableInfo& tableInfo);


		~TablePrimaryKeys() {};


		/*!
		* \brief	Query database about the Primary Keys of the passed Table.
		*			Marks object as initialized on success.
		* \throw	If querying fails or parsing fails.
		*/
		void Initialize(const Database* pDb, const TableInfo& tableInfo);


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
		*			The pure table-name from the passed TableInfo is used as tablename for the entries created.
		*			Note that this will not fail if columnBuffers is empty or has no columns marked as primary keys.
		* \param	tablePks table primary keys
		* \throw	Exception If parsing fails.
		*/
		void Initialize(const TableInfo& tableInfo, const ColumnBufferPtrMap& columnBuffers) { m_initialized = false; Parse(tableInfo, columnBuffers); m_initialized = true; };


		/*!
		* \brief	Returns true if Primary Keys have been parsed and can be queried.
		*/
		bool IsInitialized() const { return m_initialized; };


		/*!
		* \brief	Compare if the last part of the passed queryName and matches one of the key names.
		* \details	Uses the WeakCompare()
		*/
		bool IsPrimaryKey(const std::wstring queryName) const;


		/*!
		* \brief	Returns true if for every PrimaryKey there is a matching columnBuffer that is Bound.
		* \details	Uses the WeakCompare() to determine the matching ColumnBuffer.
		*/
		bool AreAllPrimaryKeysBound(const ColumnBufferPtrMap& columnBuffers) const;


		/*!
		* \brief	Returns true if for every PrimaryKey there is a matching columnBuffer.
		* \details	Uses the WeakCompare() to determine the matching ColumnBuffer.
		*/
		bool AreAllPrimaryKeysInMap(const ColumnBufferPtrMap& columnBuffers) const;


		/*!
		* \brief	Seths the CF_PRIMARY_KEY flag on every ColumnBuffer that machtes one of the PrimaryKeys.
		* \details	Uses the WeakCompare() to determine the matching ColumnBuffer.
		*/
		void SetPrimaryKeyFlags(const ColumnBufferPtrMap& columnBuffers) const;


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
		*			The pure table-name from the passed TableInfo is used as tablename for the entries created.
		* \throw	Exception
		*/
		void Parse(const TableInfo& tableInfo, const ColumnBufferPtrMap& columnBuffers);


		/*!
		* \brief	Tokenizes the two passed strings using '.' as tokenizer. Compares only the last two
		*			tokens found for equality.
		*/
		bool WeakCompare(std::wstring name1, std::wstring name2) const;

		bool m_initialized;

		TablePrimaryKeysVector m_pksVector;
	};

} // namesapce exodbc

#endif // TABLE_PRIMARY_KEYS_H