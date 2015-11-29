// BufferOverlowReally.cpp : Defines the entry point for the console application.
//

#define DSN L"exMySql"
#define USER L"ex"
#define PASS L"extest"

// system
#include <iostream>
#include <tchar.h>
#include <windows.h>

// odbc-things
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>

void printErrors(SQLSMALLINT handleType, SQLHANDLE h)
{
	SQLSMALLINT recNr = 1;
	SQLRETURN ret = SQL_SUCCESS;
	SQLSMALLINT cb = 0;

	SQLWCHAR sqlState[5 + 1];
	SQLINTEGER nativeErr;
	SQLWCHAR msg[SQL_MAX_MESSAGE_LENGTH + 1];

	while (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
	{
		msg[0] = 0;
		ret = SQLGetDiagRec(handleType, h, recNr, sqlState, &nativeErr, msg, SQL_MAX_MESSAGE_LENGTH + 1, &cb);
		if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO)
		{
			std::wcout << L"SQLSTATE: " << sqlState << L"; nativeErr: " << nativeErr << L" Msg: " << msg << std::endl;
		}
		++recNr;
	}
}

int main()
{
	SQLHENV henv = SQL_NULL_HENV;
	SQLHDBC hdbc = SQL_NULL_HDBC;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLHDESC hdesc = SQL_NULL_HDESC;

	SQLRETURN ret = 0;
	ret = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
	ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
	ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	ret = SQLConnect(hdbc, (SQLWCHAR*)DSN, SQL_NTS, USER, SQL_NTS, PASS, SQL_NTS);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrors(SQL_HANDLE_DBC, hdbc);
		getchar();
		return -1;
	}
	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	SQLWCHAR buff[20];
	
	SQLINTEGER cb = 0;
	
	ret = SQLBindCol(hstmt, 1, SQL_C_WCHAR, (SQLPOINTER*)buff, 20, &cb);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrors(SQL_HANDLE_STMT, hstmt);
		getchar();
		return -1;
	}
	ret = SQLExecDirect(hstmt, L"SELECT idintegertypes FROM integertypes WHERE idintegertypes = 4", SQL_NTS);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrors(SQL_HANDLE_STMT, hstmt);
		getchar();
		return -1;
	}

	ret = SQLFetch(hstmt);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrors(SQL_HANDLE_STMT, hstmt);
		getchar();
		return -1;
	}

	std::wcout << buff << std::endl;

	return 0;
}

