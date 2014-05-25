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
		m_odbcInfo = GetParam();
		m_pConnectInf = new wxDbConnectInf(NULL, m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password);
		HENV henv = m_pConnectInf->GetHenv();
		ASSERT_TRUE(henv  != 0);
		m_pDb = new wxDb(henv, m_odbcInfo.m_cursorType == SOdbcInfo::forwardOnlyCursors);
		ASSERT_TRUE(m_pDb->Open(m_pConnectInf));
	}

	void DbTableTest::TearDown()
	{
		delete m_pDb;
		delete m_pConnectInf;
	}

	TEST_P(DbTableTest, ReadDateTypes)
	{
		DateTypesTable* pTable = new DateTypesTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.datetypes WHERE iddatetypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 26, pTable->m_date.day);
		EXPECT_EQ( 01, pTable->m_date.month);
		EXPECT_EQ( 1983, pTable->m_date.year);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.datetypes WHERE iddatetypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 13, pTable->m_time.hour);
		EXPECT_EQ( 55, pTable->m_time.minute);
		EXPECT_EQ( 56, pTable->m_time.second);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.datetypes WHERE iddatetypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 26, pTable->m_timestamp.day);
		EXPECT_EQ( 01, pTable->m_timestamp.month);
		EXPECT_EQ( 1983, pTable->m_timestamp.year);
		EXPECT_EQ( 13, pTable->m_timestamp.hour);
		EXPECT_EQ( 55, pTable->m_timestamp.minute);
		EXPECT_EQ( 56, pTable->m_timestamp.second);
		
		// MySql does not have fractions, ibm db2 adds '000' at the end (?)
		if(m_odbcInfo.m_dsn == DB2_DSN)
		{
			EXPECT_EQ( 123456000, pTable->m_timestamp.fraction);
		}

		delete pTable;
	}
	
	TEST_P(DbTableTest, ReadIntTypes)
	{
		IntTypesTable* pTable = new IntTypesTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( -32768, pTable->m_smallInt);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 32767, pTable->m_smallInt);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 3"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( INT_MIN /* -2147483648 */, pTable->m_int);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 4"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 2147483647, pTable->m_int);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 5"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( LLONG_MIN /* -9223372036854775808 */, pTable->m_bigInt);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 6"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 9223372036854775807, pTable->m_bigInt);

		// IBM DB2 has no support for unsigned int types
		if(m_odbcInfo.m_dsn != DB2_DSN)
		{
			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 7"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( 0, pTable->m_usmallInt);

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 8"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( 65535, pTable->m_usmallInt);

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 9"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( 0, pTable->m_uint);

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 10"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( 4294967295, pTable->m_uint);

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 11"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( 0, pTable->m_ubigInt);

			// With the 5.2 odbc driver bigint seems to be wrong?
			if(m_odbcInfo.m_dsn != MYSQL_5_2_DSN)
			{
				EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 12"));
				EXPECT_TRUE( pTable->GetNext() );
				EXPECT_EQ( 18446744073709551615, pTable->m_ubigInt);
			}

		}

		delete pTable;
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