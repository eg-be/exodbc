/*!
* \file Exception.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 24.01.2014
* \brief Source file for the Exception class and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
*/

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
	const char* Exception::what() const noexcept
	{
		return m_what.c_str();
	}


	void Exception::SetSourceInformation(int line, const std::wstring& fileName, const std::wstring& functionName) noexcept
	{
		m_line = line;
		m_file = fileName;
		m_functionname = functionName;

		m_what = utf16ToUtf8(ToString());
	}


	std::wstring Exception::ToString() const noexcept
	{
		std::wstringstream ws;
		ws << GetName();
		ws << L"[";
		if (m_line > 0)
		{
			// keep only filename
			std::wstring fname = m_file;
			size_t pos = m_file.rfind(L"\\");
			if (pos != std::wstring::npos && m_file.length() > pos + 1)
			{
				fname = m_file.substr(pos + 1);
			}
			ws << fname << L"(" << m_line << L")@" << m_functionname;
		}
		ws << L"]";
		if (!m_msg.empty())
		{
			ws << L": " << m_msg;
		}
		return ws.str();
	}
}
