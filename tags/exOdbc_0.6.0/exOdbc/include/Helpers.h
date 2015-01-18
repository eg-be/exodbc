/*!
* \file Helpers.h
* \author Elias Gerber <egerber@gmx.net>
* \date 23.07.2014
* \brief Header file for generic Helpers, mostly about error-handling and assertions.
*/ 

#pragma once
#ifndef HELPERS_H
#define HELPERS_H

// Same component headers
#include "exOdbc.h"

// Other headers
#include "boost/log/trivial.hpp"

// System headers
#include <string>
#include <iostream>

// Forward declarations
// --------------------
namespace exodbc
{
	extern void exOnAssert(const std::wstring& file, int line, const std::wstring& function, const std::wstring& condition, const std::wstring& msg);
}

// Macros available for everybody
// ----------------

/*  size of statically declared array */
#define EXSIZEOF(array)   (sizeof(array)/sizeof(array[0]))

/*!
* \brief exASSERT_MSG(cond, msg) - MACRO
*
* If cond evalutes to false, this Macro will always call exOnAssert()
* If _DEBUG is defined, it will call __debugbreak() to let you trap 
* into the debugger.
*/
#ifdef _DEBUG
#define exASSERT_MSG(cond, msg)											\
do {																\
	if ( !(cond) )  {												\
		exOnAssert(__FILEW__, __LINE__, __FUNCTIONW__,L#cond,msg);	\
		__debugbreak();												\
	}                                                               \
} while ( 0 )
#else
#define exASSERT_MSG(cond, msg)										\
do {																\
	if ( !(cond) )  {												\
		exOnAssert(__FILEW__, __LINE__, __FUNCTIONW__,L#cond,msg);	\
	}                                                               \
} while ( 0 )
#endif

/*!
* \brief exASSERT(cond) - MACRO
*
* This macro is a simple shorthand to the macro exASSERT_MSG(const, msg), passing an empty message
*/
#define exASSERT(cond) exASSERT_MSG(cond, L"")

/*!
* \brief exDEBUG(cond) - MACRO
*
* If _DEBUG is defined this macro is a simple shorthand to the macro exASSERT(const).
# Else it does nothing
*/
#ifdef _DEBUG
#define exDEBUG(cond) exASSERT(cond)
#else
#define exDEBUG(cond)
#endif

// exFAIL must become something that will always trigger something, not depending on any flags
#define exFAIL_MSG(msg)												\
	do {															\
	BOOST_LOG_TRIVIAL(error) << msg;								\
	} while ( 0 )

#define exNOT_IMPL	\
	do {			\
		BOOST_LOG_TRIVIAL(error) << __FILEW__ << L" (" << __LINE__ << L"): Not Implemented";	\
		exASSERT_MSG(false, L"Not Implemented");	\
	} while( 0 )	


namespace exodbc
{

	// Structs
	// -------
	/*!
	* \class SErrorInfo
	*
	* \brief Store error-information from odbc
	*/
	struct EXODBCAPI SErrorInfo
	{
		SErrorInfo()
		{
			SqlState[0] = 0;
			NativeError = 0;
			ErrorHandleType = 0;
		}

		SQLSMALLINT		ErrorHandleType; // Handle-type of the error. Is either SQL_HANDLE_ENV, SQL_HANDLE_DBC or SQL_HANDLE_STMT
		SQLWCHAR		SqlState[5 + 1];
		SQLINTEGER		NativeError;
		std::wstring	Msg;
		friend std::wostream& operator<< (std::wostream &out, const SErrorInfo& ei);

		std::wstring ToString() const;
	};


	/*!
	* \brief Ugly conversion of small to wide - use only if you know that you have only ASCII chars in w.
	*
	* \detailed Transforms from wide to small by simple taking the char-values.
	* \param const std::wstring& w
	* \return std::string
	*/
	extern std::string w2s(const std::wstring& w);


	/*!
	* \brief Returns the string TRUE, FALSE or ????? for the values SQL_TRUE, SQL_FALSE or anything else.
	*
	* \param SQLSMALLINT b SQL_TRUE or SQL_FALSE
	* \return std::wstring TRUE, FALSE or ?????
	*/
	extern EXODBCAPI std::wstring SqlTrueFalse2s(SQLSMALLINT b);

	/*!
	* \brief Translates some often encountered SQLRETURN values to a string.
	*
	* \param SQLRETURN ret Return code to translate.
	* \return std::wstring Translation or '???' if unknown.
	*/
	extern EXODBCAPI std::wstring SqlReturn2s(SQLRETURN ret);


	/*!
	* \brief Transform the SQL_types like SQL_CHAR, SQL_NUMERIC, etc. to some string.
	*
	* \param SQLSMALLINT sqlType
	* \return std::wstring
	*/
	extern EXODBCAPI std::wstring SqlType2s(SQLSMALLINT sqlType);


	/*!
	* \brief Transform the SQL C-types like SQL_C_SLONG, SQL_C_WCHAR, etc. to some string, like "SQL_C_SLONG", etc.
	*
	* \param SQLSMALLINT sqlCType
	* \return std::wstring
	*/
	extern EXODBCAPI std::wstring SqLCType2s(SQLSMALLINT sqlCType);


	/*!
	* \brief	Transform the SQL C-types like SQL_C_SLONG, SQL_C_WCHAR, etc. to the corresponding string value of the ODBC C Type.
	*			Like SQL_C_SLONG becomes "SQLINTEGER"
	*
	* \param SQLSMALLINT sqlCType
	* \return std::wstring
	*/
	extern EXODBCAPI std::wstring SqlCType2OdbcS(SQLSMALLINT sqlCType);


	/*!
	* \todo: Implement this, once we have flags and do no longer need to reference the table in here.
	* \brief	Translate Table::OpenMode to String
	* \param openMode
	* \return std::wstring
	*/
	//extern EXODBCAPI std::wstring OpenMode2s(Table::OpenMode openMode);


	/*!
	 * \brief	Gets all errors for all passed handles.
	 *
	 * \param	hEnv 	(Optional) the environment.
	 * \param	hDbc 	(Optional) the dbc.
	 * \param	hStmt	(Optional) the statement.
	 * \param	hStmt	(Optional) the description.
	 *
	 * \return	all errors.
	 */
	extern EXODBCAPI std::vector<SErrorInfo> GetAllErrors(SQLHANDLE hEnv = SQL_NULL_HENV, SQLHANDLE hDbc = SQL_NULL_HDBC, SQLHANDLE hStmt = SQL_NULL_HSTMT, SQLHANDLE hDesc = SQL_NULL_HDESC);

	/*!
	 * \brief	Gets the last environment error, if one is available.
	 *
	 * \param	hEnv		The environment. If NULL, a warning is logged and Default SErrorInfo object returned.
	 * \param [out]	totalErrors	The total number of errors available.
	 *
	 * \return	The last environment error, or the default SErrorInfo object if no error could be fetched.
	 */
	extern EXODBCAPI SErrorInfo GetLastEnvError(SQLHANDLE hEnv, SQLSMALLINT& totalErrors);

	/*!
	* \brief	Gets the last Dbc error, if one is available.
	*
	* \param	hDbc		The database handle. If NULL, a warning is logged and Default SErrorInfo object returned.
	* \param [out]	totalErrors	The total number of errors available.
	*
	* \return	The last dbc error, or the default SErrorInfo object if no error could be fetched.
	*/
	extern EXODBCAPI SErrorInfo GetLastDbcError(SQLHANDLE hDbc, SQLSMALLINT& totalErrors);

	/*!
	* \brief	Gets the last statement error, if one is available.
	*
	* \param	hEnv		The statement. If NULL, a warning is logged and Default SErrorInfo object returned.
	* \param [out]	totalErrors	The total number of errors available.
	*
	* \return	The last statement error, or the default SErrorInfo object if no error could be fetched.
	*/
	extern EXODBCAPI SErrorInfo GetLastStmtError(SQLHANDLE hStmt, SQLSMALLINT& totalErrors);

	/*!
	* \brief	Gets the last environment error, if one is available.
	*
	* \param	hEnv		The environment. If NULL, a warning is logged and Default SErrorInfo object returned.
	*
	* \return	The last environment error, or the default SErrorInfo object if no error could be fetched.
	*/
	extern EXODBCAPI SErrorInfo GetLastEnvError(SQLHANDLE hEnv);

	/*!
	* \brief	Gets the last Dbc error, if one is available.
	*
	* \param	hDbc		The database handle. If NULL, a warning is logged and Default SErrorInfo object returned.
	*
	* \return	The last dbc error, or the default SErrorInfo object if no error could be fetched.
	*/
	extern EXODBCAPI SErrorInfo GetLastDbcError(SQLHANDLE hDbc);

	/*!
	* \brief	Gets the last statement error, if one is available.
	*
	* \param	hEnv		The statement. If NULL, a warning is logged and Default SErrorInfo object returned.
	*
	* \return	The last statement error, or the default SErrorInfo object if no error could be fetched.
	*/
	extern EXODBCAPI SErrorInfo GetLastStmtError(SQLHANDLE hStmt);

	enum CloseMode
	{
		FailIfNotOpen,	//< Returns false if Cursor is not open. 
		IgnoreNotOpen	//< Returns true also if cursor was not open
	};


	/*!
	* \brief	Close the cursor associated with the passed statement handle.
	*
	* \param	hStmt		The statement handle.
	* \param	mode		Determine whether the function should fail if the cursor is not open.
	* \return	Depends on CloseMode
	*/
	extern EXODBCAPI bool	CloseStmtHandle(const SQLHANDLE& hStmt, CloseMode mode);


	/*!
	* \brief	Ensure that the passed Statement-handle is closed after this function returns.
	* \detailed	The function expects that the passed Statement-handle is already closed. It will
	*			try to close the statement again, and if it does not fail (which means the statement
	*			was still open) the function will return false and log an error.
	*			If it fails to close the statement (as the statement was already closed) it will return
	*			true and log nothing.
	*			Note: This function is probably only used in debug-assertions to detect erroneously
	*			open statements.
	*			Note: This function will only work correctly when working with an odbc 3.x driver
	*			Note: This function does not work using MySQL ODBC Driver ?
	* \param	hStmt		The statement handle.
	* \param	dbms		The Database-Type connected to.
	* \return	True if hStmt was already closed when the function was called.
	*/
	extern EXODBCAPI bool	EnsureStmtIsClosed(const SQLHANDLE& hStmt, DatabaseProduct dbms);


	/*!
	* \brief	Compares two SQL-States, returns true if they are equal.
	* \detailed	This is a imple shorthand to a wcsncmp with a max of 5 chars
	* \param	sqlState1	First SQL-State
	* \param	sqlState2	Second SQL-State
	* \return	True if the strings sqlState1 and sqlState2 are equal for the first 5 chars
	*/
	extern EXODBCAPI bool	CompareSqlState(const wchar_t* sqlState1, const wchar_t* sqlState2);


	/*!
	* \brief	A wrapper to SQLNumResultCols.
	* \detailed	Counts how many columns are available on the result set of the
	*			passed Statement handle. Fails if statement is in wrong state
	* \param	hStmt		The statement handle.
	* \see		http://msdn.microsoft.com/en-us/library/ms715393%28v=vs.85%29.aspx
	* \return	True if hStmt was already closed when the function was called.
	*/
	extern EXODBCAPI SQLSMALLINT GetResultColumnsCount(SQLHANDLE hStmt);


	/*!
	* \brief	A wrapper to SQLGetInfo.
	*
	* \param	hDbc					The dbc.
	* \param	fInfoType				Type of the information.
	* \param [in,out]	sValue			String to receive the value read.
	* \detailed This will first call SQLGetInfo to determine the size of the buffer, then allocate a
	*			corresponding buffer and call GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
	* \see		SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
	* \return	true if it succeeds, false if it fails.
	*/
	extern EXODBCAPI bool		GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, std::wstring& sValue);


	/*!
	 * \brief	A wrapper to SQLGetInfo.
	 *
	 * \param	hDbc					The dbc.
	 * \param	fInfoType				Type of the information.
	 * \param	rgbInfoValue			Output buffer pointer.
	 * \param	cbInfoValueMax			Length of buffer.
	 * \param [in,out]	pcbInfoValue	Out-pointer for total length in bytes (excluding null-terminate char for string-values).
	 * \see		http://msdn.microsoft.com/en-us/library/ms711681%28v=vs.85%29.aspx
	 * \return	true if it succeeds, false if it fails.
	 */
	extern EXODBCAPI bool		GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue);


	/*!
	 * \brief	Gets one field of a record of the passed stmt-handle.
	 *
	 * \param	hStmt				  	The statement-handle.
	 * \param	colOrParamNr		  	The col or parameter nr.
	 * \param	targetType			  	Type of the target.
	 * \param	pTargetValue			Pointer to the target buffer.
	 * \param	bufferLen			  	Length of the buffer in Bytes (not strings!).
	 * \param [in,out]	strLenOrIndPtr	Pointer to return the number of bytes read.
	 * \param [in,out]	pIsNull		  	If pIsNull is not NULL, set to TRUE if the field is NULL. Ignored if pIsNull is NULL.
	 * \param	nullTerminate		  	(Optional) true to null terminate. Can only be set if targetType is SQL_C_WCHAR or SQL_C_CHAR.
	 * 									Will null-terminate at strLenOrIndPtr-value.
	 * 									TODO: Test that this works
	 *
	 * \return	true if it succeeds, false if it fails.
	 */
	extern EXODBCAPI bool		GetData(SQLHSTMT hStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER pTargetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr, bool* pIsNull, bool nullTerminate = false);


	/*!
	 * \brief	Gets string data. Allocates a wchar_t buffer with maxNrOfChars wchars + one char for the null-terminate:
	 * 			wchar_t* buff = new buff[maxNrOfChars + 1];
	 * 			Then calls GetData with that buffer and takes into account that GetData needs a buffer-size, not a char-size.
	 * 			If the data is null or reading fails, value is set to an empty string.
	 *
	 * \param	hStmt		 	The statement.
	 * \param	colNr		 	The col nr.
	 * \param	maxNrOfChars 	The maximum nr of characters.
	 * \param [in,out]	value	The value.
	 *
	 * \return	true if it succeeds, false if it fails.
	 */
	extern EXODBCAPI bool		GetData(SQLHSTMT hStmt, SQLUSMALLINT colNr, size_t maxNrOfChars, std::wstring& value, bool* pIsNull = NULL);


	//	extern EXODBCAPI bool		GetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLINTEGER& value);


	extern EXODBCAPI bool		SetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value);

	enum RowDescriptorType
	{
		RDT_ROW = SQL_ATTR_APP_ROW_DESC,
		RDT_PARAM = SQL_ATTR_APP_PARAM_DESC
	};
	extern EXODBCAPI bool		GetRowDescriptorHandle(SQLHSTMT hStmt, RowDescriptorType type, SQLHDESC& hDesc);


	extern EXODBCAPI SQL_TIME_STRUCT InitTime(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second);


	extern EXODBCAPI SQL_DATE_STRUCT InitDate(SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year);


	extern EXODBCAPI SQL_TIMESTAMP_STRUCT InitTimestamp(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second, SQLUINTEGER fraction, SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year);


	extern EXODBCAPI bool IsTimeEqual(const SQL_TIME_STRUCT& t1, const SQL_TIME_STRUCT& t2);

	
	extern EXODBCAPI bool IsDateEqual(const SQL_DATE_STRUCT& d1, const SQL_DATE_STRUCT& d2);


	extern EXODBCAPI bool IsTimestampEqual(const SQL_TIMESTAMP_STRUCT& ts1, const SQL_TIMESTAMP_STRUCT& ts2);


	extern EXODBCAPI SQL_NUMERIC_STRUCT InitNumeric(SQLCHAR precision, SQLSCHAR scale, SQLCHAR sign, SQLCHAR val[SQL_MAX_NUMERIC_LEN]);


	extern EXODBCAPI SQL_NUMERIC_STRUCT InitNullNumeric();


	/*!
	* \brief Convert the value of a SQL_NUMERIC_STRUCT to the long value
	* \detailed Just copied from the ms sample.
	* \see https://support.microsoft.com/kb/222831/en-us
	*/
	extern EXODBCAPI long Str2Hex2Long(unsigned char hexValue[16]);


	extern EXODBCAPI void Long2StrHex(long value, char* hexValue);

	// Classes
	// -------

}


#define LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, logLevel) \
	do { \
		std::wstring msgStr(msg); \
		std::vector<SErrorInfo> errs = GetAllErrors(hEnv, hDbc, hStmt, hDesc); \
		std::vector<SErrorInfo>::const_iterator it; \
		std::wstringstream handles; \
		if(hEnv) handles << L"Env=" << hEnv << L";"; \
		if(hDbc) handles << L"Dbc=" << hDbc << L";"; \
		if(hStmt) handles << L"Stmt=" << hStmt << L";"; \
		if(hDesc) handles << L"Desc=" << hDesc << L";"; \
		std::wstring type = L"Error(s)"; \
		if(ret == SQL_SUCCESS_WITH_INFO) { type = L"Info(s)"; } \
		std::wstringstream ws; \
		ws << __FILEW__ << L"(" << __LINE__ << L") " << __FUNCTIONW__ << L": " ; \
		if(msgStr.length() > 0) \
			ws << msgStr << L": "; \
		ws << L"ODBC-Function '" << L#SqlFunction << L"' returned " << SqlReturn2s(ret) << L" (" << ret << L"), with " << errs.size() << L" ODBC-" << type << L" from handle(s) '" << handles.str() << L"': "; \
		for(it = errs.begin(); it != errs.end(); it++) \
		{ \
			const SErrorInfo& err = *it; \
			ws << std::endl << L"\t" << err; \
		} \
		BOOST_LOG_TRIVIAL(logLevel) << ws.str(); \
	} while( 0 )

// ODBC-Loggers, with a message
#define LOG_ERROR_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, error)
#define LOG_WARNING_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, warning)
#define LOG_INFO_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, info)

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
		BOOST_LOG_TRIVIAL(error) << __FILEW__ << L"(" << __LINE__ << L") " << __FUNCTIONW__ << L": ODBC-Function '" << L#SqlFunction << L"' returned " << ret; \
	} while( 0 )

// Log expected NO_DATA
#define LOG_ERROR_EXPECTED_SQL_NO_DATA(ret, SqlFunction) \
	do { \
		BOOST_LOG_TRIVIAL(error) << __FILEW__ << L"(" << __LINE__ << L") " << __FUNCTIONW__ << L": ODBC-Function '" << L#SqlFunction << L"' returned " << ret << L", but we expected SQL_NO_DATA (" << SQL_NO_DATA << L")"; \
	} while( 0 )

// Generic Log-entry
#define LOG_MSG(logLevel, msg) \
	do { \
	BOOST_LOG_TRIVIAL(logLevel) << __FILEW__ << L"(" << __LINE__ << L") " << __FUNCTIONW__ << L": " << msg; \
	} while( 0 )

// Generic Log-entry shortcuts
#define LOG_ERROR(msg) LOG_MSG(error, msg)
#define LOG_WARNING(msg) LOG_MSG(warning, msg)
#define LOG_INFO(msg) LOG_MSG(info, msg)
#define LOG_DEBUG(msg) LOG_MSG(debug, msg)

#endif // HELPERS_H
