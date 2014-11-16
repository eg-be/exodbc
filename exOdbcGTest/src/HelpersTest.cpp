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
#include "Environment.h"

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
	void HelpersTest::SetUp()
	{
		// Set up is called for every test
		m_odbcInfo = GetParam();
//		RecordProperty("DSN", eli::w2mb(m_odbcInfo.m_dsn));
	}

	void HelpersTest::TearDown()
	{
	}

	//TEST_P(HelpersTest, AllocDbcHandle)
	//{
	//	Environment env(m_odbcInfo.m_odbcVersion);
	//	ASSERT_TRUE(env.HaveHenv());

	//	SQLHANDLE hDbc = AllocDbcHandle(env.GetHenv());
	//	EXPECT_FALSE(SQL_NULL_HDBC == hDbc);

	//	// Allocating from a hDbc handle should not work
	//	BOOST_LOG_TRIVIAL(warning) << L"This test is supposed to spit warnings"; 
	//	SQLHANDLE hCopy = hDbc;
	//	SQLHANDLE hFail = AllocDbcHandle(hCopy);
	//	EXPECT_EQ(SQL_NULL_HDBC, hFail);

	//	FreeDbcHandle(hDbc);
	//}

	//TEST_P(HelpersTest, FreeDbcHandle)
	//{
	//	Environment env(m_odbcInfo.m_odbcVersion);
	//	ASSERT_TRUE(env.HaveHenv());

	//	SQLHANDLE hDbc = AllocDbcHandle(env.GetHenv());
	//	ASSERT_FALSE(SQL_NULL_HDBC == hDbc);

	//	// freeing twice should not work
	//	BOOST_LOG_TRIVIAL(warning) << L"This test is supposed to spit warnings";
	//	SQLHANDLE hCopy = hDbc;
	//	EXPECT_TRUE(FreeDbcHandle(hDbc));
	//	
	//	EXPECT_FALSE(FreeDbcHandle(hCopy));
	//}
	// Interfaces
	// ----------

} // namespace exodbc