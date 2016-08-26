/*!
* \file TestDbCreator.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 04.04.2015
* \copyright GNU Lesser General Public License Version 3
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
using namespace exodbc;

namespace exodbctest
{
	// Construction
	// -------------
	TestDbCreator::TestDbCreator(const TestParams& odbcInfo)
		: m_odbcInfo(odbcInfo)
	{
		// prepare env
		m_pEnv = std::make_shared<Environment>();
		m_pEnv->Init(OdbcVersion::V_3);

		// Create and open Database
		m_pDb = std::make_shared<Database>();
		m_pDb->Init(m_pEnv);
		if (odbcInfo.HasConnectionString())
		{
			m_pDb->Open(odbcInfo.m_connectionString);
		}
		else
		{
			m_pDb->Open(odbcInfo.m_dsn, odbcInfo.m_username, odbcInfo.m_password);
		}
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
				if (!ba::trim_copy(stmt).empty())
				{
					try
					{
						m_pDb->ExecSql(stmt);
					}
					catch (const Exception& ex)
					{
						// exec failed - guess if this was the drop statement
						if (ba::contains(ba::to_lower_copy(stmt), L"drop table"))
						{
							LOG_WARNING(boost::str(boost::wformat(L"Execution of a statement failed, but it was probably a DROP statement: %s") % ex.ToString()));
						}
						else
						{
							throw;
						}
					}
					stmt = L"";
				}
			}
		}
		if (!ba::trim_copy(stmt).empty())
		{
			m_pDb->ExecSql(stmt);
		}
		m_pDb->CommitTrans();
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
				m_pDb->ExecSql(stmt);
				stmt = L"";
			}
		}
		if (!stmt.empty())
		{
			m_pDb->ExecSql(stmt);
		}
		m_pDb->CommitTrans();
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
			m_pDb->ExecSql(sqlstmt);
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
			m_pDb->ExecSql(drop);
		}
		catch (const Exception& ex)
		{
			LOG_WARNING(boost::str(boost::wformat(L"Failed to drop Table %s: %s") % tableName % ex.ToString()));
		}
	}

} //namespace exodbctest
