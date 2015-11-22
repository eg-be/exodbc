/*!
* \file TestDbCreator.h
* \author Elias Gerber <eg@elisium.ch>
* \date 04.04.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief Header-file description]
*/

#pragma once
#ifndef TESTDBCREATOR_H
#define TESTDBCREATOR_H

// Same component headers
#include "exOdbcGTest.h"
#include "TestTables.h"

// Other headers
#include "Environment.h"
#include "Database.h"
#include "gtest/gtest.h"
#include "boost/filesystem.hpp"

// System headers
#include <map>

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
namespace exodbc
{

	class TestDbCreator
	{
	public:
		TestDbCreator(const TestParams& odbcInfo);
		~TestDbCreator();

		void RunAllScripts();
		void RunScript(const std::wstring& scriptName);
		void RunScript(const boost::filesystem::wpath& scriptPath);

		void SetScriptDirectory(const boost::filesystem::wpath& path);
		boost::filesystem::wpath GetScriptDirectory() const;

		DatabaseProduct GetDbms() const { return m_pDb->GetDbms(); };

	private:

		std::vector<std::wstring> LoadScriptFile(const boost::filesystem::wpath& path) const;

		void ExecSqlIgnoreFail(const std::wstring sqlstmt);

		void DropIfExists(const std::wstring& tableName);

		boost::filesystem::wpath m_scriptDirectoryPath;

		TestParams m_odbcInfo;
		EnvironmentPtr m_pEnv;
		DatabasePtr m_pDb;
	};

} // namespace exodbc

#endif // TESTDBCREATOR_H