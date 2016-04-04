/*!
* \file SqlStmtCloserTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2015
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

	class StatementCloserTest : public ::testing::Test
	{

	protected:
		virtual void SetUp();

		TestParams m_odbcInfo;
		EnvironmentPtr m_pEnv = std::make_shared<Environment>();
		DatabasePtr m_pDb = std::make_shared<Database>();
	};

} // namespace exodbc
