/*!
 * \file DbTest.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
 * 
 * [Brief Header-file description]
 */ 

#pragma once
#ifndef DBTEST_H
#define DBTEST_H

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

// Structs
// -------

// Classes
// -------
namespace exodbc
{
	class DatabaseTest : public ::testing::Test
	{
	public:
		static void SetUpTestCase();
		static void TearDownTestCase() {};

	protected:
		virtual void SetUp();
		virtual void TearDown();

		EnvironmentPtr m_pEnv;
		DatabasePtr	m_pDb;
		TestParams m_odbcInfo;
	};
} // namespace exodbc

#endif // DBTEST_H
