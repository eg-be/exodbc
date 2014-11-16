/*!
* \file EnvironmentTest.h
* \author Elias Gerber <egerber@gmx.net>
* \date 27.07.2014
* 
* [Brief Header-file description]
*/ 

#pragma once
#ifndef EnvironmentTEST_H
#define EnvironmentTEST_H

// Same component headers
#include "exOdbcGTest.h"
#include "TestParams.h"

// Other headers
#include "gtest/gtest.h"

// Forward declarations
// --------------------

namespace exodbc
{
	// Structs
	// -------

	// Classes
	// -------
	class EnvironmentTest : public ::testing::TestWithParam<SOdbcInfo>
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
		EnvironmentTest,
		::testing::ValuesIn(g_odbcInfos));
} // namespace exodbc

#endif // EnvironmentTEST_H
