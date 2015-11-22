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
	class ParamHelpersTest : public ::testing::Test
	{

	public:
		//static void SetUpTestCase() {};
		//static void TearDownTestCase() {};

	protected:
		virtual void SetUp();
		virtual void TearDown();

		TestParams m_odbcInfo;

		EnvironmentPtr m_pEnv = std::make_shared<Environment>();
		DatabasePtr m_pDb = std::make_shared<Database>();
	};

	class StaticHelpersTest : public ::testing::Test {

	protected:
		virtual void SetUp();
		virtual void TearDown();
	};


	class StatementCloserTest : public ::testing::Test
	{

	protected:
		virtual void SetUp();;

		TestParams m_odbcInfo;
		EnvironmentPtr m_pEnv = std::make_shared<Environment>();
		DatabasePtr m_pDb = std::make_shared<Database>();
	};
} // namespace exodbc
#endif // HELPERSTEST_H
