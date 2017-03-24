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


	TEST_F(UnicodeTest, ReadUnicodeColumn)
	{
		// Test that we can read utf-8 data as SQL_C_CHAR
		Table uTable(m_pDb, TableAccessFlag::AF_READ, GetTableName(TableId::UNICODE_TABLE));

		// If we simply read back as SQL_C_CHAR on DB2 we get garbage on windows.
		// Reading the CHAR Column as WCHAR works on windows
#ifdef _WIN32
		if (m_pDb->GetDbms() == DatabaseProduct::DB2)
		{
			DefaultSql2BufferMapPtr pBindMap = std::make_shared<DefaultSql2BufferMap>(m_pEnv->GetOdbcVersion());
			pBindMap->RegisterType(SQL_VARCHAR, SQL_C_WCHAR);
			uTable.SetSql2BufferTypeMap(pBindMap);
		}
#else
        // and if we read SQL_WVARCHAR on linux we get garbage here. Force SQL Server to pBindMap
        // its WVARCHAR columns to SQL_C_CHAR
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			DefaultSql2BufferMapPtr pBindMap = std::make_shared<DefaultSql2BufferMap>(m_pEnv->GetOdbcVersion());
			pBindMap->RegisterType(SQL_WVARCHAR, SQL_C_CHAR);
			uTable.SetSql2BufferTypeMap(pBindMap);
		}
#endif

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
			string r1 = u8"После смерти отца оставил учёбу и поступил на службу газетным репортёром";
			string r2 = u8"因此他对世界的原型的认识就比较广泛。迁徙和旅行对他的创作很有助益";
			string r3 = u8"Zürih üniversitesin de başladığı Alman Filolojisi eğitimini 1933 deki babasının ölümü üzerine";
			string r4 = u8"מקס פריש נולד ב-15 במאי 1911 בציריך, בן לאדריכל פרנץ ברונו פריש ולאשתו קרולינה בטינה";
			string r5 = u8"Στο έργο του ο Φρις διεισδύει, σχολιάζει και αναλύει τα διλήμματα του σύγχρονου ανθρώπου:";
			switch (i)
			{
			case 1:
				EXPECT_EQ(r1, content);
				break;
			case 2:
				EXPECT_EQ(r2, content);
				break;
			case 3:
				EXPECT_EQ(r3, content);
				break;
			case 4:
				EXPECT_EQ(r4, content);
				break;
			case 5:
				EXPECT_EQ(r5, content);
				break;
			}
		}
	}

} // namespace exodbctest
