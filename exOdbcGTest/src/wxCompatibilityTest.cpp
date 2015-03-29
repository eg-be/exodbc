/*!
* \file wxCompatibilityTest.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 31.08.2014
* \copyright wxWindows Library Licence, Version 3.1
* 
* [Brief CPP-file description]
*/ 

#include "stdafx.h"

// Own header
#include "wxCompatibilityTest.h"

// Same component headers
#include "ManualTestTables.h"

// Other headers
#include "Database.h"

// Debug
#include "DebugNew.h"

using namespace std;

namespace exodbc
{
	//Static consts
	// -------------
	
	//Construction
	// -------------
	void wxCompatibilityTest::SetUp()
	{
		// Run for every test
		m_odbcInfo = GetParam();
//		RecordProperty("DSN", eli::w2mb(m_odbcInfo.m_dsn));

		// Set up environment
		m_env.AllocateEnvironmentHandle();
		m_env.SetOdbcVersion(OdbcVersion::V_3);

		// And the db
		ASSERT_NO_THROW(m_db.AllocateConnectionHandle(m_env));
		ASSERT_NO_THROW(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
	}
	
	//Destructor
	// -----------
	void wxCompatibilityTest::TearDown()
	{
		// Run after every test
	}
	
	//Implementation
	// --------------


	// Test opening the tables we use in the later tests
	// This also tests that calling the old Open(checkPrivs, checkExists) function still returns true
	TEST_P(wxCompatibilityTest, OpenTableNoChecks)
	{
		MCharTypesTable charTypesTable(m_db, m_odbcInfo.m_namesCase);
		EXPECT_NO_THROW(charTypesTable.Open(m_db, TOF_NONE));

		MIntTypesTable intTypesTable(m_db, m_odbcInfo.m_namesCase);
		EXPECT_NO_THROW(intTypesTable.Open(m_db, TOF_NONE));

		MDateTypesTable dateTypesTable(m_db, m_odbcInfo.m_namesCase);
		EXPECT_NO_THROW(dateTypesTable.Open(m_db, TOF_NONE));

		MFloatTypesTable floatTypesTable(m_db, m_odbcInfo.m_namesCase);
		EXPECT_NO_THROW(floatTypesTable.Open(m_db, TOF_NONE));

//		MNumericTypesTable numericTypesTable(m_pDb, NumericTypesTable::ReadAsChar, m_odbcInfo.m_namesCase);
//		EXPECT_TRUE(numericTypesTable.Open(false, false));

		MBlobTypesTable blobTypesTable(m_db, m_odbcInfo.m_namesCase);
		EXPECT_NO_THROW(blobTypesTable.Open(m_db, TOF_NONE));
	}


	// Test basic GetNext
	TEST_P(wxCompatibilityTest, GetNextTest)
	{
		MCharTypesTable table(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		// Expect 6 entries
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.chartypes");
		int count = 0;
		while(table.SelectNext() && count <= 7)
		{
			count++;
		}
		EXPECT_EQ(6, count);

		// Expect no entries
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.chartypes WHERE idchartypes = 7");
		EXPECT_FALSE(table.SelectNext());

		// Expect 2 entries
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.chartypes WHERE idchartypes >= 2 AND idchartypes <= 3");
		count = 0;
		while(table.SelectNext() && count <= 3)
		{
			count++;
		}
		EXPECT_EQ(2, count);
	}


	// Test reading datatypes
	TEST_P(wxCompatibilityTest, SelectDateTypes)
	{
		MDateTypesTable table(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.datetypes WHERE iddatetypes = 1");
		if (m_db.Dbms() == dbmsMS_SQL_SERVER)
		{
			// MS will complain about data loss
			LogLevelError llErr;
			EXPECT_TRUE(table.SelectNext());
		}
		else
		{
			EXPECT_TRUE(table.SelectNext());
		}
		EXPECT_EQ(26, table.m_date.day);
		EXPECT_EQ( 01, table.m_date.month);
		EXPECT_EQ( 1983, table.m_date.year);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.datetypes WHERE iddatetypes = 1");
		if (m_db.Dbms() == dbmsMS_SQL_SERVER)
		{
			// MS will complain about data loss
			LogLevelError llErr;
			EXPECT_TRUE(table.SelectNext());
		}
		else
		{
			EXPECT_TRUE(table.SelectNext());
		}
		EXPECT_EQ( 13, table.m_time.hour);
		EXPECT_EQ( 55, table.m_time.minute);
		EXPECT_EQ( 56, table.m_time.second);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.datetypes WHERE iddatetypes = 2");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( 26, table.m_timestamp.day);
		EXPECT_EQ( 01, table.m_timestamp.month);
		EXPECT_EQ( 1983, table.m_timestamp.year);
		EXPECT_EQ( 13, table.m_timestamp.hour);
		EXPECT_EQ( 55, table.m_timestamp.minute);
		EXPECT_EQ( 56, table.m_timestamp.second);

		SQLUINTEGER fraction = 0;
		switch(m_db.Dbms())
		{
		case dbmsDB2:
			fraction = 123456000;
			break;
		case dbmsMS_SQL_SERVER:
			fraction = 123000000;
			break;
		default:
			fraction = 0;
			break;
		}
		EXPECT_EQ( fraction, table.m_timestamp.fraction);

		// Test for NULL
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.datetypes WHERE iddatetypes = 2");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_FALSE( table.IsColumnNull(0) );
		EXPECT_TRUE(table.IsColumnNull(1));
		EXPECT_TRUE(table.IsColumnNull(2));
		EXPECT_FALSE(table.IsColumnNull(3));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.datetypes WHERE iddatetypes = 3");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_FALSE(table.IsColumnNull(0));
		EXPECT_TRUE(table.IsColumnNull(1));
		EXPECT_TRUE(table.IsColumnNull(2));
		EXPECT_TRUE(table.IsColumnNull(3));
	}


	TEST_P(wxCompatibilityTest, SelectIntTypes)
	{
		MIntTypesTable table(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 1");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( -32768, table.m_smallInt);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 2");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( 32767, table.m_smallInt);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 3");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( INT_MIN /* -2147483648 */, table.m_int);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 4");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( 2147483647, table.m_int);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 5");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( LLONG_MIN /* -9223372036854775808 */, table.m_bigInt);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 6");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( 9223372036854775807, table.m_bigInt);

		// Test for NULL-Values
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 1");
		EXPECT_TRUE( table.SelectNext());

		EXPECT_FALSE( table.IsColumnNull(0) );
		EXPECT_FALSE( table.IsColumnNull(1) );
		EXPECT_TRUE( table.IsColumnNull(2) );
		EXPECT_TRUE( table.IsColumnNull(3) );

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 3");
		EXPECT_TRUE( table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(2) );

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 5");
		EXPECT_TRUE( table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(3) );
	}


	TEST_P(wxCompatibilityTest, SelectWCharTypes)
	{
		MWCharTypesTable table(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		// Note: We trim the values read from the db on the right side, as for example db2 pads with ' ' by default

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.chartypes WHERE idchartypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), std::wstring(table.m_varchar));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.chartypes WHERE idchartypes = 2");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), boost::trim_right_copy(std::wstring(table.m_char)));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.chartypes WHERE idchartypes = 3");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"הצüאיט"), boost::trim_right_copy(std::wstring(table.m_varchar)));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.chartypes WHERE idchartypes = 4");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"הצüאיט"), boost::trim_right_copy(std::wstring(table.m_char)));

		// Test for NULL-Values
		EXPECT_FALSE( table.IsColumnNull(0) );
		EXPECT_TRUE( table.IsColumnNull(1) );
		EXPECT_FALSE( table.IsColumnNull(2) );

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.chartypes WHERE idchartypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(0) );
		EXPECT_FALSE( table.IsColumnNull(1) );
		EXPECT_TRUE( table.IsColumnNull(2) );
	}



	TEST_P(wxCompatibilityTest, SelectFloatTypes)
	{		
		MFloatTypesTable table(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( 0.0, table.m_float);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 2");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( 3.141, table.m_float);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 3");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( -3.141, table.m_float);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 4");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( 0.0, table.m_double);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 5");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( 3.141592, table.m_double);

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 6");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( -3.141592, table.m_double);

		// Test for NULL
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_TRUE( table.IsColumnNull(1));
		EXPECT_FALSE( table.IsColumnNull(2));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 4");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(1));
		EXPECT_TRUE( table.IsColumnNull(2));
	}


	TEST_P(wxCompatibilityTest, SelectNumericTypesAsChar)
	{
		MNumericTypesAsCharTable table(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"0"), std::wstring(table.m_wcdecimal_18_0));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 2");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"123456789012345678"), std::wstring(table.m_wcdecimal_18_0));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 3");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"-123456789012345678"), std::wstring(table.m_wcdecimal_18_0));

		// DB2 sends a ',', mysql/ms sends a '.' as delimeter
		RecordProperty("Ticket", 35);
		if(m_db.Dbms() == dbmsDB2)
		{
			table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 4");
			EXPECT_TRUE(table.SelectNext());
			EXPECT_EQ( std::wstring(L"0,0000000000"), std::wstring(table.m_wcdecimal_18_10));	

			table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 5");
			EXPECT_TRUE(table.SelectNext());
			EXPECT_EQ( std::wstring(L"12345678,9012345678"), std::wstring(table.m_wcdecimal_18_10));	

			table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 6");
			EXPECT_TRUE(table.SelectNext());
			EXPECT_EQ( std::wstring(L"-12345678,9012345678"), std::wstring(table.m_wcdecimal_18_10));	
		}
		else
		{
			table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 4");
			EXPECT_TRUE(table.SelectNext());
			// ms does not send first 0 ?
			if(m_db.Dbms() == dbmsMS_SQL_SERVER)
				EXPECT_EQ( std::wstring(L".0000000000"), std::wstring(table.m_wcdecimal_18_10));	
			else
				EXPECT_EQ( std::wstring(L"0.0000000000"), std::wstring(table.m_wcdecimal_18_10));	

			table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 5");
			EXPECT_TRUE(table.SelectNext());
			EXPECT_EQ( std::wstring(L"12345678.9012345678"), std::wstring(table.m_wcdecimal_18_10));	

			table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 6");
			EXPECT_TRUE(table.SelectNext());
			EXPECT_EQ( std::wstring(L"-12345678.9012345678"), std::wstring(table.m_wcdecimal_18_10));	
		}

		// Test for NULL
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(1) );
		EXPECT_TRUE( table.IsColumnNull(2) );

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 4");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_TRUE( table.IsColumnNull(1) );
		EXPECT_FALSE( table.IsColumnNull(2) );

	}


	TEST_P(wxCompatibilityTest, SelectBlobTypes)
	{
		MBlobTypesTable table(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 1");
		EXPECT_TRUE(table.SelectNext());
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

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ(0, memcmp(empty, table.m_blob, sizeof(table.m_blob)));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 2");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ(0, memcmp(ff, table.m_blob, sizeof(table.m_blob)));

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 3");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ(0, memcmp(abc, table.m_blob, sizeof(table.m_blob)));

		// Test for NULL
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(1) );

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 4");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_TRUE( table.IsColumnNull(1) );
	}



	TEST_P(wxCompatibilityTest, SelectIncompleteTable)
	{
		MCharTable table(m_db, m_odbcInfo.m_namesCase);
		MIncompleteCharTable incTable(m_db, m_odbcInfo.m_namesCase);
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));
		ASSERT_NO_THROW(incTable.Open(m_db, TOF_NONE));

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM exodbc.chartable WHERE idchartable >= 0";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans() );

		// Select using '*' on complete table
		sqlstmt = L"INSERT INTO exodbc.chartable (idchartable, col2, col3, col4) VALUES (1, 'r1_c2', 'r1_c3', 'r1_c4')";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.chartable WHERE idchartable = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ(std::wstring(L"r1_c2"), boost::trim_right_copy(std::wstring(table.m_col2)));
		EXPECT_EQ(std::wstring(L"r1_c3"), boost::trim_right_copy(std::wstring(table.m_col3)));
		EXPECT_EQ(std::wstring(L"r1_c4"), boost::trim_right_copy(std::wstring(table.m_col4)));
		EXPECT_FALSE(table.SelectNext());

		// Select with fields on complete table
		table.m_col2[0] = 0;
		table.m_col3[0] = 0;
		table.m_col4[0] = 0;
		table.SelectBySqlStmt(L"SELECT idchartable, col2, col3, col4 FROM exodbc.chartable WHERE idchartable = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ(std::wstring(L"r1_c2"), boost::trim_right_copy(std::wstring(table.m_col2)));
		EXPECT_EQ(std::wstring(L"r1_c3"), boost::trim_right_copy(std::wstring(table.m_col3)));
		EXPECT_EQ(std::wstring(L"r1_c4"), boost::trim_right_copy(std::wstring(table.m_col4)));
		EXPECT_FALSE(table.SelectNext());

		// Now test by reading the incomplete table using '*': It works as we've still used the indexes "as in the db" when we've bound the columns
		incTable.m_col2[0] = 0;
		incTable.m_col4[0] = 0;
		incTable.SelectBySqlStmt(L"SELECT * FROM exodbc.chartable WHERE idchartable = 1");
		EXPECT_TRUE(incTable.SelectNext());
		EXPECT_EQ(std::wstring(L"r1_c2"), boost::trim_right_copy(std::wstring(incTable.m_col2)));
		EXPECT_EQ(std::wstring(L"r1_c4"), boost::trim_right_copy(std::wstring(incTable.m_col4)));
		EXPECT_FALSE(incTable.SelectNext());

		// It does not work if you do not use the '*' to select, see ticket #15
		// because the returned recordset only has indexes	0 (id), 1 (col2), 2 (col4) 
		// but we've bound									0 (id), 1 (col2), 3 (col4)
		// so 0 (id), 1 (col2) match, but index 3 is not populated
		RecordProperty("Ticket", 15);
		incTable.m_col2[0] = 0;
		incTable.m_col4[0] = 0;
		incTable.SelectBySqlStmt(L"SELECT idchartable, col2, col4 FROM exodbc.chartable WHERE idchartable = 1");
		EXPECT_TRUE(incTable.SelectNext());
		EXPECT_EQ(std::wstring(L"r1_c2"), boost::trim_right_copy(wstring(incTable.m_col2)));
		EXPECT_NE(std::wstring(L"r1_c4"), boost::trim_right_copy(wstring(incTable.m_col4)));
		EXPECT_FALSE(incTable.SelectNext());

		// .. But reading using '*' or 'all fields' really works.. -> because we are bound to indexes and like that the indexes match again
		// the recordset returned
		incTable.m_col2[0] = 0;
		incTable.m_col4[0] = 0;
		incTable.SelectBySqlStmt(L"SELECT  idchartable, col2, col3, col4 FROM exodbc.chartable WHERE idchartable = 1");
		EXPECT_TRUE(incTable.SelectNext());
		EXPECT_EQ(std::wstring(L"r1_c2"), boost::trim_right_copy(std::wstring(incTable.m_col2)));
		EXPECT_EQ(std::wstring(L"r1_c4"), boost::trim_right_copy(std::wstring(incTable.m_col4)));
		EXPECT_FALSE(incTable.SelectNext());
	}


	TEST_P(wxCompatibilityTest, ExecSql_InsertCharTypes)
	{
		MWCharTypesTable table(m_db, m_odbcInfo.m_namesCase, L"CharTypes_tmp");
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		std::wstring sqlstmt = L"DELETE FROM exodbc.chartypes_tmp WHERE idchartypes >= 0";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());

		// Note the escaping:
		// IBM DB2 wants to escape ' using '', mysql wants \'
		// MYSQL needs \\ for \ 
		RecordProperty("Ticket", 36);
		if (m_db.Dbms() == dbmsDB2 || m_db.Dbms() == dbmsMS_SQL_SERVER)
		{
			sqlstmt = (boost::wformat(L"INSERT INTO exodbc.chartypes_tmp (idchartypes, tvarchar, tchar) VALUES (1, '%s', '%s')") % L" !\"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~" % L" !\"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~").str();
		}
		else
		{
			sqlstmt = (boost::wformat(L"INSERT INTO exodbc.chartypes_tmp (idchartypes, tvarchar, tchar) VALUES (1, '%s', '%s')") % L" !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~" % L" !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~").str();
		}
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());

		// And note the triming
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.chartypes_tmp ORDER BY idchartypes ASC");
		EXPECT_TRUE( table.SelectNext());
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), std::wstring(table.m_varchar));
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), boost::trim_right_copy(std::wstring(table.m_char)));
		EXPECT_FALSE(table.SelectNext());
	}


	TEST_P(wxCompatibilityTest, ExecSQL_InsertFloatTypes)
	{
		MFloatTypesTable table(m_db, m_odbcInfo.m_namesCase, L"FloatTypes_tmp");
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		std::wstring sqlstmt = L"DELETE FROM exodbc.floattypes_tmp WHERE idfloattypes >= 0";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());

		sqlstmt = L"INSERT INTO exodbc.floattypes_tmp (idfloattypes, tdouble, tfloat) VALUES (1, -3.141592, -3.141)";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.floattypes_tmp WHERE idfloattypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( -3.141592, table.m_double);
		EXPECT_EQ( -3.141, table.m_float);
		EXPECT_FALSE(table.SelectNext());

		sqlstmt = L"INSERT INTO exodbc.floattypes_tmp (idfloattypes, tdouble, tfloat) VALUES (2, 3.141592, 3.141)";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.floattypes_tmp WHERE idfloattypes = 2");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( 3.141592, table.m_double);
		EXPECT_EQ( 3.141, table.m_float);
		EXPECT_FALSE(table.SelectNext());
	}

	TEST_P(wxCompatibilityTest, ExecSQL_InsertNumericTypesAsChar)
	{
		MNumericTypesAsCharTable table(m_db, m_odbcInfo.m_namesCase, L"NumericTypes_tmp");
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM exodbc.numerictypes_tmp WHERE idnumerictypes >= 0";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());

		sqlstmt = L"INSERT INTO exodbc.numerictypes_tmp (idnumerictypes, tdecimal_18_0, tdecimal_18_10) VALUES (1, -123456789012345678, -12345678.9012345678)";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());
		// Note: DB2 sends a ',', mysql sends a '.' as delimeter
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes_tmp WHERE idnumerictypes = 1");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( std::wstring(L"-123456789012345678"), std::wstring(table.m_wcdecimal_18_0));

		if (m_db.Dbms() == dbmsDB2)
			EXPECT_EQ( std::wstring(L"-12345678,9012345678"), std::wstring(table.m_wcdecimal_18_10));	
		else
			EXPECT_EQ( std::wstring(L"-12345678.9012345678"), std::wstring(table.m_wcdecimal_18_10));	

		sqlstmt = L"INSERT INTO exodbc.numerictypes_tmp (idnumerictypes, tdecimal_18_0, tdecimal_18_10) VALUES (2, 123456789012345678, 12345678.9012345678)";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());
		// TODO: DB2 sends a ',', mysql sends a '.' as delimeter
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes_tmp WHERE idnumerictypes = 2");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"123456789012345678"), std::wstring(table.m_wcdecimal_18_0));

		if (m_db.Dbms() == dbmsDB2)
			EXPECT_EQ( std::wstring(L"12345678,9012345678"), std::wstring(table.m_wcdecimal_18_10));	
		else
			EXPECT_EQ( std::wstring(L"12345678.9012345678"), std::wstring(table.m_wcdecimal_18_10));	

		sqlstmt = L"INSERT INTO exodbc.numerictypes_tmp (idnumerictypes, tdecimal_18_0, tdecimal_18_10) VALUES (3, 0, 0.0000000000)";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());
		// Note: DB2 sends a ',', mysql sends a '.' as delimeter
		// (but maybe this also depends on os settings)
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.numerictypes_tmp WHERE idnumerictypes = 3");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"0"), std::wstring(table.m_wcdecimal_18_0));

		if (m_db.Dbms() == dbmsDB2)
			EXPECT_EQ( std::wstring(L"0,0000000000"), std::wstring(table.m_wcdecimal_18_10));	
		else if (m_db.Dbms() == dbmsMY_SQL)
			EXPECT_EQ( std::wstring(L"0.0000000000"), std::wstring(table.m_wcdecimal_18_10));	
		else if (m_db.Dbms() == dbmsMS_SQL_SERVER)
			EXPECT_EQ( std::wstring(L".0000000000"), std::wstring(table.m_wcdecimal_18_10));	
		else
			EXPECT_TRUE(false);
	}


	TEST_P(wxCompatibilityTest, ExecSQL_InsertIntTypes)
	{
		MIntTypesTable table(m_db, m_odbcInfo.m_namesCase, L"IntegerTypes_tmp");
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM exodbc.integertypes_tmp WHERE idintegertypes >= 0";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes_tmp");
		EXPECT_FALSE(table.SelectNext());

		sqlstmt = L"INSERT INTO exodbc.integertypes_tmp (idintegertypes, tsmallint, tint, tbigint) VALUES (1, -32768, -2147483648, -9223372036854775808)";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());

		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes_tmp ORDER BY idintegertypes ASC");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( -32768, table.m_smallInt);
		EXPECT_EQ( INT_MIN, table.m_int);
		EXPECT_EQ( -LLONG_MIN, table.m_bigInt);
		EXPECT_FALSE(table.SelectNext());

		sqlstmt = L"INSERT INTO exodbc.integertypes_tmp (idintegertypes, tsmallint, tint, tbigint) VALUES (2, 32767, 2147483647, 9223372036854775807)";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.integertypes_tmp ORDER BY idintegertypes ASC");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( 32767, table.m_smallInt);
		EXPECT_EQ( 2147483647, table.m_int);
		EXPECT_EQ( 9223372036854775807, table.m_bigInt);
		EXPECT_FALSE(table.SelectNext());
	}


	TEST_P(wxCompatibilityTest, ExecSQL_InsertDateTypes)
	{
		MDateTypesTable table(m_db, m_odbcInfo.m_namesCase, L"DateTypes_tmp");
		ASSERT_NO_THROW(table.Open(m_db, TOF_NONE));

		std::wstring sqlstmt;
		sqlstmt = L"DELETE FROM exodbc.datetypes_tmp WHERE iddatetypes >= 0";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());

		sqlstmt = L"INSERT INTO exodbc.datetypes_tmp (iddatetypes, tdate, ttime, ttimestamp) VALUES (1, '1983-01-26', '13:55:56', '1983-01-26 13:55:56')";
		EXPECT_NO_THROW(m_db.ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_db.CommitTrans());
		table.SelectBySqlStmt(L"SELECT * FROM exodbc.datetypes_tmp WHERE iddatetypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( 26, table.m_date.day);
		EXPECT_EQ( 01, table.m_date.month);
		EXPECT_EQ( 1983, table.m_date.year);

		EXPECT_EQ( 13, table.m_time.hour);
		EXPECT_EQ( 55, table.m_time.minute);
		EXPECT_EQ( 56, table.m_time.second);

		EXPECT_EQ( 26, table.m_timestamp.day);
		EXPECT_EQ( 01, table.m_timestamp.month);
		EXPECT_EQ( 1983, table.m_timestamp.year);
		EXPECT_EQ( 13, table.m_timestamp.hour);
		EXPECT_EQ( 55, table.m_timestamp.minute);
		EXPECT_EQ( 56, table.m_timestamp.second);

		EXPECT_FALSE(table.SelectNext());
	}



	//Interfaces
	// ----------
	// 

}