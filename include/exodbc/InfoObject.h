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
	* \class TableInfo
	*
	* \brief Information about a Table
	*
	* \details Holds information about a table found using one of the catalog functions.
	*/
	class EXODBCAPI TableInfo
	{	
	public:
		TableInfo();
		TableInfo(const std::string& tableName, const std::string& tableType, const std::string& tableRemarks, const std::string& catalogName, const std::string schemaName, DatabaseProduct dbms = DatabaseProduct::UNKNOWN);
		TableInfo(const std::string& tableName, const std::string& tableType, const std::string& tableRemarks, const std::string& catalogName, const std::string schemaName, bool isCatalogNull, bool isSchemaNull, DatabaseProduct dbms = DatabaseProduct::UNKNOWN);

		std::string GetQueryName() const;
		std::string GetPureName() const;

		std::string		GetType() const { return m_tableType; };
		std::string		GetCatalog() const { return m_catalogName;};
		std::string		GetSchema() const {	return m_schemaName; };

		bool				HasSchema() const { return !m_isSchemaNull && m_schemaName.length() > 0; };
		bool				HasCatalog() const { return !m_isCatalogNull && m_catalogName.length() > 0; };

		bool operator==(const TableInfo& other) const noexcept;
		bool operator!=(const TableInfo& other) const noexcept;

	private:
		DatabaseProduct		m_dbms;

		std::string		m_tableName;		///< Name
		std::string		m_tableType;        ///< "TABLE" or "SYSTEM TABLE" etc.
		std::string		m_tableRemarks;		///< Remarks
		std::string		m_catalogName;		///< catalog
		std::string		m_schemaName;		///< schema
		bool				m_isCatalogNull;	///< True if m_catalogName is null.
		bool				m_isSchemaNull;		///< True if m_schemaName is null.
	};

	/*!
	* \typedef TableInfosVector
	* \brief std::vector of TableInfo objects.
	*/
	typedef std::vector<TableInfo> TableInfosVector;


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
	* \typedef ColumnInfosVector
	* \brief std::vector of ColumnInfo objects.
	*/
	typedef std::vector<ColumnInfo> ColumnInfosVector;


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
	* \class	DatabaseInfo
	* \brief	The following structure contains database information gathered from the Database
	* 			when the Database is first Opened.
	*/
	class EXODBCAPI DatabaseInfo
	{
	public:
		DatabaseInfo();

		enum class StringProperty
		{
			ServerName = SQL_SERVER_NAME,
			DatabaseName = SQL_DATABASE_NAME,

			DbmsName = SQL_DBMS_NAME,
			DbmsVersion = SQL_DBMS_VER,

			DriverName = SQL_DRIVER_NAME,
			DriverOdbcVersion = SQL_DRIVER_ODBC_VER,
			DriverVersion = SQL_DRIVER_VER,

			OdbcSupportIEF = SQL_ODBC_SQL_OPT_IEF,
			OdbcVersion = SQL_ODBC_VER,

			OuterJoins = SQL_OUTER_JOINS,
			ProcedureSupport = SQL_PROCEDURES,
			AccessibleTables = SQL_ACCESSIBLE_TABLES,
			SearchPatternEscape = SQL_SEARCH_PATTERN_ESCAPE,
		};

		enum class USmallIntProperty
		{
			MaxConnections = SQL_MAX_DRIVER_CONNECTIONS,
			MaxConcurrentActivs = SQL_MAX_CONCURRENT_ACTIVITIES,

			OdbcSagCliConformance = SQL_ODBC_SAG_CLI_CONFORMANCE,

			CursorCommitBehavior = SQL_CURSOR_COMMIT_BEHAVIOR,
			CursorRollbackBehavior = SQL_CURSOR_ROLLBACK_BEHAVIOR,
			NonNullableColumns = SQL_NON_NULLABLE_COLUMNS,
			TxnCapable = SQL_TXN_CAPABLE,

			MaxCatalogNameLen = SQL_MAX_CATALOG_NAME_LEN,
			MaxSchemaNameLen = SQL_MAX_SCHEMA_NAME_LEN,
			MaxTableNameLen = SQL_MAX_TABLE_NAME_LEN,
			MaxColumnNameLen = SQL_MAX_COLUMN_NAME_LEN,
		};

		enum class UIntProperty
		{
			DefaultTxnIsolation = SQL_DEFAULT_TXN_ISOLATION,
			TxnIsolationOption = SQL_TXN_ISOLATION_OPTION,
			ScrollOptions = SQL_SCROLL_OPTIONS,
			CursorSensitity = SQL_CURSOR_SENSITIVITY,
			DynamicCursorAttributes1 = SQL_DYNAMIC_CURSOR_ATTRIBUTES1,
			ForwardOnlyCursorAttributes1 = SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1,
			KeysetCursorAttributes1 = SQL_KEYSET_CURSOR_ATTRIBUTES1,
			StaticCursorAttributes1 = SQL_STATIC_CURSOR_ATTRIBUTES1,
			KeysetCursorAttributes2 = SQL_KEYSET_CURSOR_ATTRIBUTES2,
			StaticCursorAttributes2 = SQL_STATIC_CURSOR_ATTRIBUTES2,
		};

		enum class IntProperty
		{
			PosOperations = SQL_POS_OPERATIONS,
			PositionedStatements = SQL_POSITIONED_STATEMENTS,
		};

		typedef std::map<StringProperty, std::string> StringMap;
		typedef std::map<USmallIntProperty, SQLUSMALLINT> USmallIntMap;
		typedef std::map<UIntProperty, SQLUINTEGER> UIntMap;
		typedef std::map<IntProperty, SQLINTEGER> IntMap;

		std::string GetPropertyName(StringProperty prop) const;
		std::string GetPropertyName(USmallIntProperty prop) const;
		std::string GetPropertyName(UIntProperty prop) const;
		std::string GetPropertyName(IntProperty prop) const;

		void SetProperty(StringProperty prop, const std::string& value);
		void SetProperty(USmallIntProperty prop, SQLUSMALLINT value);
		void SetProperty(UIntProperty prop, SQLUINTEGER value);
		void SetProperty(IntProperty prop, SQLINTEGER value);

		void ReadAndStoryProperty(SqlDbcHandlePtr pHDbc, StringProperty prop);
		void ReadAndStoryProperty(SqlDbcHandlePtr pHDbc, USmallIntProperty prop);
		void ReadAndStoryProperty(SqlDbcHandlePtr pHDbc, UIntProperty prop);
		void ReadAndStoryProperty(SqlDbcHandlePtr pHDbc, IntProperty prop);

		std::string GetStringProperty(StringProperty prop) const;
		SQLUSMALLINT GetUSmallIntProperty(USmallIntProperty prop) const;
		SQLUINTEGER GetUIntProperty(UIntProperty prop) const;
		SQLINTEGER GetIntProperty(IntProperty prop) const;

		StringMap GetStringMap() const noexcept { return m_stringMap; };
		USmallIntMap GetUSmallIntMap() const noexcept { return m_uSmallIntMap; };
		UIntMap GetUIntMap() const noexcept { return m_uIntMap; };
		IntMap GetIntMap() const noexcept { return m_intMap; };

		/*!
		* \brief	Returns true if TxnCapable (SQL_TXN_CAPABLE) is not set to SQL_TC_NONE.
		* \throw	If Property TxnCapable is not set to SQL_TC_NONE.
		*/
		bool		GetSupportsTransactions() const;


		/*!
		* \brief	Returns true if bit SQL_SO_FORWARD_ONLY is not the only bit set in ScrollOptions (SQL_SCROLL_OPTIONS).
		* \throw	If SQL_SO_FORWARD_ONLY is only bit set in ScrollOptions.
		*/
		bool		GetForwardOnlyCursors() const;


		std::string GetDriverOdbcVersion() const;
		std::string GetDriverName() const;
		std::string GetDriverVersion() const;

		std::string GetDbmsName() const;

		SQLUSMALLINT GetMaxCatalogNameLen() const;
		SQLUSMALLINT GetMaxSchemaNameLen() const;
		SQLUSMALLINT GetMaxTableNameLen() const;
		SQLUSMALLINT GetMaxColumnNameLen() const;
		SQLUSMALLINT GetMaxTableTypeNameLen() const { return DB_MAX_TABLE_TYPE_LEN; };

		std::string ToString() const;

	private:
		StringMap m_stringMap;
		USmallIntMap m_uSmallIntMap;
		UIntMap m_uIntMap;
		IntMap m_intMap;
	};


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
	* \struct	SDbCatalogInfo
	* \brief	Description of the catalog of a database
	*/
	struct EXODBCAPI SDbCatalogInfo
	{
		TableInfosVector m_tables;
		std::set<std::string> m_catalogs;
		std::set<std::string> m_schemas;
	};


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
