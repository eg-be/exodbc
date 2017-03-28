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

	WRITE_STDOUT_ENDL(u8"Usage: exodbctest [OPTION]... [-DSN <dsn> [-U <user>] [-P <pass>] | -CS <connectionString>]");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL(u8"Run unit-tests against a database.");
	WRITE_STDOUT_ENDL(u8"Requires a Data Source Name (DSN) or Connection String (CS).");
	WRITE_STDOUT_ENDL(u8"A DSN or a CS can be passed as arguments. If no argument '-DSN' or '-CS' is");
	WRITE_STDOUT_ENDL(u8"passed, the executable directory is searched for a file 'TestSettings.xml'.");
	WRITE_STDOUT_ENDL(u8"If no -argument '-DSN' or '-CS' is passed and file 'TestSettings.xml' is not");
	WRITE_STDOUT_ENDL(u8"found test execution fails.");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL(u8"To connect using a DSN use the arguments:");
	WRITE_STDOUT_ENDL(u8" -DSN      <dsn>    Data Source Name");
	WRITE_STDOUT_ENDL(u8" -U        <user>   Username. Optional.");
	WRITE_STDOUT_ENDL(u8" -P        <pass>   Password. Optional.");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL(u8"To connect using a  CS use the argument:");
	WRITE_STDOUT_ENDL(u8" -CS       <cs>     Connection String");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL("OPTION can be:");
	WRITE_STDOUT_ENDL(" --createDb           Run the scripts to create the test database. exodbctest");
	WRITE_STDOUT_ENDL("                      will connect to the database and try to detect the");
	WRITE_STDOUT_ENDL("                      database system. If there is a subdirectory matching the");
	WRITE_STDOUT_ENDL("                      database system name in the 'CreateScripts' directory,");
	WRITE_STDOUT_ENDL("                      all scripts inside that directory are run against the");
	WRITE_STDOUT_ENDL("                      database. If creation is successfully, exodbctest runs the");
	WRITE_STDOUT_ENDL(" --createDbOnly       Only do the '--createDb' task then exit without running");
	WRITE_STDOUT_ENDL("                      any tests.");
	WRITE_STDOUT_ENDL(u8"                    tests afterwards, else it exits with a non-zero status.");
	WRITE_STDOUT_ENDL(u8" --logLevel <level> Set the Log Level. <level> can be 'Debug', Info',");
	WRITE_STDOUT_ENDL(u8"                    'Warning' or 'Error'. Default is 'Info'.");
	WRITE_STDOUT_ENDL(u8" --logFile  <file>  Log additionally to a file named <file>. <file> can be");
	WRITE_STDOUT_ENDL(u8"                    relative to exodbctest or absolute.");
	WRITE_STDOUT_ENDL(u8" --case     <u|l>   Specify whether table and column names are in lower or");
	WRITE_STDOUT_ENDL(u8"                    upper case letters. Must be either 'u' for uppercase or");
	WRITE_STDOUT_ENDL(u8"                    'l' for lowercase. Default is 'l'.");
	WRITE_STDOUT_ENDL(u8" -help              Show this text.");
	WRITE_STDOUT_ENDL(u8" --help             Show this text and the help for googletest.");
	WRITE_STDOUT_ENDL(u8"");
}


#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
#ifdef _DEBUG
	// Leaks are being reported, always the same size, like
	/*
	Detected memory leaks!
	Dumping objects ->
	{8868750} normal block at 0x01137B28, 8 bytes long.
	Data: < z      > F0 7A 13 01 00 00 00 00
	{8868749} normal block at 0x010973E0, 32 bytes long.
	Data: < s   s   s      > E0 73 09 01 E0 73 09 01 E0 73 09 01 01 01 CD CD
	{8868748} normal block at 0x01137AF0, 12 bytes long.
	Data: <({   s      > 28 7B 13 01 E0 73 09 01 00 00 00 00
	{8868747} normal block at 0x010A8628, 24 bytes long.
	Data: <                > FF FF FF FF FF FF FF FF 00 00 00 00 00 00 00 00
	Object dump complete.
	*/
	// I have no clue if the report is wrong or where the leak
	// should come from.. see #260
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(9554);
	//_CrtSetBreakAlloc(9553);
	//_CrtSetBreakAlloc(9552);
	//_CrtSetBreakAlloc(9609);

	// its even enough to add a return call here and a leak with the 
	// same size will be reported, whats wrong?
	// return 10;
#endif

	using namespace exodbctest;
	using namespace std;
	using namespace exodbc;
	namespace ba = boost::algorithm;

	bool haveConsoleLogLevel = false;

	// If --help is passed, just print usage and exit
	// If --logLevel is set, set the log-level early
	const std::string userKey = u8"-U";
	const std::string passKey = u8"-P";
	const std::string dsnKey = u8"-DSN";
	const std::string csKey = u8"-CS";
	const std::string createDbKey = u8"--createDb";
	const std::string createDbOnlyKey = u8"--createDbOnly";
	const std::string logLevelKey = u8"--logLevel";
	const std::string logFileKey = u8"--logFile";
	const std::string caseKey = u8"--case";
	const std::string helpKey = u8"-help";
	const std::string helpHelpKey = u8"--help";
	std::string userValue, passValue, dsnValue;
	std::string csValue;
	bool createDbValue = false;
	bool createDbOnlyValue = false;
	LogLevel logLevelValue = LogLevel::Info;
	Case caseValue = Case::LOWER;

	for (int i = 0; i < argc; i++)
	{
		std::string argNext;
#ifdef _WIN32
		std::string arg(utf16ToUtf8(argv[i]));
		if (i + 1 < argc)
			argNext = utf16ToUtf8(argv[i + 1]);
#else
		std::string arg(argv[i]);
		if (i + 1 < argc)
			argNext = argv[i + 1];
#endif
		if (ba::equals(arg, helpKey))
		{
			printHelp();
			return 0;
		}
		if (ba::equals(arg, helpHelpKey))
		{
			printHelp();
			// And also print googletest help
			::testing::InitGoogleTest(&argc, argv);
			return 0;
		}
		if (ba::equals(arg, userKey) && i + 1 < argc)
		{
			userValue = argNext;
		}
		if (ba::equals(arg, passKey) && i + 1 < argc)
		{
			passValue = argNext;
		}
		if (ba::equals(arg, dsnKey) && i + 1 < argc)
		{
			dsnValue = argNext;
		}
		if (ba::equals(arg, csKey) && i + 1 < argc)
		{
			csValue = argNext;
		}
		if (ba::equals(arg, logFileKey) && i + 1 < argc)
		{
			FileLogHandlerPtr pFileLogger = std::make_shared<FileLogHandler>(argNext, true);
			LogManager::Get().RegisterLogHandler(pFileLogger);
		}
		if (ba::equals(arg, logLevelKey) && i + 1 < argc)
		{
			string logLevelString = argNext;
			if (ba::iequals(logLevelString, u8"Debug"))
				LogManager::Get().SetGlobalLogLevel(LogLevel::Debug);
			else if (ba::iequals(logLevelString, u8"Info"))
				LogManager::Get().SetGlobalLogLevel(LogLevel::Info);
			else if (ba::iequals(logLevelString, u8"Warning"))
				LogManager::Get().SetGlobalLogLevel(LogLevel::Warning);
			else if (ba::iequals(logLevelString, u8"Error"))
				LogManager::Get().SetGlobalLogLevel(LogLevel::Error);
			else
			{
				LOG_ERROR(boost::str(boost::format(u8"Unknown Log Level '%s'") % logLevelString));
				return 1;
			}
		}
		if (ba::equals(arg, caseKey) && i + 1 < argc)
		{
			string caseKeyString = argNext;
			if (ba::iequals(caseKeyString, u8"l"))
				caseValue = Case::LOWER;
			else if (ba::iequals(caseKeyString, u8"u"))
				caseValue = Case::UPPER;
			else
			{
				LOG_ERROR(boost::str(boost::format(u8"Unknown Case '%s'") % caseKeyString));
				return 1;
			}
		}
		if (ba::equals(arg, createDbKey))
		{
			createDbValue = true;
		}
		if (ba::equals(arg, createDbOnlyKey))
		{
			createDbOnlyValue = true;
		}
	}

	if (!dsnValue.empty() && !csValue.empty())
	{
		LOG_ERROR(u8"Muest use either '-CS' or '-DSN [-U <user>] [-P <pass>]', cannot use both");
		return 1;
	}

	string customFilter;
	if (!dsnValue.empty() || !csValue.empty())
	{
		if (!dsnValue.empty())
		{
			TestParams dsnEntry(dsnValue, userValue, passValue, caseValue);
			g_odbcInfo = dsnEntry;
		}
		if (!csValue.empty())
		{
			TestParams csEntry(csValue, caseValue);
			g_odbcInfo = csEntry;
		}
		g_odbcInfo.m_createDb = createDbValue;
	}
	else
	{
		// Read default settings
		LOG_INFO(u8"No argument '-DSN' or -CS' passed, loading settings from TestSettings.xml");
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
			while (!fs::exists(settingsPath) && confDir.has_parent_path())
			{
				// try if it is up somewhere in my working copy
				confDir = confDir.parent_path();
				settingsPath = confDir;
				settingsPath /= u8"TestSettings.xml";
				if (!fs::exists(settingsPath))
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
			// Remember a log-level set on the console
			LogLevel logLevel = LogManager::Get().GetGlobalLogLevel();
			g_odbcInfo.Load(settingsPath, skipNames);
			if (haveConsoleLogLevel)
			{
				LogManager::Get().SetGlobalLogLevel(logLevel);
			}
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
			if (!createDbValue)
			{
				createDbValue = g_odbcInfo.m_createDb;
			}
			else
			{
				g_odbcInfo.m_createDb = createDbValue;
			}
		}
		catch (const Exception& ex)
		{
			LOG_ERROR(ex.ToString());
			return 10;
		}
	}
	
	stringstream ss;
	ss << u8"Using connection information: " << g_odbcInfo;
	LOG_INFO(ss.str());

	// Check if we need to re-create the dbs
	if (createDbValue || createDbOnlyValue)
	{
		try
		{
			namespace fs = boost::filesystem;
			// Prepare Db-creator
			TestDbCreator creator(g_odbcInfo);
			fs::wpath exePath(argv[0]);
			LOG_DEBUG(boost::str(boost::format(u8"Using exePath '%1%' as basepath to search for ScriptDirectory") % exePath));
			fs::wpath exeDir = exePath.parent_path();
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
			return 20;
		}
		if (createDbOnlyValue)
		{
			return 0;
		}
	}

	// Pass our maybe modified arguments to google-test
	int gTestArgc = argc;
#ifdef _WIN32
	_TCHAR** gTestArgv = NULL;
#else
	char** gTestArgv = NULL;
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
#ifdef _WIN32
		wstring wa = gTestArgv[i];
		string sa = utf16ToUtf8(wa);
#else
		string sa = gTestArgv[i];        
#endif
		string msg = boost::str(boost::format(u8"%d\t%s") % i % sa);
		LOG_INFO(msg);
	}

	// Note: We cannot call Init earlier, we must call it after we've set up the global with the param-values
	::testing::InitGoogleTest(&gTestArgc, gTestArgv);

	int result = 0;
	try
	{
		result = RUN_ALL_TESTS();
		LOG_INFO(u8"Test run finished");
	}
	catch (const Exception& ex)
	{
		result = 100;
		LOG_ERROR(u8"TEST RUN ABORTED");
		LOG_ERROR(ex.ToString());
	}
	
	delete[] gTestArgv;

	return result;
}
