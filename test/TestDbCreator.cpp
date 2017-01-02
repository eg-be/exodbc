/*!
* \file TestDbCreator.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 04.04.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

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
	void TestDbCreator::SetScriptDirectory(const boost::filesystem::path& path)
	{
		m_scriptDirectoryPath = path;
	}
	

	boost::filesystem::path TestDbCreator::GetScriptDirectory() const
	{
		if (m_scriptDirectoryPath.empty())
		{
			IllegalArgumentException ex(u8"No ScriptDirectory set.");
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		return m_scriptDirectoryPath;
	}


	void TestDbCreator::RunAllScripts()
	{

		fs::path scriptDir = GetScriptDirectory();
		fs::directory_iterator end_itr;
		for (fs::directory_iterator itr(scriptDir);	itr != end_itr;	++itr)
		{
			fs::path filePath = *itr;
#ifdef _WIN32
			string ext = utf16ToUtf8(filePath.extension().native());
#else
			string ext = filePath.extension().native();
#endif
			ba::to_lower(ext);
			if (ext == u8".sql")
			{
#ifdef _WIN32
				LOG_INFO(boost::str(boost::format(u8"Running script '%s'") % utf16ToUtf8(filePath.native())));
#else
				LOG_INFO(boost::str(boost::format(u8"Running script '%s'") % filePath.native()));
#endif
				RunScript(filePath);
			}
		}
	}


	void TestDbCreator::RunScript(const boost::filesystem::path& scriptPath)
	{
		// Load the passed script
		vector<string> lines = LoadScriptFile(scriptPath);
		// Run as one long statement
		std::string stmt;
		for (vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			namespace ba = boost::algorithm;
			string line = *it;
			stmt += (line + u8" ");
			// execute on ';' or on 'GO' or on empty line
			bool execute = (line.empty() || ba::ends_with(ba::trim_copy(stmt), u8";") || ba::ends_with(ba::trim_copy(stmt), u8"GO"));
			if (execute)
			{
				// remove 'GO'
				if (ba::trim_copy(line) == u8"GO")
				{
					ba::erase_last(stmt, u8"GO");
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
						if (ba::contains(ba::to_lower_copy(stmt), u8"drop table"))
						{
							LOG_WARNING(boost::str(boost::format(u8"Execution of a statement failed, but it was probably a DROP statement: %s") % ex.ToString()));
						}
						else
						{
							throw;
						}
					}
					stmt = u8"";
				}
			}
		}
		if (!ba::trim_copy(stmt).empty())
		{
			m_pDb->ExecSql(stmt);
		}
		m_pDb->CommitTrans();
	}


	void TestDbCreator::RunScript(const std::string& scriptName)
	{
		// Load the passed script
		vector<string> lines = LoadScriptFile(scriptName);
		// Run as one long statement
		std::string stmt;
		for (vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			namespace ba = boost::algorithm;
			string line = *it;
			stmt += line;
			bool execute = !stmt.empty() && (line.empty() || ba::ends_with(ba::trim_copy(stmt), u8";"));
			if (execute)
			{
				m_pDb->ExecSql(stmt);
				stmt = u8"";
			}
		}
		if (!stmt.empty())
		{
			m_pDb->ExecSql(stmt);
		}
		m_pDb->CommitTrans();
	}


	vector<string> TestDbCreator::LoadScriptFile(const boost::filesystem::path& path) const
	{
		vector<string> lines;

		ifstream file(path.native());
#pragma push_macro("new")
#ifdef new
#undef new
#endif
		file.imbue(std::locale(std::locale::empty(), new std::codecvt_utf8<wchar_t>));
#pragma pop_macro("new")
		if (!file.is_open())
		{
#ifdef _WIN32
			THROW_WITH_SOURCE(Exception, boost::str(boost::format(u8"Failed to open file '%s'") % utf16ToUtf8(path.native())));
#else
			THROW_WITH_SOURCE(Exception, boost::str(boost::format(u8"Failed to open file '%s'") % path.native()));
#endif
		}
		string line;
		while (getline(file, line))
		{
			lines.push_back(line);
		}
		if (file.bad())
		{
#ifdef _WIN32
			THROW_WITH_SOURCE(Exception, boost::str(boost::format(u8"Failed to read from file '%s'") % utf16ToUtf8(path.native())));
#else
			THROW_WITH_SOURCE(Exception, boost::str(boost::format(u8"Failed to read from file '%s'") % path.native()));
#endif
		}

		return lines;
	}
	

	void TestDbCreator::ExecSqlIgnoreFail(const std::string sqlstmt)
	{
		try
		{
			m_pDb->ExecSql(sqlstmt);
		}
		catch (const Exception& ex)
		{
			LOG_WARNING(boost::str(boost::format(u8"Failed to Execute Statement: %s") % ex.ToString()));
		}
	}


	void TestDbCreator::DropIfExists(const std::string& tableName)
	{
		try
		{
			string drop = boost::str(boost::format(u8"DROP TABLE %s") % tableName);
			m_pDb->ExecSql(drop);
		}
		catch (const Exception& ex)
		{
			LOG_WARNING(boost::str(boost::format(u8"Failed to drop Table %s: %s") % tableName % ex.ToString()));
		}
	}

} //namespace exodbctest
