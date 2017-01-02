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
#include <fstream>

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
		* \brief	Called from LogManager on every log message received. Do something useful with it.
		*/
		virtual void OnLogMessage(LogLevel level, const std::string& msg, const std::string& filename = u8"", int line = 0, const std::string& functionname = u8"") const = 0;
		
		/*!
		* \brief	Called from the LogManager to ensure that not two equal LogHandlers are registered.
		*/
		virtual bool operator==(const LogHandler& other) const = 0;
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
		virtual void OnLogMessage(LogLevel level, const std::string& msg, const std::string& filename = u8"", int line = 0, const std::string& functionname = u8"") const override;
		virtual bool operator==(const LogHandler& other) const override;
	};
	
	typedef std::shared_ptr<StdErrLogHandler> StdErrLogHandlerPtr;

	/*!
	* \class	NullLogHandler
	* \brief	A LogHandler that does nothing.
	*/
	class EXODBCAPI NullLogHandler
		: public LogHandler
	{
	public:
		virtual void OnLogMessage(LogLevel level, const std::string& msg, const std::string& filename = u8"", int line = 0, const std::string& functionname = u8"") const override {};
		virtual bool operator==(const LogHandler& other) const override;
	};
	
	typedef std::shared_ptr<NullLogHandler> NullLogHandlerPtr;

	/*!
	* \class	FileLogHandler
	* \brief	A simple LogHandler that prints all messages nicely formated to a file.
	* \details	A file stream will be opened on the first message received and closed on destruction.
	*			If prependTimestamp is true, messages are prefixed with a timestamp.
	*/
	class EXODBCAPI FileLogHandler
		: public LogHandler
	{
	public:
		FileLogHandler() = delete;
		FileLogHandler(const FileLogHandler& other) = delete;
		FileLogHandler(const std::string& filepath, bool prependTimestamp);

		virtual ~FileLogHandler();

		virtual void OnLogMessage(LogLevel level, const std::string& msg, const std::string& filename = u8"", int line = 0, const std::string& functionname = u8"") const override;
		virtual bool operator==(const LogHandler& other) const override;

		std::string GetFilepath() const noexcept { return m_filepath; };
	private:
		mutable std::ofstream m_filestream;
		bool m_prependTimestamp;
		std::string m_filepath;
		mutable bool m_firstMessage;
	};

	typedef std::shared_ptr<FileLogHandler> FileLogHandlerPtr;
}
