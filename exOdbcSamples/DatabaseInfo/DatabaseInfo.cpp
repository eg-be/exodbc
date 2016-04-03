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
#include "exodbc/Table.h"
#include "exodbc/ExecutableStatement.h"
#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"

namespace ba = boost::algorithm;

int _tmain(int argc, _TCHAR* argv[])
{
	using namespace exodbc;
	using namespace std;

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


	}
	catch (const Exception& ex)
	{
		std::wcerr << ex.ToString() << std::endl;
	}
    return 0;
}

