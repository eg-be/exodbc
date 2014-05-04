//#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#include <stdlib.h>
//#include <crtdbg.h>
//#ifndef DBG_NEW
//#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
//#define new DBG_NEW
//#endif
//#endif

#include "DbTableTest.h"
#include "GenericTestTables.h"
#include "db.h"
//#include "MySqlParams.h"
//#include "Db2Params.h"
//#include "db.h"

using namespace std;

namespace wxOdbc3Test
{

	//TEST_F(DbTest, OpenForwardOnly_MySql_3_51)
	//{
	//	OpenTest(MYSQL_3_51_DSN, MYSQL_USER, MYSQL_PASS, true);
	//}

	void DbTableTest::SetUpTestCase()
	{
//		SOdbcInfo oi = GetParam();
		int p = 3;
	}
//
//	void DbTableTest::TearDownTestCase()
//	{
//		int p = 3;
//	}

	void DbTableTest::SetUp()
	{
		SOdbcInfo odbcInfo = GetParam();
		m_pConnectInf = new wxDbConnectInf(NULL, odbcInfo.m_dsn, odbcInfo.m_username, odbcInfo.m_password);
		HENV henv = m_pConnectInf->GetHenv();
		ASSERT_TRUE(henv  != 0);
		m_pDb = new wxDb(henv, odbcInfo.m_cursorType == SOdbcInfo::forwardOnlyCursors);
		ASSERT_TRUE(m_pDb->Open(m_pConnectInf));
	}

	void DbTableTest::TearDown()
	{
		delete m_pDb;
		delete m_pConnectInf;
	}

	TEST_P(DbTableTest, ReadCharTypes)
	{
		CharTypesTable* pTable = new CharTypesTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		// Note: We trim the values read from the db on the right side, as for example db2 pads with ' ' by default

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( wxString(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), wxString(pTable->m_varchar));

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( wxString(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), wxString(pTable->m_char).Trim());

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 3"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( wxString(L"הצüאיט"), wxString(pTable->m_varchar).Trim());

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 4"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( wxString(L"הצüאיט"), wxString(pTable->m_char).Trim());

		// Test for NULL-Values
		EXPECT_TRUE( !pTable->IsColNull(0) );
		EXPECT_TRUE( pTable->IsColNull(1) );
		EXPECT_TRUE( !pTable->IsColNull(2) );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_TRUE( !pTable->IsColNull(0) );
		EXPECT_TRUE( !pTable->IsColNull(1) );
		EXPECT_TRUE( pTable->IsColNull(2) );

		delete pTable;
	}

	//TEST_P(DbTableTest, ReadIntTypes)
	//{
	//	EXPECT_TRUE(true);
	//}



	//void DbTest::OpenTest(const std::wstring dsn, const std::wstring user, const std::wstring pass, bool forwardOnlyCursors)
	//{
	//	wxDbConnectInf* pConnectInf = new wxDbConnectInf(NULL, dsn, user, pass);
	//	HENV henv = pConnectInf->GetHenv();
	//	if(!henv)
	//	{
	//		delete pConnectInf;
	//		ASSERT_TRUE(henv);
	//	}

	//	wxDb* pDb = new wxDb(henv, forwardOnlyCursors);

	//	// Open without failing on unsupported datatypes
	//	EXPECT_TRUE(pDb->Open(pConnectInf, false));
	//	pDb->Close();
	//	delete pDb;

	//	// Open with failing on unsupported datatypes
	//	pDb = new wxDb(henv, forwardOnlyCursors);
	//	EXPECT_TRUE(pDb->Open(pConnectInf, true));
	//	pDb->Close();

	//	delete pDb;
	//	delete pConnectInf;
	//}

}