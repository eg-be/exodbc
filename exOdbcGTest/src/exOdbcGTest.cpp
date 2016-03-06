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
	using namespace exodbctest;

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

	wstring customFilter;
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



