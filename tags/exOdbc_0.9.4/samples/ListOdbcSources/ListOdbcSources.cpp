/*!
* \file ListOdbcSources.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 26.03.2017
* \copyright GNU Lesser General Public License Version 3
*
* ListOdbcSources Sample.
*/

#ifdef _WIN32
#include <SDKDDKVer.h>
#include <tchar.h>
#endif


// And the rest
#include "exodbc/exOdbc.h"
#include "exodbc/Exception.h"
#include "exodbc/LogManager.h"
#include "exodbc/Environment.h"

#include "boost/format.hpp"

using namespace exodbc;
using namespace std;

namespace ba = boost::algorithm;

void printUsage()
{
	WRITE_STDOUT_ENDL(u8"Usage: listodbcsources  [OPTION]...");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL(u8"Search for ODBC DSN entries and prints found entries to stdout.");
	WRITE_STDOUT_ENDL(u8"Searches for ODBC DSN entries of type 'All' as default.");
	WRITE_STDOUT_ENDL("OPTION can be:");
	WRITE_STDOUT_ENDL(u8" --systemOnly            If passed, only System DSN entries are searched.");
	WRITE_STDOUT_ENDL(u8" --userOnly              If passed, only User DSN entries are searched.");
	WRITE_STDOUT_ENDL(u8" --odbcVersion <version> Set ODBC Version to use. Valid values are '2', '3'");
	WRITE_STDOUT_ENDL(u8"                         or '3.8'. Default is '3'.");
	WRITE_STDOUT_ENDL(u8" --showArch              If set, tests if sizeof(void*) is 4 or 8 and prints");
	WRITE_STDOUT_ENDL(u8"                         the result as 'x86' for 4 and 'x64' for 8.");
}

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	try
	{
		// Parse arguments
		const std::string helpKey = u8"-help";
		const std::string helpKey2 = u8"--help";
		const std::string systemOnlyKey = u8"--systemOnly";
		const std::string userOnlyKey = u8"--userOnly";
		const std::string odbcVersionKey = u8"--odbcVersion";
		const std::string showArchKey = u8"--showArch";
		bool systemOnly = false;
		bool userOnly = false;
		bool showArch = false;
		OdbcVersion odbcVersionValue = OdbcVersion::V_3;
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
			if (ba::equals(arg, helpKey) || ba::equals(arg, helpKey2))
			{
				printUsage();
				return 0;
			}
			if (ba::equals(arg, systemOnlyKey))
			{
				systemOnly = true;
			}
			if (ba::equals(arg, userOnlyKey))
			{
				userOnly = true;
			}
			if (ba::equals(arg, showArchKey))
			{
				showArch = true;
			}
			if (ba::equals(arg, odbcVersionKey) && i + 1 < argc)
			{
				std::string odbcVersionStringValue = argNext;
				if (ba::iequals(u8"2", odbcVersionStringValue))
					odbcVersionValue = OdbcVersion::V_2;
				else if (ba::iequals(u8"3", odbcVersionStringValue))
					odbcVersionValue = OdbcVersion::V_3;
				else if (ba::iequals(u8"3.8", odbcVersionStringValue))
					odbcVersionValue = OdbcVersion::V_3_8;
				else
				{
					WRITE_STDERR_ENDL(boost::str(boost::format(u8"Unknown OdvcVersion '%s'") % odbcVersionStringValue));
					return 1;
				}
			}
		}

		if (systemOnly && userOnly)
		{
			WRITE_STDERR_ENDL(boost::str(boost::format(u8"Setting '%s' and '%s' will result in no data sources found") % systemOnlyKey % userOnlyKey));
			return 1;
		}

		if (showArch)
		{
			size_t s = sizeof(void*);
			if (s == 4)
			{
				WRITE_STDOUT_ENDL(boost::str(boost::format(u8"sizeof(void*) is %d, indicating 'x86'") % s));
			}
			else if (s == 8)
			{
				WRITE_STDOUT_ENDL(boost::str(boost::format(u8"sizeof(void*) is %d, indicating 'x64'") % s));
			}
			else
			{
				WRITE_STDOUT_ENDL(boost::str(boost::format(u8"sizeof(void*) is %d, indicating unknown architecture.") % s));
			}
		}

		Environment env(odbcVersionValue);
		Environment::DataSourceVector sources;
		string sourcesTypeName;
		if (systemOnly)
		{
			sources = env.ListDataSources(Environment::ListMode::System);
			sourcesTypeName = u8"System";
		}
		else if (userOnly)
		{
			sources = env.ListDataSources(Environment::ListMode::User);
			sourcesTypeName = u8"User";
		}
		else
		{
			sources = env.ListDataSources(Environment::ListMode::All);
			sourcesTypeName = u8"All";
		}
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"Found %d DSN entries of type '%s':") % sources.size() % sourcesTypeName ));
		WRITE_STDOUT_ENDL(u8"");
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8" %-20s %s") % u8"Name" % u8"Description"));
		WRITE_STDOUT_ENDL(u8"===================================================================");
		for (Environment::DataSourceVector::const_iterator it = sources.begin(); it != sources.end(); ++it)
		{
			WRITE_STDOUT_ENDL(boost::str(boost::format(u8" %-20s %s") % it->m_dsn % it->m_description));
		}
	}
	catch (const Exception& ex)
	{
		WRITE_STDERR_ENDL(ex.ToString());
	}
	return 0;
}

