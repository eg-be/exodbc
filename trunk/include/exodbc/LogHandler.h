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
	* \brief	Interface for a LogHandler. Must be able to do something with log messages. Note that OnLogMessage()
	*			will be called from different threads from the LogManager.
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
	* \class	StdLogHandler
	* \brief	A simple LogHandler that prints all messages nicely formated to stderr or stdout, depending on their level.
	*			Messages with a LogLevel of Warning or Error are logged to stderr, Info and Debug to stdout.
	*			The logger can be tweaked to output to stderr or stdout only, or to both.
	* \details	Depending on RedirectToStdErr() and RedirectToStdOut() the logger will log to only stderr, stdout or 
	*			to both. If only one of RedirectToStdErr() or RedirectToStdOut() is set to true, messages are
	*			logged to that stream exclusively. If both are set, messages are logged to both streams. If none is
	*			of them are set, messages are logged to stderr or stdout depending on their LogLevel. This is the
	*			default behaviour.
	*/
	class EXODBCAPI StdLogHandler
		: public LogHandler
	{
	public:
		StdLogHandler()
			: m_redirectToStdErr(false)
			, m_redirectToStdOut(false)
			, m_showFileInfo(true)
		{};

		virtual void OnLogMessage(LogLevel level, const std::string& msg, const std::string& filename = u8"", int line = 0, const std::string& functionname = u8"") const override;
		virtual bool operator==(const LogHandler& other) const override;

		void RedirectToStdErr(bool enable) noexcept { m_redirectToStdErr = enable; };
		void RedirectoToStdOut(bool enable) noexcept { m_redirectToStdOut = enable; };
		void SetShowFileInfo(bool enable) noexcept { m_showFileInfo = enable; };

		bool IsRedirectingToStdErr() const noexcept { return m_redirectToStdErr; };
		bool IsRedirectingToStdOut() const noexcept { return m_redirectToStdOut; };
		bool IsShowingFileInfo() const noexcept { return m_showFileInfo; };

	private:
		bool m_showFileInfo;	///< If not set, filename, line-nr and function-name are not included.
		bool m_redirectToStdErr;
		bool m_redirectToStdOut;
		mutable std::mutex m_logMutex;
	};
	
	typedef std::shared_ptr<StdLogHandler> StdErrLogHandlerPtr;

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
		mutable std::mutex m_logMutex;
	};

	typedef std::shared_ptr<FileLogHandler> FileLogHandlerPtr;
}
