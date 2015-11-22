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
}
