/*!
* \file LogManagerOdbcMacros.h
* \author Elias Gerber <eg@elisium.ch>
* \date 12.03.2017
* \brief Header file for ODBC helper macros for Logging. Use this file from 
*		 within CPP files only or you will pull in a linker dependency on
*		 odbc32.lib.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "LogManager.h"
#include "SpecializedExceptions.h"

// Other headers
// System headers

// ODBC-Logging
// ------------
#define LOG_ODBC_MSG(hEnv, hDbc, hStmt, hDesc, ret, SqlFunction, msg, logLevel) \
	do { \
		std::string logOdbcMsgMsg = exodbc::ErrorHelper::FormatOdbcMessages(hEnv, hDbc, hStmt, hDesc, ret, #SqlFunction, msg); \
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

// SqlResultException Macros
// -------------------------
#define THROW_IFN_SUCCEEDED_SILENT_MSG(sqlFunctionName, sqlReturn, handleType, handle, msg) \
	do { \
		if(!SQL_SUCCEEDED(sqlReturn)) { \
			SqlResultException ex(#sqlFunctionName, sqlReturn, handleType, handle, msg); \
			SET_EXCEPTION_SOURCE(ex); \
			throw ex; \
		} \
	} while(0)

#define THROW_IFN_SUCCEEDED_MSG(sqlFunctionName, sqlReturn, handleType, handle, msg) \
	do { \
		if(!SQL_SUCCEEDED(sqlReturn)) { \
			SqlResultException ex(#sqlFunctionName, sqlReturn, handleType, handle, msg); \
			SET_EXCEPTION_SOURCE(ex); \
			throw ex; \
		} \
		if(SQL_SUCCESS_WITH_INFO == sqlReturn) { \
			switch(handleType) { \
			case SQL_HANDLE_ENV: \
				LOG_INFO_ODBC(handle, SQL_NULL_HDBC, SQL_NULL_HSTMT, SQL_NULL_HDESC, sqlReturn, sqlFunctionName); \
				break; \
			case SQL_HANDLE_DBC: \
				LOG_INFO_ODBC(SQL_NULL_HENV, handle, SQL_NULL_HSTMT, SQL_NULL_HDESC, sqlReturn, sqlFunctionName); \
				break; \
			case SQL_HANDLE_STMT: \
				LOG_INFO_ODBC(SQL_NULL_HENV, SQL_NULL_HDBC, handle, SQL_NULL_HDESC, sqlReturn, sqlFunctionName); \
				break; \
			case SQL_HANDLE_DESC: \
				LOG_INFO_ODBC(SQL_NULL_HENV, SQL_NULL_HDBC, SQL_NULL_HSTMT, handle, sqlReturn, sqlFunctionName); \
				break; \
			default: \
				THROW_WITH_SOURCE(IllegalArgumentException, u8"Unknown handleType"); \
			} \
		} \
	} while(0)

#define THROW_IFN_SUCCESS_MSG(sqlFunctionName, sqlReturn, handleType, handle, msg) \
	do { \
		if(SQL_SUCCESS != sqlReturn) { \
			SqlResultException ex(#sqlFunctionName, sqlReturn, handleType, handle, msg); \
			SET_EXCEPTION_SOURCE(ex); \
			throw ex; \
		} \
	} while (0)

#define THROW_IFN_SUCCEEDED(sqlFunctionName, sqlReturn, handleType, handle) \
	do { \
		THROW_IFN_SUCCEEDED_MSG(sqlFunctionName, sqlReturn, handleType, handle, u8""); \
	} while(0) \

#define THROW_IFN_SUCCESS(sqlFunctionName, sqlReturn, handleType, handle) \
	do { \
		THROW_IFN_SUCCESS_MSG(sqlFunctionName, sqlReturn, handleType, handle, u8""); \
	} while(0) \

#define THROW_IFN_NO_DATA(sqlFunctionName, sqlReturn) \
	do { \
		if(SQL_NO_DATA != sqlReturn) \
		{ \
			SqlResultException ex(#sqlFunctionName, ret, u8"Expected SQL_NO_DATA."); \
			SET_EXCEPTION_SOURCE(ex); \
			throw ex; \
		} \
	} while(0)
