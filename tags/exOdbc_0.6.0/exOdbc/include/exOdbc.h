/*!
 * \file exOdbc.h
 * \author Elias Gerber <eg@zame.ch>
 * \date 09.02.2014
 * \brief Header file to set up dll import/exports, consts, structs used often, etc.
*/ 

#pragma once
#ifndef WXODBC3_H
#define WXODBC3_H

// Defines to dll-import/export
// ----------------------------

#ifdef EXODBC_EXPORTS
	#define EXODBCAPI __declspec(dllexport)
#else
	#define EXODBCAPI __declspec(dllimport)
#endif

/* There are too many false positives for this one, particularly when using templates like wxVector<T> */
/* class 'foo' needs to have dll-interface to be used by clients of class 'bar'" */
#pragma warning(disable:4251)

#include <windows.h>

#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#include <odbcinst.h>
#if HAVE_MSODBCSQL_H
	#include "msodbcsql.h"
#endif

namespace exodbc
{
	// Global Consts
	// -------------

	// Some defaults when binding to chars but no reasonable char-length can be determined.
	const int DB_MAX_BIGINT_CHAR_LENGTH = 30;
	const int DB_MAX_DOUBLE_CHAR_LENGTH = 30;

	// Database Globals or defaults. The values named _DEFAULT are used as fallback
	// if the corresponding value cannot be determined when querying the database about itself.
	const int DB_MAX_TYPE_NAME_LEN				= 40;
	const int DB_MAX_LOCAL_TYPE_NAME_LEN		= 256;
//	const int DB_MAX_STATEMENT_LEN			= 4096;
//	const int DB_MAX_WHERE_CLAUSE_LEN		= 2048;
	const int DB_MAX_TABLE_NAME_LEN_DEFAULT			= 128;	///< This value is sometimes also available from SDbInfo::maxTableNameLen
	const int DB_MAX_SCHEMA_NAME_LEN_DEFAULT		= 128;	///< This value is sometimes also available from SDbInfo::maxSchemaNameLen
	const int DB_MAX_CATALOG_NAME_LEN_DEFAULT		= 128;	///< This value is sometimes also available from SDbInfo::maxCatalogNameLen
	const int DB_MAX_COLUMN_NAME_LEN_DEFAULT		= 128;	///< Value sometimes available from SdbInfo::m_maxColumnNameLen
	const int DB_MAX_TABLE_TYPE_LEN			= 128;
	const int DB_MAX_TABLE_REMARKS_LEN		= 512;
	const int DB_MAX_COLUMN_REMARKS_LEN		= 512;
	const int DB_MAX_COLUMN_DEFAULT_LEN		= 512;
	const int DB_MAX_LITERAL_PREFIX_LEN		= 128;
	const int DB_MAX_LITERAL_SUFFIX_LEN		= 128;
	const int DB_MAX_CREATE_PARAMS_LIST_LEN = 512;	
	const int DB_MAX_GRANTOR_LEN			= 128;
	const int DB_MAX_GRANTEE_LEN			= 128;
	const int DB_MAX_PRIVILEGES_LEN			= 128;
	const int DB_MAX_IS_GRANTABLE_LEN		= 4;
	const int DB_MAX_YES_NO_LEN				= 3;
	const int DB_MAX_PRIMARY_KEY_NAME_LEN	= 128;


	// Enums
	// -----
	/*!
	* \enum	OdbcVersion
	*
	* \brief	Defines the ODBC-Version to be set.
	* 			see: http://msdn.microsoft.com/en-us/library/ms709316%28v=vs.85%29.aspx
	*
	*/
	enum OdbcVersion
	{
		OV_UNKNOWN = 0, //< Unknown Version
		OV_2 = 2UL,		//< Version 2.x
		OV_3 = 3UL,		//< Version 3.x
		OV_3_8 = 380UL	//< Version 3.8
	};

	/*!
	* \enum	CommitMode
	*
	* \brief	Defines whether auto commit is on or off.
	* 			see: http://msdn.microsoft.com/en-us/library/ms713600%28v=vs.85%29.aspx
	*
	*/
	enum CommitMode
	{
		CM_UNKNOWN = 50000,			//< Unknown Commit mode
		CM_AUTO_COMMIT = SQL_AUTOCOMMIT,		//< Autocommit on
		CM_MANUAL_COMMIT = SQL_AUTOCOMMIT_OFF	//< Autocommit off
	};


	/*!
	* \enum	TransactionIsolationMode
	*
	* \brief	Defines the Transaction Isolation Mode
	*			see: http://msdn.microsoft.com/en-us/library/ms709374%28v=vs.85%29.aspx
	*/
	enum TransactionIsolationMode
	{
		TI_UNKNOWN = 50000,
		TI_READ_UNCOMMITTED = SQL_TXN_READ_UNCOMMITTED,
		TI_READ_COMMITTED = SQL_TXN_READ_COMMITTED,
		TI_REPEATABLE_READ = SQL_TXN_REPEATABLE_READ,
		TI_SERIALIZABLE = SQL_TXN_SERIALIZABLE
#if HAVE_MSODBCSQL_H
		, TI_SNAPSHOT = SQL_TXN_SS_SNAPSHOT
#endif
	};


	/*!
	* \enum		AutoBindingMode
	* \brief	Provide additional information to what types to bind columns.
	* \detailed Usually when a table binds a column it will query the database about the SQL-Type and create the
	*			corresponding buffer-type. Using this option you can specify that columns reported as SQL_CHAR
	*			will be bound to a SQLWCHAR* buffer, or the other way round, or that everything is bound to a
	*			SQLCHAR* / SQLWCHAR*.
	*			This comes in handy as drivers are usually quite good about converting wide-stuff to non-wide stuff,
	*			but doing that in the code is just a pain.
	*/
	enum class AutoBindingMode
	{
		BIND_AS_REPORTED,	///< Use the type reported by the DB for the buffer (default)
		BIND_WCHAR_AS_CHAR,		///< Bind also SQL_WCHAR and SQL_WVARCHAR columns to a SQLCHAR* buffer
		BIND_CHAR_AS_WCHAR,		///< Bind also SQL_CHAR and SQL_VARCHAR columns to a SQLWCHAR* buffer
		BIND_ALL_AS_CHAR,		///< Bind all columns as SQLCHAR, ignoring their type
		BIND_ALL_AS_WCHAR		///< Bind all columns as SQLWCHAR* ignoring their type
	};


	// Known Databases
	// ---------------
	// These are the databases currently tested and working with these classes
	// See the comments in wxDb::Dbms() for exceptions/issues with
	// each of these database engines
	enum DatabaseProduct
	{
		dbmsUNIDENTIFIED,	///< Unknown DB
		//dbmsORACLE,
		//dbmsSYBASE_ASA,        // Adaptive Server Anywhere
		//dbmsSYBASE_ASE,        // Adaptive Server Enterprise
		dbmsMS_SQL_SERVER,	///< Microsoft SQL Server
		dbmsMY_SQL,			///< MySQL
		//dbmsPOSTGRES,
		//dbmsACCESS,
		//dbmsDBASE,
		//dbmsINFORMIX,
		//dbmsVIRTUOSO,
		dbmsDB2,			///< IBM DB2
		//dbmsINTERBASE,
		//dbmsPERVASIVE_SQL,
		//dbmsXBASE_SEQUITER,
		//dbmsFIREBIRD,
		//dbmsMAXDB,
	};


	/*!
	* \enum		ColumnAttribute
	* \brief	A helper for the arguments in SQLColAttribute.
	* \see		http://msdn.microsoft.com/en-us/library/ms713558%28v=vs.85%29.aspx
	* \see		Table::SelectColumnAttribute()
	*/
	enum ColumnAttribute
	{
		CA_PRECISION = SQL_DESC_PRECISION ///< A numeric value that for a numeric data type denotes the applicable precision, For data types SQL_TYPE_TIME, SQL_TYPE_TIMESTAMP, and all the interval data types that represent a time interval, its value is the applicable precision of the fractional seconds component. 
	};


	enum QueryNameFlags
	{
		CATALOG = 0x1,
		SCHEMA = 0x2,
		TABLE = 0x4,
		TYPE = 0x8,
		COLUMN = 0x10
	};


	enum ColumnFlags
	{
		CF_SELECT = 0x1,
		CF_UPDATE = 0x2,
		CF_INSERT = 0x4,
		CF_NULLABLE = 0x8,
		CF_PRIMARY_KEY = 0x10,

		CF_READ = CF_SELECT,
		CF_WRITE = CF_UPDATE | CF_INSERT,
		CF_READ_WRITE = CF_SELECT | CF_UPDATE | CF_INSERT
	};


	// Structs
	// -------

	/*!
	 * \class	SDbInfo
	 *
	 * \brief	The following structure contains database information gathered from the datasource
	 * 			when the datasource is first Opened.
	 */
	struct EXODBCAPI SDbInfo
	{
		SDbInfo();

		~SDbInfo()
		{
			int p = 3;
		}

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
		SQLUSMALLINT	m_maxStmts;						///< Maximum # of SQLHSTMTs per SQLHDBC
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
	 * \class	SSqlTypeInfo
	 *
	 * \brief	Contains DataType informations read from the database uppon Open().
	 * 			See http://msdn.microsoft.com/en-us/library/ms714632%28v=vs.85%29.aspx
	 * 			
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
	 * \class	SColumnInfo
	 *
	 * \brief	Information about a column fetched using the catalog
	 * 			function SQLColumns.
	 * 			See: http://msdn.microsoft.com/en-us/library/ms711683%28v=vs.85%29.aspx
	 */
	struct EXODBCAPI SColumnInfo
	{
	public:
		SColumnInfo();

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

		bool			m_isCatalogNull;			///< See SColumnInfo::m_catalogName
		bool			m_isSchemaNull;				///< See SColumnInfo::m_schemaName
		bool			m_isColumnSizeNull;			///< See SColumnInfo::m_columnSize
		bool			m_isBufferSizeNull;			///< See SColumnInfo::m_bufferSize
		bool			m_isDecimalDigitsNull;		///< See SColumnInfo::m_decimalDigits
		bool			m_isNumPrecRadixNull;		///< See SColumnInfo::m_numPrecRadix
		bool			m_isRemarksNull;			///< See SColumnInfo::m_remarks
		bool			m_isDefaultValueNull;		///< See SColumnInfo::m_defaultValue
		bool			m_isDatetimeSubNull;		///< See SColumnInfo::m_sqlDatetimeSub
		bool			m_isCharOctetLengthNull;	///< See SColumnInfo::m_charOctetLength
		bool			m_isIsNullableNull;			///< See SColumnInfo::isNullable

		bool				HasSchema() const { return !m_isSchemaNull && m_schemaName.length() > 0; };
		bool				HasCatalog() const { return !m_isCatalogNull && m_catalogName.length() > 0; };

		std::wstring		GetSqlName(int flags = TABLE | COLUMN) const;

	};

	/*!
	 * \class	STableInfo
	 *
	 * \brief	Description of a table fetched using the catalog function SQLTables
	 */
	struct EXODBCAPI STableInfo        // Description of a Table: Used only in the Description of a database, (catalog info)
	{
	public:
		STableInfo();

		std::wstring		m_tableName;
		std::wstring		m_tableType;           // "TABLE" or "SYSTEM TABLE" etc.
		std::wstring		m_tableRemarks;
		std::wstring		m_catalogName;
		std::wstring		m_schemaName;
		bool				m_isCatalogNull;
		bool				m_isSchemaNull;

		bool				HasSchema() const { return !m_isSchemaNull && m_schemaName.length() > 0; };
		bool				HasCatalog() const { return !m_isCatalogNull && m_catalogName.length() > 0; };

		std::wstring		GetSqlName(int flags = CATALOG | SCHEMA | TABLE) const;
	};

	/*!
	 * \class	SDbCatalogInfo
	 *
	 * \brief	Description of the catalog of a database
	 */
	struct EXODBCAPI SDbCatalogInfo
	{
		std::vector<STableInfo> m_tables;
		std::set<std::wstring> m_catalogs;
		std::set<std::wstring> m_schemas;
	};

	/*!
	 * \class	STablePrivilegesInfo
	 *
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
	typedef std::vector<STablePrivilegesInfo> TablePrivilegesVector;


	/*!
	* \class	STablePrimaryKeyInfo
	*
	* \brief	Primary Keys of a table as fetched using SQLPrimaryKeys
	*/
	struct EXODBCAPI STablePrimaryKeyInfo
	{
		STablePrimaryKeyInfo() : m_keySequence(0), m_isPrimaryKeyNameNull(true) {};

		std::wstring	m_catalogName;	///< TABLE_CAT [Nullable]. Primary key table catalog name.
		std::wstring	m_schemaName;	///< TABLE_SCHEM [Nullable]. Primary key table schema name.
		std::wstring	m_tableName;	///< TABLE_NAME. Primary key table name.
		std::wstring	m_columnName;	///< COLUMN_NAME. Primary key column name.

		SQLSMALLINT		m_keySequence;	///< KEY_SEQ. Column sequence number in key (starting with 1).
		std::wstring	m_primaryKeyName;	///< PK_NAME [Nullable]. Column sequence number in key (starting with 1).

		bool			m_isCatalogNull;		///< True if TABLE_CAT is Null.
		bool			m_isSchemaNull;			///< True if TABLE_SCHEM is Null.
		bool			m_isPrimaryKeyNameNull;	///< True if PK_NAME is Null.

		std::wstring GetSqlName(int flags = TABLE | COLUMN) const;
	};
	typedef std::vector<STablePrimaryKeyInfo> TablePrimaryKeysVector;


	// List the well-known SQL-State we need in the code
	namespace SqlState
	{
		const wchar_t INVALID_CURSOR_STATE[6] = L"24000";
	}


	//// ODBC Error codes (derived from ODBC SqlState codes)
	//enum wxODBC_ERRORS
	//{
	//	DB_FAILURE                        = 0,
	//	DB_SUCCESS                        = 1,
	//	DB_ERR_NOT_IN_USE,
	//	DB_ERR_GENERAL_WARNING,                            // SqlState = '01000'
	//	DB_ERR_DISCONNECT_ERROR,                           // SqlState = '01002'
	//	DB_ERR_DATA_TRUNCATED,                             // SqlState = '01004'
	//	DB_ERR_PRIV_NOT_REVOKED,                           // SqlState = '01006'
	//	DB_ERR_INVALID_CONN_STR_ATTR,                      // SqlState = '01S00'
	//	DB_ERR_ERROR_IN_ROW,                               // SqlState = '01S01'
	//	DB_ERR_OPTION_VALUE_CHANGED,                       // SqlState = '01S02'
	//	DB_ERR_NO_ROWS_UPD_OR_DEL,                         // SqlState = '01S03'
	//	DB_ERR_MULTI_ROWS_UPD_OR_DEL,                      // SqlState = '01S04'
	//	DB_ERR_WRONG_NO_OF_PARAMS,                         // SqlState = '07001'
	//	DB_ERR_DATA_TYPE_ATTR_VIOL,                        // SqlState = '07006'
	//	DB_ERR_UNABLE_TO_CONNECT,                          // SqlState = '08001'
	//	DB_ERR_CONNECTION_IN_USE,                          // SqlState = '08002'
	//	DB_ERR_CONNECTION_NOT_OPEN,                        // SqlState = '08003'
	//	DB_ERR_REJECTED_CONNECTION,                        // SqlState = '08004'
	//	DB_ERR_CONN_FAIL_IN_TRANS,                         // SqlState = '08007'
	//	DB_ERR_COMM_LINK_FAILURE,                          // SqlState = '08S01'
	//	DB_ERR_INSERT_VALUE_LIST_MISMATCH,                 // SqlState = '21S01'
	//	DB_ERR_DERIVED_TABLE_MISMATCH,                     // SqlState = '21S02'
	//	DB_ERR_STRING_RIGHT_TRUNC,                         // SqlState = '22001'
	//	DB_ERR_NUMERIC_VALUE_OUT_OF_RNG,                   // SqlState = '22003'
	//	DB_ERR_ERROR_IN_ASSIGNMENT,                        // SqlState = '22005'
	//	DB_ERR_DATETIME_FLD_OVERFLOW,                      // SqlState = '22008'
	//	DB_ERR_DIVIDE_BY_ZERO,                             // SqlState = '22012'
	//	DB_ERR_STR_DATA_LENGTH_MISMATCH,                   // SqlState = '22026'
	//	DB_ERR_INTEGRITY_CONSTRAINT_VIOL,                  // SqlState = '23000'
	//	DB_ERR_INVALID_CURSOR_STATE,                       // SqlState = '24000'
	//	DB_ERR_INVALID_TRANS_STATE,                        // SqlState = '25000'
	//	DB_ERR_INVALID_AUTH_SPEC,                          // SqlState = '28000'
	//	DB_ERR_INVALID_CURSOR_NAME,                        // SqlState = '34000'
	//	DB_ERR_SYNTAX_ERROR_OR_ACCESS_VIOL,                // SqlState = '37000'
	//	DB_ERR_DUPLICATE_CURSOR_NAME,                      // SqlState = '3C000'
	//	DB_ERR_SERIALIZATION_FAILURE,                      // SqlState = '40001'
	//	DB_ERR_SYNTAX_ERROR_OR_ACCESS_VIOL2,               // SqlState = '42000'
	//	DB_ERR_OPERATION_ABORTED,                          // SqlState = '70100'
	//	DB_ERR_UNSUPPORTED_FUNCTION,                       // SqlState = 'IM001'
	//	DB_ERR_NO_DATA_SOURCE,                             // SqlState = 'IM002'
	//	DB_ERR_DRIVER_LOAD_ERROR,                          // SqlState = 'IM003'
	//	DB_ERR_SQLALLOCENV_FAILED,                         // SqlState = 'IM004'
	//	DB_ERR_SQLALLOCCONNECT_FAILED,                     // SqlState = 'IM005'
	//	DB_ERR_SQLSETCONNECTOPTION_FAILED,                 // SqlState = 'IM006'
	//	DB_ERR_NO_DATA_SOURCE_DLG_PROHIB,                  // SqlState = 'IM007'
	//	DB_ERR_DIALOG_FAILED,                              // SqlState = 'IM008'
	//	DB_ERR_UNABLE_TO_LOAD_TRANSLATION_DLL,             // SqlState = 'IM009'
	//	DB_ERR_DATA_SOURCE_NAME_TOO_LONG,                  // SqlState = 'IM010'
	//	DB_ERR_DRIVER_NAME_TOO_LONG,                       // SqlState = 'IM011'
	//	DB_ERR_DRIVER_KEYWORD_SYNTAX_ERROR,                // SqlState = 'IM012'
	//	DB_ERR_TRACE_FILE_ERROR,                           // SqlState = 'IM013'
	//	DB_ERR_TABLE_OR_VIEW_ALREADY_EXISTS,               // SqlState = 'S0001'
	//	DB_ERR_TABLE_NOT_FOUND,                            // SqlState = 'S0002'
	//	DB_ERR_INDEX_ALREADY_EXISTS,                       // SqlState = 'S0011'
	//	DB_ERR_INDEX_NOT_FOUND,                            // SqlState = 'S0012'
	//	DB_ERR_COLUMN_ALREADY_EXISTS,                      // SqlState = 'S0021'
	//	DB_ERR_COLUMN_NOT_FOUND,                           // SqlState = 'S0022'
	//	DB_ERR_NO_DEFAULT_FOR_COLUMN,                      // SqlState = 'S0023'
	//	DB_ERR_GENERAL_ERROR,                              // SqlState = 'S1000'
	//	DB_ERR_MEMORY_ALLOCATION_FAILURE,                  // SqlState = 'S1001'
	//	DB_ERR_INVALID_COLUMN_NUMBER,                      // SqlState = 'S1002'
	//	DB_ERR_PROGRAM_TYPE_OUT_OF_RANGE,                  // SqlState = 'S1003'
	//	DB_ERR_SQL_DATA_TYPE_OUT_OF_RANGE,                 // SqlState = 'S1004'
	//	DB_ERR_OPERATION_CANCELLED,                        // SqlState = 'S1008'
	//	DB_ERR_INVALID_ARGUMENT_VALUE,                     // SqlState = 'S1009'
	//	DB_ERR_FUNCTION_SEQUENCE_ERROR,                    // SqlState = 'S1010'
	//	DB_ERR_OPERATION_INVALID_AT_THIS_TIME,             // SqlState = 'S1011'
	//	DB_ERR_INVALID_TRANS_OPERATION_CODE,               // SqlState = 'S1012'
	//	DB_ERR_NO_CURSOR_NAME_AVAIL,                       // SqlState = 'S1015'
	//	DB_ERR_INVALID_STR_OR_BUF_LEN,                     // SqlState = 'S1090'
	//	DB_ERR_DESCRIPTOR_TYPE_OUT_OF_RANGE,               // SqlState = 'S1091'
	//	DB_ERR_OPTION_TYPE_OUT_OF_RANGE,                   // SqlState = 'S1092'
	//	DB_ERR_INVALID_PARAM_NO,                           // SqlState = 'S1093'
	//	DB_ERR_INVALID_SCALE_VALUE,                        // SqlState = 'S1094'
	//	DB_ERR_FUNCTION_TYPE_OUT_OF_RANGE,                 // SqlState = 'S1095'
	//	DB_ERR_INF_TYPE_OUT_OF_RANGE,                      // SqlState = 'S1096'
	//	DB_ERR_COLUMN_TYPE_OUT_OF_RANGE,                   // SqlState = 'S1097'
	//	DB_ERR_SCOPE_TYPE_OUT_OF_RANGE,                    // SqlState = 'S1098'
	//	DB_ERR_NULLABLE_TYPE_OUT_OF_RANGE,                 // SqlState = 'S1099'
	//	DB_ERR_UNIQUENESS_OPTION_TYPE_OUT_OF_RANGE,        // SqlState = 'S1100'
	//	DB_ERR_ACCURACY_OPTION_TYPE_OUT_OF_RANGE,          // SqlState = 'S1101'
	//	DB_ERR_DIRECTION_OPTION_OUT_OF_RANGE,              // SqlState = 'S1103'
	//	DB_ERR_INVALID_PRECISION_VALUE,                    // SqlState = 'S1104'
	//	DB_ERR_INVALID_PARAM_TYPE,                         // SqlState = 'S1105'
	//	DB_ERR_FETCH_TYPE_OUT_OF_RANGE,                    // SqlState = 'S1106'
	//	DB_ERR_ROW_VALUE_OUT_OF_RANGE,                     // SqlState = 'S1107'
	//	DB_ERR_CONCURRENCY_OPTION_OUT_OF_RANGE,            // SqlState = 'S1108'
	//	DB_ERR_INVALID_CURSOR_POSITION,                    // SqlState = 'S1109'
	//	DB_ERR_INVALID_DRIVER_COMPLETION,                  // SqlState = 'S1110'
	//	DB_ERR_INVALID_BOOKMARK_VALUE,                     // SqlState = 'S1111'
	//	DB_ERR_DRIVER_NOT_CAPABLE,                         // SqlState = 'S1C00'
	//	DB_ERR_TIMEOUT_EXPIRED                             // SqlState = 'S1T00'
	//};

	//// SQL-Log-State
	//enum wxDbSqlLogState
	//{
	//	sqlLogOFF,
	//	sqlLogON
	//};


}

#endif // WXODBC3_H
