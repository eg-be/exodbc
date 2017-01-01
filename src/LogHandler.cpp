/*!
* \file LogHandler.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Source file for name objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "LogHandler.h"

// Same component headers
// Other headers
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	std::wstring FormatLogMessage(LogLevel level, const std::wstring& msg, const std::wstring& filename /* = L"" */, int line /* = 0 */, const std::wstring& functionname /* = L"" */)
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
			ws << L"(\?\?)";
		if (!functionname.empty())
			ws << functionname;

		if (haveSourceInfo)
			ws << L"]";

		ws << L": " << msg;

		return ws.str();
	}

	void StdErrLogHandler::OnLogMessage(LogLevel level, const std::wstring& msg, const std::wstring& filename /* = L"" */, int line /* = 0 */, const std::wstring& functionname /* = L"" */) const
	{
		wcout << FormatLogMessage(level, msg, filename, line, functionname) << endl;
	}


	bool StdErrLogHandler::operator==(const LogHandler& other) const
	{
		try
		{
			// all StdLogHandlers are equal
			dynamic_cast<const StdErrLogHandler&>(other);
			return true;
		}
		catch (const std::bad_cast& ex) 
		{
			HIDE_UNUSED(ex);
			return false;
		}
	}


	bool NullLogHandler::operator==(const LogHandler& other) const
	{
		try
		{
			// all NullLogHandler are equal
			dynamic_cast<const NullLogHandler&>(other);
			return true;
		}
		catch (const std::bad_cast& ex)
		{
			HIDE_UNUSED(ex);
			return false;
		}
	}


	FileLogHandler::FileLogHandler(const std::wstring& filepath, bool prependTimestamp)
		: m_prependTimestamp(prependTimestamp)
		, m_filepath(filepath)
		, m_firstMessage(true)
	{
	}


	FileLogHandler::~FileLogHandler()
	{
		if (m_filestream.is_open())
		{
			m_filestream.close();
		}
	}


	void FileLogHandler::OnLogMessage(LogLevel level, const std::wstring& msg, const std::wstring& filename /* = L"" */, int line /* = 0 */, const std::wstring& functionname /* = L"" */) const
	{
		if (m_firstMessage)
		{
			m_filestream = wofstream(m_filepath, std::ofstream::out);
			m_firstMessage = false;
		}
		if (m_filestream.is_open())
		{
			if (m_prependTimestamp)
			{
				auto t = std::time(nullptr);
				std::tm timeinfo;
				localtime_s(&timeinfo, &t);
				//*std::localtime(&t);
				m_filestream << std::put_time(&timeinfo, L"%d-%m-%Y %H-%M-%S") << L": ";
			}

			m_filestream << FormatLogMessage(level, msg, filename, line, functionname) << endl;
		}
	}


	bool FileLogHandler::operator==(const LogHandler& other) const
	{
		try
		{
			// equal if the same file
			auto& otherFileLogger = dynamic_cast<const FileLogHandler&>(other);
			return otherFileLogger.GetFilepath() == GetFilepath();
		}
		catch (const std::bad_cast& ex)
		{
			HIDE_UNUSED(ex);
			return false;
		}
	}

}
