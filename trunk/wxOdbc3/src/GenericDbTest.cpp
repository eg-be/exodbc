/*!
 * \file GenericDbTest.cpp
 * \author Elias Gerber <eg@zame.ch>
 * \date 22.09.2013
 * 
 * [Brief CPP-file description]
 */ 

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifndef DBG_NEW
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#define new DBG_NEW
#endif
#endif

// Own header
#include "GenericDbTest.h"

// Same component headers
#include "db.h"

// Other headers
#include "cppunit/config/SourcePrefix.h"

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// DbConnectInfTest
// ----------------
void DbConnectInfTest::testAllocHenv()
{
	// Allocating a Hevn will always work, no tests for users / pws are done atm
	wxDbConnectInf DbConnectInf;
	DbConnectInf.SetDsn(L"MyDSN");
	DbConnectInf.SetUserID(L"MyUserName");
	DbConnectInf.SetPassword(L"MyPassword");
	DbConnectInf.SetDefaultDir(L"");	
	CPPUNIT_ASSERT( DbConnectInf.AllocHenv() );
	CPPUNIT_ASSERT( DbConnectInf.GetHenv() != 0 );

	// Create using c'tor
	wxDbConnectInf DbConnectInf2(NULL, L"MyDSN", L"UserId", L"Pass");
	CPPUNIT_ASSERT( DbConnectInf.GetHenv() != 0 );
}
