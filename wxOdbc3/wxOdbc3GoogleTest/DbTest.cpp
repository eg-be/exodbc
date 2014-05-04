#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif

#include "DbTest.h"
#include "MySqlParams.h"
#include "Db2Params.h"
#include "db.h"

using namespace std;

namespace wxOdbc3Test
{

	void DbTest::SetUp()
	{
		SOdbcInfo odbcInfo = GetParam();
		m_pConnectInf = new wxDbConnectInf(NULL, odbcInfo.m_dsn, odbcInfo.m_username, odbcInfo.m_password);
		m_forwardOnlyCursors = (odbcInfo.m_cursorType == SOdbcInfo::forwardOnlyCursors);
	}

	void DbTest::TearDown()
	{
		delete m_pConnectInf;
		m_pConnectInf = NULL;
	}


	TEST_P(DbTest, OpenTest)
	{
		HENV henv = m_pConnectInf->GetHenv();
		ASSERT_TRUE(henv  != 0);

		wxDb* pDb = new wxDb(henv, m_forwardOnlyCursors);

		// Open without failing on unsupported datatypes
		EXPECT_TRUE(pDb->Open(m_pConnectInf, false));
		pDb->Close();
		delete pDb;

		// Open with failing on unsupported datatypes
		pDb = new wxDb(henv, m_forwardOnlyCursors);
		EXPECT_TRUE(pDb->Open(m_pConnectInf, true));
		pDb->Close();
		delete pDb;
	}

}