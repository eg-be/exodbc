/*!
 * \file exOdbc.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 09.02.2014
 * \brief Header file to set up dll import/exports, consts, structs used often, etc.
 * \copyright GNU Lesser General Public License Version 3
*/ 

#pragma once

// Defines to dll-import/export
// ----------------------------

#ifdef EXODBC_EXPORTS
	#define EXODBCAPI __declspec(dllexport)
#else
	#define EXODBCAPI __declspec(dllimport)
#endif

// class 'foo' needs to have dll-interface to be used by clients of class 'bar'"
// see Ticket #94
#pragma warning(disable:4251)
#include <windows.h>

#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#include <odbcinst.h>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>

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
	* \brief Called by macros to throw Assertions
	* \throw AssertionException if called.
	*/
	extern EXODBCAPI void exOnAssert(const std::wstring& file, int line, const std::wstring& function, const std::wstring& condition, const std::wstring& msg);


	/*!
	* \brief Ugly conversion of small to wide - use only if you know that you have only ASCII chars in w.
	*
	* \details Transforms from wide to small by simply taking the numeric char-values.
	* \param w String to transform
	* \return std::string
	*/
	extern std::string w2s(const std::wstring& w) noexcept;


	/*!
	* \brief Ugly conversion of wide to small - use only if you know that you have only ASCII chars in s.
	*
	* \details Transforms small wide to wide by simply taking the numeric char-values.
	* \param s String to transform
	* \return std::wstring
	*/
	extern std::wstring s2w(const std::string& s) noexcept;


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
}

// Macros
// ------
/*!
* \brief exASSERT_MSG(cond, msg) - MACRO
*
* If cond evalutes to false, this Macro will always call exOnAssert().
*/
#define exASSERT_MSG(cond, msg)										\
do {																\
	if ( !(cond) )  {												\
		exOnAssert(__FILEW__, __LINE__, __FUNCTIONW__,L#cond,msg);	\
	}                                                               \
} while ( 0 )


/*!
* \brief exASSERT(cond) - MACRO
*
* This macro is a simple shorthand to the macro exASSERT_MSG(const, msg), passing an empty message
*/
#define exASSERT(cond) exASSERT_MSG(cond, L"")

// LOG Helpers
// ===========

namespace exodbc
{
	/*!
	* \enum LogLevel
	* \brief Log level to use when logging messages. Currently only sets some prefix..
	*/
	enum class LogLevel
	{
		Error,
		Warning,
		Info,
		Debug
	};


	/*!
	* \brief Format all Infos and Errors from passed handles into something human-readable.
	*/
	extern EXODBCAPI std::wstring FormatOdbcMessages(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt, SQLHDESC hDesc, SQLRETURN ret, std::wstring sqlFunctionName, std::wstring msg, LogLevel logLevel);
}

// Generic Log-entry
#define LOG_MSG(logLevel, msg) \
	do { \
    switch(logLevel) \
	{ \
	case exodbc::LogLevel::Error: \
		std::wcerr << L"ERROR: "; break;\
	case exodbc::LogLevel::Warning: \
		std::wcerr << L"WARNING: "; break;\
	case exodbc::LogLevel::Info: \
		std::wcerr << L"INFO: "; break;\
	case exodbc::LogLevel::Debug: \
		std::wcerr << L"DEBUG: "; break;\
	} \
	std::wcerr << __FILEW__ << L"(" << __LINE__ << L") " << __FUNCTIONW__ << L": " << msg << std::endl; \
	} while( 0 )

// Generic Log-entry shortcuts
#define LOG_ERROR(msg) LOG_MSG(exodbc::LogLevel::Error, msg)
#define LOG_WARNING(msg) LOG_MSG(exodbc::LogLevel::Warning, msg)
#define LOG_INFO(msg) LOG_MSG(exodbc::LogLevel::Info, msg)
#define LOG_DEBUG(msg) LOG_MSG(exodbc::LogLevel::Debug, msg)

// ODBC-Logging
#define LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, logLevel) \
	do { \
		std::wstring logOdbcMsgMsg = exodbc::FormatOdbcMessages(hEnv, hDbc, hStmt, hDesc, ret, L#SqlFunction, msg, logLevel); \
		LOG_MSG(logLevel, logOdbcMsgMsg); \
	} while( 0 )

// ODBC-Loggers, with a message
#define LOG_ERROR_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, exodbc::LogLevel::Error)
#define LOG_WARNING_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, exodbc::LogLevel::Warning)
#define LOG_INFO_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, exodbc::LogLevel::Info)

// ODBC-Loggers, no message
#define LOG_ERROR_ODBC(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction) LOG_ERROR_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, L"")
#define LOG_WARNING_ODBC(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction) LOG_WARNING_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, L"")
#define LOG_INFO_ODBC(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction) LOG_INFO_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, L"")

// Log ODBC-Errors, from a specific handle (env, dbc, stmt, desc), with a message
#define LOG_ERROR_ENV_MSG(hEnv, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(hEnv, NULL, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_ERROR_DBC_MSG(hDbc, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(NULL, hDbc, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_ERROR_STMT_MSG(hStmt, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(NULL, NULL, hStmt, NULL, ret, SqlFunction, msgStr)
#define LOG_ERROR_DESC_MSG(hDesc, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(NULL, NULL, NULL, hDesc, ret, SqlFunction, msgStr)

// Log ODBC-Warnings, from a specific handle (env, dbc or stmt), with a message
#define LOG_WARNING_ENV_MSG(hEnv, ret, SqlFunction, msgStr) LOG_WARNING_ODBC_MSG(hEnv, NULL, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_WARNING_DBC_MSG(hDbc, ret, SqlFunction, msgStr) LOG_WARNING_ODBC_MSG(NULL, hDbc, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_WARNING_STMT_MSG(hStmt, ret, SqlFunction, msgStr) LOG_WARNING_ODBC_MSG(NULL, NULL, hStmt, NULL, ret, SqlFunction, msgStr)
#define LOG_WARNING_DESC_MSG(hDesc, ret, SqlFunction, msgStr) LOG_WARNING_ODBC_MSG(NULL, NULL, NULL, hDesc, ret, SqlFunction, msgStr)

// Log ODBC-Infos, from a specific handle (env, dbc or stmt), with a message
#define LOG_INFO_ENV_MSG(hEnv, ret, SqlFunction, msgStr) LOG_INFO_ODBC_MSG(hEnv, NULL, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_INFO_DBC_MSG(hDbc, ret, SqlFunction, msgStr) LOG_INFO_ODBC_MSG(NULL, hDbc, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_INFO_STMT_MSG(hStmt, ret, SqlFunction, msgStr) LOG_INFO_ODBC_MSG(NULL, NULL, hStmt, NULL, ret, SqlFunction, msgStr)
#define LOG_INFO_DESC_MSG(hDesc, ret, SqlFunction, msgStr) LOG_INFO_ODBC_MSG(NULL, NULL, NULL, hDesc, ret, SqlFunction, msgStr)

// Log ODBC-Errors, from a specific handle (env, dbc or stmt), no message
#define LOG_ERROR_ENV(hEnv, ret, SqlFunction) LOG_ERROR_ODBC(hEnv, NULL, NULL, NULL, ret, SqlFunction)
#define LOG_ERROR_DBC(hDbc, ret, SqlFunction) LOG_ERROR_ODBC(NULL, hDbc, NULL, NULL, ret, SqlFunction)
#define LOG_ERROR_STMT(hStmt, ret, SqlFunction) LOG_ERROR_ODBC(NULL, NULL, hStmt, NULL, ret, SqlFunction)
#define LOG_ERROR_DESC(hDesc, ret, SqlFunction) LOG_ERROR_ODBC(NULL, NULL, NULL, hDesc, ret, SqlFunction)

// Log ODBC-Warnings, from a specific handle (env, dbc or stmt), no message
#define LOG_WARNING_ENV(hEnv, ret, SqlFunction) LOG_WARNING_ODBC(hEnv, NULL, NULL, NULL, ret, SqlFunction)
#define LOG_WARNING_DBC(hDbc, ret, SqlFunction) LOG_WARNING_ODBC(NULL, hDbc, NULL, NULL, ret, SqlFunction)
#define LOG_WARNING_STMT(hStmt, ret, SqlFunction) LOG_WARNING_ODBC(NULL, NULL, hStmt, NULL, ret, SqlFunction)
#define LOG_WARNING_DESC(hDesc, ret, SqlFunction) LOG_WARNING_ODBC(NULL, NULL, hDesc, NULL, ret, SqlFunction)

// Log ODBC-Infos, from a specific handle (env, dbc or stmt), no message
#define LOG_INFO_ENV(hEnv, ret, SqlFunction) LOG_INFO_ODBC(hEnv, NULL, NULL, NULL, ret, SqlFunction)
#define LOG_INFO_DBC(hDbc, ret, SqlFunction) LOG_INFO_ODBC(NULL, hDbc, NULL, NULL, ret, SqlFunction)
#define LOG_INFO_STMT(hStmt, ret, SqlFunction) LOG_INFO_ODBC(NULL, NULL, hStmt, NULL, ret, SqlFunction)
#define LOG_INFO_DESC(hDesc, ret, SqlFunction) LOG_INFO_ODBC(NULL, NULL, NULL, hDesc, ret, SqlFunction)

// Log NO_SUCESS
#define LOG_ERROR_SQL_NO_SUCCESS(ret, SqlFunction) \
	do { \
		std::wstringstream ws; \
		ws << L"ODBC-Function '" < L#SqlFunction << L"' returned " << ret; \
		LOG_MSG(exodbc::LogLevel::Error, ws.str()); \
	} while( 0 )

// Log expected NO_DATA
#define LOG_ERROR_EXPECTED_SQL_NO_DATA(ret, SqlFunction) \
	do { \
		std::wstringstream ws; \
		ws << L"ODBC-Function '" < L#SqlFunction << L"' returned " << ret << L", but we expected SQL_NO_DATA (" << SQL_NO_DATA << L")"; \
		LOG_MSG(exodbc::LogLevel::Error, ws.str()); \
	} while( 0 )

// Exception Helpers
// =================
#define SET_EXCEPTION_SOURCE(Exception) \
	do { \
		Exception.SetSourceInformation(__LINE__, __FILEW__, __FUNCTIONW__); \
	} while(0)

#define THROW_WITH_SOURCE(ExceptionType, msg) \
	do { \
		ExceptionType ex(msg); \
		SET_EXCEPTION_SOURCE(ex); \
		throw ex; \
	} while(0)

// Helpers to throw if not successfully
#define THROW_IFN_SUCCEEDED_SILENT_MSG(sqlFunctionName, sqlReturn, handleType, handle, msg) \
	do { \
		if(!SQL_SUCCEEDED(sqlReturn)) { \
			SqlResultException ex(L#sqlFunctionName, sqlReturn, handleType, handle, msg); \
			SET_EXCEPTION_SOURCE(ex); \
			throw ex; \
		} \
	} while(0)

#define THROW_IFN_SUCCEEDED_MSG(sqlFunctionName, sqlReturn, handleType, handle, msg) \
	do { \
		if(!SQL_SUCCEEDED(sqlReturn)) { \
			SqlResultException ex(L#sqlFunctionName, sqlReturn, handleType, handle, msg); \
			SET_EXCEPTION_SOURCE(ex); \
			throw ex; \
		} \
		if(SQL_SUCCESS_WITH_INFO == sqlReturn) { \
			switch(handleType) { \
			case SQL_HANDLE_ENV: \
				LOG_INFO_ODBC(handle, SQL_NULL_HDBC, SQL_NULL_HSTMT, SQL_NULL_HDESC, sqlReturn, sqlFunctionName); \
				break; \
			case SQL_HANDLE_DBC: \
				LOG_INFO_ODBC(SQL_NULL_HENV, handle, SQL_NULL_HSTMT, SQL_NULL_HDESC, sqlReturn, sqlFunctionName); \
				break; \
			case SQL_HANDLE_STMT: \
				LOG_INFO_ODBC(SQL_NULL_HENV, SQL_NULL_HDBC, handle, SQL_NULL_HDESC, sqlReturn, sqlFunctionName); \
				break; \
			case SQL_HANDLE_DESC: \
				LOG_INFO_ODBC(SQL_NULL_HENV, SQL_NULL_HDBC, SQL_NULL_HSTMT, handle, sqlReturn, sqlFunctionName); \
				break; \
			default: \
				THROW_WITH_SOURCE(IllegalArgumentException, L"Unknown handleType"); \
			} \
		} \
	} while(0)

#define THROW_IFN_SUCCESS_MSG(sqlFunctionName, sqlReturn, handleType, handle, msg) \
	do { \
		if(SQL_SUCCESS != sqlReturn) { \
			SqlResultException ex(L#sqlFunctionName, sqlReturn, handleType, handle, msg); \
			SET_EXCEPTION_SOURCE(ex); \
			throw ex; \
		} \
	} while (0)

#define THROW_IFN_SUCCEEDED(sqlFunctionName, sqlReturn, handleType, handle) \
	do { \
		THROW_IFN_SUCCEEDED_MSG(sqlFunctionName, sqlReturn, handleType, handle, L""); \
	} while(0)

#define THROW_IFN_SUCCESS(sqlFunctionName, sqlReturn, handleType, handle) \
	do { \
		THROW_IFN_SUCCESS_MSG(sqlFunctionName, sqlReturn, handleType, handle, L""); \
	} while(0)

#define THROW_IFN_NO_DATA(sqlFunctionName, sqlReturn) \
	do { \
		if(SQL_NO_DATA != sqlReturn) \
		{ \
			SqlResultException ex(L#sqlFunctionName, ret, L"Expected SQL_NO_DATA."); \
			SET_EXCEPTION_SOURCE(ex); \
			throw ex; \
		} \
	} while(0)

#define THROW_NOT_IMPLEMENTED() \
	do { \
		Exception ex; \
		SET_EXCEPTION_SOURCE(ex); \
		throw ex; \
	} while(0)

// Compiler Helpers
// ================
#define HIDE_UNUSED(object) \
	do { \
		(void)object; \
	} while(0)

