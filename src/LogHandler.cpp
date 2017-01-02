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
	std::string FormatLogMessage(LogLevel level, const std::string& msg, const std::string& filename /* = u8"" */, int line /* = 0 */, const std::string& functionname /* = u8"" */)
	{
		std::stringstream ss;

		switch (level)
		{
		case exodbc::LogLevel::Error: \
			ss << L"ERROR"; break; \
		case exodbc::LogLevel::Warning: \
			ss << L"WARNING"; break; \
		case exodbc::LogLevel::Info: \
			ss << L"INFO"; break; \
		case exodbc::LogLevel::Debug: \
			ss << L"DEBUG"; break; \
		}

		bool haveSourceInfo = !filename.empty() || line > 0 || !functionname.empty();
		if (haveSourceInfo)
			ss << L" [";

		if (!filename.empty())
			ss << filename;
		if (line > 0)
			ss << L"(" << line << L")";
		else if (!filename.empty())
			ss << L"(\?\?)";
		if (!functionname.empty())
			ss << functionname;

		if (haveSourceInfo)
			ss << L"]";

		ss << L": " << msg;

		return ss.str();
	}

	void StdErrLogHandler::OnLogMessage(LogLevel level, const std::string& msg, const std::string& filename /* = u8"" */, int line /* = 0 */, const std::string& functionname /* = u8"" */) const
	{
#ifdef _WIN32
		wcout << utf8ToUtf16(FormatLogMessage(level, msg, filename, line, functionname)) << endl;
#else
		cout << FormatLogMessage(level, msg, filename, line, functionname) << endl;
#endif
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


	FileLogHandler::FileLogHandler(const std::string& filepath, bool prependTimestamp)
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


	void FileLogHandler::OnLogMessage(LogLevel level, const std::string& msg, const std::string& filename /* = u8"" */, int line /* = 0 */, const std::string& functionname /* = u8"" */) const
	{
		if (m_firstMessage)
		{
			m_filestream = std::ofstream(SQLAPICHARCONVERT(m_filepath), std::ofstream::out);
			m_firstMessage = false;
		}
		if (m_filestream.is_open())
		{
			if (m_prependTimestamp)
			{
                std::time_t t = std::time(nullptr);
                std::tm tm = *std::localtime(&t);
				m_filestream << std::put_time(&tm, "%d-%m-%Y %H-%M-%S") << u8": ";
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
