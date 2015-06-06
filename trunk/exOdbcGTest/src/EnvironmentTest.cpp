/*!
* \file EnvironmentTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 27.07.2014
* \copyright wxWindows Library Licence, Version 3.1
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
		m_odbcInfo = GetParam();
//		RecordProperty("DSN", eli::w2mb(m_odbcInfo.m_dsn));
	}

	void EnvironmentTest::TearDown()
	{

	}


	TEST_P(EnvironmentTest, AllocateHenv)
	{
		Environment env;
		ASSERT_NO_THROW(env.AllocateEnvironmentHandle());
		// We will fail to allocate a second one
		// Suppress the output of the assertion helper
		LogLevelFatal lf;
		DontDebugBreak ddb;
		EXPECT_THROW(env.AllocateEnvironmentHandle(), AssertionException);
	}


	TEST_P(EnvironmentTest, FreeEnvironmentHandle)
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


	TEST_P(EnvironmentTest, SetOdbcVersion)
	{
		// Test with setting it explict
		Environment env_v2;
		Environment env_v3;
		Environment env_v3_80;

		ASSERT_NO_THROW(env_v2.AllocateEnvironmentHandle());
		ASSERT_NO_THROW(env_v3.AllocateEnvironmentHandle());
		ASSERT_NO_THROW(env_v3_80.AllocateEnvironmentHandle());

		EXPECT_NO_THROW(env_v2.SetOdbcVersion(OdbcVersion::V_2));
		EXPECT_NO_THROW(env_v3.SetOdbcVersion(OdbcVersion::V_3));
		EXPECT_NO_THROW(env_v3_80.SetOdbcVersion(OdbcVersion::V_3_8));

		EXPECT_EQ(SQL_OV_ODBC2, (int) env_v2.ReadOdbcVersion());
		EXPECT_EQ(SQL_OV_ODBC3, (int) env_v3.ReadOdbcVersion());
		EXPECT_EQ(SQL_OV_ODBC3_80, (int) env_v3_80.ReadOdbcVersion());
	}

	TEST_P(EnvironmentTest, ListDataSources)
	{
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
