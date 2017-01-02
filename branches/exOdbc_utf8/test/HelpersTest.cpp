/*!
* \file HelpersTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 09.08.2014
* \copyright GNU Lesser General Public License Version 3
* 
* [Brief CPP-file description]
*/ 

// Own header
#include "HelpersTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "exodbc/SqlStatementCloser.h"

// Debug
#include "DebugNew.h"

using namespace exodbc;

namespace exodbctest
{


	// Static consts
	// -------------

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------



	TEST_F(HelpersTest, GetInfo)
	{
		// Simply test reading a string and an int value works - that means, does return some data
		std::string serverName;
		DatabasePtr pDb = OpenTestDb();

		ASSERT_TRUE(pDb->HasConnectionHandle());
		GetInfo(pDb->GetSqlDbcHandle(), SQL_SERVER_NAME, serverName);
		EXPECT_FALSE(serverName.empty());

		// and read some int value
		// okay, it would be bad luck if one driver reports 1313
		SQLUSMALLINT maxStmts = 1313;
		SWORD cb = 0;
		GetInfo(pDb->GetSqlDbcHandle(), SQL_MAX_CONCURRENT_ACTIVITIES, &maxStmts, sizeof(maxStmts), &cb);
		EXPECT_NE(1313, maxStmts);
		EXPECT_NE(0, cb);
	}


	TEST_F(HelpersTest, GetData)
	{
		// Test by reading some char value

		// Allocate a valid statement handle from an open connection handle
		DatabasePtr pDb = OpenTestDb();
		SqlStmtHandlePtr pHStmt = std::make_shared<SqlStmtHandle>();
		ASSERT_NO_THROW(pHStmt->AllocateWithParent(pDb->GetSqlDbcHandle()));

		// Open statement by doing some operation on it
		std::string sqlstmt;
		if (pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			sqlstmt = boost::str(boost::format(u8"SELECT * FROM %s WHERE %s = 3") % GetTableName(TableId::CHARTYPES) % GetIdColumnName(TableId::CHARTYPES));
		}
		else
		{
			sqlstmt = boost::str(boost::format(u8"SELECT * FROM exodbc.%s WHERE %s = 3") % GetTableName(TableId::CHARTYPES) % GetIdColumnName(TableId::CHARTYPES));
		}
		// convert schema name to upper if needed
		if (g_odbcInfo.m_namesCase == Case::UPPER)
		{
			boost::algorithm::to_upper(sqlstmt);
		}
		SQLRETURN ret = SQLExecDirect(pHStmt->GetHandle(), (SQLAPICHARTYPE*) SQLAPICHARCONVERT(sqlstmt).c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));		
		ret = SQLFetch(pHStmt->GetHandle());
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		// Read some non-null string-data with enough chars
		bool isNull = false;
		std::string value;
		EXPECT_NO_THROW(GetData(pHStmt, 2, 20, value, &isNull));
		EXPECT_FALSE(isNull);
		EXPECT_EQ(u8"הצüאיט", value);
		EXPECT_NO_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::IgnoreNotOpen));

		// Trim the read-value in GetData
		ret = SQLExecDirect(pHStmt->GetHandle(), (SQLAPICHARTYPE*)SQLAPICHARCONVERT(sqlstmt).c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		ret = SQLFetch(pHStmt->GetHandle());
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		{
			// note that this will info about data truncation
			EXPECT_NO_THROW(GetData(pHStmt, 2, 3, value, &isNull));
		}
		EXPECT_EQ(u8"הצü", value);
		EXPECT_NO_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::IgnoreNotOpen));

		// Read some int value
		ret = SQLExecDirect(pHStmt->GetHandle(), (SQLAPICHARTYPE*)SQLAPICHARCONVERT(sqlstmt).c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		ret = SQLFetch(pHStmt->GetHandle());
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		SQLINTEGER id = 0;
		SQLLEN ind = 0;
		EXPECT_NO_THROW(GetData(pHStmt, 1, SQL_C_SLONG, &id, NULL, &ind, &isNull));
		EXPECT_EQ(3, id);
		EXPECT_NO_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::IgnoreNotOpen));

		// And test at least one null value
		ret = SQLExecDirect(pHStmt->GetHandle(), (SQLAPICHARTYPE*)SQLAPICHARCONVERT(sqlstmt).c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		ret = SQLFetch(pHStmt->GetHandle());
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		value = u8"";
		EXPECT_NO_THROW(GetData(pHStmt, 3, 20, value, &isNull));
		EXPECT_TRUE(isNull);
		EXPECT_EQ(u8"", value);
		
		EXPECT_NO_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::IgnoreNotOpen));
	}


	TEST_F(HelpersTest, SetDescriptionField)
	{
		// Test by setting the description of a numeric column parameter
		
		// Allocate a valid statement handle from an open connection handle
		DatabasePtr pDb = OpenTestDb();
		SqlStmtHandlePtr pHStmt = std::make_shared<SqlStmtHandle>();
		ASSERT_NO_THROW(pHStmt->AllocateWithParent(pDb->GetSqlDbcHandle()));

		// Open statement by doing some operation on it
		std::string sqlstmt;
		if (pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			sqlstmt = boost::str(boost::format(u8"SELECT * FROM %s WHERE %s = 3") % GetTableName(TableId::CHARTYPES) % GetIdColumnName(TableId::CHARTYPES));
		}
		else
		{
			sqlstmt = boost::str(boost::format(u8"SELECT * FROM exodbc.%s WHERE %s = 3") % GetTableName(TableId::CHARTYPES) % GetIdColumnName(TableId::CHARTYPES));
		}
		if (g_odbcInfo.m_namesCase == Case::UPPER)
		{
			boost::algorithm::to_upper(sqlstmt);
		}
		SQLRETURN ret = SQLExecDirect(pHStmt->GetHandle(), (SQLAPICHARTYPE*)SQLAPICHARCONVERT(sqlstmt).c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		// Except to fail when passing a null handle
		{
			EXPECT_THROW(SetDescriptionField(SQL_NULL_HSTMT, 3, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC), AssertionException);
		}

		// Get Descriptor for a param
		SqlDescHandle hDesc(pHStmt, RowDescriptorType::PARAM);
		//SQLHANDLE hDesc = SQL_NULL_HDESC;
		//EXPECT_NO_THROW(hDesc = GetRowDescriptorHandle(hStmt, RowDescriptorType::PARAM));

		if (pDb->GetDbms() != DatabaseProduct::ACCESS)
		{
			SQL_NUMERIC_STRUCT num;
			EXPECT_NO_THROW(SetDescriptionField(hDesc.GetHandle(), 3, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC));
			EXPECT_NO_THROW(SetDescriptionField(hDesc.GetHandle(), 3, SQL_DESC_PRECISION, (SQLPOINTER)18));
			EXPECT_NO_THROW(SetDescriptionField(hDesc.GetHandle(), 3, SQL_DESC_SCALE, (SQLPOINTER)10));
			EXPECT_NO_THROW(SetDescriptionField(hDesc.GetHandle(), 3, SQL_DESC_DATA_PTR, (SQLPOINTER)&num));
		}
	}


	// HelpersTest
	// =================
	TEST_F(HelpersTest, GetAllErrors)
	{
		// We except an assertion if not at least one handle is valid
		{
			EXPECT_THROW(GetAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, SQL_NULL_HSTMT, SQL_NULL_HDESC), AssertionException);
		}
	}


	TEST_F(HelpersTest, InitTime)
	{
		SQL_TIME_STRUCT time = InitTime(13, 14, 15);
		EXPECT_EQ(13, time.hour);
		EXPECT_EQ(14, time.minute);
		EXPECT_EQ(15, time.second);
	}


	TEST_F(HelpersTest, InitDate)
	{
		SQL_DATE_STRUCT date = InitDate(26, 1, 1983);
		EXPECT_EQ(26, date.day);
		EXPECT_EQ(1, date.month);
		EXPECT_EQ(1983, date.year);
	}


	TEST_F(HelpersTest, InitTimestamp)
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


	TEST_F(HelpersTest, InitNumeric)
	{
		SQLCHAR val[SQL_MAX_NUMERIC_LEN];
		FillMemory(val, SQL_MAX_NUMERIC_LEN, 1);
		SQL_NUMERIC_STRUCT num = InitNumeric(18, 10, 1, val);
		EXPECT_EQ(18, num.precision);
		EXPECT_EQ(10, num.scale);
		EXPECT_EQ((SQLCHAR)1, num.sign);
		EXPECT_TRUE(memcmp(val, num.val, SQL_MAX_NUMERIC_LEN) == 0);
	}


	TEST_F(HelpersTest, InitNullNumeric)
	{
		char nullMem[sizeof(SQL_NUMERIC_STRUCT)];
		ZeroMemory(nullMem, sizeof(SQL_NUMERIC_STRUCT));

		SQL_NUMERIC_STRUCT num = InitNullNumeric();
		EXPECT_TRUE(memcmp(&num, &nullMem, sizeof(SQL_NUMERIC_STRUCT)) == 0);
	}


	TEST_F(HelpersTest, IsTimeEqual)
	{
		SQL_TIME_STRUCT t1 = InitTime(13, 14, 15);
		SQL_TIME_STRUCT t2 = InitTime(07, 14, 27);
		SQL_TIME_STRUCT t3 = InitTime(13, 14, 15);
		EXPECT_TRUE(IsTimeEqual(t1, t3));
		EXPECT_FALSE(IsTimeEqual(t1, t2));
		EXPECT_FALSE(IsTimeEqual(t3, t2));

	}


	TEST_F(HelpersTest, IsDateEqual)
	{
		SQL_DATE_STRUCT d1 = InitDate(26, 01, 1983);
		SQL_DATE_STRUCT d2 = InitDate(25, 04, 1983);
		SQL_DATE_STRUCT d3 = InitDate(26, 01, 1983);
		EXPECT_TRUE(IsDateEqual(d1, d3));
		EXPECT_FALSE(IsDateEqual(d1, d2));
		EXPECT_FALSE(IsDateEqual(d3, d2));
	}


	TEST_F(HelpersTest, IsTimestampEqual)
	{
		SQL_TIMESTAMP_STRUCT ts1 = InitTimestamp(13, 14, 15, 123456789, 26, 1, 1983);
		SQL_TIMESTAMP_STRUCT ts2 = InitTimestamp(13, 14, 15, 12345, 26, 1, 1983);
		SQL_TIMESTAMP_STRUCT ts3 = InitTimestamp(13, 14, 15, 123456789, 26, 1, 1983);

		EXPECT_TRUE(IsTimestampEqual(ts1, ts3));
		EXPECT_FALSE(IsTimestampEqual(ts1, ts2));
		EXPECT_FALSE(IsTimestampEqual(ts3, ts2));
	}
} // namespace exodbctest