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


	class EXODBCAPI LogHandlerBase
	{
	public:
		LogHandlerBase()
			: m_showFileInfo(true)
			, m_showLogLevel(true)
		{};

		virtual ~LogHandlerBase() {};

		/*!
		* \brief If enabled, source information is shown when formating log messages.
		*/
		void SetShowFileInfo(bool enable) noexcept { m_showFileInfo = enable; };


		/*!
		* \brief If enabled, the log level is shown when formating log messages.
		*/
		void SetShowLogLevel(bool enable) noexcept { m_showLogLevel = enable; };


		/*!
		* \brief Disables outputting the log level in front of a log message for the given
		*			LogLevel. 
		*/
		void SetHideLogLevel(LogLevel level) noexcept { m_hideLogLevel.insert(level); };


		/*!
		* \brief Clear flag to not print passed LogLevel in front of log messages.
		*/
		void ClearHideLogLevel(LogLevel level) noexcept { m_hideLogLevel.erase(level); };


		/*!
		* \brief Returns true if source information is added to log messages.
		*/
		bool GetShowFileInfo() const noexcept { return m_showFileInfo; };


		/*!
		* \brief Returns true if the log level is added to log messages.
		*/
		bool GetShowLogLevel() const noexcept { return m_showLogLevel; };


		/*!
		* \brief Returns true if adding the passed log level in front of a message has been disabled.
		*/
		bool GetHideLogLevel(LogLevel level) const noexcept { return m_hideLogLevel.find(level) != m_hideLogLevel.end(); };


		/*!
		* \brief By default, a LogHandler logs all messages it receives from the LogManager.
		*		Use this method to set a custom LogLevel for this LogHandler. Note that this will not change
		*		the global LogLevel set in the LogManager.
		*/
		void SetLogLevel(LogLevel level) noexcept { m_hasCustomLogLevel = true; m_customLogLevel = level; }


		/*!
		* \brief Clear a custom LogLevel that has bee set on this LogHandler.
		*/
		void ClearLogLevel() noexcept { m_hasCustomLogLevel = false; };


	protected:
		bool CustomLogLevelHidesMsg(LogLevel msgLevel) const noexcept { return m_hasCustomLogLevel && (msgLevel < m_customLogLevel); };

		std::string FormatLogMessage(LogLevel level, const std::string& msg, const std::string& filename /* = u8"" */, int line /* = 0 */, const std::string& functionname /* = u8"" */) const noexcept;

		bool m_showFileInfo;	///< If not set, filename, line-nr and function-name are not included.
		bool m_showLogLevel;	///< If not set, the log level like ERROR, INFO, etc. is never shown at the beginning of the message
		std::set<LogLevel> m_hideLogLevel;	///< If a LogLevel is part of this list, it is NOT shown at the beginning of the message

		bool m_hasCustomLogLevel;
		LogLevel m_customLogLevel;
	};


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
		, public LogHandlerBase
	{
	public:
		StdLogHandler()
			: LogHandlerBase()
			, m_redirectToStdErr(false)
			, m_redirectToStdOut(false)
		{};

		virtual void OnLogMessage(LogLevel level, const std::string& msg, const std::string& filename = u8"", int line = 0, const std::string& functionname = u8"") const override;
		virtual bool operator==(const LogHandler& other) const override;

		void RedirectToStdErr(bool enable) noexcept { m_redirectToStdErr = enable; };
		void RedirectoToStdOut(bool enable) noexcept { m_redirectToStdOut = enable; };

		bool IsRedirectingToStdErr() const noexcept { return m_redirectToStdErr; };
		bool IsRedirectingToStdOut() const noexcept { return m_redirectToStdOut; };

	private:
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
		, public LogHandlerBase
	{
	public:
		NullLogHandler()
			: LogHandlerBase()
		{};

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
		, public LogHandlerBase
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
