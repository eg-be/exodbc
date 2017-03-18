/*!
 * \file UnicodeTest.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 17.03.2017
 * \copyright GNU Lesser General Public License Version 3
 * 
 * Test for reading / writing unicode strings.
 */ 

// Own header
#include "UnicodeTest.h"

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
	void UnicodeTest::SetUp()
	{
		ASSERT_TRUE(g_odbcInfo.IsUsable());

		// Set up Env
		// Try to set to the ODBC v3 : We need that for the tests to run correct. 3.8 is not supported by all databases and we dont use specific stuff from it.
		// except for the TIME2 things, sql server specific. those tests can create their own env.
		m_pEnv->Init(OdbcVersion::V_3);

		// And database
		ASSERT_NO_THROW(m_pDb = OpenTestDb(m_pEnv));
	}


	void UnicodeTest::TearDown()
	{
		if (m_pDb->IsOpen())
		{
			EXPECT_NO_THROW(m_pDb->Close());
		}
	}
	

	TEST_F(UnicodeTest, OpenAuto)
	{
		// Test that for every database auto columns with utf8 data can be created
		Table uTable(m_pDb, TableAccessFlag::AF_READ, GetTableName(TableId::UNICODE_TABLE));

		uTable.Open();
		EXPECT_EQ(2, uTable.GetColumnBufferCount());
		auto unicodeCol = uTable.GetColumnBufferPtrVariant(1);
		
		// Depending on the database used, the concrete column type created could be any of
		// WCharColumnBuffer or CharColumnBuffer
		// But all should have a column with a query name ending with 'content' (the
		// name of the column)
		string queryName = boost::apply_visitor(QueryNameVisitor(), unicodeCol);
		EXPECT_TRUE(boost::algorithm::iends_with(queryName, u8"content"));
	}


	TEST_F(UnicodeTest, CreatedUnicodeColumn)
	{
		// Test that we can read utf-8 data as SQL_C_CHAR
		Table u8Table(m_pDb, TableAccessFlag::AF_READ, GetTableName(TableId::UNICODE_TABLE));

		// For utf-8 data enforce binding to char instead of wchar
		//auto bindMap = std::make_shared<DefaultSql2BufferMap>(OdbcVersion::V_3);
		//bindMap->RegisterType(SQL_WVARCHAR, SQL_C_CHAR);
		//// In sql server we used a varbinary column in the table.
		//// lets override binding VARBINARY to SQL_C_BINARAY (which is SQL_C_CHAR probably anyway)
		//if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		//{
		//	bindMap->RegisterType(SQL_VARBINARY, SQL_C_CHAR);
		//}
		//u8Table.SetSql2BufferTypeMap(bindMap);

		//u8Table.Open();
		//string idColName = GetIdColumnName(TableId::UNICODE_TABLE);
		//string sqlWhere = boost::str(boost::format(u8"%s = 1") % idColName);
		//u8Table.Select(sqlWhere);
		//u8Table.SelectNext();
		
		//auto Content;
		//if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		//{
		//	auto pContent = u8Table.GetColumnBufferPtr<BinaryColumnBufferPtr>(1);
		//	string s1 = pContent->GetString();
		//	string s2 = u8"После смерти отца оставил учёбу и поступил на службу газетным репортёром";
		//	int p = 3;
		//}
		//else
		//{
		//	auto pContent = u8Table.GetColumnBufferPtr<WCharColumnBufferPtr>(1);
		//	wstring s1 = pContent->GetWString();
		//	string s2 = u8"После смерти отца оставил учёбу и поступил на службу газетным репортёром";
		//	wstring w3 = utf8ToUtf16(s2);
		//	EXPECT_EQ(w3, s1);
		//	int p = 3;
		//}
		//int p = 3;
	}

} // namespace exodbctest
