/*!
* \file Exception.h
* \author Elias Gerber <eg@elisium.ch>
* \date 24.01.2015
* \brief Header file for the Exception class and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "exOdbc.h"

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

#pragma warning(push)
#pragma warning(disable: 4275)	// warning C4275: non dll-interface class 'std::runtime_error' used as base for dll-interface class 'exodbc::Exception'
								// we are exporting from something that is part of std, it is save to ignore the warning

	/*!
	* \class Exception
	* \brief Base class of all exceptions thrown.
	* \details	The message held by m_what is created during construction by calling
	*			the ToString() method.
	*/
	class EXODBCAPI Exception
		: public std::runtime_error
	{
	public:
		/*!
		* \brief Create empty Exception.
		*/
		Exception() noexcept 
			: std::runtime_error("exodbc::Exception")
			, m_line(0) 
		{ 
			m_what = ToString(); 
		};
		
		/*!
		* \brief Create Exception with passed message. 
		*/
		Exception(const std::string& msg) noexcept 
			: std::runtime_error("exodbc::Exception")
			, m_line(0)
			, m_msg(msg) 
		{ 
			m_what = ToString(); 
		};

		virtual ~Exception() noexcept;

		/*!
		* \brief Formats a message using GetName(), message and if available source information.
		*/
		virtual std::string ToString() const noexcept;

		/*!
		* \brief Return name of this Exception. Probably class name.
		*/
		virtual std::string GetName() const noexcept	{ return u8"exodbc::Exception"; };

		/*!
		* \brief Set source information of the Exception origin.
		*/
		void SetSourceInformation(int line, const std::string& fileName, const std::string& functionName) noexcept;

		/*!
		* \brief Return line number that triggered Exception.
		*/
		int GetLine() const noexcept						{ return m_line; };

		/*!
		* \brief Return file name that triggered Exception.
		*/
		std::string GetFile() const noexcept			{ return m_file; };

		/*!
		* \brief Return function name that triggered Exception.
		*/
		std::string GetFunctionName() const noexcept	{ return m_functionname;; }

		/*!
		* \brief Return message.
		*/
		std::string GetMsg() const noexcept				{ return m_msg; };

		/*!
		* \brief	Returns the same as ToString(), but converted to an utf8-string.
		*/
		virtual const char* what() const noexcept;

	protected:

		int			m_line;
		std::string	m_file;
		std::string	m_functionname;
		std::string	m_msg;
		std::string	m_what;
	};

#pragma warning(pop)
}

// Generic Exception Macros
// ------------------------

#define SET_EXCEPTION_SOURCE(Exception) \
	do { \
		Exception.SetSourceInformation(__LINE__, __FILE__, __FUNCTION__); \
	} while(0)

#define THROW_WITH_SOURCE(ExceptionType, msg) \
	do { \
		ExceptionType ex(msg); \
		SET_EXCEPTION_SOURCE(ex); \
		throw ex; \
	} while(0)

