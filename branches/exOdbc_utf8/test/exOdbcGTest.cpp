/*!
 * \file exOdbcGTest.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
 * 
 * Defines the entry point for the console (test-)application.
 */ 

// Own header
#include "exOdbcGTest.h"

// Same component headers
#include "TestDbCreator.h"

// Other headers
#include "boost/filesystem.hpp"
#include "exodbc/LogHandler.h"
#ifdef _WIN32
    #include <tchar.h>
#endif

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
void printHelp()
{
	using namespace std;

	cerr << u8"Usage: exOdbcGTest [OPTION]... [DATABASE]" << std::endl;
	cerr << u8"Run unit-tests against DATABASE or read configuration from TestSettings.xml." << std::endl;
	cerr << u8"If DATABASE is not specified, all settings will be read from the file" << std::endl;
	cerr << u8"TestSettings.xml, located in either the directory of the app, or in a" << std::endl;
	cerr << u8"subdirectory named 'exOdbcGTest'. If the file is not found in the directory" << std::endl;
	cerr << u8"of the app, all parent directories are searched the same way." << std::endl;
	cerr << std::endl;
	cerr << u8"If DATABASE is specified, you can either supply a connection-string or a DSN-entry" << std::endl;
	cerr << u8"to indicate the connection info of the test database:" << std::endl;
	cerr << u8" To connect using a configured DSN entry use:" << std::endl;
	cerr << u8"" << std::endl;
	cerr << u8"  DSN=dsn;user;pass (to connect to a database with UPPER-case names (tables/columns))" << std::endl;
	cerr << u8"  dsn=dsn;user;pass (to connect to a database with lower-case names (tables/columns))" << std::endl;
	cerr << std::endl;
	cerr << u8" or to connect using a connection-string use:" << std::endl;
	cerr << std::endl;
	cerr << u8"  CS=connectionString (to connect to a database with UPPER-case names (tables/columns))" << std::endl;
	cerr << u8"  cs=connectionString (to connect to a database with lower-case names (tables/columns))" << std::endl;
	cerr << std::endl;
	cerr << u8"Note that you must frame the 'cs=connectionString' with \" if your connection" << std::endl;
	cerr << u8"string contains white spaces." << std::endl;
	cerr << std::endl;
	cerr << u8"OPTION can be:" << std::endl;
	cerr << u8" --createDb       Run the scripts to create the test database before running the tests." << std::endl;
	cerr << u8"                    The TestDbCreator will connect to the database and try to detect" << std::endl;
	cerr << u8"                    the database system. If there is a subdirectory matching the" << std::endl;
	cerr << u8"                    database system name in 'CreateScripts' directory, all scripts" << std::endl;
	cerr << u8"                    inside that directory are run." << std::endl;
	cerr << u8" --logLevelX      Set the log level, where X must be either"<< std::endl;
	cerr << u8"                    E, W, I or D for Error, Warning, Info or Debug." << std::endl;
	cerr << u8" --logFile        Log additionally to a file 'exODbcGTest.log' in the current directory" << std::endl;
	cerr << u8" --help           To show this help text." << std::endl;
	cerr << std::endl;
	cerr << u8"Examples:" << std::endl;
	cerr << u8" exodbcGTest --createDb --logLevelW DSN=db2;uid;pass" << std::endl;
	cerr << u8"  to run the tests against a configured DSN entry named 'db2', using uppercase" << std::endl;
	cerr << u8"  names and log level Waring. Before the tests are run, the scripts to create" << std::endl;
	cerr << u8"  the test database are run." << std::endl;
	cerr << u8" exodbcGTest CS=\"Driver={IBM DB2 ODBC DRIVER};Database=EXODBC;Hostname=192.168.56.20;Port=50000;Protocol=TCPIP;Uid=db2ex;Pwd=extest;\"" << std::endl;
	cerr << u8"  to run the tests using a connection string, against a DB which uses uppercase." << std::endl;
}

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	using namespace exodbctest;
	using namespace std;
	using namespace exodbc;
	namespace ba = boost::algorithm;

	// If --help is passed, just print usage and exit
	// IF --logLevelX is set, set the log-level early
	for (int i = 0; i < argc; i++)
	{
#ifdef _WIN32
		string arg = utf16ToUtf8(argv[i]);
#else
		string arg = argv[i];
#endif
		if (ba::iequals(arg, u8"--help"))
		{
			printHelp();
			return -1;
		}
		else if (ba::iequals(arg, u8"--logLevelE"))
		{
			LogManager::Get().SetGlobalLogLevel(LogLevel::Error);
		}
		else if (ba::iequals(arg, u8"--logLevelW"))
		{
			LogManager::Get().SetGlobalLogLevel(LogLevel::Warning);
		}
		else if (ba::iequals(arg, u8"--logLevelI"))
		{
			LogManager::Get().SetGlobalLogLevel(LogLevel::Info);
		}
		else if (ba::iequals(arg, u8"--logLevelD"))
		{
			LogManager::Get().SetGlobalLogLevel(LogLevel::Debug);
		}
		else if (ba::iequals(arg, u8"--logFile"))
		{
			FileLogHandlerPtr pFileLogger = std::make_shared<FileLogHandler>(u8"exOdbcGTest.log", true);
			LogManager::Get().RegisterLogHandler(pFileLogger);
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
		std::string createDb = u8"--createDb";
		std::string dsn = u8"dsn=";
		std::string DSN = u8"DSN=";
		std::string cs = u8"cs=";
		std::string CS = u8"CS=";
#if _WIN32
		std::string arg = utf16ToUtf8(argv[i]);
#else
		std::string arg(argv[i]);
#endif
		std::string upperDsn, lowerDsn, upperCs, lowerCs;
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
			std::string dsn = upperDsn.length() > 0 ? upperDsn : lowerDsn;
			std::vector<std::string> tokens;
			boost::split(tokens, dsn, boost::is_any_of(u8";"));
			if (tokens.size() != 3)
			{
				LOG_WARNING(boost::str(boost::format(u8"Ignoring Dsn entry '%s' because it does not match the form 'dsn;user;pass'") % arg));
			}
			else if (tokens[0].empty())
			{
				LOG_WARNING(boost::str(boost::format(u8"Ignoring Dsn entry '%s' because DSN is empty.") % arg));
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

	string customFilter;
	if (!g_odbcInfo.IsUsable())
	{
		// Read default settings
		LOG_INFO(u8"No usable DSN or ConnectionString passed, loading settings from TestSettings.xml");
		try
		{
			// Try to locate TestSettings.xml in directory of exe
			namespace fs = boost::filesystem;
			fs::wpath exePath(argv[0]);
			exePath.normalize();
			fs::wpath confDir(fs::current_path());
			if (exePath.is_absolute())
			{
				confDir = exePath.parent_path();
			}
			LOG_INFO(boost::str(boost::format(u8"Searching in %1% and its parent directories for TestSettings.xml") % confDir));
			fs::wpath settingsPath = confDir / u8"TestSettings.xml";
			while( ! fs::exists(settingsPath) && confDir.has_parent_path())
			{ 
				// try if it is up somewhere in my working copy
				confDir = confDir.parent_path();
				settingsPath = confDir;
				settingsPath /= u8"TestSettings.xml";
				if(!fs::exists(settingsPath))
				{ 
					settingsPath = confDir;
					settingsPath /= u8"exOdbcGTest/TestSettings.xml";
				}
			}
			if (!fs::exists(settingsPath))
			{
				NotFoundException ex(boost::str(boost::format(u8"No TestSettings.xml file found")));
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			LOG_INFO(boost::str(boost::format(u8"Using settings from %1%") % settingsPath));
			vector<string> skipNames;
			g_odbcInfo.Load(settingsPath, skipNames);
			// Set a gtest-filter statement to skip those names if no gtest-filter arg is setted
			bool haveFilter = false;
			for (int i = 1; i < argc; i++)
			{
#ifdef _WIN32
				string arg = utf16ToUtf8(argv[i]);
#else
				string arg = argv[i];
#endif
				if (arg.compare(0, 15, u8"--gtest_filter=") == 0)
				{
					haveFilter = true;
					break;
				}
			}
			if (!haveFilter && !skipNames.empty())
			{
				// Add our default-filter
				string filter = u8"--gtest_filter=-" + skipNames[0];
				size_t i = 1;
				while (i < skipNames.size())
				{
					filter += u8":" + skipNames[i];
					++i;
				}
				customFilter = filter;
			}

			// if we have been requested to create the db from the command-line
			// let the command line win.
			if (!doCreateDb)
			{
				doCreateDb = g_odbcInfo.m_createDb;
			}
			else
			{
				g_odbcInfo.m_createDb = doCreateDb;
			}
		}
		catch (const Exception& ex)
		{
			LOG_ERROR(ex.ToString());
			return -1;
		}
	}
	
	stringstream ss;
	ss << u8"Running tests against: " << g_odbcInfo;
	LOG_INFO(ss.str());

	// Check if we need to re-create the dbs
	if (doCreateDb)
	{
		try
		{
			namespace fs = boost::filesystem;
			// Prepare Db-creator
			TestDbCreator creator(g_odbcInfo);
			fs::wpath exeDir(fs::current_path());
			fs::wpath scriptDir = exeDir / u8"CreateScripts" / DatabaseProcudt2s(creator.GetDbms());
			if (!fs::is_directory(scriptDir))
			{
				THROW_WITH_SOURCE(Exception, boost::str(boost::format(u8"ScriptDirectory '%1%' is not a directory") % scriptDir));
			}
			LOG_INFO(boost::str(boost::format(u8"Running scripts from directory %1%") % scriptDir));
			creator.SetScriptDirectory(scriptDir);
			creator.RunAllScripts();
		}
		catch (const Exception& ex)
		{
			LOG_ERROR(boost::str(boost::format(u8"Failed to create Test-Database for DSN '%s': %s") % g_odbcInfo.m_dsn % ex.ToString()));
			::getchar();
			return -1;
		}
	}

	// Pass our maybe modified arguments to google-test
	int gTestArgc = argc;
#ifdef _WIN32
	_TCHAR** gTestArgv = NULL;
#else
	char** gTestArgv = NULL
#endif
	if (!customFilter.empty())
	{
		gTestArgc = argc + 1;
	}
#ifdef _WIN32
	gTestArgv = new _TCHAR*[gTestArgc + 1];
#else
	gTestArgv = new char*[gTestArgc + 1];
#endif
	gTestArgv[gTestArgc] = NULL;
	for (int i = 0; i < argc; i++)
	{
		gTestArgv[i] = argv[i];
	}
	// create the wide version so it will still exist later.
	wstring wCustomFilter = utf8ToUtf16(customFilter);
	if (!customFilter.empty())
	{
#ifdef _WIN32
		gTestArgv[gTestArgc - 1] = (_TCHAR*) wCustomFilter.c_str();
#else
		gTestArgv[gTestArgc - 1] = (char*)customFilter.c_str();
#endif
	}

	LOG_INFO(boost::str(boost::format(u8"Passing the following %d arguments to InitGoogleTest:") % gTestArgc ));
	for (int i = 0; i < gTestArgc; i++)
	{
		wstring wa = gTestArgv[i];
		string sa = utf16ToUtf8(wa);
		string msg = boost::str(boost::format(u8"%d\t%s") % i % sa);
		LOG_INFO(msg);
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
		LOG_INFO(u8"Test run finished");
	}
	catch (const Exception& ex)
	{
		result = -13;
		LOG_ERROR(u8"TEST RUN ABORTED");
		LOG_ERROR(ex.ToString());
	}
	
	delete[] gTestArgv;

	return result;
}



