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

// We use _UNICODE on windows and therefore the w-versions of the sql api.
// convert our utf8 strings to utf16 wstring when calling the sql api on windows:
#ifdef _WIN32
	#define SQLAPICHARTYPE SQLWCHAR
	#define SQLAPICHARTYPENAME SQL_C_WCHAR
	#define EXODBCSTR_TO_SQLAPISTR(s) utf8ToUtf16(s)
    #define SQLAPICHARPTR_TO_EXODBCSTR(ws) utf16ToUtf8(ws)
#else
	#define SQLAPICHARTYPE SQLCHAR
	#define SQLAPICHARTYPENAME SQL_C_CHAR
	#define EXODBCSTR_TO_SQLAPISTR(s) s
    #define SQLAPICHARPTR_TO_EXODBCSTR(s) std::string(reinterpret_cast<const char*>(s))
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

// Now we use the generic helper definitions above to define import and export
// EXODBC_LIB is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// EXODBCLOCAL could be used for non-api symbols.

#ifdef EXODBC_LIB // defined if exodbc is compiled as a shared lib (dll on WIN32)
  #ifdef libexodbc_EXPORTS // defined if we are building exodbc (instead of using it)
    #define EXODBCAPI EXODBC_HELPER_DLL_EXPORT
  #else
    #define EXODBCAPI EXODBC_HELPER_DLL_IMPORT
  #endif // libexodbc_EXPORTS
  #define EXODBCLOCAL EXODBC_HELPER_DLL_LOCAL
#else // EXODBC_DLL is not defined, we have a static lib
  #define EXODBCAPI
  #define EXODBCLOCAL
#endif // EXODBC_LIB

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

	// Database Globals or defaults. The values named _DEFAULT are used as fall back
	// if value cannot be read from SQLGetInfo().
	const int DB_MAX_CONNECTSTR_LEN					= 1024;	///< SQLDriverConnects returns an output-connection string, assume this is the max-length of this string returned. MS recommends at least 1024 characters.
	const int DB_MAX_TYPE_NAME_LEN					= 40;
	const int DB_MAX_LOCAL_TYPE_NAME_LEN			= 256;
	const int DB_MAX_TABLE_NAME_LEN_DEFAULT			= 128;	///< Fall back value if SQLGetInfo returned 0 for SQL_MAX_TABLE_NAME_LEN
	const int DB_MAX_SCHEMA_NAME_LEN_DEFAULT		= 128;	///< Fall back value if SQLGetInfo returned 0 for SQL_MAX_SCHEMA_NAME_LEN
	const int DB_MAX_CATALOG_NAME_LEN_DEFAULT		= 128;	///< Fall back value if SQLGetInfo returned 0 for SQL_MAX_CATALOG_NAME_LEN
	const int DB_MAX_COLUMN_NAME_LEN_DEFAULT		= 128;	///< Fall back value if SQLGetInfo returned 0 for SQL_MAX_COLUMN_NAME_LEN
	const int DB_MAX_TABLE_TYPE_LEN					= 128;
	const int DB_MAX_TABLE_REMARKS_LEN				= 512;
	const int DB_MAX_COLUMN_REMARKS_LEN				= 512;
	const int DB_MAX_COLUMN_DEFAULT_LEN				= 512;
	const int DB_MAX_LITERAL_PREFIX_LEN				= 128;
	const int DB_MAX_LITERAL_SUFFIX_LEN				= 128;
	const int DB_MAX_CREATE_PARAMS_LIST_LEN			= 512;	
	const int DB_MAX_PRIMARY_KEY_NAME_LEN			= 128;
	const int DB_MAX_YES_NO_LEN						= 3;

    const SQLLEN SQL_NO_TOTAL_BUFFER_LENGTH = 65536;	///< Fall back: If trying to create a buffer with a length value of SQL_NO_TOTAL, this value is used as the buffer size.    

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


	/*
	* \brief istream overload for OdbcVersion:
	* \details The following conversions are applied:
	*
	* OdbcVersion		| String value
	* ------------------|------------
	* V_2				| 2
	* V_3				| 3
	* V_3_8				| 3.8
	*
	* UNKOWN sets the failure bit in the istream.
	*/
	std::istream& operator >> (std::istream& in, exodbc::OdbcVersion& ov);


	/*
	* \brief ostream overload for OdbcVersion:
	* \details The following conversions are applied:
	*
	* String value		| OdbcVersion
	* ------------------|------------
	* 2					| V_2
	* 3					| V_3
	* 3.8				| V_3_8
	*
	* All other string values set the failure bit in the ostream.
	*/
	std::ostream& operator << (std::ostream& os, const exodbc::OdbcVersion& ov);


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
		POSTGRESQL,		///< PostgreSQL
	};


	// Flags
	// =====


	// Global Helpers without any logic to the SQL-API
	// --------------

	/*!
	* \brief Converts a utf16 std::wstring to a utf-8 std::string.
	*
	* \param w String to transform
	* \return std::string
	* \throw ConversionException If conversion fails
	*/
	extern EXODBCAPI std::string utf16ToUtf8(const std::wstring& w);


	/*!
	* \see utf16ToUtf8(const std::wstring& w)
	*/
 	extern EXODBCAPI std::string utf16ToUtf8(const SQLWCHAR* w);


	/*!
	* \brief Converts a utf8 std::string to a uf16 std::wstring
	*
	* \param s String to transform
	* \return std::string
	* \throw ConversionException If conversion fails
	*/
	extern EXODBCAPI std::wstring utf8ToUtf16(const std::string& s);


	/*!
	* \see utf8ToUtf16(const std::string& s)
	*/
    extern EXODBCAPI std::wstring utf8ToUtf16(const SQLCHAR* s);


	/*!
	* \class	ErrorHelper
	* \brief	Contains some error handling related helper functions.
	*/
	class EXODBCAPI ErrorHelper
	{
	public:

		const static SQLCHAR* SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED;

		/*!
		* \struct SErrorInfo
		*
		* \brief Store error-information from ODBC.
		* \see		GetAllErrors()
		*/
		struct SErrorInfo
		{
			SErrorInfo()
			{
				SqlState[0] = 0;
				NativeError = 0;
				ErrorHandleType = 0;
			}

			SQLSMALLINT		ErrorHandleType; ///< Handle-type of the error. Is either SQL_HANDLE_ENV, SQL_HANDLE_DBC, SQL_HANDLE_STMT or SQL_HANDLE_DESC
			SQLCHAR			SqlState[5 + 1];
			SQLINTEGER		NativeError;
			std::string	Msg;

			std::string ToString() const;
		};
		/*!
		* \typedef SErrorInfoVector
		* \brief	Vector of SErrorInfo struct.
		*/
		typedef std::vector<SErrorInfo> SErrorInfoVector;


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
		static SErrorInfoVector GetAllErrors(SQLHANDLE hEnv, SQLHANDLE hDbc, SQLHANDLE hStmt, SQLHANDLE hDesc);


		/*!
		* \brief	A shorthand to GetAllErrors(SQLHANDLE hEnv, SQLHANDLE hDbc, SQLHANDLE hStmt, SQLHANDLE hDesc)
		*			you pass in at least one non-NULL handle.
		* \see		GetAllErrors(SQLHANDLE hEnv, SQLHANDLE hDbc, SQLHANDLE hStmt, SQLHANDLE hDesc)
		*/
		static SErrorInfoVector GetAllErrors(SQLSMALLINT handleType, SQLHANDLE handle);


		/*!
		* \brief Format all Infos and Errors from passed handles into something human-readable.
		*/
		static std::string FormatOdbcMessages(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt, SQLHDESC hDesc, SQLRETURN ret, std::string sqlFunctionName, std::string msg);
	};
}


// Compiler Helpers
// ================
#define HIDE_UNUSED(object) \
	do { \
		(void)object; \
	} while(0)

