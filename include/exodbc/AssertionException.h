﻿/*!
* \file AssertionException.h
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Header file for the AssertionException class and all
*			corresponding function and macros.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "Exception.h"

// Other headers
// System headers


// Forward declarations
// --------------------

namespace exodbc
{
	// Typedefs
	// --------

	// Structs
	// -------

	// Classes
	// -------

	/*!
	* \class AssertionException
	* \brief Thrown if trapped on assertion.
	*/
	class EXODBCAPI AssertionException
		: public Exception
	{
	public:
		AssertionException() = delete;

		/*!
		* \brief Create AssertionException with source information.
		*/
		AssertionException(int line, const std::string& file, const std::string& functionname, const std::string& condition) noexcept
			: Exception()
			, m_condition(condition)
		{
			SetSourceInformation(line, file, functionname);
			m_what = ToString();
		};

		/*!
		* \brief Create AssertionException with source information and message.
		*/
		AssertionException(int line, const std::string& file, const std::string& functionname, const std::string& condition, const std::string& msg) noexcept
			: Exception(msg)
			, m_condition(condition)
		{
			SetSourceInformation(line, file, functionname);
			m_what = ToString();
		};

		virtual ~AssertionException() {};

		std::string GetName() const noexcept override { return u8"exodbc::AssertionException"; };
		std::string ToString() const noexcept override;

	protected:
		std::string m_condition;
	};

	/*!
	* \brief Called by macros to throw Assertions
	* \throw AssertionException if called.
	*/
	extern EXODBCAPI void exOnAssert(const std::string& file, int line, const std::string& function, const std::string& condition, const std::string& msg);
}

// AssertionException Macros
// -------------------------

/*!
* \brief exASSERT_MSG(cond, msg) - MACRO
*
* If cond evalutes to false, this Macro will always call exOnAssert().
*/
#define exASSERT_MSG(cond, msg)										\
do {																\
	if ( !(cond) )  {												\
		exodbc::exOnAssert(__FILE__, __LINE__, __FUNCTION__, #cond, msg);	\
	}                                                               \
} while ( 0 )


/*!
* \brief exASSERT(cond) - MACRO
*
* This macro is a simple shorthand to the macro exASSERT_MSG(const, msg), passing an empty message
*/
#define exASSERT(cond) exASSERT_MSG(cond, "")
