/*!
* \file EnumFlags.h
* \author Elias Gerber <eg@elisium.ch>
* \date 21.11.2015
* \brief Header file for info objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "bitmask_operators.hpp"

// Other headers

// System headers

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class EnumFlags
	* \brief Wrapper around a enum class defining flags.
	*/
	template<typename ET, typename std::enable_if<std::is_enum<ET>::value, void>::type* = 0>
	class EnumFlags
	{
	public:
		EnumFlags()
		{
			m_flags = static_cast<ET>(0);
		};

		EnumFlags(ET flags)
			: m_flags(flags)
		{};

		virtual ~EnumFlags() 
		{};

		EnumFlags(const EnumFlags& other) = default;
		EnumFlags& operator=(const EnumFlags& other) = default;

		bool Test(ET flag) const noexcept
		{
			return (m_flags & flag) == flag;
		};

		void Set(ET flag) noexcept
		{
			m_flags |= flag;
		};

		void Clear(ET flag) noexcept
		{
			m_flags &= ~flag;
		};

		bool operator==(const EnumFlags& other) const noexcept
		{
			return m_flags == other.m_flags;
		}

	private:
		ET m_flags;
	};

	/*!
	* \enum ColumnFlag
	* \brief Define flags of a Column.
	*/
	enum class ColumnFlag
	{
		NONE = 0x0,		///< No flags.

		SELECT = 0x1,	///< Include Column in Selects.
		UPDATE = 0x2,	///< Include Column in Updates.
		INSERT = 0x4,	///< Include Column in Inserts.
		NULLABLE = 0x8,	///< Column is null able.
		PRIMARY_KEY = 0x10,	///< Column is primary key.

		READ = SELECT,	///< SELECT
		WRITE = UPDATE | INSERT,	///< UPDATE | INSERT
		READ_WRITE = SELECT | UPDATE | INSERT	///< SELECT | UPDATE | INSERT
	};
	template<>
	struct enable_bitmask_operators<ColumnFlag> {
		static const bool enable = true;
	};

	typedef EnumFlags<ColumnFlag> ColumnFlags;


	/*!
	* \enum AccessFlag
	* \brief Defines how to Access a table.
	*/
	enum class TableAccessFlag
	{
		AF_NONE = 0x0,			///< No AccessFlags, no statements are going to be created.

		AF_SELECT = 0x1,		///< Access for SELECTing.

		AF_UPDATE_PK = 0x2,		///< Access for UPDATEing where rows to update are identified by the bound primary key value(s).
		AF_UPDATE_WHERE = 0x4,	///< Access for UPDATEing where rows to update are identified using a manually passed where clause.
		AF_UPDATE = AF_UPDATE_PK | AF_UPDATE_WHERE,	///< AF_UPDATE_PK | AF_UPDATE_WHERE

		AF_INSERT = 0x8,		///< Access for INSERTing.

		AF_DELETE_PK = 0x10,	///< Access for DELETEing where rows to delete are identified by the bound primary key value(s).
		AF_DELETE_WHERE = 0x20,	///< Access for DELETEing where rows to delete are identified using a manually passed where clause.
		AF_DELETE = AF_DELETE_PK | AF_DELETE_WHERE,	///< AF_DELETE_PK | AF_DELETE_WHERE

		AF_READ = AF_SELECT,	///< AF_SELECT
		AF_WRITE = AF_UPDATE | AF_INSERT | AF_DELETE,	///<AF_UPDATE | AF_INSERT | AF_DELETE
		AF_READ_WRITE = AF_READ | AF_WRITE	///< AF_READ | AF_WRITE
	};
	template<>
	struct enable_bitmask_operators<TableAccessFlag> {
		static const bool enable = true;
	};

	typedef EnumFlags<TableAccessFlag> TableAccessFlags;
}
