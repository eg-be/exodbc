/*!
* \file wxCompatibilityTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 31.08.2014
* \copyright GNU Lesser General Public License Version 3
* 
* Contains some tests that try to operate on
* the table/db objects the same way as it was operated
* by me using wx 2.8
*/ 

#pragma once

// Same component headers
#include "exOdbcGTest.h"
#include "TestParams.h"

// Other headers
#include "gtest/gtest.h"
#include "Environment.h"
#include "Database.h"

// System headers


namespace exodbctest
{
	//	 Forward declarations
	// --------------------

	// Structs
	// -------
	
	// Classes
	// -------
	class wxCompatibilityTest : public ::testing::Test
	{
	protected:
		virtual void SetUp();
		virtual void TearDown();

		exodbc::EnvironmentPtr m_pEnv = std::make_shared<exodbc::Environment>();
		exodbc::DatabasePtr m_pDb = std::make_shared<exodbc::Database>();

	};
}

