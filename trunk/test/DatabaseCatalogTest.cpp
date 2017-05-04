/*!
 * \file DatabaseCatalogTest.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 20.04.2017
 * \copyright GNU Lesser General Public License Version 3
 * 
 * [Brief CPP-file description]
 */ 

// Own header
#include "DatabaseCatalogTest.h"

// Same component headers
#include "ManualTestTables.h"
#include "exOdbcGTestHelpers.h"

// Other headers
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/DatabaseCatalog.h"

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
	void DatabaseCatalogTest::SetUp()
	{
		// Set up is called for every test
		m_pEnv->Init(OdbcVersion::V_3);
		m_pDb = OpenTestDb(m_pEnv);

	}


	void DatabaseCatalogTest::TearDown()
	{
		if(m_pDb->IsOpen())
		{
			m_pDb->Close();
		}
	}


	TEST_F(DatabaseCatalogTest, Init)
	{
		DatabaseCatalog dbCat;

		// Fail for not allocated or connected connection handle
		{
			SqlDbcHandlePtr pDbc = SqlDbcHandle::Create(m_pEnv->GetSqlEnvHandle());
			LogLevelSetter ll(LogLevel::None);
			EXPECT_THROW(dbCat.Init(pDbc, SqlInfoProperties()), AssertionException);
		}

		// But succeed for valid connection
		EXPECT_NO_THROW(dbCat.Init(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties()));
	}


	TEST_F(DatabaseCatalogTest, EscapePatternValueArguments)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		string pattern = u8"Foo%Ba_r";
		string escaped = dbCat.EscapePatternValueArguments(pattern);
		
		string escSeq = dbCat.GetSearchPatternEscape();
		string exp = boost::str(boost::format(u8"Foo%s%%Ba%s_r") % escSeq % escSeq);
		EXPECT_EQ(exp, escaped);
	}


	TEST_F(DatabaseCatalogTest, SearchTables)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// we should find some tables if not restricting anything
		TableInfoVector allTables = dbCat.SearchTables(u8"%");
		EXPECT_TRUE(allTables.size() > 0);

		// now restrict to tables named '%tmp' - we should have some in our test-db:
		// that should be less than all tables:
		TableInfoVector tmpTables = dbCat.SearchTables(u8"%tmp");
		EXPECT_GT(allTables.size(), tmpTables.size());
	}


	TEST_F(DatabaseCatalogTest, SearchTablesByTableName)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// we should find some tables if not restricting anything
		TableInfoVector allTables = dbCat.SearchTables(u8"%");
		EXPECT_TRUE(allTables.size() > 0);

		// now restrict to tables named '%tmp' - we should have some in our test-db:
		// that should be less than all tables:
		TableInfoVector tmpTables = dbCat.SearchTables(u8"%tmp");
		EXPECT_GT(allTables.size(), tmpTables.size());

		// And now restrict to matching really empty catalog / schema names:
		// that must be less or equal to all tables
		TableInfoVector emptyCatSchemTables = dbCat.SearchTables(u8"%", u8"", u8"", u8"");
		EXPECT_LE(emptyCatSchemTables.size(), allTables.size());
	}


	TEST_F(DatabaseCatalogTest, SearchPatternEscape)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// find some tmp table ending with '_tmp'
		string esc = dbCat.GetSearchPatternEscape();
		EXPECT_FALSE(esc.empty());

		string tableNamePattern = boost::str(boost::format(u8"%%%s_tmp") % esc);
		if (g_odbcInfo.m_namesCase == Case::UPPER)
			boost::algorithm::to_upper(tableNamePattern);
		TableInfoVector tmpTables = dbCat.SearchTables(tableNamePattern);
		EXPECT_FALSE(tmpTables.empty());

		// every table must end with _tmp
		for (TableInfoVector::const_iterator it = tmpTables.begin(); it != tmpTables.end(); ++it)
		{
			string tableName = it->GetName();
			EXPECT_TRUE(boost::algorithm::iends_with(tableName, u8"_tmp"));
		}
	}


	TEST_F(DatabaseCatalogTest, SearchTablesByDetectingSchemaOrCatalog)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// find some table
		string tableNamePattern = u8"integertypes";
		if (g_odbcInfo.m_namesCase == Case::UPPER)
		{
			boost::algorithm::to_upper(tableNamePattern);
		}
		TableInfoVector tables = dbCat.SearchTables(tableNamePattern);
		ASSERT_FALSE(tables.empty());

		// can only do test if table has either catalog or schema only
		TableInfo ti = tables.front();
		TableInfoVector tables2;
		if ((ti.HasSchema() && !ti.HasCatalog()) || (!ti.HasSchema() && ti.HasCatalog()))
		{
			string schemaOrCatalogName = ti.HasSchema() ? ti.GetSchema() : ti.GetCatalog();
			tables2 = dbCat.SearchTables(ti.GetName(), schemaOrCatalogName, ti.GetType());
		}
		else
		{
			// cannot do test
			LOG_INFO(u8"Skipping test because not either schema or catalog was found");
			return;
		}
		ASSERT_EQ(1, tables2.size());
		EXPECT_EQ(ti, tables.front());
	}


	TEST_F(DatabaseCatalogTest, SearchTablesBySettingSchemaOrCatalog)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// find some table
		string tableNamePattern = u8"integertypes";
		if (g_odbcInfo.m_namesCase == Case::UPPER)
			boost::algorithm::to_upper(tableNamePattern);
		TableInfoVector tables = dbCat.SearchTables(tableNamePattern);
		ASSERT_FALSE(tables.empty());
		
		TableInfo ti = tables.front();
		TableInfoVector tables2;
		if(ti.HasCatalog())
		{
			tables2 = dbCat.SearchTables(ti.GetName(), ti.GetCatalog(), DatabaseCatalog::SchemaOrCatalogType::Catalog , ti.GetType());
		}
		else if (ti.HasSchema())
		{
			tables2 = dbCat.SearchTables(ti.GetName(), ti.GetSchema(), DatabaseCatalog::SchemaOrCatalogType::Schema, ti.GetType());
		}
		else
		{
			// cannot do test
			LOG_INFO(u8"Skipping test because nor schema or catalog was found");
			return;
		}
		ASSERT_EQ(1, tables2.size());
		EXPECT_EQ(ti, tables2.front());
	}


	TEST_F(DatabaseCatalogTest, SearchTablesBySchemaAndCatalog)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// find some table
		string tableNamePattern = u8"integertypes";
		if (g_odbcInfo.m_namesCase == Case::UPPER)
			boost::algorithm::to_upper(tableNamePattern);
		TableInfoVector tables = dbCat.SearchTables(tableNamePattern);
		ASSERT_FALSE(tables.empty());

		TableInfo ti = tables.front();
		TableInfoVector tables2;
		if (ti.HasCatalog() && ti.HasSchema())
		{
			tables2 = dbCat.SearchTables(ti.GetName(), ti.GetSchema(), ti.GetCatalog(), u8"");
		}
		else
		{
			// cannot do test
			LOG_INFO(u8"Skipping test because not schema and catalog was found");
			return;
		}
		ASSERT_EQ(1, tables2.size());
		EXPECT_EQ(ti, tables2.front());
	}


	TEST_F(DatabaseCatalogTest, FindOneTable)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// find one table
		string tableNamePattern = u8"integertypes";
		if (g_odbcInfo.m_namesCase == Case::UPPER)
			boost::algorithm::to_upper(tableNamePattern);
		EXPECT_NO_THROW(dbCat.FindOneTable(tableNamePattern));

		// Fail if we have multiple matching tables
		tableNamePattern = u8"integertypes%";
		if (g_odbcInfo.m_namesCase == Case::UPPER)
			boost::algorithm::to_upper(tableNamePattern);
		EXPECT_THROW(dbCat.FindOneTable(tableNamePattern), NotFoundException);
	}


	TEST_F(DatabaseCatalogTest, ListCatalogs)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		vector<string> catalogs = dbCat.ListCatalogs();
		if (dbCat.GetSupportsCatalogs())
		{
			EXPECT_FALSE(catalogs.empty());
		}
		else
		{
			LOG_INFO(u8"Skipping test because databse does not support catalogs");
		}
	}


	TEST_F(DatabaseCatalogTest, ListSchemas)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		vector<string> schemas = dbCat.ListSchemas();
		if (dbCat.GetSupportsSchemas())
		{
			EXPECT_FALSE(schemas.empty());
		}
		else
		{
			LOG_INFO(u8"Skipping test because databse does not support schemas");
		}
	}


	TEST_F(DatabaseCatalogTest, ListTableTypes)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		vector<string> types = dbCat.ListTableTypes();
		EXPECT_FALSE(types.empty());
	}


	TEST_F(DatabaseCatalogTest, Reset)
	{
		DatabaseCatalog dbCat;
		// Init once is okay
		dbCat.Init(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		// But twice must fail
		EXPECT_THROW(dbCat.Init(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties()), AssertionException);

		// But resetting it must allow to init again
		dbCat.Reset();
		EXPECT_NO_THROW(dbCat.Init(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties()));
	}


	TEST_F(DatabaseCatalogTest, ReadColumnInfo)
	{
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());

		// Find a table
		string tableNamePattern = u8"integertypes";
		if (g_odbcInfo.m_namesCase == Case::UPPER)
			boost::algorithm::to_upper(tableNamePattern);
		
		TableInfo intTable;
		ASSERT_NO_THROW(intTable = dbCat.FindOneTable(tableNamePattern));

		// And query all columns of this table:
		ColumnInfoVector colInfo = dbCat.ReadColumnInfo(intTable);
		ASSERT_EQ(4, colInfo.size());

		ColumnInfo col1 = colInfo[0];
		ColumnInfo col2 = colInfo[1];
		ColumnInfo col3 = colInfo[2];
		ColumnInfo col4 = colInfo[3];

		string col1Name = u8"idintegertypes";
		string col2Name = u8"tsmallint";
		string col3Name = u8"tint";
		string col4Name = u8"tbigint";
		if (g_odbcInfo.m_namesCase == Case::UPPER)
		{
			boost::algorithm::to_upper(col1Name);
			boost::algorithm::to_upper(col2Name);
			boost::algorithm::to_upper(col3Name);
			boost::algorithm::to_upper(col4Name);
		}
		EXPECT_EQ(col1Name, col1.GetColumnName());
		EXPECT_EQ(col2Name, col2.GetColumnName());
		EXPECT_EQ(col3Name, col3.GetColumnName());
		EXPECT_EQ(col4Name, col4.GetColumnName());
	}


	TEST_F(DatabaseCatalogTest, ReadNumericColumnInfo)
	{
		// some more tests, try to read a specific numeric column and
		// check that we get what we expect in the various members:

		// First identify numerictypes table:
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());

		// Find a table
		TableInfo numTable;
		ASSERT_NO_THROW(numTable = dbCat.FindOneTable(GetTableName(TableId::NUMERICTYPES)));

		// And query all columns of this table:
		ColumnInfoVector colInfo = dbCat.ReadColumnInfo(numTable);
		ASSERT_EQ(4, colInfo.size());

		// Check concrete numeric values of the 3rd column:
		ColumnInfo col = colInfo[2];
		EXPECT_FALSE(col.IsNumPrecRadixNull());
		EXPECT_FALSE(col.IsColumnSizeNull());
		EXPECT_FALSE(col.IsDecimalDigitsNull());
		EXPECT_EQ(10, col.GetNumPrecRadix());
		EXPECT_EQ(18, col.GetColumnSize());
		EXPECT_EQ(10, col.GetDecimalDigits());
	}


	TEST_F(DatabaseCatalogTest, ReadPrimaryKeyInfo)
	{
		PrimaryKeyInfoVector pks;

		// Find the table-info
		TableInfo iInfo;
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		ASSERT_NO_THROW(iInfo = dbCat.FindOneTable(GetTableName(TableId::INTEGERTYPES)));

		// And read the primary keys 
		pks = dbCat.ReadPrimaryKeyInfo(iInfo);
		ASSERT_EQ(1, pks.size());
		EXPECT_EQ(GetIdColumnName(TableId::INTEGERTYPES), pks[0].GetColumnName());

		// And check for a table with multiple keys:
		TableInfo mkInfo;
		string multiKeyTableName = GetTableName(TableId::MULTIKEY);
		string mkId1 = ToDbCase(u8"id1");
		string mkId2 = ToDbCase(u8"id2");
		string mkId3 = ToDbCase(u8"id3");
		mkInfo = dbCat.FindOneTable(multiKeyTableName);

		pks = dbCat.ReadPrimaryKeyInfo(mkInfo);
		ASSERT_EQ(3, pks.size());
		EXPECT_EQ(mkId1, pks[0].GetColumnName());
		EXPECT_EQ(1, pks[0].GetKeySequence());
		EXPECT_EQ(mkId2, pks[1].GetColumnName());
		EXPECT_EQ(2, pks[1].GetKeySequence());
		EXPECT_EQ(mkId3, pks[2].GetColumnName());
		EXPECT_EQ(3, pks[2].GetKeySequence());
	}


	TEST_F(DatabaseCatalogTest, ReadSqlDataTypeInfo)
	{
		std::vector<SqlTypeInfo> types;
		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());

		ASSERT_NO_THROW(types = dbCat.ReadSqlTypeInfo());
		EXPECT_TRUE(types.size() > 0);

		std::stringstream ws;
		ws << u8"TypeInfo of database with DSN '" << (g_odbcInfo.HasConnectionString() ? g_odbcInfo.m_connectionString : g_odbcInfo.m_dsn) << u8"', total " << types.size() << u8" types reported:" << std::endl;
		bool first = true;
		std::vector<SqlTypeInfo>::const_iterator it = types.begin();
		while (it != types.end())
		{
			SqlTypeInfo t = *it;

			++it;

			ws << t.ToOneLineStr(first, it == types.end()) << std::endl;
			if (first)
				first = false;

		}

		LOG_INFO(ws.str());
	}


	TEST_F(DatabaseCatalogTest, ReadSpecialColumnInfo)
	{
		// The integertypes table has an id column that should be unique inside a cursor.
		// but others might change the value of a row identified by id during a transaction, etc.

		DatabaseCatalog dbCat(m_pDb->GetSqlDbcHandle(), m_pDb->GetProperties());
		TableInfo intTableInfo = dbCat.FindOneTable(GetTableName(TableId::INTEGERTYPES));

		SpecialColumnInfoVector specColsTransaction = dbCat.ReadSpecialColumnInfo(intTableInfo, SpecialColumnInfo::IdentifierType::UNIQUE_ROW, SpecialColumnInfo::RowIdScope::TRANSCATION);
		SpecialColumnInfoVector specColsSession = dbCat.ReadSpecialColumnInfo(intTableInfo, SpecialColumnInfo::IdentifierType::UNIQUE_ROW, SpecialColumnInfo::RowIdScope::SESSION);
		SpecialColumnInfoVector specColsCursor = dbCat.ReadSpecialColumnInfo(intTableInfo, SpecialColumnInfo::IdentifierType::UNIQUE_ROW, SpecialColumnInfo::RowIdScope::CURSOR);
		EXPECT_EQ(1, specColsCursor.size());

		string autoIdTableName = GetTableName(TableId::MULTIKEY);
		TableInfo autoIdTableInfo = dbCat.FindOneTable(autoIdTableName, u8"", u8"", u8"");

		specColsTransaction = dbCat.ReadSpecialColumnInfo(autoIdTableInfo, SpecialColumnInfo::IdentifierType::UNIQUE_ROW, SpecialColumnInfo::RowIdScope::TRANSCATION);
		specColsSession = dbCat.ReadSpecialColumnInfo(autoIdTableInfo, SpecialColumnInfo::IdentifierType::UNIQUE_ROW, SpecialColumnInfo::RowIdScope::SESSION);
		specColsCursor = dbCat.ReadSpecialColumnInfo(autoIdTableInfo, SpecialColumnInfo::IdentifierType::UNIQUE_ROW, SpecialColumnInfo::RowIdScope::CURSOR);
		EXPECT_EQ(3, specColsCursor.size());

	}

} //namespace exodbc
