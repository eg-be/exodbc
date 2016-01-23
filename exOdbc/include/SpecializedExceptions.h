/*!
* \file SpezializedExceptions.h
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Header file for all specialized Exception classes and all
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

// SqlResultException Macros
// -------------------------

#define THROW_IFN_SUCCEEDED_SILENT_MSG(sqlFunctionName, sqlReturn, handleType, handle, msg) \
	do { \
		if(!SQL_SUCCEEDED(sqlReturn)) { \
			SqlResultException ex(L#sqlFunctionName, sqlReturn, handleType, handle, msg); \
			SET_EXCEPTION_SOURCE(ex); \
			throw ex; \
		} \
	} while(0)

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