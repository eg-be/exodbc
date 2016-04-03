/*!
* \file ShortIntro.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 03.04.2016
* \copyright GNU Lesser General Public License Version 3
*
* ShortIntro Sample.
*/

#include <SDKDDKVer.h>
#include <iostream>
#include <tchar.h>

#include "exodbc/exOdbc.h"
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/InfoObject.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"

namespace ba = boost::algorithm;

using namespace exodbc;
using namespace std;

void printDbAndDriverInfo(ConstDatabasePtr pDb)
{
	DatabaseInfo dbInfo = pDb->GetDbInfo();
	
	// print header for trac
	std::wcout << boost::str(boost::wformat(L"== %s (%s) ==") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DbmsName) % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DbmsVersion) ) << std::endl;
	std::wcout << boost::str(boost::wformat(L"=== Driver Info ===")) << std::endl;
	std::wcout << boost::str(boost::wformat(L"* Driver Name: %s") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DriverName)) << std::endl;
	std::wcout << boost::str(boost::wformat(L"* Driver Version: %s") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DriverVersion)) << std::endl;
	std::wcout << boost::str(boost::wformat(L"* Driver ODBC Version: %s") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DriverOdbcVersion)) << std::endl;

	// And a table with all information
	std::wcout << L"=== Database Info ===" << std::endl;
	std::wcout << L"||=Property Name =||= Property Value =||" << std::endl;
	DatabaseInfo::WStringMap wstringMap = dbInfo.GetWstringMap();
	for (auto it = wstringMap.begin(); it != wstringMap.end(); ++it)
	{
		std::wcout << boost::str(boost::wformat(L"||%-38s ||  %s  ||") % dbInfo.GetPropertyName(it->first) % it->second) << std::endl;
	}
	DatabaseInfo::USmallIntMap usmallIntMap = dbInfo.GetUSmallIntMap();
	for (auto it = usmallIntMap.begin(); it != usmallIntMap.end(); ++it)
	{
		std::wcout << boost::str(boost::wformat(L"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second % it->second) << std::endl;
	}
	DatabaseInfo::UIntMap uintMap = dbInfo.GetUIntMap();
	for (auto it = uintMap.begin(); it != uintMap.end(); ++it)
	{
		std::wcout << boost::str(boost::wformat(L"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second % it->second) << std::endl;
	}
	DatabaseInfo::IntMap intMap = dbInfo.GetIntMap();
	for (auto it = intMap.begin(); it != intMap.end(); ++it)
	{
		std::wcout << boost::str(boost::wformat(L"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second %it->second) << std::endl;
	}
}


void printDatabaseInfo()
{

}


int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		// Parse arguments
		wstring connectionString;
		wstring dsn, user, pass;

		for (int i = 0; i < argc; i++)
		{
			std::wstring dsnKey = L"dsn=";
			std::wstring csKey = L"cs=";
			std::wstring arg(argv[i]);
			std::wstring dsnValue;
			if (ba::starts_with(arg, dsnKey) && arg.length() > dsnKey.length())
			{
				dsnValue = arg.substr(dsnKey.length());
			}
			else if (ba::starts_with(arg, csKey) && arg.length() > csKey.length())
			{
				connectionString = arg.substr(csKey.length());
			}
			if (dsnValue.length() > 0)
			{
				std::vector<std::wstring> tokens;
				boost::split(tokens, dsnValue, boost::is_any_of(L";"));
				if (tokens.size() != 3)
				{
					LOG_WARNING(boost::str(boost::wformat(L"Ignoring Dsn entry '%s' because it does not match the form 'dsnValue;user;pass'") % arg));
				}
				else if (tokens[0].empty())
				{
					LOG_WARNING(boost::str(boost::wformat(L"Ignoring Dsn entry '%s' because DSN is empty.") % arg));
				}
				else
				{
					dsn = tokens[0];
					user = tokens[1];
					pass = tokens[2];
				}
			}
		}

		if (connectionString.empty() && dsn.empty())
		{
			LOG_ERROR(L"No connection string given and no dsn given, exiting");
			return -1;
		}

		// Create an environment with ODBC Version 3.0
		EnvironmentPtr pEnv = Environment::Create(OdbcVersion::V_3);

		// And connect to a database using the environment.
		DatabasePtr pDb = Database::Create(pEnv);
		if (!connectionString.empty())
		{
			pDb->Open(connectionString);
		}
		else
		{
			pDb->Open(dsn, user, pass);
		}

		// Print some info
		printDbAndDriverInfo(pDb);

	}
	catch (const Exception& ex)
	{
		std::wcerr << ex.ToString() << std::endl;
	}
    return 0;
}

