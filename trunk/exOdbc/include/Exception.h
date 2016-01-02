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
#pragma  warning(push)
#pragma warning(disable:4275)

	/*!
	* \class Exception
	* \brief Base class of all exceptions thrown.
	*/
	class EXODBCAPI Exception
		: public std::runtime_error
	{
	public:
		Exception() throw() : std::runtime_error("exodbc::Exception"), m_line(0) { m_what = w2s(ToString()); };
		Exception(const std::wstring& msg) throw() : std::runtime_error("exodbc::Exception"), m_line(0), m_msg(msg) { m_what = w2s(ToString()); };

		virtual ~Exception() throw();

		virtual std::wstring ToString() const throw();
		virtual std::wstring GetName() const throw()	{ return L"exodbc::Exception"; };

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
		//std::string ToUtf8Str(const std::wstring& s) const throw();
		//std::wstring ToUtf16Str(const std::string& s) const throw();

		int				m_line;
		std::wstring	m_file;
		std::wstring	m_functionname;
		std::wstring	m_msg;
		std::string	m_what;
	};

#pragma warning(disable:4251)

	/*!
	* \class AssertionException
	* \brief Thrown if trapped on assertion.
	*/
	class EXODBCAPI AssertionException
		: public Exception
	{
	private:
		AssertionException() : Exception() {};

	public:
		AssertionException(int line, const std::wstring& file, const std::wstring& functionname, const std::wstring& condition) throw()
			: Exception()
			, m_condition(condition)
		{
			SetSourceInformation(line, file, functionname);
			m_what = w2s(ToString());
		};

		AssertionException(int line, const std::wstring& file, const std::wstring& functionname, const std::wstring& condition, const std::wstring& msg) throw()
			: Exception(msg)
			, m_condition(condition)
		{
			SetSourceInformation(line, file, functionname);
			m_what = w2s(ToString());
		};

		virtual ~AssertionException() {};

		virtual std::wstring GetName() const throw() { return L"exodbc::AssertionException"; };
		virtual std::wstring ToString() const throw();

	protected:
		std::wstring m_condition;
	};


	/*!
	* \class SqlResultException
	* \brief Thrown if a SQLFunction does not return successfully.
	*/
	class EXODBCAPI SqlResultException
		: public Exception
	{
	private:
		SqlResultException() : Exception() {};

	public:
		SqlResultException(const std::wstring& sqlFunctionName, SQLRETURN ret, const std::wstring& msg = L"") throw();
		SqlResultException(const std::wstring& sqlFunctionName, SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle, const std::wstring& msg = L"") throw();

		virtual ~SqlResultException() {};

		virtual std::wstring GetName() const throw() { return L"exodbc::SqlResultException"; };
		virtual std::wstring ToString() const throw();

		SQLRETURN GetRet() const noexcept { return m_ret; };

	protected:
		void FetchErrorInfo(SQLSMALLINT handleType, SQLHANDLE handle) throw();
		void BuildErrorMsg(const std::wstring& sqlFunctionName, SQLRETURN ret) throw();

		SErrorInfoVector m_errors;
		std::wstring m_errorMsg;
		SQLRETURN m_ret;
	};


	/*!
	* \class IllegalArgumentException
	* \brief Thrown on illegal arguments.
	*/
	class EXODBCAPI IllegalArgumentException
		: public Exception
	{
	private:
		IllegalArgumentException() : Exception() {};

	public:
		IllegalArgumentException(const std::wstring msg) throw()
			: Exception(msg)
		{};

		virtual ~IllegalArgumentException() {};

		virtual std::wstring GetName() const throw() { return L"exodbc::IllegalArgumentException"; };
	};


	/*!
	* \class NotSupportedException
	* \brief Thrown on not supported cominbations of data types, etc.
	*/
	class EXODBCAPI NotSupportedException
		: public Exception
	{
	private:
		NotSupportedException() : Exception() {};

	public:
		enum class Type
		{
			SQL_C_TYPE = 1,
			SQL_TYPE = 2
		};

		NotSupportedException(Type notSupported, SQLSMALLINT smallInt) throw()
			: Exception()
			, m_notSupported(notSupported)
			, m_smallInt(smallInt)
		{
			m_what = w2s(ToString());
		};
		NotSupportedException(Type notSupported, SQLSMALLINT smallInt, const std::wstring& msg) throw()
			: Exception(msg)
			, m_notSupported(notSupported)
			, m_smallInt(smallInt)
		{
			m_what = w2s(ToString());
		};

		virtual ~NotSupportedException() {};

		virtual std::wstring GetName() const throw() { return L"exodbc:NotSupportedException"; };
		virtual std::wstring ToString() const throw();

	private:
		Type m_notSupported;
		SQLSMALLINT m_smallInt;
	};


	/*!
	* \class WrapperException
	* \brief Wrapper around a std::exception
	*/
	class EXODBCAPI WrapperException
		: public Exception
	{
	private:
		WrapperException() : Exception() {};

	public:
		WrapperException(const std::exception& ex) throw()
			: Exception()
			, m_innerExceptionMsg(ex.what())
		{
			m_what = w2s(ToString());
		};

		virtual std::wstring GetName() const throw() { return L"exodbc::WrapperException"; };
		virtual std::wstring ToString() const throw();

	private:
		std::string m_innerExceptionMsg;
	};


	/*!
	* \class CastException
	* \brief Thrown if a Visitor cannot cast a value.
	*
	* Contains information about the source-type and the cast-target-type.
	*/
	class EXODBCAPI CastException
		: public Exception
	{
	private:
		CastException()
			: Exception()
			, m_cSourceType(SQL_UNKNOWN_TYPE)
			, m_cDestType(SQL_UNKNOWN_TYPE)
		{};

	public:
		CastException(SQLSMALLINT cSourceType, SQLSMALLINT cDestType)
			: Exception()
			, m_cSourceType(cSourceType)
			, m_cDestType(cDestType)
		{
			m_what = w2s(ToString());
		}

		virtual ~CastException() {};

		virtual std::wstring GetName() const throw() { return L"exodbc::CastException"; };
		virtual std::wstring ToString() const throw();

		virtual const char* what() const throw() { return m_what.c_str(); };

	private:
		std::string m_what;
	public:
		const SQLSMALLINT m_cSourceType;	///< Sql Type used as source value for the cast.
		const SQLSMALLINT m_cDestType;		///< ODBC C Type used as destination value for the cast.
	};	// class CastException


	/*!
	* \class NullValueException
	* \brief Thrown if trying to access a value that is NULL.
	*/
	class EXODBCAPI NullValueException
		: public Exception
	{
	private:
		NullValueException() : Exception() {};

	public:
		NullValueException(std::wstring columnName)
			: Exception()
			, m_columnName(columnName)
		{
			m_what = w2s(ToString());
		}

		virtual ~NullValueException() {};

		virtual std::wstring GetName() const throw() { return L"exodbc::NullValueException"; };
		virtual std::wstring ToString() const throw();

		virtual const char* what() const throw() { return m_what.c_str(); };

	private:
		std::wstring m_columnName;
	};


	/*!
	* \class NotImplementedException
	* \brief Thrown on not implemented stuff.
	*/
	class EXODBCAPI NotImplementedException
		: public Exception
	{
	public:
		NotImplementedException()
			: Exception(L"Functionality not implemented")
		{};

		virtual ~NotImplementedException() {};

		virtual std::wstring GetName() const throw() { return L"exodbc::NotImplementedException"; };
	};


	/*!
	* \class NotFoundException
	* \brief Thrown on not found stuff.
	*/
	class EXODBCAPI NotFoundException
		: public Exception
	{
	private:
		NotFoundException() : Exception() {};

	public:
		NotFoundException(const std::wstring& msg)
			: Exception(msg)
		{};

	public:
		virtual ~NotFoundException() {};

		virtual std::wstring GetName() const throw() { return L"exodbc::NotFoundException"; };
	};
}
