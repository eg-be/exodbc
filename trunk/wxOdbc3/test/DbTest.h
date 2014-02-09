/*!
 * \file DbTest.h
 * \author Elias Gerber <eg@zame.ch>
 * \date 22.09.2013
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef DBTEST_H
#define DBTEST_H

// Same component headers
// Other headers
#include "cppunit/extensions/HelperMacros.h"

// System headers

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
class DbTest : public CPPUNIT_NS::TestFixture
{ 
	CPPUNIT_TEST_SUITE( DbTest );
	CPPUNIT_TEST( testFail );
	CPPUNIT_TEST( testOk );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();
	void testFail();
	void testOk();

};

CPPUNIT_TEST_SUITE_REGISTRATION( DbTest );

#endif // DBTEST_H
