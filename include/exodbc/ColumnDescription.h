/*!
* \file ColumnDescription.h
* \author Elias Gerber <eg@elisium.ch>
* \date 04.05.2017
* \brief Header file for ColumnDescription.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlHandle.h"
#include "SqlInfoProperty.h"

// Other headers
// System headers
#include <string>
#include <vector>

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class ColumnDescription
	* \brief	Result of SQLDescribeCol operation.
	*/
	class EXODBCAPI ColumnDescription
	{
	public:
		/*!
		* \brief Default constructor, all members are set to empty, 0 or unknown values.
		*/
		ColumnDescription()
			: m_sqlType(SQL_UNKNOWN_TYPE)
			, m_charSize(0)
			, m_decimalDigits(0)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};


		/*!
		* \brief Use passed values to init members.
		*/
		ColumnDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits, SQLSMALLINT paramNullable)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(paramNullable)
		{};


		/*!
		* \brief Use passed values to init members, set nullable to SQL_NULLABLE_UNKNOWN
		*/
		ColumnDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};


		/*!
		* \brief Create a new ColumnDescription by querying the passed statement for passed columnNr.
		*/
		ColumnDescription(ConstSqlStmtHandlePtr pStmt, SQLUSMALLINT columnNr, const SqlInfoProperties& props);


		/*!
		* \return Column Name.
		*/
		std::string GetName() const noexcept { return m_name; };


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
		std::string m_name;
		SQLSMALLINT m_sqlType;
		SQLULEN m_charSize;
		SQLSMALLINT m_decimalDigits;
		SQLSMALLINT m_nullable;
	};
}
