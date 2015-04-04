/*!
* \file Helpers.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 23.07.2014
* \brief Source file for the Helpers
* \copyright wxWindows Library Licence, Version 3.1
*/ 

#include "stdafx.h"

// Own header
#include "Helpers.h"

// Same component headers
#include "Exception.h"

// Other headers
#include "boost/thread/locks.hpp"

// Debug
#include "DebugNew.h"

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
	bool g_dontDebugBreak = false;
	boost::shared_mutex g_dontDebugBreakMutex;

	void SetDontDebugBreak(bool value)
	{
		boost::unique_lock<boost::shared_mutex> lock(g_dontDebugBreakMutex);
		g_dontDebugBreak = value;
	}


	bool GetDontDebugBreak()
	{
		boost::shared_lock<boost::shared_mutex> lock(g_dontDebugBreakMutex);
		return g_dontDebugBreak;
	}


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

		// Throw exception
		throw AssertionException(line, file, function, condition, msg);
	}


	std::wostream& operator<< (std::wostream &out, const SErrorInfo& ei)
	{
		out << L"SQLSTATE " << ei.SqlState << L"; Native Error: " << ei.NativeError << L"; " << ei.Msg.c_str();
		return out;
	}


	std::wstring SErrorInfo::ToString() const
	{
		std::wstringstream ws;
		ws << this;
		return ws.str();
	}


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


	std::wstring s2w(const std::string& s)
	{
		std::wstringstream ws;

		for (size_t i = 0; i < s.length(); i++)
		{
			char c = (char)s[i];
			ws << c;
		}

		return ws.str();
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


	std::wstring SqlReturn2s(SQLRETURN ret)
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


	std::wstring DatabaseProcudt2s(DatabaseProduct dbms)
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
		default:
			return L"UnknownDbms";
		}
	}

	//std::wstring OpenMode2s(Table::OpenMode openMode)
	//{
	//	switch (openMode)
	//	{
	//	case Table::READ_ONLY:
	//		return L"READ_ONLY";
	//	case Table::READ_WRITE:
	//		return L"READ_WRITE";
	//	default:
	//		return L"???";
	//	}
	//}


	SErrorInfoVector GetAllErrors(SQLHANDLE hEnv, SQLHANDLE hDbc, SQLHANDLE hStmt, SQLHANDLE hDesc)
	{
		exASSERT(hEnv != SQL_NULL_HENV || hDbc != SQL_NULL_HDBC || hStmt != SQL_NULL_HSTMT || hDesc != SQL_NULL_HDESC);

		SErrorInfoVector errors;
		SQLHANDLE handle = NULL;
		SQLSMALLINT handleType = NULL;

		for(int i = 0; i < 4; i++)
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
			else if (i == 3 && hDesc)
			{
				handle = hDesc;
				handleType = SQL_HANDLE_DESC;
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


	void CloseStmtHandle(SQLHANDLE hStmt, StmtCloseMode mode)
	{
		exASSERT(hStmt);

		SQLRETURN ret;
		if (mode == StmtCloseMode::IgnoreNotOpen)
		{
			//  calling SQLFreeStmt with the SQL_CLOSE option has no effect on the application if no cursor is open on the statement
			ret = SQLFreeStmt(hStmt, SQL_CLOSE);
			THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, hStmt);
		}
		else
		{
			// SQLCloseCursor returns SQLSTATE 24000 (Invalid cursor state) if no cursor is open. 
			ret = SQLCloseCursor(hStmt);
			THROW_IFN_SUCCEEDED(SQLCloseCursor, ret, SQL_HANDLE_STMT, hStmt);
		}
	}


	void GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, std::wstring& sValue)
	{
		// Determine buffer length
		exASSERT(hDbc != NULL);
		SQLSMALLINT bufferSize = 0;
		SQLRETURN ret = SQLGetInfo(hDbc, fInfoType, NULL, NULL, &bufferSize);
		{
			// \note: DB2 will here always return SQL_SUCCESS_WITH_INFO to report that data got truncated, although we didnt even feed in a buffer.
			// To avoid having tons of warning with the (wrong) info that data has been truncated, we just hide those messages here
			THROW_IFN_SUCCEEDED_MSG(SQLGetInfo, ret, SQL_HANDLE_DBC, hDbc, (boost::wformat(L"GetInfo for fInfoType %d failed") % fInfoType).str());
		}

		// According to the doc SQLGetInfo will always return byte-size. Therefore:
		exASSERT((bufferSize % sizeof(SQLWCHAR)) == 0);

		// Allocate buffer, add one for terminating 0 char.
		SQLSMALLINT charSize = (bufferSize / sizeof(SQLWCHAR)) + 1;
		bufferSize = charSize * sizeof(SQLWCHAR);
		std::unique_ptr<SQLWCHAR[]> buff(new SQLWCHAR[charSize]);
		buff[0] = 0;
		SQLSMALLINT cb;

		GetInfo(hDbc, fInfoType, (SQLPOINTER)buff.get(), bufferSize, &cb);

		sValue = buff.get();
	}


	void GetInfo(SQLHDBC hDbc, SQLUSMALLINT fInfoType, SQLPOINTER rgbInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue)
	{
		exASSERT(hDbc != NULL);
		SQLRETURN ret = SQLGetInfo(hDbc, fInfoType, rgbInfoValue, cbInfoValueMax, pcbInfoValue);
		THROW_IFN_SUCCEEDED_MSG(SQLGetInfo, ret, SQL_HANDLE_DBC, hDbc, (boost::wformat(L"GetInfo for fInfoType %d failed") % fInfoType).str());
	}


	void GetData(SQLHSTMT hStmt, SQLUSMALLINT colOrParamNr, SQLSMALLINT targetType, SQLPOINTER pTargetValue, SQLLEN bufferLen, SQLLEN* strLenOrIndPtr, bool* pIsNull)
	{
		exASSERT(hStmt != SQL_NULL_HSTMT);
		exASSERT(strLenOrIndPtr != NULL);

		bool isNull;
		SQLRETURN ret = SQLGetData(hStmt, colOrParamNr, targetType, pTargetValue, bufferLen, strLenOrIndPtr);
		THROW_IFN_SUCCEEDED_MSG(SQLGetData, ret, SQL_HANDLE_STMT, hStmt, (boost::wformat(L"SGLGetData failed for Column %d") % colOrParamNr).str());

		isNull = (*strLenOrIndPtr == SQL_NULL_DATA);
		if (pIsNull)
		{
			*pIsNull = isNull;
		}
	}


	void GetData(SQLHSTMT hStmt, SQLUSMALLINT colOrParamNr, size_t maxNrOfChars, std::wstring& value, bool* pIsNull /* = NULL */)
	{
		value = L"";
		std::unique_ptr<SQLWCHAR[]> buffer(new SQLWCHAR[maxNrOfChars + 1]);
		size_t buffSize = sizeof(SQLWCHAR) * (maxNrOfChars + 1);
		SQLLEN cb;
		bool isNull = false;

		GetData(hStmt, colOrParamNr, SQL_C_WCHAR, buffer.get(), buffSize, &cb, &isNull);

		if(!isNull)
		{
			value = buffer.get();
		}
		if(pIsNull)
			*pIsNull = isNull;
	}


	void SetDescriptionField(SQLHDESC hDesc, SQLSMALLINT recordNumber, SQLSMALLINT descriptionField, SQLPOINTER value)
	{
		exASSERT(hDesc != SQL_NULL_HDESC);
		exASSERT(recordNumber > 0);
		SQLRETURN ret = SQLSetDescField(hDesc, recordNumber, descriptionField, value, 0);
		THROW_IFN_SUCCEEDED(SQLSetDescField, ret, SQL_HANDLE_DESC, hDesc);
	}


	SQLHDESC GetRowDescriptorHandle(SQLHSTMT hStmt, RowDescriptorType type)
	{
		exASSERT(hStmt != SQL_NULL_HSTMT);

		SQLHDESC hDesc = SQL_NULL_HDESC;
		SQLRETURN ret = SQLGetStmtAttr(hStmt, (SQLINTEGER) type, &hDesc, 0, NULL);
		THROW_IFN_SUCCEEDED(SQLGetStmtAttr, ret, SQL_HANDLE_STMT, hStmt);

		return hDesc;
	}


	SQL_TIME_STRUCT InitTime(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second) throw()
	{
		SQL_TIME_STRUCT time;
		time.hour = hour;
		time.minute = minute;
		time.second = second;

		return time;
	}


#ifdef HAVE_MSODBCSQL_H
	SQL_SS_TIME2_STRUCT InitTime2(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second, SQLUINTEGER fraction) throw()
	{
		SQL_SS_TIME2_STRUCT time2;
		time2.hour = hour;
		time2.minute = minute;
		time2.second = second;
		time2.fraction = fraction;

		return time2;
	}
#endif


	SQL_DATE_STRUCT InitDate(SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year) throw()
	{
		SQL_DATE_STRUCT date;
		date.year = year;
		date.month = month;
		date.day = day;

		return date;
	}


	SQL_TIMESTAMP_STRUCT InitTimestamp(SQLUSMALLINT hour, SQLUSMALLINT minute, SQLUSMALLINT second, SQLUINTEGER fraction, SQLUSMALLINT day, SQLUSMALLINT month, SQLSMALLINT year) throw()
	{
		SQL_TIMESTAMP_STRUCT timestamp;
		timestamp.hour = hour;
		timestamp.minute = minute;
		timestamp.second = second;
		timestamp.fraction = fraction;
		timestamp.day = day;
		timestamp.month = month;
		timestamp.year = year;

		return timestamp;
	}


	bool IsTimeEqual(const SQL_TIME_STRUCT& t1, const SQL_TIME_STRUCT& t2) throw()
	{
		return t1.hour == t2.hour
			&& t1.minute == t2.minute
			&& t1.second == t2.second;
	}


	bool IsDateEqual(const SQL_DATE_STRUCT& d1, const SQL_DATE_STRUCT& d2) throw()
	{
		return d1.day == d2.day
			&& d1.month == d2.month
			&& d1.year == d2.year;
	}


	bool IsTimestampEqual(const SQL_TIMESTAMP_STRUCT& ts1, const SQL_TIMESTAMP_STRUCT& ts2) throw()
	{
		return ts1.hour == ts2.hour
			&& ts1.minute == ts2.minute
			&& ts1.second == ts2.second
			&& ts1.fraction == ts2.fraction
			&& ts1.day == ts2.day
			&& ts1.month == ts2.month
			&& ts1.year == ts2.year;
	}


	SQL_NUMERIC_STRUCT InitNumeric(SQLCHAR precision, SQLSCHAR scale, SQLCHAR sign, SQLCHAR val[SQL_MAX_NUMERIC_LEN]) throw()
	{
		SQL_NUMERIC_STRUCT num;
		num.precision = precision;
		num.scale = scale;
		num.sign = sign;
		memcpy(num.val, val, SQL_MAX_NUMERIC_LEN);
		
		return num;
	}


	SQL_NUMERIC_STRUCT InitNullNumeric() throw()
	{
		SQL_NUMERIC_STRUCT num;
		ZeroMemory(&num, sizeof(num));
		return num;
	}


	long Str2Hex2Long(unsigned char hexValue[16])
	{
		long val = 0;
		long value = 0;
		int i = 1;
		int last = 1;
		int current;
		int a = 0;
		int b = 0;

		for (i = 0; i <= 15; i++)
		{
			current = (int)hexValue[i];
			a = current % 16; //Obtain LSD
			b = current / 16; //Obtain MSD

			value += last* a;
			last = last * 16;
			value += last* b;
			last = last * 16;
		}
		return value;
	}


	SQLHSTMT AllocateStatementHandle(SQLHDBC hDbc)
	{
		exASSERT(hDbc != SQL_NULL_HDBC);

		SQLHSTMT stmt;
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &stmt);
		THROW_IFN_SUCCEEDED(SQLAllocHandle, ret, SQL_HANDLE_DBC, hDbc);
		return stmt;
	}


	SQLHSTMT FreeStatementHandle(SQLHSTMT hStmt, FreeStatementThrowFlags flags /* = FSTF_THROW_ON_SQL_ERROR | FSTF_THROW_ON_SQL_INVALID_HANDLE */)
	{
		// Returns only SQL_SUCCESS, SQL_ERROR, or SQL_INVALID_HANDLE.
		SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		if (ret == SQL_ERROR)
		{
			// if SQL_ERROR is returned, the handle is still valid, error information can be fetched
			if ( (flags & FSTF_THROW_ON_SQL_ERROR) == FSTF_THROW_ON_SQL_ERROR )
			{
				SqlResultException ex(L"SQLFreeHandle", ret, SQL_HANDLE_STMT, hStmt, L"Freeing ODBC-Statement Handle failed with SQL_ERROR, handle is still valid.");
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			SErrorInfoVector errs = GetAllErrors(SQL_HANDLE_STMT, hStmt);
			for (SErrorInfoVector::const_iterator it = errs.begin(); it != errs.end(); ++it)
			{
				LOG_ERROR(boost::str(boost::wformat(L"Failed in SQLFreeHandle %s (%d): %s") % ret % SqlReturn2s(ret) % it->ToString()));
			}
			// handle still valid, return it
			return hStmt;
		}
		else if (ret == SQL_INVALID_HANDLE)
		{
			// If we've received INVALID_HANDLE our handle has probably already be deleted - anyway, its invalid, reset it.
			// We are unable to get any error information
			hStmt = SQL_NULL_HSTMT;
			if ((flags & FSTF_THROW_ON_SQL_INVALID_HANDLE) == FSTF_THROW_ON_SQL_INVALID_HANDLE)
			{
				SqlResultException ex(L"SQLFreeHandle", ret, L"Freeing ODBC-Statement Handle failed with SQL_INVALID_HANDLE.");
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			// handle is invalid
			return SQL_NULL_HSTMT;
		}
		// SQL_SUCCESS
		return SQL_NULL_HSTMT;
	}


	StatementCloser::StatementCloser(SQLHSTMT hStmt, bool closeOnConstruction /* = false */, bool closeOnDestruction /* = true */)
		: m_hStmt(hStmt)
		, m_closeOnDestruction(closeOnDestruction)
	{
		if (closeOnDestruction)
		{
			CloseStmtHandle(m_hStmt, StmtCloseMode::IgnoreNotOpen);
		}
	}


	StatementCloser::~StatementCloser()
	{
		try
		{
			CloseStmtHandle(m_hStmt, StmtCloseMode::IgnoreNotOpen);
		}
		catch (Exception ex)
		{
			// Should never happen?
			// \todo Ticket #100
			LOG_ERROR(ex.ToString());
		}		
	}
}

// Interfaces
// ----------

