/*!
* \file SqlHandleTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2014
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "SqlHandleTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "SqlHandle.h"
#include "Environment.h"
#include "Database.h"

// Debug
#include "DebugNew.h"

using namespace exodbctest;

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
	TEST_F(SqlHandleTest, AllocEnv)
	{
		// Simply allocate an env handle
		SqlEnvHandle henv;
		EXPECT_NO_THROW(henv.Allocate());

		// But fail allocating twice
		EXPECT_THROW(henv.Allocate(), AssertionException);

		// But not if we free
		EXPECT_NO_THROW(henv.Free());
		EXPECT_NO_THROW(henv.Allocate());

		// Fail to allocate with parent -- oops, fails to compile
	}


	TEST_F(SqlHandleTest, AllocDbc)
	{
		// Use the Environment
		exodbc::Environment env(OdbcVersion::V_3);

		// Simply allocate a dbc handle
		SqlDbcHandle hDbc;
		EXPECT_NO_THROW(hDbc.AllocateWithParent(env.GetSqlEnvHandle()));
		// But fail allocating twice
		EXPECT_THROW(hDbc.Allocate(), AssertionException);

		// But not if we free
		EXPECT_NO_THROW(hDbc.Free());
		EXPECT_NO_THROW(hDbc.AllocateWithParent(env.GetSqlEnvHandle()));

		// Fail without parent
		SqlDbcHandle hDbc2;
		EXPECT_THROW(hDbc2.Allocate(), AssertionException);

		// Construct one using the parent constructor
		EXPECT_NO_THROW(SqlDbcHandle hDbc2(env.GetSqlEnvHandle()));
	}


	TEST_F(SqlHandleTest, AllocStmt)
	{
		// Use a Database
		DatabasePtr pDb = OpenTestDb(OdbcVersion::V_3);

		// Simply allocate a stmt handle
		SqlStmtHandle hStmt;
		EXPECT_NO_THROW(hStmt.AllocateWithParent(pDb->GetSqlDbcHandle()));
		// But fail allocating twice
		EXPECT_THROW(hStmt.Allocate(), AssertionException);

		// But not if we free
		EXPECT_NO_THROW(hStmt.Free());
		EXPECT_NO_THROW(hStmt.AllocateWithParent(pDb->GetSqlDbcHandle()));

		// Fail without parent
		SqlStmtHandle hStmt2;
		EXPECT_THROW(hStmt2.Allocate(), AssertionException);

		// Construct one using the parent constructor
		EXPECT_NO_THROW(SqlStmtHandle hStmt2(pDb->GetSqlDbcHandle()));
	}


	TEST_F(SqlHandleTest, AllocDesc)
	{
		// Use a Database and create a stmt handle from there
		DatabasePtr pDb = OpenTestDb(OdbcVersion::V_3);
		SqlStmtHandlePtr pHStmt = std::make_shared<SqlStmtHandle>();
		ASSERT_NO_THROW(pHStmt->AllocateWithParent(pDb->GetSqlDbcHandle()));

		// Open statement by doing some operation on it
		std::wstring sqlstmt;
		if (pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM %s WHERE %s = 3") % test::GetTableName(test::TableId::CHARTYPES, g_odbcInfo.m_namesCase) % test::GetIdColumnName(test::TableId::CHARTYPES, g_odbcInfo.m_namesCase));
		}
		else
		{
			sqlstmt = boost::str(boost::wformat(L"SELECT * FROM exodbc.%s WHERE %s = 3") % test::GetTableName(test::TableId::CHARTYPES, g_odbcInfo.m_namesCase) % test::GetIdColumnName(test::TableId::CHARTYPES, g_odbcInfo.m_namesCase));
		}
		if (g_odbcInfo.m_namesCase == test::Case::UPPER)
		{
			boost::algorithm::to_upper(sqlstmt);
		}
		SQLRETURN ret = SQLExecDirect(pHStmt->GetHandle(), (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		ASSERT_TRUE(SQL_SUCCEEDED(ret));

		// now test by creating a row descriptor:
		SqlDescHandle hDesc(pHStmt, RowDescriptorType::ROW);
	}


	TEST_F(SqlHandleTest, FreedHandleSignal)
	{
		SqlEnvHandle hEnv;
		hEnv.Allocate();
		bool signalCalled = false;
		hEnv.ConnectFreedSignal([&](const SqlEnvHandle& h) -> void
		{
			signalCalled = true;
		});
		
		hEnv.Free();
		EXPECT_TRUE(signalCalled);

		// If it goes out of scope it should be called to
		bool signalCalled2 = false;
		{
			SqlEnvHandle env2;
			env2.Allocate();
			env2.ConnectFreedSignal([&](const SqlEnvHandle& h) -> void
			{
				signalCalled2 = true;
			});
			// do not free, just let it go out of scope
		}
		EXPECT_TRUE(signalCalled2);
	}

} // namespace exodbc