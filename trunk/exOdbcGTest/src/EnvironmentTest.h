/*!
* \file EnvironmentTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 27.07.2014
* \copyright wxWindows Library Licence, Version 3.1
* 
* [Brief Header-file description]
*/ 

#pragma once
#ifndef EnvironmentTEST_H
#define EnvironmentTEST_H

// Same component headers
#include "exOdbcGTest.h"
#include "TestParams.h"
#include "TestSkipper.h"

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

		static TestSkipper s_testSkipper;
	};

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		EnvironmentTest,
		::testing::ValuesIn(g_odbcInfos));

} // namespace exodbc

#endif // EnvironmentTEST_H
