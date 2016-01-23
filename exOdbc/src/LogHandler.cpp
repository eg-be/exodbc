/*!
* \file ObjectName.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Source file for name objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
#include "LogHandler.h"

// Same component headers
// Other headers
#include <iostream>
#include <sstream>

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	void StdErrLogHandler::LogMessage(LogLevel level, const std::wstring& msg, const std::wstring& filename /* = L"" */, int line /* = 0 */, const std::wstring& functionname /* = L"" */) const
	{
		std::wstringstream ws;

		switch (level)
		{
		case exodbc::LogLevel::Error: \
			ws << L"ERROR"; break; \
		case exodbc::LogLevel::Warning: \
			ws << L"WARNING"; break; \
		case exodbc::LogLevel::Info: \
			ws << L"INFO"; break; \
		case exodbc::LogLevel::Debug: \
			ws << L"DEBUG"; break; \
		}

		bool haveSourceInfo = !filename.empty() || line > 0 || !functionname.empty();
		if (haveSourceInfo)
			ws << L" [";

		if (!filename.empty())
			ws << filename;
		if (line > 0)
			ws << L"(" << line << L")";
		else if (!filename.empty())
			ws << L"(??)";
		if (!functionname.empty())
			ws << functionname;

		if (haveSourceInfo)
			ws << L"]";

		ws << L": " << msg;

		wcout << ws.str() << endl;
	}
}
