/*!
* \file EnvironmentTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 27.07.2014
* \copyright GNU Lesser General Public License Version 3
* 
* [Brief Header-file description]
*/ 

#pragma once

// Same component headers
#include "exOdbcTest.h"
#include "TestParams.h"

// Other headers
#include "gtest/gtest.h"

// Forward declarations
// --------------------

namespace exodbctest
{
	// Structs
	// -------

	// Classes
	// -------
	class EnvironmentTest : public ::testing::Test
	{
	protected:
		virtual void SetUp();
		virtual void TearDown();
	};
} // namespace exodbctest

