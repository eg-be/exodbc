/*!
* \file PreparedStatementTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 20.12.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief Header-file description]
*/

#pragma once

// Same component headers
#include "exOdbcGTest.h"
#include "TestParams.h"

// Other headers
#include "gtest/gtest.h"
#include "Database.h"

// System headers

// Forward declarations
// --------------------

namespace exodbc
{


	// Structs
	// -------

	// Classes
	// -------

	class ExecutableStatementTest : public ::testing::Test
	{
	protected:
		virtual void SetUp();

		EnvironmentPtr m_pEnv = std::make_shared<Environment>();
		DatabasePtr m_pDb = std::make_shared<Database>();
	};

} // namespace exodbc
