/*!
 * \file DbTest.cpp
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
#include "DbTest.h"

// Same component headers
// Other headers

#include "cppunit/config/SourcePrefix.h"

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
void DbTest::setUp()
{

}


void DbTest::tearDown()
{

}

void DbTest::testFail()
{
	CPPUNIT_ASSERT( false );
}

void DbTest::testOk()
{
	CPPUNIT_ASSERT( true );
}

// Interfaces
// ----------
