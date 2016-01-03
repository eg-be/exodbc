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
			m_what = w2s(ToString()); 
		};
		
		/*!
		* \brief Create Exception with passed message. 
		*/
		Exception(const std::wstring& msg) noexcept 
			: std::runtime_error("exodbc::Exception")
			, m_line(0)
			, m_msg(msg) 
		{ 
			m_what = w2s(ToString()); 
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
		* \brief	Returns the same as ToString(), but with a way to primitive
		*			transformation to std::string (from std::wstring). Use only
		*			ascii chars in all exceptions, or you will get garbage.
		*/
		virtual const char* what() const noexcept;

	protected:

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
	public:
		AssertionException() = delete;

		/*!
		* \brief Create AssertionException with source information.
		*/
		AssertionException(int line, const std::wstring& file, const std::wstring& functionname, const std::wstring& condition) noexcept
			: Exception()
			, m_condition(condition)
		{
			SetSourceInformation(line, file, functionname);
			m_what = w2s(ToString());
		};

		/*!
		* \brief Create AssertionException with source information and message.
		*/
		AssertionException(int line, const std::wstring& file, const std::wstring& functionname, const std::wstring& condition, const std::wstring& msg) noexcept
			: Exception(msg)
			, m_condition(condition)
		{
			SetSourceInformation(line, file, functionname);
			m_what = w2s(ToString());
		};

		virtual ~AssertionException() {};

		std::wstring GetName() const noexcept override { return L"exodbc::AssertionException"; };
		std::wstring ToString() const noexcept override;

	protected:
		std::wstring m_condition;
	};


	/*!
	* \class SqlResultException
	* \brief Thrown if a SQLFunction does not return successfully.
	* \details Can collect error information during construction.
	*/
	class EXODBCAPI SqlResultException
		: public Exception
	{
	public:
		SqlResultException() = delete;

		/*!
		* \brief Create a new SqlResultException with the information passed.
		*/
		SqlResultException(const std::wstring& sqlFunctionName, SQLRETURN ret, const std::wstring& msg = L"") noexcept;

		/*!
		* \brief Create a new SqlResultException that collects error information available from the passed handle.
		*/
		SqlResultException(const std::wstring& sqlFunctionName, SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle, const std::wstring& msg = L"") noexcept;

		virtual ~SqlResultException() {};

		std::wstring GetName() const noexcept override { return L"exodbc::SqlResultException"; };
		std::wstring ToString() const noexcept override;

		/*!
		* \brief Returns the SQLRETURN value set during construction.
		*/
		SQLRETURN GetRet() const noexcept { return m_ret; };

	protected:
		void FetchErrorInfo(SQLSMALLINT handleType, SQLHANDLE handle) noexcept;
		void BuildErrorMsg(const std::wstring& sqlFunctionName, SQLRETURN ret) noexcept;

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
	public:
		IllegalArgumentException() = delete;

		/*!
		* \brief Create new IllegalArgumentException with given message.
		*/
		IllegalArgumentException(const std::wstring msg) noexcept
			: Exception(msg)
		{};

		virtual ~IllegalArgumentException() {};

		std::wstring GetName() const noexcept override { return L"exodbc::IllegalArgumentException"; };
	};


	/*!
	* \class NotSupportedException
	* \brief Thrown on anything not supported for now either SQL C Type or SQL Type.
	* \details Depending on the type, the formated message will try to add some
	*			human-readable information.
	*/
	class EXODBCAPI NotSupportedException
		: public Exception
	{
	public:
		NotSupportedException() = delete;

		/*!
		* \enum Type What was not supported?
		*/
		enum class Type
		{
			SQL_C_TYPE = 1,	///< SQL C Type is not supported.
			SQL_TYPE = 2	///< SQL Type is not supported.
		};

		/*!
		* \brief Create new NotSupportedException.
		*/
		NotSupportedException(Type notSupported, SQLSMALLINT smallInt) noexcept
			: Exception()
			, m_notSupported(notSupported)
			, m_smallInt(smallInt)
		{
			m_what = w2s(ToString());
		};

		/*!
		* \brief Create new NotSupportedException with additional message.
		*/
		NotSupportedException(Type notSupported, SQLSMALLINT smallInt, const std::wstring& msg) noexcept
			: Exception(msg)
			, m_notSupported(notSupported)
			, m_smallInt(smallInt)
		{
			m_what = w2s(ToString());
		};

		virtual ~NotSupportedException() {};

		std::wstring GetName() const noexcept override { return L"exodbc:NotSupportedException"; };
		std::wstring ToString() const noexcept override;

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
	public:
		WrapperException() = delete;

		/*!
		* \brief	Create a new WrapperException. The Message of the WrapperException will be included
		*			in the format output of this WrapperException.
		* \details	Mostly used to wrap eventually occuring boost::exceptions into an Exception derived from
		*			exodbc::Exception.
		*/
		WrapperException(const std::exception& ex) noexcept
			: Exception()
			, m_innerExceptionMsg(ex.what())
		{
			m_what = w2s(ToString());
		};

		std::wstring GetName() const noexcept override { return L"exodbc::WrapperException"; };
		std::wstring ToString() const noexcept override;

	private:
		std::string m_innerExceptionMsg;
	};


	/*!
	* \class NullValueException
	* \brief Thrown if trying to access a value that is NULL.
	*/
	class EXODBCAPI NullValueException
		: public Exception
	{
	public:
		NullValueException() = delete;

		/*!
		* \brief Create new NullValueException, the passed columnName is currently NULL.
		*/
		NullValueException(std::wstring columnName)
			: Exception()
			, m_columnName(columnName)
		{
			m_what = w2s(ToString());
		}

		virtual ~NullValueException() {};

		std::wstring GetName() const noexcept override { return L"exodbc::NullValueException"; };
		std::wstring ToString() const noexcept override;

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

		std::wstring GetName() const noexcept override { return L"exodbc::NotImplementedException"; };
	};


	/*!
	* \class NotFoundException
	* \brief Thrown on not found stuff.
	*/
	class EXODBCAPI NotFoundException
		: public Exception
	{
	public:
		NotFoundException() = delete;

		/*!
		* \brief Create new NotFoundException, adding a message about what was not found.
		*/
		NotFoundException(const std::wstring& msg)
			: Exception(msg)
		{};

		virtual ~NotFoundException() {};

		std::wstring GetName() const noexcept override { return L"exodbc::NotFoundException"; };
	};
}
