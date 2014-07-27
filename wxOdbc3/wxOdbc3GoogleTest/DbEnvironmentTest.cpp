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

namespace exOdbcTest
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
	}

	void DbEnvironmentTest::TearDown()
	{

	}

	TEST_P(DbEnvironmentTest, ListDataSources)
	{
		DbEnvironment* pEnv = new DbEnvironment();
		bool allocatedHenv = pEnv->AllocHenv();
		if(!allocatedHenv)
		{
			delete pEnv;
			ASSERT_TRUE(allocatedHenv);
		}

		vector<SDataSource> dataSources = pEnv->ListDataSources();

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

		delete pEnv;
	}
	// Interfaces
	// ----------

}