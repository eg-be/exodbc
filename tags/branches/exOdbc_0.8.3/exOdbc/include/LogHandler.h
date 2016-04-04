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
	/*!
	* \class	LogHandler
	* \brief	Interface for a LogHandler. Must be able to do something with log messages.
	*/
	class EXODBCAPI LogHandler
	{
	public:
		/*!
		* \brief	Called from LogManager on every log message received. Do something usefull with it.
		*/
		virtual void OnLogMessage(LogLevel level, const std::wstring& msg, const std::wstring& filename = L"", int line = 0, const std::wstring& functionname = L"") const = 0;
	};

	typedef std::shared_ptr<LogHandler> LogHandlerPtr;

	/*!
	* \class	StdErrLogHandler
	* \brief	A simple LogHandler that prints all messages nicely formated to stderr.
	*/
	class EXODBCAPI StdErrLogHandler
		: public LogHandler
	{
	public:
		virtual void OnLogMessage(LogLevel level, const std::wstring& msg, const std::wstring& filename = L"", int line = 0, const std::wstring& functionname = L"") const;
	};

	typedef std::shared_ptr<StdErrLogHandler> StdErrLogHandlerPtr;


}
