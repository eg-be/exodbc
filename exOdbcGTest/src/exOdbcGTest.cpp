/*!
 * \file exOdbc3GoogleTest.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
 * 
 * Defines the entry point for the console (test-)application.
 */ 

#include "stdafx.h"

// Own header
#include "exOdbcGTest.h"

// Same component headers
#include "TestDbCreator.h"

// Other headers
#include "boost/log/trivial.hpp"
#include "boost/log/core.hpp"
#include "boost/log/expressions.hpp"
#include "boost/filesystem.hpp"

// Debug
#include "DebugNew.h"

// Globals
// -------
namespace exodbc
{
	TestParams g_odbcInfo;
}

namespace ba = boost::algorithm;

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
bool extractParamValue( int argc, const _TCHAR* const argv[],const std::wstring& name, std::wstring& value)
{
	for(int i = 0; i < argc; i++)
	{
		std::wstring arg(argv[i]);
		if(arg.length() > name.length() && arg.substr(0, name.length()) == name)
		{
			value = arg.substr(name.length());
			return true;
		}
	}

	return false;
}

#include "boost/variant/polymorphic_get.hpp"

void printHelp()
{
	struct Base
	{
		int m = 5;
	};
	struct Der : public Base
	{
		int u = 3;
	};
	boost::variant<Der> var = Der();
	try
	{
		Base b = boost::polymorphic_get<Base>(var);
		Der d = boost::polymorphic_get<Der>(var);
		int p = 3;
	}
	catch (boost::bad_get& ex)
	{
		int p = 3;
	}
	using namespace std;

	wcerr << L"Usage: exOdbcGTest [OPTION]... [DATABASE]\n";
	wcerr << L"Run unit-tests against DATABASE or read configuration from TestSettings.xml.\n";
	wcerr << L"If DATABASE is not specified, all settings will be read from the file\n";
	wcerr << L"TestSettings.xml located in the search-path for the app.\n";
	wcerr << L"\n";
	wcerr << L"DATABASE must specify how to connect to the database and which case the test\n";
	wcerr << L"tables use. You can either supply a connection-string or a DSN-entry (which also\n";
	wcerr << L"requires username and password).\n";
	wcerr << L" To connect using a configured DSN entry use:\n";
	wcerr << L"\n";
	wcerr << L"  DSN=dsn;user;pass (to connect to a database with UPPER-case names)\n";
	wcerr << L"  dsn=dsn;user;pass (to connect to a database with lower-case names)\n";
	wcerr << L"\n";
	wcerr << L" or to connect using a connection-string use:\n";
	wcerr << L"\n";
	wcerr << L"  CS=connectionString (to connect to a database with UPPER-case names)\n";
	wcerr << L"  cs=connectionString (to connect to a database with lower-case names)\n";
	wcerr << L"\n";
	wcerr << L"Note that you must frame the 'cs=connectionString' with \" if your connection\n";
	wcerr << L"string contains white spaces.\n";
	wcerr << L"\n";
	wcerr << L"--createDb       Runs the script to create the test databases.\n";
	wcerr << L"--logLevelN      Set the logLevel to N, where N must be a single Letter:\n";
	wcerr << L"                 (T)race, (D)ebug, (I)nfo, (W)arning, (E)rror, (F)atal\n";
	wcerr << L"\n";
	wcerr << L"Examples:\n";
	wcerr << L" exodbcGTest --createDb --logLevelW DSN=db2;uid;pass\n";
	wcerr << L"  to run the tests against a configured DSN entry named 'db2', using uppercase\n";
	wcerr << L"  names and log level Waring. Before the tests are run, the scripts to create\n";
	wcerr << L"  the test database are run.\n";
	wcerr << L" exodbcGTest CS=\"Driver={IBM DB2 ODBC DRIVER};Database=EXODBC;Hostname=192.168.56.20;Port=50000;Protocol=TCPIP;Uid=db2ex;Pwd=extest;\"\n";
	wcerr << L"  to run the tests using a connection string, against a DB which uses uppercase.\n";
}


int _tmain(int argc, _TCHAR* argv[])
{
	printHelp();

	using namespace std;
	using namespace exodbc;
	namespace ba = boost::algorithm;

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	int status = 0;

	// Set defaults
	bool doCreateDb = false;
	g_odbcInfo.m_logSeverity = boost::log::trivial::info;

	// Iterate given options
	for (int i = 1; i < argc; i++)
	{
		std::wstring logLevel = L"--logLevel";
		std::wstring createDb = L"--createDb";
		std::wstring dsn = L"dsn=";
		std::wstring DSN = L"DSN=";
		std::wstring cs = L"cs=";
		std::wstring CS = L"CS=";
		std::wstring arg(argv[i]);
		std::wstring upperDsn, lowerDsn, upperCs, lowerCs;
		if (ba::starts_with(arg, logLevel) && arg.length() > logLevel.length())
		{
			switch (arg[logLevel.length()])
			{
			case 'T':
				g_odbcInfo.m_logSeverity = boost::log::trivial::trace;
				break;
			case 'D':
				g_odbcInfo.m_logSeverity = boost::log::trivial::debug;
				break;
			case 'I':
				g_odbcInfo.m_logSeverity = boost::log::trivial::info;
				break;
			case 'W':
				g_odbcInfo.m_logSeverity = boost::log::trivial::warning;
				break;
			case 'E':
				g_odbcInfo.m_logSeverity = boost::log::trivial::error;
				break;
			case 'F':
				g_odbcInfo.m_logSeverity = boost::log::trivial::fatal;
				break;
			}
		}
		else if (arg == createDb)
		{
			doCreateDb = true;
		}
		else if (ba::starts_with(arg, DSN) && arg.length() > DSN.length())
		{
			upperDsn = arg.substr(DSN.length());
		}
		else if (ba::starts_with(arg, dsn) && arg.length() > dsn.length())
		{
			lowerDsn = arg.substr(dsn.length());
		}
		else if (ba::starts_with(arg, CS) && arg.length() > CS.length())
		{
			upperCs = arg.substr(CS.length());
		}
		else if (ba::starts_with(arg, cs) && arg.length() > cs.length())
		{
			lowerCs = arg.substr(cs.length());
		}
		if (upperDsn.length() > 0 || lowerDsn.length() > 0)
		{
			std::wstring dsn = upperDsn.length() > 0 ? upperDsn : lowerDsn;
			std::vector<std::wstring> tokens;
			boost::split(tokens, dsn, boost::is_any_of(L";"));
			if (tokens.size() != 3)
			{
				LOG_WARNING(boost::str(boost::wformat(L"Ignoring Dsn entry '%s' because it does not match the form 'dsn;user;pass'") % arg));
			}
			else if (tokens[0].empty())
			{
				LOG_WARNING(boost::str(boost::wformat(L"Ignoring Dsn entry '%s' because DSN is empty.") % arg));
			}
			else
			{
				test::Case nameCase = upperDsn.length() > 0 ? test::Case::UPPER : test::Case::LOWER;
				TestParams dsnEntry(tokens[0], tokens[1], tokens[2], nameCase);
				g_odbcInfo = dsnEntry;
			}
		}
		if (upperCs.length() > 0 || lowerCs.length() > 0)
		{
			test::Case nameCase = upperCs.length() > 0 ? test::Case::UPPER : test::Case::LOWER;
			TestParams csEntry(upperCs.length() > 0 ? upperCs : lowerCs, nameCase);
			g_odbcInfo = csEntry;
		}
	}

	if (!g_odbcInfo.IsUsable())
	{
		// Read default settings
		LOG_INFO(L"Loading Default Settings from TestSettings.xml");
		try
		{
			// Try to locate TestSettings.xml in directory of exe
			namespace fs = boost::filesystem;
			fs::wpath exePath(argv[0]);
			exePath.normalize();
			fs::wpath exeDir = exePath.parent_path();
			fs::wpath settingsPath = exeDir / L"TestSettings.xml";
			if( ! fs::exists(settingsPath))
			{ 
				// try if it is up somewhere in my working copy
				settingsPath = settingsPath.parent_path().parent_path().parent_path().parent_path();
				settingsPath /= L"exOdbcGTest/TestSettings.xml";
			}
			g_odbcInfo.Load(settingsPath);
			doCreateDb = g_odbcInfo.m_createDb;
		}
		catch (const Exception& ex)
		{
			LOG_ERROR(ex.ToString());
			return -1;
		}
	}

	LOG_INFO(L"Running tests against: " << g_odbcInfo);

	// Set a filter for the logging
	boost::log::core::get()->set_filter
		(
		boost::log::trivial::severity >= boost::ref(g_odbcInfo.m_logSeverity)
		);

	// Check if we need to re-create the dbs
	if (doCreateDb)
	{
		try
		{
			namespace fs = boost::filesystem;
			// Prepare Db-creator
			TestDbCreator creator(g_odbcInfo);
			// The base-path is relative to our app-path
			TCHAR moduleFile[MAX_PATH];
			if (::GetModuleFileName(NULL, moduleFile, MAX_PATH) == 0)
			{
				THROW_WITH_SOURCE(Exception, L"Failed in GetModuleFileName");
			}
			fs::wpath exePath(moduleFile);
			fs::wpath exeDir = exePath.parent_path();
			fs::wpath scriptDir = exeDir / L"CreateScripts" / DatabaseProcudt2s(creator.GetDbms());
			if (!fs::is_directory(scriptDir))
			{
				THROW_WITH_SOURCE(Exception, boost::str(boost::wformat(L"ScriptDirectory '%s' is not a directory") % scriptDir.native()));
			}
			creator.SetScriptDirectory(scriptDir);
			creator.RunAllScripts();
		}
		catch (const Exception& ex)
		{
			LOG_ERROR(boost::str(boost::wformat(L"Failed to create Test-Database for DSN '%s': %s") % g_odbcInfo.m_dsn % ex.ToString()));
			::getchar();
			return -1;
		}
	}

	// Note: We cannot call Init earlier, we must call it after we've set up the global with the param-values
	::testing::InitGoogleTest(&argc, argv);

	if (status != 0)
	{
		printHelp();
		return status;
	}

	int result = RUN_ALL_TESTS();
	
	return result;
}

// Interfaces
// ----------


