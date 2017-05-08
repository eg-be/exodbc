/*!
* \file ExcelTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 15.03.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief Header-file description]
*/

#pragma once

// Same component headers
#include "exOdbcTest.h"

// Other headers
#include "gtest/gtest.h"
#include "exodbc/Environment.h"
#include "exodbc/Database.h"

// System headers

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
namespace exodbctest
{

	class ExcelTest : public ::testing::Test
	{
	
	protected:
		virtual void SetUp();

		virtual void TearDown();

		bool IsExcelDb();

		exodbc::EnvironmentPtr m_pEnv;
		exodbc::DatabasePtr m_pDb;
	};

} // namespace exodbctest
