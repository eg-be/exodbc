/*!
 * \file Utf8Test.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 17.03.2017
 * \copyright GNU Lesser General Public License Version 3
 * 
 * Test for reading / writing utf8 strings.
 */ 

// Own header
#include "Utf8Test.h"

// Same component headers
#include "exOdbcGTest.h"
#include "exOdbcGTestHelpers.h"

// Other headers
#include "exodbc/Environment.h"
#include "exodbc/Table.h"
#include "exodbc/ColumnBufferVisitors.h"

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
	void Utf8Test::SetUp()
	{
		ASSERT_TRUE(g_odbcInfo.IsUsable());

		// Set up Env
		// Try to set to the ODBC v3 : We need that for the tests to run correct. 3.8 is not supported by all databases and we dont use specific stuff from it.
		// except for the TIME2 things, sql server specific. those tests can create their own env.
		m_pEnv->Init(OdbcVersion::V_3);

		// And database
		ASSERT_NO_THROW(m_pDb = OpenTestDb(m_pEnv));
	}


	void Utf8Test::TearDown()
	{
		if (m_pDb->IsOpen())
		{
			EXPECT_NO_THROW(m_pDb->Close());
		}
	}
	

	TEST_F(Utf8Test, OpenAsSqlChar)
	{
		// Test that for every database auto columns with utf8 data can be created
		Table t(m_pDb, TableAccessFlag::AF_READ, GetTableName(TableId::UTF8_TABLE));

		// For utf-8 data enforce binding to char instead of wchar
		auto bindMap = std::make_shared<DefaultSql2BufferMap>(OdbcVersion::V_3);
		bindMap->RegisterType(SQL_WVARCHAR, SQL_C_CHAR);
		// In sql server we used a varbinary column in the table.
		// lets override binding VARBINARY to SQL_C_BINARAY (which is SQL_C_CHAR probably anyway)
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			bindMap->RegisterType(SQL_VARBINARY, SQL_C_CHAR);
		}
		t.SetSql2BufferTypeMap(bindMap);

		t.Open();
		EXPECT_EQ(2, t.GetColumnBufferCount());
		auto utf8Col = t.GetColumnBufferPtrVariant(1);
		
		// Test if we now have a binary SQL_C_CHAR column that might hold our utf-8 data:
		SqlCTypeVisitor cTypeV;
		//SQLSMALLINT t1 = boost::apply_visitor(cTypeV, utf8Col);
		//string s = SqlCType2OdbcS(t1);
		EXPECT_EQ(SQL_C_CHAR, boost::apply_visitor(cTypeV, utf8Col));
	}

} // namespace exodbctest
