/*!
* \file SqlHandleTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2014
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

// Own header
#include "SqlHandleTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "exodbc/SqlHandle.h"
#include "exodbc/Environment.h"
#include "exodbc/Database.h"

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
		ASSERT_TRUE(SQL_SUCCEEDED(ret));

		// now test by creating a row descriptor:
		SqlDescHandle hDesc(pHStmt, RowDescriptorType::ROW);
	}


	TEST_F(SqlHandleTest, FreeHandleSignal)
	{
		SqlEnvHandle hEnv;
		hEnv.Allocate();
		bool signalCalled = false;
		hEnv.ConnectFreeSignal([&](const SqlEnvHandle& h) -> void
		{
			// The handle must still be the same and allocated
			EXPECT_EQ(hEnv, h);
			EXPECT_TRUE(h.IsAllocated());
			signalCalled = true;
		});

		hEnv.Free();
		EXPECT_TRUE(signalCalled);

		// If it goes out of scope it should be called to
		bool signalCalled2 = false;
		{
			SqlEnvHandle env2;
			env2.Allocate();
			env2.ConnectFreeSignal([&](const SqlEnvHandle& h) -> void
			{
				// The handle must be the same and allocated
				EXPECT_EQ(env2, h);
				EXPECT_TRUE(h.IsAllocated());
				signalCalled2 = true;
			});
			// do not free, just let it go out of scope
		}
		EXPECT_TRUE(signalCalled2);
	}

} // namespace exodbctest