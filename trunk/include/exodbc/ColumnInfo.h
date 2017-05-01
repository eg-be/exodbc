/*!
* \file ColumnInfo.h
* \author Elias Gerber <eg@elisium.ch>
* \date 01.05.2017
* \brief Header file for ColumnInfo.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "AssertionException.h"

// Other headers
// System headers
#include <string>
#include <vector>

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class	ColumnInfo
	* \brief	Information about a column fetched using the catalog function SQLColumns.
	* \see: http://msdn.microsoft.com/en-us/library/ms711683%28v=vs.85%29.aspx
	*
	*/
	class EXODBCAPI ColumnInfo
	{
	private:
		ColumnInfo();

	public:
		/*!
		* \brief	Create new ColumnInfo.
		* \details
		* \param catalogName
		* \param schemaName
		* \param tableName
		* \param columnName
		* \param sqlType
		* \param typeName
		* \param columnSize
		* \param bufferSize
		* \param decimalDigits
		* \param numPrecRadix
		* \param nullable
		* \param remarks
		* \param defaultValue
		* \param sqlDataType
		* \param sqlDatetimeSub
		* \param charOctetLength
		* \param ordinalPosition
		* \param isNullable
		* \param isCatalogNull
		* \param isSchemaNull
		* \param isColumnSizeNull
		* \param isBufferSizeNull
		* \param isDecimalDigitsNull
		* \param isNumPrecRadixNull
		* \param isRemarksNull
		* \param isDefaultValueNull
		* \param isSqlDatetimeSubNull
		* \param isIsNullableNull
		* \throw Exception If columnName is empty.
		*/
		ColumnInfo(const std::string& catalogName, const std::string& schemaName, const std::string& tableName, const std::string& columnName,
			SQLSMALLINT sqlType, const std::string& typeName, SQLINTEGER columnSize, SQLINTEGER bufferSize, SQLSMALLINT decimalDigits, SQLSMALLINT numPrecRadix,
			SQLSMALLINT nullable, const std::string& remarks, const std::string& defaultValue, SQLSMALLINT sqlDataType, SQLSMALLINT sqlDatetimeSub,
			SQLINTEGER charOctetLength, SQLINTEGER ordinalPosition, const std::string& isNullable, bool isCatalogNull, bool isSchemaNull, bool isColumnSizeNull,
			bool isBufferSizeNull, bool isDecimalDigitsNull, bool isNumPrecRadixNull, bool isRemarksNull, bool isDefaultValueNull, bool isSqlDatetimeSubNull,
			bool isIsNullableNull);

		/*!
		* \brief	Returns only the ColumnName.
		*/
		std::string GetQueryName() const noexcept;


		/*!
		* \brief	Returns the pure name, which is the columnName.
		* \return std::string
		*/
		std::string GetPureName() const noexcept;

		bool				HasSchema() const { return !m_isSchemaNull && m_schemaName.length() > 0; };
		bool				HasCatalog() const { return !m_isCatalogNull && m_catalogName.length() > 0; };

		std::string	GetCatalogName() const { exASSERT(!IsCatalogNull());  return m_catalogName; };
		std::string	GetSchemaName() const { exASSERT(!IsSchemaNull()); return m_schemaName; };
		std::string	GetTableName() const { return m_tableName; };
		std::string	GetColumnName() const { return m_columnName; };
		SQLSMALLINT		GetSqlType() const { return m_sqlType; };
		std::string	GetTypeName() const { return m_typeName; };
		SQLINTEGER		GetColumnSize() const { exASSERT(!IsColumnSizeNull()); return m_columnSize; };
		SQLINTEGER		GetBufferSize() const { exASSERT(!IsBufferSizeNull()); return m_bufferSize; };
		SQLSMALLINT		GetDecimalDigits() const { exASSERT(!IsDecimalDigitsNull()); return m_decimalDigits; };
		SQLSMALLINT		GetNumPrecRadix() const { exASSERT(!IsNumPrecRadixNull()); return m_numPrecRadix; };
		SQLSMALLINT		GetNullable() const { return m_nullable; };
		std::string	GetRemarks() const { exASSERT(!IsRemarksNull()); return m_remarks; };
		std::string	GetDefaultValue() const { exASSERT(!IsDefaultValueNull()); return m_defaultValue; };
		SQLSMALLINT		GetSqlDataType() const { return m_sqlDataType; };
		SQLSMALLINT		GetSqlDatetimeSub() const { exASSERT(!IsSqlDatetimeSubNull()); return m_sqlDatetimeSub; };
		SQLINTEGER		GetCharOctetLength() const { exASSERT(!IsCharOctetLengthNull()); return m_charOctetLength; };
		SQLINTEGER		GetOrdinalPosition() const { return m_ordinalPosition; };
		std::string	GetIsNullable() const { return m_isNullable; };

		bool			IsCatalogNull() const { return m_isCatalogNull; };
		bool			IsSchemaNull() const { return m_isSchemaNull; };
		bool			IsColumnSizeNull() const { return m_isColumnSizeNull; };
		bool			IsBufferSizeNull() const { return m_isBufferSizeNull; };
		bool			IsDecimalDigitsNull() const { return m_isDecimalDigitsNull; };
		bool			IsNumPrecRadixNull() const { return m_isNumPrecRadixNull; };
		bool			IsRemarksNull() const { return m_isRemarksNull; };
		bool			IsDefaultValueNull() const { return m_isDefaultValueNull; };
		bool			IsSqlDatetimeSubNull() const { return m_isSqlDatetimeSubNull; };
		bool			IsCharOctetLengthNull() const { return m_isCharOctetLengthNull; };
		bool			IsIsNullableNull() const { return m_isIsNullableNull; };

	private:

		std::string	m_catalogName;		///< [NULLABLE] Catalog name
		std::string	m_schemaName;		///< [NULLABLE] Schema name
		std::string	m_tableName;		///< Table name
		std::string	m_columnName;		///< Column Name. Empty for columns without a name
		SQLSMALLINT		m_sqlType;			///< SQL data type
		std::string	m_typeName;			///< Data source-dependent type name
		SQLINTEGER		m_columnSize;		///< [NULLABLE] for char-columns the max length in characters; numeric total nr of digits or total number of bits, see numPrecRadix.
		SQLINTEGER		m_bufferSize;		///< [NULLABLE] Length of bits needed for SQLGetDat, SQLFetch if used with SQL_C_DEFAULT.
		SQLSMALLINT		m_decimalDigits;	///< [NULLABLE] Total number of significant digits right of decimal. For time-stuff: number of digits in fractional part, ..
		SQLSMALLINT		m_numPrecRadix;		///< [NULLABLE] See msdn, defines nr. of decimal digits.
		SQLSMALLINT		m_nullable;			///< SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
		std::string	m_remarks;			///< [NULLABLE] Description
		std::string	m_defaultValue;		///< [NULLABLE] Default value
		SQLSMALLINT		m_sqlDataType;		///< [ODBC 3.0] Sql Data Type
		SQLSMALLINT		m_sqlDatetimeSub;	///< [ODBC 3.0, NULLABLE] The subtype code for datetime and interval data types
		SQLINTEGER		m_charOctetLength;	///< [ODBC 3.0, NULLABLE] The maximum length in bytes of a character or binary data type column. 
		SQLINTEGER		m_ordinalPosition;	///< [ODBC 3.0] The ordinal position of the column in the table. The first column in the table is number 1.
		std::string	m_isNullable;		///< [ODBC 3.0] NO, YES or zero-length string if unknown

		bool			m_isCatalogNull;			///< See ColumnInfo::m_catalogName
		bool			m_isSchemaNull;				///< See ColumnInfo::m_schemaName
		bool			m_isColumnSizeNull;			///< See ColumnInfo::m_columnSize
		bool			m_isBufferSizeNull;			///< See ColumnInfo::m_bufferSize
		bool			m_isDecimalDigitsNull;		///< See ColumnInfo::m_decimalDigits
		bool			m_isNumPrecRadixNull;		///< See ColumnInfo::m_numPrecRadix
		bool			m_isRemarksNull;			///< See ColumnInfo::m_remarks
		bool			m_isDefaultValueNull;		///< See ColumnInfo::m_defaultValue
		bool			m_isSqlDatetimeSubNull;		///< See ColumnInfo::m_sqlDatetimeSub
		bool			m_isCharOctetLengthNull;	///< See ColumnInfo::m_charOctetLength
		bool			m_isIsNullableNull;			///< See ColumnInfo::isNullable
	};

	/*!
	* \typedef ColumnInfoVector
	* \brief std::vector of ColumnInfo objects.
	*/
	typedef std::vector<ColumnInfo> ColumnInfoVector;
}
