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


	void Exception::SetSourceInformation(int line, const std::string& fileName, const std::string& functionName) noexcept
	{
		m_line = line;
		m_file = fileName;
		m_functionname = functionName;

		m_what = ToString();
	}


	std::string Exception::ToString() const noexcept
	{
		std::stringstream ss;
		ss << GetName();
		ss << u8"[";
		if (m_line > 0)
		{
			// keep only filename
			std::string fname = m_file;
			size_t pos = m_file.rfind(u8"\\");
			if (pos != std::wstring::npos && m_file.length() > pos + 1)
			{
				fname = m_file.substr(pos + 1);
			}
			ss << fname << u8"(" << m_line << u8")@" << m_functionname;
		}
		ss << u8"]";
		if (!m_msg.empty())
		{
			ss << u8": " << m_msg;
		}
		return ss.str();
	}
}
