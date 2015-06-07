/*!
* \file TestSkipper.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 06.04.2015
* \copyright wxWindows Library Licence, Version 3.1
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "TestSkipper.h"

// Same component headers
// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
using namespace std;

namespace exodbc
{
	void TestSkipper::AddDatabase(DatabaseProduct db)
	{
		m_dbs.insert(db);
	}


	void TestSkipper::AddTest(const std::string& testName)
	{
		m_testsByName.insert(testName);
	}


	void TestSkipper::AddTest(DatabaseProduct db, const std::string& testName)
	{
		// If we already have a set use that, else create a new one
		DatabaseTestsMap::iterator it = m_testsByDb.find(db);
		if (it != m_testsByDb.end())
		{
			it->second.insert(testName);
		}
		else
		{
			TestNamesSet tests = { testName };
			m_testsByDb[db] = tests;
		}
	}


	bool TestSkipper::ContainsDb(DatabaseProduct db) const
	{
		return m_dbs.find(db) != m_dbs.end();
	}


	bool TestSkipper::ContainsTest(const testing::TestInfo* const pTestInfo) const
	{
		return ContainsTest(pTestInfo->name());
	}


	bool TestSkipper::ContainsTest(const std::string& testName) const
	{
		string searchName = testName;
		// Remove last occurrence of '/0', '/1' etc. in TestName
		size_t slashPos = searchName.find_last_of("/");
		if (slashPos != string::npos)
		{
			searchName = testName.substr(0, slashPos);
		}
		if (m_testsByName.find(searchName) == m_testsByName.end())
		{
			return false;
		}
		return true;
	}


	bool TestSkipper::ContainsTest(DatabaseProduct db, const std::string& testName) const
	{
		DatabaseTestsMap::const_iterator it = m_testsByDb.find(db);
		if (it == m_testsByDb.end())
		{
			return false;
		}
		string searchName = testName;
		// Remove last occurrence of '/0', '/1' etc. in TestName
		size_t slashPos = searchName.find_last_of("/");
		if (slashPos != string::npos)
		{
			searchName = testName.substr(0, slashPos);
		}
		TestNamesSet names = it->second;
		if (names.find(searchName) == names.end())
		{
			return false;
		}
		return true;
	}


	bool TestSkipper::ContainsTest(DatabaseProduct db, const testing::TestInfo* const pTestInfo) const
	{
		return ContainsTest(db, pTestInfo->name());
	}


	std::wstring TestSkipper::FormatSkipMessage(DatabaseProduct db, const testing::TestInfo* const pTestInfo) const
	{
		string sName(pTestInfo->name());
		wstring wsName(sName.begin(), sName.end());

		return boost::str(boost::wformat(L"Database '%s' is skipping Test '%s'") % DatabaseProcudt2s(db) % wsName);
	}

	// Interfaces
	// ----------


} // namespace exodbc