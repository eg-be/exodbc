/*!
 * \file exOdbc.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 09.02.2014
 * \brief Header file to set up dll import/exports, consts, structs used often, etc.
 * \copyright GNU Lesser General Public License Version 3
*/ 

#pragma once

// Some compiler tuning for different platforms
// --------------------------------------------
#ifdef _WIN32
	#include <SDKDDKVer.h>
	#include <windows.h>
	
	#pragma warning(disable: 4251) // 'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
	#ifndef _SCL_SECURE_NO_WARNINGS
		#define _SCL_SECURE_NO_WARNINGS 1	// 'function': was declared deprecated also 'std::<function name>': Function call with parameters that may be unsafe - this call relies on the caller to check that the passed values are correct.
	#endif
	#ifndef _CRT_SECURE_NO_WARNINGS
		#define _CRT_SECURE_NO_WARNINGS 1	// 'function': This function or variable may be unsafe. Consider using strcpy_s instead.
	#endif
#else
    #include <limits.h>
    #define MAX_PATH PATH_MAX
#endif

#ifdef _WIN32
    #define FILENAME __FILEW__
    #define FUNCTIONNAME __FUNCTIONW__
#else
    #define FILENAME utf8ToUtf16(__FILE__)
    #define FUNCTIONNAME utf8ToUtf16(__FUNCTION__)
#endif

#ifdef _WIN32
    #define SQLAPICHARTYPE SQLWCHAR
    #define SQLAPICHARTYPENAME SQL_C_WCHAR
    #define SYSTEMSTRINGTYPE std::wstring
#else
    #define SQLAPICHARTYPE SQLCHAR
    #define SQLAPICHARTYPENAME SQL_C_CHAR
    #define SYSTEMSTRINGTYPE std::string
#endif

#ifdef _WIN32
    #define SQLAPICHARCONVERT(ws) ws
    #define SQLRESCHARCONVERT(ws) ws
    #define SQLAPICHARTYPE_TO_SYSTEMSTRINGTYPE(ws) std::wstring(reinterpret_cast<const wchar_t*>(ws))
#else
    #define SQLAPICHARCONVERT(ws) utf16ToUtf8(ws)
    #define SQLRESCHARCONVERT(s) utf8ToUtf16(s)
    #define SQLAPICHARTYPE_TO_SYSTEMSTRINGTYPE(s) std::string(reinterpret_cast<const char*>(s))
#endif

// Defines to dll-import/export
// ----------------------------

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
  #define EXODBC_HELPER_DLL_IMPORT __declspec(dllimport)
  #define EXODBC_HELPER_DLL_EXPORT __declspec(dllexport)
  #define EXODBC_HELPER_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define EXODBC_HELPER_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define EXODBC_HELPER_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define EXODBC_HELPER_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define EXODBC_HELPER_DLL_IMPORT
    #define EXODBC_HELPER_DLL_EXPORT
    #define EXODBC_HELPER_DLL_LOCAL
  #endif
#endif

// Now we use the generic helper definitions above to define FOX_API and FOX_LOCAL.
// FOX_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// FOX_LOCAL is used for non-api symbols.

#ifdef EXODBC_DLL // defined if FOX is compiled as a DLL
  #ifdef libexodbc_EXPORTS // defined if we are building the FOX DLL (instead of using it)
    #define EXODBCAPI EXODBC_HELPER_DLL_EXPORT
  #else
    #define EXODBCAPI EXODBC_HELPER_DLL_IMPORT
  #endif // FOX_DLL_EXPORTS
  #define EXODBCLOCAL EXODBC_HELPER_DLL_LOCAL
#else // FOX_DLL is not defined: this means FOX is a static lib.
  #define EXODBCAPI
  #define EXODBCLOCAL
#endif // FOX_DLL

// #ifdef libexodbc_EXPORTS
// 	#define EXODBCAPI __declspec(dllexport)
// #else
// 	#ifdef libexodbc_IMPORTS
// 		#define EXODBCAPI __declspec(dllimport)
// 	#else
// 		#define EXODBCAPI
// 	#endif
// #endif

// libs - boost stuff
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/signals2.hpp"
#include "boost/lexical_cast.hpp"
#include "boost/variant.hpp"

// System includes
#include <locale>
#include <codecvt>
#include <string>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include <map>
#include <algorithm>

// odbc-things
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#include <odbcinst.h>

namespace exodbc
{
	// Global Consts
	// =============

	const int DB_MAX_CONNECTSTR_LEN = 1024;	///< SQLDriverConnects returns an output-connection string, assume this is the max-length of this string returned. MS recommends at least 1024 characters.

	// Some defaults when binding to chars but no reasonable char-length can be determined.
	const int DB_MAX_BIGINT_CHAR_LENGTH = 30;	///< If no reasonable char length can be determined from a columnInfo, this value is used for the size of the char-buffer (if converting bigints to char)
	const int DB_MAX_DOUBLE_CHAR_LENGTH = 30;	///< If no reasonable char length can be determined from a columnInfo, this value is used for the size of the char-buffer (if converting doubles to char)

	// Database Globals or defaults. The values named _DEFAULT are used as fallback
	// if the corresponding value cannot be determined when querying the database about itself.
	const int DB_MAX_TYPE_NAME_LEN				= 40;
	const int DB_MAX_LOCAL_TYPE_NAME_LEN		= 256;
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


	/*!
	* \enum		RowDescriptorType
	* \brief	A wrapper for the values of SQLGetStmtAttr to fetch a descriptor handle.
	* \see		GetRowDescriptorHandle
	*/
	enum class RowDescriptorType
	{
		ROW = SQL_ATTR_APP_ROW_DESC,	///< SQL_ATTR_APP_ROW_DESC
		PARAM = SQL_ATTR_APP_PARAM_DESC	///< SQL_ATTR_APP_PARAM_DESC
	};


	/*!
	* \enum		ConnectionPooling
	* \brief	Wrapper around the values for connection pooling
	*/
	enum class ConnectionPooling
	{
		OFF = SQL_CP_OFF,	///< Connection Pooling disabled
		PER_DRIVER = SQL_CP_ONE_PER_DRIVER, ///< One pool per driver
		PER_HENV = SQL_CP_ONE_PER_HENV ///< One pool per environment
	};


	/*!
	* \enum		ConnectionPoolingMatch
	* \brief	Attributes values for an environment if it works in
	*			strict match mode or relaxed mode if connection pooling
	*			is enabled.
	*/
	enum class ConnectionPoolingMatch
	{
		STRICT_MATCH = SQL_CP_STRICT_MATCH,
		RELAXED_MATCH = SQL_CP_RELAXED_MATCH
	};


	/*!
	* \enum		IdentifierType
	* \brief	Attribute values to query special columns: Type of special columns to query.
	*/
	enum class IdentifierType
	{
		IDENTIFY_ROW_UNIQUELY = SQL_BEST_ROWID,	///< Optimal set to identify a row uniquely. Can also be pseudo-columns like ROWID
		AUTO_UPDATED = SQL_ROWVER	///< Set of columns that are updated automatically if any value in the row changes.
	};


	/*!
	* \enum		RowIdScope
	* \brief	Attribute values to query special columns: Scope of row-id values.
	*/
	enum class RowIdScope
	{
		CURSOR = SQL_SCOPE_CURROW,	///< Row id values are valid only while cursor is positioned on that row. Might change on later select of the same row.
		TRANSCATION = SQL_SCOPE_TRANSACTION, ///< Row id values are valid during ongoing transaction.
		SESSION = SQL_SCOPE_SESSION	///< Row id values are valid beyond transaction boundaries.
	};


	/*!
	* \enum		PseudoColumn
	* \brief	Information if a column is a pseudo column or not.
	*/
	enum class PseudoColumn
	{
		UNKNOWN = SQL_PC_UNKNOWN,	///< Not known
		NOT_PSEUDO = SQL_PC_NOT_PSEUDO,	///< no pseudo column
		PSEUDO = SQL_PC_PSEUDO	///< pseudo column, like Oracle ROWID
	};


	// Flags
	// =====

	// Structs
	// -------
	/*!
	* \struct SErrorInfo
	*
	* \brief Store error-information from ODBC.
	* \see		GetAllErrors()
	*/
	struct EXODBCAPI SErrorInfo
	{
		SErrorInfo()
		{
			SqlState[0] = 0;
			NativeError = 0;
			ErrorHandleType = 0;
		}

		SQLSMALLINT		ErrorHandleType; ///< Handle-type of the error. Is either SQL_HANDLE_ENV, SQL_HANDLE_DBC, SQL_HANDLE_STMT or SQL_HANDLE_DESC
		SQLWCHAR		SqlState[5 + 1];
		SQLINTEGER		NativeError;
		std::wstring	Msg;

		std::wstring ToString() const;
	};

	/*!
	* \typedef SErrorInfoVector
	* \brief	Vector of SErrorInfo structs.
	*/
	typedef std::vector<SErrorInfo> SErrorInfoVector;

	// Global Helpers
	// --------------

	/*!
	* \brief Converts a utf16 std::wstring to a utf-8 std::string.
	*
	* \details This method will never throw an exception but return garbage in case it failed.
	* \param w String to transform
	* \return std::string
	*/
	extern EXODBCAPI std::string utf16ToUtf8(const std::wstring& w) noexcept;

 	extern EXODBCAPI std::string utf16ToUtf8(const SQLWCHAR* w) noexcept;

	/*!
	* \brief Converts a utf8 std::string to a uf16 std::wstring
	*
	* \details This method will never throw an exception but return garbage in case it failed.
	* \param s String to transform
	* \return std::wstring
	*/
	extern EXODBCAPI std::wstring utf8ToUtf16(const std::string& s) noexcept;

    extern EXODBCAPI std::wstring utf8ToUtf16(const SQLCHAR* s) noexcept;


	/*!
	* \brief Returns the string TRUE, FALSE or ????? for the values SQL_TRUE, SQL_FALSE or anything else.
	*
	* \param b SQL_TRUE or SQL_FALSE
	* \return std::wstring TRUE, FALSE or ?????
	*/
	extern EXODBCAPI std::wstring SqlTrueFalse2s(SQLSMALLINT b) noexcept;


	/*!
	* \brief Translates some often encountered SQLRETURN values to a string.
	*
	* \param ret Return code to translate.
	* \return std::wstring Translation or '???' if unknown.
	*/
	extern EXODBCAPI std::wstring SqlReturn2s(SQLRETURN ret) noexcept;


	/*!
	* \brief Transform the SQL_types like SQL_CHAR, SQL_NUMERIC, etc. to some string.
	*
	* \param sqlType SQL Type..
	* \return std::wstring
	*/
	extern EXODBCAPI std::wstring SqlType2s(SQLSMALLINT sqlType) noexcept;


	/*!
	* \brief Transform the SQL C-types like SQL_C_SLONG, SQL_C_WCHAR, etc. to some string, like "SQL_C_SLONG", etc.
	*
	* \param sqlCType Sql-C Type..
	* \return std::wstring
	*/
	extern EXODBCAPI std::wstring SqLCType2s(SQLSMALLINT sqlCType) noexcept;


	/*!
	* \brief	Transform the SQL C-types like SQL_C_SLONG, SQL_C_WCHAR, etc. to the corresponding string value of the ODBC C Type.
	*			Like SQL_C_SLONG becomes "SQLINTEGER"
	*
	* \param sqlCType Sql-C Type.
	* \return std::wstring
	*/
	extern EXODBCAPI std::wstring SqlCType2OdbcS(SQLSMALLINT sqlCType) noexcept;


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
	* \return std::wstring
	*/
	extern EXODBCAPI std::wstring DatabaseProcudt2s(DatabaseProduct dbms) noexcept;


	/*!
	* \brief Returns ENV, DBC, STMT, DESC or ???
	*/
	extern EXODBCAPI std::wstring HandleType2s(SQLSMALLINT type) noexcept;


	/*!
	* \brief	Gets all errors for all passed handles. Should never throw as long
	*			you pass in at least one non-NULL handle.
	*
	* \param	hEnv 	(Optional) the environment.
	* \param	hDbc 	(Optional) the dbc.
	* \param	hStmt	(Optional) the statement.
	* \param	hStmt	(Optional) the description.
	* \param	hDesc	(Optional) the description handle.
	*
	* \return	all errors.
	* \throw	AssertionException if not at least one of the handles is not a NULL handle.
	*			This function will not throw if reading the errors from SQLGetDiagRec fails,
	*			it will just log a warning.
	*/
	extern EXODBCAPI SErrorInfoVector GetAllErrors(SQLHANDLE hEnv, SQLHANDLE hDbc, SQLHANDLE hStmt, SQLHANDLE hDesc);


	/*!
	* \brief	A shorthand to GetAllErrors(SQLHANDLE hEnv, SQLHANDLE hDbc, SQLHANDLE hStmt, SQLHANDLE hDesc)
	*			you pass in at least one non-NULL handle.
	* \see		GetAllErrors(SQLHANDLE hEnv, SQLHANDLE hDbc, SQLHANDLE hStmt, SQLHANDLE hDesc)
	*/
	extern EXODBCAPI SErrorInfoVector GetAllErrors(SQLSMALLINT handleType, SQLHANDLE handle);


	/*!
	* \brief Format all Infos and Errors from passed handles into something human-readable.
	*/
	extern EXODBCAPI std::wstring FormatOdbcMessages(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt, SQLHDESC hDesc, SQLRETURN ret, std::wstring sqlFunctionName, std::wstring msg);
    
	extern EXODBCAPI std::wstring FormatOdbcMessages(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt, SQLHDESC hDesc, SQLRETURN ret, std::string sqlFunctionName, std::wstring msg);    
}


// Compiler Helpers
// ================
#define HIDE_UNUSED(object) \
	do { \
		(void)object; \
	} while(0)

