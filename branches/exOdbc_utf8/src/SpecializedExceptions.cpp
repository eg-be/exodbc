/*!
* \file SpecializedExceptions.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Source file for specialized Exceptions and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "SpecializedExceptions.h"

// Same component headers
// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Construction
	// ------------

	// Destructor
	// -----------

	// Implementation
	// --------------
	SqlResultException::SqlResultException(const std::string& sqlFunctionName, SQLRETURN ret, const std::string& msg /* = u8"" */) noexcept
		: Exception(msg)
		, m_ret(0)
	{
		// We have no Error-Info to fetch
		BuildErrorMsg(sqlFunctionName, ret);
		m_what = ToString();
	}


	SqlResultException::SqlResultException(const std::string& sqlFunctionName, SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle, const std::string& msg /* = u8"" */) noexcept
		: Exception(msg)
		, m_ret(ret)
	{
		// Fetch error-information from the database and build the error-msg
		FetchErrorInfo(handleType, handle);
		BuildErrorMsg(sqlFunctionName, ret);
		m_what = ToString();
	}


	void SqlResultException::FetchErrorInfo(SQLSMALLINT handleType, SQLHANDLE handle) noexcept
	{
		try
		{
			m_errors = GetAllErrors(handleType, handle);
		}
		catch (const Exception& e)
		{
			// Append a faked Error-Info
			SErrorInfo errInfo;
			errInfo.ErrorHandleType = handleType;
			errInfo.SqlState[0] = 0;
			errInfo.NativeError = 0;
			errInfo.Msg = (boost::format(u8"Failed to fetch Error-Info: %s") % e.ToString()).str();
			m_errors.push_back(errInfo);
		}
	}


	void SqlResultException::BuildErrorMsg(const std::string& sqlFunctionName, SQLRETURN ret) noexcept
	{
		try
		{
			std::stringstream ss;
			ss << u8"SQL-Function " << sqlFunctionName << u8" returned ";
			ss << SqlReturn2s(ret) << u8"(" << ret << L") with " << m_errors.size() << " ODBC-Error(s):";
			m_errorMsg = ss.str();
		}
		catch (const Exception& e)
		{
			HIDE_UNUSED(e);
			m_errorMsg = u8"Failed to BuildErrorMsg";
		}
	}


	std::string SqlResultException::ToString() const noexcept
	{
		std::stringstream ss;
		ss << Exception::ToString();
		if (!m_errorMsg.empty())
		{
			ss << u8": " << m_errorMsg;
		}
		ss << u8"\n";
		for (size_t i = 0; i < m_errors.size(); i++)
		{
			ss << m_errors[i].ToString() << u8"\n";
		}
		return ss.str();
	}


	std::string NotSupportedException::ToString() const noexcept
	{
		std::stringstream ss;
		ss << Exception::ToString();
		ss << u8"Not Supported ";
		switch (m_notSupported)
		{
		case Type::SQL_C_TYPE:
			ss << u8"SQL C Type " << SqLCType2s(m_smallInt) << u8" (" << m_smallInt << u8")";
			break;
		case Type::SQL_TYPE:
			ss << u8"SQL Type" << SqlType2s(m_smallInt) << u8" (" << m_smallInt << u8")";
			break;
		}
		return ss.str();
	}


	std::string WrapperException::ToString() const noexcept
	{
		std::stringstream ss;
		ss << Exception::ToString();
		ss << m_innerExceptionMsg;
		return ss.str();
	}


	std::string NullValueException::ToString() const noexcept
	{
		std::stringstream ss;
		ss << Exception::ToString();
		ss << u8"Cannot fetch value of column '" << m_columnName << u8"', the value is NULL.";
		return ss.str();
	}

}
