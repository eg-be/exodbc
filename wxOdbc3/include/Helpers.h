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
	extern void exOnAssert(const char* file, int line, const char* function, const char* condition, const char* msg);
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
	exOnAssert(__FILE__, __LINE__, __FUNCTION__,#cond,msg);	\
	__debugbreak();												\
	}                                                               \
	} while ( 0 )
#else
#define exASSERT_MSG(cond, msg)
#endif

// a version without any additional message, don't use unless condition
// itself is fully self-explanatory
#define exASSERT(cond) exASSERT_MSG(cond, (const char*)NULL)

// exFAIL must become something that will always trigger something, not depending on any flags
#define exFAIL_MSG(msg)												\
	do {																\
	BOOST_LOG_TRIVIAL(error) << msg;								\
	} while ( 0 )


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
	std::string w2s(const std::wstring& w);

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
	std::vector<SErrorInfo> GetAllErrors(SQLHANDLE hEnv = NULL, SQLHANDLE hDbc = NULL, SQLHANDLE hStmt = NULL);

	// Classes
	// -------

}


#endif // HELPERS_H
