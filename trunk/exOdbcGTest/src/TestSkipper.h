/*!
* \file TestSkipper.h
* \author Elias Gerber <eg@elisium.ch>
* \date 06.04.2015
* \copyright wxWindows Library Licence, Version 3.1
*
* [Brief Header-file description]
*/

#pragma once
#ifndef TESTSKIPPER_H
#define TESTSKIPPER_H

// Same component headers
#include "TestTables.h"

// Other headers
#include "exOdbc.h"

// System headers
#include <map>
#include <set>
#include <string>

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
namespace exodbc
{
	typedef std::set<std::string> TestNamesSet;
	typedef std::set<DatabaseProduct> DatabasesSet;
	typedef std::map<DatabaseProduct, TestNamesSet> DatabaseTestsMap;

	class TestSkipper
	{
	public:
		void AddDatabase(DatabaseProduct db);
		bool ContainsDb(DatabaseProduct db) const;

		void AddTest(DatabaseProduct db, const std::string& testName);
		bool ContainsTest(DatabaseProduct db, const std::string& testName) const;
		bool ContainsTest(DatabaseProduct db, const testing::TestInfo* const pTestInfo) const;
		std::wstring FormatSkipMessage(DatabaseProduct db, const testing::TestInfo* const pTestInfo) const;

	private:
		DatabaseTestsMap m_testsByDb;
		DatabasesSet m_dbs;
	};


#define MAYBE_SKIPP_TEST(testSkipper, db) \
		{ \
		if (testSkipper.ContainsDb(db.GetDbms())) { \
			return; \
		} \
		if (testSkipper.ContainsTest(db.GetDbms(), ::testing::UnitTest::GetInstance()->current_test_info())) \
		{ \
			LOG_WARNING(testSkipper.FormatSkipMessage(m_db.GetDbms(), ::testing::UnitTest::GetInstance()->current_test_info())); \
			return; \
		} \
	} while (0)

} // namespace exodbc

#endif // TESTSKIPPER_H
