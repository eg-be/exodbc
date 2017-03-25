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
#include <tchar.h>
#endif
#include <iostream>

#include "exodbc/exOdbc.h"
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/Table.h"
#include "exodbc/ExecutableStatement.h"
#include "exodbc/LogManager.h"
#include "exodbc/ColumnBufferWrapper.h"

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	using namespace exodbc;

	try
	{
		// Create an environment with ODBC Version 3.0
		EnvironmentPtr pEnv = Environment::Create(OdbcVersion::V_3);

		// And connect to a database using the environment.
		DatabasePtr pDb = Database::Create(pEnv);

		// if argv[1] is given, assume its a connection string, else some built-in default cs:
		// std::string connectionString = "Provider=MSDASQL;Driver={MySQL ODBC 5.3 UNICODE Driver};Server=192.168.56.20;Database=exodbc;User=ex;Password=extest;Option=3;"
		// std::string connectionString = ""Driver={IBM DB2 ODBC DRIVER};Database=EXODBC;Hostname=192.168.56.20;Port=50000;Protocol=TCPIP;Uid=db2ex;Pwd=extest;CurrentSchema=EXODBC"
		std::string connectionString = u8"Driver={SQL Server Native Client 11.0};Server=192.168.56.20\\EXODBC,1433;Database=exodbc;Uid=ex;Pwd=extest;";
		if (argc >= 2)
		{
#ifdef _WIN32
			connectionString = utf16ToUtf8(argv[1]);
#else
			connectionString = argv[1];
#endif
		}
		
		// Note: WRITE_STDOUT_ENDL will write to std::wcout on _WIN32, and convert utf-8 to utf-16
		// else directly to std::cout
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"Connecting to: %s") % connectionString));
		
		// Open connection to database:
		pDb->Open(connectionString);
		DatabaseInfo dbInfo = pDb->GetDbInfo();
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"Connected to: %s, using %s") % dbInfo.GetDbmsName() % dbInfo.GetDriverName()));

		// Create a test table in the database with a few entries
		// try to drop first if it already exists, but ignore failing to drop
		// auto commit is off by default, so commit changes manually
		try
		{
			pDb->ExecSql(u8"DROP TABLE T1");
			pDb->CommitTrans();
		}
		catch (const SqlResultException& ex)
		{
			std::stringstream ss;
			ss << u8"Warning: Drop failed: " << ex.ToString();
			WRITE_STDOUT_ENDL(ss.str());
		}
		if (pDb->GetDbms() == DatabaseProduct::DB2)
		{
			pDb->ExecSql(u8"CREATE TABLE T1 ( id INTEGER NOT NULL PRIMARY KEY, name varchar(16), lastUpdate timestamp)");
		}
		else
		{
			pDb->ExecSql(u8"CREATE TABLE T1 ( id INTEGER NOT NULL PRIMARY KEY, name varchar(16), lastUpdate datetime)");
		}
		pDb->ExecSql(u8"INSERT INTO T1 (id, name, lastUpdate) VALUES(101, 'Cat', '1993-10-24 21:12:04')");
		pDb->ExecSql(u8"INSERT INTO T1 (id, name, lastUpdate) VALUES(102, 'Dog', '2011-08-01 04:02:06')");
		pDb->CommitTrans();

		// Create the Table object and Open() it.
		// The Database will be queried about a table named 't1' of any type (view, table, ..) during open.
		Table t(pDb, TableAccessFlag::AF_READ_WRITE, u8"T1");
		t.Open();
		// As we did not specify any ColumnBuffer manually before calling Open()
		// the Database has been queried about the columns of table 't1'.
		// For every column found its SQL Type (like SQL_VARCHAR, SQL_INTEGER) is examined
		// to create a ColumnBufferPtr with a matching SQL C Type (like SQL_C_CHAR, SQL_C_SLONG).
		// (The mapping of SQL Types to SQL C Types is defined in a Sql2BufferTypeMap and can
		// be changed dynamically.)

		// For an INTEGER column most probably a LongColumnBuffer has been created,
		// while for a datetime column a TypeTimestampColumnBuffer has been created:
		auto pIdCol = t.GetColumnBufferPtr<LongColumnBufferPtr>(0);
		auto pLastUpdateCol = t.GetColumnBufferPtr<TypeTimestampColumnBufferPtr>(2);

		// Depending on the database, driver and operating system, a string column
		// might get reported as SQL_VARCHAR or SQL_WVARCHAR. We could either query
		// the ColumnBufferPtrVariant about its concrete type, or just ignore
		// the concrete type and work with a StringColumnWrapper.
		// The StringColumnWrapper will convert from/to string/wstring as needed.
		StringColumnWrapper nameColumnWrapper(t.GetColumnBufferPtrVariant(1));

		// Lets iterate all rows in the database,
		// and print them line by line:
		WRITE_STDOUT_ENDL(u8"Printing content of Table 'T1':");
		WRITE_STDOUT_ENDL(u8"");

		t.Select();
		std::string s = boost::str(boost::format(u8"%4s %16s %s")
			% u8"id" % u8"name" % u8"lastUpdate"
		);
		WRITE_STDOUT_ENDL(s);
		WRITE_STDOUT_ENDL(u8"==============================");
		while (t.SelectNext())
		{
			s = boost::str(boost::format(u8"%4d %16s %s") 
				% pIdCol->GetValue() 
				% nameColumnWrapper.GetValue<std::string>()
				% TimestampToSqlString(pLastUpdateCol->GetValue())
				);
			WRITE_STDOUT_ENDL(s);
		}
		WRITE_STDOUT_ENDL(u8"");

		// Update the row with a primary key id value of 101:
		pIdCol->SetValue(101);
		pLastUpdateCol->SetValue(InitUtcTimestamp());
		nameColumnWrapper.SetValue(u8"Updated");
		t.Update();

		// Insert a single entry
		pIdCol->SetValue(200);
		pLastUpdateCol->SetValue(InitUtcTimestamp());
		nameColumnWrapper.SetValue(u8"Inserted");
		t.Insert();

		// Insert multiple rows using a prepared statement:
		ExecutableStatement stmt(pDb);
		std::string sql = u8"INSERT INTO T1 (id, name, lastUpdate) VALUES(?, ?, ?)";
		stmt.Prepare(sql);
		// Bind our columns as parameters:
		ColumnBufferPtrVariant pNameCol = t.GetColumnBufferPtrVariant(1);
		stmt.BindParameter(pIdCol, 1);
		stmt.BindParameter(pNameCol, 2);
		stmt.BindParameter(pLastUpdateCol, 3);

		// Execute the prepared statement multiple times:
		for (int i = 300; i < 310; ++i)
		{
			pIdCol->SetValue(i);
			std::string name = boost::str(boost::format(u8"Insert: %d") % i);
			nameColumnWrapper.SetValue(name);
			pLastUpdateCol->SetValue(InitUtcTimestamp());
			stmt.ExecutePrepared();
		}

		// Commit everything
		pDb->CommitTrans();

		// And print the table again:
		WRITE_STDOUT_ENDL(u8"Printing content of Table 'T1':");
		WRITE_STDOUT_ENDL(u8"");

		t.Select();
		s = boost::str(boost::format(u8"%4s %16s %s")
			% u8"id" % u8"name" % u8"lastUpdate"
		);
		WRITE_STDOUT_ENDL(s);
		WRITE_STDOUT_ENDL(u8"==============================");
		while (t.SelectNext())
		{
			SQL_TIMESTAMP_STRUCT ts = pLastUpdateCol->GetValue();
			s = boost::str(boost::format(u8"%4d %16s %s")
				% pIdCol->GetValue()
				% nameColumnWrapper.GetValue<std::string>()
				% TimestampToSqlString(pLastUpdateCol->GetValue())
			);
			WRITE_STDOUT_ENDL(s);
		}
		WRITE_STDOUT_ENDL(u8"");
	}
	catch (const Exception& ex)
	{
		WRITE_STDERR_ENDL(ex.ToString());
	}
	catch (const std::exception& ex)
	{
		WRITE_STDERR_ENDL(ex.what());
	}
    return 0;
}

