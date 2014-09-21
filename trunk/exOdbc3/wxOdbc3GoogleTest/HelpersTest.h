/*!
* \file HelpersTest.h
* \author Elias Gerber <egerber@gmx.net>
* \date 09.08.2014
* 
* [Brief Header-file description]
*/ 

#pragma once
#ifndef HELPERSTEST_H
#define HELPERSTEST_H

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


	// Structs
	// -------

	// Classes
	// -------
	class HelpersTest : public ::testing::TestWithParam<SOdbcInfo>
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
		HelpersTest,
		::testing::ValuesIn(g_odbcInfos));

} // namespace exodbc
#endif // HELPERSTEST_H
