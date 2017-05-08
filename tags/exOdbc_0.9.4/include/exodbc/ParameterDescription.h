/*!
* \file ParameterDescription.h
* \author Elias Gerber <eg@elisium.ch>
* \date 04.05.2017
* \brief Header file for ParameterDescription.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlHandle.h"

// Other headers
// System headers
#include <string>
#include <vector>

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class ParameterDescription
	* \brief Holds the description of a parameter fetched using SQLDescribeParam.
	*/
	class EXODBCAPI ParameterDescription
	{
	public:
		/*!
		* \brief Default constructor, all members are set to empty, 0 or unknown values.
		*/
		ParameterDescription()
			: m_sqlType(SQL_UNKNOWN_TYPE)
			, m_charSize(0)
			, m_decimalDigits(0)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};


		/*!
		* \brief Use passed values to init members.
		*/
		ParameterDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits, SQLSMALLINT paramNullable)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(paramNullable)
		{};


		/*!
		* \brief Use passed values to init members, set nullable to SQL_NULLABLE_UNKNOWN
		*/
		ParameterDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};


		/*!
		* \brief Create a new ParameterDescription by querying the passed statement for passed paramNr.
		*/
		ParameterDescription(ConstSqlStmtHandlePtr pStmt, SQLUSMALLINT paramNr);


		/*!
		* \return SQL Type.
		*/
		SQLSMALLINT GetSqlType() const noexcept { return m_sqlType; };


		/*!
		* \return Char Size.
		*/
		SQLULEN GetCharSize() const noexcept { return m_charSize; };


		/*!
		* \return Decimal Digits.
		*/
		SQLSMALLINT GetDecimalDigits() const noexcept { return m_decimalDigits; };


		/*!
		* \return Nullable.
		*/
		SQLSMALLINT GetNullable() const noexcept { return m_nullable; };


	private:
		SQLSMALLINT m_sqlType;
		SQLULEN m_charSize;
		SQLSMALLINT m_decimalDigits;
		SQLSMALLINT m_nullable;
	};
}
