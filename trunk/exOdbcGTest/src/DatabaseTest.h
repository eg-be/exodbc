/*!
 * \file DbTest.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright wxWindows Library Licence, Version 3.1
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
#include "Environment.h"
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
	class DatabaseTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	
	public:
		static void SetUpTestCase() {};
		static void TearDownTestCase() {};

	protected:
		virtual void SetUp();
		virtual void TearDown();

		exodbc::Environment m_env;
		exodbc::Database	m_db;
		SOdbcInfo m_odbcInfo;
	};

	static std::vector<SOdbcInfo> test;

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		DatabaseTest,
		::testing::ValuesIn(g_odbcInfos));

} // namespace exodbc

#endif // DBTEST_H
