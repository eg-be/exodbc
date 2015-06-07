/*!
* \file ExcelTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 15.03.2015
* \copyright wxWindows Library Licence, Version 3.1
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

	class ExcelTest : public ::testing::TestWithParam<SOdbcInfo>
	{
	
	protected:
		virtual void SetUp();

		virtual void TearDown();

		Environment m_env;
		SOdbcInfo m_odbcInfo;
	};

	INSTANTIATE_TEST_CASE_P(
		ParametrizedOdbc,
		ExcelTest,
		::testing::ValuesIn(g_odbcInfos));

} // namespace exodbc

#endif // EXCELTEST_H