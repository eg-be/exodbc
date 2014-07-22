/*!
 * \file DbTest.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "DbTest.h"

// Same component headers
#include "GenericTestTables.h"

// Other headers
#include "db.h"
#include "boost/algorithm/string.hpp"

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
using namespace std;

namespace wxOdbc3Test
{

	void DbTest::SetUp()
	{
		// Set up is called for every test
		m_odbcInfo = GetParam();
		RecordProperty("DSN", eli::w2mb(m_odbcInfo.m_dsn));
		m_pConnectInf = new wxDbConnectInf(NULL, m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password);
		HENV henv = m_pConnectInf->GetHenv();
		ASSERT_TRUE(henv  != 0);
		m_pDb = new wxDb(henv, m_odbcInfo.m_cursorType == SOdbcInfo::forwardOnlyCursors);
		ASSERT_TRUE(m_pDb->Open(m_pConnectInf));
	}

	void DbTest::TearDown()
	{
		delete m_pConnectInf;
		delete m_pDb;
		m_pConnectInf = NULL;
		m_pDb = NULL;
	}


	TEST_P(DbTest, Open)
	{
		HENV henv = m_pConnectInf->GetHenv();
		ASSERT_TRUE(henv  != 0);

		wxDb* pDb = new wxDb(henv, m_odbcInfo.m_cursorType == SOdbcInfo::forwardOnlyCursors);

		// Open without failing on unsupported datatypes
		EXPECT_TRUE(pDb->Open(m_pConnectInf, false));
		pDb->Close();
		delete pDb;

		// Try to open with a different password / user, expect to fail when opening the db.
		// TODO: Getting the HENV never fails? Add some tests for the HENV
		wxDbConnectInf* pFailConnectInf = new wxDbConnectInf(NULL, L"ThisDNSDoesNotExist", L"NorTheUser", L"WithThisPassword");
		HENV henvFail = pFailConnectInf->GetHenv();
		EXPECT_TRUE(henvFail != NULL);
		wxDb* pFailDb = new wxDb(henvFail, true);
		EXPECT_FALSE(pFailDb->Open(pFailConnectInf, false));
		delete pFailDb;
		delete pFailConnectInf;

		// Open with failing on unsupported datatypes
		// TODO: This test is stupid, we should also test that we fail
		pDb = new wxDb(henv, m_odbcInfo.m_cursorType == SOdbcInfo::forwardOnlyCursors);
		EXPECT_TRUE(pDb->Open(m_pConnectInf, true));
		pDb->Close();
		delete pDb;
	}


	TEST_P(DbTest, DetectDbms)
	{
		// We just know how we name the different odbc-sources
		// TODO: This is not nice, but is there any other reliable way? Add to doc somewhere
		m_odbcInfo = GetParam();
		if(boost::algorithm::find_first(m_odbcInfo.m_dsn, L"DB2"))
		{
			EXPECT_TRUE(m_pDb->Dbms() == dbmsDB2);
		}
		else if(boost::algorithm::find_first(m_odbcInfo.m_dsn, L"MySql"))
		{
			EXPECT_TRUE(m_pDb->Dbms() == dbmsMY_SQL);
		}
		else
		{
			EXPECT_EQ(wxString(L"Unknown DSN name"), m_odbcInfo.m_dsn);
		}
	}


	TEST_P(DbTest, ExecSql_InsertCharTypes)
	{
		CharTypesTmpTable* pTable = new CharTypesTmpTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		wxString sqlstmt;
		sqlstmt.Printf(L"DELETE FROM wxodbc3.chartypes_tmp WHERE idchartypes_tmp >= 0");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		// Note the escaping:
		// IBM DB2 wants to escape ' using '', mysql wants \'
		// MYSQL needs \\ for \ 
		if(m_pDb->Dbms() == dbmsDB2)
		{
			sqlstmt.Printf("INSERT INTO wxodbc3.chartypes_tmp (idchartypes_tmp, tvarchar, tchar) VALUES (1, '%s', '%s')", L" !\"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", L" !\"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
		}
		else
		{
			sqlstmt.Printf("INSERT INTO wxodbc3.chartypes_tmp (idchartypes_tmp, tvarchar, tchar) VALUES (1, '%s', '%s')", L" !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", L" !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~");
		}
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		// And note the triming
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.chartypes_tmp ORDER BY idchartypes_tmp ASC"));
		EXPECT_TRUE( pTable->GetNext());
		EXPECT_EQ( wxString(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), wxString(pTable->m_varchar));
		EXPECT_EQ( wxString(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), wxString(pTable->m_char).Trim());
		EXPECT_FALSE(pTable->GetNext());

		delete pTable;
	}

	TEST_P(DbTest, ExecSQL_InsertFloatTypes)
	{
		FloatTypesTmpTable* pTable = new FloatTypesTmpTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		wxString sqlstmt;
		sqlstmt.Printf(L"DELETE FROM wxodbc3.floattypes_tmp WHERE idfloattypes_tmp >= 0");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		sqlstmt.Printf("INSERT INTO wxodbc3.floattypes_tmp (idfloattypes_tmp, tdouble, tfloat) VALUES (1, -3.141592, -3.141)");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes_tmp WHERE idfloattypes_tmp = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( -3.141592, pTable->m_double);
		EXPECT_EQ( -3.141, pTable->m_float);
		EXPECT_FALSE( pTable->GetNext() );

		sqlstmt.Printf("INSERT INTO wxodbc3.floattypes_tmp (idfloattypes_tmp, tdouble, tfloat) VALUES (2, 3.141592, 3.141)");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.floattypes_tmp WHERE idfloattypes_tmp = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 3.141592, pTable->m_double);
		EXPECT_EQ( 3.141, pTable->m_float);
		EXPECT_FALSE( pTable->GetNext() );


		delete pTable;
	}


	TEST_P(DbTest, ExecSQL_InsertNumericTypesAsChar)
	{
		NumericTypesTmpTable* pTable = new NumericTypesTmpTable(m_pDb, NumericTypesTmpTable::ReadAsChar);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		wxString sqlstmt;
		sqlstmt.Printf(L"DELETE FROM wxodbc3.numerictypes_tmp WHERE idnumerictypes_tmp >= 0");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		sqlstmt.Printf("INSERT INTO wxodbc3.numerictypes_tmp (idnumerictypes_tmp, tdecimal_18_0, tdecimal_18_10) VALUES (1, -123456789012345678, -12345678.9012345678)");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		// TODO: DB2 sends a ',', mysql sends a '.' as delimeter
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes_tmp WHERE idnumerictypes_tmp = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( wxString(L"-123456789012345678"), wxString(pTable->m_wcdecimal_18_0));

		if(m_pDb->Dbms() == dbmsDB2)
			EXPECT_EQ( wxString(L"-12345678,9012345678"), wxString(pTable->m_wcdecimal_18_10));	
		else
			EXPECT_EQ( wxString(L"-12345678.9012345678"), wxString(pTable->m_wcdecimal_18_10));	

		sqlstmt.Printf("INSERT INTO wxodbc3.numerictypes_tmp (idnumerictypes_tmp, tdecimal_18_0, tdecimal_18_10) VALUES (2, 123456789012345678, 12345678.9012345678)");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		// TODO: DB2 sends a ',', mysql sends a '.' as delimeter
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes_tmp WHERE idnumerictypes_tmp = 2"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( wxString(L"123456789012345678"), wxString(pTable->m_wcdecimal_18_0));

		if(m_pDb->Dbms() == dbmsDB2)
			EXPECT_EQ( wxString(L"12345678,9012345678"), wxString(pTable->m_wcdecimal_18_10));	
		else
			EXPECT_EQ( wxString(L"12345678.9012345678"), wxString(pTable->m_wcdecimal_18_10));	

		sqlstmt.Printf("INSERT INTO wxodbc3.numerictypes_tmp (idnumerictypes_tmp, tdecimal_18_0, tdecimal_18_10) VALUES (3, 0, 0.0000000000)");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		// TODO: DB2 sends a ',', mysql sends a '.' as delimeter
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.numerictypes_tmp WHERE idnumerictypes_tmp = 3"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( wxString(L"0"), wxString(pTable->m_wcdecimal_18_0));
		
		if(m_pDb->Dbms() == dbmsDB2)
			EXPECT_EQ( wxString(L"0,0000000000"), wxString(pTable->m_wcdecimal_18_10));	
		else
			EXPECT_EQ( wxString(L"0.0000000000"), wxString(pTable->m_wcdecimal_18_10));	

		delete pTable;
	}


	TEST_P(DbTest, ExecSQL_InsertIntTypes)
	{
		IntTypesTmpTable* pTable = new IntTypesTmpTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		wxString sqlstmt;
		sqlstmt.Printf(L"DELETE FROM wxodbc3.integertypes_tmp WHERE idintegertypes_tmp >= 0");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp"));
		EXPECT_FALSE( pTable->GetNext());

		sqlstmt.Printf("INSERT INTO wxodbc3.integertypes_tmp (idintegertypes_tmp, tsmallint, tint, tbigint) VALUES (1, -32768, -2147483648, -9223372036854775808)");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp ORDER BY idintegertypes_tmp ASC"));
		EXPECT_TRUE( pTable->GetNext());
		EXPECT_EQ( -32768, pTable->m_smallInt);
		EXPECT_EQ( INT_MIN, pTable->m_int);
		EXPECT_EQ( -LLONG_MIN, pTable->m_bigInt);
		EXPECT_FALSE( pTable->GetNext());

		sqlstmt.Printf("INSERT INTO wxodbc3.integertypes_tmp (idintegertypes_tmp, tsmallint, tint, tbigint) VALUES (2, 32767, 2147483647, 9223372036854775807)");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp ORDER BY idintegertypes_tmp ASC"));
		EXPECT_TRUE( pTable->GetNext());
		EXPECT_TRUE( pTable->GetNext());
		EXPECT_EQ( 32767, pTable->m_smallInt);
		EXPECT_EQ( 2147483647, pTable->m_int);
		EXPECT_EQ( 9223372036854775807, pTable->m_bigInt);
		EXPECT_FALSE( pTable->GetNext());

		// IBM DB2 has no support for unsigned int types, so far we only know about mysql
		if(m_pDb->Dbms() == dbmsMY_SQL)
		{
			sqlstmt.Printf("INSERT INTO wxodbc3.integertypes_tmp (idintegertypes_tmp, tusmallint, tuint, tubigint) VALUES (4, 0, 0, 0)");
			EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
			EXPECT_TRUE( m_pDb->CommitTrans() );
			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp WHERE idintegertypes_tmp = 4"));
			EXPECT_TRUE( pTable->GetNext());
			EXPECT_EQ( 0, pTable->m_usmallInt);
			EXPECT_EQ( 0, pTable->m_uint);
			EXPECT_EQ( 0, pTable->m_ubigInt);
			EXPECT_FALSE( pTable->GetNext());

			sqlstmt.Printf("INSERT INTO wxodbc3.integertypes_tmp (idintegertypes_tmp, tusmallint, tuint, tubigint) VALUES (5, 65535, 4294967295, 18446744073709551615)");
			EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
			EXPECT_TRUE( m_pDb->CommitTrans() );
			EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.integertypes_tmp  WHERE idintegertypes_tmp = 5"));
			EXPECT_TRUE( pTable->GetNext());
			EXPECT_EQ( 65535, pTable->m_usmallInt);
			EXPECT_EQ( 4294967295, pTable->m_uint);
			// With the 5.2 odbc driver ubigint seems to be wrong?
			// TODO: This is ugly, use some kind of disabled test, or log a warning..
			if(wxString(m_pDb->GetDriverVersion()) != wxString(L"05.02.0006"))
			{
				EXPECT_EQ( 18446744073709551615, pTable->m_ubigInt);
			}
			EXPECT_FALSE( pTable->GetNext());
		}

		delete pTable;
	}

	TEST_P(DbTest, ExecSQL_InsertDateTypes)
	{
		DateTypesTmpTable* pTable = new DateTypesTmpTable(m_pDb);
		if(!pTable->Open(false, false))
		{
			delete pTable;
			ASSERT_FALSE(true);
		}

		wxString sqlstmt;
		sqlstmt.Printf(L"DELETE FROM wxodbc3.datetypes_tmp WHERE iddatetypes_tmp >= 0");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );

		sqlstmt.Printf("INSERT INTO wxodbc3.datetypes_tmp (iddatetypes_tmp, tdate, ttime, ttimestamp) VALUES (1, '1983-01-26', '13:55:56', '1983-01-26 13:55:56')");
		EXPECT_TRUE( m_pDb->ExecSql(sqlstmt) );
		EXPECT_TRUE( m_pDb->CommitTrans() );
		EXPECT_TRUE( pTable->QueryBySqlStmt(L"SELECT * FROM wxodbc3.datetypes_tmp WHERE iddatetypes_tmp = 1"));
		EXPECT_TRUE( pTable->GetNext() );
		EXPECT_EQ( 26, pTable->m_date.day);
		EXPECT_EQ( 01, pTable->m_date.month);
		EXPECT_EQ( 1983, pTable->m_date.year);

		EXPECT_EQ( 13, pTable->m_time.hour);
		EXPECT_EQ( 55, pTable->m_time.minute);
		EXPECT_EQ( 56, pTable->m_time.second);

		EXPECT_EQ( 26, pTable->m_timestamp.day);
		EXPECT_EQ( 01, pTable->m_timestamp.month);
		EXPECT_EQ( 1983, pTable->m_timestamp.year);
		EXPECT_EQ( 13, pTable->m_timestamp.hour);
		EXPECT_EQ( 55, pTable->m_timestamp.minute);
		EXPECT_EQ( 56, pTable->m_timestamp.second);

		EXPECT_FALSE( pTable->GetNext() );

		delete pTable;
	}
}

// Interfaces
// ----------
