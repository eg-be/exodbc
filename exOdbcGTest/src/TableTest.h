/*!
 * \file DbTableTest.h
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef DBTABLETEST_H
#define DBTABLETEST_H

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
	class Table;
	class MIntTypesTable;
}

// Structs
// -------

// Classes
// -------
namespace exodbc
{
	class TableTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	public:
		static void SetUpTestCase() {};
		static void TearDownTestCase() {};

	protected:
		exodbc::Environment m_env;
		exodbc::Database m_db;
		exodbc::Database*	m_pDb;
		exodbc::Environment* m_pEnv;
		exodbc::Table* m_pIntTypesAutoTable;
		exodbc::MIntTypesTable* m_pIntTypesManualTable;

		SOdbcInfo m_odbcInfo;
		virtual void SetUp();
		virtual void TearDown();
	};

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		TableTest,
		::testing::ValuesIn(g_odbcInfos));
} // namespace exodbc


#endif // DBTABLETEST_H
