/*
// create database using:

create a table decTable with an id and a decimal(5,3) column:

SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE[dbo].[decTable](
[id][int] NOT NULL,
[dec][decimal](5, 3) NULL,
CONSTRAINT[PK_decTable] PRIMARY KEY CLUSTERED
(
[id] ASC
)WITH(PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON[PRIMARY]
) ON[PRIMARY]

GO


// Then create an odbc DSN entry that can be used:
*/

#define DSN L"exOdbc_SqlServer_2014"
#define USER L"exodbc"
#define PASS L"testexodbc"

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
	ret = SQLConnect(hdbc, (SQLWCHAR*)DSN, SQL_NTS, USER, SQL_NTS, PASS, SQL_NTS);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrors(SQL_HANDLE_DBC, hdbc);
		getchar();
		return -1;
	}
	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

	// Bind id as parameter
	SQLINTEGER id = 0;
	SQLINTEGER cbId = 0;
	ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &id, sizeof(id), &cbId);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
	}

	// Bind numStr as Insert-parameter
	SQL_NUMERIC_STRUCT numStr;
	ZeroMemory(&numStr, sizeof(numStr));
	SQLINTEGER cbNum = 0;
	ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_NUMERIC, SQL_NUMERIC, 18, 10, &numStr, sizeof(cbNum), &cbNum);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
	}

	/* Modify the fields in the implicit application parameter descriptor */
	SQLGetStmtAttr(hstmt, SQL_ATTR_APP_PARAM_DESC, &hdesc, 0, NULL);
	SQLSetDescField(hdesc, 2, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC, 0);
	SQLSetDescField(hdesc, 2, SQL_DESC_PRECISION, (SQLPOINTER)18, 0);
	SQLSetDescField(hdesc, 2, SQL_DESC_SCALE, (SQLPOINTER)10, 0);
	SQLSetDescField(hdesc, 2, SQL_DESC_DATA_PTR, (SQLPOINTER)&numStr, 0);

	// Prepare statement
	ret = SQLPrepare(hstmt, L"INSERT INTO [test].[dbo].[dec3Table] (id, dec) values(?, ?)", SQL_NTS);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
	}

	// Set some data and execute
	id = 1;
	//SQLINTEGER iVal = 12345;
	//memcpy(numStr.val, &iVal, sizeof(iVal));
	numStr.val[0] = 78;
	numStr.val[1] = 243;
	numStr.val[2] = 48;
	numStr.val[3] = 166;
	numStr.val[4] = 75;
	numStr.val[5] = 155;
	numStr.val[6] = 182;
	numStr.val[7] = 1;
	numStr.precision = 18;
	numStr.scale = 10;
	numStr.sign = 1;
	//SQLINTEGER iVal2 = (SQLINTEGER)numStr.val;
	ret = SQLExecute(hstmt);
	if (!SQL_SUCCEEDED(ret))
	{
		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
	}

	getchar();
	return 0;
}


//int _tmain(int argc, _TCHAR* argv[])
//{
//	SQLHENV henv = SQL_NULL_HENV;
//	SQLHDBC hdbc = SQL_NULL_HDBC;
//	SQLHSTMT hstmt = SQL_NULL_HSTMT;
//	SQLHDESC hdesc = SQL_NULL_HDESC;
//
//	SQLRETURN ret = 0;
//
//	// Connect to DB
//	ret = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
//	ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
//	ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
//	ret = SQLConnect(hdbc, (SQLWCHAR*)DSN, SQL_NTS, USER, SQL_NTS, PASS, SQL_NTS);
//	if (!SQL_SUCCEEDED(ret))
//	{
//		printErrors(SQL_HANDLE_DBC, hdbc);
//		getchar();
//		return -1;
//	}
//	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
//
//	// Bind id as parameter
//	SQLINTEGER id = 0;
//	SQLINTEGER cbId = 0;
//	ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &id, sizeof(id), &cbId);
//	if (!SQL_SUCCEEDED(ret))
//	{
//		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
//	}
//
//	// Bind numStr as Insert-parameter
//	SQL_NUMERIC_STRUCT numStr;
//	ZeroMemory(&numStr, sizeof(numStr));
//	SQLINTEGER cbNum = 0;
//	ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_NUMERIC, SQL_NUMERIC, 18, 0, &numStr, sizeof(cbNum), &cbNum);
//	if (!SQL_SUCCEEDED(ret))
//	{
//		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
//	}
//
//	/* Modify the fields in the implicit application parameter descriptor */
//	SQLGetStmtAttr(hstmt, SQL_ATTR_APP_PARAM_DESC, &hdesc, 0, NULL);
//	SQLSetDescField(hdesc, 2, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC, 0);
//	SQLSetDescField(hdesc, 2, SQL_DESC_PRECISION, (SQLPOINTER)18, 0);
//	SQLSetDescField(hdesc, 2, SQL_DESC_SCALE, (SQLPOINTER)0, 0);
//	SQLSetDescField(hdesc, 2, SQL_DESC_DATA_PTR, (SQLPOINTER)&numStr, 0);
//
//	// Prepare statement
//	ret = SQLPrepare(hstmt, L"INSERT INTO [test].[dbo].[dec2Table] (id, dec) values(?, ?)", SQL_NTS);
//	if (!SQL_SUCCEEDED(ret))
//	{
//		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
//	}
//
//	// Set some data and execute
//	id = 1;
//	//SQLINTEGER iVal = 12345;
//	//memcpy(numStr.val, &iVal, sizeof(iVal));
//	numStr.val[0] = 78;
//	numStr.val[1] = 243;
//	numStr.val[2] = 48;
//	numStr.val[3] = 166;
//	numStr.val[4] = 75;
//	numStr.val[5] = 155;
//	numStr.val[6] = 182;
//	numStr.val[7] = 1;
//	numStr.precision = 18;
//	numStr.scale = 0;
//	numStr.sign = 1;
//	//SQLINTEGER iVal2 = (SQLINTEGER)numStr.val;
//	ret = SQLExecute(hstmt);
//	if (!SQL_SUCCEEDED(ret))
//	{
//		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
//	}
//
//	getchar();
//	return 0;
//}


//int _tmain(int argc, _TCHAR* argv[])
//{
//	SQLHENV henv = SQL_NULL_HENV;
//	SQLHDBC hdbc = SQL_NULL_HDBC;
//	SQLHSTMT hstmt = SQL_NULL_HSTMT;
//	SQLHDESC hdesc = SQL_NULL_HDESC;
//
//	SQLRETURN ret = 0;
//
//	// Connect to DB
//	ret = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
//	ret = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
//	ret = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
//	ret = SQLConnect(hdbc, (SQLWCHAR*)DSN, SQL_NTS, USER, SQL_NTS, PASS, SQL_NTS);
//	if (!SQL_SUCCEEDED(ret))
//	{
//		printErrors(SQL_HANDLE_DBC, hdbc);
//		getchar();
//		return -1;
//	}
//	ret = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
//
//	// Bind id as parameter
//	SQLINTEGER id = 0;
//	SQLINTEGER cbId = 0;
//	ret = SQLBindParameter(hstmt, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &id, sizeof(id), &cbId);
//	if (!SQL_SUCCEEDED(ret))
//	{
//		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
//	}
//
//	// Bind numStr as Insert-parameter
//	SQL_NUMERIC_STRUCT numStr;
//	ZeroMemory(&numStr, sizeof(numStr));
//	SQLINTEGER cbNum = 0;
//	ret = SQLBindParameter(hstmt, 2, SQL_PARAM_INPUT, SQL_C_NUMERIC, SQL_NUMERIC, 5, 3, &numStr, sizeof(cbNum), &cbNum);
//	if (!SQL_SUCCEEDED(ret))
//	{
//		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
//	}
//	
//	/* Modify the fields in the implicit application parameter descriptor */
//	SQLGetStmtAttr(hstmt, SQL_ATTR_APP_PARAM_DESC, &hdesc, 0, NULL);
//	SQLSetDescField(hdesc, 2, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC, 0);
//	SQLSetDescField(hdesc, 2, SQL_DESC_PRECISION, (SQLPOINTER)5, 0);
//	SQLSetDescField(hdesc, 2, SQL_DESC_SCALE, (SQLPOINTER)3, 0);
//	SQLSetDescField(hdesc, 2, SQL_DESC_DATA_PTR, (SQLPOINTER)&numStr, 0);
//
//	// Prepare statement
//	ret = SQLPrepare(hstmt, L"INSERT INTO [test].[dbo].[decTable] (id, dec) values(?, ?)", SQL_NTS);
//	if (!SQL_SUCCEEDED(ret))
//	{
//		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
//	}
//	
//	// Set some data and execute
//	id = 1;
//	SQLINTEGER iVal = 12345;
//	//memcpy(numStr.val, &iVal, sizeof(iVal));
//	numStr.val[0] = 57;
//	numStr.val[1] = 48;
//	numStr.precision = 5;
//	numStr.scale = 3;
//	numStr.sign = 1;
//	SQLINTEGER iVal2 = (SQLINTEGER)numStr.val;
//	ret = SQLExecute(hstmt);
//	if (!SQL_SUCCEEDED(ret))
//	{
//		printErrorsAndAbort(SQL_HANDLE_STMT, hstmt);
//	}
//	
//	getchar();
//	return 0;
//}
//
