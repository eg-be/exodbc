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

	// Interfaces
	// ----------

}
