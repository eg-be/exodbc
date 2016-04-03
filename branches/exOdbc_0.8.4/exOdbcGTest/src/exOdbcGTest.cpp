/*!
 * \file exOdbcGTest.cpp
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
#include "boost/filesystem.hpp"

// Debug
#include "DebugNew.h"

// Globals
// -------
namespace exodbctest
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


void printHelp()
{
	using namespace std;

	wcerr << L"Usage: exOdbcGTest [OPTION]... [DATABASE]" << std::endl;
	wcerr << L"Run unit-tests against DATABASE or read configuration from TestSettings.xml." << std::endl;
	wcerr << L"If DATABASE is not specified, all settings will be read from the file" << std::endl;
	wcerr << L"TestSettings.xml, located in either the directory of the app, or in a" << std::endl;
	wcerr << L"subdirectory named 'exOdbcGTest'. If the file is not found in the directory" << std::endl;
	wcerr << L"of the app, all parent directories are searched the same way." << std::endl;
	wcerr << std::endl;
	wcerr << L"If DATABASE is specified, you can either supply a connection-string or a DSN-entry" << std::endl;
	wcerr << L"to indicate the connection info of the test database:" << std::endl;
	wcerr << L" To connect using a configured DSN entry use:" << std::endl;
	wcerr << L"" << std::endl;
	wcerr << L"  DSN=dsn;user;pass (to connect to a database with UPPER-case names (tables/columns))" << std::endl;
	wcerr << L"  dsn=dsn;user;pass (to connect to a database with lower-case names (tables/columns))" << std::endl;
	wcerr << std::endl;
	wcerr << L" or to connect using a connection-string use:" << std::endl;
	wcerr << std::endl;
	wcerr << L"  CS=connectionString (to connect to a database with UPPER-case names (tables/columns))" << std::endl;
	wcerr << L"  cs=connectionString (to connect to a database with lower-case names (tables/columns))" << std::endl;
	wcerr << std::endl;
	wcerr << L"Note that you must frame the 'cs=connectionString' with \" if your connection" << std::endl;
	wcerr << L"string contains white spaces." << std::endl;
	wcerr << std::endl;
	wcerr << L"OPTION can be:" << std::endl;
	wcerr << L" --createDb       Run the scripts to create the test database before running the tests." << std::endl;
	wcerr << L"                    The TestDbCreator will connect to the database and try to detect" << std::endl;
	wcerr << L"                    the database system. If there is a subdirectory matching the" << std::endl;
	wcerr << L"                    database system name in 'CreateScripts' directory, all scripts" << std::endl;
	wcerr << L"                    inside that directory are run." << std::endl;
	wcerr << L" --logLevelX      Set the log level, where X must be either"<< std::endl;
	wcerr << L"                    E, W, I or D for Error, Warning, Info or Debug." << std::endl;
	wcerr << L" --help           To show this help text." << std::endl;
	wcerr << std::endl;
	wcerr << L"Examples:" << std::endl;
	wcerr << L" exodbcGTest --createDb --logLevelW DSN=db2;uid;pass" << std::endl;
	wcerr << L"  to run the tests against a configured DSN entry named 'db2', using uppercase" << std::endl;
	wcerr << L"  names and log level Waring. Before the tests are run, the scripts to create" << std::endl;
	wcerr << L"  the test database are run." << std::endl;
	wcerr << L" exodbcGTest CS=\"Driver={IBM DB2 ODBC DRIVER};Database=EXODBC;Hostname=192.168.56.20;Port=50000;Protocol=TCPIP;Uid=db2ex;Pwd=extest;\"" << std::endl;
	wcerr << L"  to run the tests using a connection string, against a DB which uses uppercase." << std::endl;
}

int _tmain(int argc, _TCHAR* argv[])
{
	using namespace exodbctest;
	using namespace std;
	using namespace exodbc;
	namespace ba = boost::algorithm;

	// If --help is passed, just print usage and exit
	// IF --logLevelX is set, set the log-level early
	for (int i = 0; i < argc; i++)
	{
		std::wcout << argv[i] << std::endl;
		if (ba::iequals(argv[i], L"--help"))
		{
			printHelp();
			return -1;
		}
		else if (ba::iequals(argv[i], L"--logLevelE"))
		{
			g_logManager.SetGlobalLogLevel(LogLevel::Error);
		}
		else if (ba::iequals(argv[i], L"--logLevelW"))
		{
			g_logManager.SetGlobalLogLevel(LogLevel::Warning);
		}
		else if (ba::iequals(argv[i], L"--logLevelI"))
		{
			g_logManager.SetGlobalLogLevel(LogLevel::Info);
		}
		else if (ba::iequals(argv[i], L"--logLevelD"))
		{
			g_logManager.SetGlobalLogLevel(LogLevel::Debug);
		}
	}

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	int status = 0;

	// Set defaults
	bool doCreateDb = false;

	// Iterate given options
	for (int i = 1; i < argc; i++)
	{
		std::wstring createDb = L"--createDb";
		std::wstring dsn = L"dsn=";
		std::wstring DSN = L"DSN=";
		std::wstring cs = L"cs=";
		std::wstring CS = L"CS=";
		std::wstring arg(argv[i]);
		std::wstring upperDsn, lowerDsn, upperCs, lowerCs;
		if (arg == createDb)
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
				Case nameCase = upperDsn.length() > 0 ? Case::UPPER : Case::LOWER;
				TestParams dsnEntry(tokens[0], tokens[1], tokens[2], nameCase);
				g_odbcInfo = dsnEntry;
			}
		}
		if (upperCs.length() > 0 || lowerCs.length() > 0)
		{
			Case nameCase = upperCs.length() > 0 ? Case::UPPER : Case::LOWER;
			TestParams csEntry(upperCs.length() > 0 ? upperCs : lowerCs, nameCase);
			g_odbcInfo = csEntry;
		}
	}
	if (g_odbcInfo.IsUsable())
	{
		g_odbcInfo.m_createDb = doCreateDb;
	}

	wstring customFilter;
	if (!g_odbcInfo.IsUsable())
	{
		// Read default settings
		LOG_INFO(L"No Dsn or Connection-String has been passed, trying to load settings from TestSettings.xml...");
		try
		{
			// Try to locate TestSettings.xml in directory of exe
			namespace fs = boost::filesystem;
			fs::wpath exePath(fs::current_path());
			exePath.normalize();
			fs::wpath confDir = exePath.parent_path();
			LOG_INFO(boost::str(boost::wformat(L"Searching for file TestSettings.xml in directory %1% and its parent directories.") % confDir));
			fs::wpath settingsPath = confDir / L"TestSettings.xml";
			while( ! fs::exists(settingsPath) && confDir.has_parent_path())
			{ 
				// try if it is up somewhere in my working copy
				confDir = confDir.parent_path();
				settingsPath = confDir;
				settingsPath /= L"TestSettings.xml";
				if(!fs::exists(settingsPath))
				{ 
					settingsPath = confDir;
					settingsPath /= L"exOdbcGTest/TestSettings.xml";
				}
			}
			if (!fs::exists(settingsPath))
			{
				NotFoundException ex(boost::str(boost::wformat(L"No TestSettings.xml file found in directory %1% and its parent directories") % exePath.parent_path()));
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			LOG_INFO(boost::str(boost::wformat(L"Using TestSettings.xml from %1%") % settingsPath));
			vector<wstring> skipNames;
			g_odbcInfo.Load(settingsPath, skipNames);
			// Set a gtest-filter statement to skip those names if no gtest-filter arg is setted
			bool haveFilter = false;
			for (int i = 1; i < argc; i++)
			{
				wstring arg = argv[i];
				if (arg.compare(0, 15, L"--gtest_filter=") == 0)
				{
					haveFilter = true;
					break;
				}
			}
			if (!haveFilter && !skipNames.empty())
			{
				// Add our default-filter
				wstring filter = L"--gtest_filter=-" + skipNames[0];
				size_t i = 1;
				while (i < skipNames.size())
				{
					filter += L":" + skipNames[i];
					++i;
				}
				customFilter = filter;
			}

			doCreateDb = g_odbcInfo.m_createDb;
		}
		catch (const Exception& ex)
		{
			LOG_ERROR(ex.ToString());
			return -1;
		}
	}
	
	wstringstream ws;
	ws << L"Running tests against: " << g_odbcInfo;
	LOG_INFO(ws.str());

	// Check if we need to re-create the dbs
	if (doCreateDb)
	{
		try
		{
			namespace fs = boost::filesystem;
			// Prepare Db-creator
			TestDbCreator creator(g_odbcInfo);
			fs::wpath exeDir(fs::current_path());
			fs::wpath scriptDir = exeDir / L"CreateScripts" / DatabaseProcudt2s(creator.GetDbms());
			if (!fs::is_directory(scriptDir))
			{
				THROW_WITH_SOURCE(Exception, boost::str(boost::wformat(L"ScriptDirectory '%s' is not a directory") % scriptDir.native()));
			}
			LOG_INFO(boost::str(boost::wformat(L"Running scripts from directory %s%") % scriptDir.native()));
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

	// Pass our maybe modified arguments to google-test
	int gTestArgc = argc;
	_TCHAR** gTestArgv = NULL;
	if (!customFilter.empty())
	{
		gTestArgc = argc + 1;
	}
	gTestArgv = new _TCHAR*[gTestArgc + 1];
	gTestArgv[gTestArgc] = NULL;
	for (int i = 0; i < argc; i++)
	{
		gTestArgv[i] = argv[i];
	}
	if (!customFilter.empty())
	{
		gTestArgv[gTestArgc - 1] = (_TCHAR*) customFilter.c_str();
	}

	// Note: We cannot call Init earlier, we must call it after we've set up the global with the param-values
	::testing::InitGoogleTest(&gTestArgc, gTestArgv);

	if (status != 0)
	{
		printHelp();
		return status;
	}

	int result = 0;
	try
	{
		result = RUN_ALL_TESTS();
	}
	catch (const Exception& ex)
	{
		result = -13;
		LOG_ERROR(L"TEST RUN ABORTED");
		LOG_ERROR(ex.ToString());
	}
	
	delete[] gTestArgv;

	return result;
}



