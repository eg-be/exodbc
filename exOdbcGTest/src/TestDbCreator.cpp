/*!
* \file TestDbCreator.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 04.04.2015
* \copyright wxWindows Library Licence, Version 3.1
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "TestDbCreator.h"

// Same component headers

// Other headers
#include <sstream>
#include <fstream>
#include <codecvt>
#include <locale>

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace  std;
namespace fs = boost::filesystem;
namespace ba = boost::algorithm;

namespace exodbc
{
	// Construction
	// -------------
	TestDbCreator::TestDbCreator(const SOdbcInfo& odbcInfo)
		: m_odbcInfo(odbcInfo)
	{
		// prepare env
		m_env.AllocateEnvironmentHandle();
		m_env.SetOdbcVersion(OdbcVersion::V_3);
		// Create and open Database
		m_db.AllocateConnectionHandle(m_env);
		m_db.Open(odbcInfo.m_dsn, odbcInfo.m_username, odbcInfo.m_password);
	}

	// Destructor
	// -----------
	TestDbCreator::~TestDbCreator()
	{

	}

	// Implementation
	// --------------
	void TestDbCreator::SetScriptDirectory(const boost::filesystem::wpath& path)
	{
		m_scriptDirectoryPath = path;
	}


	boost::filesystem::wpath TestDbCreator::GetScriptDirectory() const
	{
		if (m_scriptDirectoryPath.empty())
		{
			IllegalArgumentException ex(L"No ScriptDirectory set.");
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		return m_scriptDirectoryPath;
	}


	void TestDbCreator::RunAllScripts()
	{

		fs::wpath scriptDir = GetScriptDirectory();
		fs::directory_iterator end_itr;
		for (fs::directory_iterator itr(scriptDir);	itr != end_itr;	++itr)
		{
			fs::wpath filePath = *itr;
			wstring ext = filePath.extension().native();
			ba::to_lower(ext);
			if (ext == L".sql")
			{
				LOG_INFO(boost::str(boost::wformat(L"Running script '%s'") % filePath.native()));
				RunScript(filePath);
			}
		}
	}


	void TestDbCreator::RunScript(const boost::filesystem::wpath& scriptPath)
	{
		// Load the passed script
		vector<wstring> lines = LoadScriptFile(scriptPath);
		// Run as one long statement
		std::wstring stmt;
		for (vector<wstring>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			namespace ba = boost::algorithm;
			wstring line = *it;
			stmt += (line + L" ");
			// execute on ';' or on 'GO' or on empty line
			bool execute = (line.empty() || ba::ends_with(ba::trim_copy(stmt), L";") || ba::ends_with(ba::trim_copy(stmt), L"GO"));
			if (execute)
			{
				// remove 'GO'
				if (ba::trim_copy(line) == L"GO")
				{
					ba::erase_last(stmt, L"GO");
				}
				// might have become empty if GO was on a single line or so
				if (!stmt.empty())
				{
					m_db.ExecSql(stmt);
					stmt = L"";
				}
			}
		}
		if (!stmt.empty())
		{
			m_db.ExecSql(stmt);
		}
		m_db.CommitTrans();
	}


	void TestDbCreator::RunScript(const std::wstring& scriptName)
	{
		// Load the passed script
		vector<wstring> lines = LoadScriptFile(scriptName);
		// Run as one long statement
		std::wstring stmt;
		for (vector<wstring>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			namespace ba = boost::algorithm;
			wstring line = *it;
			stmt += line;
			bool execute = !stmt.empty() && (line.empty() || ba::ends_with(ba::trim_copy(stmt), L";"));
			if (execute)
			{
				m_db.ExecSql(stmt);
				stmt = L"";
			}
		}
		if (!stmt.empty())
		{
			m_db.ExecSql(stmt);
		}
		m_db.CommitTrans();
	}


	vector<wstring> TestDbCreator::LoadScriptFile(const boost::filesystem::wpath& path) const
	{
		vector<wstring> lines;

		wifstream wfile(path.native());
#pragma push_macro("new")
#ifdef new
#undef new
#endif
		wfile.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
#pragma pop_macro("new")
		if (!wfile.is_open())
		{
			THROW_WITH_SOURCE(Exception, boost::str(boost::wformat(L"Failed to open file '%s'") % path.native()));
		}
		wstring line;
		while (getline(wfile, line))
		{
			lines.push_back(line);
		}
		if (wfile.bad())
		{
			THROW_WITH_SOURCE(Exception, boost::str(boost::wformat(L"Failed to read from file '%s'") % path.native()));
		}

		return lines;
	}
	

	void TestDbCreator::ExecSqlIgnoreFail(const std::wstring sqlstmt)
	{
		try
		{
			m_db.ExecSql(sqlstmt);
		}
		catch (const Exception& ex)
		{
			LOG_WARNING(boost::str(boost::wformat(L"Failed to Execute Statement: %s") % ex.ToString()));
		}
	}


	void TestDbCreator::DropIfExists(const std::wstring& tableName)
	{
		try
		{
			wstring drop = boost::str(boost::wformat(L"DROP TABLE %s") % tableName);
			m_db.ExecSql(drop);
		}
		catch (const Exception& ex)
		{
			LOG_WARNING(boost::str(boost::wformat(L"Failed to drop Table %s: %s") % tableName % ex.ToString()));
		}
	}

	// Interfaces
	// ----------

} //namespace exodbc
