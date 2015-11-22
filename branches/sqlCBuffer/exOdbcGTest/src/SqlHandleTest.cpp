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
	}

} // namespace exodbc