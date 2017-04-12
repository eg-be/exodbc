/*!
* \file SqlInfo.h
* \author Elias Gerber <eg@elisium.ch>
* \date 12.04.2017
* \brief Header file for the SqlInfo class.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"

// Other headers
#include <boost/variant.hpp>
// System headers

// Forward declarations
// --------------------

namespace exodbc
{
	// Consts
	// ------

	// Structs
	// -------

	// Classes
	// -------
	class SqlInfo
	{
	public:
		enum class InfoType
		{
			Driver,
			DBMS,
			DataSource,
			SupportedSql,
			SqlLimits,
			ScalarFunction,
			Conversion
		};

		enum class ValueType
		{
			USmallInt,
			UInt,
			String
		};

		typedef boost::variant<
			SQLUSMALLINT,
			SQLUINTEGER,
			std::string
		> Value;

		SqlInfo(InfoType type, Value v)
			: m_type(type)
			, m_value(v)
		{};

		ValueType GetValueType() const noexcept;
		InfoType GetInfoType() const noexcept { return m_type; };
		Value GetValue() const noexcept { return m_value; };

	private:
		InfoType m_type;
		Value m_value;
	};


} // namespace exodbc
