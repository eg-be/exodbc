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

	TEST_F(DbTest, OpenForwardOnly_MySql_3_51)
	{
		OpenTest(MYSQL_3_51_DSN, MYSQL_USER, MYSQL_PASS, true);
	}

	TEST_F(DbTest, OpenNotForwardOnly_MySql_3_51)
	{
		OpenTest(MYSQL_3_51_DSN, MYSQL_USER, MYSQL_PASS, false);
	}

	TEST_F(DbTest, OpenForwardOnly_DB2)
	{
		OpenTest(DB2_DSN, DB2_USER, DB2_PASS, true);
	}

	TEST_F(DbTest, OpenNotForwardOnly_DB2)
	{
		OpenTest(DB2_DSN, DB2_USER, DB2_PASS, false);
	}

	void DbTest::OpenTest(const std::wstring dsn, const std::wstring user, const std::wstring pass, bool forwardOnlyCursors)
	{
		wxDbConnectInf* pConnectInf = new wxDbConnectInf(NULL, dsn, user, pass);
		HENV henv = pConnectInf->GetHenv();
		if(!henv)
		{
			delete pConnectInf;
			ASSERT_TRUE(henv);
		}

		wxDb* pDb = new wxDb(henv, forwardOnlyCursors);

		// Open without failing on unsupported datatypes
		EXPECT_TRUE(pDb->Open(pConnectInf, false));
		pDb->Close();
		delete pDb;

		// Open with failing on unsupported datatypes
		pDb = new wxDb(henv, forwardOnlyCursors);
		EXPECT_TRUE(pDb->Open(pConnectInf, true));
		pDb->Close();

		delete pDb;
		delete pConnectInf;
	}

}