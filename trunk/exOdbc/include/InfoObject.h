/*!
* \file InfoObject.h
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2014
* \brief Header file for info objects.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#pragma once
#ifndef INFOOBJECT_H
#define INFOOBJECT_H

// Same component headers
#include "exOdbc.h"
#include "ObjectName.h"

// Other headers

// System headers
#include <string>
#include <vector>

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
		: public ObjectName
	{
	public:
		TableInfo();
		TableInfo(const std::wstring& tableName, const std::wstring& tableType, const std::wstring& tableRemarks, const std::wstring& catalogName, const std::wstring schemaName, DatabaseProduct dbms = DatabaseProduct::UNKNOWN);
		TableInfo(const std::wstring& tableName, const std::wstring& tableType, const std::wstring& tableRemarks, const std::wstring& catalogName, const std::wstring schemaName, bool isCatalogNull, bool isSchemaNull, DatabaseProduct dbms = DatabaseProduct::UNKNOWN);

		virtual std::wstring GetQueryName() const;
		virtual std::wstring GetPureName() const;

		std::wstring		GetType() const { return m_tableType; };
		std::wstring		GetCatalog() const { return m_catalogName;};
		std::wstring		GetSchema() const {	return m_schemaName; };

		bool				HasSchema() const { return !m_isSchemaNull && m_schemaName.length() > 0; };
		bool				HasCatalog() const { return !m_isCatalogNull && m_catalogName.length() > 0; };

	private:
		DatabaseProduct		m_dbms;

		std::wstring		m_tableName;		///< Name
		std::wstring		m_tableType;        ///< "TABLE" or "SYSTEM TABLE" etc.
		std::wstring		m_tableRemarks;		///< Remarks
		std::wstring		m_catalogName;		///< catalog
		std::wstring		m_schemaName;		///< schema
		bool				m_isCatalogNull;	///< True if m_catalogName is null.
		bool				m_isSchemaNull;		///< True if m_schemaName is null.

	};

	/*!
	* \typedef TableInfosVector
	* \brief std::vector of TableInfo objects.
	*/
	typedef std::vector<TableInfo> TableInfosVector;


	/*!
	* \class	ColumnInfo
	* \brief	Information about a column fetched using the catalog function SQLColumns.
	* \see: http://msdn.microsoft.com/en-us/library/ms711683%28v=vs.85%29.aspx
	*/
	class EXODBCAPI ColumnInfo
	{
	public:
		ColumnInfo();

		std::wstring	m_catalogName;		///< [NULLABLE] Catalog name
		std::wstring	m_schemaName;		///< [NULLABLE] Schema name
		std::wstring	m_tableName;		///< Table name
		std::wstring	m_columnName;		///< Column Name. Empty for columns without a name
		SQLSMALLINT		m_sqlType;			///< SQL data type
		std::wstring	m_typeName;			///< Data source-dependent type name
		SQLINTEGER		m_columnSize;		///< [NULLABLE] for char-columns the max length in characters; numeric total nr of digits or total number of bits, see numPrecRadix.
		SQLINTEGER		m_bufferSize;		///< [NULLABLE] Length of bits needed for SQLGetDat, SQLFetch if used with SQL_C_DEFAULT.
		SQLSMALLINT		m_decimalDigits;	///< [NULLABLE] Total number of significant digits right of decimal. For time-stuff: number of digits in fractional part, ..
		SQLSMALLINT		m_numPrecRadix;		///< [NULLABLE] See msdn, defines nr. of decimal digits.
		SQLSMALLINT		m_nullable;			///< SQL_NO_NULLS, SQL_NULLABLE or SQL_NULLABLE_UNKNOWN
		std::wstring	m_remarks;			///< [NULLABLE] Description
		std::wstring	m_defaultValue;		///< [NULLABLE] Default value
		SQLSMALLINT		m_sqlDataType;		///< [ODBC 3.0] Sql Data Type
		SQLSMALLINT		m_sqlDatetimeSub;	///< [ODBC 3.0, NULLABLE] The subtype code for datetime and interval data types
		SQLINTEGER		m_charOctetLength;	///< [ODBC 3.0, NULLABLE] The maximum length in bytes of a character or binary data type column. 
		SQLINTEGER		m_ordinalPosition;	///< [ODBC 3.0] The ordinal position of the column in the table. The first column in the table is number 1.
		std::wstring	m_isNullable;		///< [ODBC 3.0] NO, YES or zero-length string if unknown

		bool			m_isCatalogNull;			///< See ColumnInfo::m_catalogName
		bool			m_isSchemaNull;				///< See ColumnInfo::m_schemaName
		bool			m_isColumnSizeNull;			///< See ColumnInfo::m_columnSize
		bool			m_isBufferSizeNull;			///< See ColumnInfo::m_bufferSize
		bool			m_isDecimalDigitsNull;		///< See ColumnInfo::m_decimalDigits
		bool			m_isNumPrecRadixNull;		///< See ColumnInfo::m_numPrecRadix
		bool			m_isRemarksNull;			///< See ColumnInfo::m_remarks
		bool			m_isDefaultValueNull;		///< See ColumnInfo::m_defaultValue
		bool			m_isDatetimeSubNull;		///< See ColumnInfo::m_sqlDatetimeSub
		bool			m_isCharOctetLengthNull;	///< See ColumnInfo::m_charOctetLength
		bool			m_isIsNullableNull;			///< See ColumnInfo::isNullable

		bool				HasSchema() const { return !m_isSchemaNull && m_schemaName.length() > 0; };
		bool				HasCatalog() const { return !m_isCatalogNull && m_catalogName.length() > 0; };

		//		void		SetSqlNameHint(ColumnQueryNameHint hint) { m_queryNameHint = hint; };
		std::wstring		GetSqlName() const;
		std::wstring GetPureColumnName() const;

	private:
		//		ColumnQueryNameHint m_queryNameHint;
	};

	/*!
	* \typedef ColumnInfosVector
	* \brief std::vector of ColumnInfo objects.
	*/
	typedef std::vector<ColumnInfo> ColumnInfosVector;



	/*!
	* \struct SDataSource
	* \brief Contains information about a DataSource-Entry from the driver-manager
	* \see Environment::ListDataSources
	*/
	struct EXODBCAPI SDataSource
	{
		std::wstring m_dsn;			///< DSN name.
		std::wstring m_description;	///< Description.
	};

	/*!
	* \typedef DataSourcesVector
	* \brief std::vector of SDataSource objects.
	*/
	typedef std::vector<SDataSource> DataSourcesVector;


	/*!
	* \struct	SDbInfo
	* \brief	The following structure contains database information gathered from the datasource
	* 			when the datasource is first Opened.
	*/
	struct EXODBCAPI SDbInfo
	{
		SDbInfo();

		~SDbInfo() {};

		// [Output] Pointer to a buffer in which to return the information. Depending on the InfoType requested, 
		// the information returned will be one of the following: a null-terminated character string, an SQLUSMALLINT value, 
		// an SQLUINTEGER bitmask, an SQLUINTEGER flag, a SQLUINTEGER binary value, or a SQLULEN value.
		// See: http://msdn.microsoft.com/en-us/library/ms711681%28v=vs.85%29.aspx

		std::wstring	m_dbmsName;						///< Name of the dbms product
		std::wstring	m_dbmsVer;						///< Version # of the dbms product
		std::wstring	m_driverName;					///< Driver name
		std::wstring	m_odbcVer;						///< ODBC version of the driver
		std::wstring	m_drvMgrOdbcVer;				///< ODBC version of the driver manager
		std::wstring	m_driverVer;					///< Driver version
		std::wstring	m_serverName;					///< Server Name, typically a connect string
		std::wstring	m_databaseName;					///< Database filename
		std::wstring	m_outerJoins;					///< Indicates whether the data source supports outer joins
		std::wstring	m_procedureSupport;				///< Indicates whether the data source supports stored procedures
		std::wstring	m_accessibleTables;				///< Indicates whether the data source only reports accessible tables in SQLTables.
		SQLUSMALLINT	m_maxConnections;				///< Maximum # of connections the data source supports
		SQLUSMALLINT	m_maxActiveStmts;				///< SQL_MAX_CONCURRENT_ACTIVITIES Maximum # of concurent active SQLHSTMTs per SQLHDBC.
		SQLUSMALLINT	m_cliConfLvl;					///< Indicates whether the data source is SAG compliant
		SQLUSMALLINT	m_cursorCommitBehavior;			///< Indicates how cursors are affected by a db commit
		SQLUSMALLINT	m_cursorRollbackBehavior;		///< Indicates how cursors are affected by a db rollback
		SQLUSMALLINT	m_supportNotNullClause;			///< Indicates if data source supports NOT NULL clause
		std::wstring	m_supportIEF;					///< Integrity Enhancement Facility (Referential Integrity)
		SQLUINTEGER		m_txnIsolation;					///< Default transaction isolation level supported by the driver
		SQLUINTEGER		m_txnIsolationOptions;			///< Transaction isolation level options available
		SQLINTEGER		m_posOperations;				///< Position operations supported in SQLSetPos
		SQLINTEGER		m_posStmts;						///< An SQLINTEGER bitmask enumerating the supported positioned SQL statements.
		SQLUINTEGER		m_scrollOptions;				///< Scroll Options supported for scrollable cursors
		SQLUSMALLINT	m_txnCapable;					///< Indicates if the data source supports transactions
		// TODO: Connection attribute
		//			UDWORD loginTimeout;                ///< Number seconds to wait for a login request
		SQLUSMALLINT	m_maxCatalogNameLen;			///< Max length of a catalog name. Can be 0 if no limit, or limit is unknown
		SQLUSMALLINT	m_maxSchemaNameLen;				///< Max length of a schema name. Can be 0 if no limit, or limit is unknown
		SQLUSMALLINT	m_maxTableNameLen;				///< Max length of a table name. Can be 0 if no limit, or limit is unknown
		SQLUSMALLINT	m_maxColumnNameLen;				///< Max length of a column name. Can be 0 if no limit, or limit is unknown
		std::wstring	m_searchPatternEscape;			///< SQL_SEARCH_PATTERN_ESCAPE: How to escape string-search patterns in pattern-value arguments in catalog functions
		std::wstring ToStr() const;

		SQLUSMALLINT GetMaxCatalogNameLen() const;
		SQLUSMALLINT GetMaxSchemaNameLen() const;
		SQLUSMALLINT GetMaxTableNameLen() const;
		SQLUSMALLINT GetMaxColumnNameLen() const;
		SQLUSMALLINT GetMaxTableTypeNameLen() const { return DB_MAX_TABLE_TYPE_LEN; };
	};


	/*!
	* \struct	SSqlTypeInfo
	* \brief	Contains DataType informations read from the database uppon Open().
	* \see http://msdn.microsoft.com/en-us/library/ms714632%28v=vs.85%29.aspx
	*/
	struct EXODBCAPI SSqlTypeInfo
	{
		SSqlTypeInfo();

		std::wstring	m_typeName;					///<  1 Data source dependent data-type name
		SQLSMALLINT		m_sqlType;					///<  2 SQL data type. This can be an ODBC SQL data type or a driver-specific SQL data type.
		SQLINTEGER		m_columnSize;				///<  3 [NULLABLE] The maximum column size that the server supports for this data type. For numeric data, this is the maximum precision. For string data, this is the length in characters. For datetime data types, this is the length in characters of the string representation (assuming the maximum allowed precision of the fractional seconds component). NULL is returned for data types where column size is not applicable.
		bool			m_columnSizeIsNull;			///<  3 See SSqlTypeInfo::m_columnSize
		std::wstring	m_literalPrefix;			///<  4 [NULLABLE] Character or characters used to prefix a literal; for example, a single quotation mark (') for character data types or 0x for binary data types
		bool			m_literalPrefixIsNull;		///<  4 See SSqlTypeInfo::m_literalPrefix
		std::wstring	m_literalSuffix;			///<  5 [NULLABLE] Character or characters used to terminate a literal; for example, a single quotation mark (') for character data types;
		bool			m_literalSuffixIsNull;		///<  5 See SSqlTypeInfo::m_literalSuffix
		std::wstring	m_createParams;				///<  6 [NULLABLE] A list of keywords, separated by commas, corresponding to each parameter that the application may specify in parentheses when using the name that is returned in the TYPE_NAME field.
		bool			m_createParamsIsNull;		///<  6 See SSqlTypeInfo::m_createParams
		SQLSMALLINT		m_nullable;					///<  7 Whether the data type accepts a NULL value: SQL_NO_NULLS, SQL_NULLABLE or	SQL_NULLABLE_UNKNOWN.
		SQLSMALLINT		m_caseSensitive;			///<  8 Whether a character data type is case-sensitive in collations and comparisons: SQL_TRUE or SQL_FALSE
		SQLSMALLINT		m_searchable;				///<  9 How the data type is used in a WHERE clause: SQL_PRED_NONE (no use), SQL_PRED_CHAR (only with LIKE), SQL_PRED_BASIC (all except LIKE), SQL_SEARCHABLE (anything)
		SQLSMALLINT		m_unsigned;					///< 10 [NULLABLE] Whether the data type is unsigned: SQL_TRUE or SQL_FALSE
		bool			m_unsignedIsNull;			///< 10 See SSqlTypeInfo::m_unsigned
		SQLSMALLINT		m_fixedPrecisionScale;		///< 11 Whether the data type has predefined fixed precision and scale (which are data source–specific), such as a money data type: SQL_TRUE or SQL_FALSE
		SQLSMALLINT		m_autoUniqueValue;			///< 12 [NULLABLE] Whether the data type is autoincrementing: SQL_TRUE or SQL_FALSE
		bool			m_autoUniqueValueIsNull;	///< 12 See SSqlTypeInfo::m_autoUniqueValue
		std::wstring	m_localTypeName;			///< 13 [NULLABLE] localized version of the data source–dependent name of the data type.
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

		std::wstring ToOneLineStr(bool withHeaderLines = false, bool withEndLine = false) const;
		std::wstring ToStr() const;
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
		std::set<std::wstring> m_catalogs;
		std::set<std::wstring> m_schemas;
	};


	/*!
	* \struct	STablePrivilegesInfo
	* \brief	TablePrivileges fetched using the catalog function SQLTablePrivilege
	*/
	struct EXODBCAPI STablePrivilegesInfo
	{
		std::wstring	m_catalogName;
		std::wstring	m_schemaName;
		std::wstring	m_tableName;
		std::wstring	m_grantor;
		std::wstring	m_grantee;
		std::wstring	m_privilege;
		std::wstring	m_grantable;

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
	* \struct	STablePrimaryKeyInfo
	* \brief	Primary Keys of a table as fetched using SQLPrimaryKeys
	*/
	struct EXODBCAPI STablePrimaryKeyInfo
	{
		STablePrimaryKeyInfo()
			: m_keySequence(0)
			, m_isPrimaryKeyNameNull(true)
			, m_isCatalogNull(true)
			, m_isSchemaNull(true)
		{};

		std::wstring	m_catalogName;	///< TABLE_CAT [Nullable]. Primary key table catalog name.
		std::wstring	m_schemaName;	///< TABLE_SCHEM [Nullable]. Primary key table schema name.
		std::wstring	m_tableName;	///< TABLE_NAME. Primary key table name.
		std::wstring	m_columnName;	///< COLUMN_NAME. Primary key column name.

		SQLSMALLINT		m_keySequence;	///< KEY_SEQ. Column sequence number in key (starting with 1).
		std::wstring	m_primaryKeyName;	///< PK_NAME [Nullable]. Column sequence number in key (starting with 1).

		bool			m_isCatalogNull;		///< True if TABLE_CAT is Null.
		bool			m_isSchemaNull;			///< True if TABLE_SCHEM is Null.
		bool			m_isPrimaryKeyNameNull;	///< True if PK_NAME is Null.

		std::wstring GetSqlName(QueryNameFlags flags = QNF_TABLE | QNF_COLUMN) const;
	};

	/*!
	* \typedef TablePrimaryKeysVector
	* \brief std::vector of STablePrimaryKeyInfo objects.
	*/
	typedef std::vector<STablePrimaryKeyInfo> TablePrimaryKeysVector;


}

#endif // INFOOBJECT_H