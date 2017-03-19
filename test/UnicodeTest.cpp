﻿/*!
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
		Table uTable(m_pDb, TableAccessFlag::AF_READ, GetTableName(TableId::UNICODE_TABLE));

		// Read all rows
		uTable.Open();
		string idColName = GetIdColumnName(TableId::UNICODE_TABLE);

		for (int i = 1; i <= 5; i++)
		{
			string sqlWhere = boost::str(boost::format(u8"%s = %d") % idColName % i);
			uTable.Select(sqlWhere);
			uTable.SelectNext();

			// Depending on the underlying database type, the driver may have reported a
			// wchar or a char column. Be flexible on the buffer type expected:
			auto contentColumnVariant = uTable.GetColumnBufferPtrVariant(1);
			SQLSMALLINT sqlType = boost::apply_visitor(SqlCTypeVisitor(), contentColumnVariant);
			string content;
			switch (sqlType)
			{
			case SQL_C_WCHAR:
			{
				auto wcharContent = uTable.GetColumnBufferPtr<WCharColumnBufferPtr>(1);
				wstring wContent = wcharContent->GetWString();
				content = utf16ToUtf8(wContent);
				break;
			}
			case SQL_C_CHAR:
			{
				auto charContent = uTable.GetColumnBufferPtr<CharColumnBufferPtr>(1);
				content = charContent->GetString();
				break;
			}
			default:
			{
				ASSERT_TRUE(false);
				break;
			}
			}

			switch (i)
			{
			case 1:
				EXPECT_EQ(u8"После смерти отца оставил учёбу и поступил на службу газетным репортёром", content);
				break;
			case 2:
				EXPECT_EQ(u8"因此他对世界的原型的认识就比较广泛。迁徙和旅行对他的创作很有助益", content);
				break;
			case 3:
				EXPECT_EQ(u8"Zürih üniversitesin de başladığı Alman Filolojisi eğitimini 1933 deki babasının ölümü üzerine", content);
				break;
			case 4:
				EXPECT_EQ(u8"מקס פריש נולד ב-15 במאי 1911 בציריך, בן לאדריכל פרנץ ברונו פריש ולאשתו קרולינה בטינה", content);
				break;
			case 5:
				EXPECT_EQ(u8"Στο έργο του ο Φρις διεισδύει, σχολιάζει και αναλύει τα διλήμματα του σύγχρονου ανθρώπου:", content);
				break;
			}
		}
	}

} // namespace exodbctest
