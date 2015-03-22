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
		EXPECT_TRUE(SQL_SUCCEEDED(ret));

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

	}


	TEST_P(ParamHelpersTest, SetDescriptionField)
	{

	}


	TEST_P(ParamHelpersTest, GetRowDescriptorHandle)
	{

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