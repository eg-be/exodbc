/*!
* \file GetDataWrapperTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 04.05.2017
* \copyright GNU Lesser General Public License Version 3
* 
* [Brief CPP-file description]
*/ 

// Own header
#include "GetDataWrapperTest.h"

// Same component headers
#include "exOdbcTestHelpers.h"

// Other headers
#include "exodbc/GetDataWrapper.h"
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
	TEST_F(GetDataWrapperTest, GetData)
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
		SQLRETURN ret = SQLExecDirect(pHStmt->GetHandle(), (SQLAPICHARTYPE*) EXODBCSTR_TO_SQLAPISTR(sqlstmt).c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));		
		ret = SQLFetch(pHStmt->GetHandle());
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		// Read some non-null string-data with enough chars
		bool isNull = false;
		std::string value;
		EXPECT_NO_THROW(GetDataWrapper::GetData(pHStmt, 2, 20, value, &isNull));
		EXPECT_FALSE(isNull);
		EXPECT_EQ(u8"abcdef", value);
		EXPECT_NO_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::IgnoreNotOpen));

		// Trim the read-value in GetData. 
		ret = SQLExecDirect(pHStmt->GetHandle(), (SQLAPICHARTYPE*)EXODBCSTR_TO_SQLAPISTR(sqlstmt).c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		ret = SQLFetch(pHStmt->GetHandle());
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		{
			// note that this will info about data truncation
            // note that if we set a size of 4 chars to read, this does not mean
            // that we can read 4 logical chars, as  one char might use multiple bytes
            // so we only test for starts_with.
			EXPECT_NO_THROW(GetDataWrapper::GetData(pHStmt, 2, 4, value, &isNull));
		}
		EXPECT_TRUE( boost::algorithm::starts_with(value, u8"abcd"));
		EXPECT_NO_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::IgnoreNotOpen));

		// Read some int value
		ret = SQLExecDirect(pHStmt->GetHandle(), (SQLAPICHARTYPE*)EXODBCSTR_TO_SQLAPISTR(sqlstmt).c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		ret = SQLFetch(pHStmt->GetHandle());
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		SQLINTEGER id = 0;
		SQLLEN ind = 0;
		EXPECT_NO_THROW(GetDataWrapper::GetData(pHStmt, 1, SQL_C_SLONG, &id, 0, &ind, &isNull));
		EXPECT_EQ(3, id);
		EXPECT_NO_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::IgnoreNotOpen));

		// And test at least one null value
		ret = SQLExecDirect(pHStmt->GetHandle(), (SQLAPICHARTYPE*)EXODBCSTR_TO_SQLAPISTR(sqlstmt).c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		ret = SQLFetch(pHStmt->GetHandle());
		EXPECT_TRUE(SQL_SUCCEEDED(ret));
		value = u8"";
		EXPECT_NO_THROW(GetDataWrapper::GetData(pHStmt, 3, 20, value, &isNull));
		EXPECT_TRUE(isNull);
		EXPECT_EQ(u8"", value);
		
		EXPECT_NO_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::IgnoreNotOpen));
	}



} // namespace exodbctest
