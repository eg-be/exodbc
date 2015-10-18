/*!
* \file SqlCBufferTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 18.10.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief Header-file description]
*/

#pragma once

// Same component headers
#include "exOdbcGTest.h"

// Other headers
#include "gtest/gtest.h"

// System headers

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
namespace exodbc
{
	class SqlCBufferLengthIndicatorTest : public ::testing::Test
	{
	protected:
		virtual void SetUp() {};
		virtual void TearDown() {};
	};


	class SqlCBufferTest : public ::testing::Test
	{
	protected:
		virtual void SetUp() {};
		virtual void TearDown() {};
	};

} // namespace exodbc
