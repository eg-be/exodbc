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

	TEST_P(DbTableTest, OpenTableNoChecks)
	{
		CharTypesTable* pCharTypesTable = new CharTypesTable(m_pDb);
		EXPECT_TRUE(pCharTypesTable->Open(false, false));
		delete pCharTypesTable;

		IntTypesTable* pIntTypesTable = new IntTypesTable(m_pDb);
		EXPECT_TRUE(pIntTypesTable->Open(false, false));
		delete pIntTypesTable;

		DateTypesTable* pDateTypesTable = new DateTypesTable(m_pDb);
		EXPECT_TRUE(pDateTypesTable->Open(false, false));
		delete pDateTypesTable;

		FloatTypesTable* pFloatTypesTable = new FloatTypesTable(m_pDb);
		EXPECT_TRUE(pFloatTypesTable->Open(false, false));
		delete pFloatTypesTable;

		NumericTypesTable* pNumericTypesTable = new NumericTypesTable(m_pDb, NumericTypesTable::ReadAsChar);
		EXPECT_TRUE(pNumericTypesTable->Open(false, false));
		delete pNumericTypesTable;

		BlobTypesTable* pBlobTypesTable = new BlobTypesTable(m_pDb);
		EXPECT_TRUE(pBlobTypesTable->Open(false, false));
		delete pBlobTypesTable;
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

		// Test for NULL
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.datetypes WHERE iddatetypes = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_FALSE( pTable->IsColNull(0) );
		EXPECT_TRUE( pTable->IsColNull(1) );
		EXPECT_TRUE( pTable->IsColNull(2) );
		EXPECT_TRUE( pTable->IsColNull(3) );

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

		// Test for NULL-Values
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 1"));
		EXPECT_TRUE( pTable->GetNext());

		EXPECT_FALSE( pTable->IsColNull(0) );
		EXPECT_FALSE( pTable->IsColNull(1) );
		EXPECT_TRUE( pTable->IsColNull(2) );
		EXPECT_TRUE( pTable->IsColNull(3) );
		if(m_odbcInfo.m_dsn != DB2_DSN)
		{
			EXPECT_TRUE( pTable->IsColNull(4) );
			EXPECT_TRUE( pTable->IsColNull(5) );
			EXPECT_TRUE( pTable->IsColNull(6) );
		}

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 3"));
		EXPECT_TRUE( pTable->GetNext());
		EXPECT_FALSE( pTable->IsColNull(2) );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 5"));
		EXPECT_TRUE( pTable->GetNext());
		EXPECT_FALSE( pTable->IsColNull(3) );

		if(m_odbcInfo.m_dsn != DB2_DSN)
		{
			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 7"));
			EXPECT_TRUE( pTable->GetNext());
			EXPECT_FALSE( pTable->IsColNull(4) );

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 9"));
			EXPECT_TRUE( pTable->GetNext());
			EXPECT_FALSE( pTable->IsColNull(5) );

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes WHERE idintegertypes = 11"));
			EXPECT_TRUE( pTable->GetNext());
			EXPECT_FALSE( pTable->IsColNull(6) );
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
		EXPECT_FALSE( pTable->IsColNull(0) );
		EXPECT_TRUE( pTable->IsColNull(1) );
		EXPECT_FALSE( pTable->IsColNull(2) );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_FALSE( pTable->IsColNull(0) );
		EXPECT_FALSE( pTable->IsColNull(1) );
		EXPECT_TRUE( pTable->IsColNull(2) );

		delete pTable;
	}


	TEST_P(DbTableTest, ReadFloatTypes)
	{		
		FloatTypesTable* pTable = new FloatTypesTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 0.0, pTable->m_float);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 3.141, pTable->m_float);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 3"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( -3.141, pTable->m_float);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 4"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 0.0, pTable->m_double);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 5"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 3.141592, pTable->m_double);

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 6"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( -3.141592, pTable->m_double);

		// Test for NULL
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_TRUE( pTable->IsColNull(1));
		EXPECT_FALSE( pTable->IsColNull(2));

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes WHERE idfloattypes = 4"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_FALSE( pTable->IsColNull(1));
		EXPECT_TRUE( pTable->IsColNull(2));

		delete pTable;
	}

	// TODO: Numeric things do not work at all
	//template< class T>
	//void changeEndianess( T & input)
	//{
	//	char temp[ sizeof( T ) ];
	//	for( int j=sizeof( T )-1; j>=0; --j )
	//		temp[sizeof(T)-j-1] = *((char *)&input+j);
	//	memcpy( &input, temp, sizeof( T ) );
	//}

	//TEST_P(DbTableTest, ReadNumericTypes)
	//{
	//	NumericTypesTable* pTable = new NumericTypesTable(m_pDb);
	//	if(!pTable->Open(false, false))
	//	{
	//		delete pTable;
	//		ASSERT_FALSE(true);
	//	}

//		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 1"));
//		EXPECT_TRUE( pTable->GetNext() );
//		void* t = (void*) &(pTable->m_decimal_18_0.val);
//		uint16_t i = *((uint16_t*) t);
//		uint16_t u;
//		size_t s = sizeof(pTable->m_decimal_18_0.val);
//		memcpy(&u, &(pTable->m_decimal_18_0.val), 16);
//		changeEndianess<uint16_t>(u);
//		//uint16_t i = (int) pTable->m_decimal_18_0.val;
////		EXPECT_EQ( 0.0, pTable->m_float);
//
//		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 2"));
//		EXPECT_TRUE( pTable->GetNext() );
////		i = (int) pTable->m_decimal_18_0.val;
//		t = (void*) &(pTable->m_decimal_18_0.val);
//		i = *((uint16_t*) t);
//		memcpy(&u, &(pTable->m_decimal_18_0.val), 16);
//		changeEndianess<uint16_t>(u);
//

//
//		delete pTable;
		
	//}

	TEST_P(DbTableTest, ReadNumericTypesAsChar)
	{
		NumericTypesTable* pTable = new NumericTypesTable(m_pDb, NumericTypesTable::ReadAsChar);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( wxString(L"0"), wxString(pTable->m_wcdecimal_18_0));

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( wxString(L"123456789012345678"), wxString(pTable->m_wcdecimal_18_0));
	
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 3"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( wxString(L"-123456789012345678"), wxString(pTable->m_wcdecimal_18_0));
	
		// DB2 sends a ',', mysql sends a '.' as delimeter
		if(m_odbcInfo.m_dsn == DB2_DSN)
		{
			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 4"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( wxString(L"0,0000000000"), wxString(pTable->m_wcdecimal_18_10));	

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 5"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( wxString(L"12345678,9012345678"), wxString(pTable->m_wcdecimal_18_10));	

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 6"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( wxString(L"-12345678,9012345678"), wxString(pTable->m_wcdecimal_18_10));	
		}
		else
		{
			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 4"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( wxString(L"0.0000000000"), wxString(pTable->m_wcdecimal_18_10));	

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 5"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( wxString(L"12345678.9012345678"), wxString(pTable->m_wcdecimal_18_10));	

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 6"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( wxString(L"-12345678.9012345678"), wxString(pTable->m_wcdecimal_18_10));	
		}
	
		// Test for NULL
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_FALSE( pTable->IsColNull(1) );
		EXPECT_TRUE( pTable->IsColNull(2) );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 4"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_TRUE( pTable->IsColNull(1) );
		EXPECT_FALSE( pTable->IsColNull(2) );

		delete pTable;
	}


	TEST_P(DbTableTest, ReadBlobTypes)
	{
		BlobTypesTable* pTable = new BlobTypesTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.blobtypes WHERE idblobtypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		SQLCHAR empty[] = {	0, 0, 0, 0,
							0, 0, 0, 0,
							0, 0, 0, 0,
							0, 0, 0, 0
		};

		SQLCHAR ff[] = {	255, 255, 255, 255,
							255, 255, 255, 255,
							255, 255, 255, 255,
							255, 255, 255, 255
		};

		SQLCHAR abc[] = {   0xab, 0xcd, 0xef, 0xf0,
							0x12, 0x34, 0x56, 0x78,
							0x90, 0xab, 0xcd, 0xef,
							0x01, 0x23, 0x45, 0x67
		};

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.blobtypes WHERE idblobtypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ(wxString(empty, sizeof(pTable->m_blob)), wxString(pTable->m_blob, sizeof(pTable->m_blob)));

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.blobtypes WHERE idblobtypes = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ(wxString(ff, sizeof(pTable->m_blob)), wxString(pTable->m_blob, sizeof(pTable->m_blob)));

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.blobtypes WHERE idblobtypes = 3"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ(wxString(abc, sizeof(pTable->m_blob)), wxString(pTable->m_blob, sizeof(pTable->m_blob)));

		// Test for NULL
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.blobtypes WHERE idblobtypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_FALSE( pTable->IsColNull(1) );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.blobtypes WHERE idblobtypes = 4"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_TRUE( pTable->IsColNull(1) );

		delete pTable;
	}
}