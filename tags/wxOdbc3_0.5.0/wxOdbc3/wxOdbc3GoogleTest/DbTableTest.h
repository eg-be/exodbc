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
#include "wxOdbc3GoogleTest.h"
#include "TestParams.h"

// Other headers
#include "gtest/gtest.h"

// System headers

// Forward declarations
// --------------------
class wxDb;
class wxDbConnectInf;

// Structs
// -------

// Classes
// -------
namespace wxOdbc3Test
{
	class DbTableTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	public:
		static void SetUpTestCase() {};
		static void TearDownTestCase() {};

	protected:
		wxDb*	m_pDb;
		wxDbConnectInf* m_pConnectInf;
		SOdbcInfo m_odbcInfo;
		virtual void SetUp();
		virtual void TearDown();
	};

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		DbTableTest,
		::testing::ValuesIn(g_odbcInfos));
}


#endif // DBTABLETEST_H
