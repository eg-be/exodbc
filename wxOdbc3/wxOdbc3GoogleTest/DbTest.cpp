/*!
 * \file DbTest.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "DbTest.h"

// Same component headers
#include "GenericTestTables.h"

// Other headers
#include "DbEnvironment.h"
#include "Database.h"
#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
using namespace std;

namespace exodbc
{

	void DbTest::SetUp()
	{
		// Set up is called for every test
		m_odbcInfo = GetParam();
		RecordProperty("DSN", eli::w2mb(m_odbcInfo.m_dsn));
		m_pDbEnv = new DbEnvironment(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password);
		HENV henv = m_pDbEnv->GetHenv();
		ASSERT_TRUE(henv  != 0);
		m_pDb = new Database(m_pDbEnv);
		ASSERT_TRUE(m_pDb->Open(m_pDbEnv));
	}

	void DbTest::TearDown()
	{
		if(m_pDb)
		{
			// TODO: Why do we need to commit with DB2? We did not start anything??
			m_pDb->CommitTrans();

			m_pDb->Close();
			delete m_pDb;
		}

		if(m_pDbEnv)
			delete m_pDbEnv;

		m_pDbEnv = NULL;
		m_pDb = NULL;
	}


	TEST_P(DbTest, OpenFromEnv)
	{
		Database db(m_pDbEnv);

		EXPECT_TRUE(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		db.Close();
//		int p = 3;
	}

	// TODO: Test Close. Close should return a value if succeeded
	TEST_P(DbTest, Close)
	{
		// Try to close a db that really is open
		Database db1(m_pDbEnv);
		ASSERT_TRUE(db1.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		EXPECT_TRUE(db1.Close());

		// TODO: Close works okay, does not return false if there is nothing to do
		//// And one that is not open
		//Database db2(m_pDbEnv);
		//EXPECT_FALSE(db2.Close());
	}

	TEST_P(DbTest, ReadDataTypesInfo)
	{
		Database db(m_pDbEnv);

		ASSERT_TRUE(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		std::vector<SSqlTypeInfo> types;
		bool ok = db.ReadDataTypesInfo(types);
		EXPECT_TRUE(ok);
		EXPECT_TRUE(types.size() > 0);

		std::wstringstream ws;
		ws << L"TypeInfo of database with DSN '" << m_odbcInfo.m_dsn << L"', total " << types.size() << L" types reported:" << std::endl;
		bool first = true;
		std::vector<SSqlTypeInfo>::const_iterator it = types.begin();
		while(it != types.end())
		{
			SSqlTypeInfo t = *it;

			++it;

			ws << t.ToOneLineStr(first, it == types.end()) << std::endl;
			if(first)
				first = false;

		}

		BOOST_LOG_TRIVIAL(info) << ws.str();

		db.Close();
	}

	TEST_P(DbTest, GetDbInfo)
	{
		Database db(m_pDbEnv);

		EXPECT_TRUE(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		SDbInfo dbInfo = db.GetDbInfo();
		std::wstring sInfo = dbInfo.ToStr();

		EXPECT_TRUE(sInfo.length() > 0);
		BOOST_LOG_TRIVIAL(info) << L"DbInfo of database connected to DSN " << m_odbcInfo.m_dsn << std::endl << sInfo;

		db.Close();

	}

	TEST_P(DbTest, Open)
	{
		HENV henv = m_pDbEnv->GetHenv();
		ASSERT_TRUE(henv  != 0);

		Database* pDb = new Database(m_pDbEnv);

		// Open without failing on unsupported datatypes
		EXPECT_TRUE(pDb->Open(m_pDbEnv));
		pDb->Close();
		delete pDb;

		// Try to open with a different password / user, expect to fail when opening the db.
		// TODO: Getting the HENV never fails? Add some tests for the HENV
		DbEnvironment* pFailEnv = new DbEnvironment(L"ThisDNSDoesNotExist", L"NorTheUser", L"WithThisPassword");
		EXPECT_TRUE(pFailEnv->GetHenv() != NULL);
		Database* pFailDb = new Database(pFailEnv);
		BOOST_LOG_TRIVIAL(warning) << L"This test is supposed to spit warnings";
		EXPECT_FALSE(pFailDb->Open(pFailEnv));
		pFailDb->Close();
		delete pFailDb;
		delete pFailEnv;

		// Open with failing on unsupported datatypes
		// TODO: This test is stupid, we should also test that we fail
		pDb = new Database(m_pDbEnv);
		EXPECT_TRUE(pDb->Open(m_pDbEnv));
		pDb->Close();
		delete pDb;
	}


	TEST_P(DbTest, DetectDbms)
	{
		// We just know how we name the different odbc-sources
		// TODO: This is not nice, but is there any other reliable way? Add to doc somewhere
		m_odbcInfo = GetParam();
		if(boost::algorithm::find_first(m_odbcInfo.m_dsn, L"DB2"))
		{
			EXPECT_TRUE(m_pDb->Dbms() == dbmsDB2);
		}
		else if(boost::algorithm::find_first(m_odbcInfo.m_dsn, L"MySql"))
		{
			EXPECT_TRUE(m_pDb->Dbms() == dbmsMY_SQL);
		}
		else
		{
			EXPECT_EQ(std::wstring(L"Unknown DSN name"), m_odbcInfo.m_dsn);
		}
	}


	TEST_P(DbTest, TestCommitTransaction)
	{
		IntTypesTmpTable* pTable = new IntTypesTmpTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM wxodbc3.integertypes_tmp WHERE idintegertypes_tmp >= 0";
		ASSERT_TRUE( m_pDb->ExecSql(sqlstmt) );
		ASSERT_TRUE( m_pDb->CommitTrans() );

		ASSERT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp"));
		ASSERT_FALSE( pTable->GetNext());

		sqlstmt = L"INSERT INTO wxodbc3.integertypes_tmp (idintegertypes_tmp, tsmallint, tint, tbigint) VALUES (1, -32768, -2147483648, -9223372036854775808)";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		// Test: If we do not commit we still have zero records
		// TOOD: Why do we get the records here? We need to read more about autocommit and stuff like that, but lets fix the tables first
		//ASSERT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp"));
		//EXPECT_FALSE( pTable->GetNext());
		// Once we commit we have one record
		ASSERT_TRUE( m_pDb->CommitTrans() );
		ASSERT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp"));
		EXPECT_TRUE( pTable->GetNext());

		delete pTable;
	}

	TEST_P(DbTest, TestRollbackTransaction)
	{
		IntTypesTmpTable* pTable = new IntTypesTmpTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM wxodbc3.integertypes_tmp WHERE idintegertypes_tmp >= 0";
		ASSERT_TRUE( m_pDb->ExecSql(sqlstmt) );
		ASSERT_TRUE( m_pDb->CommitTrans() );

		ASSERT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp"));
		ASSERT_FALSE( pTable->GetNext());

		sqlstmt = L"INSERT INTO wxodbc3.integertypes_tmp (idintegertypes_tmp, tsmallint, tint, tbigint) VALUES (1, -32768, -2147483648, -9223372036854775808)";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		// We rollback and expect no record
		ASSERT_TRUE( m_pDb->RollbackTrans() );
		ASSERT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp"));
		EXPECT_FALSE( pTable->GetNext());

		delete pTable;
	}


	TEST_P(DbTest, ReadCatalogs)
	{
		std::vector<std::wstring> cats;
		EXPECT_TRUE(m_pDb->ReadCatalogs(cats));
		if(m_pDb->Dbms() == dbmsDB2)
		{
			// DB2 does not support catalogs. it reports zero catalogs
			EXPECT_EQ(0, cats.size());
		}
		else if(m_pDb->Dbms() == dbmsMY_SQL)
		{
			// MySQL reports our test-db as a catalog
			EXPECT_TRUE(std::find(cats.begin(), cats.end(), L"wxodbc3") != cats.end());
		}
	}

	TEST_P(DbTest, ReadSchemas)
	{
		std::vector<std::wstring> schemas;
		EXPECT_TRUE(m_pDb->ReadSchemas(schemas));
		if(m_pDb->Dbms() == dbmsDB2)
		{
			// DB2 has a wonderful correct schema for our test-db
			EXPECT_TRUE(schemas.size() > 0);
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"WXODBC3") != schemas.end());
		}
		else if(m_pDb->Dbms() == dbmsMY_SQL)
		{
			// MySQL reports exactly one schema with an empty name
			EXPECT_TRUE(schemas.size() == 1);
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"") != schemas.end());
		}

		int p = 3;
	}

	TEST_P(DbTest, ReadTableTypes)
	{
		std::vector<std::wstring> tableTypes;
		EXPECT_TRUE(m_pDb->ReadTableTypes(tableTypes));
		// Check that we have at least a type TABLE and a type VIEW
		EXPECT_TRUE(std::find(tableTypes.begin(), tableTypes.end(), L"TABLE") != tableTypes.end());
		EXPECT_TRUE(std::find(tableTypes.begin(), tableTypes.end(), L"VIEW") != tableTypes.end());
	}

	TEST_P(DbTest, FindTables)
	{
		std::vector<DbCatalogTable> tables;
		std::wstring tableName;
		std::wstring schemaName;
		std::wstring catalogName;
		if(m_pDb->Dbms() == dbmsMY_SQL)
		{
			// We know that mySql uses catalogs, not schemas:
			tableName = L"integertypes";
			schemaName = L"";
			catalogName = L"wxodbc3";
		}
		else if(m_pDb->Dbms() == dbmsDB2)
		{
			// We know that DB2 uses schemas:
			tableName = L"INTEGERTYPES";
			schemaName = L"WXODBC3";
			catalogName = L"";
		}
		// Find one table by using only the table-name as search param
		EXPECT_TRUE(m_pDb->FindTables(tableName, L"", L"", L"", tables));
		EXPECT_EQ(1, tables.size());
		// Find one table by using table-name and schema/catalog
		EXPECT_TRUE(m_pDb->FindTables(tableName, schemaName, catalogName, L"", tables));
		EXPECT_EQ(1, tables.size());
		// In all cases, we should not find anything if we use a schema or a catalog that does not exist
		// Note: When using MySQL-Odbc driver, if 'do not use INFORMATION_SCHEMA' is set, this will fail due to an access denied for database "wrongCatalog"
		EXPECT_TRUE(m_pDb->FindTables(tableName, L"WrongSchema", L"", L"", tables));
		EXPECT_EQ(0, tables.size());
		EXPECT_TRUE(m_pDb->FindTables(tableName, L"", L"WrongCatalog", L"", tables));
		EXPECT_EQ(0, tables.size());
		// Also, we have a table, not a view
		EXPECT_TRUE(m_pDb->FindTables(tableName, L"", L"", L"VIEW", tables));
		EXPECT_EQ(0, tables.size());
		EXPECT_TRUE(m_pDb->FindTables(tableName, L"", L"", L"TABLE", tables));
		EXPECT_EQ(1, tables.size());
		// What about search-patterns? Note: Not use for table-type
		// TODO: They just dont work with MySql 3.51 ?
		EXPECT_TRUE(m_pDb->FindTables(tableName, L"%", L"%", L"", tables));
		EXPECT_EQ(1, tables.size());
		if(tables.size() > 0)
		{
			EXPECT_EQ(tableName, tables[0].m_tableName);
			EXPECT_EQ(schemaName, tables[0].m_schema);
			EXPECT_EQ(catalogName, tables[0].m_catalog);
		}
		std::wstring schemaPattern = schemaName;
		if(schemaName.length() > 0)
			schemaPattern = (boost::wformat(L"%%%s%%") %schemaName).str();
		std::wstring catalogPattern = catalogName;
		if(catalogName.length() > 0)
			catalogPattern = (boost::wformat(L"%%%s%%") %catalogName).str();
		EXPECT_TRUE(m_pDb->FindTables(tableName, schemaPattern, catalogPattern, L"", tables));
		EXPECT_EQ(1, tables.size());
		if(tables.size() > 0)
		{
			EXPECT_EQ(tableName, tables[0].m_tableName);
			EXPECT_EQ(schemaName, tables[0].m_schema);
			EXPECT_EQ(catalogName, tables[0].m_catalog);
		}
	}

	TEST_P(DbTest, ReadCompleteCatalog)
	{
		SDbCatalog cat;
		EXPECT_TRUE(m_pDb->ReadCompleteCatalog(cat));
		// TODO: This is confusing. DB2 reports schemas, what is correct, but mysql reports catalogs?? wtf?
		if(m_pDb->Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(cat.m_schemas.find(L"WXODBC3") != cat.m_schemas.end());
		}
		else if(m_pDb->Dbms() == dbmsMY_SQL)
		{
			EXPECT_TRUE(cat.m_catalogs.find(L"wxodbc3") != cat.m_catalogs.end());
		}
		EXPECT_TRUE(cat.m_tables.size() >= 12);
	}

	TEST_P(DbTest, ReadColumnCount)
	{
		std::wstring tableName = L"";
		std::wstring schemaName = L"";
		std::wstring catalogName = L"";
		int nrCols = 0;
		if(m_pDb->Dbms() == dbmsDB2)
		{
			// DB2 has schemas
			tableName = L"INTEGERTYPES";
			schemaName = L"WXODBC3";
			nrCols = 4;
		}
		else if(m_pDb->Dbms() == dbmsMY_SQL)
		{
			// mysql has catalogs
			tableName = L"integertypes";
			catalogName = L"wxodbc3";
			nrCols = 7;
		}
		EXPECT_EQ(nrCols, m_pDb->ReadColumnCount(tableName, schemaName, catalogName));
		// we should also work if we just search by the tableName, as long as tableName is unique within db
		EXPECT_EQ(nrCols, m_pDb->ReadColumnCount(tableName, L"", L""));
	}

	TEST_P(DbTest, ExecSql_InsertCharTypes)
	{
		CharTypesTmpTable* pTable = new CharTypesTmpTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		std::wstring sqlstmt = L"DELETE FROM wxodbc3.chartypes_tmp WHERE idchartypes_tmp >= 0";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		// Note the escaping:
		// IBM DB2 wants to escape ' using '', mysql wants \'
		// MYSQL needs \\ for \ 
		RecordProperty("Ticket", 36);
		if(m_pDb->Dbms() == dbmsDB2)
		{
			sqlstmt = (boost::wformat(L"INSERT INTO wxodbc3.chartypes_tmp (idchartypes_tmp, tvarchar, tchar) VALUES (1, '%s', '%s')") % L" !\"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~" % L" !\"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~").str();
		}
		else
		{
			sqlstmt = (boost::wformat(L"INSERT INTO wxodbc3.chartypes_tmp (idchartypes_tmp, tvarchar, tchar) VALUES (1, '%s', '%s')") % L" !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~" % L" !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~").str();
		}
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		// And note the triming
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes_tmp ORDER BY idchartypes_tmp ASC"));
		EXPECT_TRUE( pTable->GetNext());
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), std::wstring(pTable->m_varchar));
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), boost::trim_right_copy(std::wstring(pTable->m_char)));
		EXPECT_FALSE(pTable->GetNext());

		delete pTable;
	}

	TEST_P(DbTest, ExecSQL_InsertFloatTypes)
	{
		FloatTypesTmpTable* pTable = new FloatTypesTmpTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		std::wstring sqlstmt = L"DELETE FROM wxodbc3.floattypes_tmp WHERE idfloattypes_tmp >= 0";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		sqlstmt = L"INSERT INTO wxodbc3.floattypes_tmp (idfloattypes_tmp, tdouble, tfloat) VALUES (1, -3.141592, -3.141)";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes_tmp WHERE idfloattypes_tmp = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( -3.141592, pTable->m_double);
		EXPECT_EQ( -3.141, pTable->m_float);
		EXPECT_FALSE( pTable->GetNext() );

		sqlstmt = L"INSERT INTO wxodbc3.floattypes_tmp (idfloattypes_tmp, tdouble, tfloat) VALUES (2, 3.141592, 3.141)";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes_tmp WHERE idfloattypes_tmp = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 3.141592, pTable->m_double);
		EXPECT_EQ( 3.141, pTable->m_float);
		EXPECT_FALSE( pTable->GetNext() );


		delete pTable;
	}


	TEST_P(DbTest, ExecSQL_InsertNumericTypesAsChar)
	{
		NumericTypesTmpTable* pTable = new NumericTypesTmpTable(m_pDb, NumericTypesTmpTable::ReadAsChar);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM wxodbc3.numerictypes_tmp WHERE idnumerictypes_tmp >= 0";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		sqlstmt = L"INSERT INTO wxodbc3.numerictypes_tmp (idnumerictypes_tmp, tdecimal_18_0, tdecimal_18_10) VALUES (1, -123456789012345678, -12345678.9012345678)";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		// TODO: DB2 sends a ',', mysql sends a '.' as delimeter
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes_tmp WHERE idnumerictypes_tmp = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( std::wstring(L"-123456789012345678"), std::wstring(pTable->m_wcdecimal_18_0));

		RecordProperty("Ticket", 35);
		if(m_pDb->Dbms() == dbmsDB2)
			EXPECT_EQ( std::wstring(L"-12345678,9012345678"), std::wstring(pTable->m_wcdecimal_18_10));	
		else
			EXPECT_EQ( std::wstring(L"-12345678.9012345678"), std::wstring(pTable->m_wcdecimal_18_10));	

		sqlstmt = L"INSERT INTO wxodbc3.numerictypes_tmp (idnumerictypes_tmp, tdecimal_18_0, tdecimal_18_10) VALUES (2, 123456789012345678, 12345678.9012345678)";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		// TODO: DB2 sends a ',', mysql sends a '.' as delimeter
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes_tmp WHERE idnumerictypes_tmp = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( std::wstring(L"123456789012345678"), std::wstring(pTable->m_wcdecimal_18_0));

		if(m_pDb->Dbms() == dbmsDB2)
			EXPECT_EQ( std::wstring(L"12345678,9012345678"), std::wstring(pTable->m_wcdecimal_18_10));	
		else
			EXPECT_EQ( std::wstring(L"12345678.9012345678"), std::wstring(pTable->m_wcdecimal_18_10));	

		sqlstmt = L"INSERT INTO wxodbc3.numerictypes_tmp (idnumerictypes_tmp, tdecimal_18_0, tdecimal_18_10) VALUES (3, 0, 0.0000000000)";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		// TODO: DB2 sends a ',', mysql sends a '.' as delimeter
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes_tmp WHERE idnumerictypes_tmp = 3"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( std::wstring(L"0"), std::wstring(pTable->m_wcdecimal_18_0));
		
		if(m_pDb->Dbms() == dbmsDB2)
			EXPECT_EQ( std::wstring(L"0,0000000000"), std::wstring(pTable->m_wcdecimal_18_10));	
		else
			EXPECT_EQ( std::wstring(L"0.0000000000"), std::wstring(pTable->m_wcdecimal_18_10));	

		delete pTable;
	}


	TEST_P(DbTest, ExecSQL_InsertIntTypes)
	{
		IntTypesTmpTable* pTable = new IntTypesTmpTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM wxodbc3.integertypes_tmp WHERE idintegertypes_tmp >= 0";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp"));
		EXPECT_FALSE( pTable->GetNext());

		sqlstmt = L"INSERT INTO wxodbc3.integertypes_tmp (idintegertypes_tmp, tsmallint, tint, tbigint) VALUES (1, -32768, -2147483648, -9223372036854775808)";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp ORDER BY idintegertypes_tmp ASC"));
		EXPECT_TRUE( pTable->GetNext());
		EXPECT_EQ( -32768, pTable->m_smallInt);
		EXPECT_EQ( INT_MIN, pTable->m_int);
		EXPECT_EQ( -LLONG_MIN, pTable->m_bigInt);
		EXPECT_FALSE( pTable->GetNext());

		sqlstmt = L"INSERT INTO wxodbc3.integertypes_tmp (idintegertypes_tmp, tsmallint, tint, tbigint) VALUES (2, 32767, 2147483647, 9223372036854775807)";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp ORDER BY idintegertypes_tmp ASC"));
		EXPECT_TRUE( pTable->GetNext());
		EXPECT_TRUE( pTable->GetNext());
		EXPECT_EQ( 32767, pTable->m_smallInt);
		EXPECT_EQ( 2147483647, pTable->m_int);
		EXPECT_EQ( 9223372036854775807, pTable->m_bigInt);
		EXPECT_FALSE( pTable->GetNext());

		// IBM DB2 has no support for unsigned int types, so far we only know about mysql
		RecordProperty("Ticket", 33);
		if(m_pDb->Dbms() == dbmsMY_SQL)
		{
			sqlstmt = L"INSERT INTO wxodbc3.integertypes_tmp (idintegertypes_tmp, tusmallint, tuint, tubigint) VALUES (4, 0, 0, 0)";
			EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
			EXPECT_TRUE( m_pDb->CommitTrans() );
			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp WHERE idintegertypes_tmp = 4"));
			EXPECT_TRUE( pTable->GetNext());
			EXPECT_EQ( 0, pTable->m_usmallInt);
			EXPECT_EQ( 0, pTable->m_uint);
			EXPECT_EQ( 0, pTable->m_ubigInt);
			EXPECT_FALSE( pTable->GetNext());

			sqlstmt = L"INSERT INTO wxodbc3.integertypes_tmp (idintegertypes_tmp, tusmallint, tuint, tubigint) VALUES (5, 65535, 4294967295, 18446744073709551615)";
			EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
			EXPECT_TRUE( m_pDb->CommitTrans() );
			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp  WHERE idintegertypes_tmp = 5"));
			EXPECT_TRUE( pTable->GetNext());
			EXPECT_EQ( 65535, pTable->m_usmallInt);
			EXPECT_EQ( 4294967295, pTable->m_uint);
			// With the 5.2 odbc driver ubigint seems to be wrong?
			// TODO: This is ugly, use some kind of disabled test, or log a warning..
			if(std::wstring(m_pDb->GetDriverVersion()) != std::wstring(L"05.02.0006"))
			{
				EXPECT_EQ( 18446744073709551615, pTable->m_ubigInt);
			}
			EXPECT_FALSE( pTable->GetNext());
		}

		delete pTable;
	}

	TEST_P(DbTest, ExecSQL_InsertDateTypes)
	{
		DateTypesTmpTable* pTable = new DateTypesTmpTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM wxodbc3.datetypes_tmp WHERE iddatetypes_tmp >= 0";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		sqlstmt = L"INSERT INTO wxodbc3.datetypes_tmp (iddatetypes_tmp, tdate, ttime, ttimestamp) VALUES (1, '1983-01-26', '13:55:56', '1983-01-26 13:55:56')";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.datetypes_tmp WHERE iddatetypes_tmp = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 26, pTable->m_date.day);
		EXPECT_EQ( 01, pTable->m_date.month);
		EXPECT_EQ( 1983, pTable->m_date.year);

		EXPECT_EQ( 13, pTable->m_time.hour);
		EXPECT_EQ( 55, pTable->m_time.minute);
		EXPECT_EQ( 56, pTable->m_time.second);

		EXPECT_EQ( 26, pTable->m_timestamp.day);
		EXPECT_EQ( 01, pTable->m_timestamp.month);
		EXPECT_EQ( 1983, pTable->m_timestamp.year);
		EXPECT_EQ( 13, pTable->m_timestamp.hour);
		EXPECT_EQ( 55, pTable->m_timestamp.minute);
		EXPECT_EQ( 56, pTable->m_timestamp.second);

		EXPECT_FALSE( pTable->GetNext() );

		delete pTable;
	}

	// Interfaces
	// ----------

} //namespace exodbc
