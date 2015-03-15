/*!
 * \file DbTest.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * \copyright wxWindows Library Licence, Version 3.1
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "DatabaseTest.h"

// Same component headers
#include "ManualTestTables.h"

// Other headers
#include "Environment.h"
#include "Database.h"
#include "Exception.h"
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
		m_env.AllocateEnvironmentHandle();
		m_env.SetOdbcVersion(OV_3);
		ASSERT_NO_THROW(m_db.AllocateConnectionHandle(m_env));
		ASSERT_NO_THROW(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
	}

	void DatabaseTest::TearDown()
	{
		if(m_db.IsOpen())
		{
			EXPECT_NO_THROW(m_db.Close());
		}
	}

#ifdef EXODBCDEBUG
	// We disable this test to avoid trapping into an assertion 
	TEST_P(DatabaseTest, DISABLED_DetectOrphanedTables)
	{
		Database db(m_env);
		ASSERT_NO_THROW(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		MIntTypesTable t1(&db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(t1.Open());
		{
			MCharTypesTable t2(&db, m_odbcInfo.m_namesCase);
			ASSERT_NO_THROW(t2.Open());
		}
		{
			LogLevelDebug lld;
			LOG_DEBUG(L"You should see an orphaned IntTypesTable table now");
			db.Close();
		}
	}
#endif

	TEST_P(DatabaseTest, SetConnectionAttributes)
	{
		Database db(m_env);

		EXPECT_NO_THROW(db.SetConnectionAttributes());
	}


	TEST_P(DatabaseTest, ReadTransactionIsolationMode)
	{
		TransactionIsolationMode tiMode = m_db.ReadTransactionIsolationMode();
		EXPECT_NE(TI_UNKNOWN, tiMode);
	}

	TEST_P(DatabaseTest, SetTransactionIsolationMode)
	{
		TransactionIsolationMode tiMode = TI_READ_COMMITTED;
		EXPECT_NO_THROW(m_db.SetTransactionIsolationMode(tiMode));
		tiMode = m_db.ReadTransactionIsolationMode();
		EXPECT_EQ(TI_READ_COMMITTED, tiMode);
		EXPECT_NO_THROW(m_db.ExecSql(L"SELECT * FROM exodbc.integertypes"));

		tiMode = TI_READ_UNCOMMITTED;
		EXPECT_NO_THROW(m_db.SetTransactionIsolationMode(tiMode));
		tiMode = m_db.ReadTransactionIsolationMode();
		EXPECT_EQ(TI_READ_UNCOMMITTED, tiMode);
		EXPECT_NO_THROW(m_db.ExecSql(L"SELECT * FROM exodbc.integertypes"));

		tiMode = TI_REPEATABLE_READ;
		EXPECT_NO_THROW(m_db.SetTransactionIsolationMode(tiMode));
		tiMode = m_db.ReadTransactionIsolationMode();
		EXPECT_EQ(TI_REPEATABLE_READ, tiMode);
		EXPECT_NO_THROW(m_db.ExecSql(L"SELECT * FROM exodbc.integertypes"));

		tiMode = TI_SERIALIZABLE;
		EXPECT_NO_THROW(m_db.SetTransactionIsolationMode(tiMode));
		tiMode = m_db.ReadTransactionIsolationMode();
		EXPECT_EQ(TI_SERIALIZABLE, tiMode);
		EXPECT_NO_THROW(m_db.ExecSql(L"SELECT * FROM exodbc.integertypes"));

		if (m_db.Dbms() == dbmsMS_SQL_SERVER)
		{
#if HAVE_MSODBCSQL_H
			tiMode = TI_SNAPSHOT;
			EXPECT_NO_THROW(m_db.SetTransactionIsolationMode(tiMode));
			tiMode = m_db.ReadTransactionIsolationMode();
			EXPECT_EQ(TI_SNAPSHOT, tiMode);
			EXPECT_NO_THROW(m_db.ExecSql(L"SELECT * FROM exodbc.integertypes"));
#else
			LOG_WARNING(L"Skipping test because testing against Ms Sql Server but msodbcsql.h is missing");
			EXPECT_TRUE(false);
#endif
		}
	}

	TEST_P(DatabaseTest, CanSetTransactionIsolationMode)
	{
		EXPECT_TRUE(m_db.CanSetTransactionIsolationMode(TI_READ_COMMITTED));
		EXPECT_TRUE(m_db.CanSetTransactionIsolationMode(TI_READ_UNCOMMITTED));
		EXPECT_TRUE(m_db.CanSetTransactionIsolationMode(TI_REPEATABLE_READ));
		EXPECT_TRUE(m_db.CanSetTransactionIsolationMode(TI_SERIALIZABLE));
		EXPECT_FALSE(m_db.CanSetTransactionIsolationMode(TI_UNKNOWN));
	}


	TEST_P(DatabaseTest, Open)
	{		
		// Open an existing db by passing the Env to the ctor
		Environment env(OV_3);
		ASSERT_TRUE(env.HasEnvironmentHandle());
		Database db(env);
		EXPECT_NO_THROW(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		EXPECT_NO_THROW(db.Close());

		// Open an existing db using the default c'tor and setting params on db
		Database db2;
		ASSERT_NO_THROW(db2.AllocateConnectionHandle(env));
		EXPECT_NO_THROW(db2.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		EXPECT_NO_THROW(db2.Close());

		// Try to open with a different password / user, expect to fail when opening the db.
		{
			Database failDb(m_env);
			EXPECT_THROW(failDb.Open(L"ThisDNSDoesNotExist", L"NorTheUser", L"WithThisPassword"), SqlResultException);
		}
	}


	// TODO: Test Close. Close should return a value if succeeded
	TEST_P(DatabaseTest, Close)
	{
		// Try to close a db that really is open
		Database db1(m_env);
		ASSERT_NO_THROW(db1.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		EXPECT_NO_THROW(db1.Close());

		// \todo: Close works okay, does not return false if there is nothing to do
		//// And one that is not open
		//Database db2(m_pDbEnv);
		//EXPECT_FALSE(db2.Close());
	}

	TEST_P(DatabaseTest, ReadCommitMode)
	{
		// We default to manual commit
		EXPECT_EQ(CM_MANUAL_COMMIT, m_db.ReadCommitMode());
	}

	TEST_P(DatabaseTest, SetCommitMode)
	{
		Database db(m_env);
		ASSERT_NO_THROW(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		// We default to manual commit
		EXPECT_EQ(CM_MANUAL_COMMIT, db.GetCommitMode());
		EXPECT_EQ(CM_MANUAL_COMMIT, db.ReadCommitMode());

		// Switch to auto
		EXPECT_NO_THROW(db.SetCommitMode(CM_AUTO_COMMIT));
		// internal member should already be updated
		EXPECT_EQ(CM_AUTO_COMMIT, db.GetCommitMode());
		EXPECT_EQ(CM_AUTO_COMMIT, db.ReadCommitMode());

		// and back to manual
		EXPECT_NO_THROW(db.SetCommitMode(CM_MANUAL_COMMIT));
		// internal member should already be updated
		EXPECT_EQ(CM_MANUAL_COMMIT, db.GetCommitMode());
		EXPECT_EQ(CM_MANUAL_COMMIT, db.ReadCommitMode());
	}


	TEST_P(DatabaseTest, ReadDataTypesInfo)
	{
		Database db(m_env);

		ASSERT_NO_THROW(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));

		std::vector<SSqlTypeInfo> types;
		ASSERT_NO_THROW(types = db.ReadDataTypesInfo());
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


	TEST_P(DatabaseTest, CommitTransaction)
	{
		std::wstring tableName = TestTables::GetTableName(L"integertypes_tmp", m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", Table::READ_ONLY);
		ASSERT_NO_THROW(iTable.Open(false, true));

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM exodbc.integertypes_tmp WHERE idintegertypes >= 0";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW( m_db.CommitTrans() );

		iTable.Select();
		EXPECT_FALSE( iTable.SelectNext());

		sqlstmt = L"INSERT INTO exodbc.integertypes_tmp (idintegertypes, tsmallint, tint, tbigint) VALUES (1, -32768, -2147483648, -9223372036854775808)";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));

		// Note: If we try to read now from a different database, we do not see the inserted recorded until it is committed
		// BUT: Microsoft SQL Server will just block the second database while a transaction is open
		// We need to examine that behavior, it must be some option. -> Yes, it is the SNAPSHOT Transaction isolation
		{
			Database db2(m_env);
			EXPECT_NO_THROW(db2.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
			// This is extremely confusing: Why do we need to switch to AUTO_COMMIT to set the transaction mode to SNAPSHOT?
			// Note: Setting the transaction mode works always, but doing a query afterwards only if it was set during an auto-commit-mode ?? 
			// TODO: WTF???
			// I guess I got it now: If we change the Transaction-mode on the Connection-Handle, we need to get a new statement-handle afterwards or so:
			// Or: If we set the transaction-mode in the database itself at the very beginning, before the statement-handle of the database is opened
			// thing work fine, without the need to change the transaction-mode
			if (m_db.Dbms() == dbmsMS_SQL_SERVER)
			{
#if HAVE_MSODBCSQL_H
				EXPECT_NO_THROW(db2.SetTransactionIsolationMode(TI_SNAPSHOT));
				exodbc::Table iTable2(&db2, tableName, L"", L"", L"", Table::READ_ONLY);
				ASSERT_NO_THROW(iTable2.Open(false, true));
				iTable2.Select();
				EXPECT_FALSE(iTable2.SelectNext());
#else
				LOG_WARNING(L"Test runs agains MS SQL Server but HAVE_MSODBCSQL_H is not defined to 1. Cannot run test without setting Transaction Mode to Snapshot, or the test would block");
				EXPECT_TRUE(false);
#endif
			}
			else
			{
				EXPECT_NO_THROW(db2.SetTransactionIsolationMode(TI_READ_COMMITTED));
				exodbc::Table iTable2(&db2, tableName, L"", L"", L"", Table::READ_ONLY);
				ASSERT_NO_THROW(iTable2.Open(false, true));
				iTable2.Select();
				EXPECT_FALSE(iTable2.SelectNext());
			}
		}

		// Once we commit we have one record, also in a different database
		EXPECT_NO_THROW(m_db.CommitTrans());

		Database db2(m_env);
		EXPECT_NO_THROW(db2.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		{
			exodbc::Table iTable2(&m_db, tableName, L"", L"", L"", Table::READ_ONLY);
			ASSERT_NO_THROW(iTable2.Open(false, true));
			iTable2.Select();
			EXPECT_TRUE(iTable2.SelectNext());
		}
	}


	TEST_P(DatabaseTest, RollbackTransaction)
	{
		std::wstring tableName = TestTables::GetTableName(L"integertypes_tmp", m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, tableName, L"", L"", L"", Table::READ_ONLY);
		ASSERT_NO_THROW(iTable.Open(false, true));

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM exodbc.integertypes_tmp WHERE idintegertypes >= 0";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());

		iTable.Select();
		EXPECT_FALSE( iTable.SelectNext());

		sqlstmt = L"INSERT INTO exodbc.integertypes_tmp (idintegertypes, tsmallint, tint, tbigint) VALUES (1, -32768, -2147483648, -9223372036854775808)";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		// We rollback and expect no record
		EXPECT_NO_THROW(m_db.RollbackTrans());
		iTable.Select();
		EXPECT_FALSE( iTable.SelectNext());
	}


	TEST_P(DatabaseTest, ReadCatalogs)
	{
		std::vector<std::wstring> cats;
		EXPECT_NO_THROW(cats = m_db.ReadCatalogs());
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
		EXPECT_NO_THROW(schemas = m_db.ReadSchemas());
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
		EXPECT_NO_THROW(tableTypes = m_db.ReadTableTypes());
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
		std::wstring typeName;
		switch(m_db.Dbms())
		{
		case dbmsMY_SQL:
			// We know that mySql uses catalogs, not schemas:
			tableName = L"integertypes";
			schemaName = L"";
			catalogName = L"exodbc";
			typeName = L"";
			LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #52");
			break;
		case dbmsDB2:
			// We know that DB2 uses schemas:
			tableName = L"INTEGERTYPES";
			schemaName = L"EXODBC";
			catalogName = L"";
			typeName = L"";
			break;
		case dbmsMS_SQL_SERVER:
			// And ms uses catalogs, which map to dbs and schemaName
			tableName = L"integertypes";
			schemaName = L"exodbc";
			catalogName = L"exodbc";
			typeName = L"";
			break;
		}
		// \todo: This is simply not working with MySQL, see Ticket #76
		EXPECT_NO_THROW(privs = m_db.ReadTablePrivileges(tableName, schemaName, catalogName, typeName));
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


	TEST_P(DatabaseTest, ReadTablePrimaryKeysInfo)
	{
		TablePrimaryKeysVector pks;
		
		// Find the table-info
		STableInfo iInfo;
		wstring intTableName = TestTables::GetTableName(L"IntegerTypes", m_odbcInfo.m_namesCase);
		wstring idName = TestTables::GetColName(L"IdIntegerTypes", m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(iInfo = m_db.FindOneTable(intTableName, L"", L"", L""));

		EXPECT_NO_THROW(pks = m_db.ReadTablePrimaryKeys(iInfo));
		EXPECT_EQ(1, pks.size());
		if (pks.size() == 1)
		{
			EXPECT_EQ(idName, pks[0].m_columnName);
		}

		STableInfo mkInfo;
		wstring multiKeyTableName = TestTables::GetTableName(L"MultiKey", m_odbcInfo.m_namesCase);
		wstring mkId1 = TestTables::GetColName(L"id1", m_odbcInfo.m_namesCase);
		wstring mkId2 = TestTables::GetColName(L"id2", m_odbcInfo.m_namesCase);
		wstring mkId3 = TestTables::GetColName(L"id3", m_odbcInfo.m_namesCase);
		EXPECT_NO_THROW(mkInfo = m_db.FindOneTable(multiKeyTableName, L"", L"", L""));

		EXPECT_NO_THROW(pks = m_db.ReadTablePrimaryKeys(mkInfo));
		EXPECT_EQ(3, pks.size());
		if (pks.size() == 3)
		{
			EXPECT_EQ(mkId1, pks[0].m_columnName);
			EXPECT_EQ(1, pks[0].m_keySequence);
			EXPECT_EQ(mkId2, pks[1].m_columnName);
			EXPECT_EQ(2, pks[1].m_keySequence);
			EXPECT_EQ(mkId3, pks[2].m_columnName);
			EXPECT_EQ(3, pks[2].m_keySequence);
		}

	}


	TEST_P(DatabaseTest, ReadTableColumnInfo)
	{
		std::vector<SColumnInfo> cols;
		std::wstring tableName;
		std::wstring schemaName;
		std::wstring catalogName;
		std::wstring typeName;
		switch(m_db.Dbms())
		{
		case dbmsMY_SQL:
			// We know that mySql uses catalogs, not schemas:
			tableName = L"numerictypes";
			schemaName = L"";
			catalogName = L"exodbc";
			typeName = L"";
			break;
		case dbmsDB2:
			// We know that DB2 uses schemas:
			tableName = L"NUMERICTYPES";
			schemaName = L"EXODBC";
			catalogName = L"";
			typeName = L"";
			break;
		case dbmsMS_SQL_SERVER:
			// And ms uses catalogs, which map to dbs and schemaName
			tableName = L"numerictypes";
			schemaName = L"exodbc";
			catalogName = L"exodbc";
			typeName = L"";
		}
		EXPECT_NO_THROW(cols = m_db.ReadTableColumnInfo(tableName, schemaName, catalogName, typeName));
		// Our decimals columns must have a num prec radix value of 10, a column size of the total digits, and a decimal digits the nr of digits after the delimeter
		ASSERT_TRUE(cols.size() == 4);
		SColumnInfo col = cols[2];
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
			// We know that DB2 uses schemas (and uppercase):
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
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"", L"", L""));
		EXPECT_EQ(1, tables.size());
		// Find one table by using table-name and schema/catalog
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, schemaName, catalogName, L""));
		EXPECT_EQ(1, tables.size());
		// In all cases, we should not find anything if we use a schema or a catalog that does not exist
		// \todo: Create Ticket (Info): Note: When using MySQL-Odbc driver, if 'do not use INFORMATION_SCHEMA' is set, this will fail due to an access denied for database "wrongCatalog"
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"WrongSchema", L"", L""));
		EXPECT_EQ(0, tables.size());
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"", L"WrongCatalog", L""));
		EXPECT_EQ(0, tables.size());
		// Also, we have a table, not a view
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"", L"", L"VIEW"));
		EXPECT_EQ(0, tables.size());
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"", L"", L"TABLE"));
		EXPECT_EQ(1, tables.size());
		// What about search-patterns? Note: Not use for table-type
		// \todo: They just dont work with MySql 3.51 ?
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"%", L"%", L""));
		EXPECT_EQ(1, tables.size());
		if(tables.size() > 0)
		{
			EXPECT_EQ(tableName, tables[0].m_tableName);
			EXPECT_EQ(schemaName, tables[0].m_schemaName);
			EXPECT_EQ(catalogName, tables[0].m_catalogName);
		}
		std::wstring schemaPattern = schemaName;
		if(schemaName.length() > 0)
			schemaPattern = (boost::wformat(L"%%%s%%") %schemaName).str();
		std::wstring catalogPattern = catalogName;
		if(catalogName.length() > 0)
			catalogPattern = (boost::wformat(L"%%%s%%") %catalogName).str();
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, schemaPattern, catalogPattern, L""));
		EXPECT_EQ(1, tables.size());
		if(tables.size() > 0)
		{
			EXPECT_EQ(tableName, tables[0].m_tableName);
			EXPECT_EQ(schemaName, tables[0].m_schemaName);
			EXPECT_EQ(catalogName, tables[0].m_catalogName);
		}
	}


	TEST_P(DatabaseTest, ReadCompleteCatalog)
	{
		SDbCatalogInfo cat;
		EXPECT_NO_THROW( cat = m_db.ReadCompleteCatalog());
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
		std::wstring typeName = L"";
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
		EXPECT_EQ(nrCols, m_db.ReadColumnCount(tableName, schemaName, catalogName, typeName));
		// we should also work if we just search by the tableName, as long as tableName is unique within db
		EXPECT_EQ(nrCols, m_db.ReadColumnCount(tableName, L"", L"", L""));
	}


	// Interfaces
	// ----------

} //namespace exodbc
