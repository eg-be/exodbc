/*!
* \file Helpers.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 23.07.2014
* \brief Source file for the Helpers
*/ 

#include "stdafx.h"

// Own header
#include "Helpers.h"

// Same component headers
// Other headers

// Static consts
// -------------
namespace exodbc
{

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------
	void exOnAssert(const std::wstring& file, int line, const std::wstring& function, const std::wstring& condition, const std::wstring& msg)
	{
		std::wstringstream ws;
		ws 	<< L"ASSERTION failure!" << std::endl;
		ws	<< L" File:      " << file << std::endl;
		ws	<< L" Line:      " << line << std::endl;
		ws	<< L" Function:  " << function << std::endl;
		ws	<< L" Condition: " << condition << std::endl;
		if(msg.length() > 0)
		{
			ws << L" Msg:       " << msg << std::endl;
		}
		BOOST_LOG_TRIVIAL(error) << ws.str();
	}


	std::wostream& operator<< (std::wostream &out, const SErrorInfo& ei)
	{
		out << L"SQLSTATE " << ei.SqlState << L"; Native Error: " << ei.NativeError << L"; " << ei.Msg.c_str();
		return out;
	}

	std::ostream& operator<< (std::ostream &out, const SErrorInfo& ei)
	{

		out << "SQLSTATE " << w2s(ei.SqlState) << "; Native Error: " << ei.NativeError << "; " << w2s(ei.Msg);
		return out;
	}

	// Ticket #44
	std::string w2s(const std::wstring& w)
	{
		std::stringstream ss;

		for(size_t i = 0; i < w.length(); i++)
		{
			char c = (char) w[i];
			ss << c;
		}

		return ss.str();
	}

	std::wstring SqlTrueFalse2s(SQLSMALLINT b)
	{
		switch(b)
		{
		case SQL_TRUE:
			return L"TRUE";
		case SQL_FALSE:
			return L"FALSE";
		default:
			return L"?????";
		}
	}


	std::wstring SqlType2s(SQLSMALLINT sqlType)
	{
		switch(sqlType)
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


	std::wstring SqLCType2s(SQLSMALLINT sqlCType)
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


	std::wstring SqlCType2OdbcS(SQLSMALLINT sqlCType)
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

	std::vector<SErrorInfo> GetAllErrors(SQLHANDLE hEnv /* = SQL_NULL_HENV */, SQLHANDLE hDbc /* = SQL_NULL_HDBC */, SQLHANDLE hStmt /* = SQL_NULL_HSTMT */)
	{
		exASSERT(hEnv != SQL_NULL_HENV || hDbc != SQL_NULL_HDBC || hStmt != SQL_NULL_HSTMT);

		std::vector<SErrorInfo> errors;
		SQLHANDLE handle = NULL;
		SQLSMALLINT handleType = NULL;

		for(int i = 0; i < 3; i++)
		{
			if(i == 0 && hEnv)
			{
				handle = hEnv;
				handleType = SQL_HANDLE_ENV;
			}
			else if(i == 1 && hDbc)
			{
				handle = hDbc;
				handleType = SQL_HANDLE_DBC;
			}
			else if(i == 2 && hStmt)
			{
				handle = hStmt;
				handleType = SQL_HANDLE_STMT;
			}
			else
				continue;

			SQLSMALLINT recNr = 1;
			SQLRETURN ret = SQL_SUCCESS;

			while(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
			{
				SErrorInfo errInfo;
				SQLWCHAR errMsg[SQL_MAX_MESSAGE_LENGTH + 1];
				errMsg[0] = 0;
				SQLSMALLINT cb = 0;
				ret = SQLGetDiagRec(handleType, handle, recNr, errInfo.SqlState, &errInfo.NativeError, errMsg, SQL_MAX_MESSAGE_LENGTH + 1, &cb);
				if(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
				{
					errInfo.ErrorHandleType = handleType;
					errInfo.Msg = errMsg;
					errors.push_back(errInfo);
					if(ret == SQL_SUCCESS_WITH_INFO)
						BOOST_LOG_TRIVIAL(warning) << L"Error msg from recNr " << recNr << L" got truncated";
				}
				++recNr;
			}

			if(ret != SQL_NO_DATA)
			{
				BOOST_LOG_TRIVIAL(warning) << L"SQLGetDiagRec did not end with SQL_NO_DATA (100) but with " << ret;
			}
		}

		return errors;
	}


	SErrorInfo GetLastEnvError(SQLHANDLE hEnv, SQLSMALLINT& totalErrors)
	{
		std::vector<SErrorInfo> errs;
		if(hEnv)
			errs = GetAllErrors(hEnv);
		else
			BOOST_LOG_TRIVIAL(warning) << L"Cannot fetch errors, hEnv is NULL";

		totalErrors = errs.size();
		if(totalErrors > 0)
			return errs[0];

		return SErrorInfo();
	}


	SErrorInfo GetLastDbcError(SQLHANDLE hDbc, SQLSMALLINT& totalErrors)
	{
		std::vector<SErrorInfo> errs;
		if(hDbc)
			errs = GetAllErrors(NULL, hDbc);
		else
			BOOST_LOG_TRIVIAL(warning) << L"Cannot fetch errors, hDbc is NULL";

		totalErrors = errs.size();
		if(totalErrors > 0)
			return errs[0];

		return SErrorInfo();
	}


	SErrorInfo GetLastStmtError(SQLHANDLE hStmt, SQLSMALLINT& totalErrors)
	{
		std::vector<SErrorInfo> errs;
		if(hStmt)
			errs = GetAllErrors(NULL, NULL, hStmt);
		else
			BOOST_LOG_TRIVIAL(warning) << L"Cannot fetch errors, hStmt is NULL";

		totalErrors = errs.size();
		if(totalErrors > 0)
			return errs[0];

		return SErrorInfo();
	}


	SErrorInfo GetLastEnvError(SQLHANDLE hEnv)
	{
		SQLSMALLINT tot = 0;
		return GetLastEnvError(hEnv, tot);
	}

	SErrorInfo GetLastDbcError(SQLHANDLE hDbc)
	{
		SQLSMALLINT tot = 0;
		return GetLastDbcError(hDbc, tot);
	}


	SErrorInfo GetLastStmtError(SQLHANDLE hStmt)
	{
		SQLSMALLINT tot = 0;
		return GetLastStmtError(hStmt, tot);
	}


	bool FreeDbcHandle(SQLHANDLE& hDbc)
	{
		exASSERT(hDbc);

		SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
		if(ret != SQL_SUCCESS)
		{
			// if SQL_ERROR is returned, the handle is still valid, error information can be fetched
			if(ret == SQL_ERROR)
				BOOST_LOG_TRIVIAL(warning) << L"Failed to SQLFreeHandle of type SQL_HANDLE_DBC (return code was SQL_ERROR, handle is still valid): " << GetLastDbcError(hDbc);
			else
				BOOST_LOG_TRIVIAL(warning) << L"Failed to SQLFreeHandle of type SQL_HANDLE_DBC (return code was " << ret << L", handle is invalid)";
		}
		if(ret != SQL_ERROR)
		{
			hDbc = SQL_NULL_HDBC;
		}

		return ret == SQL_SUCCESS;
	}


	bool CloseStmtHandle(const SQLHANDLE& hStmt, CloseMode mode)
	{
		exASSERT(hStmt);

		SQLRETURN ret;
		if (mode == IgnoreNotOpen)
		{
			//  calling SQLFreeStmt with the SQL_CLOSE option has no effect on the application if no cursor is open on the statement
			ret = SQLFreeStmt(hStmt, SQL_CLOSE);
			if (ret != SQL_SUCCESS)
			{
				LOG_ERROR_STMT(hStmt, ret, SQLFreeStmt);
			}
		}
		else
		{
			// SQLCloseCursor returns SQLSTATE 24000 (Invalid cursor state) if no cursor is open. 
			ret = SQLCloseCursor(hStmt);
			if (ret != SQL_SUCCESS)
			{
				LOG_ERROR_STMT(hStmt, ret, SQLCloseCursor);
			}
		}

		return ret == SQL_SUCCESS;
	}


	bool EnsureStmtIsClosed(const SQLHANDLE& hStmt, DatabaseProduct dbms)
	{
		exASSERT(hStmt);
		SQLRETURN ret;
		bool wasOpen = false;
		// SQLCloseCursor returns SQLSTATE 24000 (Invalid cursor state) if no cursor is open. 
		// TODO: Using the MySQL ODBC Driver this function never fails ?
		ret = SQLCloseCursor(hStmt);
		if (ret == SQL_SUCCESS && dbms != dbmsMY_SQL)
		{
			// The Cursor was open as closing worked fine
			LOG_WARNING(L"Closing the Cursor worked fine, but we expected that it was already closed");
			wasOpen = true;
		}
		else if (dbms != dbmsMY_SQL)
		{
			// We would expect SQLSTATE 24000
			const std::vector<SErrorInfo>& errs = GetAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, hStmt);
			std::vector<SErrorInfo>::const_iterator it;
			bool haveOtherErrs = false;
			std::wstringstream ws;
			ws << L"SQLCloseCursor returned a different error than the expected SQLSTATE 24000:\n";
			for (it = errs.begin(); it != errs.end(); it++)
			{
				if ( ! CompareSqlState(it->SqlState, SqlState::INVALID_CURSOR_STATE))
				{
					// but we have something different
					haveOtherErrs = true;
					ws << *it << L"\n";
				}
			}
			if (haveOtherErrs)
			{
				LOG_ERROR(ws.str());
			}
		}
		return !wasOpen;
	}


	bool CompareSqlState(const wchar_t* sqlState1, const wchar_t* sqlState2)
	{
		exASSERT(sqlState1);
		exASSERT(sqlState2);
		return (wcsncmp(sqlState1, sqlState2, 5) == 0);
	}


	SQLSMALLINT GetResultColumnsCount(SQLHANDLE hStmt)
	{
		exASSERT(hStmt);

		SQLSMALLINT count = 0;
		SQLRETURN ret = SQLNumResultCols(hStmt, &count);
		if (ret != SQL_SUCCESS)
		{
			LOG_ERROR_STMT(hStmt, ret, SQLNumResultCols);
			count = -1;
		}
		return count;
	}


	bool GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, std::wstring& sValue)
	{
		// Determine buffer length
		exASSERT(hDbc != NULL);
		SQLSMALLINT bufferSize = 0;
		SQLRETURN ret = SQLGetInfo(hDbc, fInfoType, NULL, NULL, &bufferSize);
		if ( ! SQL_SUCCEEDED(ret))
		{
			LOG_ERROR_DBC_MSG(hDbc, ret, SQLGetInfo, (boost::wformat(L"GetInfo for fInfoType %d failed") % fInfoType).str());
			return false;
		}
		// According to the doc SQLGetInfo will always return byte-size. Therefore:
		exASSERT((bufferSize % sizeof(wchar_t)) == 0);
		// Allocate buffer, add one for terminating 0 char.
		SQLSMALLINT charSize = (bufferSize / sizeof(wchar_t)) + 1;
		bufferSize = charSize * sizeof(wchar_t);
		wchar_t* buff = new wchar_t[charSize];
		buff[0] = 0;
		SQLSMALLINT cb;
		bool ok = GetInfo(hDbc, fInfoType, (SQLPOINTER)buff, bufferSize, &cb);
		if (ok)
		{
			sValue = buff;
		}
		delete[] buff;
		return ok;
	}


	bool GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER rgbInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
	{
		exASSERT(hDbc != NULL);
		SQLRETURN ret = SQLGetInfo(hDbc, fInfoType, rgbInfoValue, cbInfoValueMax, pcbInfoValue);
		if( ret != SQL_SUCCESS )
		{
			LOG_ERROR_DBC_MSG(hDbc, ret, SQLGetInfo, (boost::wformat(L"GetInfo for fInfoType %d failed") %fInfoType).str());
		}

		return ret == SQL_SUCCESS;
	}


	bool GetData(SQLHSTMT hStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER pTargetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr, bool* pIsNull, bool nullTerminate /* = false */)
	{
		exASSERT(hStmt != SQL_NULL_HSTMT);
		if(nullTerminate)
		{
			exASSERT(targetType == SQL_C_CHAR || targetType == SQL_C_WCHAR);
		}

		bool isNull;
		SQLRETURN ret = SQLGetData(hStmt, colOrParamNr, targetType, pTargetValue, bufferLen, strLenOrIndPtr);
		if( ! (ret == SQL_SUCCESS ))
		{
			LOG_ERROR_STMT_MSG(hStmt, ret, SQLGetData, (boost::wformat(L"SGLGetData failed for Column %d") %colOrParamNr).str());
			return false;
		}

		// ret is SQL_SUCCESS
		{
			isNull = (*strLenOrIndPtr == SQL_NULL_DATA);
			if(pIsNull)
				*pIsNull = isNull;

			// a string that is null does not get terminated. just dont use it.
			if(nullTerminate && !isNull)
			{
				exASSERT(*strLenOrIndPtr != SQL_NO_TOTAL);
				if(targetType == SQL_C_CHAR)
				{
					char* pc = (char*) pTargetValue;
					exASSERT(bufferLen >= *strLenOrIndPtr);
					pc[*strLenOrIndPtr] = 0;
				}
				else
				{
					wchar_t* pw = (wchar_t*) pTargetValue;
					int p = *strLenOrIndPtr / sizeof(wchar_t);
					exASSERT(bufferLen >= p);
					pw[ p ] = 0;
				}
			}
		}

		return true;
	}

	bool GetData(SQLHSTMT hStmt, SQLUSMALLINT colOrParamNr, size_t maxNrOfChars, std::wstring& value, bool* pIsNull /* = NULL */)
	{
		value = L"";
		wchar_t* buffer = new wchar_t[maxNrOfChars + 1];
		size_t buffSize = sizeof(wchar_t) * (maxNrOfChars + 1);
		SQLLEN cb;
		bool isNull = false;
		bool ok = GetData(hStmt, colOrParamNr, SQL_C_WCHAR, buffer, buffSize, &cb, &isNull, true);
		if(ok && !isNull)
		{
			value = buffer;
		}
		if(pIsNull)
			*pIsNull = isNull;

		delete[] buffer;
		return ok;
	}
}

// Interfaces
// ----------

