/*!
* \file HelpersTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 09.08.2014
* \copyright wxWindows Library Licence, Version 3.1
* 
* [Brief CPP-file description]
*/ 

#include "stdafx.h"

// Own header
#include "HelpersTest.h"

// Same component headers
// Other headers


// Debug
#include "DebugNew.h"

namespace exodbc
{


	// Static consts
	// -------------

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------

	// ParamHelpersTest
	// ================
	void ParamHelpersTest::SetUp()
	{
		// Set up is called for every test
		m_odbcInfo = GetParam();
		ASSERT_NO_THROW(m_env.AllocateEnvironmentHandle());
		ASSERT_NO_THROW(m_env.SetOdbcVersion(OdbcVersion::V_3));

		ASSERT_NO_THROW(m_db.AllocateConnectionHandle(m_env));
		ASSERT_NO_THROW(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
	}

	void ParamHelpersTest::TearDown()
	{

	}


	TEST_P(ParamHelpersTest, AllocateStatementHandle)
	{
		// We expect failure if we pass an invalid handle
		SQLHDBC hNull = SQL_NULL_HDBC;
		{
			DontDebugBreak ddb;
			LogLevelFatal llf;
			EXPECT_THROW(AllocateStatementHandle(hNull), AssertionException);
		}

		// But success on a valid handle
		ASSERT_TRUE(m_db.HasConnectionHandle());
		SQLHSTMT hStmt = SQL_NULL_HSTMT;
		EXPECT_NO_THROW(hStmt = AllocateStatementHandle(m_db.GetConnectionHandle()));

		EXPECT_NO_THROW(FreeStatementHandle(hStmt));
	}


	TEST_P(ParamHelpersTest, FreeStatementHandle)
	{
		// We expect failure if we pass an invalid handle and have the flag INVALID set
		SQLHSTMT hNull = SQL_NULL_HSTMT;
		{
			DontDebugBreak ddb;
			LogLevelFatal llf;
			EXPECT_THROW(FreeStatementHandle(hNull, FSTF_THROW_ON_SQL_INVALID_HANDLE), SqlResultException);
		}

		// But not if we pass only the error flag or the no-throw flag
		EXPECT_NO_THROW(FreeStatementHandle(hNull, FSTF_THROW_ON_SQL_ERROR));
		EXPECT_NO_THROW(FreeStatementHandle(hNull, FSTF_NO_THROW));

		// Now take a valid handle, this should not throw
		ASSERT_TRUE(m_db.HasConnectionHandle());
		SQLHSTMT hStmt = SQL_NULL_HSTMT;
		EXPECT_NO_THROW(hStmt = AllocateStatementHandle(m_db.GetConnectionHandle()));

		EXPECT_NO_THROW(FreeStatementHandle(hStmt));
	}


	TEST_P(ParamHelpersTest, CloseStmtHandle)
	{
		SQLHSTMT hNull = SQL_NULL_HSTMT;
		{
			// We assert if we pass a null handle
			DontDebugBreak ddb;
			LogLevelFatal llf;
			EXPECT_THROW(CloseStmtHandle(hNull, StmtCloseMode::IgnoreNotOpen), AssertionException);
		}

		// Allocate a valid statement handle
		SQLHSTMT hStmt = SQL_NULL_HSTMT;
		ASSERT_NO_THROW(hStmt = AllocateStatementHandle(m_db.GetConnectionHandle()));

		// This is not open yet, we shall fail to free it
		{
			if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
			{
				LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #120");
			}
			DontDebugBreak ddb;
			LogLevelFatal llf;
			EXPECT_THROW(CloseStmtHandle(hStmt, StmtCloseMode::ThrowIfNotOpen), SqlResultException);
		}
		// But not if we ignore the cursor-state
		EXPECT_NO_THROW(CloseStmtHandle(hStmt, StmtCloseMode::IgnoreNotOpen));

		// Open statement by doing some operation on it
		std::wstring sqlstmt;
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM %s") % test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase));
		}
		else
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM exodbc.%s") % test::GetTableName(test::TableId::INTEGERTYPES, m_odbcInfo.m_namesCase));
		}
		// convert schema name to upper if needed
		if (m_odbcInfo.m_namesCase == test::Case::UPPER)
		{
			boost::algorithm::to_upper(sqlstmt);
		}

		SQLRETURN ret = SQLExecDirect(hStmt, (SQLWCHAR*) sqlstmt.c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		// Closing it first time must work
		EXPECT_NO_THROW(CloseStmtHandle(hStmt, StmtCloseMode::ThrowIfNotOpen));

		// Closing it a second time must fail
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #120");
		}
		DontDebugBreak ddb;
		LogLevelFatal llf;
		EXPECT_THROW(CloseStmtHandle(hStmt, StmtCloseMode::ThrowIfNotOpen), SqlResultException);

		// Close the statement
		EXPECT_NO_THROW(FreeStatementHandle(hStmt));
	}


	TEST_P(ParamHelpersTest, GetInfo)
	{
		// Simply test reading a string and an int value works - that means, does return some data
		std::wstring serverName;

		ASSERT_TRUE(m_db.HasConnectionHandle());
		GetInfo(m_db.GetConnectionHandle(), SQL_SERVER_NAME, serverName);
		EXPECT_FALSE(serverName.empty());

		// and read some int value
		// okay, it would be bad luck if one driver reports 1313
		SQLUSMALLINT maxStmts = 1313;
		SWORD cb = 0;
		GetInfo(m_db.GetConnectionHandle(), SQL_MAX_CONCURRENT_ACTIVITIES, &maxStmts, sizeof(maxStmts), &cb);
		EXPECT_NE(1313, maxStmts);
		EXPECT_NE(0, cb);
	}


	TEST_P(ParamHelpersTest, GetData)
	{
		// Test by reading some char value

		// Allocate a valid statement handle
		SQLHSTMT hStmt = SQL_NULL_HSTMT;
		ASSERT_NO_THROW(hStmt = AllocateStatementHandle(m_db.GetConnectionHandle()));

		// Open statement by doing some operation on it
		std::wstring sqlstmt;
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM %s WHERE %s = 3") % test::GetTableName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase) % test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase));
		}
		else
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM exodbc.%s WHERE %s = 3") % test::GetTableName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase) % test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase));
		}
		// convert schema name to upper if needed
		if (m_odbcInfo.m_namesCase == test::Case::UPPER)
		{
			boost::algorithm::to_upper(sqlstmt);
		}
		SQLRETURN ret = SQLExecDirect(hStmt, (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));		
		ret = SQLFetch(hStmt);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		// Read some non-null string-data with enough chars
		bool isNull = false;
		std::wstring value;
		EXPECT_NO_THROW(GetData(hStmt, 2, 20, value, &isNull));
		EXPECT_FALSE(isNull);
		EXPECT_EQ(L"������", value);
		EXPECT_NO_THROW(CloseStmtHandle(hStmt, StmtCloseMode::IgnoreNotOpen));

		// Trim the read-value in GetData
		ret = SQLExecDirect(hStmt, (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		ret = SQLFetch(hStmt);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		EXPECT_NO_THROW(GetData(hStmt, 2, 3, value, &isNull));
		EXPECT_EQ(L"���", value);
		EXPECT_NO_THROW(CloseStmtHandle(hStmt, StmtCloseMode::IgnoreNotOpen));

		// Read some int value
		ret = SQLExecDirect(hStmt, (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		ret = SQLFetch(hStmt);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		SQLINTEGER id = 0;
		SQLLEN ind = 0;
		EXPECT_NO_THROW(GetData(hStmt, 1, SQL_C_SLONG, &id, NULL, &ind, &isNull));
		EXPECT_EQ(3, id);
		EXPECT_NO_THROW(CloseStmtHandle(hStmt, StmtCloseMode::IgnoreNotOpen));

		// And test at least one null value
		ret = SQLExecDirect(hStmt, (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		ret = SQLFetch(hStmt);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		value = L"";
		EXPECT_NO_THROW(GetData(hStmt, 3, 20, value, &isNull));
		EXPECT_TRUE(isNull);
		EXPECT_EQ(L"", value);
		
		EXPECT_NO_THROW(CloseStmtHandle(hStmt, StmtCloseMode::IgnoreNotOpen));
		EXPECT_NO_THROW(FreeStatementHandle(hStmt));

	}


	TEST_P(ParamHelpersTest, GetRowDescriptorHandle)
	{
		// Open a statement, to test getting the row-descriptor
		SQLHSTMT hStmt = SQL_NULL_HSTMT;
		ASSERT_NO_THROW(hStmt = AllocateStatementHandle(m_db.GetConnectionHandle()));

		// Open statement by doing some operation on it
		std::wstring sqlstmt;
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM %s WHERE %s = 3") % test::GetTableName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase) % test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase));
		}
		else
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM exodbc.%s WHERE %s = 3") % test::GetTableName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase) % test::GetIdColumnName(test::TableId::CHARTYPES, m_odbcInfo.m_namesCase));
		}
		if (m_odbcInfo.m_namesCase == test::Case::UPPER)
		{
			boost::algorithm::to_upper(sqlstmt);
		}
		SQLRETURN ret = SQLExecDirect(hStmt, (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		// Test getting descriptor-handle
		SQLHDESC hDesc = SQL_NULL_HDESC;
		// We shall fail if we pass an invalid handle
		{
			LogLevelFatal llf;
			DontDebugBreak ddb;
			EXPECT_THROW(GetRowDescriptorHandle(SQL_NULL_HSTMT, RowDescriptorType::ROW), AssertionException);
		}

		// but not by passing the valid statement
		EXPECT_NO_THROW(hDesc = GetRowDescriptorHandle(hStmt, RowDescriptorType::ROW));
		EXPECT_NO_THROW(hDesc = GetRowDescriptorHandle(hStmt, RowDescriptorType::PARAM));

		// Close things
		EXPECT_NO_THROW(CloseStmtHandle(hStmt, StmtCloseMode::IgnoreNotOpen));
		EXPECT_NO_THROW(FreeStatementHandle(hStmt));
	}


	TEST_P(ParamHelpersTest, SetDescriptionField)
	{
		// Test by setting the description of a numeric column

		// Open a statement,
		SQLHSTMT hStmt = SQL_NULL_HSTMT;
		ASSERT_NO_THROW(hStmt = AllocateStatementHandle(m_db.GetConnectionHandle()));

		// Open statement by doing some operation on it
		std::wstring sqlstmt;
		if (m_db.GetDbms() == DatabaseProduct::ACCESS)
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM %s WHERE %s = 5") % test::GetTableName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase) % test::GetIdColumnName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase));
		}
		else
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM exodbc.%s WHERE %s = 5") % test::GetTableName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase) % test::GetIdColumnName(test::TableId::NUMERICTYPES, m_odbcInfo.m_namesCase));
		}
		if (m_odbcInfo.m_namesCase == test::Case::UPPER)
		{
			boost::algorithm::to_upper(sqlstmt);
		}
		SQLRETURN ret = SQLExecDirect(hStmt, (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		// Except to fail when passing a null handle
		{
			LogLevelFatal llf;
			DontDebugBreak ddb;
			EXPECT_THROW(SetDescriptionField(SQL_NULL_HSTMT, 3, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC), AssertionException);
		}

		SQLHANDLE hDesc = SQL_NULL_HDESC;
		EXPECT_NO_THROW(hDesc = GetRowDescriptorHandle(hStmt, RowDescriptorType::PARAM));

		SQL_NUMERIC_STRUCT num;
		EXPECT_NO_THROW(SetDescriptionField(hDesc, 3, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC));
		EXPECT_NO_THROW(SetDescriptionField(hDesc, 3, SQL_DESC_PRECISION, (SQLPOINTER)18));
		EXPECT_NO_THROW(SetDescriptionField(hDesc, 3, SQL_DESC_SCALE, (SQLPOINTER)10));
		EXPECT_NO_THROW(SetDescriptionField(hDesc, 3, SQL_DESC_DATA_PTR, (SQLPOINTER)&num));

		// Close things
		EXPECT_NO_THROW(CloseStmtHandle(hStmt, StmtCloseMode::IgnoreNotOpen));
		EXPECT_NO_THROW(FreeStatementHandle(hStmt));
	}


	// StaticHelpersTest
	// =================

	void StaticHelpersTest::SetUp()
	{

	}


	void StaticHelpersTest::TearDown()
	{

	}


	TEST_F(StaticHelpersTest, GetAllErrors)
	{
		// We except an assertion if not at least one handle is valid
		{
			DontDebugBreak ddb;
			LogLevelFatal llf;
			EXPECT_THROW(GetAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, SQL_NULL_HSTMT, SQL_NULL_HDESC), AssertionException);
		}
	}


	TEST_F(StaticHelpersTest, DontDebugBreak)
	{
		// DebugBreak shall default to false (that means we will break)
		EXPECT_FALSE(GetDontDebugBreak());
		{
			// Disable breaking
			DontDebugBreak ddb;
			EXPECT_TRUE(GetDontDebugBreak());
		}
		// and here we break again again
		EXPECT_FALSE(GetDontDebugBreak());
	}


	TEST_F(StaticHelpersTest, InitTime)
	{
		SQL_TIME_STRUCT time = InitTime(13, 14, 15);
		EXPECT_EQ(13, time.hour);
		EXPECT_EQ(14, time.minute);
		EXPECT_EQ(15, time.second);
	}


#if HAVE_MSODBCSQL_H
	TEST_F(StaticHelpersTest, InitTime2)
	{
		SQL_SS_TIME2_STRUCT time2 = InitTime2(13, 01, 17, 123456000);
		EXPECT_EQ(13, time2.hour);
		EXPECT_EQ(1, time2.minute);
		EXPECT_EQ(17, time2.second);
		EXPECT_EQ(123456000, time2.fraction);
	}
#endif


	TEST_F(StaticHelpersTest, InitDate)
	{
		SQL_DATE_STRUCT date = InitDate(26, 1, 1983);
		EXPECT_EQ(26, date.day);
		EXPECT_EQ(1, date.month);
		EXPECT_EQ(1983, date.year);
	}


	TEST_F(StaticHelpersTest, InitTimestamp)
	{
		SQL_TIMESTAMP_STRUCT ts = InitTimestamp(13, 14, 15, 123456789, 26, 1, 1983);
		EXPECT_EQ(13, ts.hour);
		EXPECT_EQ(14, ts.minute);
		EXPECT_EQ(15, ts.second);
		EXPECT_EQ(123456789, ts.fraction);
		EXPECT_EQ(26, ts.day);
		EXPECT_EQ(1, ts.month);
		EXPECT_EQ(1983, ts.year);
	}


	TEST_F(StaticHelpersTest, InitNumeric)
	{
		SQLCHAR val[SQL_MAX_NUMERIC_LEN];
		FillMemory(val, SQL_MAX_NUMERIC_LEN, 1);
		SQL_NUMERIC_STRUCT num = InitNumeric(18, 10, 1, val);
		EXPECT_EQ(18, num.precision);
		EXPECT_EQ(10, num.scale);
		EXPECT_EQ((SQLCHAR)1, num.sign);
		EXPECT_TRUE(memcmp(val, num.val, SQL_MAX_NUMERIC_LEN) == 0);
	}


	TEST_F(StaticHelpersTest, InitNullNumeric)
	{
		char nullMem[sizeof(SQL_NUMERIC_STRUCT)];
		ZeroMemory(nullMem, sizeof(SQL_NUMERIC_STRUCT));

		SQL_NUMERIC_STRUCT num = InitNullNumeric();
		EXPECT_TRUE(memcmp(&num, &nullMem, sizeof(SQL_NUMERIC_STRUCT)) == 0);
	}


	TEST_F(StaticHelpersTest, IsTimeEqual)
	{
		SQL_TIME_STRUCT t1 = InitTime(13, 14, 15);
		SQL_TIME_STRUCT t2 = InitTime(07, 14, 27);
		SQL_TIME_STRUCT t3 = InitTime(13, 14, 15);
		EXPECT_TRUE(IsTimeEqual(t1, t3));
		EXPECT_FALSE(IsTimeEqual(t1, t2));
		EXPECT_FALSE(IsTimeEqual(t3, t2));

	}


	TEST_F(StaticHelpersTest, IsDateEqual)
	{
		SQL_DATE_STRUCT d1 = InitDate(26, 01, 1983);
		SQL_DATE_STRUCT d2 = InitDate(25, 04, 1983);
		SQL_DATE_STRUCT d3 = InitDate(26, 01, 1983);
		EXPECT_TRUE(IsDateEqual(d1, d3));
		EXPECT_FALSE(IsDateEqual(d1, d2));
		EXPECT_FALSE(IsDateEqual(d3, d2));
	}


	TEST_F(StaticHelpersTest, IsTimestampEqual)
	{
		SQL_TIMESTAMP_STRUCT ts1 = InitTimestamp(13, 14, 15, 123456789, 26, 1, 1983);
		SQL_TIMESTAMP_STRUCT ts2 = InitTimestamp(13, 14, 15, 12345, 26, 1, 1983);
		SQL_TIMESTAMP_STRUCT ts3 = InitTimestamp(13, 14, 15, 123456789, 26, 1, 1983);

		EXPECT_TRUE(IsTimestampEqual(ts1, ts3));
		EXPECT_FALSE(IsTimestampEqual(ts1, ts2));
		EXPECT_FALSE(IsTimestampEqual(ts3, ts2));
	}

	// Interfaces
	// ----------

} // namespace exodbc