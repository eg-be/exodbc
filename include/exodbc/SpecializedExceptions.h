﻿/*!
* \file SpecializedExceptions.h
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
		SqlResultException(const std::string& sqlFunctionName, SQLRETURN ret, const std::string& msg = u8"") noexcept;


		/*!
		* \brief Create a new SqlResultException that collects error information available from the passed handle.
		*/
		SqlResultException(const std::string& sqlFunctionName, SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle, const std::string& msg = u8"") noexcept;

        
		virtual ~SqlResultException() {};

		std::string GetName() const noexcept override { return u8"exodbc::SqlResultException"; };
		std::string ToString() const noexcept override;

		/*!
		* \brief Returns the SQLRETURN value set during construction.
		*/
		SQLRETURN GetRet() const noexcept { return m_ret; };


		/*!
		* \brief Returns the list of errors collected during construction (if any were collected).
		*/
		ErrorHelper::SErrorInfoVector GetErrorInfos() const noexcept { return m_errors; };


		/*!
		* \brief Returns true if the list of errors collected during construction contains
		*		one SErrorInfo with passed SQLSTATE.
		*/
		bool HasErrorInfo(const SQLCHAR* sqlState) const noexcept;

	protected:
		void FetchErrorInfo(SQLSMALLINT handleType, SQLHANDLE handle) noexcept;
		void BuildErrorMsg(const std::string& sqlFunctionName, SQLRETURN ret) noexcept;

		ErrorHelper::SErrorInfoVector m_errors;
		std::string m_errorMsg;
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
		IllegalArgumentException(const std::string msg) noexcept
			: Exception(msg)
		{};

		virtual ~IllegalArgumentException() {};

		std::string GetName() const noexcept override { return u8"exodbc::IllegalArgumentException"; };
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
			m_what = ToString();
		};

		/*!
		* \brief Create new NotSupportedException with additional message.
		*/
		NotSupportedException(Type notSupported, SQLSMALLINT smallInt, const std::string& msg) noexcept
			: Exception(msg)
			, m_notSupported(notSupported)
			, m_smallInt(smallInt)
		{
			m_what = ToString();
		};

		virtual ~NotSupportedException() {};

		std::string GetName() const noexcept override { return u8"exodbc:NotSupportedException"; };
		std::string ToString() const noexcept override;

	private:
		Type m_notSupported;
		SQLSMALLINT m_smallInt;
	};


	/*!
	* \class NotAllowedException
	* \brief Thrown if operation is not allowed, for example setting a column
	*		to NULL if it doesnt have the NULLABLE flag set.
	*/
	class EXODBCAPI NotAllowedException
		: public Exception
	{
	public:
		NotAllowedException() = delete;

		NotAllowedException(const std::string& msg) noexcept
			: Exception(msg)
		{};

		std::string GetName() const noexcept override { return u8"exodbc::NotAllowedException"; };
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
			m_what = ToString();
		};

		std::string GetName() const noexcept override { return u8"exodbc::WrapperException"; };
		std::string ToString() const noexcept override;

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
		NullValueException(std::string columnName)
			: Exception()
			, m_columnName(columnName)
		{
			m_what = ToString();
		}

		virtual ~NullValueException() {};

		std::string GetName() const noexcept override { return u8"exodbc::NullValueException"; };
		std::string ToString() const noexcept override;

	private:
		std::string m_columnName;
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
			: Exception(u8"Functionality not implemented")
		{};

		virtual ~NotImplementedException() {};

		std::string GetName() const noexcept override { return u8"exodbc::NotImplementedException"; };
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
		NotFoundException(const std::string& msg)
			: Exception(msg)
		{};

		virtual ~NotFoundException() {};

		std::string GetName() const noexcept override { return u8"exodbc::NotFoundException"; };
	};


	/*!
	* \class ConversionException
	* \brief Thrown if converting from and to utf8 / utf16 fails.
	*/
	class EXODBCAPI ConversionException
		: public Exception
	{
	public:
		ConversionException() = delete;

		/*!
		* \enum Type What conversion failed?
		*/
		enum class Type
		{
			UTF16_TO_UTF8 = 1,	///< utf-16 to utf-8 failed.
			UTF8_TO_UTF16 = 2	///< utf-8 to utf-16 failed.
		};

		/*!
		* \brief Create a new ConversionException.
		*/
		ConversionException(Type conversion)
			: Exception()
			, m_conversion(conversion)
		{
			m_what = ToString();
		};

		/*!
		* \brief Create new ConversionException, adding a message.
		*			Do not include what failed converting, it might be converted for output
		*			later again.
		*/
		ConversionException(Type conversion, const std::string& msg)
			: Exception(msg)
			, m_conversion(conversion)
		{
			m_what = ToString();
		};

		virtual ~ConversionException() {};

		std::string GetName() const noexcept override { return u8"exodbc::ConversionException"; };
		std::string ToString() const noexcept override;

	private:
		Type m_conversion;
	};
}
