/*!
 * \file GenericDbTest.h
 * \author Elias Gerber <eg@zame.ch>
 * \date 22.09.2013
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef GENERICDBTEST_H
#define GENERICDBTEST_H

// Same component headers
// Other headers
#include "cppunit/extensions/HelperMacros.h"

// System headers

// Forward declarations
// --------------------
class wxDbConnectInf;
class wxDb;

// Structs
// -------

// DbConnectInfTest
// ----------------
class DbConnectInfTest : public CPPUNIT_NS::TestFixture
{
	CPPUNIT_TEST_SUITE( DbConnectInfTest );
	CPPUNIT_TEST( testAllocHenv );
	CPPUNIT_TEST_SUITE_END();

public:
	void testAllocHenv();
};
CPPUNIT_TEST_SUITE_REGISTRATION( DbConnectInfTest );


#endif // GENERICDBTEST_H
