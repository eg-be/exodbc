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

	TEST_P(DbEnvironmentTest, SetOdbcVersion)
	{
		DbEnvironment* pEnv_v2 = new DbEnvironment();
		DbEnvironment* pEnv_v3 = new DbEnvironment();
		DbEnvironment* pEnv_v3_80 = new DbEnvironment();
		DbEnvironment* pEnv_vInvalid = new DbEnvironment();
		
		pEnv_v2->SetOdbcVersion(SQL_OV_ODBC2);
		pEnv_v3->SetOdbcVersion(SQL_OV_ODBC3);
		pEnv_v3_80->SetOdbcVersion(SQL_OV_ODBC3_80);

		EXPECT_TRUE(pEnv_v2->AllocHenv());
		EXPECT_TRUE(pEnv_v3->AllocHenv());
		EXPECT_TRUE(pEnv_v3_80->AllocHenv());

		EXPECT_FALSE(pEnv_vInvalid->SetOdbcVersion(123445));

		delete pEnv_v2;
		delete pEnv_v3;
		delete pEnv_v3_80;
		delete pEnv_vInvalid;

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