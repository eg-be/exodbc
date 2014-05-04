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
	}

	void DbTableTest::TearDown()
	{
		delete m_pDb;
		delete m_pConnectInf;
	}

	TEST_P(DbTableTest, ReadCharTypes)
	{
		EXPECT_TRUE(false);
	}

	TEST_P(DbTableTest, ReadIntTypes)
	{
		EXPECT_TRUE(false);
	}

//	void DbTableTest::TestCharTypes(CharTypesTable* pTable)
	//{
		//CPPUNIT_ASSERT( m_connectedDb2 );

		//IntTypesTable* pTable = NULL;
		//try
		//{
		//	pTable = new IntTypesTable(m_pDbDb2);
		//	// Open table
		//	CPPUNIT_ASSERT( pTable->Open( ) );

		//	// test for min-max values 
		//	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 1"));
		//	CPPUNIT_ASSERT( pTable->GetNext());
		//	CPPUNIT_ASSERT_EQUAL( (int16_t) -32768, pTable->m_smallInt );

		//	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 2"));
		//	CPPUNIT_ASSERT( pTable->GetNext());
		//	CPPUNIT_ASSERT_EQUAL( (int16_t) 32767, pTable->m_smallInt );

		//	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 3"));
		//	CPPUNIT_ASSERT( pTable->GetNext());
		//	CPPUNIT_ASSERT_EQUAL( (int32_t) -2147483648, pTable->m_int );

		//	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 4"));
		//	CPPUNIT_ASSERT( pTable->GetNext());
		//	CPPUNIT_ASSERT_EQUAL( (int32_t) 2147483647, pTable->m_int );

		//	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 5"));
		//	CPPUNIT_ASSERT( pTable->GetNext());
		//	CPPUNIT_ASSERT_EQUAL( (int64_t) -9223372036854775808, pTable->m_bigInt );

		//	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 6"));
		//	CPPUNIT_ASSERT( pTable->GetNext());
		//	CPPUNIT_ASSERT_EQUAL( (int64_t) 9223372036854775807, pTable->m_bigInt );

		//	// Test for NULL-Values
		//	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 1"));
		//	CPPUNIT_ASSERT( pTable->GetNext());
		//	CPPUNIT_ASSERT( !pTable->IsColNull(0) );
		//	CPPUNIT_ASSERT( !pTable->IsColNull(1) );
		//	CPPUNIT_ASSERT( pTable->IsColNull(2) );
		//	CPPUNIT_ASSERT( pTable->IsColNull(3) );

		//	CPPUNIT_ASSERT( pTable->QueryBySqlStmt(L"SELECT * FROM WXODBC3.INTEGERTYPES WHERE IDINTEGERTYPES = 3"));
		//	CPPUNIT_ASSERT( pTable->GetNext());
		//	CPPUNIT_ASSERT( !pTable->IsColNull(0) );
		//	CPPUNIT_ASSERT( pTable->IsColNull(1) );
		//	CPPUNIT_ASSERT( !pTable->IsColNull(2) );
		//	CPPUNIT_ASSERT( pTable->IsColNull(3) );
		//}
		//CATCH_LOG_RETHROW_DELETE_TABLE(m_pDbDb2, pTable);
		//if(pTable)
		//	delete pTable;
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