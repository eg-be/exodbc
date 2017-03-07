/*!
* \file TestDbCreator.h
* \author Elias Gerber <eg@elisium.ch>
* \date 04.04.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief Header-file description]
*/

#pragma once

// Same component headers
#include "exOdbcGTest.h"

// Other headers
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
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
namespace exodbctest
{

	class TestDbCreator
	{
	public:
		TestDbCreator(const TestParams& odbcInfo);
		~TestDbCreator();

		void RunAllScripts();
		void RunScript(const std::string& scriptName);
		void RunScript(const boost::filesystem::path& scriptPath);

		void SetScriptDirectory(const boost::filesystem::path& path);
		boost::filesystem::wpath GetScriptDirectory() const;

		exodbc::DatabaseProduct GetDbms() const { return m_pDb->GetDbms(); };

	private:

		std::vector<std::string> LoadScriptFile(const boost::filesystem::path& path) const;

		void ExecSqlIgnoreFail(const std::string sqlstmt);

		void DropIfExists(const std::string& tableName);

		boost::filesystem::path m_scriptDirectoryPath;

		TestParams m_odbcInfo;
		exodbc::EnvironmentPtr m_pEnv;
		exodbc::DatabasePtr m_pDb;
	};

} // namespace exodbctest
