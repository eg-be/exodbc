/*!
* \file LogManager.h
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Header file for LogManager and helper macros for Logging
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "exOdbc.h"

// Other headers
// System headers
#include <vector>
#include <mutex>

// Forward declarations
// --------------------
namespace exodbc
{
	class LogHandler;
	typedef std::shared_ptr<LogHandler> LogHandlerPtr;

	/*!
	* \enum LogLevel
	* \brief Log level to use when logging messages.
	*/
	enum class LogLevel
	{
		None = 100,		//< No Output at all. Do not use to create log messages.
		Error = 10,		//< Error and above.
		Warning = 9,	//< Warning and above.
		Output = 7,		//< Output from result sets.
		Info = 5,		//< Info and above.
		Debug = 3		//< Everything.
	};


	/*!
	* \class	LogLevelSetter
	* \brief	On construction, remembers the active global LogLevel, then 
	*			sets the global LogLevel to the passed value and on 
	*			destructions restores original global LogLevel.
	*/
	class EXODBCAPI LogLevelSetter
	{
	public:
		LogLevelSetter(LogLevel level);
		~LogLevelSetter();

	private:
		LogLevel m_originalLogLevel;
	};


	/*!
	* \class	LogManager
	* \brief	A class that can hold LogHandler instances and forward 
	*			log-messages to those LogHandlers.
	*			The class is thread-safe.
	*			The class is a Singleton, use Get() to get the only existing
	*			instance.
	* \details	The LogManager has a global level that can be set on it. It
	*			will only forward messages to its registered LogHandlers that
	*			have a logLevel greater or equal than the currently set 
	*			global LogLevel.
	*/
	class EXODBCAPI LogManager
	{
	private:
		/*!
		* \brief Default Constructor. Registers a StdErrLogHandler during construction
		*			and sets LogLevel to LogLevel::Info in release builds and
		*			LogLevel::Debug in debug builds.
		*/
		LogManager();

	public:
		/*!
		* \brief Get the Singleton-instance.
		*/
		static LogManager& Get();

		/*!
		* \brief Registers the passed LogHandler.
		*/
		void RegisterLogHandler(LogHandlerPtr pHandler);


		/*!
		* \brief Removes the passed LogHandler, does nothing if not registered.
		*/
		void RemoveLogHandler(LogHandlerPtr pHandler);


		/*!
		* \brief Removes all LogHandlers.
		*/
		void ClearLogHandlers() noexcept;

#ifdef _WIN32
		/*!
		* \brief Forwards the passed message to all LogHandlers registered. passed wstrings must be utf-16 encoded.
		* \details This method is only enabled if _WIN32 is defined. The strings must be utf-16 encoded,
		*			the method will only convert them to utf-8 strings and forward the call.
		*/
		void LogMessage(LogLevel level, const std::wstring& msg, const std::wstring& file = L"", int line = 0, const std::wstring& function = L"") const;
#endif

		/*!
		* \brief Forwards the passed message to all LogHandlers registered.
		*/
		void LogMessage(LogLevel level, const std::string& msg, const std::string& file = u8"", int line = 0, const std::string& function = u8"") const;


		/*!
		* \brief Output the passed message to stderr. On linux, this will output to cerr, on windows it will
		* convert the message to UTF-16 and output to wcerr. A newline is added at the end of msg.
		*/
		void WriteStdErr(const std::string& msg, bool appendEndl) const;


		/*!
		* \brief Output the passed message to stdout. On linux, this will output to cout, on windows it will
		* convert the message to UTF-16 and output to wcout. A newline is added at the end of msg.
		*/
		void WriteStdOut(const std::string& msg, bool appendEndl) const;


		/*!
		* \brief Return how many LogHandler instances are registered.
		*/
		size_t GetRegisteredLogHandlersCount() const noexcept;

		/*!
		* \brief Return all LogHandlers registered.
		*/
		std::vector<LogHandlerPtr> GetLogHandlers() const noexcept;

		/*!
		* \brief Set the global log level. LogManager will only forward log messages
		*			with a level higher or equal 
		*/
		void SetGlobalLogLevel(LogLevel level) noexcept;

		/*!
		* \brief
		*/
		LogLevel GetGlobalLogLevel() const noexcept;

	private:
		std::vector<LogHandlerPtr> m_logHandlers;
		mutable std::mutex m_logHandlersMutex;

		LogLevel m_globalLogLevel;
		mutable std::mutex m_globalLogLevelMutex;

		mutable std::mutex m_stdoutMutex;
		mutable std::mutex m_stderrMutex;
	};
}

// Generic Log-entry
#define LOG_MSG(logLevel, msg) \
	do { \
		exodbc::LogManager::Get().LogMessage(logLevel, msg, __FILE__, __LINE__, __FUNCTION__); \
	} while( 0 )

// Generic Log-entry shortcuts
#define LOG_ERROR(msg) LOG_MSG(exodbc::LogLevel::Error, msg)
#define LOG_WARNING(msg) LOG_MSG(exodbc::LogLevel::Warning, msg)
#define LOG_OUTPUT(msg) LOG_MSG(exodbc::LogLevel::Output, msg)
#define LOG_INFO(msg) LOG_MSG(exodbc::LogLevel::Info, msg)
#define LOG_DEBUG(msg) LOG_MSG(exodbc::LogLevel::Debug, msg)

// Write to std::cout or std::wcout, adding a std::endl after the message
#define WRITE_STDOUT_ENDL(msg) exodbc::LogManager::Get().WriteStdOut(msg, true);

// Write to std::cerr or std::wcerr
#define WRITE_STDERR_ENDL(msg) exodbc::LogManager::Get().WriteStdErr(msg, true);

// Write to std::cout or std::wcout
#define WRITE_STDOUT(msg) exodbc::LogManager::Get().WriteStdOut(msg, false);

// Write to std::cerr or std::wcerr, adding a std::endl after the message
#define WRITE_STDERR(msg) exodbc::LogManager::Get().WriteStdErr(msg, false);
