/*!
* \file HelpersTest.cpp
* \author Elias Gerber <egerber@gmx.net>
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
	void ParamHelpersTest::SetUp()
	{
		// Set up is called for every test
		m_odbcInfo = GetParam();
		ASSERT_NO_THROW(m_env.AllocateEnvironmentHandle());
		ASSERT_NO_THROW(m_env.SetOdbcVersion(OV_3));

		ASSERT_NO_THROW(m_db.AllocateConnectionHandle(m_env));
		ASSERT_NO_THROW(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
	}

	void ParamHelpersTest::TearDown()
	{

	}


	TEST_P(ParamHelpersTest, CloseStmtHandle)
	{
		SQLHANDLE hNull = SQL_NULL_HSTMT;
		{
			// We assert if we pass a null handle
			DontDebugBreak ddb;
			LogLevelFatal llf;
			EXPECT_THROW(CloseStmtHandle(hNull, StmtCloseMode::IgnoreNotOpen), AssertionException);
		}

		// Allocate a valid statement handle
		SQLHSTMT hStmt = SQL_NULL_HSTMT;
		EXPECT_NO_THROW(hStmt = AllocateStatement(m_db.GetConnectionHandle()));

		// This is not open yet, we shall fail to free it
		{
			if (m_db.Dbms() == dbmsMY_SQL)
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
		std::wstring sqlstmt = boost::str(boost::wformat(L"SELECT * FROM %s") % TestTables::GetTableName(TestTables::Table::INTEGERTYPES, m_odbcInfo.m_namesCase));
		SQLRETURN ret = SQLExecDirect(hStmt, (SQLWCHAR*) sqlstmt.c_str(), SQL_NTS);
		ASSERT_TRUE(SQL_SUCCEEDED(ret));

		// Closing it first time must work
		EXPECT_NO_THROW(CloseStmtHandle(hStmt, StmtCloseMode::ThrowIfNotOpen));

		// Closing it a second time must fail
		if (m_db.Dbms() == dbmsMY_SQL)
		{
			LOG_WARNING(L"This test is known to fail with MySQL, see Ticket #120");
		}
		DontDebugBreak ddb;
		LogLevelFatal llf;
		EXPECT_THROW(CloseStmtHandle(hStmt, StmtCloseMode::ThrowIfNotOpen), SqlResultException);
	}


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


	TEST_F(StaticHelpersTest, TestDontDebugBreak)
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

	// Interfaces
	// ----------

} // namespace exodbc