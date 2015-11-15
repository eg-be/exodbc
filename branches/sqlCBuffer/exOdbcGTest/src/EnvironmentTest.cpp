/*!
* \file EnvironmentTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 27.07.2014
* \copyright GNU Lesser General Public License Version 3
* 
* [Brief CPP-file description]
*/ 

#include "stdafx.h"

// Own header
#include "EnvironmentTest.h"

// Same component headers
// Other headers
#include "Environment.h"
#include "Database.h"
#include "Exception.h"

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
	using namespace std;
	using namespace exodbc;

	void EnvironmentTest::SetUp()
	{
		ASSERT_TRUE(g_odbcInfo.IsUsable());
		m_odbcInfo = g_odbcInfo;
	}

	void EnvironmentTest::TearDown()
	{

	}


	TEST_F(EnvironmentTest, AllocateEnvironmentHandle)
	{
		Environment env;
		ASSERT_NO_THROW(env.AllocateEnvironmentHandle());
		// We will fail to allocate a second one
		// Suppress the output of the assertion helper
		LogLevelFatal lf;
		DontDebugBreak ddb;
		EXPECT_THROW(env.AllocateEnvironmentHandle(), AssertionException);
	}


	TEST_F(EnvironmentTest, FreeEnvironmentHandle)
	{
		// First simply test alloc - free
		Environment env;
		ASSERT_NO_THROW(env.AllocateEnvironmentHandle());
		EXPECT_NO_THROW(env.FreeEnvironmentHandle());
		
		// Now test that we fail to free if there is still a database around
		ASSERT_NO_THROW(env.AllocateEnvironmentHandle());
		ASSERT_NO_THROW(env.SetOdbcVersion(OdbcVersion::V_3));
		{
			Database db(&env);
			EXPECT_THROW(env.FreeEnvironmentHandle(), SqlResultException);
		}

		// Once the database is gone, we can free the env
		EXPECT_NO_THROW(env.FreeEnvironmentHandle());
	}


	TEST_F(EnvironmentTest, CopyConstructor)
	{
		Environment env(OdbcVersion::V_3);
		ASSERT_TRUE(env.HasEnvironmentHandle());
		ASSERT_EQ(OdbcVersion::V_3, env.GetOdbcVersion());
		ASSERT_TRUE(env.GetEnvironmentHandle() != SQL_NULL_HENV);

		Environment copy(env);
		EXPECT_TRUE(copy.HasEnvironmentHandle());
		EXPECT_EQ(OdbcVersion::V_3, copy.GetOdbcVersion());
		EXPECT_NE(env.GetEnvironmentHandle(), copy.GetEnvironmentHandle());

		Environment e2;
		Environment c2(e2);
		EXPECT_FALSE(c2.HasEnvironmentHandle());
		EXPECT_EQ(OdbcVersion::UNKNOWN, c2.GetOdbcVersion());
	}


	TEST_F(EnvironmentTest, SetOdbcVersion)
	{
		// Test with setting it explict
		Environment env_v2;
		Environment env_v3;
		Environment env_v3_80;

		EXPECT_NO_THROW(env_v2.Init(OdbcVersion::V_2));
		EXPECT_NO_THROW(env_v3.Init(OdbcVersion::V_3));
		EXPECT_NO_THROW(env_v3_80.Init(OdbcVersion::V_3_8));

		EXPECT_EQ(SQL_OV_ODBC2, (int) env_v2.ReadOdbcVersion());
		EXPECT_EQ(SQL_OV_ODBC3, (int) env_v3.ReadOdbcVersion());
		EXPECT_EQ(SQL_OV_ODBC3_80, (int) env_v3_80.ReadOdbcVersion());
	}


	TEST_F(EnvironmentTest, ListDataSources)
	{
		if (m_odbcInfo.HasConnectionString())
		{
			LOG_WARNING(L"Skipping Test ListDataSources, because Test is implemented to stupid it only works with a named DSN");
			return;
		}

		Environment env(OdbcVersion::V_3);
		ASSERT_TRUE(env.HasEnvironmentHandle());

		vector<SDataSource> dataSources;
		ASSERT_NO_THROW(dataSources = env.ListDataSources(Environment::ListMode::All));

		// Expect that we find our DataSource in the list
		bool foundDataSource = false;
		vector<SDataSource>::const_iterator it;
		for(it = dataSources.begin(); it != dataSources.end(); it++)
		{
			if(it->m_dsn == m_odbcInfo.m_dsn)
			{
				foundDataSource = true;
				break;
			}
		}

		EXPECT_TRUE(foundDataSource);

	}
	// Interfaces
	// ----------

} // namespace exodbc
