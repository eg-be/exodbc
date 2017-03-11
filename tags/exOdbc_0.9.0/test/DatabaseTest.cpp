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
		ASSERT_TRUE(g_odbcInfo.IsUsable());

		m_pEnv->Init(OdbcVersion::V_3);
		ASSERT_NO_THROW(m_pDb = OpenTestDb(m_pEnv));
	}

	void DatabaseTest::TearDown()
	{
		if(m_pDb->IsOpen())
		{
			EXPECT_NO_THROW(m_pDb->Close());
		}
	}


	TEST_F(DatabaseTest, CopyConstructor)
	{
		Database db1(m_pEnv);
		Database c1(db1);

		EXPECT_TRUE(c1.HasConnectionHandle());
		{
			EXPECT_THROW(c1.Init(m_pEnv), AssertionException);
		}

		Database db2;
		Database c2(db2);
		EXPECT_FALSE(c2.HasConnectionHandle());
		EXPECT_NO_THROW(c2.Init(m_pEnv));
	}


	TEST_F(DatabaseTest, PrintDatabaseInfo)
	{
		LOG_INFO(u8"Will print Database Information now");
		LOG_INFO(u8"===================================");
		LOG_INFO(m_pDb->GetDbInfo().ToString());
		LOG_INFO(u8"===================================");
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


	TEST_F(DatabaseTest, GetTracefile)
	{
		string tracefile;
		EXPECT_NO_THROW(tracefile = m_pDb->GetTracefile());
		LOG_INFO(boost::str(boost::format(u8"Tracefile is: '%s'") % tracefile));
	}


	TEST_F(DatabaseTest, SetTracefile)
	{
        boost::filesystem::path tmpDir = boost::filesystem::temp_directory_path();
		string filename = boost::str(boost::format(u8"trace_%s.log") % DatabaseProcudt2s(m_pDb->GetDbms()));
		boost::filesystem::path tracefilePath = tmpDir / filename;

#ifdef _WIN32
		string tracefilePathStr = utf16ToUtf8(tracefilePath.native());
#else
		string tracefilePathStr = tracefilePath.native();
#endif

		EXPECT_NO_THROW(m_pDb->SetTracefile(tracefilePathStr));
	}


	TEST_F(DatabaseTest, SetTrace)
	{
		Database db(m_pEnv);
		EXPECT_NO_THROW(db.SetTrace(true));

		// and disable again
		EXPECT_NO_THROW(db.SetTrace(false));
	}


	TEST_F(DatabaseTest, GetTrace)
	{
		Database db(m_pEnv);
		ASSERT_NO_THROW(db.SetTrace(true));
		EXPECT_TRUE(db.GetTrace());
		ASSERT_NO_THROW(db.SetTrace(false));
		EXPECT_FALSE(db.GetTrace());
	}


	TEST_F(DatabaseTest, DoSomeTrace)
	{
		namespace fs = boost::filesystem;

		// Set tracefile first, do that on a db not open yet
		boost::filesystem::path tmpDir = boost::filesystem::temp_directory_path();
		string filename = boost::str(boost::format(u8"trace_%s.log") % DatabaseProcudt2s(m_pDb->GetDbms()));
		boost::filesystem::path tracefilePath = tmpDir / filename;

#ifdef _WIN32
		string tracefilePathStr = utf16ToUtf8(tracefilePath.native());
#else
		string tracefilePathStr = tracefilePath.native();
#endif

		// remove an existing trace-file
		boost::system::error_code fserr;
		fs::remove(tracefilePath, fserr);
		ASSERT_FALSE(fs::exists(tracefilePath));

		Database db(m_pEnv);
		ASSERT_NO_THROW(db.SetTracefile(tracefilePathStr));
		EXPECT_NO_THROW(db.SetTrace(true));

		// Open db
		if (g_odbcInfo.HasConnectionString())
		{
			db.Open(g_odbcInfo.m_connectionString);
		}
		else
		{
			db.Open(g_odbcInfo.m_dsn, g_odbcInfo.m_username, g_odbcInfo.m_password);
		}

		// now one must exist
		EXPECT_TRUE(fs::exists(tracefilePath));
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


	TEST_F(DatabaseTest, ReadDataTypesInfo)
	{
		std::vector<SSqlTypeInfo> types;
		ASSERT_NO_THROW(types = m_pDb->ReadDataTypesInfo());
		EXPECT_TRUE(types.size() > 0);

		std::stringstream ws;
		ws << u8"TypeInfo of database with DSN '" << (g_odbcInfo.HasConnectionString() ? g_odbcInfo.m_connectionString : g_odbcInfo.m_dsn) << u8"', total " << types.size() << u8" types reported:" << std::endl;
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

		LOG_INFO(ws.str());
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


	TEST_F(DatabaseTest, ReadCatalogs)
	{
 		std::vector<std::string> cats;
		ASSERT_NO_THROW(cats = m_pDb->ReadCatalogs());
		switch(m_pDb->GetDbms())
		{
		case DatabaseProduct::DB2:
			// DB2 does not support catalogs. it reports zero catalogs
			EXPECT_EQ(0, cats.size());
			break;

		case DatabaseProduct::MS_SQL_SERVER:
		case DatabaseProduct::MY_SQL:
			// Those report our test-db as a catalog
			EXPECT_TRUE(std::find(cats.begin(), cats.end(), u8"exodbc") != cats.end());
			break;
		case DatabaseProduct::ACCESS:
			// Returns the file-path as catalog, must be our file named 'exodbc' (note: .mdb is not in the path)
			EXPECT_EQ(1, cats.size());
			EXPECT_TRUE(boost::algorithm::ends_with(cats[0], u8"exodbc"));
			break;
		}
	}

	TEST_F(DatabaseTest, ReadSchemas)
	{
		std::vector<std::string> schemas;
		EXPECT_NO_THROW(schemas = m_pDb->ReadSchemas());
		switch(m_pDb->GetDbms())
		{
		case DatabaseProduct::DB2:
			// DB2 reports our test-db as a schema
			EXPECT_TRUE(schemas.size() > 0);
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), u8"EXODBC") != schemas.end());
			break;
		case DatabaseProduct::MY_SQL:
			// Mysql reports one schema with an empty name
			EXPECT_TRUE(schemas.size() == 1);
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), u8"") != schemas.end());
			break;
		case DatabaseProduct::MS_SQL_SERVER:
			// ms sql server reported at least 3 schemas:
			EXPECT_TRUE(schemas.size() >= 3);
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), u8"exodbc") != schemas.end());
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), u8"INFORMATION_SCHEMA") != schemas.end());
			EXPECT_TRUE(std::find(schemas.begin(), schemas.end(), u8"sys") != schemas.end());
			break;
		}
	}


	TEST_F(DatabaseTest, ReadTableTypes)
	{
		std::vector<std::string> tableTypes;
		EXPECT_NO_THROW(tableTypes = m_pDb->ReadTableTypes());
		// Check that we have at least a type TABLE and a type VIEW
		EXPECT_TRUE(std::find(tableTypes.begin(), tableTypes.end(), u8"TABLE") != tableTypes.end());
		EXPECT_TRUE(std::find(tableTypes.begin(), tableTypes.end(), u8"VIEW") != tableTypes.end());
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


	TEST_F(DatabaseTest, ReadTablePrimaryKeysInfo)
	{
		TablePrimaryKeysVector pks;
		
		// Find the table-info
		TableInfo iInfo;
		string intTableName = GetTableName(TableId::INTEGERTYPES);
		string idName = GetIdColumnName(TableId::INTEGERTYPES);
		ASSERT_NO_THROW(iInfo = m_pDb->FindOneTable(intTableName, u8"", u8"", u8""));

		EXPECT_NO_THROW(pks = m_pDb->ReadTablePrimaryKeys(iInfo));
		EXPECT_EQ(1, pks.size());
		if (pks.size() == 1)
		{
			EXPECT_EQ(idName, pks[0].GetColumnName());
		}

		TableInfo mkInfo;
		string multiKeyTableName = GetTableName(TableId::MULTIKEY);
		string mkId1 = ToDbCase(u8"id1");
		string mkId2 = ToDbCase(u8"id2");
		string mkId3 = ToDbCase(u8"id3");
		EXPECT_NO_THROW(mkInfo = m_pDb->FindOneTable(multiKeyTableName, u8"", u8"", u8""));

		EXPECT_NO_THROW(pks = m_pDb->ReadTablePrimaryKeys(mkInfo));
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
		std::string tableName;
		std::string schemaName;
		std::string catalogName;
		std::string typeName;
		switch(m_pDb->GetDbms())
		{
		case DatabaseProduct::MY_SQL:
			// We know that mySql uses catalogs, not schemas:
			tableName = u8"numerictypes";
			schemaName = u8"";
			catalogName = u8"exodbc";
			typeName = u8"";
			break;
		case DatabaseProduct::DB2:
			// We know that DB2 uses schemas:
			tableName = u8"NUMERICTYPES";
			schemaName = u8"EXODBC";
			catalogName = u8"";
			typeName = u8"";
			break;
		case DatabaseProduct::MS_SQL_SERVER:
			// And ms uses catalogs, which map to dbs and schemaName
			tableName = u8"numerictypes";
			schemaName = u8"exodbc";
			catalogName = u8"exodbc";
			typeName = u8"";
			break;
		case DatabaseProduct::ACCESS:
			tableName = u8"integertypes";
			break;
		}
		EXPECT_NO_THROW(cols = m_pDb->ReadTableColumnInfo(tableName, schemaName, catalogName, typeName));
		// Note: Access reads a different table
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
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
		std::string tableName;
		std::string schemaName;
		std::string catalogName;
		switch(m_pDb->GetDbms())
		{
		case DatabaseProduct::MY_SQL:
			// We know that mySql uses catalogs, not schemas:
			tableName = u8"integertypes";
			schemaName = u8"";
			catalogName = u8"exodbc";
			break;
		case DatabaseProduct::DB2:
			// We know that DB2 uses schemas (and uppercase):
			tableName = u8"INTEGERTYPES";
			schemaName = u8"EXODBC";
			catalogName = u8"";
			break;
		case DatabaseProduct::MS_SQL_SERVER:
			// And ms uses catalogs, which map to dbs and schemaName
			tableName = u8"integertypes";
			schemaName = u8"exodbc";
			catalogName = u8"exodbc";
			break;
		case DatabaseProduct::ACCESS:
			// only tablenames
			tableName = u8"integertypes";
			break;
		default:
			LOG_ERROR(u8"Dbms not known, cannot run test!");
			ASSERT_TRUE(false);
		}
		// Find one table by using only the table-name as search param
		EXPECT_NO_THROW(tables = m_pDb->FindTables(tableName, u8"", u8"", u8""));
		EXPECT_EQ(1, tables.size());
		// Find one table by using table-name and schema/catalog
		EXPECT_NO_THROW(tables = m_pDb->FindTables(tableName, schemaName, catalogName, u8""));
		EXPECT_EQ(1, tables.size());
		// In all cases, we should not find anything if we use a schema or a catalog that does not exist
		// \todo: Create Ticket (Info): Note: When using MySQL-Odbc driver, if 'do not use INFORMATION_SCHEMA' is set, this will fail due to an access denied for database "wrongCatalog"
		// Access has no support for schemas
		if (m_pDb->GetDbms() != DatabaseProduct::ACCESS)
		{
			EXPECT_NO_THROW(tables = m_pDb->FindTables(tableName, u8"WrongSchema", u8"", u8""));
			EXPECT_EQ(0, tables.size());
		}
		EXPECT_NO_THROW(tables = m_pDb->FindTables(tableName, u8"", u8"WrongCatalog", u8""));
		EXPECT_EQ(0, tables.size());
		// Also, we have a table, not a view
		EXPECT_NO_THROW(tables = m_pDb->FindTables(tableName, u8"", u8"", u8"VIEW"));
		EXPECT_EQ(0, tables.size());
		EXPECT_NO_THROW(tables = m_pDb->FindTables(tableName, u8"", u8"", u8"TABLE"));
		EXPECT_EQ(1, tables.size());
		// What about search-patterns? Note: Not use for table-type
		// search patterns are not supported for schemas by Access:
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			EXPECT_NO_THROW(tables = m_pDb->FindTables(tableName, u8"", u8"%", u8""));
		}
		else
		{
			EXPECT_NO_THROW(tables = m_pDb->FindTables(tableName, u8"%", u8"%", u8""));
		}
		EXPECT_EQ(1, tables.size());
		if(tables.size() > 0)
		{
			EXPECT_EQ(tableName, tables[0].GetPureName());
			EXPECT_EQ(schemaName, tables[0].GetSchema());
			if (m_pDb->GetDbms() != DatabaseProduct::ACCESS)
			{
				// still no support for catalogs on Access
				EXPECT_EQ(catalogName, tables[0].GetCatalog());
			}
		}
		std::string schemaPattern = schemaName;
		if(schemaName.length() > 0)
			schemaPattern = (boost::format(u8"%%%s%%") %schemaName).str();
		std::string catalogPattern = catalogName;
		if(catalogName.length() > 0)
			catalogPattern = (boost::format(u8"%%%s%%") %catalogName).str();
		EXPECT_NO_THROW(tables = m_pDb->FindTables(tableName, schemaPattern, catalogPattern, u8""));
		EXPECT_EQ(1, tables.size());
		if(tables.size() > 0)
		{
			EXPECT_EQ(tableName, tables[0].GetPureName());
			EXPECT_EQ(schemaName, tables[0].GetSchema());
			if (m_pDb->GetDbms() != DatabaseProduct::ACCESS)
			{
				// still no support for catalogs on Access
				EXPECT_EQ(catalogName, tables[0].GetCatalog());
			}
		}
	}


	TEST_F(DatabaseTest, ReadCompleteCatalog)
	{
		SDbCatalogInfo cat;
		EXPECT_NO_THROW( cat = m_pDb->ReadCompleteCatalog());
		// TODO: This is confusing. DB2 reports schemas, what is correct, but mysql reports catalogs?? wtf?
		if (m_pDb->GetDbms() == DatabaseProduct::DB2)
		{
			EXPECT_TRUE(cat.m_schemas.find(u8"EXODBC") != cat.m_schemas.end());
		}
		else if (m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
		{
			EXPECT_TRUE(cat.m_catalogs.find(u8"exodbc") != cat.m_catalogs.end());
		}
		EXPECT_GE((SQLINTEGER) cat.m_tables.size(), 12);
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


	TEST_F(DatabaseTest, ReadSpecialColumns)
	{
		// The integertypes table has an id column that should be unique inside a cursor.
		// but others might change the value of a row identified by id during a transaction, etc.

		string intTableName = GetTableName(TableId::INTEGERTYPES);
		TableInfo intTableInfo = m_pDb->FindOneTable(intTableName, u8"", u8"", u8"");
		
		SpecialColumnInfosVector specColsTransaction = m_pDb->ReadSpecialColumns(intTableInfo, IdentifierType::IDENTIFY_ROW_UNIQUELY, RowIdScope::TRANSCATION);
		SpecialColumnInfosVector specColsSession = m_pDb->ReadSpecialColumns(intTableInfo, IdentifierType::IDENTIFY_ROW_UNIQUELY, RowIdScope::SESSION);
		SpecialColumnInfosVector specColsCursor = m_pDb->ReadSpecialColumns(intTableInfo, IdentifierType::IDENTIFY_ROW_UNIQUELY, RowIdScope::CURSOR);
		EXPECT_EQ(1, specColsCursor.size());

		string autoIdTableName = GetTableName(TableId::MULTIKEY);
		TableInfo autoIdTableInfo = m_pDb->FindOneTable(autoIdTableName, u8"", u8"", u8"");

		specColsTransaction = m_pDb->ReadSpecialColumns(autoIdTableInfo, IdentifierType::IDENTIFY_ROW_UNIQUELY, RowIdScope::TRANSCATION);
		specColsSession = m_pDb->ReadSpecialColumns(autoIdTableInfo, IdentifierType::IDENTIFY_ROW_UNIQUELY, RowIdScope::SESSION);
		specColsCursor = m_pDb->ReadSpecialColumns(autoIdTableInfo, IdentifierType::IDENTIFY_ROW_UNIQUELY, RowIdScope::CURSOR);
		EXPECT_EQ(3, specColsCursor.size());

	}

} //namespace exodbc
