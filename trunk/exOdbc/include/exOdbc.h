/*!
 * \file exOdbc.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 09.02.2014
 * \brief Header file to set up dll import/exports, consts, structs used often, etc.
 * \copyright wxWindows Library Licence, Version 3.1
*/ 

#pragma once
#ifndef EXODBC_H
#define EXODBC_H

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
	// =============

	// Some defaults when binding to chars but no reasonable char-length can be determined.
	const int DB_MAX_BIGINT_CHAR_LENGTH = 30;	///< If no reasonable char length can be determined from a columnInfo, this value is used for the size of the char-buffer (if converting bigints to char)
	const int DB_MAX_DOUBLE_CHAR_LENGTH = 30;	///< If no reasonable char length can be determined from a columnInfo, this value is used for the size of the char-buffer (if converting doubles to char)

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
	// =====
	/*!
	* \enum	OdbcVersion
	* \brief	Defines the ODBC-Version to be set.
	* 			see: http://msdn.microsoft.com/en-us/library/ms709316%28v=vs.85%29.aspx
	*/
	enum class OdbcVersion
	{
		UNKNOWN = 0,			///< Unknown Version
		V_2 = SQL_OV_ODBC2,		///< Version 2.x
		V_3 = SQL_OV_ODBC3,		///< Version 3.x
		V_3_8 = SQL_OV_ODBC3_80	///< Version 3.8
	};


	/*!
	* \enum	CommitMode
	* \brief	Defines whether auto commit is on or off.
	* 			see: http://msdn.microsoft.com/en-us/library/ms713600%28v=vs.85%29.aspx
	*/
	enum class CommitMode
	{
		UNKNOWN = 50000,			///< Unknown Commit mode
		AUTO = SQL_AUTOCOMMIT,		///< Autocommit on
		MANUAL = SQL_AUTOCOMMIT_OFF	///< Autocommit off
	};


	/*!
	* \enum	TransactionIsolationMode
	*
	* \brief	Defines the Transaction Isolation Mode
	*			see: http://msdn.microsoft.com/en-us/library/ms709374%28v=vs.85%29.aspx
	*/
	enum class TransactionIsolationMode
	{
		UNKNOWN = 50000,								///< Unknown Transaction Isolation LEvel
		READ_UNCOMMITTED = SQL_TXN_READ_UNCOMMITTED,	///< Read Uncommitted
		READ_COMMITTED = SQL_TXN_READ_COMMITTED,		///< Read Committed
		REPEATABLE_READ = SQL_TXN_REPEATABLE_READ,		///< Repeatable Read
		SERIALIZABLE = SQL_TXN_SERIALIZABLE				///< Serializable
#if HAVE_MSODBCSQL_H
		, SNAPSHOT = SQL_TXN_SS_SNAPSHOT				///< Snapshot, only for MS SQL Server, and only if HAVE_MSODBCSQL_H is defined
#endif
	};


	/*!
	* \enum		DatabaseProduct
	* \brief	Known databases, identified by their product name while connecting the Database.
	* \details	For the database products listed here, some tests should exists.
	*/
	enum class DatabaseProduct
	{
		UNKNOWN,		///< Unknown DB
		MS_SQL_SERVER,	///< Microsoft SQL Server
		MY_SQL,			///< MySQL
		DB2,			///< IBM DB2
		EXCEL,			///< Microsoft Excel
		ACCESS,			///< Microsoft Access
	};


	/*!
	* \enum		ColumnAttribute
	* \brief	A helper for the arguments in SQLColAttribute.
	* \see		http://msdn.microsoft.com/en-us/library/ms713558%28v=vs.85%29.aspx
	* \see		Table::SelectColumnAttribute()
	*/
	enum class ColumnAttribute
	{
		CA_PRECISION = SQL_DESC_PRECISION ///< A numeric value that for a numeric data type denotes the applicable precision, For data types SQL_TYPE_TIME, SQL_TYPE_TIMESTAMP, and all the interval data types that represent a time interval, its value is the applicable precision of the fractional seconds component. 
	};


	// Flags
	// =====

	/*!
	* \enum ColumnFlag
	* \brief Define flags of a Column.
	*/
	enum ColumnFlag
	{
		CF_NONE = 0x0,		///< No flags.

		CF_SELECT = 0x1,	///< Include Column in Selects.
		CF_UPDATE = 0x2,	///< Include Column in Updates.
		CF_INSERT = 0x4,	///< Include Column in Inserts.
		CF_NULLABLE = 0x8,	///< Column is null able.
		CF_PRIMARY_KEY = 0x10,	///< Column is primary key.

		CF_READ = CF_SELECT,	///< CF_SELECT
		CF_WRITE = CF_UPDATE | CF_INSERT,	///< CF_UPDATE | CF_INSERT
		CF_READ_WRITE = CF_SELECT | CF_UPDATE | CF_INSERT	///< CF_SELECT | CF_UPDATE | CF_INSERT
	};

	/*!
	* \typedef ColumnFlags
	* \brief Flag holder for ColumnFlag flags.
	*/
	typedef unsigned int ColumnFlags;


	/*!
	* \enum AccessFlag
	* \brief Defines how to Access a table.
	*/
	enum AccessFlag
	{
		AF_NONE = 0x0,			///< No AccessFlags, no statements are going to be created.

		AF_SELECT = 0x1,		///< Access for SELECTing.
		
		AF_UPDATE_PK = 0x2,		///< Access for UPDATEing where rows to update are identified by the bound primary key value(s).
		AF_UPDATE_WHERE = 0x4,	///< Access for UPDATEing where rows to update are identified using a manually passed where clause.
		AF_UPDATE = AF_UPDATE_PK | AF_UPDATE_WHERE,	///< AF_UPDATE_PK | AF_UPDATE_WHERE
		
		AF_INSERT = 0x8,		///< Access for INSERTing.

		AF_DELETE_PK = 0x10,	///< Access for DELETEing where rows to delete are identified by the bound primary key value(s).
		AF_DELETE_WHERE = 0x20,	///< Access for DELETEing where rows to delete are identified using a manually passed where clause.
		AF_DELETE = AF_DELETE_PK | AF_DELETE_WHERE,	///< AF_DELETE_PK | AF_DELETE_WHERE

		AF_READ = AF_SELECT,	///< AF_SELECT
		AF_WRITE = AF_UPDATE | AF_INSERT | AF_DELETE,	///<AF_UPDATE | AF_INSERT | AF_DELETE
		AF_READ_WRITE = AF_READ | AF_WRITE	///< AF_READ | AF_WRITE
	};

	/*!
	* \typedef AccessFlags
	* \brief Flag holder for AccessFlag flags.
	*/
	typedef unsigned int AccessFlags;


	/*!
	* \enum TableOpenFlag
	* \brief Defines how to open a table.
	*/
	enum TableOpenFlag
	{
		TOF_NONE = 0x0,				///< No special flags are set.
		TOF_CHECK_EXISTANCE = 0x1,	///< Always check that a table identified by the STableInfo exists.
		TOF_CHECK_PRIVILEGES = 0x2,	///< Check that we have sufficient privileges to open the table for the given AccessFlags.
		TOF_SKIP_UNSUPPORTED_COLUMNS = 0x4,	///< Skip unsupported Columns.
		TOF_CHAR_TRIM_RIGHT = 0x8,	///< If set, string/wstring values accessed through this table are trimmed on the right before being returned as string/string.
		TOF_CHAR_TRIM_LEFT = 0x10,	///< If set, string/wstring values accessed through this table are trimmed on the left before being returned as string/string.
		TOF_DO_NOT_QUERY_PRIMARY_KEYS = 0x20, ///< If set, primary keys are not queried from the Database during Open().
		TOF_IGNORE_DB_TYPE_INFOS = 0x40 ///< If set, the SQL Type info from the Databas is not used to valide the given Columns SQL Data type.
	};

	/*!
	* \typedef TableOpenFlags
	* \brief Flag holder for TableOpenFlag flags.
	*/
	typedef unsigned int TableOpenFlags;


	// Structs
	// -------
}

#endif // EXODBC_H
