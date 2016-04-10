/*!
* \file SqlCPointerBuffer.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 10.04.2016
* \copyright GNU Lesser General Public License Version 3
*
* ShortIntro Sample.
*/


#include <SDKDDKVer.h>
#include <iostream>
#include <tchar.h>

// Include all the stuff required to include the extensions for Sql Server
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#include <odbcinst.h>

// Include Sql Server extensions
#include "msodbcsql.h"

// And the rest
#include "exodbc/exOdbc.h"
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/Table.h"

#include "boost/format.hpp"

using namespace exodbc;
using namespace std;

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		// SQL_SS_TIME2 requires ODBC Version 3.8
		EnvironmentPtr pEnv = Environment::Create(OdbcVersion::V_3_8);

		// And connect to a database using the environment.
		DatabasePtr pDb = Database::Create(pEnv);
		pDb->Open(L"Driver={SQL Server Native Client 11.0};Server=192.168.56.20\\EXODBC;Database=exodbc;Uid=ex;Pwd=extest;");

		// Create a table with a SQL Server time(7) column.
		// The time2 column has a fractional part (while the general time columns in SQL do not have
		// a fractional part).
		try
		{
			pDb->ExecSql(L"DROP TABLE timeTable");
			pDb->CommitTrans();
		}
		catch (const SqlResultException& ex)
		{
			std::wcerr << L"Warning: Drop failed: " << ex.ToString() << std::endl;
		}
		pDb->ExecSql(L"CREATE TABLE timeTable ( id INTEGER IDENTITY(1, 1) PRIMARY KEY, timeCol time(7))");
		pDb->ExecSql(L"INSERT INTO timeTable (timeCol) VALUES('15:13:02.1234567')");
		pDb->CommitTrans();

		// Opening the table will fail: This database sql type is not registered, an exception like
		// exodbc:NotSupportedException[sql2buffertypemap.cpp(73)@exodbc::Sql2BufferTypeMap::GetBufferType]: SQL Type is not registered in Sql2BufferTypeMap and no default Type is set: Not Supported SQL Type??? (-154)
		// will be thrown during Open()
		Table t(pDb, TableAccessFlag::AF_READ_WRITE, L"timeTable");
		try
		{
			t.Open();
		}
		catch (const NotSupportedException& nse)
		{
			std::wcerr << L"As expected, opening fails because type is not supported: " << nse.ToString() << std::endl;
		}

		// If we want to open a table for updating where row to update is identified by the primary key,
		// we need the primary key column too, so lets add it:
		// (note: The primary key column is an auto-increment column (IDENDITY(1,1), we will only read it)
		LongColumnBufferPtr pIdCol = LongColumnBuffer::Create(L"id", SQL_INTEGER, ColumnFlag::CF_PRIMARY_KEY | ColumnFlag::CF_READ);
		t.SetColumn(1, pIdCol);

		// We can bind the Sql Sever 'time(7)' column to a type SQL_SS_TIME2_STRUCT by creating the structure
		// and passing structure to a SqlCPointerBuffer (which will accept any SQLPOINTER as buffer).
		// The variant used to store ColumnBuffers can handle a SqlCPointerBufferPtr

		// Query the database about the appropriate values for ColumnSize and DecimalDigits:
		SQLINTEGER columnSize = 0;
		SQLSMALLINT decimalDigits = 0;
		ColumnInfosVector colInfos = pDb->ReadTableColumnInfo(L"timeTable", L"", L"", L"");
		for (auto it = colInfos.begin(); it != colInfos.end(); ++it)
		{
			if (it->GetQueryName() == L"timeCol")
			{
				columnSize = it->GetColumnSize();
				decimalDigits = it->GetDecimalDigits();
				break;
			}
		}
		SQL_SS_TIME2_STRUCT time2;
		int p = SQL_C_SS_TIME2;
		SqlCPointerBufferPtr pTime2Col = SqlCPointerBuffer::Create(
			L"timeCol",  // column name, used in queries
			SQL_SS_TIME2, // column Sql type
			(SQLPOINTER)&time2, // pointer to buffer
			SQL_C_SS_TIME2, // buffer type
			sizeof(time2),  // size of buffer in bytes
			ColumnFlag::CF_READ_WRITE | ColumnFlag::CF_NULLABLE, // flags for the column, we want to insert, update, read
			columnSize, // column size
			decimalDigits  // decimal digits
		);
		pTime2Col->SetSqlType(SQL_SS_TIME2);
		t.SetColumn(2, ColumnBufferPtrVariant(pTime2Col));

		// Now opening will succeed:
		t.Open();

		// Read the time2 value, including a fractional part:
		t.Select(L"", boost::str(boost::wformat(L"%s ASC") % pTime2Col->GetQueryName()));
		if(t.SelectNext())
		{ 
			// And print the value
			std::wcout << L"TimeCol value: " << time2.hour << L":" << time2.minute << L":" << time2.second << L"." << time2.fraction << endl;
		}
		else
		{
			std::wcerr << L"No data found!" << std::endl;
		}

		// And modify the value through our buffer:
		// A fraction in a Timestamp structure is the number of billionths of a second (would range from 0 through 999,999,999).
		time2.hour = 14;
		time2.minute = 00;
		time2.second = 30;
		time2.fraction = 500000000; // 0.5 seconds -> 500'000'000 billionths of a second
		t.Update();
		time2.fraction =  25000000; // 0.025 seconds -> 25'000'000 billionths of a second
		t.Insert();
		pDb->CommitTrans();

		// read back
		t.Select(L"", boost::str(boost::wformat(L"%s DESC") % pTime2Col->GetQueryName()));
		while (t.SelectNext())
		{
			std::wcout << L"TimeCol value: " << time2.hour << L":" << time2.minute << L":" << time2.second << L"." << time2.fraction << endl;
		}
	}
	catch (const Exception& ex)
	{
		std::wcerr << ex.ToString() << std::endl;
	}
	return 0;
}

