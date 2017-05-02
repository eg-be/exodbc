/*!
* \file SqlTypeInfo.h
* \author Elias Gerber <eg@elisium.ch>
* \date 01.05.2017
* \brief Header file for SqlTypeInfo.
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
	* \struct	SqlTypeInfo
	* \brief	Contains DataType informations read using catalog function SQLGetTypeInfo.
	* \see http://msdn.microsoft.com/en-us/library/ms714632%28v=vs.85%29.aspx
	* \see	DatabaseCatalog::ReadSqlTypeInfo()
	*/
	class EXODBCAPI SqlTypeInfo
	{
	public:
		/*!
		* \brief Init all members with empty or 0 values. All null flags are set to true.
		*/
		SqlTypeInfo();


		/*!
		* \brief Create from a statement that is assumed to hold the results of SQLGetTypeInfo. The cursor must
		*		be positioned at the row and is not modified, but column values are read.
		* \throw Exception If reading any value fails, or if props does not hold all required properties.
		*/
		SqlTypeInfo(ConstSqlStmtHandlePtr pStmt, const SqlInfoProperties& props);


		/*!
		* \return SQL Data Type (ODBC 3.0).
		*/
		SQLSMALLINT GetSqlDataType() const noexcept { return m_sqlDataType; };


		/*!
		* \return SQL Type.
		*/
		SQLSMALLINT GetSqlType() const noexcept { return m_sqlType; };


		std::string ToOneLineStrForTrac(bool withHeaderLine /* = false */) const;
		std::string ToOneLineStr(bool withHeaderLines = false, bool withEndLine = false) const;
		std::string ToStr() const;

		bool operator<(const SqlTypeInfo& other) const;

	private:
		std::string	m_typeName;					///<  1 Data source dependent data-type name
		SQLSMALLINT		m_sqlType;					///<  2 SQL data type. This can be an ODBC SQL data type or a driver-specific SQL data type.
		SQLINTEGER		m_columnSize;				///<  3 [NULLABLE] The maximum column size that the server supports for this data type. For numeric data, this is the maximum precision. For string data, this is the length in characters. For datetime data types, this is the length in characters of the string representation (assuming the maximum allowed precision of the fractional seconds component). NULL is returned for data types where column size is not applicable.
		bool			m_columnSizeIsNull;			///<  3 See SqlTypeInfo::m_columnSize
		std::string	m_literalPrefix;			///<  4 [NULLABLE] Character or characters used to prefix a literal; for example, a single quotation mark (') for character data types or 0x for binary data types
		bool			m_literalPrefixIsNull;		///<  4 See SqlTypeInfo::m_literalPrefix
		std::string	m_literalSuffix;			///<  5 [NULLABLE] Character or characters used to terminate a literal; for example, a single quotation mark (') for character data types;
		bool			m_literalSuffixIsNull;		///<  5 See SqlTypeInfo::m_literalSuffix
		std::string	m_createParams;				///<  6 [NULLABLE] A list of keywords, separated by commas, corresponding to each parameter that the application may specify in parentheses when using the name that is returned in the TYPE_NAME field.
		bool			m_createParamsIsNull;		///<  6 See SqlTypeInfo::m_createParams
		SQLSMALLINT		m_nullable;					///<  7 Whether the data type accepts a NULL value: SQL_NO_NULLS, SQL_NULLABLE or	SQL_NULLABLE_UNKNOWN.
		SQLSMALLINT		m_caseSensitive;			///<  8 Whether a character data type is case-sensitive in collations and comparisons: SQL_TRUE or SQL_FALSE
		SQLSMALLINT		m_searchable;				///<  9 How the data type is used in a WHERE clause: SQL_PRED_NONE (no use), SQL_PRED_CHAR (only with LIKE), SQL_PRED_BASIC (all except LIKE), SQL_SEARCHABLE (anything)
		SQLSMALLINT		m_unsigned;					///< 10 [NULLABLE] Whether the data type is unsigned: SQL_TRUE or SQL_FALSE
		bool			m_unsignedIsNull;			///< 10 See SqlTypeInfo::m_unsigned
		SQLSMALLINT		m_fixedPrecisionScale;		///< 11 Whether the data type has predefined fixed precision and scale (which are data source–specific), such as a money data type: SQL_TRUE or SQL_FALSE
		SQLSMALLINT		m_autoUniqueValue;			///< 12 [NULLABLE] Whether the data type is autoincrementing: SQL_TRUE or SQL_FALSE
		bool			m_autoUniqueValueIsNull;	///< 12 See SqlTypeInfo::m_autoUniqueValue
		std::string	m_localTypeName;			///< 13 [NULLABLE] localized version of the data source–dependent name of the data type.
		bool			m_localTypeNameIsNull;		///< 13 See SqlTypeInfo::m_localTypeName
		SQLSMALLINT		m_minimumScale;				///< 14 [NULLABLE] The minimum scale of the data type on the data source. If a data type has a fixed scale, the MINIMUM_SCALE and MAXIMUM_SCALE columns both contain this value.
		bool			m_minimumScaleIsNull;		///< 14 See SqlTypeInfo::m_minimumScale
		SQLSMALLINT		m_maximumScale;				///< 15 [NULLABLE] The maximum scale of the data type on the data source. NULL is returned where scale is not applicable. 
		bool			m_maximumScaleIsNull;		///< 15 See SqlTypeInfo::m_maximumScale
		SQLSMALLINT		m_sqlDataType;				///< 16 [ODBC 3.0] The value of the SQL data type as it appears in the SQL_DESC_TYPE field of the descriptor. This column is the same as the DATA_TYPE column, except for interval and datetime data types.
		SQLSMALLINT		m_sqlDateTimeSub;			///< 17 [ODBC 3.0, NULLABLE] When the value of SQL_DATA_TYPE is SQL_DATETIME or SQL_INTERVAL, this column contains the datetime/interval subcode. For data types other than datetime and interval, this field is NULL.
		bool			m_sqlDateTimeSubIsNull;		///< 17 See SqlTypeInfo::m_sqlDateTimeSub
		SQLINTEGER		m_numPrecRadix;				///< 18 [ODBC 3.0, NULLABLE] If the data type is an approximate numeric type, this column contains the value 2 to indicate that COLUMN_SIZE specifies a number of bits. For exact numeric types, this column contains the value 10 to indicate that COLUMN_SIZE specifies a number of decimal digits. Otherwise, this column is NULL.
		bool			m_numPrecRadixIsNull;		///< 18 See See SqlTypeInfo::m_numPrecRadix
		SQLINTEGER		m_intervalPrecision;		///< 19 [ODBC 3.0, NULLABLE] If the data type is an interval data type, then this column contains the value of the interval leading precision. Otherwise, this column is NULL.
		bool			m_intervalPrecisionIsNull;	///< 19 See SqlTypeInfo::m_intervalPrecision
	};

	/*!
	* \typedef SqlTypeInfosVector
	* \brief std::vector of SqlTypeInfo objects.
	*/
	typedef std::vector<SqlTypeInfo> SqlTypeInfosVector;
}
