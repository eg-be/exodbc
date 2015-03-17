/*!
 * \file wxOdbc3GoogleTest.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
 * \copyright wxWindows Library Licence, Version 3.1
 * 
 * Defines the entry point for the console (test-)application.
 */ 

#include "stdafx.h"

// Own header
#include "exOdbcGTest.h"

// Same component headers
// Other headers
#include "boost/log/trivial.hpp"
#include "boost/log/core.hpp"
#include "boost/log/expressions.hpp"

// Debug
#include "DebugNew.h"

// Globals
// -------
namespace exodbc
{
	std::wstring g_excelDsn = L"";
	std::vector<SOdbcInfo> g_odbcInfos = std::vector<SOdbcInfo>();
	boost::log::trivial::severity_level g_logSeverity = boost::log::trivial::error;
}

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

int _tmain(int argc, _TCHAR* argv[])
{
	using namespace std;
	using namespace exodbc;

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	int status = 0;
	if(argc < 4)
	{
		// Not enough args
		wcerr << L"Not enough arguments!\n";
		wcerr << L"Usage: wxOdbc3GoogleTest --dsn=dsn1,dsn2,..,dsnN    --user=user1,user2,..,userN --pass=pass1,pass2,..,passN [--case=u|l,u|l,...,u|l] [--logLevel=0-5] [--excelDsn=excelDSNName]\n";
		wcerr << L" logLevel: 0: trace; 1: debug; 2: info; 3: warning; 4: error; 5: fatal\n";
		wcerr << L"           Default is warning (3)\n";
		wcerr << L" case: Defines the case of the table- and column-names to be used during the tests. If not given\n";
		wcerr << L"       lowercase-letters are used. If parameter is used, it must be set for every dsn entry\n";
		wcerr << L"             l: lower: All table- and column-names for the test tables will be in lowercase-letters (default)\n";
		wcerr << L"             u: upper: All table- and column-names for the test tables will be in UPPERCASE-letters\n";
		status = 10;
	}
	std::wstring dsn, user, pass, logLevelS, caseS, excelDsn;
	bool haveCase = false;
	if(!extractParamValue(argc, argv, L"--dsn=", dsn))
	{
		wcerr << L"Missing argument --dsn=\n";
		status = 11;
	}
	if(!extractParamValue(argc, argv, L"--user=", user))
	{
		wcerr << L"Missing argument --user=\n";
		status = 11;
	}
	if(!extractParamValue(argc, argv, L"--pass=", pass))
	{
		wcerr << L"Missing argument --pass=\n";
		status = 11;
	}
	int logLevel = 3;
	if (extractParamValue(argc, argv, L"--logLevel=", logLevelS))
	{
		std::wistringstream convert(logLevelS);
		if (!(convert >> logLevel) || logLevel < 0 || logLevel > 5)
		{
			wcout << L"Warning: LogLevel is not a value in the range 0-5. Falling back to default 'warning' (3)\n";
			logLevel = 3;
		}
	}
	haveCase = extractParamValue(argc, argv, L"--case=", caseS);
	vector<wstring> dsns, users, passes, cases;
	boost::split(dsns, dsn, boost::is_any_of(L","));
	boost::split(users, user, boost::is_any_of(L","));
	boost::split(passes, pass, boost::is_any_of(L","));
	if (haveCase)
	{
		boost::split(cases, caseS, boost::is_any_of(L","));
	}
	if(! (dsns.size() == users.size() && dsns.size() == passes.size() && ( !haveCase || dsns.size() == cases.size()) ))
	{
		std::wcerr << L"Not equal number of dsn, user and password (and maybe case)\n";
		status = 12;
	}
	// default to lowerCase with cases
	if (!haveCase)
	{
		for (size_t i = 0; i < dsns.size(); i++)
		{
			cases.push_back(L"l");
		}
	}
	for(size_t i = 0; status == 0 && i < dsns.size(); i++)
	{
		TestTables::NameCase nameCase = TestTables::NC_LOWER;
		if (cases[i] == L"u")
			nameCase = TestTables::NC_UPPER;
		else if (cases[i] != L"l")
			wcout << L"Warning: Unknown case '" << cases[i] << L"' falling back to default of 'l' (lowercase)\n";

		g_odbcInfos.push_back(SOdbcInfo(dsns[i], users[i], passes[i], cases[i] == L"l" ? TestTables::NC_LOWER : TestTables::NC_UPPER));
	}
	// Read an eventually set excel Dsn
	if (extractParamValue(argc, argv, L"--excelDsn=", excelDsn))
	{
		::boost::algorithm::trim(excelDsn);
		g_excelDsn = excelDsn;
	}

	// Note: We cannot call Init earlier, we must call it after we've set up the global with the param-values
	::testing::InitGoogleTest(&argc, argv);

	if(status != 0)
		return status;
	
	// Set a filter for the logging
	// Also update the global value
	g_logSeverity = (boost::log::trivial::severity_level) logLevel;
	boost::log::core::get()->set_filter
		(
		boost::log::trivial::severity >= boost::ref(g_logSeverity)
		);

	int result = RUN_ALL_TESTS();
	std::wcerr << L"Any key to exit";
	getchar();
	
	return result;
}

// Interfaces
// ----------

