/*!
* \file HelpersTest.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 09.08.2014
* 
* [Brief CPP-file description]
*/ 

#include "stdafx.h"

// Own header
#include "HelpersTest.h"

// Same component headers
// Other headers
#include "Database.h"
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
	void HelpersTest::SetUp()
	{
		// Set up is called for every test
		m_odbcInfo = GetParam();
		RecordProperty("DSN", eli::w2mb(m_odbcInfo.m_dsn));
		RecordProperty("OdbcVersion", m_odbcInfo.m_odbcVersion);
	}

	void HelpersTest::TearDown()
	{
	}

	TEST_P(HelpersTest, AllocDbcHandle)
	{
		DbEnvironment env(m_odbcInfo.m_odbcVersion);
		ASSERT_TRUE(env.HaveHenv());

		SQLHANDLE hDbc = AllocDbcHandle(env.GetHenv());
		EXPECT_FALSE(SQL_NULL_HDBC == hDbc);

		FreeDbcHandle(hDbc);
	}

	TEST_P(HelpersTest, FreeDbcHandle)
	{
		DbEnvironment env(m_odbcInfo.m_odbcVersion);
		ASSERT_TRUE(env.HaveHenv());

		SQLHANDLE hDbc = AllocDbcHandle(env.GetHenv());
		ASSERT_FALSE(SQL_NULL_HDBC == hDbc);

		EXPECT_TRUE(FreeDbcHandle(hDbc));

	}
	// Interfaces
	// ----------

}