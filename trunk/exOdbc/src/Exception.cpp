/*!
* \file Exception.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 24.01.2014
* \brief Source file for the Exception class and its helpers.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#include "stdafx.h"

// Own header
#include "Exception.h"

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
	const char* Exception::what() const throw()
	{
		return ToUtf8Str(m_msg).c_str();
	}


	void Exception::SetSourceInformation(int line, const std::wstring& fileName, const std::wstring& functionName)
	{
		m_line = line;
		m_file = fileName;
		m_functionname = functionName;
	}


	std::wstring Exception::ToString() const throw()
	{
		std::wstringstream ws;
		ws << GetName();
		if (m_line > 0)
		{
			ws << L"[" << m_file << L"(" << m_line << L")@" << m_functionname << L"]";
		}
		if (!m_msg.empty())
		{
			ws << L": " << m_msg;
		}
		return ws.str();
	}


	std::string Exception::ToUtf8Str(const std::wstring& s) const throw()
	{
		try
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			std::string narrow = converter.to_bytes(m_msg);
			return narrow.c_str();
		}
		catch (std::exception e)
		{
			return e.what();
		}
	}


	std::wstring Exception::ToUtf16Str(const std::string& s) const throw()
	{
		try
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
			std::wstring wide = converter.from_bytes(s);
			return wide;
		}
		catch (std::exception e)
		{
			try
			{
				std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
				std::wstring wide = converter.from_bytes(e.what());
				return wide;
			}
			catch (std::exception e)
			{
				return L"Failed to Convert Exception-Message to wide-string";
			}
		}
	}


	std::wstring AssertionException::ToString() const throw()
	{
		std::wstringstream ws;
		ws << Exception::ToString();
		ws << L"\n";
		ws << L"\tCondition: " << m_condition;
		return ws.str();
	}


	SqlResultException::SqlResultException(const std::wstring& sqlFunctionName, SQLRETURN ret, const std::wstring& msg /* = L"" */) throw()
		: Exception(msg)
	{
		// We have no Error-Info to fetch
		BuildErrorMsg(sqlFunctionName, ret);
	}

	SqlResultException::SqlResultException(const std::wstring& sqlFunctionName, SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle, const std::wstring& msg /* = L"" */) throw()
		: Exception(msg)
	{
		// Fetch error-information from the database and build the error-msg
		FetchErrorInfo(handleType, handle);
		BuildErrorMsg(sqlFunctionName, ret);
	}


	void SqlResultException::FetchErrorInfo(SQLSMALLINT handleType, SQLHANDLE handle) throw()
	{
		try
		{
			m_errors = GetAllErrors(handleType, handle);
		}
		catch (Exception e)
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


	void SqlResultException::BuildErrorMsg(const std::wstring& sqlFunctionName, SQLRETURN ret) throw()
	{
		try
		{
			std::wstringstream ws;
			ws << L"SQL-Function " << sqlFunctionName << L" returned ";
			ws << SqlReturn2s(ret) << L"(" << ret << L") with " << m_errors.size() << " ODBC-Errors";
			m_errorMsg = ws.str();
		}
		catch (Exception e)
		{
			m_errorMsg = L"Failed to BuildErrorMsg";
		}
	}


	std::wstring SqlResultException::ToString() const throw()
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
			ws << m_errors[i] << L"\n";
		}
		return ws.str();
	}


	// Interfaces
	// ----------

}
