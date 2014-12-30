/*
// create database using:

create database if not exists test;
drop table if exists test.numtable;
create table if not exists test.numtable (
id INT not null,
num DECIMAL(5, 3),
primary key(id)
);
delete from test.numtable where id > 0;
insert into test.numtable (id, num) values (1, 12.345);
insert into test.numtable (id, num) values (2, null);

// Then create an odbc DSN entry that can be used:
*/

#define DSN L"MySqlTest"
#define USER L"User"
#define PASS L"Pass"

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


void printErrorsAndAbort(SQLSMALLINT handleType, SQLHANDLE h)
{
	printErrors(handleType, h);
	getchar();
	abort();
}


int _tmain(int argc, _TCHAR* argv[])
{
	SQLHENV henv = SQL_NULL_HENV;
	SQLHDBC hdbc = SQL_NULL_HDBC;
	SQLHSTMT hstmt = SQL_NULL_HSTMT;
	SQLHDESC hdesc = SQL_NULL_HDESC;

	SQLRETURN ret = 0;

	// Connect to DB
	ret = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
	ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
	ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	ret = SQLConnect(hdbc, (SQLWCHAR*) DSN, SQL_NTS, USER, SQL_NTS, PASS, SQL_NTS);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrors(SQL_HANDLE_DBC, hdbc);
		return -1;
	}
	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	
	// Get the application row descriptor for the statement handle using
	ret = SQLGetStmtAttr(hstmt, SQL_ATTR_APP_ROW_DESC, &hdesc, 0, NULL);

	// Bind numStr using SQLSetDescField
	SQLSMALLINT recNr = 2;
	SQL_NUMERIC_STRUCT numStr;
	ZeroMemory(&numStr, sizeof(numStr));
	SQLINTEGER indicator = 0;

	ret = SQLSetDescField(hdesc, recNr, SQL_DESC_TYPE, (VOID*) SQL_C_NUMERIC, 0);
	ret = SQLSetDescField(hdesc, recNr, SQL_DESC_PRECISION, (VOID*) 5, 0);
	ret = SQLSetDescField(hdesc, recNr, SQL_DESC_SCALE, (VOID*) 3, 0);
	// Set the Indicator-pointer.
	ret = SQLSetDescField(hdesc, recNr, SQL_DESC_INDICATOR_PTR, (VOID*)&indicator, 0);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrorsAndAbort(SQL_HANDLE_DESC, hdesc);
	}
	ret = SQLSetDescField(hdesc, recNr, SQL_DESC_DATA_PTR, (VOID*) &numStr, 0);

	// Query a null row:
	ret = SQLExecDirect(hstmt, L"SELECT id, num FROM test.numtable WHERE id = 2", SQL_NTS);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
	}

	ret = SQLFetch(hstmt);
	if (ret != SQL_SUCCESS)
	{
		// We get SQLSTATE 22002 here, saying indicator not bound. But it was bound without error
		printErrors(SQL_HANDLE_STMT, hstmt);
	}
	// No data has been fetched, indicator is not null
	std::wcout << L"Indicator is: ";
	if (indicator == SQL_NULL_DATA)
	{
		std::wcout << L" Set to NULL" << std::endl;
	}
	else
	{
		std::wcout << L" Not null: " << indicator << std::endl;
	}
	// Close cursor:
	ret = SQLCloseCursor(hstmt);
	
	// set indicator to nonsense:
	indicator = 131313;
	// Query a non-null row
	ret = SQLExecDirect(hstmt, L"SELECT id, num FROM test.numtable WHERE id = 1", SQL_NTS);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
	}

	ret = SQLFetch(hstmt);
	if (ret != SQL_SUCCESS)
	{
		printErrors(SQL_HANDLE_STMT, hstmt);
	}
	// we have the result:
	SQLBIGINT* pInt = (SQLBIGINT*)&numStr.val;
	std::wcout << L"Value: " << *pInt << std::endl;
	std::wcout << L"Scale: " << numStr.scale << std::endl;
	std::wcout << L"Sign: " << numStr.sign << std::endl;
	// but indicator was not changed:
	std::wcout << L"Indicator: " << indicator << std::endl;

	getchar();

	return 0;
}

