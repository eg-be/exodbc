/*!
 * \file DbTest.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
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
	void DatabaseTest::SetUpTestCase()
	{
	}

	void DatabaseTest::SetUp()
	{
		// Set up is called for every test
		m_odbcInfo = g_odbcInfo;
		m_env.Init(OdbcVersion::V_3);
		ASSERT_NO_THROW(m_db.Init(&m_env));
		if (m_odbcInfo.HasConnectionString())
		{
			ASSERT_NO_THROW(m_db.Open(m_odbcInfo.m_connectionString));
		}
		else
		{
			ASSERT_NO_THROW(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
	}

	void DatabaseTest::TearDown()
	{
		if(m_db.IsOpen())
		{
			EXPECT_NO_THROW(m_db.Close());
		}
	}


	TEST_F(DatabaseTest, CopyConstructor)
	{
		Database db1(&m_env);
		Database c1(db1);

		EXPECT_TRUE(c1.HasConnectionHandle());
		{
			DontDebugBreak ddb;
			LogLevelError lle;
			EXPECT_THROW(c1.Init(&m_env), AssertionException);
		}

		Database db2;
		Database c2(db2);
		EXPECT_FALSE(c2.HasConnectionHandle());
		EXPECT_NO_THROW(c2.Init(&m_env));
	}


	TEST_F(DatabaseTest, PrintDatabaseInfo)
	{
		Database db(&m_env);
		if (m_odbcInfo.HasConnectionString())
		{
			ASSERT_NO_THROW(db.Open(m_odbcInfo.m_connectionString));
		}
		else
		{
			ASSERT_NO_THROW(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
		{
			LogLevelInfo lli;
			LOG_INFO(L"Will print Database Information now");
			LOG_INFO(L"===================================");
			LOG_INFO(db.GetDbInfo().ToString());
			LOG_INFO(L"===================================");
		}
	}


	TEST_F(DatabaseTest, PrintDatabaseTypeInfo)
	{
		Database db(&m_env);
		if (m_odbcInfo.HasConnectionString())
		{
			ASSERT_NO_THROW(db.Open(m_odbcInfo.m_connectionString));
		}
		else
		{
			ASSERT_NO_THROW(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
		{
			LogLevelInfo lli;
			std::wstringstream ws;
			ws << L"Will print Database Type Information now" << std::endl;
			ws << L"===================================" << std::endl;
			ws << std::endl;
			SqlTypeInfosVector types = db.GetTypeInfos();
			std::set<SSqlTypeInfo> typesSet(types.begin(), types.end());
			bool first = true;
			for (std::set<SSqlTypeInfo>::const_iterator it = typesSet.begin(); it != typesSet.end(); ++it)
			{
				ws << it->ToOneLineStr(first) << std::endl;
				if (first)
					first = false;
			}
			ws << L"===================================";
			LOG_INFO(ws.str());
		}
	}


	TEST_F(DatabaseTest, SetConnectionAttributes)
	{
		Database db(&m_env);

		EXPECT_NO_THROW(db.SetConnectionAttributes());
	}


	TEST_F(DatabaseTest, ReadTransactionIsolationMode)
	{
		TransactionIsolationMode tiMode = m_db.ReadTransactionIsolationMode();
		EXPECT_NE(TransactionIsolationMode::UNKNOWN, tiMode);
	}


	TEST_F(DatabaseTest, SetTransactionIsolationMode)
	{
		TransactionIsolationMode tiMode = TransactionIsolationMode::READ_COMMITTED;
		EXPECT_NO_THROW(m_db.SetTransactionIsolationMode(tiMode));
		tiMode = m_db.ReadTransactionIsolationMode();
		EXPECT_EQ(TransactionIsolationMode::READ_COMMITTED, tiMode);

		// The rest is not supported by access
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			return;
		}

		tiMode = TransactionIsolationMode::READ_UNCOMMITTED;
		EXPECT_NO_THROW(m_db.SetTransactionIsolationMode(tiMode));
		tiMode = m_db.ReadTransactionIsolationMode();
		EXPECT_EQ(TransactionIsolationMode::READ_UNCOMMITTED, tiMode);

		tiMode = TransactionIsolationMode::REPEATABLE_READ;
		EXPECT_NO_THROW(m_db.SetTransactionIsolationMode(tiMode));
		tiMode = m_db.ReadTransactionIsolationMode();
		EXPECT_EQ(TransactionIsolationMode::REPEATABLE_READ, tiMode);

		tiMode = TransactionIsolationMode::SERIALIZABLE;
		EXPECT_NO_THROW(m_db.SetTransactionIsolationMode(tiMode));
		tiMode = m_db.ReadTransactionIsolationMode();
		EXPECT_EQ(TransactionIsolationMode::SERIALIZABLE, tiMode);

		if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
#if HAVE_MSODBCSQL_H
			tiMode = TransactionIsolationMode::SNAPSHOT;
			EXPECT_NO_THROW(m_db.SetTransactionIsolationMode(tiMode));
			tiMode = m_db.ReadTransactionIsolationMode();
			EXPECT_EQ(TransactionIsolationMode::SNAPSHOT, tiMode);
#else
			LOG_WARNING(L"Skipping test because testing against Ms Sql Server but msodbcsql.h is missing");
			EXPECT_TRUE(false);
#endif
		}
	}


	TEST_F(DatabaseTest, CanSetTransactionIsolationMode)
	{
		EXPECT_TRUE(m_db.CanSetTransactionIsolationMode(TransactionIsolationMode::READ_COMMITTED));
		// Those are not supported by Access, for the rest we expect support
		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
		{
			EXPECT_TRUE(m_db.CanSetTransactionIsolationMode(TransactionIsolationMode::READ_UNCOMMITTED));
			EXPECT_TRUE(m_db.CanSetTransactionIsolationMode(TransactionIsolationMode::REPEATABLE_READ));
			EXPECT_TRUE(m_db.CanSetTransactionIsolationMode(TransactionIsolationMode::SERIALIZABLE));
		}
		EXPECT_FALSE(m_db.CanSetTransactionIsolationMode(TransactionIsolationMode::UNKNOWN));
	}


	TEST_F(DatabaseTest, GetTracefile)
	{
		wstring tracefile;
		EXPECT_NO_THROW(tracefile = m_db.GetTracefile());
		LOG_INFO(boost::str(boost::wformat(L"Tracefile is: '%s'") % tracefile));
	}


	TEST_F(DatabaseTest, SetTracefile)
	{
		TCHAR tmpDir[MAX_PATH + 1];
		DWORD ret = GetTempPath(MAX_PATH + 1, tmpDir);
		ASSERT_TRUE(ret > 0);
		wstring tracefile(&tmpDir[0]);
		wstring filename = boost::str(boost::wformat(L"trace_%s.log") % DatabaseProcudt2s(m_db.GetDbms()));
		tracefile += filename;

		EXPECT_NO_THROW(m_db.SetTracefile(tracefile));
	}


	TEST_F(DatabaseTest, SetTrace)
	{
		Database db(&m_env);
		EXPECT_NO_THROW(db.SetTrace(true));

		// and disable again
		EXPECT_NO_THROW(db.SetTrace(false));
	}


	TEST_F(DatabaseTest, GetTrace)
	{
		Database db(&m_env);
		ASSERT_NO_THROW(db.SetTrace(true));
		EXPECT_TRUE(db.GetTrace());
		ASSERT_NO_THROW(db.SetTrace(false));
		EXPECT_FALSE(db.GetTrace());
	}


	TEST_F(DatabaseTest, DoSomeTrace)
	{
		// Set tracefile first, do that on a db not open yet
		TCHAR tmpDir[MAX_PATH + 1];
		DWORD ret = GetTempPath(MAX_PATH + 1, tmpDir);
		ASSERT_TRUE(ret > 0);
		wstring tracefile(&tmpDir[0]);
		wstring filename = boost::str(boost::wformat(L"trace_%s_DoSomeTrace.log") % DatabaseProcudt2s(m_db.GetDbms()));
		tracefile += filename;

		Database db(&m_env);
		ASSERT_NO_THROW(db.SetTracefile(tracefile));
		EXPECT_NO_THROW(db.SetTrace(true));

		// Open db, and open a table, do some query on the table
		if (m_odbcInfo.HasConnectionString())
		{
			db.Open(m_odbcInfo.m_connectionString);
		}
		else
		{
			db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password);
		}

		test::ClearIntTypesTmpTable(db, m_odbcInfo.m_namesCase);
		Table iTable(&db, AF_READ_WRITE, test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase), L"", L"", L"");
		if (db.GetDbms() == DatabaseProduct::ACCESS)
		{
			iTable.SetColumnPrimaryKeyIndexes({ 0 });
		}
		iTable.Open();
		iTable.SetColumnValue(0, (SQLINTEGER) 100);
		iTable.SetColumnNull(1);
		iTable.SetColumnValue(2, (SQLINTEGER) 113);
		iTable.SetColumnNull(3);
		iTable.Insert();
		
		iTable.SetColumnValue(2, (SQLINTEGER) 223);
		iTable.Update();
	}


	TEST_F(DatabaseTest, Open)
	{		
		// Open an existing db by passing the Env to the ctor
		Environment env(OdbcVersion::V_3);
		ASSERT_TRUE(env.HasEnvironmentHandle());
		Database db(&env);
		if (m_odbcInfo.HasConnectionString())
		{
			EXPECT_NO_THROW(db.Open(m_odbcInfo.m_connectionString));
		}
		else
		{
			EXPECT_NO_THROW(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
		EXPECT_NO_THROW(db.Close());

		// Open an existing db using the default c'tor and setting params on db
		Database db2;
		ASSERT_NO_THROW(db2.Init(&env));
		if (m_odbcInfo.HasConnectionString())
		{
			// also test that the returned connection-string is not empty
			wstring outConnStr;
			EXPECT_NO_THROW(outConnStr = db2.Open(m_odbcInfo.m_connectionString));
			EXPECT_FALSE(outConnStr.empty());
		}
		else
		{
			EXPECT_NO_THROW(db2.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
		EXPECT_NO_THROW(db2.Close());

		// Try to open with a different password / user, expect to fail when opening the db.
		{
			Database failDb(&m_env);
			EXPECT_THROW(failDb.Open(L"ThisDSNDoesNotExist", L"NorTheUser", L"WithThisPassword"), SqlResultException);
		}
	}


	// TODO: Test Close. Close should return a value if succeeded
	TEST_F(DatabaseTest, Close)
	{
		// Try to close a db that really is open
		Database db1(&m_env);
		if (m_odbcInfo.HasConnectionString())
		{
			ASSERT_NO_THROW(db1.Open(m_odbcInfo.m_connectionString));
		}
		else
		{
			ASSERT_NO_THROW(db1.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}

		EXPECT_NO_THROW(db1.Close());

		// \todo: Close works okay, does not return false if there is nothing to do
		//// And one that is not open
		//Database db2(m_pDbEnv);
		//EXPECT_FALSE(db2.Close());
	}

	TEST_F(DatabaseTest, ReadCommitMode)
	{
		// We default to manual commit
		EXPECT_EQ(CommitMode::MANUAL, m_db.ReadCommitMode());
	}

	TEST_F(DatabaseTest, SetCommitMode)
	{
		Database db(&m_env);
		if (m_odbcInfo.HasConnectionString())
		{
			ASSERT_NO_THROW(db.Open(m_odbcInfo.m_connectionString));
		}
		else
		{
			ASSERT_NO_THROW(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}

		// We default to manual commit
		EXPECT_EQ(CommitMode::MANUAL, db.GetCommitMode());
		EXPECT_EQ(CommitMode::MANUAL, db.ReadCommitMode());

		// Switch to auto
		EXPECT_NO_THROW(db.SetCommitMode(CommitMode::AUTO));
		// internal member should already be updated
		EXPECT_EQ(CommitMode::AUTO, db.GetCommitMode());
		EXPECT_EQ(CommitMode::AUTO, db.ReadCommitMode());

		// and back to manual
		EXPECT_NO_THROW(db.SetCommitMode(CommitMode::MANUAL));
		// internal member should already be updated
		EXPECT_EQ(CommitMode::MANUAL, db.GetCommitMode());
		EXPECT_EQ(CommitMode::MANUAL, db.ReadCommitMode());
	}


	TEST_F(DatabaseTest, ReadDataTypesInfo)
	{
		Database db(&m_env);

		if (m_odbcInfo.HasConnectionString())
		{
			ASSERT_NO_THROW(db.Open(m_odbcInfo.m_connectionString));
		}
		else
		{
			ASSERT_NO_THROW(db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}

		std::vector<SSqlTypeInfo> types;
		ASSERT_NO_THROW(types = db.ReadDataTypesInfo());
		EXPECT_TRUE(types.size() > 0);

		std::wstringstream ws;
		ws << L"TypeInfo of database with DSN '" << (m_odbcInfo.HasConnectionString() ? m_odbcInfo.m_connectionString : m_odbcInfo.m_dsn) << L"', total " << types.size() << L" types reported:" << std::endl;
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


	TEST_F(DatabaseTest, DetectDbms)
	{
		// We just know how we name the different odbc-sources
		// TODO: This is not nice, but is there any other reliable way? Add to doc somewhere

		if (m_odbcInfo.HasConnectionString())
		{
			LOG_WARNING(L"Skipping Test DetectDbms, because Test is implemented to stupid it only works with a named DSN");
			return;
		}

		if(boost::algorithm::find_first(m_odbcInfo.m_dsn, L"DB2"))
		{
			EXPECT_TRUE(m_db.GetDbms() == DatabaseProduct::DB2);
		}
		else if(boost::algorithm::find_first(m_odbcInfo.m_dsn, L"MySql"))
		{
			EXPECT_TRUE(m_db.GetDbms() == DatabaseProduct::MY_SQL);
		}
		else if(boost::algorithm::find_first(m_odbcInfo.m_dsn, L"SqlServer"))
		{
			EXPECT_TRUE(m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER);
		}
		else if (boost::algorithm::find_first(m_odbcInfo.m_dsn, L"Access"))
		{
			EXPECT_TRUE(m_db.GetDbms() == DatabaseProduct::ACCESS);
		}
		else
		{
			EXPECT_EQ(std::wstring(L"Unknown DSN name"), m_odbcInfo.m_dsn);
		}
	}


	TEST_F(DatabaseTest, CommitTransaction)
	{
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
		ASSERT_NO_THROW(iTable.Open());

		ASSERT_NO_THROW(test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase));
		ASSERT_NO_THROW(test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, m_db, 1, 44, 54543, test::ValueIndicator::IS_NULL, false));

		// Note: If we try to read now from a different database, we do not see the inserted recorded until it is committed
		// BUT: Microsoft SQL Server will just block the second database while a transaction is open
		// We need to examine that behavior, it must be some option. -> Yes, it is the SNAPSHOT Transaction isolation
		{
			Database db2(&m_env);
			if (m_odbcInfo.HasConnectionString())
			{
				EXPECT_NO_THROW(db2.Open(m_odbcInfo.m_connectionString));
			}
			else
			{
				EXPECT_NO_THROW(db2.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
			}
			// This is extremely confusing: Why do we need to switch to AUTO_COMMIT to set the transaction mode to SNAPSHOT?
			// Note: Setting the transaction mode works always, but doing a query afterwards only if it was set during an auto-commit-mode ?? 
			// TODO: WTF???
			// I guess I got it now: If we change the Transaction-mode on the Connection-Handle, we need to get a new statement-handle afterwards or so:
			// Or: If we set the transaction-mode in the database itself at the very beginning, before the statement-handle of the database is opened
			// thing work fine, without the need to change the transaction-mode
			if (m_db.GetDbms() == DatabaseProduct::MS_SQL_SERVER)
			{
#if HAVE_MSODBCSQL_H
				EXPECT_NO_THROW(db2.SetTransactionIsolationMode(TransactionIsolationMode::SNAPSHOT));
				exodbc::Table iTable2(&db2, AF_READ, tableName, L"", L"", L"");
				ASSERT_NO_THROW(iTable2.Open());
				iTable2.Select();
				EXPECT_FALSE(iTable2.SelectNext());
#else
				LOG_WARNING(L"Test runs agains MS SQL Server but HAVE_MSODBCSQL_H is not defined to 1. Cannot run test without setting Transaction Mode to Snapshot, or the test would block");
				EXPECT_TRUE(false);
#endif
			}
			else
			{
				EXPECT_NO_THROW(db2.SetTransactionIsolationMode(TransactionIsolationMode::READ_COMMITTED));
				exodbc::Table iTable2(&db2, AF_READ, tableName, L"", L"", L"");
				ASSERT_NO_THROW(iTable2.Open());
				iTable2.Select();
				EXPECT_FALSE(iTable2.SelectNext());
			}
		}

		// Once we commit we have one record, also in a different database
		EXPECT_NO_THROW(m_db.CommitTrans());

		Database db2(&m_env);
		if (m_odbcInfo.HasConnectionString())
		{
			EXPECT_NO_THROW(db2.Open(m_odbcInfo.m_connectionString));
		}
		else
		{
			EXPECT_NO_THROW(db2.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
		{
			exodbc::Table iTable2(&m_db, AF_READ, tableName, L"", L"", L"");
			ASSERT_NO_THROW(iTable2.Open());
			iTable2.Select();
			EXPECT_TRUE(iTable2.SelectNext());
		}
	}


	TEST_F(DatabaseTest, RollbackTransaction)
	{
		std::wstring tableName = test::GetTableName(test::TableId::INTEGERTYPES_TMP, m_odbcInfo.m_namesCase);
		exodbc::Table iTable(&m_db, AF_READ, tableName, L"", L"", L"");
		ASSERT_NO_THROW(iTable.Open());

		ASSERT_NO_THROW(test::ClearIntTypesTmpTable(m_db, m_odbcInfo.m_namesCase));

		// No records now
		iTable.Select();
		EXPECT_FALSE( iTable.SelectNext());

		// Insert one
		ASSERT_NO_THROW(test::InsertIntTypesTmp(m_odbcInfo.m_namesCase, m_db, 1, 44, 54543, test::ValueIndicator::IS_NULL, false));
		// We have a record now
		iTable.Select();
		EXPECT_TRUE( iTable.SelectNext() );

		// We rollback and expect no record
		EXPECT_NO_THROW(m_db.RollbackTrans());
		iTable.Select();
		EXPECT_FALSE( iTable.SelectNext());
	}


	TEST_F(DatabaseTest, ReadCatalogs)
	{
 		std::vector<std::wstring> cats;
		ASSERT_NO_THROW(cats = m_db.ReadCatalogs());
		switch(m_db.GetDbms())
		{
		case DatabaseProduct::DB2:
			// DB2 does not support catalogs. it reports zero catalogs
			EXPECT_EQ(0, cats.size());
			break;

		case DatabaseProduct::MS_SQL_SERVER:
		case DatabaseProduct::MY_SQL:
			// Those report our test-db as a catalog
			EXPECT_TRUE(std::find(cats.begin(), cats.end(), L"exodbc") != cats.end());
			break;
		case DatabaseProduct::ACCESS:
			// Returns the file-path as catalog, must be our file named 'exodbc' (note: .mdb is not in the path)
			EXPECT_EQ(1, cats.size());
			EXPECT_TRUE(boost::algorithm::ends_with(cats[0], L"exodbc"));
			break;
		}
	}

	TEST_F(DatabaseTest, ReadSchemas)
	{
		std::vector<std::wstring> schemas;
		EXPECT_NO_THROW(schemas = m_db.ReadSchemas());
		switch(m_db.GetDbms())
		{
		case DatabaseProduct::DB2:
			// DB2 reports our test-db as a schema
			EXPECT_TRUE(schemas.size() > 0);
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"EXODBC") != schemas.end());
			break;
		case DatabaseProduct::MY_SQL:
			// Mysql reports one schema with an empty name
			EXPECT_TRUE(schemas.size() == 1);
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"") != schemas.end());
			break;
		case DatabaseProduct::MS_SQL_SERVER:
			// ms sql server reported at least 3 schemas:
			EXPECT_TRUE(schemas.size() >= 3);
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"exodbc") != schemas.end());
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"INFORMATION_SCHEMA") != schemas.end());
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), L"sys") != schemas.end());
			break;
		}
	}


	TEST_F(DatabaseTest, ReadTableTypes)
	{
		std::vector<std::wstring> tableTypes;
		EXPECT_NO_THROW(tableTypes = m_db.ReadTableTypes());
		// Check that we have at least a type TABLE and a type VIEW
		EXPECT_TRUE(std::find(tableTypes.begin(), tableTypes.end(), L"TABLE") != tableTypes.end());
		EXPECT_TRUE(std::find(tableTypes.begin(), tableTypes.end(), L"VIEW") != tableTypes.end());
	}


	TEST_F(DatabaseTest, ReadTablePrivileges)
	{
		TablePrivilegesVector privs;
		std::wstring tableName;
		std::wstring schemaName;
		std::wstring catalogName;
		std::wstring typeName;
		switch(m_db.GetDbms())
		{
		case DatabaseProduct::MY_SQL:
			// We know that mySql uses catalogs, not schemas:
			tableName = L"integertypes";
			schemaName = L"";
			catalogName = L"exodbc";
			typeName = L"";
			LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #76");
			break;
		case DatabaseProduct::DB2:
			// We know that DB2 uses schemas:
			tableName = L"INTEGERTYPES";
			schemaName = L"EXODBC";
			catalogName = L"";
			typeName = L"";
			break;
		case DatabaseProduct::MS_SQL_SERVER:
			// And ms uses catalogs, which map to dbs and schemaName
			tableName = L"integertypes";
			schemaName = L"exodbc";
			catalogName = L"exodbc";
			typeName = L"";
			break;
		case DatabaseProduct::ACCESS:
			// access only tablenames
			tableName = L"integertypes";
		}
		// \todo: This is simply not working with MySQL, see Ticket #76
		EXPECT_NO_THROW(privs = m_db.ReadTablePrivileges(tableName, schemaName, catalogName, typeName));
		bool canSelect = false;
		bool canInsert = false;
		bool canDelete = false;
		bool canUpdate = false;
		TablePrivilegesVector::const_iterator it;
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


	TEST_F(DatabaseTest, ReadTablePrimaryKeysInfo)
	{
		TablePrimaryKeysVector pks;
		
		// Find the table-info
		TableInfo iInfo;
		wstring intTableName = test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		wstring idName = test::GetIdColumnName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(iInfo = m_db.FindOneTable(intTableName, L"", L"", L""));

		EXPECT_NO_THROW(pks = m_db.ReadTablePrimaryKeys(iInfo));
		EXPECT_EQ(1, pks.size());
		if (pks.size() == 1)
		{
			EXPECT_EQ(idName, pks[0].GetColumnName());
		}

		TableInfo mkInfo;
		wstring multiKeyTableName = test::GetTableName(test::TableId::MULTIKEY, m_odbcInfo.m_namesCase);
		wstring mkId1 = test::ConvertNameCase(L"id1", m_odbcInfo.m_namesCase);
		wstring mkId2 = test::ConvertNameCase(L"id2", m_odbcInfo.m_namesCase);
		wstring mkId3 = test::ConvertNameCase(L"id3", m_odbcInfo.m_namesCase);
		EXPECT_NO_THROW(mkInfo = m_db.FindOneTable(multiKeyTableName, L"", L"", L""));

		EXPECT_NO_THROW(pks = m_db.ReadTablePrimaryKeys(mkInfo));
		EXPECT_EQ(3, pks.size());
		if (pks.size() == 3)
		{
			EXPECT_EQ(mkId1, pks[0].GetColumnName());
			EXPECT_EQ(1, pks[0].GetKeySequence());
			EXPECT_EQ(mkId2, pks[1].GetColumnName());
			EXPECT_EQ(2, pks[1].GetKeySequence());
			EXPECT_EQ(mkId3, pks[2].GetColumnName());
			EXPECT_EQ(3, pks[2].GetKeySequence());
		}

	}


	TEST_F(DatabaseTest, ReadTableColumnInfo)
	{
		std::vector<ColumnInfo> cols;
		std::wstring tableName;
		std::wstring schemaName;
		std::wstring catalogName;
		std::wstring typeName;
		switch(m_db.GetDbms())
		{
		case DatabaseProduct::MY_SQL:
			// We know that mySql uses catalogs, not schemas:
			tableName = L"numerictypes";
			schemaName = L"";
			catalogName = L"exodbc";
			typeName = L"";
			break;
		case DatabaseProduct::DB2:
			// We know that DB2 uses schemas:
			tableName = L"NUMERICTYPES";
			schemaName = L"EXODBC";
			catalogName = L"";
			typeName = L"";
			break;
		case DatabaseProduct::MS_SQL_SERVER:
			// And ms uses catalogs, which map to dbs and schemaName
			tableName = L"numerictypes";
			schemaName = L"exodbc";
			catalogName = L"exodbc";
			typeName = L"";
			break;
		case DatabaseProduct::ACCESS:
			tableName = L"integertypes";
			break;
		}
		EXPECT_NO_THROW(cols = m_db.ReadTableColumnInfo(tableName, schemaName, catalogName, typeName));
		// Note: Access reads a different table
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			EXPECT_EQ(4, cols.size());
		}
		else
		{
			// Our decimals columns must have a num prec radix value of 10, a column size of the total digits, and a decimal digits the nr of digits after the delimeter
			ASSERT_TRUE(cols.size() == 4);
			ColumnInfo col = cols[2];
			EXPECT_FALSE(col.IsNumPrecRadixNull());
			EXPECT_FALSE(col.IsColumnSizeNull());
			EXPECT_FALSE(col.IsDecimalDigitsNull());
			EXPECT_EQ(10, col.GetNumPrecRadix());
			EXPECT_EQ(18, col.GetColumnSize());
			EXPECT_EQ(10, col.GetDecimalDigits());
		}
	}


	TEST_F(DatabaseTest, FindTables)
	{
		TableInfosVector tables;
		std::wstring tableName;
		std::wstring schemaName;
		std::wstring catalogName;
		switch(m_db.GetDbms())
		{
		case DatabaseProduct::MY_SQL:
			// We know that mySql uses catalogs, not schemas:
			tableName = L"integertypes";
			schemaName = L"";
			catalogName = L"exodbc";
			break;
		case DatabaseProduct::DB2:
			// We know that DB2 uses schemas (and uppercase):
			tableName = L"INTEGERTYPES";
			schemaName = L"EXODBC";
			catalogName = L"";
			break;
		case DatabaseProduct::MS_SQL_SERVER:
			// And ms uses catalogs, which map to dbs and schemaName
			tableName = L"integertypes";
			schemaName = L"exodbc";
			catalogName = L"exodbc";
			break;
		case DatabaseProduct::ACCESS:
			// only tablenames
			tableName = L"integertypes";
			break;
		}
		// Find one table by using only the table-name as search param
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"", L"", L""));
		EXPECT_EQ(1, tables.size());
		// Find one table by using table-name and schema/catalog
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, schemaName, catalogName, L""));
		EXPECT_EQ(1, tables.size());
		// In all cases, we should not find anything if we use a schema or a catalog that does not exist
		// \todo: Create Ticket (Info): Note: When using MySQL-Odbc driver, if 'do not use INFORMATION_SCHEMA' is set, this will fail due to an access denied for database "wrongCatalog"
		// Access has no support for schemas
		if (m_db.GetDbms() != DatabaseProduct::ACCESS)
		{
			EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"WrongSchema", L"", L""));
			EXPECT_EQ(0, tables.size());
		}
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"", L"WrongCatalog", L""));
		EXPECT_EQ(0, tables.size());
		// Also, we have a table, not a view
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"", L"", L"VIEW"));
		EXPECT_EQ(0, tables.size());
		EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"", L"", L"TABLE"));
		EXPECT_EQ(1, tables.size());
		// What about search-patterns? Note: Not use for table-type
		// search patterns are not supported for schemas by Access:
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"", L"%", L""));
		}
		else
		{
			EXPECT_NO_THROW(tables = m_db.FindTables(tableName, L"%", L"%", L""));
		}
		EXPECT_EQ(1, tables.size());
		if(tables.size() > 0)
		{
			EXPECT_EQ(tableName, tables[0].GetPureName());
			EXPECT_EQ(schemaName, tables[0].GetSchema());
			if (m_db.GetDbms() != DatabaseProduct::ACCESS)
			{
				// still no support for catalogs on Access
				EXPECT_EQ(catalogName, tables[0].GetCatalog());
			}
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
			EXPECT_EQ(tableName, tables[0].GetPureName());
			EXPECT_EQ(schemaName, tables[0].GetSchema());
			if (m_db.GetDbms() != DatabaseProduct::ACCESS)
			{
				// still no support for catalogs on Access
				EXPECT_EQ(catalogName, tables[0].GetCatalog());
			}
		}
	}


	TEST_F(DatabaseTest, ReadCompleteCatalog)
	{
		SDbCatalogInfo cat;
		EXPECT_NO_THROW( cat = m_db.ReadCompleteCatalog());
		// TODO: This is confusing. DB2 reports schemas, what is correct, but mysql reports catalogs?? wtf?
		if (m_db.GetDbms() == DatabaseProduct::DB2)
		{
			EXPECT_TRUE(cat.m_schemas.find(L"EXODBC") != cat.m_schemas.end());
		}
		else if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			EXPECT_TRUE(cat.m_catalogs.find(L"exodbc") != cat.m_catalogs.end());
		}
		EXPECT_GE((SQLINTEGER) cat.m_tables.size(), 12);
	}


	TEST_F(DatabaseTest, ReadColumnCount)
	{
		std::wstring tableName = L"";
		std::wstring schemaName = L"";
		std::wstring catalogName = L"";
		std::wstring typeName = L"";
		int nrCols = 4;
		switch(m_db.GetDbms())
		{
		case DatabaseProduct::DB2:
			// DB2 has schemas
			tableName = L"INTEGERTYPES";
			schemaName = L"EXODBC";
			break;
		case DatabaseProduct::MY_SQL:
			// mysql has catalogs
			tableName = L"integertypes";
			catalogName = L"exodbc";
			break;
		case DatabaseProduct::MS_SQL_SERVER:
			// ms has catalogs and schemas
			tableName = L"integertypes";
			catalogName = L"exodbc";
			schemaName = L"exodbc";
			break;
		case DatabaseProduct::ACCESS:
			// access only tablenames
			tableName = L"integertypes";
			break;
		}
		EXPECT_EQ(nrCols, m_db.ReadColumnCount(tableName, schemaName, catalogName, typeName));
		// we should also work if we just search by the tableName, as long as tableName is unique within db
		EXPECT_EQ(nrCols, m_db.ReadColumnCount(tableName, L"", L"", L""));
	}


	// Interfaces
	// ----------

} //namespace exodbc
