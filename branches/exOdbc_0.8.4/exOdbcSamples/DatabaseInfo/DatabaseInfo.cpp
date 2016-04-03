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


void PrintDbHeader(ConstDatabasePtr pDb)
{
	DatabaseInfo dbInfo = pDb->GetDbInfo();

	// print header for trac
	wcout << boost::str(boost::wformat(L"== %s (%s) ==") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DbmsName) % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DbmsVersion)) << endl;
	//wcout << endl;
}


void PrintDriverInfo(ConstDatabasePtr pDb)
{
	DatabaseInfo dbInfo = pDb->GetDbInfo();

	// print header for trac
	wcout << boost::str(boost::wformat(L"=== Driver Info ===")) << endl;
	wcout << boost::str(boost::wformat(L"* Driver Name: %s") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DriverName)) << endl;
	wcout << boost::str(boost::wformat(L"* Driver Version: %s") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DriverVersion)) << endl;
	wcout << boost::str(boost::wformat(L"* Driver ODBC Version: %s") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DriverOdbcVersion)) << endl;

	//wcout << endl;
}


void PrintDbInfo(ConstDatabasePtr pDb)
{
	DatabaseInfo dbInfo = pDb->GetDbInfo();
	
	// And a table with all information
	wcout << L"=== Database Info ===" << endl;
	wcout << L"||=Property Name =||= Property Value =||" << endl;
	DatabaseInfo::WStringMap wstringMap = dbInfo.GetWstringMap();
	for (auto it = wstringMap.begin(); it != wstringMap.end(); ++it)
	{
		wcout << boost::str(boost::wformat(L"||%-38s ||  %s  ||") % dbInfo.GetPropertyName(it->first) % it->second) << endl;
	}
	DatabaseInfo::USmallIntMap usmallIntMap = dbInfo.GetUSmallIntMap();
	for (auto it = usmallIntMap.begin(); it != usmallIntMap.end(); ++it)
	{
		wcout << boost::str(boost::wformat(L"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second % it->second) << endl;
	}
	DatabaseInfo::UIntMap uintMap = dbInfo.GetUIntMap();
	for (auto it = uintMap.begin(); it != uintMap.end(); ++it)
	{
		wcout << boost::str(boost::wformat(L"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second % it->second) << endl;
	}
	DatabaseInfo::IntMap intMap = dbInfo.GetIntMap();
	for (auto it = intMap.begin(); it != intMap.end(); ++it)
	{
		wcout << boost::str(boost::wformat(L"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second %it->second) << endl;
	}
	//wcout << endl;
}


void printDatatypesInfo(ConstDatabasePtr pDb)
{
	wcout << L"=== Datatypes Info ===" << endl;
	SqlTypeInfosVector types = pDb->GetTypeInfos();
	bool first = true;
	for (auto it = types.begin(); it != types.end(); ++it)
	{
		SSqlTypeInfo typeInfo = *it;
		wcout << typeInfo.ToOneLineStrForTrac(first) << endl;
		first = false;
	}
	//wcout << endl;
}

struct SConnectionInfo
{
	wstring m_dsn;
	wstring m_user;
	wstring m_pass;
	wstring m_cs;
};

int _tmain(int argc, _TCHAR* argv[])
{
	try
	{
		// Parse arguments
		//wstring connectionString;
		//wstring dsn, user, pass;
		vector<SConnectionInfo> connectionInfos;
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
				SConnectionInfo conInfo;
				conInfo.m_cs = arg.substr(csKey.length());
				connectionInfos.push_back(conInfo);
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
					SConnectionInfo conInfo;
					conInfo.m_dsn = tokens[0];
					conInfo.m_user = tokens[1];
					conInfo.m_pass = tokens[2];
					connectionInfos.push_back(conInfo);
				}
			}
		}

		if (connectionInfos.empty())
		{
			LOG_ERROR(L"No connection string given and no dsn given, exiting");
			return -1;
		}

		// Create an environment with ODBC Version 3.0
		g_logManager.SetGlobalLogLevel(LogLevel::Error);
		EnvironmentPtr pEnv = Environment::Create(OdbcVersion::V_3);

		// And connect to a database using the environment,
		// read info for every connection
		for (auto it = connectionInfos.begin(); it != connectionInfos.end(); ++it)
		{
			SConnectionInfo conInf = *it;
			DatabasePtr pDb = Database::Create(pEnv);
			if (!conInf.m_cs.empty())
			{
				pDb->Open(conInf.m_cs);
			}
			else
			{
				pDb->Open(conInf.m_dsn, conInf.m_user, conInf.m_pass);
			}

			// Print some info
			PrintDbHeader(pDb);
			PrintDriverInfo(pDb);
			PrintDbInfo(pDb);
			printDatatypesInfo(pDb);

			// add some blank lines
			wcout << endl;
			wcout << endl;
		}

	}
	catch (const Exception& ex)
	{
		std::wcerr << ex.ToString() << endl;
	}
    return 0;
}

