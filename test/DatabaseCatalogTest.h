/*!
 * \file DatabaseCatalogTest.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 20.04.2017
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
	class DatabaseCatalogTest : public ::testing::Test
	{
	protected:
		virtual void SetUp();
		virtual void TearDown();

		exodbc::EnvironmentPtr m_pEnv = std::make_shared<exodbc::Environment>();
		exodbc::DatabasePtr	m_pDb = std::make_shared<exodbc::Database>();
	};
} // namespace exodbctest

