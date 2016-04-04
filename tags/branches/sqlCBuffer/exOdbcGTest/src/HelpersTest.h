/*!
* \file HelpersTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 09.08.2014
* \copyright GNU Lesser General Public License Version 3
* 
* [Brief Header-file description]
*/ 

#pragma once
#ifndef HELPERSTEST_H
#define HELPERSTEST_H

// Same component headers
#include "exOdbcGTest.h"
#include "TestParams.h"

// Other headers
#include "gtest/gtest.h"
#include "Environment.h"
#include "Database.h"
#include "SqlStatementCloser.h"

// System headers

// Forward declarations
// --------------------

namespace exodbc
{


	// Structs
	// -------

	// Classes
	// -------
	class HelpersTest : public ::testing::Test {

	protected:

	};
} // namespace exodbc
#endif // HELPERSTEST_H
