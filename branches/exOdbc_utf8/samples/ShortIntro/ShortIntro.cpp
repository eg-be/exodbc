/*!
* \file ShortIntro.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 03.01.2016
* \copyright GNU Lesser General Public License Version 3
*
* ShortIntro Sample.
*/

#ifdef _WIN32
#include <SDKDDKVer.h>
#endif
#include <iostream>

#include "exodbc/exOdbc.h"
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/Table.h"
#include "exodbc/ExecutableStatement.h"
#include "exodbc/LogManager.h"

int main()
{
	using namespace exodbc;

	try
	{
		// Create an environment with ODBC Version 3.0
		EnvironmentPtr pEnv = Environment::Create(OdbcVersion::V_3);

		// And connect to a database using the environment.
		DatabasePtr pDb = Database::Create(pEnv);
		pDb->Open(u8"Driver={SQL Server Native Client 11.0};Server=192.168.56.20\\EXODBC,1433;Database=exodbc;Uid=ex;Pwd=extest;");

		// Set up some test table - try to drop first if it already exists
		// auto commit is off by default, so commit changes manually
		try
		{
			pDb->ExecSql(u8"DROP TABLE t1");
			pDb->CommitTrans();
		}
		catch (const SqlResultException& ex)
		{
			std::cerr << u8"Warning: Drop failed: " << ex.ToString() << std::endl;
		}
		pDb->ExecSql(u8"CREATE TABLE t1 ( id INTEGER NOT NULL PRIMARY KEY, name nvarchar(255), lastUpdate datetime)");
		pDb->CommitTrans();

		// Create the Table. The Database will be queried about a table named 't1' of any type during open.
		Table t(pDb, TableAccessFlag::AF_READ_WRITE, u8"t1");
		t.Open();

		// As we did not specify any columns to be bound, the Database has been queried about the columns of 't1'
		// It used the DefaultSql2BufferMap to map the SQL types reported by the Database to SQL C types supported
		// by the variant used to store ColumnBuffers
		auto pIdCol = t.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pNameCol = t.GetColumnBufferPtr<WCharColumnBufferPtr>(1);

		// or identify the column by its queryname:
		SQLUSMALLINT lastUpdateColIndex = t.GetColumnBufferIndex(u8"lastUpdate");
		auto pLastUpdateCol = t.GetColumnBufferPtr<TypeTimestampColumnBufferPtr>(lastUpdateColIndex);

		// The column buffers have been bound for reading and writing data. During Open()
		// statements have been prepared to so dome common tasks like inserting, updating, etc.
		// so lets insert a row:
		pIdCol->SetValue(13);
		pNameCol->SetWString(L"lazy dog"); // Only compilable if underlying ColumnBuffer is of type <SQLWCHAR>
		// fully specify the timestamp:
		SQL_TIMESTAMP_STRUCT ts;
		ts.day = 26; ts.month = 1; ts.year = 1983;
		ts.hour = 13; ts.minute = 45; ts.second = 27; 
		ts.fraction = 0; // its database dependent if fractions are supported and with which precision. Show that in a later sample.
		pLastUpdateCol->SetValue(ts);

		// Insert the values in the column Buffers.
		t.Insert();
		pDb->CommitTrans();

		// Reading back those timestamp values is easier if we let the driver convert it to a (w)string.
		// So open the table again, but let the driver convert everything to WCHAR buffers:
		t.Close();

#ifdef _WIN32
		t.SetSql2BufferTypeMap(std::make_shared<WCharSql2BufferMap>());
#else
		t.SetSql2BufferTypeMap(std::make_shared<CharSql2BufferMap>());
#endif
		t.Open();

#ifdef _WIN32
		auto pIdCharCol = t.GetColumnBufferPtr<WCharColumnBufferPtr>(0);
		auto pNameCharCol = t.GetColumnBufferPtr<WCharColumnBufferPtr>(1);
		auto pLastUpdateCharCol = t.GetColumnBufferPtr<WCharColumnBufferPtr>(2);
#else
		auto pIdCharCol = t.GetColumnBufferPtr<CharColumnBufferPtr>(0);
		auto pNameCharCol = t.GetColumnBufferPtr<CharColumnBufferPtr>(1);
		auto pLastUpdateCharCol = t.GetColumnBufferPtr<CharColumnBufferPtr>(2);
#endif

		// Simply query by manually providing a where statement:
		t.Select(u8"id = 13");
		while (t.SelectNext())
		{
			std::stringstream ss;
#ifdef _WIN32
			ss << u8"id: " << utf16ToUtf8(pIdCharCol->GetWString()) << u8"; Name: " << utf16ToUtf8(pNameCharCol->GetWString()) << u8"; LastUpdate: " << utf16ToUtf8(pLastUpdateCharCol->GetWString());
#else
			ss << u8"id: " << pIdCharCol->GetString() << u8"; Name: " << pNameCharCol->GetString() << u8"; LastUpdate: " << pLastUpdateCharCol->GetString();
#endif
			WRITE_STDOUT_ENDL(ss.str());
		}

		// Or create a prepared statement that binds buffers as result columns and as parameter markers:
		ExecutableStatement stmt(pDb);
		stmt.Prepare(u8"SELECT name, lastUpdate FROM t1 WHERE id = ?");
		// column and param indexes start at 1
		stmt.BindColumn(pNameCharCol, 1); // map to result column name
		stmt.BindColumn(pLastUpdateCharCol, 2); // map to result column lastUpdate
		stmt.BindParameter(pIdCol, 1); // map to first param marker
		pIdCol->SetValue(13);
		stmt.ExecutePrepared();
		
		while (stmt.SelectNext())
		{
			std::stringstream ss;
#ifdef _WIN32
			ss << u8"id: " << pIdCol->GetValue() << u8"; Name: " << utf16ToUtf8(pNameCharCol->GetWString()) << u8"; LastUpdate: " << utf16ToUtf8(pLastUpdateCharCol->GetWString());
#else
			ss << u8"id: " << pIdCol->GetValue() << u8"; Name: " << pNameCharCol->GetString() << u8"; LastUpdate: " << pLastUpdateCharCol->GetString();
#endif
			WRITE_STDOUT_ENDL(ss.str());
		}

		// Execute the same prepared statement again with different where values:
		pIdCol->SetValue(99);
		if (!stmt.SelectNext())
		{
			std::stringstream ss;
			ss << u8"No results found for id: " << *pIdCol;
			WRITE_STDOUT_ENDL(ss.str());
		}
	}
	catch (const Exception& ex)
	{
		WRITE_STDERR_ENDL(ex.ToString());
	}
    return 0;
}

