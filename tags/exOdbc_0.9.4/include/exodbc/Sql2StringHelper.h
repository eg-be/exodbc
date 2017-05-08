/*!
 * \file   Sql2StringHelper.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 09.02.2014
 * \brief Header file for Sql2StringHelper
 * \copyright GNU Lesser General Public License Version 3
*/ 

#pragma once

// Same component headers
#include "exOdbc.h"

// Other headers
// System headers
#include <string>

// Forward declarations
// --------------------



namespace exodbc
{
	/*!
	* \class Sql2StringHelper
	* \brief Some static helper to convert SQL types, values etc. to a string.
	*/
	class Sql2StringHelper
	{
	public:
		/*!
		* \brief Returns the string TRUE, FALSE or ????? for the values SQL_TRUE, SQL_FALSE or anything else.
		*
		* \param b SQL_TRUE or SQL_FALSE
		* \return std::string TRUE, FALSE or ?????
		*/
		static std::string SqlTrueFalse2s(SQLSMALLINT b) noexcept;


		/*!
		* \brief Translates some often encountered SQLRETURN values to a string.
		*
		* \param ret Return code to translate.
		* \return std::string Translation or '???' if unknown.
		*/
		static std::string SqlReturn2s(SQLRETURN ret) noexcept;


		/*!
		* \brief Transform the SQL_types like SQL_CHAR, SQL_NUMERIC, etc. to some string.
		*
		* \param sqlType SQL Type..
		* \return std::string
		*/
		static std::string SqlType2s(SQLSMALLINT sqlType) noexcept;


		/*!
		* \brief Transform the SQL C-types like SQL_C_SLONG, SQL_C_WCHAR, etc. to some string, like "SQL_C_SLONG", etc.
		*
		* \param sqlCType Sql-C Type..
		* \return std::string
		*/
		static std::string SqLCType2s(SQLSMALLINT sqlCType) noexcept;


		/*!
		* \brief	Transform the SQL C-types like SQL_C_SLONG, SQL_C_WCHAR, etc. to the corresponding string value of the ODBC C Type.
		*			Like SQL_C_SLONG becomes "SQLINTEGER"
		*
		* \param sqlCType Sql-C Type.
		* \return std::string
		*/
		static std::string SqlCType2Odbcs(SQLSMALLINT sqlCType) noexcept;


		/*!
		* \brief	Return "No Null" for SQL_NO_NULLS, "Nullable" for SQL_NULLABLE or "Nullable Unknown" for SQL_NULLABLE_UNKNOWN.
		*/
		static std::string SqlNullable2s(SQLSMALLINT nullable) noexcept;


		/*!
		* \brief Return PRED_NONE, PRED_CHAR, PRED_BASIC or SEARCHABLE for corresponding SQL_ values.
		*/
		static std::string SqlSearchable2s(SQLSMALLINT searchable) noexcept;


		/*!
		* \brief	Transform a DatabaseProduct id to a name:
		* \details
		*  DatabaseProduct			| Value
		*  -------------------------|------------
		*  MS_SQL_SERVER			| SqlServer
		*  MY_SQL					| MySql
		*  DB2						| DB2
		*  EXCEL					| Excel
		*  ACCESS					| Access
		*
		* \param dbms
		* \return std::string
		*/
		static std::string DatabaseProcudt2s(DatabaseProduct dbms) noexcept;


		/*!
		* \brief Returns ENV, DBC, STMT, DESC or ???
		*/
		static std::string HandleType2s(SQLSMALLINT type) noexcept;

	};
}


