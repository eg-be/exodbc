/*!
 * \file DbTableTest.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
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
	class TableTest : public ::testing::Test
	{
	public:
		static void SetUpTestCase();
		static void TearDownTestCase() {};

	protected:
		virtual void SetUp();
		virtual void TearDown();

		exodbc::Environment m_env;
		exodbc::Database m_db;
		TestParams m_odbcInfo;
	};
} // namespace exodbc


#endif // DBTABLETEST_H