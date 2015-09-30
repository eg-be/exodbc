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

// System headers

// Forward declarations
// --------------------

namespace exodbc
{


	// Structs
	// -------

	// Classes
	// -------
	class ParamHelpersTest : public ::testing::TestWithParam<SOdbcInfo>
	{

	public:
		//static void SetUpTestCase() {};
		//static void TearDownTestCase() {};

	protected:
		virtual void SetUp();
		virtual void TearDown();

		SOdbcInfo m_odbcInfo;

		Environment m_env;
		Database m_db;
	};

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		ParamHelpersTest,
		::testing::ValuesIn(g_odbcInfos));

	class StaticHelpersTest : public ::testing::Test {

	protected:
		virtual void SetUp();
		virtual void TearDown();
	};

} // namespace exodbc
#endif // HELPERSTEST_H
