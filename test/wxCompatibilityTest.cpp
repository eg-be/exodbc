/*!
* \file wxCompatibilityTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 31.08.2014
* \copyright GNU Lesser General Public License Version 3
* 
* [Brief CPP-file description]
*/ 

// Own header
#include "wxCompatibilityTest.h"

// Same component headers
#include "ManualTestTables.h"
#include "exOdbcTestHelpers.h"

// Other headers
#include "exodbc/Database.h"

// Debug
#include "DebugNew.h"

using namespace std;
using namespace exodbc;

namespace exodbctest
{
	//Static consts
	// -------------
	
	//Construction
	// -------------

	void wxCompatibilityTest::SetUp()
	{
		// Set up environment
		m_pEnv->Init(OdbcVersion::V_3);

		// And database
		ASSERT_NO_THROW(m_pDb = OpenTestDb(m_pEnv));
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
	TEST_F(wxCompatibilityTest, OpenTableCheckExistance)
	{
		MCharTypesTable charTypesTable(m_pDb);
		EXPECT_NO_THROW(charTypesTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		MIntTypesTable intTypesTable(m_pDb);
		EXPECT_NO_THROW(intTypesTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		MDateTypesTable dateTypesTable(m_pDb);
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			// Note: Sql Server does not report time as supported datatype, but rather time2.
			// but if we wish to use that, we would need odbc 3.8
			EXPECT_NO_THROW(dateTypesTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));
		}
		else
		{
			EXPECT_NO_THROW(dateTypesTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));
		}

		MFloatTypesTable floatTypesTable(m_pDb);
		EXPECT_NO_THROW(floatTypesTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));
		
		// we do not have a numeric table on all test-sources
//		MNumericTypesTable numericTypesTable(m_pDb, NumericTypesTable::ReadAsChar, m_odbcInfo.m_namesCase);
//		EXPECT_TRUE(numericTypesTable.Open(false, false));

		MBlobTypesTable blobTypesTable(m_pDb);
		EXPECT_NO_THROW(blobTypesTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));
	}


	// Test basic GetNext
	TEST_F(wxCompatibilityTest, GetNextTest)
	{
		MCharTypesTable table(m_pDb);
		ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		// Expect 6 entries
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.chartypes");
		int count = 0;
		while(table.SelectNext() && count <= 7)
		{
			count++;
		}
		EXPECT_EQ(4, count);

		// Expect no entries
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.chartypes WHERE idchartypes = 7");
		EXPECT_FALSE(table.SelectNext());

		// Expect 2 entries
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.chartypes WHERE idchartypes >= 2 AND idchartypes <= 3");
		count = 0;
		while(table.SelectNext() && count <= 3)
		{
			count++;
		}
		EXPECT_EQ(2, count);
	}


	// Test reading datatypes
	TEST_F(wxCompatibilityTest, SelectDateTypes)
	{
		MDateTypesTable table(m_pDb);
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));
		}
		else
		{
			ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));
		}

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.datetypes WHERE iddatetypes = 1");
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			// MS will complain about data loss
			EXPECT_TRUE(table.SelectNext());
		}
		else
		{
			EXPECT_TRUE(table.SelectNext());
		}
		EXPECT_EQ(26, table.m_date.day);
		EXPECT_EQ( 01, table.m_date.month);
		EXPECT_EQ( 1983, table.m_date.year);

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.datetypes WHERE iddatetypes = 1");
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			// MS will complain about data loss
			EXPECT_TRUE(table.SelectNext());
		}
		else
		{
			EXPECT_TRUE(table.SelectNext());
		}
		EXPECT_EQ( 13, table.m_time.hour);
		EXPECT_EQ( 55, table.m_time.minute);
		EXPECT_EQ( 56, table.m_time.second);

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.datetypes WHERE iddatetypes = 2");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( 26, table.m_timestamp.day);
		EXPECT_EQ( 01, table.m_timestamp.month);
		EXPECT_EQ( 1983, table.m_timestamp.year);
		EXPECT_EQ( 13, table.m_timestamp.hour);
		EXPECT_EQ( 55, table.m_timestamp.minute);
		EXPECT_EQ( 56, table.m_timestamp.second);

		SQLUINTEGER fraction = 0;
		switch(m_pDb->GetDbms())
		{
		case DatabaseProduct::DB2:
		case DatabaseProduct::POSTGRESQL:
			fraction = 123456000;
			break;
		case DatabaseProduct::MS_SQL_SERVER:
			fraction = 123000000;
			break;
		default:
			fraction = 0;
			break;
		}
		EXPECT_EQ( fraction, table.m_timestamp.fraction);

		// Test for NULL
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.datetypes WHERE iddatetypes = 2");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_FALSE( table.IsColumnNull(0) );
		EXPECT_TRUE(table.IsColumnNull(1));
		EXPECT_TRUE(table.IsColumnNull(2));
		EXPECT_FALSE(table.IsColumnNull(3));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.datetypes WHERE iddatetypes = 3");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_FALSE(table.IsColumnNull(0));
		EXPECT_TRUE(table.IsColumnNull(1));
		EXPECT_TRUE(table.IsColumnNull(3));
	}


	TEST_F(wxCompatibilityTest, SelectIntTypes)
	{
		MIntTypesTable table(m_pDb);
		ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 1");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( -32768, table.m_smallInt);

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 2");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( 32767, table.m_smallInt);

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 3");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( INT_MIN /* -2147483648 */, table.m_int);

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 4");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( 2147483647, table.m_int);

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 5");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( LLONG_MIN /* -9223372036854775808 */, table.m_bigInt);

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 6");
		EXPECT_TRUE( table.SelectNext() );
		EXPECT_EQ( 9223372036854775807, table.m_bigInt);

		// Test for NULL-Values
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 1");
		EXPECT_TRUE( table.SelectNext());

		EXPECT_FALSE( table.IsColumnNull(0) );
		EXPECT_FALSE( table.IsColumnNull(1) );
		EXPECT_TRUE( table.IsColumnNull(2) );
		EXPECT_TRUE( table.IsColumnNull(3) );

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 3");
		EXPECT_TRUE( table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(2) );

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 5");
		EXPECT_TRUE( table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(3) );
	}


#ifdef _WIN32
	TEST_F(wxCompatibilityTest, SelectWCharTypes)
	{
		MWCharTypesTable table(m_pDb);
		ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		// Note: We trim the values read from the db on the right side, as for example db2 pads with ' ' by default

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.chartypes WHERE idchartypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_varchar)));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.chartypes WHERE idchartypes = 2");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), boost::trim_right_copy(std::wstring(reinterpret_cast<const wchar_t*>(table.m_char))));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.chartypes WHERE idchartypes = 3");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"abcdef"), boost::trim_right_copy(std::wstring(reinterpret_cast<const wchar_t*>(table.m_varchar))));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.chartypes WHERE idchartypes = 4");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"abcdef"), boost::trim_right_copy(std::wstring(reinterpret_cast<const wchar_t*>(table.m_char))));

		// Test for NULL-Values
		EXPECT_FALSE( table.IsColumnNull(0) );
		EXPECT_TRUE( table.IsColumnNull(1) );
		EXPECT_FALSE( table.IsColumnNull(2) );

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.chartypes WHERE idchartypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(0) );
		EXPECT_FALSE( table.IsColumnNull(1) );
		EXPECT_TRUE( table.IsColumnNull(2) );
	}
#endif


	TEST_F(wxCompatibilityTest, SelectFloatTypes)
	{		
		MFloatTypesTable table(m_pDb);
		ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( 0.0, table.m_float);

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 2");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( (int)(1e3 * 2.684), (int)(1e3 * table.m_float));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 3");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( (int)(1e3 * -2.684), (int)(1e3 * table.m_float));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 4");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( 0.0, table.m_double);

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 5");
		EXPECT_TRUE(table.SelectNext());
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			EXPECT_EQ((int)(1e6 * 3.141592), (int)(1e6 * table.m_doubleAsFloat));
		}
		else
		{
			EXPECT_EQ((int)(1e6 * 3.141592), (int)(1e6 * table.m_double));
		}

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 6");
		EXPECT_TRUE(table.SelectNext());
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			EXPECT_EQ((int)(1e6 * -3.141592), (int)(1e6 * table.m_doubleAsFloat));
		}
		else
		{
			EXPECT_EQ((int)(1e6 * -3.141592), (int)(1e6 * table.m_double));
		}

		// Test for NULL
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_TRUE( table.IsColumnNull(1));
		EXPECT_FALSE( table.IsColumnNull(2));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.floattypes WHERE idfloattypes = 4");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(1));
		EXPECT_TRUE( table.IsColumnNull(2));
	}

#ifdef _WIN32
	TEST_F(wxCompatibilityTest, SelectNumericTypesAsChar)
	{
		MNumericTypesAsCharTable table(m_pDb);
		ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"0"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_wcdecimal_18_0)));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 2");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"123456789012345678"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_wcdecimal_18_0)));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 3");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( std::wstring(L"-123456789012345678"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_wcdecimal_18_0)));

		// DB2 sends a ',', mysql/ms sends a '.' as delimeter
		RecordProperty("Ticket", 35);
		if (m_pDb->GetDbms() == DatabaseProduct::DB2)
		{
			table.SelectBySqlStmt(u8"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 4");
			EXPECT_TRUE(table.SelectNext());
			EXPECT_EQ( std::wstring(L"0,0000000000"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_wcdecimal_18_10)));	

			table.SelectBySqlStmt(u8"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 5");
			EXPECT_TRUE(table.SelectNext());
			EXPECT_EQ( std::wstring(L"12345678,9012345678"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_wcdecimal_18_10)));	

			table.SelectBySqlStmt(u8"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 6");
			EXPECT_TRUE(table.SelectNext());
			EXPECT_EQ( std::wstring(L"-12345678,9012345678"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_wcdecimal_18_10)));	
		}
		else
		{
			table.SelectBySqlStmt(u8"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 4");
			EXPECT_TRUE(table.SelectNext());
			// ms does not send first 0 ?
			if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
				EXPECT_EQ( std::wstring(L".0000000000"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_wcdecimal_18_10)));	
			else
				EXPECT_EQ( std::wstring(L"0.0000000000"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_wcdecimal_18_10)));	

			table.SelectBySqlStmt(u8"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 5");
			EXPECT_TRUE(table.SelectNext());
			EXPECT_EQ( std::wstring(L"12345678.9012345678"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_wcdecimal_18_10)));	

			table.SelectBySqlStmt(u8"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 6");
			EXPECT_TRUE(table.SelectNext());
			EXPECT_EQ( std::wstring(L"-12345678.9012345678"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_wcdecimal_18_10)));	
		}

		// Test for NULL
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(1) );
		EXPECT_TRUE( table.IsColumnNull(2) );

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.numerictypes WHERE idnumerictypes = 4");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_TRUE( table.IsColumnNull(1) );
		EXPECT_FALSE( table.IsColumnNull(2) );

	}
#endif

	TEST_F(wxCompatibilityTest, SelectBlobTypes)
	{
		MBlobTypesTable table(m_pDb);
		ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 1");
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

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ(0, memcmp(empty, table.m_blob, sizeof(table.m_blob)));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 2");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ(0, memcmp(ff, table.m_blob, sizeof(table.m_blob)));

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 3");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ(0, memcmp(abc, table.m_blob, sizeof(table.m_blob)));

		// Test for NULL
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_FALSE( table.IsColumnNull(1) );

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.blobtypes WHERE idblobtypes = 4");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_TRUE( table.IsColumnNull(1) );
	}
	

	TEST_F(wxCompatibilityTest, SelectUsingStarTable)
	{
		// Selecting using the '*' will work if all columns are bound
		MIntTypesTable intTable(m_pDb);
		ASSERT_NO_THROW(intTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		std::string sqlstmt;
		intTable.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 4");
		EXPECT_TRUE(intTable.SelectNext());
		EXPECT_FALSE(intTable.IsColumnNull(0));
		EXPECT_TRUE(intTable.IsColumnNull(1));
		EXPECT_FALSE(intTable.IsColumnNull(2));
		EXPECT_TRUE(intTable.IsColumnNull(3));
		EXPECT_EQ(4, intTable.m_idIntegerTypes);
		EXPECT_EQ(2147483647, intTable.m_int);

		intTable.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 7");
		EXPECT_TRUE(intTable.SelectNext());
		EXPECT_FALSE(intTable.IsColumnNull(0));
		EXPECT_FALSE(intTable.IsColumnNull(1));
		EXPECT_FALSE(intTable.IsColumnNull(2));
		EXPECT_FALSE(intTable.IsColumnNull(3));
		EXPECT_EQ(7, intTable.m_idIntegerTypes);
		EXPECT_EQ(-13, intTable.m_smallInt);
		EXPECT_EQ(26, intTable.m_int);
		EXPECT_EQ(10502, intTable.m_bigInt);
	}


	TEST_F(wxCompatibilityTest, SelectIncompleteTable)
	{
		// Selecting using the '*' will not work if not columns are bound:
		// we bind column 1, 2 and 4, but in column 4 we will get the result of the 
		// not bound column 3 if we select using '*':
		Table intTable(m_pDb, TableAccessFlag::AF_READ, ToDbCase(GetTableName(TableId::INTEGERTYPES)));

		SQLINTEGER idIntegerTypes = 0;
		SQLSMALLINT smallInt = 0;
		SQLBIGINT bigInt = 0;

		intTable.SetColumn(0, ToDbCase(u8"idintegertypes"), SQL_INTEGER, &idIntegerTypes, SQL_C_SLONG, sizeof(idIntegerTypes), ColumnFlag::CF_SELECT | ColumnFlag::CF_PRIMARY_KEY);
		intTable.SetColumn(1, ToDbCase(u8"tsmallint"), SQL_INTEGER, &smallInt, SQL_C_SSHORT, sizeof(smallInt), ColumnFlag::CF_SELECT);
		intTable.SetColumn(2, ToDbCase(u8"tbigint"), SQL_INTEGER, &bigInt, SQL_C_SBIGINT, sizeof(bigInt), ColumnFlag::CF_SELECT);


		ASSERT_NO_THROW(intTable.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		std::string sqlstmt;
		intTable.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes WHERE idintegertypes = 7");
		EXPECT_TRUE(intTable.SelectNext());
		EXPECT_FALSE(intTable.IsColumnNull(0));
		EXPECT_FALSE(intTable.IsColumnNull(1));
		EXPECT_FALSE(intTable.IsColumnNull(2));
		EXPECT_EQ(7, idIntegerTypes);
		EXPECT_EQ(-13, smallInt);
		EXPECT_EQ(26, bigInt);

		// But its not a problem if we list the columns to be selected
		intTable.SelectBySqlStmt(u8"SELECT idintegertypes, tsmallint, tbigint FROM exodbc.integertypes WHERE idintegertypes = 7");
		EXPECT_TRUE(intTable.SelectNext());
		EXPECT_FALSE(intTable.IsColumnNull(0));
		EXPECT_FALSE(intTable.IsColumnNull(1));
		EXPECT_FALSE(intTable.IsColumnNull(2));
		EXPECT_EQ(7, idIntegerTypes);
		EXPECT_EQ(-13, smallInt);
		EXPECT_EQ(10502, bigInt);
	}


#ifdef _WIN32
	TEST_F(wxCompatibilityTest, ExecSql_InsertCharTypes)
	{
		MWCharTypesTable table(m_pDb);
		ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		std::string sqlstmt = u8"DELETE FROM exodbc.chartypes_tmp WHERE idchartypes >= 0";
		EXPECT_NO_THROW(m_pDb->ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_pDb->CommitTrans());

		// Note the escaping:
		// IBM DB2: Escape using '
		// PostgresQL: Escape using '
		// Sql Server: Escape using '
		// MySql: Escape using \
		// (and remember that c++ wants \ )

		// IBM DB2 wants to escape ' using '', mysql wants \'
		// MYSQL needs \\ for \ 
		std::string sqlEscapedValue = u8" !\"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
		if (m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
		{
			sqlEscapedValue = u8" !\"#$%&\\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
		}
		sqlstmt = (boost::format(u8"INSERT INTO exodbc.chartypes_tmp (idchartypes, tvarchar, tchar) VALUES (1, '%s', '%s')") % sqlEscapedValue % sqlEscapedValue).str();
		EXPECT_NO_THROW(m_pDb->ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_pDb->CommitTrans());

		// And note the triming
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.chartypes_tmp ORDER BY idchartypes ASC");
		EXPECT_TRUE( table.SelectNext());
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), std::wstring(reinterpret_cast<const wchar_t*>(table.m_varchar)));
		EXPECT_EQ( std::wstring(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~"), boost::trim_right_copy(std::wstring(reinterpret_cast<const wchar_t*>(table.m_char))));
		EXPECT_FALSE(table.SelectNext());
	}
#endif


	TEST_F(wxCompatibilityTest, ExecSQL_InsertFloatTypes)
	{
		MFloatTypesTable table(m_pDb);
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));
		}
		else
		{
			ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));
		}

		std::string sqlstmt = u8"DELETE FROM exodbc.floattypes_tmp WHERE idfloattypes >= 0";
		EXPECT_NO_THROW(m_pDb->ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_pDb->CommitTrans());

		sqlstmt = u8"INSERT INTO exodbc.floattypes_tmp (idfloattypes, tdouble, tfloat) VALUES (1, -3.141592, -3.141)";
		EXPECT_NO_THROW(m_pDb->ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_pDb->CommitTrans());
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.floattypes_tmp WHERE idfloattypes = 1");
		EXPECT_TRUE(table.SelectNext());
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			EXPECT_EQ((int)(1e6 * -3.141592), (int)(1e6 * table.m_doubleAsFloat));
		}
		else
		{
			EXPECT_EQ((int)(1e6 * -3.141592), (int)(1e6 * table.m_double));
		}
		EXPECT_EQ((int)(1e3 * -3.141), (int)(1e3 * table.m_float));
		EXPECT_FALSE(table.SelectNext());

		sqlstmt = u8"INSERT INTO exodbc.floattypes_tmp (idfloattypes, tdouble, tfloat) VALUES (2, 3.141592, 3.141)";
		EXPECT_NO_THROW(m_pDb->ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_pDb->CommitTrans());
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.floattypes_tmp WHERE idfloattypes = 2");
		EXPECT_TRUE(table.SelectNext());
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			EXPECT_EQ((int)(1e6 * 3.141592), (int)(1e6 * table.m_doubleAsFloat));
		}
		else
		{
			EXPECT_EQ((int)(1e6 * 3.141592), (int)(1e6 * table.m_double));
		}
		EXPECT_EQ((int)(1e3 * 3.141), (int)(1e3 * table.m_float));
		EXPECT_FALSE(table.SelectNext());
	}

	
	TEST_F(wxCompatibilityTest, ExecSQL_InsertIntTypes)
	{
		MIntTypesTable table(m_pDb);
		ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));

		std::string sqlstmt;
		sqlstmt = u8"DELETE FROM exodbc.integertypes_tmp WHERE idintegertypes >= 0";
		EXPECT_NO_THROW(m_pDb->ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_pDb->CommitTrans());

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes_tmp");
		EXPECT_FALSE(table.SelectNext());

		sqlstmt = u8"INSERT INTO exodbc.integertypes_tmp (idintegertypes, tsmallint, tint, tbigint) VALUES (1, -32768, -2147483648, -9223372036854775808)";
		EXPECT_NO_THROW(m_pDb->ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_pDb->CommitTrans());

		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes_tmp ORDER BY idintegertypes ASC");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( -32768, table.m_smallInt);
		EXPECT_EQ( INT_MIN, table.m_int);
		EXPECT_EQ( LLONG_MIN, table.m_bigInt);
		EXPECT_FALSE(table.SelectNext());

		sqlstmt = u8"INSERT INTO exodbc.integertypes_tmp (idintegertypes, tsmallint, tint, tbigint) VALUES (2, 32767, 2147483647, 9223372036854775807)";
		EXPECT_NO_THROW(m_pDb->ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_pDb->CommitTrans());
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.integertypes_tmp ORDER BY idintegertypes ASC");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ( 32767, table.m_smallInt);
		EXPECT_EQ( 2147483647, table.m_int);
		EXPECT_EQ( 9223372036854775807, table.m_bigInt);
		EXPECT_FALSE(table.SelectNext());
	}


	TEST_F(wxCompatibilityTest, ExecSQL_InsertDateTypes)
	{
		MDateTypesTable table(m_pDb);
		if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));
		}
		else
		{
			ASSERT_NO_THROW(table.Open(TableOpenFlag::TOF_CHECK_EXISTANCE));
		}

		std::string sqlstmt;
		sqlstmt = u8"DELETE FROM exodbc.datetypes_tmp WHERE iddatetypes >= 0";
		EXPECT_NO_THROW(m_pDb->ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_pDb->CommitTrans());

		sqlstmt = u8"INSERT INTO exodbc.datetypes_tmp (iddatetypes, tdate, ttime, ttimestamp) VALUES (1, '1983-01-26', '13:55:56', '1983-01-26 13:55:56')";
		EXPECT_NO_THROW(m_pDb->ExecSql(sqlstmt));
		EXPECT_NO_THROW(m_pDb->CommitTrans());
		table.SelectBySqlStmt(u8"SELECT * FROM exodbc.datetypes_tmp WHERE iddatetypes = 1");
		EXPECT_TRUE(table.SelectNext());
		EXPECT_EQ(26, table.m_date.day);
		EXPECT_EQ(01, table.m_date.month);
		EXPECT_EQ(1983, table.m_date.year);

		EXPECT_EQ(13, table.m_time.hour);
		EXPECT_EQ(55, table.m_time.minute);
		EXPECT_EQ(56, table.m_time.second);

		EXPECT_EQ(26, table.m_timestamp.day);
		EXPECT_EQ(01, table.m_timestamp.month);
		EXPECT_EQ(1983, table.m_timestamp.year);
		EXPECT_EQ(13, table.m_timestamp.hour);
		EXPECT_EQ(55, table.m_timestamp.minute);
		EXPECT_EQ(56, table.m_timestamp.second);

		EXPECT_FALSE(table.SelectNext());
	}

}
