/*!
 * \file DbTableTest.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "TableTest.h"

// Same component headers
#include "exOdbcGTest.h"
#include "GenericTestTables.h"

// Other headers
#include "Environment.h"
#include "Database.h"

// Debug
#include "DebugNew.h"

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
using namespace std;
//using namespace exodbc;

namespace exodbc
{

	void TableTest::SetUp()
	{
		// Called for every unit-test
		m_odbcInfo = GetParam();
		
		// Set up Env
		ASSERT_TRUE(m_env.AllocHenv());
		ASSERT_TRUE(m_env.SetOdbcVersion(OV_3));

		// And database
		ASSERT_TRUE(m_db.AllocateHdbc(m_env));
		ASSERT_TRUE(m_db.Open(m_odbcInfo.m_dsn, m_odbcInfo.m_username, m_odbcInfo.m_password));
	}

	void TableTest::TearDown()
	{
		if (m_db.IsOpen())
		{
			// Microsoft Sql Server needs a CommitTrans()
			if (m_db.Dbms() == dbmsMS_SQL_SERVER)
			{
				EXPECT_TRUE(m_db.CommitTrans());
			}
			EXPECT_TRUE(m_db.Close());
		}
	}

	// Open
	// ----
	TEST_P(TableTest, Open)
	{

	}

	// GetNext
	// -------

	// Read
	// ----


// Interfaces
// ----------

} // namespace exodbc
