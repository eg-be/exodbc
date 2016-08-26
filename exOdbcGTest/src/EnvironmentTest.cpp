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
#include "exOdbcGTestHelpers.h"

// Other headers
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/Exception.h"
#include "exodbc/LogHandler.h"

// Debug
#include "DebugNew.h"

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
	using namespace std;
	using namespace exodbc;

	void EnvironmentTest::SetUp()
	{
		ASSERT_TRUE(g_odbcInfo.IsUsable());
	}

	void EnvironmentTest::TearDown()
	{

	}


	TEST_F(EnvironmentTest, CopyConstructor)
	{
		Environment env(OdbcVersion::V_3);
		ASSERT_TRUE(env.IsEnvHandleAllocated());
		ASSERT_EQ(OdbcVersion::V_3, env.GetOdbcVersion());
		ASSERT_TRUE(env.GetSqlEnvHandle() != SQL_NULL_HENV);

		Environment copy(env);
		EXPECT_TRUE(copy.IsEnvHandleAllocated());
		EXPECT_EQ(OdbcVersion::V_3, copy.GetOdbcVersion());
		EXPECT_NE(env.GetSqlEnvHandle(), copy.GetSqlEnvHandle());

		Environment e2;
		Environment c2(e2);
		EXPECT_FALSE(c2.IsEnvHandleAllocated());
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
		if (g_odbcInfo.HasConnectionString())
		{
			LOG_WARNING(L"Skipping Test ListDataSources, because Test is implemented to stupid it only works with a named DSN");
			return;
		}

		Environment env(OdbcVersion::V_3);
		ASSERT_TRUE(env.IsEnvHandleAllocated());

		vector<SDataSource> dataSources;
		ASSERT_NO_THROW(dataSources = env.ListDataSources(Environment::ListMode::All));

		// Expect that we find our DataSource in the list
		bool foundDataSource = false;
		vector<SDataSource>::const_iterator it;
		for(it = dataSources.begin(); it != dataSources.end(); it++)
		{
			if(it->m_dsn == g_odbcInfo.m_dsn)
			{
				foundDataSource = true;
				break;
			}
		}

		EXPECT_TRUE(foundDataSource);
	}


	TEST_F(EnvironmentTest, EnableConnectionPooling)
	{
		// Enable and disable pooling
		EXPECT_NO_THROW(Environment::EnableConnectionPooling(ConnectionPooling::OFF));
		EXPECT_NO_THROW(Environment::EnableConnectionPooling(ConnectionPooling::PER_DRIVER));
		EXPECT_NO_THROW(Environment::EnableConnectionPooling(ConnectionPooling::PER_HENV));

		// Test if open and closing a few connections is faster if pooling is enabled
		LogManager::Get().ClearLogHandlers();
		NullLogHandlerPtr pNullLogger = std::make_shared<NullLogHandler>();
		LogManager::Get().RegisterLogHandler(pNullLogger);
		size_t nrRuns = 10;
		time_t start, end, deltaDriver, deltaEnv, deltaOff;
		{
			Environment::EnableConnectionPooling(ConnectionPooling::PER_DRIVER);
			EnvironmentPtr pEnv = Environment::Create(OdbcVersion::V_3);
			time(&start);
			for (size_t i = 0; i < nrRuns; ++i)
			{
				DatabasePtr pDb = OpenTestDb(pEnv);
			}
			time(&end);
			deltaDriver = end - start;
		}

		{
			Environment::EnableConnectionPooling(ConnectionPooling::PER_HENV);
			EnvironmentPtr pEnv = Environment::Create(OdbcVersion::V_3);
			time(&start);
			for (size_t i = 0; i < nrRuns; ++i)
			{
				DatabasePtr pDb = OpenTestDb(pEnv);
			}
			time(&end);
			deltaEnv = end - start;
		}

		{
			Environment::EnableConnectionPooling(ConnectionPooling::OFF);
			EnvironmentPtr pEnv = Environment::Create(OdbcVersion::V_3);
			time(&start);
			for (size_t i = 0; i < nrRuns; ++i)
			{
				DatabasePtr pDb = OpenTestDb(pEnv);
			}
			time(&end);
			deltaOff = end - start;
		}

		StdErrLogHandlerPtr stdLogger = std::make_shared<StdErrLogHandler>();
		LogManager::Get().ClearLogHandlers();
		LogManager::Get().RegisterLogHandler(stdLogger);
		wstring msg = boost::str(boost::wformat(L"Opening %d connections using PER_DRIVER took %d seconds") % nrRuns % deltaDriver);
		LOG_INFO(msg);
		msg = boost::str(boost::wformat(L"Opening %d connections using PER_HENV took %d seconds") % nrRuns % deltaEnv);
		LOG_INFO(msg);
		msg = boost::str(boost::wformat(L"Opening %d connections using OFF took %d seconds") % nrRuns % deltaOff);
		LOG_INFO(msg);
	}

} // namespace exodbctest
