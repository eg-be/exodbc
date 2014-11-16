/*!
 * \file DbTest.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "DatabaseTest.h"

// Same component headers
#include "GenericTestTables.h"

// Other headers
#include "Environment.h"
#include "Database.h"
#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"

// Debug
#include "DebugNew.h"

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

	void DatabaseTest::SetUp()
	{
		// Set up is called for every test
		m_odbcInfo = GetParam();
//		RecordProperty("DSN", eli::w2mb(m_odbcInfo.m_dsn));
		ASSERT_TRUE(m_env.AllocHenv());
		ASSERT_TRUE(m_env.SetOdbcVersion(OV_3));
		ASSERT_TRUE(m_db.AllocateHdbc(m_env));
		ASSERT_TRUE(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
	}

	void DatabaseTest::TearDown()
	{
		if(m_db.IsOpen())
		{
			// TODO: Why do we need to commit with most databases? We did not start anything??
			// Probably because of we did not commit after one of the select statements
			EXPECT_TRUE(m_db.CommitTrans());
			EXPECT_TRUE(m_db.Close());
		}
	}


	TEST_P(DatabaseTest, OpenFromEnv)
	{
		Database db(m_env);

		EXPECT_TRUE(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		// only ms sql server needs a commit trans here so far?
		if(db.Dbms() == dbmsMS_SQL_SERVER)
		{
			db.CommitTrans();
		}
		
		db.Close();
	}

	// TODO: Test Close. Close should return a value if succeeded
	TEST_P(DatabaseTest, Close)
	{
		// Try to close a db that really is open
		Database db1(m_env);
		ASSERT_TRUE(db1.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		// only ms sql server needs a commit trans here so far?
		if(db1.Dbms() == dbmsMS_SQL_SERVER)
		{
			db1.CommitTrans();
		}

		EXPECT_TRUE(db1.Close());

		// TODO: Close works okay, does not return false if there is nothing to do
		//// And one that is not open
		//Database db2(m_pDbEnv);
		//EXPECT_FALSE(db2.Close());
	}

	TEST_P(DatabaseTest, ReadTransactionMode)
	{
		// We default to manual commit
		EXPECT_EQ(TM_MANUAL_COMMIT, m_db.ReadTransactionMode());
	}

	TEST_P(DatabaseTest, SetTransactionMode)
	{
		Database db(m_env);
		ASSERT_TRUE(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		if(db.Dbms() == dbmsMS_SQL_SERVER)
		{
			LOG_WARNING(L"This test is supposed to spit warning if connected against MS Sql Server and Ticket #51 is not fixed yet");
		}

		// We default to manual commit
		EXPECT_EQ(TM_MANUAL_COMMIT, db.GetTransactionMode());
		EXPECT_EQ(TM_MANUAL_COMMIT, db.ReadTransactionMode());

		// Switch to auto
		EXPECT_TRUE(db.SetTransactionMode(TM_AUTO_COMMIT));
		// internal member should already be updated
		EXPECT_EQ(TM_AUTO_COMMIT, db.GetTransactionMode());
		EXPECT_EQ(TM_AUTO_COMMIT, db.ReadTransactionMode());

		// and back to manual
		EXPECT_TRUE(db.SetTransactionMode(TM_MANUAL_COMMIT));
		// internal member should already be updated
		EXPECT_EQ(TM_MANUAL_COMMIT, db.GetTransactionMode());
		EXPECT_EQ(TM_MANUAL_COMMIT, db.ReadTransactionMode());

		// Close db
		db.Close();
	}

	TEST_P(DatabaseTest, ReadDataTypesInfo)
	{
		Database db(m_env);

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

		// TODO: needed for ms only (?)
		if(db.Dbms() == dbmsMS_SQL_SERVER)
		{
			db.CommitTrans();
		}

		db.Close();
	}

	TEST_P(DatabaseTest, GetDbInfo)
	{
		Database db(m_env);

		EXPECT_TRUE(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		SDbInfo dbInfo = db.GetDbInfo();
		std::wstring sInfo = dbInfo.ToStr();

		EXPECT_TRUE(sInfo.length() > 0);
		BOOST_LOG_TRIVIAL(info) << L"DbInfo of database connected to DSN " << m_odbcInfo.m_dsn.c_str() << std::endl << sInfo.c_str();

		// TODO: needed for ms only (?)
		if(db.Dbms() == dbmsMS_SQL_SERVER)
		{
			db.CommitTrans();
		}

		db.Close();

	}

	TEST_P(DatabaseTest, Open)
	{		
		// Open an existing db by passing the Env to the ctor and reading the params from the Environment (the old wx-way)
		Environment env(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password, OV_3);
		EXPECT_TRUE(env.HaveHenv());
		Database db(env);
		EXPECT_TRUE(db.Open(&env));
		// only ms sql server needs a commit trans here so far (before closing)?
		if(db.Dbms() == dbmsMS_SQL_SERVER)
		{
			db.CommitTrans();
		}
		db.Close();

		// Open an existing db using the default c'tor and setting params on db
		Database db2;
		EXPECT_TRUE(db2.AllocateHdbc(env));
		EXPECT_TRUE(db2.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		// only ms sql server needs a commit trans here so far (before closing)?
		if (db2.Dbms() == dbmsMS_SQL_SERVER)
		{
			db2.CommitTrans();
		}
		db2.Close();

		// Try to open with a different password / user, expect to fail when opening the db.
		Database failDb(m_env);
		BOOST_LOG_TRIVIAL(warning) << L"This test is supposed to spit errors";
		EXPECT_FALSE(failDb.Open(L"ThisDNSDoesNotExist", L"NorTheUser", L"WithThisPassword"));
	}


	TEST_P(DatabaseTest, DetectDbms)
	{
		// We just know how we name the different odbc-sources
		// TODO: This is not nice, but is there any other reliable way? Add to doc somewhere
		m_odbcInfo = GetParam();
		if(boost::algorithm::find_first(m_odbcInfo.m_dsn, L"DB2"))
		{
			EXPECT_TRUE(m_db.Dbms() == dbmsDB2);
		}
		else if(boost::algorithm::find_first(m_odbcInfo.m_dsn, L"MySql"))
		{
			EXPECT_TRUE(m_db.Dbms() == dbmsMY_SQL);
		}
		else if(boost::algorithm::find_first(m_odbcInfo.m_dsn, L"SqlServer"))
		{
			EXPECT_TRUE(m_db.Dbms() == dbmsMS_SQL_SERVER);
		}
		else
		{
			EXPECT_EQ(std::wstring(L"Unknown DSN name"), m_odbcInfo.m_dsn);
		}
	}


	TEST_P(DatabaseTest, TestCommitTransaction)
	{
		IntTypesTmpTable* pTable = new IntTypesTmpTable(&m_db);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM exodbc.integertypes_tmp WHERE idintegertypes_tmp >= 0";
		EXPECT_TRUE( m_db.ExecSql(sqlstmt) );
		EXPECT_TRUE( m_db.CommitTrans() );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM exodbc.integertypes_tmp"));
		EXPECT_FALSE( pTable->GetNext());

		sqlstmt = L"INSERT INTO exodbc.integertypes_tmp (idintegertypes_tmp, tsmallint, tint, tbigint) VALUES (1, -32768, -2147483648, -9223372036854775808)";
		EXPECT_TRUE( m_db.ExecSql(sqlstmt) );
		// Test: If we do not commit we still have zero records
		// TOOD: Why do we get the records here? We need to read more about autocommit and stuff like that, but lets fix the tables first
		//ASSERT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM exodbc.integertypes_tmp"));
		//EXPECT_FALSE( pTable->GetNext());
		// Once we commit we have one record
		EXPECT_TRUE( m_db.CommitTrans() );
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM exodbc.integertypes_tmp"));
		EXPECT_TRUE( pTable->GetNext());

		delete pTable;
	}

	TEST_P(DatabaseTest, TestRollbackTransaction)
	{
		IntTypesTmpTable* pTable = new IntTypesTmpTable(&m_db);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM exodbc.integertypes_tmp WHERE idintegertypes_tmp >= 0";
		EXPECT_TRUE( m_db.ExecSql(sqlstmt) );
		EXPECT_TRUE( m_db.CommitTrans() );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM exodbc.integertypes_tmp"));
		EXPECT_FALSE( pTable->GetNext());

		sqlstmt = L"INSERT INTO exodbc.integertypes_tmp (idintegertypes_tmp, tsmallint, tint, tbigint) VALUES (1, -32768, -2147483648, -9223372036854775808)";
		EXPECT_TRUE( m_db.ExecSql(sqlstmt) );
		// We rollback and expect no record
		EXPECT_TRUE( m_db.RollbackTrans() );
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM exodbc.integertypes_tmp"));
		EXPECT_FALSE( pTable->GetNext());

		delete pTable;
	}


	TEST_P(DatabaseTest, ReadCatalogs)
	{
		std::vector<std::wstring> cats;
		EXPECT_TRUE(m_db.ReadCatalogs(cats));
		switch(m_db.Dbms())
		{
		case dbmsDB2:
			// DB2 does not support catalogs. it reports zero catalogs
			EXPECT_EQ(0, cats.size());
			break;

		case dbmsMS_SQL_SERVER:
		case dbmsMY_SQL:
			// Those report our test-db as a catalog
			EXPECT_TRUE(std::find(cats.begin(), cats.end(), L"exodbc") != cats.end());
			break;
		}
	}

	TEST_P(DatabaseTest, ReadSchemas)
	{
		std::vector<std::wstring> schemas;
		EXPECT_TRUE(m_db.ReadSchemas(schemas));
		switch(m_db.Dbms())
		{
		case dbmsDB2:
			// DB2 reports our test-db as a schema
			EXPECT_TRUE(schemas.size() > 0);
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"EXODBC") != schemas.end());
			break;
		case dbmsMY_SQL:
			// Mysql reports one schema with an empty name
			EXPECT_TRUE(schemas.size() == 1);
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"") != schemas.end());
			break;
		case dbmsMS_SQL_SERVER:
			// ms sql server reported at least 3 schemas:
			EXPECT_TRUE(schemas.size() >= 3);
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"exodbc") != schemas.end());
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"INFORMATION_SCHEMA") != schemas.end());
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"sys") != schemas.end());
			break;
		}
	}

	TEST_P(DatabaseTest, ReadTableTypes)
	{
		std::vector<std::wstring> tableTypes;
		EXPECT_TRUE(m_db.ReadTableTypes(tableTypes));
		// Check that we have at least a type TABLE and a type VIEW
		EXPECT_TRUE(std::find(tableTypes.begin(), tableTypes.end(), L"TABLE") != tableTypes.end());
		EXPECT_TRUE(std::find(tableTypes.begin(), tableTypes.end(), L"VIEW") != tableTypes.end());
	}

	TEST_P(DatabaseTest, ReadTablePrivileges)
	{
		std::vector<STablePrivilegesInfo> privs;
		std::wstring tableName;
		std::wstring schemaName;
		std::wstring catalogName;
		switch(m_db.Dbms())
		{
		case dbmsMY_SQL:
			// We know that mySql uses catalogs, not schemas:
			tableName = L"integertypes";
			schemaName = L"";
			catalogName = L"exodbc";
			break;
		case dbmsDB2:
			// We know that DB2 uses schemas:
			tableName = L"INTEGERTYPES";
			schemaName = L"EXODBC";
			catalogName = L"";
			break;
		case dbmsMS_SQL_SERVER:
			// And ms uses catalogs, which map to dbs and schemaName
			tableName = L"integertypes";
			schemaName = L"exodbc";
			catalogName = L"exodbc";
			LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #52");
		}
		// TODO: With MySql we get no results here?
		EXPECT_TRUE(m_db.ReadTablePrivileges(tableName, schemaName, catalogName, privs));
		bool canSelect = false;
		bool canInsert = false;
		bool canDelete = false;
		bool canUpdate = false;
		std::vector<STablePrivilegesInfo>::const_iterator it;
		for(it = privs.begin(); it != privs.end(); it++)
		{
			const STablePrivilegesInfo& priv = *it;
			if(priv.m_privilege == L"SELECT")
				canSelect = true;
			if(priv.m_privilege == L"INSERT")
				canInsert = true;
			if(priv.m_privilege == L"DELETE")
				canDelete = true;
			if(priv.m_privilege == L"UPDATE")
				canUpdate = true;
		}
		EXPECT_TRUE(canSelect);
		EXPECT_TRUE(canInsert);
		EXPECT_TRUE(canUpdate);
		EXPECT_TRUE(canDelete);
	}


	TEST_P(DatabaseTest, ReadTableColumnInfo)
	{
		std::vector<STableColumnInfo> cols;
		std::wstring tableName;
		std::wstring schemaName;
		std::wstring catalogName;
		switch(m_db.Dbms())
		{
		case dbmsMY_SQL:
			// We know that mySql uses catalogs, not schemas:
			tableName = L"numerictypes";
			schemaName = L"";
			catalogName = L"exodbc";
			break;
		case dbmsDB2:
			// We know that DB2 uses schemas:
			tableName = L"NUMERICTYPES";
			schemaName = L"EXODBC";
			catalogName = L"";
			break;
		case dbmsMS_SQL_SERVER:
			// And ms uses catalogs, which map to dbs and schemaName
			tableName = L"numerictypes";
			schemaName = L"exodbc";
			catalogName = L"exodbc";
		}
		EXPECT_TRUE(m_db.ReadTableColumnInfo(tableName, schemaName, catalogName, cols));
		// Our decimals columns must have a num prec radix value of 10, a column size of the total digits, and a decimal digits the nr of digits after the delimeter
		ASSERT_TRUE(cols.size() == 3);
		STableColumnInfo col = cols[2];
		EXPECT_FALSE(col.m_isNumPrecRadixNull);
		EXPECT_FALSE(col.m_isColumnSizeNull);
		EXPECT_FALSE(col.m_isDecimalDigitsNull);
		EXPECT_EQ(10, col.m_numPrecRadix);
		EXPECT_EQ(18, col.m_columnSize);
		EXPECT_EQ(10, col.m_decimalDigits);
	}

	TEST_P(DatabaseTest, FindTables)
	{
		std::vector<STableInfo> tables;
		std::wstring tableName;
		std::wstring schemaName;
		std::wstring catalogName;
		switch(m_db.Dbms())
		{
		case dbmsMY_SQL:
			// We know that mySql uses catalogs, not schemas:
			tableName = L"integertypes";
			schemaName = L"";
			catalogName = L"exodbc";
			break;
		case dbmsDB2:
			// We know that DB2 uses schemas:
			tableName = L"INTEGERTYPES";
			schemaName = L"EXODBC";
			catalogName = L"";
			break;
		case dbmsMS_SQL_SERVER:
			// And ms uses catalogs, which map to dbs and schemaName
			tableName = L"integertypes";
			schemaName = L"exodbc";
			catalogName = L"exodbc";
		}
		// Find one table by using only the table-name as search param
		EXPECT_TRUE(m_db.FindTables(tableName, L"", L"", L"", tables));
		EXPECT_EQ(1, tables.size());
		// Find one table by using table-name and schema/catalog
		EXPECT_TRUE(m_db.FindTables(tableName, schemaName, catalogName, L"", tables));
		EXPECT_EQ(1, tables.size());
		// In all cases, we should not find anything if we use a schema or a catalog that does not exist
		// Note: When using MySQL-Odbc driver, if 'do not use INFORMATION_SCHEMA' is set, this will fail due to an access denied for database "wrongCatalog"
		EXPECT_TRUE(m_db.FindTables(tableName, L"WrongSchema", L"", L"", tables));
		EXPECT_EQ(0, tables.size());
		EXPECT_TRUE(m_db.FindTables(tableName, L"", L"WrongCatalog", L"", tables));
		EXPECT_EQ(0, tables.size());
		// Also, we have a table, not a view
		EXPECT_TRUE(m_db.FindTables(tableName, L"", L"", L"VIEW", tables));
		EXPECT_EQ(0, tables.size());
		EXPECT_TRUE(m_db.FindTables(tableName, L"", L"", L"TABLE", tables));
		EXPECT_EQ(1, tables.size());
		// What about search-patterns? Note: Not use for table-type
		// TODO: They just dont work with MySql 3.51 ?
		EXPECT_TRUE(m_db.FindTables(tableName, L"%", L"%", L"", tables));
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
		EXPECT_TRUE(m_db.FindTables(tableName, schemaPattern, catalogPattern, L"", tables));
		EXPECT_EQ(1, tables.size());
		if(tables.size() > 0)
		{
			EXPECT_EQ(tableName, tables[0].m_tableName);
			EXPECT_EQ(schemaName, tables[0].m_schema);
			EXPECT_EQ(catalogName, tables[0].m_catalog);
		}
	}

	TEST_P(DatabaseTest, ReadCompleteCatalog)
	{
		SDbCatalogInfo cat;
		EXPECT_TRUE(m_db.ReadCompleteCatalog(cat));
		// TODO: This is confusing. DB2 reports schemas, what is correct, but mysql reports catalogs?? wtf?
		if(m_db.Dbms() == dbmsDB2)
		{
			EXPECT_TRUE(cat.m_schemas.find(L"EXODBC") != cat.m_schemas.end());
		}
		else if(m_db.Dbms() == dbmsMY_SQL)
		{
			EXPECT_TRUE(cat.m_catalogs.find(L"exodbc") != cat.m_catalogs.end());
		}
		EXPECT_TRUE(cat.m_tables.size() >= 12);
	}

	TEST_P(DatabaseTest, ReadColumnCount)
	{
		std::wstring tableName = L"";
		std::wstring schemaName = L"";
		std::wstring catalogName = L"";
		int nrCols = 4;
		switch(m_db.Dbms())
		{
		case dbmsDB2:
			// DB2 has schemas
			tableName = L"INTEGERTYPES";
			schemaName = L"EXODBC";
			break;
		case dbmsMY_SQL:
			// mysql has catalogs
			tableName = L"integertypes";
			catalogName = L"exodbc";
			break;
		case dbmsMS_SQL_SERVER:
			// ms has catalogs and schemas
			tableName = L"integertypes";
			catalogName = L"exodbc";
			schemaName = L"exodbc";
			break;
		}
		EXPECT_EQ(nrCols, m_db.ReadColumnCount(tableName, schemaName, catalogName));
		// we should also work if we just search by the tableName, as long as tableName is unique within db
		EXPECT_EQ(nrCols, m_db.ReadColumnCount(tableName, L"", L""));
	}


	// Interfaces
	// ----------

} //namespace exodbc
