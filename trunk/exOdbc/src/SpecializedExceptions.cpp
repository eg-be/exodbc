/*!
* \file SpecializedExceptions.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Source file for specialized Exceptions and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

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
	SqlResultException::SqlResultException(const std::wstring& sqlFunctionName, SQLRETURN ret, const std::wstring& msg /* = L"" */) noexcept
		: Exception(msg)
		, m_ret(0)
	{
		// We have no Error-Info to fetch
		BuildErrorMsg(sqlFunctionName, ret);
		m_what = w2s(ToString());
	}


	SqlResultException::SqlResultException(const std::wstring& sqlFunctionName, SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle, const std::wstring& msg /* = L"" */) noexcept
		: Exception(msg)
		, m_ret(ret)
	{
		// Fetch error-information from the database and build the error-msg
		FetchErrorInfo(handleType, handle);
		BuildErrorMsg(sqlFunctionName, ret);
		m_what = w2s(ToString());
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
			errInfo.Msg = (boost::wformat(L"Failed to fetch Error-Info: %s") % e.ToString()).str();
			m_errors.push_back(errInfo);
		}
	}


	void SqlResultException::BuildErrorMsg(const std::wstring& sqlFunctionName, SQLRETURN ret) noexcept
	{
		try
		{
			std::wstringstream ws;
			ws << L"SQL-Function " << sqlFunctionName << L" returned ";
			ws << SqlReturn2s(ret) << L"(" << ret << L") with " << m_errors.size() << " ODBC-Error(s):";
			m_errorMsg = ws.str();
		}
		catch (const Exception& e)
		{
			HIDE_UNUSED(e);
			m_errorMsg = L"Failed to BuildErrorMsg";
		}
	}


	std::wstring SqlResultException::ToString() const noexcept
	{
		std::wstringstream ws;
		ws << Exception::ToString();
		if (!m_errorMsg.empty())
		{
			ws << L": " << m_errorMsg;
		}
		ws << L"\n";
		for (size_t i = 0; i < m_errors.size(); i++)
		{
			ws << m_errors[i].ToString() << L"\n";
		}
		return ws.str();
	}


	std::wstring NotSupportedException::ToString() const noexcept
	{
		std::wstringstream ws;
		ws << Exception::ToString();
		ws << L"Not Supported ";
		switch (m_notSupported)
		{
		case Type::SQL_C_TYPE:
			ws << L"SQL C Type " << SqLCType2s(m_smallInt) << L" (" << m_smallInt << L")";
			break;
		case Type::SQL_TYPE:
			ws << L"SQL Type" << SqlType2s(m_smallInt) << L" (" << m_smallInt << L")";
			break;
		}
		return ws.str();
	}


	std::wstring WrapperException::ToString() const noexcept
	{
		std::wstringstream ws;
		ws << Exception::ToString();
		ws << s2w(m_innerExceptionMsg);
		return ws.str();
	}


	std::wstring NullValueException::ToString() const noexcept
	{
		std::wstringstream ws;
		ws << Exception::ToString();
		ws << L"Cannot fetch value of column '" << m_columnName << L"', the value is NULL.";
		return ws.str();
	}

}
