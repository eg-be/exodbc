/*!
* \file AssertionException.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Source file for the AssertionException and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "AssertionException.h"

// Same component headers
#include "LogManager.h"

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
	void exOnAssert(const std::wstring& file, int line, const std::wstring& function, const std::wstring& condition, const std::wstring& msg)
	{
		std::wstringstream ws;
		ws << L"ASSERTION failure!" << std::endl;
		ws << L" File:      " << file << std::endl;
		ws << L" Line:      " << line << std::endl;
		ws << L" Function:  " << function << std::endl;
		ws << L" Condition: " << condition << std::endl;
		if (msg.length() > 0)
		{
			ws << L" Msg:       " << msg << std::endl;
		}
		LOG_ERROR(ws.str());

		// Throw exception
		throw AssertionException(line, file, function, condition, msg);
	}


	void exOnAssert(const std::wstring& file, int line, const std::wstring& function, const std::string& condition, const std::wstring& msg)
	{
		exOnAssert(file, line, function, utf8ToUtf16(condition), msg);
	}


	std::wstring AssertionException::ToString() const noexcept
	{
		std::wstringstream ws;
		ws << Exception::ToString();
		ws << L"\n";
		ws << L"\tCondition: " << m_condition;
		return ws.str();
	}
}
