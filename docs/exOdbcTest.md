# Preparing and Running the exOdbc tests
Information on how to setup and run the exOdbc tests.

## Setup Test Environment
The tests require a test database with tables containing all tested datatypes and a working DSN entry or a working connection string to connect to the test database. 
SQL scripts to create the required test tables and populate them with the expected test values are available. In the directory [test/sql/](/test/sql/), a subdirectory for every tested database system is provided with the corresponding SQL scripts to create the tables. For the following database systems create scripts are provided:

* [IBM DB2 Express-C](http://www.ibm.com/software/data/db2/express-c/)
* [Microsoft SQL Server Express](https://www.microsoft.com/de-ch/server-cloud/products/sql-server/)
* [MySQL Community Edition](https://www.mysql.de/)
* [Microsoft Access](https://products.office.com/de-ch/access)

You can either run these scripts manually in your test database, or use the [exodbctest-application](#exodbctest-application) with the parameter `--createDb` to create the required test tables structure.

The test tables and their contents can be accessed through the TestedDatabases page.

## exOdbcTest Application
The test application `exodbctest` is based on [Google Test](https://github.com/google/googletest). You can use all Google Test options to filter tests, change the output-type, etc. See the Google Test manual for the available options.

exOdbcTest provides some additional arguments to specify the connection information for the test database. You have to either specify the connection information on the command line (by referencing a configured DSN entry, or by a connection string) or exOdbcTest will search for a configuration file `TestSettings.xml`

See `exodbcest --help` for more information about how `TestSettings.xml` is located and all options and their syntax.

## TestSettings.xml 
The `TestSettings.xml` is an xml-file with the following structure:
```xml
<TestSettings>

  <CreateDb>0</CreateDb>
  <LogLevel>Debug</LogLevel>

  <Dsn>
    <Disabled>01</Disabled>
    <Name>exMySql_64</Name>
    <User>ex</User>
    <Pass>extest</Pass>
    <Case>lower</Case>
    <Skip ticket="76">DatabaseTest.ReadTablePrivileges</Skip>
    <Skip ticket="76">TableTest.CheckPrivileges</Skip>
    <Skip ticket="120">StatementCloserTest.CloseStmtHandle</Skip>
    <Workaround ticket="206">NumericColumnTest.Write_18_10_Value</Workaround>
  </Dsn>

  <ConnectionString>
    <Disabled>1</Disabled>
    <Value>Driver={SQL Server Native Client 11.0};Server=192.168.56.20\EXODBC;Database=exodbc;Uid=ex;Pwd=extest;</Value>
    <Case>lower</Case>
  </ConnectionString>

</TestSettings>
```

* `CreateDb`: If set to `1`, the parameter `--createDb` is activated.
* `LogLevel`: Set the log level. Valid values are `Debug`, `Info`, `Warning` and `Error`.
* `Disabled` (in `Dsn` and `ConnectionString` elements): The first `Dsn` or `ConnectionString` element in the file where `Disabled` is set to `0` will be used to run the tests.
* `Dsn`: Specify a DSN entry by passing the DSN `Name`, `User` and `Pass`.
* `ConnectionString`: Specify the `Value` of a connection string to use.
* `Case` (in `Dsn` and `ConnectionString` elements): Set to `lower` if the test database uses table and column names in lower-case letters, or to `upper` if the test database uses UPPERCASE table and column names.
* `Skip` (in `Dsn` and `ConnectionString` elements): Some tests are known to fail on certain databases. Either because the setup of the test database is incorrect (and to complicated to fix), or because the driver does not support some ODBC feature, etc. The test known to fail are listed as `Skip` elements. If no argument `--gtest_filter` is already set, a `--gtest_filter` argument is constructed which filters out the tests listed in `Skip` elements. If an argument `--gtest_filter` is already set, the argument is not modified and `Skip` entries are ignored.
* `Workaround` (in `Dsn` and `ConnectionString` elements): Just a hint that there is some sort of a workaround in the test to make the test pass and link to corresponding ticket.

## Known Failures
There are several things that are known to fail when running the tests:

TODO: Recreate the list of failed things, see [#5](/../../issues/5)

See [TestedDatabases](TestedDatabases.md) for more information about known failures, and what drivers, database types etc. were used during the tests.
