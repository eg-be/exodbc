/*!
 * \file DatabaseTest.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
 * 
 * [Brief CPP-file description]
 */ 

// Own header
#include "DatabaseTest.h"

// Same component headers
#include "ManualTestTables.h"
#include "exOdbcGTestHelpers.h"

// Other headers
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/Exception.h"
#include "boost/format.hpp"
#include "boost/algorithm/string.hpp"
#include "boost/filesystem.hpp"

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
using namespace exodbc;

namespace exodbctest
{
	void DatabaseTest::SetUp()
	{
		// Set up is called for every test
		m_pEnv->Init(OdbcVersion::V_3);
		m_pDb = OpenTestDb(m_pEnv);
	}


	void DatabaseTest::TearDown()
	{
		if(m_pDb->IsOpen())
		{
			m_pDb->Close();
		}
	}


	TEST_F(DatabaseTest, Init)
	{
		Database db;

		// Fail for not allocated environment
		EnvironmentPtr pEnv = std::make_shared<Environment>();

		{
			LogLevelSetter ll(LogLevel::None);
			EXPECT_THROW(db.Init(pEnv), AssertionException);
		}

		// But succeed if statement becomes valid
		pEnv->Init(OdbcVersion::V_3);
		EXPECT_NO_THROW(db.Init(pEnv));
	}


	TEST_F(DatabaseTest, CopyConstructor)
	{
		Database db1(m_pEnv);
		Database c1(db1);

		EXPECT_TRUE(c1.HasConnectionHandle());
		{
			LogLevelSetter ll(LogLevel::None);
			EXPECT_THROW(c1.Init(m_pEnv), AssertionException);
		}

		Database db2;
		Database c2(db2);
		EXPECT_FALSE(c2.HasConnectionHandle());
		EXPECT_NO_THROW(c2.Init(m_pEnv));
	}


	TEST_F(DatabaseTest, PrintDatabaseTypeInfo)
	{
		std::stringstream ss;
		ss << u8"Will print Database Type Information now" << std::endl;
		ss << u8"===================================" << std::endl;
		ss << std::endl;
		SqlTypeInfosVector types = m_pDb->GetTypeInfos();
		std::set<SSqlTypeInfo> typesSet(types.begin(), types.end());
		bool first = true;
		for (std::set<SSqlTypeInfo>::const_iterator it = typesSet.begin(); it != typesSet.end(); ++it)
		{
			ss << it->ToOneLineStr(first) << std::endl;
			if (first)
				first = false;
		}
		ss << u8"===================================";
		LOG_INFO(ss.str());
	}


	TEST_F(DatabaseTest, SetConnectionAttributes)
	{
		Database db(m_pEnv);

		EXPECT_NO_THROW(db.SetConnectionAttributes());
	}


	TEST_F(DatabaseTest, ReadTransactionIsolationMode)
	{
		TransactionIsolationMode tiMode = m_pDb->ReadTransactionIsolationMode();
		EXPECT_NE(TransactionIsolationMode::UNKNOWN, tiMode);
	}


	TEST_F(DatabaseTest, SetTransactionIsolationMode)
	{
		TransactionIsolationMode tiMode = TransactionIsolationMode::READ_COMMITTED;
		EXPECT_NO_THROW(m_pDb->SetTransactionIsolationMode(tiMode));
		tiMode = m_pDb->ReadTransactionIsolationMode();
		EXPECT_EQ(TransactionIsolationMode::READ_COMMITTED, tiMode);

		// The rest is not supported by access
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			return;
		}

		tiMode = TransactionIsolationMode::READ_UNCOMMITTED;
		EXPECT_NO_THROW(m_pDb->SetTransactionIsolationMode(tiMode));
		tiMode = m_pDb->ReadTransactionIsolationMode();
		EXPECT_EQ(TransactionIsolationMode::READ_UNCOMMITTED, tiMode);

		tiMode = TransactionIsolationMode::REPEATABLE_READ;
		EXPECT_NO_THROW(m_pDb->SetTransactionIsolationMode(tiMode));
		tiMode = m_pDb->ReadTransactionIsolationMode();
		EXPECT_EQ(TransactionIsolationMode::REPEATABLE_READ, tiMode);

		tiMode = TransactionIsolationMode::SERIALIZABLE;
		EXPECT_NO_THROW(m_pDb->SetTransactionIsolationMode(tiMode));
		tiMode = m_pDb->ReadTransactionIsolationMode();
		EXPECT_EQ(TransactionIsolationMode::SERIALIZABLE, tiMode);
	}


	TEST_F(DatabaseTest, CanSetTransactionIsolationMode)
	{
		EXPECT_TRUE(m_pDb->CanSetTransactionIsolationMode(TransactionIsolationMode::READ_COMMITTED));
		// Those are not supported by Access, for the rest we expect support
		if (m_pDb->GetDbms() != DatabaseProduct::ACCESS)
		{
			EXPECT_TRUE(m_pDb->CanSetTransactionIsolationMode(TransactionIsolationMode::READ_UNCOMMITTED));
			EXPECT_TRUE(m_pDb->CanSetTransactionIsolationMode(TransactionIsolationMode::REPEATABLE_READ));
			EXPECT_TRUE(m_pDb->CanSetTransactionIsolationMode(TransactionIsolationMode::SERIALIZABLE));
		}
		EXPECT_FALSE(m_pDb->CanSetTransactionIsolationMode(TransactionIsolationMode::UNKNOWN));
	}


	TEST_F(DatabaseTest, Open)
	{		
		// Open an existing db by passing the Env to the ctor
		EnvironmentPtr pEnv = std::make_shared<Environment>(OdbcVersion::V_3);
		//Environment env(OdbcVersion::V_3);
		ASSERT_TRUE(pEnv->IsEnvHandleAllocated());
		Database db(pEnv);
		if (g_odbcInfo.HasConnectionString())
		{
			EXPECT_NO_THROW(db.Open(g_odbcInfo.m_connectionString));
		}
		else
		{
			EXPECT_NO_THROW(db.Open(g_odbcInfo.m_dsn, g_odbcInfo.m_username, g_odbcInfo.m_password));
		}
		EXPECT_NO_THROW(db.Close());

		// Open an existing db using the default c'tor and setting params on db
		Database db2;
		ASSERT_NO_THROW(db2.Init(pEnv));
		if (g_odbcInfo.HasConnectionString())
		{
			// also test that the returned connection-string is not empty
			string outConnStr;
			EXPECT_NO_THROW(outConnStr = db2.Open(g_odbcInfo.m_connectionString));
			EXPECT_FALSE(outConnStr.empty());
		}
		else
		{
			EXPECT_NO_THROW(db2.Open(g_odbcInfo.m_dsn, g_odbcInfo.m_username, g_odbcInfo.m_password));
		}
		EXPECT_NO_THROW(db2.Close());

		// Try to open with a different password / user, expect to fail when opening the db.
		{
			Database failDb(m_pEnv);
			EXPECT_THROW(failDb.Open(u8"ThisDSNDoesNotExist", u8"NorTheUser", u8"WithThisPassword"), SqlResultException);
		}
	}


	TEST_F(DatabaseTest, Close)
	{
		// Try to close a db that really is open
		Database db1(m_pEnv);
		if (g_odbcInfo.HasConnectionString())
		{
			ASSERT_NO_THROW(db1.Open(g_odbcInfo.m_connectionString));
		}
		else
		{
			ASSERT_NO_THROW(db1.Open(g_odbcInfo.m_dsn, g_odbcInfo.m_username, g_odbcInfo.m_password));
		}

		EXPECT_NO_THROW(db1.Close());

		// Closing one that is not open must just do nothing, but should not fail
		Database db2(m_pEnv);
		EXPECT_NO_THROW(db2.Close());
	}


	TEST_F(DatabaseTest, OpenCloseOpenClose)
	{
		// subsequent open, close, open, close must be possible
		Database db1(m_pEnv);
		if (g_odbcInfo.HasConnectionString())
		{
			ASSERT_NO_THROW(db1.Open(g_odbcInfo.m_connectionString));
		}
		else
		{
			ASSERT_NO_THROW(db1.Open(g_odbcInfo.m_dsn, g_odbcInfo.m_username, g_odbcInfo.m_password));
		}

		EXPECT_NO_THROW(db1.Close());

		// and open and close again
		if (g_odbcInfo.HasConnectionString())
		{
			EXPECT_NO_THROW(db1.Open(g_odbcInfo.m_connectionString));
		}
		else
		{
			EXPECT_NO_THROW(db1.Open(g_odbcInfo.m_dsn, g_odbcInfo.m_username, g_odbcInfo.m_password));
		}

		EXPECT_NO_THROW(db1.Close());
	}


	TEST_F(DatabaseTest, ReadCommitMode)
	{
		// We default to manual commit
		EXPECT_EQ(CommitMode::MANUAL, m_pDb->ReadCommitMode());
	}

	TEST_F(DatabaseTest, SetCommitMode)
	{
		// We default to manual commit
		EXPECT_EQ(CommitMode::MANUAL, m_pDb->GetCommitMode());
		EXPECT_EQ(CommitMode::MANUAL, m_pDb->ReadCommitMode());

		// Switch to auto
		EXPECT_NO_THROW(m_pDb->SetCommitMode(CommitMode::AUTO));
		// internal member should already be updated
		EXPECT_EQ(CommitMode::AUTO, m_pDb->GetCommitMode());
		EXPECT_EQ(CommitMode::AUTO, m_pDb->ReadCommitMode());

		// and back to manual
		EXPECT_NO_THROW(m_pDb->SetCommitMode(CommitMode::MANUAL));
		// internal member should already be updated
		EXPECT_EQ(CommitMode::MANUAL, m_pDb->GetCommitMode());
		EXPECT_EQ(CommitMode::MANUAL, m_pDb->ReadCommitMode());
	}


	TEST_F(DatabaseTest, DetectDbms)
	{
		// We just know how we name the different odbc-sources
		// TODO: This is not nice, but is there any other reliable way? Add to doc somewhere

		if (g_odbcInfo.HasConnectionString())
		{
			LOG_WARNING(u8"Skipping Test DetectDbms, because Test is implemented to stupid it only works with a named DSN");
			return;
		}

		if(boost::algorithm::find_first(g_odbcInfo.m_dsn, u8"DB2"))
		{
			EXPECT_TRUE(m_pDb->GetDbms() == DatabaseProduct::DB2);
		}
		else if(boost::algorithm::find_first(g_odbcInfo.m_dsn, u8"MySql"))
		{
			EXPECT_TRUE(m_pDb->GetDbms() == DatabaseProduct::MY_SQL);
		}
		else if(boost::algorithm::find_first(g_odbcInfo.m_dsn, u8"SqlServer"))
		{
			EXPECT_TRUE(m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER);
		}
		else if (boost::algorithm::find_first(g_odbcInfo.m_dsn, u8"Access"))
		{
			EXPECT_TRUE(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
		}
		else
		{
			EXPECT_EQ(std::string(u8"Unknown DSN name"), g_odbcInfo.m_dsn);
		}
	}


	TEST_F(DatabaseTest, CommitTransaction)
	{
		ClearTmpTable(TableId::INTEGERTYPES_TMP);

		// insert something, but do not commit, just close the Db
		DatabasePtr pDb = OpenTestDb();
		string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		string idColName = GetIdColumnName(TableId::INTEGERTYPES_TMP);
		string schemaName = u8"exodbc.";
		if (pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			schemaName = u8"";
		}
		string sqlInsert = boost::str(boost::format(u8"INSERT INTO %s%s (%s) VALUES (101)") %schemaName %tableName %idColName);

		pDb->ExecSql(sqlInsert);
		pDb->Close();

		// Reading again should not show any record at all
		pDb = OpenTestDb();
		string sqlCount = boost::str(boost::format(u8"SELECT COUNT(*) FROM %s%s") % schemaName % tableName);
		pDb->ExecSql(sqlCount);
		SQLRETURN ret = SQLFetch(pDb->GetExecSqlHandle()->GetHandle());
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		SQLINTEGER count;
		SQLLEN cb;
		GetData(pDb->GetExecSqlHandle(), 1, SQL_C_SLONG, &count, sizeof(count), &cb, NULL);
		EXPECT_EQ(0, count);

		// now insert again and commit
		pDb->ExecSql(sqlInsert);
		EXPECT_NO_THROW(pDb->CommitTrans());
		pDb->Close();

		// Reading again must show the record inserted
		pDb = OpenTestDb();
		pDb->ExecSql(sqlCount);
		ret = SQLFetch(pDb->GetExecSqlHandle()->GetHandle());
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		GetData(pDb->GetExecSqlHandle(), 1, SQL_C_SLONG, &count, sizeof(count), &cb, NULL);
		EXPECT_EQ(1, count);
	}


	TEST_F(DatabaseTest, RollbackTransaction)
	{
		ClearTmpTable(TableId::INTEGERTYPES_TMP);

		// insert something, but do not commit
		DatabasePtr pDb = OpenTestDb();
		string tableName = GetTableName(TableId::INTEGERTYPES_TMP);
		string idColName = GetIdColumnName(TableId::INTEGERTYPES_TMP);
		string schemaName = u8"exodbc.";
		if (pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			schemaName = u8"";
		}
		string sqlInsert = boost::str(boost::format(u8"INSERT INTO %s%s (%s) VALUES (101)") % schemaName %tableName %idColName);

		pDb->ExecSql(sqlInsert);
		// now rollback
		EXPECT_NO_THROW(pDb->RollbackTrans());
		pDb->Close();

		// Reading again should not show any record at all
		pDb = OpenTestDb();
		string sqlCount = boost::str(boost::format(u8"SELECT COUNT(*) FROM %s%s") % schemaName % tableName);
		pDb->ExecSql(sqlCount);
		SQLRETURN ret = SQLFetch(pDb->GetExecSqlHandle()->GetHandle());
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		SQLINTEGER count;
		SQLLEN cb;
		GetData(pDb->GetExecSqlHandle(), 1, SQL_C_SLONG, &count, sizeof(count), &cb, NULL);
		EXPECT_EQ(0, count);
	}


	TEST_F(DatabaseTest, ReadTablePrivileges)
	{
		TablePrivilegesVector privs;
		std::string tableName;
		std::string schemaName;
		std::string catalogName;
		std::string typeName;
		switch(m_pDb->GetDbms())
		{
		case DatabaseProduct::MY_SQL:
			// We know that mySql uses catalogs, not schemas:
			tableName = u8"integertypes";
			schemaName = u8"";
			catalogName = u8"exodbc";
			typeName = u8"";
			LOG_WARNING(u8"This test is known to fail with MySQL, see Ticket #76");
			break;
		case DatabaseProduct::DB2:
			// We know that DB2 uses schemas:
			tableName = u8"INTEGERTYPES";
			schemaName = u8"EXODBC";
			catalogName = u8"";
			typeName = u8"";
			break;
		case DatabaseProduct::MS_SQL_SERVER:
			// And ms uses catalogs, which map to dbs and schemaName
			tableName = u8"integertypes";
			schemaName = u8"exodbc";
			catalogName = u8"exodbc";
			typeName = u8"";
			break;
		case DatabaseProduct::ACCESS:
			// access only tablenames
			tableName = u8"integertypes";
		}
		// \todo: This is simply not working with MySQL, see Ticket #76
		EXPECT_NO_THROW(privs = m_pDb->ReadTablePrivileges(tableName, schemaName, catalogName, typeName));
		bool canSelect = false;
		bool canInsert = false;
		bool canDelete = false;
		bool canUpdate = false;
		TablePrivilegesVector::const_iterator it;
		for(it = privs.begin(); it != privs.end(); it++)
		{
			const STablePrivilegesInfo& priv = *it;
			if(priv.m_privilege == u8"SELECT")
				canSelect = true;
			if(priv.m_privilege == u8"INSERT")
				canInsert = true;
			if(priv.m_privilege == u8"DELETE")
				canDelete = true;
			if(priv.m_privilege == u8"UPDATE")
				canUpdate = true;
		}
		EXPECT_TRUE(canSelect);
		EXPECT_TRUE(canInsert);
		EXPECT_TRUE(canUpdate);
		EXPECT_TRUE(canDelete);
	}



	TEST_F(DatabaseTest, ReadColumnCount)
	{
		std::string tableName = u8"";
		std::string schemaName = u8"";
		std::string catalogName = u8"";
		std::string typeName = u8"";
		int nrCols = 4;
		switch(m_pDb->GetDbms())
		{
		case DatabaseProduct::DB2:
			// DB2 has schemas
			tableName = u8"INTEGERTYPES";
			schemaName = u8"EXODBC";
			break;
		case DatabaseProduct::MY_SQL:
			// mysql has catalogs
			tableName = u8"integertypes";
			catalogName = u8"exodbc";
			break;
		case DatabaseProduct::MS_SQL_SERVER:
			// ms has catalogs and schemas
			tableName = u8"integertypes";
			catalogName = u8"exodbc";
			schemaName = u8"exodbc";
			break;
		case DatabaseProduct::ACCESS:
			// access only tablenames
			tableName = u8"integertypes";
			break;
		}
		EXPECT_EQ(nrCols, m_pDb->ReadColumnCount(tableName, schemaName, catalogName, typeName));
		// we should also work if we just search by the tableName, as long as tableName is unique within db
		EXPECT_EQ(nrCols, m_pDb->ReadColumnCount(tableName, u8"", u8"", u8""));
	}
} //namespace exodbc
