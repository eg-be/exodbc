/*!
* \file InfoObject.h
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2014
* \brief Header file for info objects.
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
#include <map>
#include <set>

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class	SpecialColumnInfo
	* \brief	Information about a special column fetched using the catalog function SQLSpecialColumns.
	* \see: https://msdn.microsoft.com/en-us/library/ms714602%28v=vs.85%29.aspx
	*
	*/
	class EXODBCAPI SpecialColumnInfo
	{
	public:
		SpecialColumnInfo()
			: m_hasScope(false)
			, m_pseudoColumn(PseudoColumn::UNKNOWN)
		{};

		SpecialColumnInfo(const std::string& columnName, RowIdScope scope, SQLSMALLINT sqlType, const std::string& sqlTypeName,
			SQLINTEGER columnSize, SQLINTEGER bufferLength, SQLSMALLINT decimalDigits, PseudoColumn pseudoColumn)
			: m_columnName(columnName)
			, m_scope(scope)
			, m_hasScope(true)
			, m_sqlType(sqlType)
			, m_sqlTypeName(sqlTypeName)
			, m_columnSize(columnSize)
			, m_bufferLength(bufferLength)
			, m_decimalDigits(decimalDigits)
			, m_pseudoColumn(pseudoColumn)
		{};

		SpecialColumnInfo(const std::string& columnName, SQLSMALLINT sqlType, const std::string& sqlTypeName,
			SQLINTEGER columnSize, SQLINTEGER bufferLength, SQLSMALLINT decimalDigits, PseudoColumn pseudoColumn)
			: m_columnName(columnName)
			, m_hasScope(false)
			, m_sqlType(sqlType)
			, m_sqlTypeName(sqlTypeName)
			, m_columnSize(columnSize)
			, m_bufferLength(bufferLength)
			, m_decimalDigits(decimalDigits)
			, m_pseudoColumn(pseudoColumn)
		{};


		std::string GetColumnName() const noexcept { return m_columnName; };
		RowIdScope GetScope() const { exASSERT(m_hasScope); return m_scope; };
		SQLSMALLINT GetSqlType() const noexcept { return m_sqlType; };
		std::string GetSqlTypeName() const noexcept { return m_sqlTypeName; };
		SQLINTEGER GetColumnSize() const noexcept { return m_columnSize; };
		SQLINTEGER GetBufferLength() const noexcept { return m_bufferLength; };
		SQLSMALLINT GetDecimalDigits() const noexcept { return m_decimalDigits; };
		PseudoColumn GetPseudoColumn() const noexcept { return m_pseudoColumn; };

	private:
		RowIdScope	m_scope;
		bool		m_hasScope;

		std::string m_columnName;
		SQLSMALLINT m_sqlType;
		std::string m_sqlTypeName;
		SQLINTEGER m_columnSize;
		SQLINTEGER m_bufferLength;
		SQLSMALLINT m_decimalDigits;
		PseudoColumn m_pseudoColumn;
	};

	/*!
	* \typedef SpecialColumnInfosVector
	* \brief std::vector of SpecialColumnInfo objects.
	*/
	typedef std::vector<SpecialColumnInfo> SpecialColumnInfosVector;


	/*!
	* \class	TablePrimaryKeyInfo
	* \brief	Primary Keys of a table as fetched using SQLPrimaryKeys
	*/
	class EXODBCAPI TablePrimaryKeyInfo
	{
	public:
		TablePrimaryKeyInfo();
		TablePrimaryKeyInfo(const std::string& tableName, const std::string& columnName, SQLSMALLINT keySequence);
		TablePrimaryKeyInfo(const std::string& catalogName, const std::string& schemaName, const std::string& tableName, const std::string& columnName,
			SQLSMALLINT keySequence, const std::string& keyName, bool isCatalogNull, bool isSchemaNull, bool isPrimaryKeyNameNull);

		std::string GetQueryName() const;
		std::string GetPureName() const;

		std::string GetCatalogName() const { exASSERT(!IsCatalogNull()); return m_catalogName; };
		std::string GetSchemaName() const { exASSERT(!IsSchemaNull()); return m_schemaName; };
		std::string GetTableName() const { return m_tableName; };
		std::string GetColumnName() const { return m_columnName; };

		SQLSMALLINT GetKeySequence() const { return m_keySequence; };

		std::string GetKeyName() const { exASSERT(!IsKeyNameNull()); return m_primaryKeyName; };

		bool IsCatalogNull() const { return m_isCatalogNull; };
		bool IsSchemaNull() const { return m_isSchemaNull; };
		bool IsKeyNameNull() const { return m_isPrimaryKeyNameNull; };

	private:
		std::string	m_catalogName;	///< TABLE_CAT [Nullable]. Primary key table catalog name.
		std::string	m_schemaName;	///< TABLE_SCHEM [Nullable]. Primary key table schema name.
		std::string	m_tableName;	///< TABLE_NAME. Primary key table name.
		std::string	m_columnName;	///< COLUMN_NAME. Primary key column name.

		SQLSMALLINT		m_keySequence;	///< KEY_SEQ. Column sequence number in key (starting with 1).
		std::string	m_primaryKeyName;	///< PK_NAME [Nullable]. Column sequence number in key (starting with 1).

		bool			m_isCatalogNull;		///< True if TABLE_CAT is Null.
		bool			m_isSchemaNull;			///< True if TABLE_SCHEM is Null.
		bool			m_isPrimaryKeyNameNull;	///< True if PK_NAME is Null.
	};

	/*!
	* \typedef TablePrimaryKeysVector
	* \brief std::vector of TablePrimaryKeyInfo objects.
	*/
	typedef std::vector<TablePrimaryKeyInfo> TablePrimaryKeysVector;


	/*!
	* \struct SDataSource
	* \brief Contains information about a DataSource-Entry from the driver-manager
	* \see Environment::ListDataSources
	*/
	struct EXODBCAPI SDataSource
	{
		std::string m_dsn;			///< DSN name.
		std::string m_description;	///< Description.
	};

	/*!
	* \typedef DataSourcesVector
	* \brief std::vector of SDataSource objects.
	*/
	typedef std::vector<SDataSource> DataSourcesVector;


	/*!
	* \struct	SSqlTypeInfo
	* \brief	Contains DataType informations read from the database uppon Open().
	* \see http://msdn.microsoft.com/en-us/library/ms714632%28v=vs.85%29.aspx
	*/
	struct EXODBCAPI SSqlTypeInfo
	{
		SSqlTypeInfo();

		std::string	m_typeName;					///<  1 Data source dependent data-type name
		SQLSMALLINT		m_sqlType;					///<  2 SQL data type. This can be an ODBC SQL data type or a driver-specific SQL data type.
		SQLINTEGER		m_columnSize;				///<  3 [NULLABLE] The maximum column size that the server supports for this data type. For numeric data, this is the maximum precision. For string data, this is the length in characters. For datetime data types, this is the length in characters of the string representation (assuming the maximum allowed precision of the fractional seconds component). NULL is returned for data types where column size is not applicable.
		bool			m_columnSizeIsNull;			///<  3 See SSqlTypeInfo::m_columnSize
		std::string	m_literalPrefix;			///<  4 [NULLABLE] Character or characters used to prefix a literal; for example, a single quotation mark (') for character data types or 0x for binary data types
		bool			m_literalPrefixIsNull;		///<  4 See SSqlTypeInfo::m_literalPrefix
		std::string	m_literalSuffix;			///<  5 [NULLABLE] Character or characters used to terminate a literal; for example, a single quotation mark (') for character data types;
		bool			m_literalSuffixIsNull;		///<  5 See SSqlTypeInfo::m_literalSuffix
		std::string	m_createParams;				///<  6 [NULLABLE] A list of keywords, separated by commas, corresponding to each parameter that the application may specify in parentheses when using the name that is returned in the TYPE_NAME field.
		bool			m_createParamsIsNull;		///<  6 See SSqlTypeInfo::m_createParams
		SQLSMALLINT		m_nullable;					///<  7 Whether the data type accepts a NULL value: SQL_NO_NULLS, SQL_NULLABLE or	SQL_NULLABLE_UNKNOWN.
		SQLSMALLINT		m_caseSensitive;			///<  8 Whether a character data type is case-sensitive in collations and comparisons: SQL_TRUE or SQL_FALSE
		SQLSMALLINT		m_searchable;				///<  9 How the data type is used in a WHERE clause: SQL_PRED_NONE (no use), SQL_PRED_CHAR (only with LIKE), SQL_PRED_BASIC (all except LIKE), SQL_SEARCHABLE (anything)
		SQLSMALLINT		m_unsigned;					///< 10 [NULLABLE] Whether the data type is unsigned: SQL_TRUE or SQL_FALSE
		bool			m_unsignedIsNull;			///< 10 See SSqlTypeInfo::m_unsigned
		SQLSMALLINT		m_fixedPrecisionScale;		///< 11 Whether the data type has predefined fixed precision and scale (which are data source–specific), such as a money data type: SQL_TRUE or SQL_FALSE
		SQLSMALLINT		m_autoUniqueValue;			///< 12 [NULLABLE] Whether the data type is autoincrementing: SQL_TRUE or SQL_FALSE
		bool			m_autoUniqueValueIsNull;	///< 12 See SSqlTypeInfo::m_autoUniqueValue
		std::string	m_localTypeName;			///< 13 [NULLABLE] localized version of the data source–dependent name of the data type.
		bool			m_localTypeNameIsNull;		///< 13 See SSqlTypeInfo::m_localTypeName
		SQLSMALLINT		m_minimumScale;				///< 14 [NULLABLE] The minimum scale of the data type on the data source. If a data type has a fixed scale, the MINIMUM_SCALE and MAXIMUM_SCALE columns both contain this value.
		bool			m_minimumScaleIsNull;		///< 14 See SSqlTypeInfo::m_minimumScale
		SQLSMALLINT		m_maximumScale;				///< 15 [NULLABLE] The maximum scale of the data type on the data source. NULL is returned where scale is not applicable. 
		bool			m_maximumScaleIsNull;		///< 15 See SSqlTypeInfo::m_maximumScale
		SQLSMALLINT		m_sqlDataType;				///< 16 [ODBC 3.0] The value of the SQL data type as it appears in the SQL_DESC_TYPE field of the descriptor. This column is the same as the DATA_TYPE column, except for interval and datetime data types.
		SQLSMALLINT		m_sqlDateTimeSub;			///< 17 [ODBC 3.0, NULLABLE] When the value of SQL_DATA_TYPE is SQL_DATETIME or SQL_INTERVAL, this column contains the datetime/interval subcode. For data types other than datetime and interval, this field is NULL.
		bool			m_sqlDateTimeSubIsNull;		///< 17 See SSqlTypeInfo::m_sqlDateTimeSub
		SQLINTEGER		m_numPrecRadix;				///< 18 [ODBC 3.0, NULLABLE] If the data type is an approximate numeric type, this column contains the value 2 to indicate that COLUMN_SIZE specifies a number of bits. For exact numeric types, this column contains the value 10 to indicate that COLUMN_SIZE specifies a number of decimal digits. Otherwise, this column is NULL.
		bool			m_numPrecRadixIsNull;		///< 18 See See SSqlTypeInfo::m_numPrecRadix
		SQLINTEGER		m_intervalPrecision;		///< 19 [ODBC 3.0, NULLABLE] If the data type is an interval data type, then this column contains the value of the interval leading precision. Otherwise, this column is NULL.
		bool			m_intervalPrecisionIsNull;	///< 19 See SSqlTypeInfo::m_intervalPrecision

		std::string ToOneLineStrForTrac(bool withHeaderLine /* = false */) const;
		std::string ToOneLineStr(bool withHeaderLines = false, bool withEndLine = false) const;
		std::string ToStr() const;

		bool operator<(const SSqlTypeInfo& other) const;
	};

	/*!
	* \typedef SqlTypeInfosVector
	* \brief std::vector of SSqlTypeInfo objects.
	*/
	typedef std::vector<SSqlTypeInfo> SqlTypeInfosVector;


	/*!
	* \struct	STablePrivilegesInfo
	* \brief	TablePrivileges fetched using the catalog function SQLTablePrivilege
	*/
	struct EXODBCAPI STablePrivilegesInfo
	{
		std::string	m_catalogName;
		std::string	m_schemaName;
		std::string	m_tableName;
		std::string	m_grantor;
		std::string	m_grantee;
		std::string	m_privilege;
		std::string	m_grantable;

		bool			m_isCatalogNull;
		bool			m_isSchemaNull;
		bool			m_isGrantorNull;
		bool			m_isGrantableNull;
	};

	/*!
	* \typedef TablePrivilegesVector
	* \brief std::vector of STablePrivilegesInfo objects.
	*/
	typedef std::vector<STablePrivilegesInfo> TablePrivilegesVector;


	/*!
	* \struct SColumnDescription
	* \brief	Result of SQLDescribeCol operation.
	*/
	struct SColumnDescription
	{
		SColumnDescription()
			: m_sqlType(SQL_UNKNOWN_TYPE)
			, m_charSize(0)
			, m_decimalDigits(0)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};
		SColumnDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits, SQLSMALLINT paramNullable)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(paramNullable)
		{};
		SColumnDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};

		std::string m_name;
		SQLSMALLINT m_sqlType;
		SQLULEN m_charSize;
		SQLSMALLINT m_decimalDigits;
		SQLSMALLINT m_nullable;
	};


	/*!
	* \struct SParameterDescription
	* \brief	Result of SQLDescribeParam operation.
	*/
	struct SParameterDescription
	{
		SParameterDescription()
			: m_sqlType(SQL_UNKNOWN_TYPE)
			, m_charSize(0)
			, m_decimalDigits(0)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};
		SParameterDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits, SQLSMALLINT paramNullable)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(paramNullable)
		{};
		SParameterDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};

		SQLSMALLINT m_sqlType;
		SQLULEN m_charSize;
		SQLSMALLINT m_decimalDigits;
		SQLSMALLINT m_nullable;
	};
}
