/*!
* \file Exception.h
* \author Elias Gerber <eg@zame.ch>
* \date 24.01.2015
* \brief Header file for the Exception class and its helpers.
* \copyright wxWindows Library Licence, Version 3.1
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
		Exception(const std::wstring& msg) throw() : m_msg(msg), m_line(0) {};

		Exception(const exception&) throw() {};

		virtual ~Exception() throw() {};

		virtual std::wstring ToString() const throw();
		virtual std::wstring GetName() const throw()	{ return L"Exception"; };

		void SetSourceInformation(int line, const std::wstring& fileName, const std::wstring& functionName) throw();

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
			: Exception()
			, m_condition(condition)
		{
			SetSourceInformation(line, file, functionname);
		};

		AssertionException(int line, const std::wstring& file, const std::wstring& functionname, const std::wstring& condition, const std::wstring& msg) throw()
			: Exception(msg)
			, m_condition(condition)
		{
			SetSourceInformation(line, file, functionname);
		};

		virtual ~AssertionException() {};

		virtual std::wstring GetName() const throw() { return L"AssertionException"; };
		virtual std::wstring ToString() const throw();

	protected:
		std::wstring m_condition;
	};


	class EXODBCAPI SqlResultException
		: public Exception
	{
	public:
		SqlResultException(const std::wstring& sqlFunctionName, SQLRETURN ret, const std::wstring& msg = L"") throw();
		SqlResultException(const std::wstring& sqlFunctionName, SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle, const std::wstring& msg = L"") throw();

		virtual ~SqlResultException() {};

		virtual std::wstring GetName() const throw() { return L"SqlResultException"; };
		virtual std::wstring ToString() const throw();

	protected:
		void FetchErrorInfo(SQLSMALLINT handleType, SQLHANDLE handle) throw();
		void BuildErrorMsg(const std::wstring& sqlFunctionName, SQLRETURN ret) throw();

		std::vector<SErrorInfo> m_errors;
		std::wstring m_errorMsg;
	};


	class EXODBCAPI IllegalArgumentException
		: public Exception
	{
	public:
		IllegalArgumentException(const std::wstring msg) throw()
			: Exception(msg)
		{};

		virtual ~IllegalArgumentException() {};

		virtual std::wstring GetName() const throw() { return L"IllegalArgumentException"; };
	};

}


#endif // Exception