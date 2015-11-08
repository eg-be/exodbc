/*!
* \file ExcelTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 15.03.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief Header-file description]
*/

#pragma once
#ifndef EXCELTEST_H
#define EXCELTEST_H

// Same component headers
#include "exOdbcGTest.h"

// Other headers
#include "gtest/gtest.h"
#include "Environment.h"

// System headers

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
namespace exodbc
{

	class ExcelTest : public ::testing::TestWithParam<TestParams>
	{
	
	protected:
		virtual void SetUp();

		virtual void TearDown();

		Environment m_env;
		TestParams m_odbcInfo;
	};

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		ExcelTest,
		::testing::ValuesIn(g_odbcInfos));

} // namespace exodbc

#endif // EXCELTEST_H