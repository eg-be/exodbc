# Tested Databases #
The test from exodbctest run directly against a test database. The following list contains links for every test-database with details about the test setup used (odbc driver, dbms system, test tables). Note that the content of the pages linked to was generated using the 
sample [ExodbcExec](/exodbc/samples/ExodbcExec/).

See [wiki:exOdbcTest] for how to prepare and run the tests.

== Access ==
* AccessDbInfo
* AccessTestTables
* SQL to create test database: source:/exodbc/trunk/test/sql/Access 

== DB2/NT64 == 
* [[Db2DbInfo]]
* [[Db2TestTables]]
* SQL to create test database: source:/exodbc/trunk/test/sql/DB2

== Excel ==
* ExcelDbInfo
* ExcelTestTables
* Test file: source:/trunk/test/db/excel/excelTest.xls

== Microsoft Sql Server ==
* SqlServerDbInfo
* SqlServerTestTables
* SQL to create test database: source:/exodbc/trunk/test/sql/SqlServer

== !MySql ==
* MySqlServerDbInfo
* MySqlServerTestTables
* SQL to create test database: source:/exodbc/trunk/test/sql/MySql

== PostgreSQL ==
* [[PostgreSQLDbInfo]]
* [[PostgreSQLTestTables]]
* SQL to create test database: source:/exodbc/trunk/test/sql/PostgreSQL

== Known Issues ==
The following is a list of things known to work not properly:

||=Database ||= Known Failures =||
||=Access || [[TicketQuery(status=new|assigned|reopened|accepted&milestone=exOdbc Known Failures&component=exOdbc.Access,order=priority)]] ||
||=DB2 || [[TicketQuery(status=new|assigned|reopened|accepted&milestone=exOdbc Known Failures&component=exOdbc.Db2,order=priority)]] ||
||=Excel || [[TicketQuery(status=new|assigned|reopened|accepted&milestone=exOdbc Known Failures&component=exOdbc.Excel,order=priority)]] ||
||=!MySql || [[TicketQuery(status=new|assigned|reopened|accepted&milestone=exOdbc Known Failures&component=exOdbc.MySql,order=priority)]] ||
||=PostgreSQL|| [[TicketQuery(status=new|assigned|reopened|accepted&milestone=exOdbc Known Failures&component=exOdbc.PostgreSQL,order=priority)]] ||
||=!SqlServer || [[TicketQuery(status=new|assigned|reopened|accepted&milestone=exOdbc Known Failures&component=exOdbc.SqlServer,order=priority)]] ||

