/*!
 * \file DbTableTest.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "DbTableTest.h"

// Same component headers
#include "wxOdbc3GoogleTest.h"
#include "GenericTestTables.h"

// Other headers
#include "db.h"

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

namespace wxOdbc3Test
{

	void DbTableTest::SetUp()
	{
		//// Called for every unit-test
		m_odbcInfo = GetParam();
		RecordProperty("DSN", eli::w2mb(m_odbcInfo.m_dsn));
		m_pConnectInf = new wxDbConnectInf(NULL, m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password);
		HENV henv = m_pConnectInf->GetHenv();
		ASSERT_TRUE(henv  != 0);
		m_pDb = new wxDb(henv, m_odbcInfo.m_cursorType == SOdbcInfo::forwardOnlyCursors);
		ASSERT_TRUE(m_pDb->Open(m_pConnectInf));
	}

	void DbTableTest::TearDown()
	{
		if(m_pDb)
			delete m_pDb;
		if(m_pConnectInf)
			delete m_pConnectInf;

		m_pDb = NULL;
		m_pConnectInf = NULL;
	}

	// Open
	// ----
	// Test opening the tables we use in the later tests
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

	// GetNext
	// -------
	// Test basic GetNext
	TEST_P(DbTableTest, GetNextTest)
	{
		CharTypesTable* pTable = new CharTypesTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		// Expect 4 entries
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes"));
		int count = 0;
		while(pTable->GetNext() && count <= 5)
		{
			count++;
		}
		EXPECT_EQ(4, count);

		// Expect no entries
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 5"));
		EXPECT_FALSE(pTable->GetNext());

		// Expect 2 entries
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes >= 2 AND idchartypes <= 3"));
		count = 0;
		while(pTable->GetNext() && count <= 3)
		{
			count++;
		}
		EXPECT_EQ(2, count);

		delete pTable;

	}

	// Read
	// ----
	// Test reading datatypes
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
		
		// TODO: MySql does not have fractions, ibm db2 adds '000' at the end (?)
		RecordProperty("Ticket", 33);
		if(m_pDb->Dbms() == dbmsDB2)
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

		// IBM DB2 has no support for unsigned int types, so far we know only about mysql having that
		RecordProperty("Ticket", 34);
		if(m_pDb->Dbms() == dbmsMY_SQL)
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
			// TODO: This is ugly, use some kind of disabled test, or log a warning..
			if(std::wstring(m_pDb->GetDriverVersion()) != std::wstring(L"05.02.0006"))
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

		// IBM DB2 has no support for unsigned int types, so far we know only about mysql having that
		if(m_pDb->Dbms() == dbmsMY_SQL)
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

		// IBM DB2 has no support for unsigned int types, so far we know only about mysql having that
		if(m_pDb->Dbms() == dbmsMY_SQL)
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
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), std::wstring(pTable->m_varchar));

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), boost::trim_right_copy(std::wstring(pTable->m_char)));

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 3"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( std::wstring(L"הצüאיט"), boost::trim_right_copy(std::wstring(pTable->m_varchar)));

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes WHERE idchartypes = 4"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( std::wstring(L"הצüאיט"), boost::trim_right_copy(std::wstring(pTable->m_char)));

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
		EXPECT_EQ( std::wstring(L"0"), std::wstring(pTable->m_wcdecimal_18_0));

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( std::wstring(L"123456789012345678"), std::wstring(pTable->m_wcdecimal_18_0));
	
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 3"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( std::wstring(L"-123456789012345678"), std::wstring(pTable->m_wcdecimal_18_0));
	
		// DB2 sends a ',', mysql sends a '.' as delimeter
		RecordProperty("Ticket", 35);
		if(m_pDb->Dbms() == dbmsDB2)
		{
			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 4"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( std::wstring(L"0,0000000000"), std::wstring(pTable->m_wcdecimal_18_10));	

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 5"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( std::wstring(L"12345678,9012345678"), std::wstring(pTable->m_wcdecimal_18_10));	

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 6"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( std::wstring(L"-12345678,9012345678"), std::wstring(pTable->m_wcdecimal_18_10));	
		}
		else
		{
			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 4"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( std::wstring(L"0.0000000000"), std::wstring(pTable->m_wcdecimal_18_10));	

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 5"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( std::wstring(L"12345678.9012345678"), std::wstring(pTable->m_wcdecimal_18_10));	

			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes WHERE idnumerictypes = 6"));
			EXPECT_TRUE( pTable->GetNext() );
			EXPECT_EQ( std::wstring(L"-12345678.9012345678"), std::wstring(pTable->m_wcdecimal_18_10));	
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
		EXPECT_EQ(0, memcmp(empty, pTable->m_blob, sizeof(pTable->m_blob)));

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.blobtypes WHERE idblobtypes = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ(0, memcmp(ff, pTable->m_blob, sizeof(pTable->m_blob)));

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.blobtypes WHERE idblobtypes = 3"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ(0, memcmp(abc, pTable->m_blob, sizeof(pTable->m_blob)));

		// Test for NULL
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.blobtypes WHERE idblobtypes = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_FALSE( pTable->IsColNull(1) );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.blobtypes WHERE idblobtypes = 4"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_TRUE( pTable->IsColNull(1) );

		delete pTable;
	}

	TEST_P(DbTableTest, ReadIncompleteTable)
	{
		CharTable* pTable = new CharTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}
		IncompleteCharTable* pIncTable = new IncompleteCharTable(m_pDb);
		if(!pIncTable->Open(false, false))
		{
			delete pTable;
			delete pIncTable;
			ASSERT_FALSE(true);
		}

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM wxodbc3.chartable WHERE idchartable >= 0";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		// Select using '*' on complete table
		sqlstmt = L"INSERT INTO wxodbc3.chartable (idchartable, col2, col3, col4) VALUES (1, 'r1_c2', 'r1_c3', 'r1_c4')";
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartable WHERE idchartable = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ(std::wstring(L"r1_c2"), boost::trim_right_copy(std::wstring(pTable->m_col2)));
		EXPECT_EQ(std::wstring(L"r1_c3"), boost::trim_right_copy(std::wstring(pTable->m_col3)));
		EXPECT_EQ(std::wstring(L"r1_c4"), boost::trim_right_copy(std::wstring(pTable->m_col4)));
		EXPECT_FALSE( pTable->GetNext() );

		// Select with fields on incomplete table
		pTable->m_col2[0] = 0;
		pTable->m_col3[0] = 0;
		pTable->m_col4[0] = 0;
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT idchartable, col2, col3, col4 FROM wxodbc3.chartable WHERE idchartable = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ(std::wstring(L"r1_c2"), boost::trim_right_copy(std::wstring(pTable->m_col2)));
		EXPECT_EQ(std::wstring(L"r1_c3"), boost::trim_right_copy(std::wstring(pTable->m_col3)));
		EXPECT_EQ(std::wstring(L"r1_c4"), boost::trim_right_copy(std::wstring(pTable->m_col4)));
		EXPECT_FALSE( pTable->GetNext() );

		// Now test by reading the incomplete table using '*': It works as we've still used the indexes "as in the db" when we've bound the columns
		pIncTable->m_col2[0] = 0;
		pIncTable->m_col4[0] = 0;
		EXPECT_TRUE( pIncTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartable WHERE idchartable = 1"));
		EXPECT_TRUE( pIncTable->GetNext() );
		EXPECT_EQ(std::wstring(L"r1_c2"), boost::trim_right_copy(std::wstring(pIncTable->m_col2)));
		EXPECT_EQ(std::wstring(L"r1_c4"), boost::trim_right_copy(std::wstring(pIncTable->m_col4)));
		EXPECT_FALSE( pIncTable->GetNext() );

		// It does not work if you do not use the '*' to select, see ticket #15
		RecordProperty("Ticket", 15);
		pIncTable->m_col2[0] = 0;
		pIncTable->m_col4[0] = 0;
		EXPECT_TRUE( pIncTable->QueryBySqlStmt(L"SELECT idchartable, col2, col4 FROM wxodbc3.chartable WHERE idchartable = 1"));
		EXPECT_TRUE( pIncTable->GetNext() );
		//EXPECT_EQ(std::wstring(L"r1_c2"), std::wstring(pIncTable->m_col2).Trim());
		//EXPECT_EQ(std::wstring(L"r1_c4"), std::wstring(pIncTable->m_col4).Trim());
		EXPECT_FALSE( pIncTable->GetNext() );

		// .. But reading using '*' or 'all fields' really works..
		pIncTable->m_col2[0] = 0;
		pIncTable->m_col4[0] = 0;
		EXPECT_TRUE( pIncTable->QueryBySqlStmt(L"SELECT  idchartable, col2, col3, col4 FROM wxodbc3.chartable WHERE idchartable = 1"));
		EXPECT_TRUE( pIncTable->GetNext() );
		EXPECT_EQ(std::wstring(L"r1_c2"), boost::trim_right_copy(std::wstring(pIncTable->m_col2)));
		EXPECT_EQ(std::wstring(L"r1_c4"), boost::trim_right_copy(std::wstring(pIncTable->m_col4)));
		EXPECT_FALSE( pIncTable->GetNext() );

		delete pTable;
		delete pIncTable;
	}
}

// Interfaces
// ----------
