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
	* \see  http://msdn.microsoft.com/en-us/library/ms711683%28v=vs.85%29.aspx
	* \see DatabaseCatalog::ReadColumnInfo()
	*/
	class EXODBCAPI ColumnInfo
	{
	public:
		/*!
		* \brief Init all members with empty or 0 values. All null flags are set to true,
		*		enum values are set to unknown value.
		*/
		ColumnInfo();


		/*!
		* \brief Use passed values to init members.
		*/
		ColumnInfo(const std::string& catalogName, const std::string& schemaName, const std::string& tableName, const std::string& columnName,
			SQLSMALLINT sqlType, const std::string& typeName, SQLINTEGER columnSize, SQLINTEGER bufferSize, SQLSMALLINT decimalDigits, SQLSMALLINT numPrecRadix,
			SQLSMALLINT nullable, const std::string& remarks, const std::string& defaultValue, SQLSMALLINT sqlDataType, SQLSMALLINT sqlDatetimeSub,
			SQLINTEGER charOctetLength, SQLINTEGER ordinalPosition, const std::string& isNullable, bool isCatalogNull, bool isSchemaNull, bool isColumnSizeNull,
			bool isBufferSizeNull, bool isDecimalDigitsNull, bool isNumPrecRadixNull, bool isRemarksNull, bool isDefaultValueNull, bool isSqlDatetimeSubNull,
			bool isIsNullableNull);

		/*!
		* \brief Return the non-empty name to be used in queries like SELECT, UPDATE, etc.
		* \throw AssertionException If no non-empty query name can be returned.
		*/
		std::string GetQueryName() const;


		/*!
		* \return Table name. Empty value might be returned.
		*/
		std::string GetName() const noexcept { return m_tableName; }


		/*!
		* \return True if null flag for Schema is not set and Schema Name is not empty.
		*/
		bool				HasSchema() const noexcept { return !m_isSchemaNull && m_schemaName.length() > 0; };


		/*!
		* \return True if null flag for Catalog is not set and Catalog Name is not empty.
		*/
		bool				HasCatalog() const noexcept { return !m_isCatalogNull && m_catalogName.length() > 0; };


		/*!
		* \brief True if null flag for Catalog is set.
		*/
		bool			IsCatalogNull() const noexcept { return m_isCatalogNull; };


		/*!
		* \return True if null flag for Schema is set.
		*/
		bool			IsSchemaNull() const noexcept { return m_isSchemaNull; };


		/*!
		* \return True if null flag for ColumSize is set.
		*/
		bool			IsColumnSizeNull() const noexcept { return m_isColumnSizeNull; };


		/*!
		* \return True if null flag for BufferSize is set.
		*/
		bool			IsBufferSizeNull() const noexcept { return m_isBufferSizeNull; };
		

		/*!
		* \return True if null flag for DecimalDigits is set.
		*/
		bool			IsDecimalDigitsNull() const noexcept { return m_isDecimalDigitsNull; };


		/*!
		* \return True if null flag for NumPrecRadix is set.
		*/
		bool			IsNumPrecRadixNull() const noexcept { return m_isNumPrecRadixNull; };


		/*!
		* \return True if null flag for Remarks is set.
		*/
		bool			IsRemarksNull() const noexcept { return m_isRemarksNull; };


		/*!
		* \return True if null flag for DefaultValueNull is set.
		*/
		bool			IsDefaultValueNull() const noexcept { return m_isDefaultValueNull; };


		/*!
		* \return True if null flag for SqlDatetimeSub is set.
		*/
		bool			IsSqlDatetimeSubNull() const noexcept { return m_isSqlDatetimeSubNull; };


		/*!
		* \return True if null flag for CharOctetLength is set.
		*/
		bool			IsCharOctetLengthNull() const noexcept { return m_isCharOctetLengthNull; };


		/*!
		* \return True if null flag for IsNullable is set.
		*/
		bool			IsIsNullableNull() const noexcept { return m_isIsNullableNull; };


		/*!
		* \return Catalog name. Empty value might be returned.
		*/
		std::string	GetCatalogName() const noexcept { return m_catalogName; };


		/*!
		* \return Schema name. Empty value might be returned.
		*/
		std::string	GetSchemaName() const noexcept { return m_schemaName; };


		/*!
		* \return Table name. Empty value might be returned.
		*/
		std::string	GetTableName() const noexcept { return m_tableName; };


		/*!
		* \return Column name. Empty value might be returned.
		*/
		std::string	GetColumnName() const noexcept { return m_columnName; };


		/*!
		* \return SQL Type.
		*/
		SQLSMALLINT		GetSqlType() const noexcept { return m_sqlType; };


		/*!
		* \return SQL Type name. Empty value might be returned.
		*/
		std::string	GetTypeName() const noexcept { return m_typeName; };


		/*!
		* \return Column Size.
		*/
		SQLINTEGER		GetColumnSize() const noexcept { return m_columnSize; };


		/*!
		* \return Buffer Size.
		*/
		SQLINTEGER		GetBufferSize() const noexcept { return m_bufferSize; };


		/*!
		* \return Decimal Digits.
		*/
		SQLSMALLINT		GetDecimalDigits() const noexcept { return m_decimalDigits; };


		/*!
		* \return Num Prec Radix
		*/
		SQLSMALLINT		GetNumPrecRadix() const noexcept { return m_numPrecRadix; };


		/*!
		* \return Nullable
		*/
		SQLSMALLINT		GetNullable() const noexcept { return m_nullable; };


		/*!
		* \return Remarks. Empty value might be returned.
		*/
		std::string	GetRemarks() const noexcept { return m_remarks; };


		/*!
		* \return Default Value. Empty value might be returned.
		*/
		std::string	GetDefaultValue() const noexcept { return m_defaultValue; };


		/*!
		* \return SQL Data Type.
		*/
		SQLSMALLINT		GetSqlDataType() const noexcept { return m_sqlDataType; };


		/*!
		* \return SQL Datetime Sub.
		*/
		SQLSMALLINT		GetSqlDatetimeSub() const noexcept { return m_sqlDatetimeSub; };


		/*!
		* \return Char Octet Length
		*/
		SQLINTEGER		GetCharOctetLength() const noexcept { return m_charOctetLength; };


		/*!
		* \return Ordinal Position.
		*/
		SQLINTEGER		GetOrdinalPosition() const noexcept { return m_ordinalPosition; };


		/*!
		* \return True if null flag for Nullable is set.
		*/
		std::string	GetIsNullable() const noexcept { return m_isNullable; };

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
