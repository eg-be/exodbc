/*!
* \file Helpers.h
* \author Elias Gerber <egerber@gmx.net>
* \date 23.07.2014
* \brief Header file for generic Helpers, mostly about error-handling and assertions.
* \copyright wxWindows Library Licence, Version 3.1
*/ 

#pragma once
#ifndef HELPERS_H
#define HELPERS_H

// Same component headers
#include "exOdbc.h"

// Other headers
#include "boost/log/trivial.hpp"
#include "boost/thread/shared_mutex.hpp"

// System headers
#include <string>
#include <iostream>

// Forward declarations
// --------------------
namespace exodbc
{
	extern bool g_dontDebugBreak;
	extern boost::shared_mutex g_dontDebugBreakMutex;
	extern EXODBCAPI void SetDontDebugBreak(bool value);
	extern EXODBCAPI bool GetDontDebugBreak();
	extern EXODBCAPI void exOnAssert(const std::wstring& file, int line, const std::wstring& function, const std::wstring& condition, const std::wstring& msg);

	struct DontDebugBreak
	{
		DontDebugBreak()
		{
			SetDontDebugBreak(true);
		}

		~DontDebugBreak()
		{
			SetDontDebugBreak(false);
		}
	};
}

// Macros available for everybody
// ----------------

/*!
* \brief exASSERT_MSG(cond, msg) - MACRO
*
* If cond evalutes to false, this Macro will always call exOnAssert()
* If _DEBUG is defined, it will call __debugbreak() before calling exOnAssert()
* to let you trap into the debugger.
*/
#ifdef _DEBUG
#define exASSERT_MSG(cond, msg)										\
do {																\
	if ( !(cond) )  {												\
		if(!GetDontDebugBreak()) {									\
			__debugbreak();											\
		}															\
		exOnAssert(__FILEW__, __LINE__, __FUNCTIONW__,L#cond,msg);	\
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

		SQLSMALLINT		ErrorHandleType; ///< Handle-type of the error. Is either SQL_HANDLE_ENV, SQL_HANDLE_DBC, SQL_HANDLE_STMT or SQL_HANDLE_DESC
		SQLWCHAR		SqlState[5 + 1];
		SQLINTEGER		NativeError;
		std::wstring	Msg;
		friend std::wostream& operator<< (std::wostream &out, const SErrorInfo& ei);

		std::wstring ToString() const;
	};
	typedef std::vector<SErrorInfo> SErrorInfoVector;


	/*!
	* \brief Ugly conversion of small to wide - use only if you know that you have only ASCII chars in w.
	*
	* \details Transforms from wide to small by simple taking the char-values.
	* \param const std::wstring& w
	* \return std::string
	*/
	extern std::string w2s(const std::wstring& w);


	/*!
	* \brief Ugly conversion of wide to small - use only if you know that you have only ASCII chars in s.
	*
	* \details Transforms small wide to wide by simple taking the char-values.
	* \param const std::string& s
	* \return std::wstring
	*/
	extern std::wstring s2w(const std::string& s);


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
	 * \brief	Gets all errors for all passed handles. Should never throw as long
	 *			you pass in at least one non-NULL handle.
	 *
	 * \param	hEnv 	(Optional) the environment.
	 * \param	hDbc 	(Optional) the dbc.
	 * \param	hStmt	(Optional) the statement.
	 * \param	hStmt	(Optional) the description.
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
	* \throw	Exception	Depending on CloseMode.
	*/
	extern EXODBCAPI void	CloseStmtHandle(const SQLHANDLE& hStmt, CloseMode mode);


	/*!
	* \brief	Ensure that the passed Statement-handle is closed after this function returns.
	* \details	The function expects that the passed Statement-handle is already closed. It will
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
	* \details	This is a imple shorthand to a wcsncmp with a max of 5 chars
	* \param	sqlState1	First SQL-State
	* \param	sqlState2	Second SQL-State
	* \return	True if the strings sqlState1 and sqlState2 are equal for the first 5 chars
	*/
	extern EXODBCAPI bool	CompareSqlState(const SQLWCHAR* sqlState1, const SQLWCHAR* sqlState2);


	/*!
	* \brief	A wrapper to SQLNumResultCols.
	* \details	Counts how many columns are available on the result set of the
	*			passed Statement handle. Fails if statement is in wrong state
	* \param	hStmt		The statement handle.
	* \see		http://msdn.microsoft.com/en-us/library/ms715393%28v=vs.85%29.aspx
	* \return	True if hStmt was already closed when the function was called.
	*/
	extern EXODBCAPI SQLSMALLINT GetResultColumnsCount(SQLHANDLE hStmt);


	/*!
	* \brief	A wrapper to SQLGetInfo to read a String info.
	* 
	* \param	hDbc					The Database connection handle.
	* \param	fInfoType				Type of the information.
	* \param [in,out]	sValue			String to receive the value read.
	* \details This will first call SQLGetInfo to determine the size of the buffer, then allocate a
	*			corresponding buffer and call GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
	* \see		SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
	* \throw	Exception
	*/
	extern EXODBCAPI void		GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, std::wstring& sValue);


	/*!
	 * \brief	A wrapper to SQLGetInfo.
	 *
	 * \param	hDbc					The dbc.
	 * \param	fInfoType				Type of the information.
	 * \param	pInfoValue			Output buffer pointer.
	 * \param	cbInfoValueMax			Length of buffer.
	 * \param [in,out]	pcbInfoValue	Out-pointer for total length in bytes (excluding null-terminate char for string-values).
	 * \see		http://msdn.microsoft.com/en-us/library/ms711681%28v=vs.85%29.aspx
	 * \throw	Exception
	 */
	extern EXODBCAPI void		GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER pInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue);


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
	 * \todo: Test that this works
	 *
	 * \return	true if it succeeds, false if it fails.
	 */
	extern EXODBCAPI void		GetData(SQLHSTMT hStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER pTargetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr, bool* pIsNull, bool nullTerminate = false);


	/*!
	 * \brief	Gets string data. Allocates a wchar_t buffer with maxNrOfChars wchars + one char for the null-terminate:
	 * 			SQLWCHAR* buff = new buff[maxNrOfChars + 1];
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
	extern EXODBCAPI void		GetData(SQLHSTMT hStmt, SQLUSMALLINT colNr, size_t maxNrOfChars, std::wstring& value, bool* pIsNull = NULL);


	/*!
	* \brief	A wrapper to SQLSetDescField
	*
	* \param	hDesc		 	The description handle to set an attribute for.
	* \param	recordNumber	Column number to set the attribute for.
	* \return	descriptionField Attribute to set.
	* \return	value			Value to set.
	* \throw	Exception
	*/
	extern EXODBCAPI void		SetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value);

	enum RowDescriptorType
	{
		RDT_ROW = SQL_ATTR_APP_ROW_DESC,
		RDT_PARAM = SQL_ATTR_APP_PARAM_DESC
	};
	/*!
	* \brief	A wrapper to SQLGetStmtAttr
	*
	* \param	hStmt		 	The statement.
	* \param	RowDescriptorType	Type of Description Handle to fetch from hStmt
	* \return	Row Descriptor Handle
	* \throw	Exception
	*/
	extern EXODBCAPI SQLHDESC		GetRowDescriptorHandle(SQLHSTMT hStmt, RowDescriptorType type);


	extern EXODBCAPI SQL_TIME_STRUCT InitTime(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second);


	extern EXODBCAPI SQL_DATE_STRUCT InitDate(SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year);


	extern EXODBCAPI SQL_TIMESTAMP_STRUCT InitTimestamp(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second, SQLUINTEGER fraction, SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year);


	extern EXODBCAPI bool IsTimeEqual(const SQL_TIME_STRUCT& t1, const SQL_TIME_STRUCT& t2);

	
	extern EXODBCAPI bool IsDateEqual(const SQL_DATE_STRUCT& d1, const SQL_DATE_STRUCT& d2);


	extern EXODBCAPI bool IsTimestampEqual(const SQL_TIMESTAMP_STRUCT& ts1, const SQL_TIMESTAMP_STRUCT& ts2);


	extern EXODBCAPI SQL_NUMERIC_STRUCT InitNumeric(SQLCHAR precision, SQLSCHAR scale, SQLCHAR sign, SQLCHAR val[SQL_MAX_NUMERIC_LEN]);


	extern EXODBCAPI SQL_NUMERIC_STRUCT InitNullNumeric();


	enum FreeStatementThrowFlag
	{
		FSTF_NO_THROW = 0x0,
		FSTF_THROW_ON_SQL_ERROR = 0x01,
		FSTF_THROW_ON_SQL_INVALID_HANDLE = 0x02
	};
	typedef unsigned int FreeStatementThrowFlags;
	extern EXODBCAPI SQLHSTMT FreeStatementHandle(SQLHSTMT hStmt, FreeStatementThrowFlags flags = FSTF_THROW_ON_SQL_ERROR | FSTF_THROW_ON_SQL_INVALID_HANDLE);


	/*!
	* \brief Convert the value of a SQL_NUMERIC_STRUCT to the long value
	* \details Just copied from the ms sample.
	* \see https://support.microsoft.com/kb/222831/en-us
	*/
	extern EXODBCAPI long Str2Hex2Long(unsigned char hexValue[16]);


	extern EXODBCAPI void Long2StrHex(long value, char* hexValue);

	// Classes
	// -------
	class StatementCloser
	{
	public:
		StatementCloser(SQLHSTMT hStmt, bool closeOnConstruction = false, bool closeOnDestruction = true);
		~StatementCloser();

	private:
		SQLHSTMT m_hStmt;
		bool m_closeOnDestruction;
	};
}


#define LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, logLevel) \
	do { \
		std::wstring msgStr(msg); \
		SErrorInfoVector errs = GetAllErrors(hEnv, hDbc, hStmt, hDesc); \
		SErrorInfoVector::const_iterator it; \
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



#endif // HELPERS_H
