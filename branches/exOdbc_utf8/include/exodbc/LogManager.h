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
	* \brief Log level to use when logging messages. Currently only sets some prefix..
	*/
	enum class LogLevel
	{
		Error = 10,
		Warning = 9,
		Info = 8,
		Debug = 4
	};

	/*!
	* \class	LogManager
	* \brief	A class that can hold LogHandler instances and forward log-messages
	*			to those LogHandlers.
	*			The class is thread-safe.
	*			The class is a Singleton, use Get() to get the only existing instance.
	* \details	The LogManager has a global level that can be set on it. It will only
	*			forward messages to its registered LogHandlers that have a logLevel
	*			greater or equal than the currently set LogLevel
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
		void WriteStdErr(const std::string& msg) const;


		/*!
		* \brief Output the passed message to stdout. On linux, this will output to cout, on windows it will
		* convert the message to UTF-16 and output to wcout. A newline is added at the end of msg.
		*/
		void WriteStdOut(const std::string& msg) const;


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
#define LOG_INFO(msg) LOG_MSG(exodbc::LogLevel::Info, msg)
#define LOG_DEBUG(msg) LOG_MSG(exodbc::LogLevel::Debug, msg)

// ODBC-Logging
#define LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, logLevel) \
	do { \
		std::string logOdbcMsgMsg = exodbc::FormatOdbcMessages(hEnv, hDbc, hStmt, hDesc, ret, #SqlFunction, msg); \
		LOG_MSG(logLevel, logOdbcMsgMsg); \
	} while( 0 )

// ODBC-Loggers, with a message
#define LOG_ERROR_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, exodbc::LogLevel::Error)
#define LOG_WARNING_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, exodbc::LogLevel::Warning)
#define LOG_INFO_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, exodbc::LogLevel::Info)

// ODBC-Loggers, no message
#define LOG_ERROR_ODBC(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction) LOG_ERROR_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, u8"")
#define LOG_WARNING_ODBC(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction) LOG_WARNING_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, u8"")
#define LOG_INFO_ODBC(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction) LOG_INFO_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, u8"")

// Log ODBC-Errors, from a specific handle (env, dbc, stmt, desc), with a message
#define LOG_ERROR_ENV_MSG(hEnv, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(hEnv, NULL, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_ERROR_DBC_MSG(hDbc, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(NULL, hDbc, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_ERROR_STMT_MSG(hStmt, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(NULL, NULL, hStmt, NULL, ret, SqlFunction, msgStr)
#define LOG_ERROR_DESC_MSG(hDesc, ret, SqlFunction, msgStr) LOG_ERROR_ODBC_MSG(NULL, NULL, NULL, hDesc, ret, SqlFunction, msgStr)

// Log ODBC-Warnings, from a specific handle (env, dbc or stmt), with a message
#define LOG_WARNING_ENV_MSG(hEnv, ret, SqlFunction, msgStr) LOG_WARNING_ODBC_MSG(hEnv, NULL, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_WARNING_DBC_MSG(hDbc, ret, SqlFunction, msgStr) LOG_WARNING_ODBC_MSG(NULL, hDbc, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_WARNING_STMT_MSG(hStmt, ret, SqlFunction, msgStr) LOG_WARNING_ODBC_MSG(NULL, NULL, hStmt, NULL, ret, SqlFunction, msgStr)
#define LOG_WARNING_DESC_MSG(hDesc, ret, SqlFunction, msgStr) LOG_WARNING_ODBC_MSG(NULL, NULL, NULL, hDesc, ret, SqlFunction, msgStr)

// Log ODBC-Infos, from a specific handle (env, dbc or stmt), with a message
#define LOG_INFO_ENV_MSG(hEnv, ret, SqlFunction, msgStr) LOG_INFO_ODBC_MSG(hEnv, NULL, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_INFO_DBC_MSG(hDbc, ret, SqlFunction, msgStr) LOG_INFO_ODBC_MSG(NULL, hDbc, NULL, NULL, ret, SqlFunction, msgStr)
#define LOG_INFO_STMT_MSG(hStmt, ret, SqlFunction, msgStr) LOG_INFO_ODBC_MSG(NULL, NULL, hStmt, NULL, ret, SqlFunction, msgStr)
#define LOG_INFO_DESC_MSG(hDesc, ret, SqlFunction, msgStr) LOG_INFO_ODBC_MSG(NULL, NULL, NULL, hDesc, ret, SqlFunction, msgStr)

// Log ODBC-Errors, from a specific handle (env, dbc or stmt), no message
#define LOG_ERROR_ENV(hEnv, ret, SqlFunction) LOG_ERROR_ODBC(hEnv, NULL, NULL, NULL, ret, SqlFunction)
#define LOG_ERROR_DBC(hDbc, ret, SqlFunction) LOG_ERROR_ODBC(NULL, hDbc, NULL, NULL, ret, SqlFunction)
#define LOG_ERROR_STMT(hStmt, ret, SqlFunction) LOG_ERROR_ODBC(NULL, NULL, hStmt, NULL, ret, SqlFunction)
#define LOG_ERROR_DESC(hDesc, ret, SqlFunction) LOG_ERROR_ODBC(NULL, NULL, NULL, hDesc, ret, SqlFunction)

// Log ODBC-Warnings, from a specific handle (env, dbc or stmt), no message
#define LOG_WARNING_ENV(hEnv, ret, SqlFunction) LOG_WARNING_ODBC(hEnv, NULL, NULL, NULL, ret, SqlFunction)
#define LOG_WARNING_DBC(hDbc, ret, SqlFunction) LOG_WARNING_ODBC(NULL, hDbc, NULL, NULL, ret, SqlFunction)
#define LOG_WARNING_STMT(hStmt, ret, SqlFunction) LOG_WARNING_ODBC(NULL, NULL, hStmt, NULL, ret, SqlFunction)
#define LOG_WARNING_DESC(hDesc, ret, SqlFunction) LOG_WARNING_ODBC(NULL, NULL, hDesc, NULL, ret, SqlFunction)

// Log ODBC-Infos, from a specific handle (env, dbc or stmt), no message
#define LOG_INFO_ENV(hEnv, ret, SqlFunction) LOG_INFO_ODBC(hEnv, NULL, NULL, NULL, ret, SqlFunction)
#define LOG_INFO_DBC(hDbc, ret, SqlFunction) LOG_INFO_ODBC(NULL, hDbc, NULL, NULL, ret, SqlFunction)
#define LOG_INFO_STMT(hStmt, ret, SqlFunction) LOG_INFO_ODBC(NULL, NULL, hStmt, NULL, ret, SqlFunction)
#define LOG_INFO_DESC(hDesc, ret, SqlFunction) LOG_INFO_ODBC(NULL, NULL, NULL, hDesc, ret, SqlFunction)

// Generic helpers to write to stdout / stderr
#define WRITE_STDOUT(msg) exodbc::LogManager::Get().WriteStdOut(msg);

// Generic STDERR
#define WRITE_STDERR(msg) exodbc::LogManager::Get().WriteStdErr(msg);