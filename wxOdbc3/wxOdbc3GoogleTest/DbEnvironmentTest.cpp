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
		// Test with setting it explict
		DbEnvironment* pEnv_v2 = new DbEnvironment();
		DbEnvironment* pEnv_v3 = new DbEnvironment();
		DbEnvironment* pEnv_v3_80 = new DbEnvironment();
		
		EXPECT_TRUE(pEnv_v2->AllocHenv());
		EXPECT_TRUE(pEnv_v3->AllocHenv());
		EXPECT_TRUE(pEnv_v3_80->AllocHenv());

		EXPECT_TRUE(pEnv_v2->SetOdbcVersion(OV_2));
		EXPECT_TRUE(pEnv_v3->SetOdbcVersion(OV_3));
		EXPECT_TRUE(pEnv_v3_80->SetOdbcVersion(OV_3_8));

		EXPECT_EQ(SQL_OV_ODBC2, pEnv_v2->GetOdbcVersion());
		EXPECT_EQ(SQL_OV_ODBC3, pEnv_v3->GetOdbcVersion());
		EXPECT_EQ(SQL_OV_ODBC3_80, pEnv_v3_80->GetOdbcVersion());

		delete pEnv_v2;
		delete pEnv_v3;
		delete pEnv_v3_80;
	}


	TEST_P(DbEnvironmentTest, FreeHenv)
	{
		DbEnvironment* pEnv_v2 = new DbEnvironment();
		DbEnvironment* pEnv_v3 = new DbEnvironment();
		DbEnvironment* pEnv_v3_80 = new DbEnvironment();

		EXPECT_TRUE(pEnv_v2->AllocHenv());
		EXPECT_TRUE(pEnv_v3->AllocHenv());
		EXPECT_TRUE(pEnv_v3_80->AllocHenv());

		pEnv_v2->SetOdbcVersion(OV_2);
		pEnv_v3->SetOdbcVersion(OV_3);
		pEnv_v3_80->SetOdbcVersion(OV_3_8);

		EXPECT_TRUE(pEnv_v2->FreeHenv());
		EXPECT_TRUE(pEnv_v3->FreeHenv());
		EXPECT_TRUE(pEnv_v3_80->FreeHenv());
		
		// TODO: Test to test if we fail (we do so) if there are still open databases on that handle?

		delete pEnv_v2;
		delete pEnv_v3;
		delete pEnv_v3_80;
	}

	TEST_P(DbEnvironmentTest, ListDataSources)
	{
		DbEnvironment* pEnv = new DbEnvironment(OV_3);
		bool haveEnvHandle = pEnv->HaveHenv();
		if(!haveEnvHandle)
		{
			delete pEnv;
			ASSERT_TRUE(haveEnvHandle);
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