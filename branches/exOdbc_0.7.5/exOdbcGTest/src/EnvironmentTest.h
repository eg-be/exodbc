/*!
* \file EnvironmentTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 27.07.2014
* \copyright GNU Lesser General Public License Version 3
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
	class EnvironmentTest : public ::testing::Test
	{

	public:
		static void SetUpTestCase() {};
		static void TearDownTestCase() {};

	protected:
		virtual void SetUp();
		virtual void TearDown();

		TestParams m_odbcInfo;
	};
} // namespace exodbc

#endif // EnvironmentTEST_H
