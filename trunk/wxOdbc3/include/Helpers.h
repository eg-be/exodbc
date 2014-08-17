/*!
* \file Helpers.h
* \author Elias Gerber <egerber@gmx.net>
* \date 23.07.2014
* 
* [Brief Header-file description]
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

	extern EXODBCAPI bool		GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER rgbInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue);
	extern EXODBCAPI bool		GetData(SQLHSTMT hStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr,  bool* pIsNull, bool nullTerminate = false);

	// Classes
	// -------

}


#define LOG_ERROR_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, msg) \
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
		BOOST_LOG_TRIVIAL(error) << ws.str(); \
	} while( 0 )

#define LOG_ERROR_ODBC(hEnv, hDbc, hStmt, ret, SqlFunction) LOG_ERROR_ODBC_MSG(hEnv, hDbc, hStmt, ret, SqlFunction, L"")

#define LOG_ERROR_ENV_MSG(hEnv, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(hEnv, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_ERROR_DBC_MSG(hDbc, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(NULL, hDbc, NULL, ret, SqlFunction, msgStr)
#define LOG_ERROR_STMT_MSG(hStmt, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(NULL, NULL, hStmt, ret, SqlFunction, msgStr)

#define LOG_ERROR_ENV(hEnv, ret, SqlFunction) LOG_ERROR_ODBC(hEnv, NULL, NULL, ret, SqlFunction)
#define LOG_ERROR_DBC(hDbc, ret, SqlFunction) LOG_ERROR_ODBC(NULL, hDbc, NULL, ret, SqlFunction)
#define LOG_ERROR_STMT(hStmt, ret, SqlFunction) LOG_ERROR_ODBC(NULL, NULL, hStmt, ret, SqlFunction)

#define LOG_ERROR_SQL_NO_SUCCESS(ret, SqlFunction) \
	do { \
		BOOST_LOG_TRIVIAL(error) << __FILEW__ << L"(" << __LINE__ << L") " << __FUNCTIONW__ << L": ODBC-Function '" << L#SqlFunction << L"' returned " << ret; \
	} while( 0 )

#define LOG_ERROR_EXPECTED_SQL_NO_DATA(ret, SqlFunction) \
	do { \
	BOOST_LOG_TRIVIAL(error) << __FILEW__ << L"(" << __LINE__ << L") " << __FUNCTIONW__ << L": ODBC-Function '" << L#SqlFunction << L"' returned " << ret << L", but we expected SQL_NO_DATA (" << SQL_NO_DATA << L")"; \
	} while( 0 )

#endif // HELPERS_H
