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
			m_what = utf16ToUtf8(ToString()); 
		};
		
		/*!
		* \brief Create Exception with passed message. 
		*/
		Exception(const std::wstring& msg) noexcept 
			: std::runtime_error("exodbc::Exception")
			, m_line(0)
			, m_msg(msg) 
		{ 
			m_what = utf16ToUtf8(ToString()); 
		};

		virtual ~Exception() noexcept;

		/*!
		* \brief Formats a message using GetName(), message and if available source information.
		*/
		virtual std::wstring ToString() const noexcept;

		/*!
		* \brief Return name of this Exception. Probably class name.
		*/
		virtual std::wstring GetName() const noexcept	{ return L"exodbc::Exception"; };

		/*!
		* \brief Set source information of the Exception origin.
		*/
		void SetSourceInformation(int line, const std::wstring& fileName, const std::wstring& functionName) noexcept;

		/*!
		* \brief Return line number that triggered Exception.
		*/
		int GetLine() const noexcept						{ return m_line; };

		/*!
		* \brief Return file name that triggered Exception.
		*/
		std::wstring GetFile() const noexcept			{ return m_file; };

		/*!
		* \brief Return function name that triggered Exception.
		*/
		std::wstring GetFunctionName() const noexcept	{ return m_functionname;; }

		/*!
		* \brief Return message.
		*/
		std::wstring GetMsg() const noexcept				{ return m_msg; };

		/*!
		* \brief	Returns the same as ToString(), but converted to an utf8-string.
		*/
		virtual const char* what() const noexcept;

	protected:

		int				m_line;
		std::wstring	m_file;
		std::wstring	m_functionname;
		std::wstring	m_msg;
		std::string	m_what;
	};

#pragma warning(pop)
}

// Generic Exception Macros
// ------------------------

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

