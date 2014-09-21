/*!
 * \file wxOdbc3GoogleTest.cpp
 * \author Elias Gerber <egerber@gmx.net>
 * \date 22.07.2014
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

// Globals
// -------
namespace exodbc
{
	std::vector<SOdbcInfo> g_odbcInfos = std::vector<SOdbcInfo>();
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

	int status = 0;

	if(argc < 4)
	{
		// Not enough args
		wcerr << L"Not enough arguments!\n";
		wcerr << L"Usage: wxOdbc3GoogleTest --dsn=dsn1,dsn2,..,dsnN --user=user1,user2,..,userN --pass=pass1,pass2,..,passN\n";
		status = 10;
	}
	std::wstring dsn, user, pass;
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
	vector<wstring> dsns, users, passes;
	boost::split(dsns, dsn, boost::is_any_of(L","));
	boost::split(users, user, boost::is_any_of(L","));
	boost::split(passes, pass, boost::is_any_of(L","));
	if(! (dsns.size() == users.size() && dsns.size() == passes.size()))
	{
		std::wcerr << L"Not equal number of dsn, user and passwords\n";
		status = 12;
	}

	for(size_t i = 0; status == 0 && i < dsns.size(); i++)
	{
		g_odbcInfos.push_back(SOdbcInfo(dsns[i], users[i], passes[i]));
	}

	// Note: We cannot call Init earlier, we must call it after we've set up the global with the param-values
	::testing::InitGoogleTest(&argc, argv);

	if(status != 0)
		return status;
	
	// Set a filter for the logging
	boost::log::core::get()->set_filter
		(
#if _DEBUG
			boost::log::trivial::severity >= boost::log::trivial::debug
#else
			boost::log::trivial::severity >= boost::log::trivial::warning
#endif
		);

	int result = RUN_ALL_TESTS();
	std::wcerr << L"Any key to exit";
	getchar();
	
	return result;
}

// Interfaces
// ----------


