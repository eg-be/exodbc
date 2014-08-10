/*!
* \file DbEnvironmentTest.h
* \author Elias Gerber <egerber@gmx.net>
* \date 27.07.2014
* 
* [Brief Header-file description]
*/ 

#pragma once
#ifndef DBENVIRONMENTTEST_H
#define DBENVIRONMENTTEST_H

// Same component headers
#include "wxOdbc3GoogleTest.h"
#include "TestParams.h"

// Other headers
#include "Utils.h"
#include "gtest/gtest.h"

// Forward declarations
// --------------------

namespace exodbc
{
	// Structs
	// -------

	// Classes
	// -------
	class DbEnvironmentTest : public ::testing::TestWithParam<SOdbcInfo>
	{

	public:
		static void SetUpTestCase() {};
		static void TearDownTestCase() {};

	protected:
		virtual void SetUp();
		virtual void TearDown();

		SOdbcInfo m_odbcInfo;
	};

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		DbEnvironmentTest,
		::testing::ValuesIn(g_odbcInfos));
} // namespace exodbc

#endif // DBENVIRONMENTTEST_H
