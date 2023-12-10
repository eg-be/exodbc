# exOdbc - open source C++ ODBC library #
> [!IMPORTANT]
> **Unfortunately I don't have the time to give this project the love it deserves.** The source is "as-is" and no issues will be addressed currently. The docs have been imported from the original trac-wiki and only some parts have been transformed to proper markdown.

exOdbc is an open source C++ library to access [ODBC](https://en.wikipedia.org/wiki/Open_Database_Connectivity) data sources. It was inspired by [wxOdbc](https://wiki.wxwidgets.org/ODBC), but has been rewritten completely. At its core exOdbc uses a [boost::variant](http://www.boost.org/doc/libs/release/doc/html/variant.html) to store the different types of column buffers. 

See [TestedDatabases](docs/TestedDatabases.md) for an overview of Databases and ODBC drivers exOdbc has been tested with.

exOdbc is released under the GNU LESSER GENERAL PUBLIC LICENSE, see [exOdbcLicense](lgpl.txt).

exOdbc uses [CMake](https://cmake.org/) to support different plattforms. It has been tested on Windows and Linux.

## Features ##
* Easy access to the catalog functions of the ODBC-API: Search for tables, views, etc. List catalogs, schemas and types. Query primary keys and special columns.
* Prepared statements for rapid execution of SQL queries. 
* Execute arbitrary SQL directly against a database.
* Table objects that upon opening query the database about the columns of the table (or view) and automatically create corresponding SQL C type buffers for data exchange.
* Provides wrappers for the SQL C types to quickly create a Buffer to hold the result of a query, or to be used as a parameter.
* Easily extensible for new SQL C types.
* UTF-8 compatible. Internally, only utf-8 std::string objects are stored. On Windows strings will get converted to UTF-16 when calling the ODBC API.
* Testing with [Google Test](https://github.com/google/googletest).
* Doxygen generated Documentation TODO: https://github.com/eg-be/exodbc/issues/1

## Current Version ##
Last Release is https://github.com/eg-be/exodbc/releases/tag/exOdbc_0.9.4

## Download and Compile ##
Make sure you have [CMake](https://cmake.org/) installed and that you have a copy of the [boost library](http://www.boost.org/) ready. To build the tests, you need compiled libraries of boost-filesystem and boost-system.

After extracting the tarball, cd into the top-level directory and:

```
mkdir build
cd build

# On Winodws, indicate the path to the boost libraries:
set BOOST_ROOT=e:/lib/boost_1_62_0

# Build only the library and the samples:
cmake -DBUILD_TESTS=OFF ..
cmake --build .
```

Resulting binaries have been placed in `bin`.

See [INSTALL.txt](INSTALL.txt) for more cmake configuration options.

## Tests ##
exOdbc uses [ggogletest](https://code.google.com/p/googletest/) to test the functionality against different databases and ODBC drivers.

To build the tests, set cmake option `BUILD_TESTS` to `ON` (default).

```
mkdir build
cd build

# On Winodws, indicate the path to the boost libraries:
# Make sure that compiled version of boost-system and boost-filesystem are ready.
set BOOST_ROOT=e:/lib/boost_1_62_0

# Build everything:
cmake ..
cmake --build .
```

Resulting test executable `exodbctest` has been placed in `bin`.

Most of the tests require a database with some tables. See [exOdbcTest](docs/exOdbcTest.md) for how to prepare the test database.

[TestedDatabases](docs/TestedDatabases.md) has more information about the tested databases and the known failures of exOdbc.

## Samples ##
Samples are available [online](/samples/) with some additional [Description](docs/samples.md).

## Short Intro ##
The following sample demonstrates basic usage of exOdbc:

See [samples/ShortIntro](/samples/ShortIntro) for updated versions.

```cpp
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
#include "exodbc/SqlStructHelper.h"

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

		// if argv[1] is given, assume its a connection string, else use some built-in default cs:
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
		SqlInfoProperties props = pDb->GetProperties();
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"Connected to: %s, using %s") % props.GetDbmsName() % props.GetDriverName()));

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
				% SqlStructHelper::TimestampToSqlString(pLastUpdateCol->GetValue())
				);
			WRITE_STDOUT_ENDL(s);
		}
		WRITE_STDOUT_ENDL(u8"");

		// Update the row with a primary key id value of 101:
		pIdCol->SetValue(101);
		pLastUpdateCol->SetValue(SqlStructHelper::InitUtcTimestamp());
		nameColumnWrapper.SetValue(u8"Updated");
		t.UpdateByPkValues();

		// Insert a single entry
		pIdCol->SetValue(200);
		pLastUpdateCol->SetValue(SqlStructHelper::InitUtcTimestamp());
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
			pLastUpdateCol->SetValue(SqlStructHelper::InitUtcTimestamp());
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
				% SqlStructHelper::TimestampToSqlString(pLastUpdateCol->GetValue())
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
```

## About ##
exOdbc was written by Elias Gerber.

Contact: eg (AT) elisium.ch
