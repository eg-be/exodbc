/*!
* \file LogManager.h
* \author Elias Gerber <eg@elisium.ch>
* \date 23.01.2016
* \brief Header file for LogManager.
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
		Error,
		Warning,
		Info,
		Debug
	};

	class EXODBCAPI LogManager
	{
	public:
		LogManager();

		void RegisterLogHandler(LogHandlerPtr pHandler);

		void ClearLogHandlers();

		void LogMessage(LogLevel level, const std::wstring& msg, const std::wstring& file = L"", int line = 0, const std::wstring& function = L"") const;

	private:
		std::vector<LogHandlerPtr> m_logHandlers;
		mutable std::mutex m_logHandlersMutex;
	};
}

// Generic Log-entry
#define LOG_MSG(logLevel, msg) \
	do { \
    switch(logLevel) \
	{ \
	case exodbc::LogLevel::Error: \
		std::wcerr << L"ERROR: "; break;\
	case exodbc::LogLevel::Warning: \
		std::wcerr << L"WARNING: "; break;\
	case exodbc::LogLevel::Info: \
		std::wcerr << L"INFO: "; break;\
	case exodbc::LogLevel::Debug: \
		std::wcerr << L"DEBUG: "; break;\
	} \
	std::wcerr << __FILEW__ << L"(" << __LINE__ << L") " << __FUNCTIONW__ << L": " << msg << std::endl; \
	} while( 0 )

// Generic Log-entry shortcuts
#define LOG_ERROR(msg) LOG_MSG(exodbc::LogLevel::Error, msg)
#define LOG_WARNING(msg) LOG_MSG(exodbc::LogLevel::Warning, msg)
#define LOG_INFO(msg) LOG_MSG(exodbc::LogLevel::Info, msg)
#define LOG_DEBUG(msg) LOG_MSG(exodbc::LogLevel::Debug, msg)

// ODBC-Logging
#define LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, logLevel) \
	do { \
		std::wstring logOdbcMsgMsg = exodbc::FormatOdbcMessages(hEnv, hDbc, hStmt, hDesc, ret, L#SqlFunction, msg); \
		LOG_MSG(logLevel, logOdbcMsgMsg); \
	} while( 0 )

// ODBC-Loggers, with a message
#define LOG_ERROR_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, exodbc::LogLevel::Error)
#define LOG_WARNING_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, exodbc::LogLevel::Warning)
#define LOG_INFO_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg) LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, exodbc::LogLevel::Info)

// ODBC-Loggers, no message
#define LOG_ERROR_ODBC(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction) LOG_ERROR_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, L"")
#define LOG_WARNING_ODBC(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction) LOG_WARNING_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, L"")
#define LOG_INFO_ODBC(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction) LOG_INFO_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, L"")

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

