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
	Exception::~Exception()
	{
	}

	// Implementation
	// --------------
	const char* Exception::what() const throw()
	{
		return m_what.c_str();
	}


	void Exception::SetSourceInformation(int line, const std::wstring& fileName, const std::wstring& functionName)
	{
		m_line = line;
		m_file = fileName;
		m_functionname = functionName;

		m_what = w2s(ToString());
	}


	std::wstring Exception::ToString() const throw()
	{
		std::wstringstream ws;
		ws << GetName();
		if (m_line > 0)
		{
			// keep only filename
			std::wstring fname = m_file;
			size_t pos = m_file.rfind(L"\\");
			if (pos != std::wstring::npos && m_file.length() > pos + 1)
			{
				fname = m_file.substr(pos + 1);
			}
			ws << L"[" << fname << L"(" << m_line << L")@" << m_functionname << L"]";
		}
		if (!m_msg.empty())
		{
			ws << L": " << m_msg;
		}
		return ws.str();
	}


	//std::string Exception::ToUtf8Str(const std::wstring& s) const throw()
	//{
	//	try
	//	{
	//		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//		std::string narrow = converter.to_bytes(m_msg);
	//		return narrow.c_str();
	//	}
	//	catch (std::exception e)
	//	{
	//		return e.what();
	//	}
	//}


	//std::wstring Exception::ToUtf16Str(const std::string& s) const throw()
	//{
	//	try
	//	{
	//		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//		std::wstring wide = converter.from_bytes(s);
	//		return wide;
	//	}
	//	catch (std::exception e)
	//	{
	//		try
	//		{
	//			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	//			std::wstring wide = converter.from_bytes(e.what());
	//			return wide;
	//		}
	//		catch (std::exception e)
	//		{
	//			return L"Failed to Convert Exception-Message to wide-string";
	//		}
	//	}
	//}


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
		m_what = w2s(ToString());
	}

	SqlResultException::SqlResultException(const std::wstring& sqlFunctionName, SQLRETURN ret, SQLSMALLINT handleType, SQLHANDLE handle, const std::wstring& msg /* = L"" */) throw()
		: Exception(msg)
	{
		// Fetch error-information from the database and build the error-msg
		FetchErrorInfo(handleType, handle);
		BuildErrorMsg(sqlFunctionName, ret);
		m_what = w2s(ToString());
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
			ws << SqlReturn2s(ret) << L"(" << ret << L") with " << m_errors.size() << " ODBC-Error(s):";
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


	std::wstring NotSupportedException::ToString() const throw()
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



	std::wstring WrapperException::ToString() const throw()
	{
		std::wstringstream ws;
		ws << Exception::ToString();
		std::string w(m_ex.what());
		ws << s2w(w);
		return ws.str();
	}


	std::wstring CastException::ToString() const throw()
	{
		std::wstringstream ws;
		ws << Exception::ToString();
		ws << L"Cannot cast from ODBC C Type " << SqLCType2s(m_cSourceType) << L"(" << m_cSourceType << L") ";
		ws << L"to ODBC C Type " << SqLCType2s(m_cDestType) << L"(" << m_cDestType << L")";
		return ws.str();
	}

	// Interfaces
	// ----------

}
