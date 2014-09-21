/*!
 * \file DbTest.h
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef DBTEST_H
#define DBTEST_H

// Same component headers
#include "wxOdbc3GoogleTest.h"
#include "TestParams.h"

// Other headers
#include "gtest/gtest.h"

// System headers

// Forward declarations
// --------------------
namespace exodbc
{
	class DbEnvironment;
	class Database;
}

// Structs
// -------

// Classes
// -------
namespace exodbc
{
	class DbTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	
	public:
		static void SetUpTestCase() {};
		static void TearDownTestCase() {};

	protected:
		virtual void SetUp();
		virtual void TearDown();

		exodbc::DbEnvironment* m_pDbEnv;
		exodbc::Database*	m_pDb;
		SOdbcInfo m_odbcInfo;
	};

	static std::vector<SOdbcInfo> test;

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		DbTest,
		::testing::ValuesIn(g_odbcInfos));

} // namespace exodbc

#endif // DBTEST_H
