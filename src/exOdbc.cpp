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

// Other headers
// Debug
#include "DebugNew.h"

namespace exodbc {

	// Static consts
	// -------------

	// Implementation
	// --------------

	std::wstring FormatOdbcMessages(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt, SQLHDESC hDesc, SQLRETURN ret, std::wstring sqlFunctionName, std::wstring msg)
	{
		std::wstring msgStr(msg);
		SErrorInfoVector errs = exodbc::GetAllErrors(hEnv, hDbc, hStmt, hDesc);
		SErrorInfoVector::const_iterator it;
		std::wstringstream handles;
		if (hEnv)
			handles << L"Env=" << hEnv << L";";
		if (hDbc)
			handles << L"Dbc=" << hDbc << L";";
		if (hStmt)
			handles << L"Stmt=" << hStmt << L";";
		if (hDesc)
			handles << L"Desc=" << hDesc << L";";

		std::wstring type = L"Error(s)";
		if (ret == SQL_SUCCESS_WITH_INFO)
		{
			type = L"Info(s)";
		}
		std::wstringstream ws;
		if (msgStr.length() > 0)
			ws << msgStr << L": ";
		ws << L"ODBC-Function '" << sqlFunctionName << L"' returned " << exodbc::SqlReturn2s(ret) << L" (" << ret << L"), with " << errs.size() << L" ODBC-" << type << L" from handle(s) '" << handles.str() << L"': ";
		for (it = errs.begin(); it != errs.end(); it++)
		{
			const SErrorInfo& err = *it;
			ws << std::endl << L"\t" << err.ToString();
		}
		return ws.str();
	}
	
	std::wstring FormatOdbcMessages(SQLHENV hEnv, SQLHDBC hDbc, SQLHSTMT hStmt, SQLHDESC hDesc, SQLRETURN ret, std::string sqlFunctionName, std::wstring msg)
    {
        return FormatOdbcMessages(hEnv, hDbc, hStmt, hDesc, ret, utf8ToUtf16(sqlFunctionName), msg);
    }


	std::wstring SErrorInfo::ToString() const
	{
		std::wstringstream ws;
		ws << L"SQLSTATE " << SqlState << L"; Native Error: " << NativeError << L"; " << Msg.c_str();
		return ws.str();
	}


	std::string utf16ToUtf8(const std::wstring& w) noexcept
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
			std::string s("Failed in utf16ToUtf8 with std::exception: ");
			s += ex.what();
			return s;
		}
		catch (...)
		{
			std::string s("Failed in utf16ToUtf8 with unknown exception.");
			return s;
		}
	}


	std::wstring utf8ToUtf16(const std::string& s) noexcept
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
			HIDE_UNUSED(ex);
			std::wstring ws(L"Failed in utf8ToUtf16 with std::exception - unable to print what() as wstring, sorry.");
			return ws;
		}
		catch (...)
		{
			std::wstring ws(L"Failed in utf8ToUtf16 with unknown exception.");
			return ws;
		}
	}



	std::wstring SqlTrueFalse2s(SQLSMALLINT b) noexcept
	{
		switch (b)
		{
		case SQL_TRUE:
			return L"TRUE";
		case SQL_FALSE:
			return L"FALSE";
		default:
			return L"?????";
		}
	}


	std::wstring SqlReturn2s(SQLRETURN ret) noexcept
	{
		switch (ret)
		{
		case SQL_SUCCESS:
			return L"SUCCESS";
		case SQL_SUCCESS_WITH_INFO:
			return L"SUCCESS_WITH_INFO";
		case SQL_NO_DATA:
			return L"SQL_NO_DATA";
		case SQL_ERROR:
			return L"SQL_ERROR";
		case SQL_NEED_DATA:
			return L"SQL_NEED_DATA";
		case SQL_INVALID_HANDLE:
			return L"SQL_INVALID_HANDLE";
		default:
			return L"???";
		}
	}


	std::wstring SqlType2s(SQLSMALLINT sqlType) noexcept
	{
		switch (sqlType)
		{
		case SQL_CHAR:
			return L"CHAR";
		case SQL_VARCHAR:
			return L"VARCHAR";
		case SQL_LONGVARCHAR:
			return L"LONGVARCHAR";
		case SQL_WCHAR:
			return L"WCHAR";
		case SQL_WVARCHAR:
			return L"WVARCHAR";
		case SQL_WLONGVARCHAR:
			return L"WLONGVARCHAR";
		case SQL_DECIMAL:
			return L"DECIMAL";
		case SQL_NUMERIC:
			return L"NUMERIC";
		case SQL_SMALLINT:
			return L"SMALLINT";
		case SQL_INTEGER:
			return L"INTEGER";
		case SQL_REAL:
			return L"REAL";
		case SQL_FLOAT:
			return L"FLOAT";
		case SQL_DOUBLE:
			return L"DOUBLE";
		case SQL_BIT:
			return L"BIT";
		case SQL_TINYINT:
			return L"TINYINT";
		case SQL_BIGINT:
			return L"BIGINT";
		case SQL_BINARY:
			return L"BINARY";
		case SQL_VARBINARY:
			return L"VARBINARY";
		case SQL_LONGVARBINARY:
			return L"LONGVARBINARY";
		case SQL_TYPE_DATE:	//[6]
			return L"TYPE_DATE";
		case SQL_TYPE_TIME: //[6]
			return L"TYPE_TIME";
		case SQL_TYPE_TIMESTAMP: //[6]
			return L"TYPE_TIMESTAMP";
		case SQL_DATE:
			return L"DATE";
		case SQL_TIME:
			return L"TIME";
		case SQL_TIMESTAMP:
			return L"TIMESTAMP";
			//		case SQL_TYPE_UTCDATETIME:
			//			return L"TYPE_UTCDATETIME";
			//		case SQL_TYPE_UTCTIME:
			//			return L"TYPE_UTCTIME";
		case SQL_INTERVAL_MONTH: //[7]
			return L"INTERVAL_MONTH";
		case SQL_INTERVAL_YEAR: //[7]
			return L"INTERVAL_YEAR";
		case SQL_INTERVAL_YEAR_TO_MONTH: //[7]
			return L"INTERVAL_YEAR_TO_MONTH";
		case SQL_INTERVAL_DAY: //[7]
			return L"INTERVAL_DAY";
		case SQL_INTERVAL_HOUR: ///[7]
			return L"INTERVAL_HOUR";
		case SQL_INTERVAL_MINUTE: //[7]
			return L"INTERVAL_MINUTE";
		case SQL_INTERVAL_SECOND: //[7]
			return L"INTERVAL_SECOND";
		case SQL_INTERVAL_DAY_TO_HOUR: //[7]
			return L"INTERVAL_DAY_TO_HOUR";
		case SQL_INTERVAL_DAY_TO_MINUTE: //[7]
			return L"INTERVAL_DAY_TO_MINUTE";
		case SQL_INTERVAL_DAY_TO_SECOND: //[7]
			return L"INTERVAL_DAY_TO_SECOND";
		case SQL_INTERVAL_HOUR_TO_MINUTE: //[7]
			return L"INTERVAL_HOUR_TO_MINUTE";
		case SQL_INTERVAL_HOUR_TO_SECOND: //[7]
			return L"INTERVAL_HOUR_TO_SECOND";
		case SQL_INTERVAL_MINUTE_TO_SECOND: //[7]
			return L"INTERVAL_MINUTE_TO_SECOND";
		case SQL_GUID:
			return L"GUID";
		default:
			return L"???";
		}
	}


	std::wstring SqLCType2s(SQLSMALLINT sqlCType) noexcept
	{
		switch (sqlCType)
		{
		case SQL_C_STINYINT:
			return L"SQL_C_STINYINT";
		case SQL_C_UTINYINT:
			return L"SQL_C_UTINYINT";
		case SQL_C_SSHORT:
			return L"SQL_C_SSHORT";
		case SQL_C_USHORT:
			return L"SQL_C_USHORT";
		case SQL_C_SLONG:
			return L"SQL_C_SLONG";
		case SQL_C_ULONG:
			// Note: This is also the type for SQL_C_BOOKMARK
			return L"SQL_C_ULONG";
		case SQL_C_SBIGINT:
			return L"SQL_C_SBIGINT";
		case SQL_C_UBIGINT:
			return L"SQL_C_UBIGINT";
		case SQL_C_CHAR:
			return L"SQL_C_CHAR";
		case SQL_C_WCHAR:
			return L"SQL_C_WCHAR";
		case SQL_C_FLOAT:
			return L"SQL_C_FLOAT";
		case SQL_C_DOUBLE:
			return L"SQL_C_DOUBLE";
		case SQL_C_BIT:
			return L"SQL_C_BIT";
		case SQL_C_BINARY:
			// Note: This is also the type for SQL_C_VARBOOKMARK:
			return L"SQL_C_BINARY";
			//case SQL_C_BOOKMARK:
			//	return L"SQL_C_BOOKMARK";
			//case SQL_C_VARBOOKMARK:
			//	return L"SQL_C_VARBOOKMARK";
		case SQL_C_TYPE_DATE:
			return L"SQL_C_TYPE_DATE";
		case SQL_C_TYPE_TIME:
			return L"SQL_C_TYPE_TIME";
		case SQL_C_TYPE_TIMESTAMP:
			return L"SQL_C_TYPE_TIMESTAMP";
		case SQL_C_NUMERIC:
			return L"SQL_C_NUMERIC";
		case SQL_C_GUID:
			return L"SQL_C_GUID";

			// Old odbc 2.x values
		case SQL_C_DATE:
			return L"SQL_C_DATE";
		case SQL_C_TIME:
			return L"SQL_C_TIME";
		case SQL_C_TIMESTAMP:
			return L"SQL_C_TIMESTAMP";

		default:
			return L"???";
		}
	}


	std::wstring SqlCType2OdbcS(SQLSMALLINT sqlCType) noexcept
	{
		switch (sqlCType)
		{
		case SQL_C_STINYINT:
			return L"SQLSCHAR";
		case SQL_C_UTINYINT:
			return L"SQLCHAR";
		case SQL_C_SSHORT:
			return L"SQLSMALLINT";
		case SQL_C_USHORT:
			return L"SQLUSMALLINT";
		case SQL_C_SLONG:
			return L"SQLINTEGER";
		case SQL_C_ULONG:
			// Note: This is also the type for SQL_C_BOOKMARK
			return L"SQLUINTEGER";
		case SQL_C_SBIGINT:
			return L"SQLBIGNT";
		case SQL_C_UBIGINT:
			return L"SQLUBIGINT";
		case SQL_C_CHAR:
			return L"SQLCHAR*";
		case SQL_C_WCHAR:
			return L"SQLWCHAR*";
		case SQL_C_FLOAT:
			return L"SQLREAL";
		case SQL_C_DOUBLE:
			return L"SQLDOUBLE";
		case SQL_C_BIT:
			return L"SQLCHAR";
		case SQL_C_BINARY:
			// Note: This is also the type for SQL_C_VARBOOKMARK:
			return L"SQLCHAR*";
			//case SQL_C_BOOKMARK:
			//	return L"SQL_C_BOOKMARK";
			//case SQL_C_VARBOOKMARK:
			//	return L"SQL_C_VARBOOKMARK";
		case SQL_C_TYPE_DATE:
			return L"SQL_DATE_STRUCT";
		case SQL_C_TYPE_TIME:
			return L"SQL_TIME_STRUCT";
		case SQL_C_TYPE_TIMESTAMP:
			return L"SQL_TIMESTAMP_STRUCT";
		case SQL_C_NUMERIC:
			return L"SQL_NUMERIC_STRUCT";
		case SQL_C_GUID:
			return L"SQL_GUID";

			// Old odbc 2.x values
		case SQL_C_DATE:
			return L"SQL_DATE_STRUCT";
		case SQL_C_TIME:
			return L"SQL_TIME_STRUCT";
		case SQL_C_TIMESTAMP:
			return L"SQL_TIMESTAMP_STRUCT";

		default:
			return L"???";
		}
	}


	std::wstring DatabaseProcudt2s(DatabaseProduct dbms) noexcept
	{
		switch (dbms)
		{
		case DatabaseProduct::DB2:
			return L"DB2";
		case DatabaseProduct::MS_SQL_SERVER:
			return L"SqlServer";
		case DatabaseProduct::MY_SQL:
			return L"MySql";
		case DatabaseProduct::EXCEL:
			return L"Excel";
		case DatabaseProduct::ACCESS:
			return L"Access";
		default:
			return L"UnknownDbms";
		}
	}


	std::wstring HandleType2s(SQLSMALLINT type) noexcept
	{
		switch (type)
		{
		case SQL_HANDLE_ENV:
			return L"ENV";
		case SQL_HANDLE_DBC:
			return L"DBC";
		case SQL_HANDLE_STMT:
			return L"STMT";
		case SQL_HANDLE_DESC:
			return L"DESC";
		default:
			return L"???";
		}
	}


	SErrorInfoVector GetAllErrors(SQLHANDLE hEnv, SQLHANDLE hDbc, SQLHANDLE hStmt, SQLHANDLE hDesc)
	{
		exASSERT(hEnv != SQL_NULL_HENV || hDbc != SQL_NULL_HDBC || hStmt != SQL_NULL_HSTMT || hDesc != SQL_NULL_HDESC);

		SErrorInfoVector errors;
		SQLHANDLE handle = NULL;
		SQLSMALLINT handleType = NULL;

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
				SQLWCHAR errMsg[SQL_MAX_MESSAGE_LENGTH + 1];
				errMsg[0] = 0;
				SQLSMALLINT cb = 0;
				ret = SQLGetDiagRec(handleType, handle, recNr, errInfo.SqlState, &errInfo.NativeError, errMsg, SQL_MAX_MESSAGE_LENGTH + 1, &cb);
				if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
				{
					errInfo.ErrorHandleType = handleType;
					errInfo.Msg = errMsg;
					errors.push_back(errInfo);
					if (ret == SQL_SUCCESS_WITH_INFO)
					{
						std::wstringstream ws;
						ws << L"Error msg from recNr " << recNr << L" got truncated";
						LOG_WARNING(ws.str());
					}
				}
				++recNr;
			}

			if (ret != SQL_NO_DATA)
			{
				LOG_WARNING(boost::str(boost::wformat(L"Calling SQLGetDiagRec did not end with %s (%d) but with %s (%d)") % SqlReturn2s(SQL_NO_DATA) % SQL_NO_DATA %SqlReturn2s(ret) % ret));
			}
		}

		return errors;
	}


	SErrorInfoVector GetAllErrors(SQLSMALLINT handleType, SQLHANDLE handle)
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
			exASSERT_MSG(false, L"Unknown handleType");
		}
		return SErrorInfoVector();
	}
}
