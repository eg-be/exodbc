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

// A much simpler form of the exASSERT-Macro
#ifdef _DEBUG
#define exASSERT_MSG(cond, msg)										\
	do {																\
	if ( !(cond) )  {												\
	exOnAssert(__FILEW__, __LINE__, __FUNCTIONW__,L#cond,msg);	\
	__debugbreak();												\
	}                                                               \
	} while ( 0 )
#else
#define exASSERT_MSG(cond, msg)
#endif

// a version without any additional message, don't use unless condition
// itself is fully self-explanatory
#define exASSERT(cond) exASSERT_MSG(cond, L"")

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
	* \brief Store error-information from odbc
	* 
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
		friend std::ostream& operator<<(std::ostream& os, const SErrorInfo& ei);
	};


	/*!
	* \brief Very ugly conversion of small to wide - DO NOT USE, see Ticket #44
	* 
	* Transforms from wide to small by simple taking the char-values. Remove as soon as #44 is done.
	* \param const std::wstring& w
	* \return std::string
	*/
	extern std::string w2s(const std::wstring& w);

	extern EXODBCAPI std::wstring SqlTrueFalse2s(SQLSMALLINT b);
	extern EXODBCAPI std::wstring SqlType2s(SQLSMALLINT sqlType);
	/*!
	 * \fn	std::vector<SErrorInfo> GetAllErrors(SQLHANDLE hEnv = NULL, SQLHANDLE hDbc = NULL, SQLHANDLE hStmt = NULL);
	 *
	 * \brief	Gets all errors for all passed handles.
	 *
	 * \param	hEnv 	(Optional) the environment.
	 * \param	hDbc 	(Optional) the dbc.
	 * \param	hStmt	(Optional) the statement.
	 *
	 * \return	all errors.
	 */
	extern EXODBCAPI std::vector<SErrorInfo> GetAllErrors(SQLHANDLE hEnv = SQL_NULL_HENV, SQLHANDLE hDbc = SQL_NULL_HDBC, SQLHANDLE hStmt = SQL_NULL_HSTMT);

	/*!
	 * \fn	extern EXODBCAPI SErrorInfo GetLastEnvError(SQLHANDLE hEnv, SQLSMALLINT& totalErrors);
	 *
	 * \brief	Gets the last environment error, if one is available.
	 *
	 * \param	hEnv			   	The environment. If NULL, a warning is logged and Default SErrorInfo object returned.
	 * \param [in,out]	totalErrors	The total errors.
	 *
	 * \return	The last environment error, or the default SErrorInfo object if no error could be fetched.
	 */
	extern EXODBCAPI SErrorInfo GetLastEnvError(SQLHANDLE hEnv, SQLSMALLINT& totalErrors);
	extern EXODBCAPI SErrorInfo GetLastDbcError(SQLHANDLE hDbc, SQLSMALLINT& totalErrors);
	extern EXODBCAPI SErrorInfo GetLastStmtError(SQLHANDLE hStmt, SQLSMALLINT& totalErrors);

	extern EXODBCAPI SErrorInfo GetLastEnvError(SQLHANDLE hEnv);
	extern EXODBCAPI SErrorInfo GetLastDbcError(SQLHANDLE hDbc);
	extern EXODBCAPI SErrorInfo GetLastStmtError(SQLHANDLE hStmt);

	enum CloseMode { FailIfNotOpen, IgnoreNotOpen};
	extern EXODBCAPI SQLRETURN	CloseStmtHandle(const SQLHANDLE& hStmt, CloseMode mode);

	/*!
	 * \fn	extern EXODBCAPI bool GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER rgbInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue);
	 *
	 * \brief	A wrapper to SQLGetInfo. See http://msdn.microsoft.com/en-us/library/ms711681%28v=vs.85%29.aspx
	 *
	 * \param	hDbc					The dbc.
	 * \param	fInfoType				Type of the information.
	 * \param	rgbInfoValue			Output buffer pointer.
	 * \param	cbInfoValueMax			Length of buffer.
	 * \param [in,out]	pcbInfoValue	Out-pointer for total length in bytes (excluding null-terminate char for string-values).
	 *
	 * \return	true if it succeeds, false if it fails.
	 */
	extern EXODBCAPI bool		GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue);

	/*!
	 * \fn	extern EXODBCAPI bool GetData(SQLHSTMT hStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr, bool* pIsNull, bool nullTerminate = false);
	 *
	 * \brief	Gets one field of a record of the passed stmt-handle.
	 *
	 * \param	hStmt				  	The statement-handle.
	 * \param	colOrParamNr		  	The col or parameter nr.
	 * \param	targetType			  	Type of the target.
	 * \param	pTargetValue			Pointer to the target buffer.
	 * \param	bufferLen			  	Length of the buffer.
	 * \param [in,out]	strLenOrIndPtr	Pointer to return the number of bytes read.
	 * \param [in,out]	pIsNull		  	If pIsNull is not NULL, set to TRUE if the field is NULL. Ignored if pIsNull is NULL.
	 * \param	nullTerminate		  	(Optional) true to null terminate. Can only be set if targetType is SQL_C_WCHAR or SQL_C_CHAR.
	 * 									Will null-terminate at strLenOrIndPtr-value.
	 * 									TODO: Test that this works
	 *
	 * \return	true if it succeeds, false if it fails.
	 */
	extern EXODBCAPI bool		GetData(SQLHSTMT hStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER pTargetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr,  bool* pIsNull, bool nullTerminate = false);

	/*!
	 * \fn	extern EXODBCAPI bool GetData(SQLHSTMT hStmt, SQLUSMALLINT colNr, size_t maxNrOfChars, std::wstring& value);
	 *
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

	// Classes
	// -------

}


#define LOG_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, msg, logLevel) \
	do { \
		std::wstring msgStr(msg); \
		std::vector<SErrorInfo> errs = GetAllErrors(hEnv, hDbc, hStmt); \
		std::vector<SErrorInfo>::const_iterator it; \
		std::wstringstream handles; \
		if(hEnv) handles << L"Env=" << hEnv << L";"; \
		if(hDbc) handles << L"Dbc=" << hDbc << L";"; \
		if(hStmt) handles << L"Stmt=" << hStmt << L";"; \
		std::wstringstream ws; \
		ws << __FILEW__ << L"(" << __LINE__ << L") " << __FUNCTIONW__ << L": " ; \
		if(msgStr.length() > 0) \
			ws << msgStr << L": "; \
		ws << L"ODBC-Function '" << L#SqlFunction << L"' returned " << ret << L", with " << errs.size() << L" ODBC-Error(s) from handle(s) '" << handles.str() << L"': "; \
		for(it = errs.begin(); it != errs.end(); it++) \
		{ \
			const SErrorInfo& err = *it; \
			ws << std::endl << L"\t" << err; \
		} \
		BOOST_LOG_TRIVIAL(logLevel) << ws.str(); \
	} while( 0 )

// ODBC-Loggers, with a message
#define LOG_ERROR_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, msg, error)
#define LOG_WARNING_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, msg, warning)
#define LOG_INFO_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, msg, info)

// ODBC-Loggers, no message
#define LOG_ERROR_ODBC(hEnv, hDbc, hStmt, ret, SqlFunction) LOG_ERROR_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, L"")
#define LOG_WARNING_ODBC(hEnv, hDbc, hStmt, ret, SqlFunction) LOG_WARNING_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, L"")
#define LOG_INFO_ODBC(hEnv, hDbc, hStmt, ret, SqlFunction) LOG_INFO_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, L"")

// Log ODBC-Errors, from a specific handle (env, dbc or stmt), with a message
#define LOG_ERROR_ENV_MSG(hEnv, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(hEnv, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_ERROR_DBC_MSG(hDbc, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(NULL, hDbc, NULL, ret, SqlFunction, msgStr)
#define LOG_ERROR_STMT_MSG(hStmt, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(NULL, NULL, hStmt, ret, SqlFunction, msgStr)

// Log ODBC-Warnings, from a specific handle (env, dbc or stmt), with a message
#define LOG_WARNING_ENV_MSG(hEnv, ret, SqlFunction, msgStr) LOG_WARNING_ODBC_MSG(hEnv, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_WARNING_DBC_MSG(hDbc, ret, SqlFunction, msgStr) LOG_WARNING_ODBC_MSG(NULL, hDbc, NULL, ret, SqlFunction, msgStr)
#define LOG_WARNING_STMT_MSG(hStmt, ret, SqlFunction, msgStr) LOG_WARNING_ODBC_MSG(NULL, NULL, hStmt, ret, SqlFunction, msgStr)

// Log ODBC-Infos, from a specific handle (env, dbc or stmt), with a message
#define LOG_INFO_ENV_MSG(hEnv, ret, SqlFunction, msgStr) LOG_INFO_ODBC_MSG(hEnv, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_INFO_DBC_MSG(hDbc, ret, SqlFunction, msgStr) LOG_INFO_ODBC_MSG(NULL, hDbc, NULL, ret, SqlFunction, msgStr)
#define LOG_INFO_STMT_MSG(hStmt, ret, SqlFunction, msgStr) LOG_INFO_ODBC_MSG(NULL, NULL, hStmt, ret, SqlFunction, msgStr)

// Log ODBC-Errors, from a specific handle (env, dbc or stmt), no message
#define LOG_ERROR_ENV(hEnv, ret, SqlFunction) LOG_ERROR_ODBC(hEnv, NULL, NULL, ret, SqlFunction)
#define LOG_ERROR_DBC(hDbc, ret, SqlFunction) LOG_ERROR_ODBC(NULL, hDbc, NULL, ret, SqlFunction)
#define LOG_ERROR_STMT(hStmt, ret, SqlFunction) LOG_ERROR_ODBC(NULL, NULL, hStmt, ret, SqlFunction)

// Log ODBC-Warnings, from a specific handle (env, dbc or stmt), no message
#define LOG_WARNING_ENV(hEnv, ret, SqlFunction) LOG_WARNING_ODBC(hEnv, NULL, NULL, ret, SqlFunction)
#define LOG_WARNING_DBC(hDbc, ret, SqlFunction) LOG_WARNING_ODBC(NULL, hDbc, NULL, ret, SqlFunction)
#define LOG_WARNING_STMT(hStmt, ret, SqlFunction) LOG_WARNING_ODBC(NULL, NULL, hStmt, ret, SqlFunction)

// Log ODBC-Infos, from a specific handle (env, dbc or stmt), no message
#define LOG_INFO_ENV(hEnv, ret, SqlFunction) LOG_INFO_ODBC(hEnv, NULL, NULL, ret, SqlFunction)
#define LOG_INFO_DBC(hDbc, ret, SqlFunction) LOG_INFO_ODBC(NULL, hDbc, NULL, ret, SqlFunction)
#define LOG_INFO_STMT(hStmt, ret, SqlFunction) LOG_INFO_ODBC(NULL, NULL, hStmt, ret, SqlFunction)

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
