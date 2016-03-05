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
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/SqlStatementCloser.h"

// System headers

// Forward declarations
// --------------------

namespace exodbctest
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
		exodbc::EnvironmentPtr m_pEnv = std::make_shared<exodbc::Environment>();
		exodbc::DatabasePtr m_pDb = std::make_shared<exodbc::Database>();
	};

} // namespace exodbctest
