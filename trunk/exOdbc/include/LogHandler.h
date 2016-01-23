/*!
* \file LogHandler.h
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Header file for Log Handler.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "LogManager.h"

// Other headers
// System headers
#include <memory>

// Forward declarations
// --------------------

namespace exodbc
{
	class EXODBCAPI LogHandler
	{
	public:
		virtual void LogMessage(LogLevel level, const std::wstring& msg, const std::wstring& filename = L"", int line = 0, const std::wstring& functionname = L"") const = 0;
	};

	typedef std::shared_ptr<LogHandler> LogHandlerPtr;

	class EXODBCAPI StdErrLogHandler
		: public LogHandler
	{
	public:
		virtual void LogMessage(LogLevel level, const std::wstring& msg, const std::wstring& filename = L"", int line = 0, const std::wstring& functionname = L"") const;
	};


}
