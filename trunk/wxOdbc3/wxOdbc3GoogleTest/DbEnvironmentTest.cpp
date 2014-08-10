/*!
* \file DbEnvironmentTest.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 27.07.2014
* 
* [Brief CPP-file description]
*/ 

#include "stdafx.h"

// Own header
#include "DbEnvironmentTest.h"

// Same component headers
// Other headers
#include "DbEnvironment.h"

using namespace exodbc;

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

	void DbEnvironmentTest::SetUp()
	{
		m_odbcInfo = GetParam();
		RecordProperty("DSN", eli::w2mb(m_odbcInfo.m_dsn));
		RecordProperty("OdbcVersion", m_odbcInfo.m_odbcVersion);
	}

	void DbEnvironmentTest::TearDown()
	{

	}


	TEST_P(DbEnvironmentTest, AllocHenv)
	{
		DbEnvironment env;
		EXPECT_TRUE(env.AllocHenv());
	}


	TEST_P(DbEnvironmentTest, FreeHenv)
	{
		DbEnvironment env;
		ASSERT_TRUE(env.AllocHenv());
		EXPECT_TRUE(env.FreeHenv());
		
		// TODO: Test to test if we fail (we do so) if there are still open databases on that handle?
	}


	TEST_P(DbEnvironmentTest, SetConnectionString)
	{
		DbEnvironment env;
		EXPECT_FALSE(env.UseConnectionStr());

		env.SetConnectionStr(L"FooString");
		EXPECT_TRUE(env.UseConnectionStr());

		env.SetConnectionStr(L"");
		EXPECT_FALSE(env.UseConnectionStr());
	}


	TEST_P(DbEnvironmentTest, SetOdbcVersion)
	{
		// Test with setting it explict
		DbEnvironment env_v2;
		DbEnvironment env_v3;
		DbEnvironment env_v3_80;

		ASSERT_TRUE(env_v2.AllocHenv());
		ASSERT_TRUE(env_v3.AllocHenv());
		ASSERT_TRUE(env_v3_80.AllocHenv());

		EXPECT_TRUE(env_v2.SetOdbcVersion(OV_2));
		EXPECT_TRUE(env_v3.SetOdbcVersion(OV_3));
		EXPECT_TRUE(env_v3_80.SetOdbcVersion(OV_3_8));

		EXPECT_EQ(SQL_OV_ODBC2, env_v2.GetOdbcVersion());
		EXPECT_EQ(SQL_OV_ODBC3, env_v3.GetOdbcVersion());
		EXPECT_EQ(SQL_OV_ODBC3_80, env_v3_80.GetOdbcVersion());
	}

	TEST_P(DbEnvironmentTest, ListDataSources)
	{
		DbEnvironment env(OV_3);
		ASSERT_TRUE(env.HaveHenv());

		vector<SDataSource> dataSources = env.ListDataSources();

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
