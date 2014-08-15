/*!
* \file Helpers.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 23.07.2014
* 
* [Brief CPP-file description]
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
	void exOnAssert(const char* file, int line, const char* function, const char* condition, const char* msg)
	{
		std::wstringstream ws;
		ws 	<< L"ASSERTION failure!" << std::endl;
		ws	<< L" File:      " << file << std::endl;
		ws	<< L" Line:      " << line << std::endl;
		ws	<< L" Function:  " << function << std::endl;
		ws	<< L" Condition: " << condition << std::endl;
		if(msg)
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

	std::vector<SErrorInfo> GetAllErrors(SQLHANDLE hEnv /* = NULL */, SQLHANDLE hDbc /* = NULL */, SQLHANDLE hStmt /* = NULL */)
	{
		exASSERT(hEnv != NULL || hDbc != NULL || hStmt != NULL);


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


	SQLHANDLE AllocDbcHandle(const SQLHANDLE& hEnv)
	{
		exASSERT(hEnv);

		SQLHANDLE handle = SQL_NULL_HENV;

		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &handle);
		if(ret != SQL_SUCCESS)
		{
			BOOST_LOG_TRIVIAL(error) << L"Failed to SQLAllocHandle of type SQL_HANDLE_DBC, (return code was " << ret << L"): " << GetLastEnvError(hEnv);
			// Note: SQLAllocHandle will set the output-handle to SQL_NULL_HDBC, SQL_NULL_HSTMT, or SQL_NULL_HDESCin case of failure
			return handle;
		}

		return handle;
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

	SQLHANDLE AllocStmtHandle(const SQLHANDLE& hDbc)
	{
		exASSERT(hDbc);

		SQLHANDLE handle = SQL_NULL_HSTMT;

		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &handle);
		if(ret != SQL_SUCCESS)
		{
			BOOST_LOG_TRIVIAL(error) << L"Failed to SQLAllocHandle of type SQL_HANDLE_STMT, (return code was " << ret << L"): " << GetLastEnvError(hDbc);
			// Note: SQLAllocHandle will set the output-handle to SQL_NULL_HDBC, SQL_NULL_HSTMT, or SQL_NULL_HDESCin case of failure
			return handle;
		}

		return handle;
	}

	bool FreeStmtHandle(SQLHANDLE& hStmt)
	{
		exASSERT(hStmt);

		SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		if(ret != SQL_SUCCESS)
		{
			// if SQL_ERROR is returned, the handle is still valid, error information can be fetched
			if(ret == SQL_ERROR)
				BOOST_LOG_TRIVIAL(warning) << L"Failed to SQLFreeHandle of type SQL_HDNCLE_STMT (return code was SQL_ERROR, handle is still valid): " << GetLastDbcError(hStmt);
			else
				BOOST_LOG_TRIVIAL(warning) << L"Failed to SQLFreeHandle of type SQL_HDNCLE_STMT (return code was " << ret << L", handle is invalid)";
		}
		if(ret != SQL_ERROR)
		{
			hStmt = SQL_NULL_HSTMT;
		}

		return ret == SQL_SUCCESS;
	}


	bool CloseStmtHandle(const SQLHANDLE& hStmt)
	{
		exASSERT(hStmt);

		SQLRETURN ret = SQLFreeStmt(hStmt, SQL_CLOSE);
		if(ret != SQL_SUCCESS)
		{
			if(ret == SQL_SUCCESS_WITH_INFO || ret == SQL_ERROR)
			{
				BOOST_LOG_TRIVIAL(warning) << L"Failed to free Stmt, SQLFreeStmt with SQL_CLOSE returned with " << ret << L": " << GetLastStmtError(hStmt);
			}
			else
			{
				BOOST_LOG_TRIVIAL(warning) << L"Failed to free Stmt, SQLFreeStmt with SQL_CLOSE returned with " << ret;
			}
		}

		return ret == SQL_SUCCESS;
	}


	bool GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER rgbInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
	{
		exASSERT(hDbc != NULL);
		SQLRETURN ret = SQLGetInfo(hDbc, fInfoType, rgbInfoValue, cbInfoValueMax, pcbInfoValue);
		if( ! (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO))
		{
			BOOST_LOG_TRIVIAL(warning) << L"Failed with return-value " << ret << L" to GetInfo for type " << fInfoType << L": " << GetLastDbcError(hDbc);
		}

		if(ret == SQL_SUCCESS_WITH_INFO)
		{
			BOOST_LOG_TRIVIAL(warning) << L"GetInfo for type " << fInfoType << L" returned with SQL_SUCCESS_WITH_INFO";
		}

		return (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
	}


	bool GetData3(SQLHSTMT hStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER targetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr, bool* pIsNull, bool nullTerminate /* = false */)
	{
		exASSERT(hStmt != SQL_NULL_HSTMT);
		if(nullTerminate)
		{
			exASSERT(targetType == SQL_C_CHAR || targetType == SQL_C_WCHAR);
		}

		bool isNull;
		SQLRETURN ret = SQLGetData(hStmt, colOrParamNr, targetType, targetValue, bufferLen, strLenOrIndPtr);
		if( ! (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO))
		{
			if(ret == SQL_ERROR)
			{
				BOOST_LOG_TRIVIAL(error) << L"GetData failed with SQL_ERROR: " << GetLastStmtError(hStmt);
			}
			else if(!ret == SQL_SUCCESS_WITH_INFO) // we report that later
			{
				BOOST_LOG_TRIVIAL(error) << L"GetData failed with return-value " << ret;
			}
		}
		else // ret is SQL_SUCCESS or SQL_SUCCESS_WITH_INFO
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
					char* pc = (char*) targetValue;
					pc[*strLenOrIndPtr] = 0;
				}
				else
				{
					wchar_t* pw = (wchar_t*) targetValue;
					int p = *strLenOrIndPtr / sizeof(wchar_t);
					pw[ *strLenOrIndPtr / sizeof(wchar_t)] = 0;
				}
			}
		}
		if(ret == SQL_SUCCESS_WITH_INFO)
		{
			BOOST_LOG_TRIVIAL(error) << L"GetData completed with SQL_SUCCESS_WITH_INFO: " << GetLastStmtError(hStmt);
		}

		return ret == SQL_SUCCESS;
	}

}

// Interfaces
// ----------

