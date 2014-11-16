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
#include "exOdbcGTest.h"
#include "TestParams.h"

// Other headers
#include "gtest/gtest.h"
#include "DbEnvironment.h"
#include "Database.h"

// System headers

// Forward declarations
// --------------------

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

		exodbc::DbEnvironment m_env;
		exodbc::Database	m_db;
		SOdbcInfo m_odbcInfo;
	};

	static std::vector<SOdbcInfo> test;

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		DbTest,
		::testing::ValuesIn(g_odbcInfos));

} // namespace exodbc

#endif // DBTEST_H
