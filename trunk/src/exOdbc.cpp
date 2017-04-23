/*!
* \file exOdbc.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.07.2014
* \brief Source file for the stuff from exOdbc.h
* \copyright GNU Lesser General Public License Version 3
*/ 

// Own header
#include "exOdbc.h"

// Same component headers
#include "Helpers.h"
#include "LogManager.h"
#include "SpecializedExceptions.h"

// Other headers
// Debug
#include "DebugNew.h"

namespace exodbc {

	// Static consts
	// -------------
	const SQLCHAR* ErrorHelper::SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED = (const SQLCHAR*)"HYC00";

	// Implementation
	// --------------
	std::string utf16ToUtf8(const std::wstring& w)
	{
		try
		{
			std::wstring_convert<
				std::codecvt_utf8_utf16< std::wstring::value_type >,
				std::wstring::value_type
			> utf16conv;
			return utf16conv.to_bytes(w);
		}
		catch (const std::exception& ex)
		{
			ConversionException ce(ConversionException::Type::UTF16_TO_UTF8, ex.what());
			SET_EXCEPTION_SOURCE(ce);
			throw ce;
		}
	}

	
	std::string utf16ToUtf8(const SQLWCHAR* w)
	{
		std::wstring ws = reinterpret_cast<const wchar_t*>(w);
        return utf16ToUtf8(ws);
    }
    
	
	std::wstring utf8ToUtf16(const std::string& s)
	{
		try
		{
			std::wstring_convert<
				std::codecvt_utf8_utf16< std::wstring::value_type >,
				std::wstring::value_type
			> utf16conv;
			return utf16conv.from_bytes(s);
		}
		catch (const std::exception& ex)
		{
			ConversionException ce(ConversionException::Type::UTF8_TO_UTF16, ex.what());
			SET_EXCEPTION_SOURCE(ce);
			throw ce;
		}
	}

	
	std::wstring utf8ToUtf16(const SQLCHAR* s)
	{
        return utf8ToUtf16(reinterpret_cast<const char*>(s));
    }


	std::string SqlTrueFalse2s(SQLSMALLINT b) noexcept
	{
		switch (b)
		{
		case SQL_TRUE:
			return u8"TRUE";
		case SQL_FALSE:
			return u8"FALSE";
		default:
			return u8"?????";
		}
	}


	std::string SqlReturn2s(SQLRETURN ret) noexcept
	{
		switch (ret)
		{
		case SQL_SUCCESS:
			return u8"SUCCESS";
		case SQL_SUCCESS_WITH_INFO:
			return u8"SUCCESS_WITH_INFO";
		case SQL_NO_DATA:
			return u8"SQL_NO_DATA";
		case SQL_ERROR:
			return u8"SQL_ERROR";
		case SQL_NEED_DATA:
			return u8"SQL_NEED_DATA";
		case SQL_INVALID_HANDLE:
			return u8"SQL_INVALID_HANDLE";
		default:
			return u8"???";
		}
	}


	std::string SqlType2s(SQLSMALLINT sqlType) noexcept
	{
		switch (sqlType)
		{
		case SQL_CHAR:
			return u8"CHAR";
		case SQL_VARCHAR:
			return u8"VARCHAR";
		case SQL_LONGVARCHAR:
			return u8"LONGVARCHAR";
		case SQL_WCHAR:
			return u8"WCHAR";
		case SQL_WVARCHAR:
			return u8"WVARCHAR";
		case SQL_WLONGVARCHAR:
			return u8"WLONGVARCHAR";
		case SQL_DECIMAL:
			return u8"DECIMAL";
		case SQL_NUMERIC:
			return u8"NUMERIC";
		case SQL_SMALLINT:
			return u8"SMALLINT";
		case SQL_INTEGER:
			return u8"INTEGER";
		case SQL_REAL:
			return u8"REAu8";
		case SQL_FLOAT:
			return u8"FLOAT";
		case SQL_DOUBLE:
			return u8"DOUBLE";
		case SQL_BIT:
			return u8"BIT";
		case SQL_TINYINT:
			return u8"TINYINT";
		case SQL_BIGINT:
			return u8"BIGINT";
		case SQL_BINARY:
			return u8"BINARY";
		case SQL_VARBINARY:
			return u8"VARBINARY";
		case SQL_LONGVARBINARY:
			return u8"LONGVARBINARY";
		case SQL_TYPE_DATE:	//[6]
			return u8"TYPE_DATE";
		case SQL_TYPE_TIME: //[6]
			return u8"TYPE_TIME";
		case SQL_TYPE_TIMESTAMP: //[6]
			return u8"TYPE_TIMESTAMP";
		case SQL_DATE:
			return u8"DATE";
		case SQL_TIME:
			return u8"TIME";
		case SQL_TIMESTAMP:
			return u8"TIMESTAMP";
			//		case SQL_TYPE_UTCDATETIME:
			//			return u8"TYPE_UTCDATETIME";
			//		case SQL_TYPE_UTCTIME:
			//			return u8"TYPE_UTCTIME";
		case SQL_INTERVAL_MONTH: //[7]
			return u8"INTERVAL_MONTH";
		case SQL_INTERVAL_YEAR: //[7]
			return u8"INTERVAL_YEAR";
		case SQL_INTERVAL_YEAR_TO_MONTH: //[7]
			return u8"INTERVAL_YEAR_TO_MONTH";
		case SQL_INTERVAL_DAY: //[7]
			return u8"INTERVAL_DAY";
		case SQL_INTERVAL_HOUR: ///[7]
			return u8"INTERVAL_HOUR";
		case SQL_INTERVAL_MINUTE: //[7]
			return u8"INTERVAL_MINUTE";
		case SQL_INTERVAL_SECOND: //[7]
			return u8"INTERVAL_SECOND";
		case SQL_INTERVAL_DAY_TO_HOUR: //[7]
			return u8"INTERVAL_DAY_TO_HOUR";
		case SQL_INTERVAL_DAY_TO_MINUTE: //[7]
			return u8"INTERVAL_DAY_TO_MINUTE";
		case SQL_INTERVAL_DAY_TO_SECOND: //[7]
			return u8"INTERVAL_DAY_TO_SECOND";
		case SQL_INTERVAL_HOUR_TO_MINUTE: //[7]
			return u8"INTERVAL_HOUR_TO_MINUTE";
		case SQL_INTERVAL_HOUR_TO_SECOND: //[7]
			return u8"INTERVAL_HOUR_TO_SECOND";
		case SQL_INTERVAL_MINUTE_TO_SECOND: //[7]
			return u8"INTERVAL_MINUTE_TO_SECOND";
		case SQL_GUID:
			return u8"GUID";
		default:
			return u8"???";
		}
	}


	std::string SqLCType2s(SQLSMALLINT sqlCType) noexcept
	{
		switch (sqlCType)
		{
		case SQL_C_STINYINT:
			return u8"SQL_C_STINYINT";
		case SQL_C_UTINYINT:
			return u8"SQL_C_UTINYINT";
		case SQL_C_SSHORT:
			return u8"SQL_C_SSHORT";
		case SQL_C_USHORT:
			return u8"SQL_C_USHORT";
		case SQL_C_SLONG:
			return u8"SQL_C_SLONG";
		case SQL_C_ULONG:
			// Note: This is also the type for SQL_C_BOOKMARK
			return u8"SQL_C_ULONG";
		case SQL_C_SBIGINT:
			return u8"SQL_C_SBIGINT";
		case SQL_C_UBIGINT:
			return u8"SQL_C_UBIGINT";
		case SQL_C_CHAR:
			return u8"SQL_C_CHAR";
		case SQL_C_WCHAR:
			return u8"SQL_C_WCHAR";
		case SQL_C_FLOAT:
			return u8"SQL_C_FLOAT";
		case SQL_C_DOUBLE:
			return u8"SQL_C_DOUBLE";
		case SQL_C_BIT:
			return u8"SQL_C_BIT";
		case SQL_C_BINARY:
			// Note: This is also the type for SQL_C_VARBOOKMARK:
			return u8"SQL_C_BINARY";
			//case SQL_C_BOOKMARK:
			//	return u8"SQL_C_BOOKMARK";
			//case SQL_C_VARBOOKMARK:
			//	return u8"SQL_C_VARBOOKMARK";
		case SQL_C_TYPE_DATE:
			return u8"SQL_C_TYPE_DATE";
		case SQL_C_TYPE_TIME:
			return u8"SQL_C_TYPE_TIME";
		case SQL_C_TYPE_TIMESTAMP:
			return u8"SQL_C_TYPE_TIMESTAMP";
		case SQL_C_NUMERIC:
			return u8"SQL_C_NUMERIC";
		case SQL_C_GUID:
			return u8"SQL_C_GUID";

			// Old odbc 2.x values
		case SQL_C_DATE:
			return u8"SQL_C_DATE";
		case SQL_C_TIME:
			return u8"SQL_C_TIME";
		case SQL_C_TIMESTAMP:
			return u8"SQL_C_TIMESTAMP";

		default:
			return u8"???";
		}
	}


	std::string SqlCType2OdbcS(SQLSMALLINT sqlCType) noexcept
	{
		switch (sqlCType)
		{
		case SQL_C_STINYINT:
			return u8"SQLSCHAR";
		case SQL_C_UTINYINT:
			return u8"SQLCHAR";
		case SQL_C_SSHORT:
			return u8"SQLSMALLINT";
		case SQL_C_USHORT:
			return u8"SQLUSMALLINT";
		case SQL_C_SLONG:
			return u8"SQLINTEGER";
		case SQL_C_ULONG:
			// Note: This is also the type for SQL_C_BOOKMARK
			return u8"SQLUINTEGER";
		case SQL_C_SBIGINT:
			return u8"SQLBIGNT";
		case SQL_C_UBIGINT:
			return u8"SQLUBIGINT";
		case SQL_C_CHAR:
			return u8"SQLCHAR*";
		case SQL_C_WCHAR:
			return u8"SQLWCHAR*";
		case SQL_C_FLOAT:
			return u8"SQLREAu8";
		case SQL_C_DOUBLE:
			return u8"SQLDOUBLE";
		case SQL_C_BIT:
			return u8"SQLCHAR";
		case SQL_C_BINARY:
			// Note: This is also the type for SQL_C_VARBOOKMARK:
			return u8"SQLCHAR*";
			//case SQL_C_BOOKMARK:
			//	return u8"SQL_C_BOOKMARK";
			//case SQL_C_VARBOOKMARK:
			//	return u8"SQL_C_VARBOOKMARK";
		case SQL_C_TYPE_DATE:
			return u8"SQL_DATE_STRUCT";
		case SQL_C_TYPE_TIME:
			return u8"SQL_TIME_STRUCT";
		case SQL_C_TYPE_TIMESTAMP:
			return u8"SQL_TIMESTAMP_STRUCT";
		case SQL_C_NUMERIC:
			return u8"SQL_NUMERIC_STRUCT";
		case SQL_C_GUID:
			return u8"SQL_GUID";

			// Old odbc 2.x values
		case SQL_C_DATE:
			return u8"SQL_DATE_STRUCT";
		case SQL_C_TIME:
			return u8"SQL_TIME_STRUCT";
		case SQL_C_TIMESTAMP:
			return u8"SQL_TIMESTAMP_STRUCT";

		default:
			return u8"???";
		}
	}


	std::string DatabaseProcudt2s(DatabaseProduct dbms) noexcept
	{
		switch (dbms)
		{
		case DatabaseProduct::DB2:
			return u8"DB2";
		case DatabaseProduct::MS_SQL_SERVER:
			return u8"SqlServer";
		case DatabaseProduct::MY_SQL:
			return u8"MySql";
		case DatabaseProduct::EXCEL:
			return u8"Excel";
		case DatabaseProduct::ACCESS:
			return u8"Access";
		default:
			return u8"UnknownDbms";
		}
	}


	std::string HandleType2s(SQLSMALLINT type) noexcept
	{
		switch (type)
		{
		case SQL_HANDLE_ENV:
			return u8"ENV";
		case SQL_HANDLE_DBC:
			return u8"DBC";
		case SQL_HANDLE_STMT:
			return u8"STMT";
		case SQL_HANDLE_DESC:
			return u8"DESC";
		default:
			return u8"???";
		}
	}


	ErrorHelper::SErrorInfoVector ErrorHelper::GetAllErrors(SQLHANDLE hEnv, SQLHANDLE hDbc, SQLHANDLE hStmt, SQLHANDLE hDesc)
	{
		exASSERT(hEnv != SQL_NULL_HENV || hDbc != SQL_NULL_HDBC || hStmt != SQL_NULL_HSTMT || hDesc != SQL_NULL_HDESC);

		SErrorInfoVector errors;
		SQLHANDLE handle = NULL;
		SQLSMALLINT handleType = 0;

		for (int i = 0; i < 4; i++)
		{
			if (i == 0 && hEnv)
			{
				handle = hEnv;
				handleType = SQL_HANDLE_ENV;
			}
			else if (i == 1 && hDbc)
			{
				handle = hDbc;
				handleType = SQL_HANDLE_DBC;
			}
			else if (i == 2 && hStmt)
			{
				handle = hStmt;
				handleType = SQL_HANDLE_STMT;
			}
			else if (i == 3 && hDesc)
			{
				handle = hDesc;
				handleType = SQL_HANDLE_DESC;
			}
			else
				continue;

			SQLSMALLINT recNr = 1;
			SQLRETURN ret = SQL_SUCCESS;

			while (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
			{
				SErrorInfo errInfo;
				SQLAPICHARTYPE errMsg[SQL_MAX_MESSAGE_LENGTH + 1];
                SQLAPICHARTYPE sqlState[5 + 1];
				errMsg[0] = '\0';
                sqlState[0] = '\0';
				SQLSMALLINT cb = 0;
				ret = SQLGetDiagRec(handleType, handle, recNr, sqlState, &errInfo.NativeError, errMsg, SQL_MAX_MESSAGE_LENGTH + 1, &cb);
				if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
				{
                    std::string ssqlState = SQLAPICHARPTR_TO_EXODBCSTR(sqlState);
                    memcpy(errInfo.SqlState, ssqlState.c_str(), std::min<size_t>(ssqlState.length(), 5));
                    errInfo.SqlState[std::min<size_t>(ssqlState.length(), 5)] = '\0';
					errInfo.ErrorHandleType = handleType;
					errInfo.Msg = SQLAPICHARPTR_TO_EXODBCSTR(errMsg);
					errors.push_back(errInfo);
					if (ret == SQL_SUCCESS_WITH_INFO)
					{
						std::stringstream ws;
						ws << u8"Error msg from recNr " << recNr << u8" got truncated";
						LOG_WARNING(ws.str());
					}
				}
				++recNr;
			}

			if (ret != SQL_NO_DATA)
			{
				LOG_WARNING(boost::str(boost::format(u8"Calling SQLGetDiagRec did not end with %s (%d) but with %s (%d)") % SqlReturn2s(SQL_NO_DATA) % SQL_NO_DATA %SqlReturn2s(ret) % ret));
			}
		}

		return errors;
	}


	ErrorHelper::SErrorInfoVector ErrorHelper::GetAllErrors(SQLSMALLINT handleType, SQLHANDLE handle)
	{
		switch (handleType)
		{
		case SQL_HANDLE_ENV:
			return GetAllErrors(handle, SQL_NULL_HDBC, SQL_NULL_HSTMT, SQL_NULL_HDESC);
		case SQL_HANDLE_DBC:
			return GetAllErrors(SQL_NULL_HENV, handle, SQL_NULL_HSTMT, SQL_NULL_HDESC);
		case SQL_HANDLE_STMT:
			return GetAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, handle, SQL_NULL_HDESC);
		case SQL_HANDLE_DESC:
			return GetAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, SQL_NULL_HSTMT, handle);
		default:
			exASSERT_MSG(false, u8"Unknown handleType");
		}
		return SErrorInfoVector();
	}


	std::string ErrorHelper::FormatOdbcMessages(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt, SQLHDESC hDesc, SQLRETURN ret, std::string sqlFunctionName, std::string msg)
	{
		std::string msgStr(msg);
		SErrorInfoVector errs = ErrorHelper::GetAllErrors(hEnv, hDbc, hStmt, hDesc);
		SErrorInfoVector::const_iterator it;
		std::stringstream handles;
		if (hEnv)
			handles << u8"Env=" << hEnv << u8";";
		if (hDbc)
			handles << u8"Dbc=" << hDbc << u8";";
		if (hStmt)
			handles << u8"Stmt=" << hStmt << u8";";
		if (hDesc)
			handles << u8"Desc=" << hDesc << u8";";

		std::string type = u8"Error(s)";
		if (ret == SQL_SUCCESS_WITH_INFO)
		{
			type = u8"Info(s)";
		}
		std::stringstream ss;
		if (msgStr.length() > 0)
			ss << msgStr << u8": ";
		ss << u8"ODBC-Function '" << sqlFunctionName << u8"' returned " << exodbc::SqlReturn2s(ret) << u8" (" << ret << u8"), with " << errs.size() << u8" ODBC-" << type << u8" from handle(s) '" << handles.str() << u8"': ";
		for (it = errs.begin(); it != errs.end(); it++)
		{
			const SErrorInfo& err = *it;
			ss << std::endl << u8"\t" << err.ToString();
		}
		return ss.str();
	}


	std::string ErrorHelper::SErrorInfo::ToString() const
	{
		std::stringstream ss;
		ss << u8"SQLSTATE " << SqlState << u8"; Native Error: " << NativeError << u8"; " << Msg.c_str();
		return ss.str();
	}
}
