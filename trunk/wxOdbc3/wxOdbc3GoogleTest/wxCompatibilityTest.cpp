/*!
* \file wxCompatibilityTest.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 31.08.2014
* 
* [Brief CPP-file description]
*/ 

#include "stdafx.h"

// Own header
#include "wxCompatibilityTest.h"

// Same component headers
// Other headers
#include "Database.h"

namespace exodbc
{
	//Static consts
	// -------------
	
	//Construction
	// -------------
	void wxCompatibilityTest::SetUp()
	{
		// Run for every test
		m_pDb = NULL;
		m_odbcInfo = GetParam();
		RecordProperty("DSN", eli::w2mb(m_odbcInfo.m_dsn));

		// Set up environment
		m_env.SetDsn(m_odbcInfo.m_dsn);
		m_env.SetUserID(m_odbcInfo.m_username);
		m_env.SetPassword(m_odbcInfo.m_password);
		ASSERT_TRUE(m_env.AllocHenv());
		ASSERT_TRUE(m_env.SetOdbcVersion(OV_3));

		// And the db
		m_pDb = new Database(&m_env);
		ASSERT_TRUE(m_pDb->Open(&m_env));
	}
	
	//Destructor
	// -----------
	void wxCompatibilityTest::TearDown()
	{
		// Run after every test
		if(m_pDb)
		{
			if(m_pDb->IsOpen())
			{
				if(m_pDb->Dbms() == dbmsMS_SQL_SERVER)
				{
					// So far only ms needs this? we need to fix this. has something to do with autocommit set to no, so also a
					// a select starts a transaction that must be closed or so.
					EXPECT_TRUE(m_pDb->CommitTrans());
				}
				EXPECT_TRUE(m_pDb->Close());
			}
			delete m_pDb;
		}
	}
	
	//Implementation
	// --------------


	TEST_P(wxCompatibilityTest, FooTest)
	{
		int p = 3;
	}

	//Interfaces
	// ----------
	// 

}