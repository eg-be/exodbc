/*!
* \file Exception.h
* \author Elias Gerber <eg@zame.ch>
* \date 24.01.2015
* \brief Header file for the Exception class and its helpers.
*
*/

#pragma once
#ifndef Exception_H
#define Exception_H

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

	/*!
	* \class Exception
	*
	* \brief Base class of all exceptions thrown.
	*
	*/
	class EXODBCAPI Exception
		: public std::exception
	{
	public:
		Exception() throw() : m_line(0) {};
		Exception(int line, const std::wstring& file, const std::wstring& functionname, const std::wstring& msg) throw() : m_line(line), m_file(file), m_functionname(functionname), m_msg(msg) {};
		Exception(const std::wstring& msg) throw() : m_msg(msg), m_line(0) {};

		Exception(const exception&) throw() {};
		
		virtual ~Exception() throw() {};
		
		virtual std::wstring ToString() const throw();
		virtual std::wstring GetName() const throw()	{ return L"Exception"; };

		int GetLine() const throw()						{ return m_line; };
		std::wstring GetFile() const throw()			{ return m_file; };
		std::wstring GetFunctionName() const throw()	{ return m_functionname;; }
		std::wstring GetMsg() const throw()				{ return m_msg; };

		/*!
		* \brief	Returns an UTF8 representation of an eventually passed message
		* \return	Passed message as UTF8 multi byte string.
		*/
		virtual const char* what() const throw();

	protected:
		std::string ToUtf8Str(const std::wstring& s) const throw();
		std::wstring ToUtf16Str(const std::string& s) const throw();

		int				m_line;
		std::wstring	m_file;
		std::wstring	m_functionname;
		std::wstring	m_msg;
	};


	class EXODBCAPI AssertionException
		: public Exception
	{
	public:
		AssertionException(int line, const std::wstring& file, const std::wstring& functionname, const std::wstring& condition) throw()
			: Exception(line, file, functionname, L"") 
			, m_condition(condition)
		{};

		AssertionException(int line, const std::wstring& file, const std::wstring& functionname, const std::wstring& condition, const std::wstring& msg) throw()
			: Exception(line, file, functionname, msg) 
			, m_condition(condition)
		{};

		virtual ~AssertionException() {};

		virtual std::wstring GetName() const throw() { return L"AssertionException"; };
		virtual std::wstring ToString() const throw();

	protected:
		std::wstring m_condition;
	};
}


#endif // Exception
