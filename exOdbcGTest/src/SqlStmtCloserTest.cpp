/*!
* \file SqlStmtCloserTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2014
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "SqlStmtCloserTest.h"
#include "exOdbcGTestHelpers.h"

// Same component headers
// Other headers


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

	// StatementCloserTest
	// ===================
	void StatementCloserTest::SetUp()
	{
		// Set up is called for every test
		m_odbcInfo = g_odbcInfo;

		ASSERT_NO_THROW(m_pEnv->Init(OdbcVersion::V_3));
		ASSERT_NO_THROW(m_pDb->Init(m_pEnv));
		if (m_odbcInfo.HasConnectionString())
		{
			ASSERT_NO_THROW(m_pDb->Open(m_odbcInfo.m_connectionString));
		}
		else
		{
			ASSERT_NO_THROW(m_pDb->Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
		}
	}


	TEST_F(StatementCloserTest, CloseStmtHandle)
	{
		SqlStmtHandlePtr pHStmt = std::make_shared<SqlStmtHandle>();
		{
			// We assert if we pass a null handle
			LogLevelFatal llf;
			EXPECT_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::IgnoreNotOpen), AssertionException);
		}

		// Allocate a valid statement handle from an open connection handle
		ASSERT_NO_THROW(pHStmt->AllocateWithParent(m_pDb->GetSqlDbcHandle()));

		// This statement is not open yet, we shall fail to free it
		{
			if (m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #120");
			}
			LogLevelFatal llf;
			EXPECT_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::ThrowIfNotOpen), SqlResultException);
		}
		// But not if we ignore the cursor-state
		EXPECT_NO_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::IgnoreNotOpen));

		// Open statement by doing some operation on it
		std::wstring sqlstmt;
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM %s") % GetTableName(TableId::INTEGERTYPES));
		}
		else
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM exodbc.%s") % GetTableName(TableId::INTEGERTYPES));
		}
		// convert schema name to upper if needed
		if (m_odbcInfo.m_namesCase == Case::UPPER)
		{
			boost::algorithm::to_upper(sqlstmt);
		}

		SQLRETURN ret = SQLExecDirect(pHStmt->GetHandle(), (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

		// Closing it first time must work
		EXPECT_NO_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::ThrowIfNotOpen));

		// Closing it a second time must fail
		if (m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
		{
			LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #120");
		}
		LogLevelFatal llf;
		EXPECT_THROW(StatementCloser::CloseStmtHandle(pHStmt, StatementCloser::Mode::ThrowIfNotOpen), SqlResultException);

		// Free the statement by letting going out of scope
	}

} // namespace exodbc