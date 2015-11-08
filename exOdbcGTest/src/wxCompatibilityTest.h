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
#ifndef WXCOMPATIBILITYTEST_H
#define WXCOMPATIBILITYTEST_H

// Same component headers
#include "exOdbcGTest.h"
#include "TestParams.h"

// Other headers
#include "gtest/gtest.h"
#include "Environment.h"
#include "Database.h"

// System headers


namespace exodbc
{
	//	 Forward declarations
	// --------------------
	class Database;

	// Structs
	// -------
	
	// Classes
	// -------
	class wxCompatibilityTest : public ::testing::TestWithParam<TestParams>
	{
	public:
		static void SetUpTestCase();
		static void TearDownTestCase() {};

	protected:
		virtual void SetUp();
		virtual void TearDown();

		TestParams m_odbcInfo;
		exodbc::Environment	m_env;
		exodbc::Database	m_db;
	};

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		wxCompatibilityTest,
		::testing::ValuesIn(g_odbcInfos));	 

}


#endif // WXCOMPATIBILITYTEST_H
