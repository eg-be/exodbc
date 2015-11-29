// BufferOverlowReally.cpp : Defines the entry point for the console application.
//

#define DSN L"exMySql"
#define USER L"ex"
#define PASS L"extest"

// system
#include <iostream>
#include <tchar.h>
#include <windows.h>
#include <vector>
#include <memory>

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
	int elements = 20;

	// Allocate a Buffer using [] - works fine.
	//SQLWCHAR buff[20];

	// and the same thing using a vector
	std::vector<SQLWCHAR> vecBuff(elements, 'a');

	// and using a sharedptr
	//std::shared_ptr<SQLWCHAR> pBuff(new SQLWCHAR[elements], std::default_delete<SQLWCHAR[]>());

	SQLINTEGER cb = 0;
	
	// bind using [] buffer
//	ret = SQLBindCol(hstmt, 1, SQL_C_WCHAR, (SQLPOINTER*)buff, elements * sizeof(SQLWCHAR), &cb);

	// bind using vector buffer
	ret = SQLBindCol(hstmt, 1, SQL_C_WCHAR, (SQLPOINTER*)&vecBuff[0], elements * sizeof(SQLWCHAR), &cb);

	// or using shared-ptr
	//ret = SQLBindCol(hstmt, 1, SQL_C_WCHAR, (SQLPOINTER*)pBuff.get(), elements * sizeof(SQLWCHAR), &cb);

	std::shared_ptr<std::vector<SQLWCHAR>> pVec = std::make_shared<std::vector<SQLWCHAR>>(9);
	(*pVec)[0] = 'a';
	(*pVec)[1] = 'b';

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

	// buff
//	std::wcout << buff << std::endl;

	// vect
	std::wstring val(vecBuff.data());
	std::wcout << val.c_str() << std::endl;

	// shared_ptr
	//std::wcout << pBuff << std::endl;
	return 0;
}

