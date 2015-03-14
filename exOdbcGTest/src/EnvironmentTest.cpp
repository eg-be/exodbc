/*!
* \file EnvironmentTest.cpp
* \author Elias Gerber <egerber@gmx.net>
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


	TEST_P(EnvironmentTest, AllocHenv)
	{
		Environment env;
		ASSERT_NO_THROW(env.AllocHenv());
		// We will fail to allocate a second one
		// Suppress the output of the assertion helper
		LogLevelFatal lf;
		DontDebugBreak ddb;
		EXPECT_THROW(env.AllocHenv(), AssertionException);
	}


	TEST_P(EnvironmentTest, FreeHenv)
	{
		// First simply test alloc - free
		Environment env;
		ASSERT_NO_THROW(env.AllocHenv());
		EXPECT_NO_THROW(env.FreeHenv());
		
		// Now test that we fail to free if there is still a database around
		ASSERT_NO_THROW(env.AllocHenv());
		ASSERT_NO_THROW(env.SetOdbcVersion(OV_3));
		{
			Database db(env);
			EXPECT_THROW(env.FreeHenv(), SqlResultException);
		}

		// Once the database is gone, we can free the env
		EXPECT_NO_THROW(env.FreeHenv());
	}


	TEST_P(EnvironmentTest, SetConnectionString)
	{
		Environment env;
		EXPECT_FALSE(env.UseConnectionStr());

		env.SetConnectionStr(L"FooString");
		EXPECT_TRUE(env.UseConnectionStr());

		env.SetConnectionStr(L"");
		EXPECT_FALSE(env.UseConnectionStr());
	}


	TEST_P(EnvironmentTest, SetOdbcVersion)
	{
		// Test with setting it explict
		Environment env_v2;
		Environment env_v3;
		Environment env_v3_80;

		ASSERT_NO_THROW(env_v2.AllocHenv());
		ASSERT_NO_THROW(env_v3.AllocHenv());
		ASSERT_NO_THROW(env_v3_80.AllocHenv());

		EXPECT_NO_THROW(env_v2.SetOdbcVersion(OV_2));
		EXPECT_NO_THROW(env_v3.SetOdbcVersion(OV_3));
		EXPECT_NO_THROW(env_v3_80.SetOdbcVersion(OV_3_8));

		EXPECT_EQ(SQL_OV_ODBC2, env_v2.ReadOdbcVersion());
		EXPECT_EQ(SQL_OV_ODBC3, env_v3.ReadOdbcVersion());
		EXPECT_EQ(SQL_OV_ODBC3_80, env_v3_80.ReadOdbcVersion());
	}

	TEST_P(EnvironmentTest, ListDataSources)
	{
		Environment env(OV_3);
		ASSERT_TRUE(env.HasHenv());

		vector<SDataSource> dataSources;
		ASSERT_NO_THROW(dataSources = env.ListDataSources(Environment::All));

		// Expect that we find our DataSource in the list
		bool foundDataSource = false;
		vector<SDataSource>::const_iterator it;
		for(it = dataSources.begin(); it != dataSources.end(); it++)
		{
			wstring tmp = (*it).Dsn;
			if(tmp == m_odbcInfo.m_dsn)
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