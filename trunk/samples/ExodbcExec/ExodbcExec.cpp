/*!
* \file OdbcExec.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 31.03.2017
* \copyright GNU Lesser General Public License Version 3
*
* OdbcExec tool. Execute SQL and print results of queries
* to the console.
*/

// Own header
#include "ExodbcExec.h"

// Same component headers
#include "CommandGenerator.h"

// Other headers
#ifdef _WIN32
	#include <SDKDDKVer.h>
	#include <tchar.h>
#endif
#include "exodbc/exOdbc.h"
#include "exodbc/LogManager.h"
#include "exodbc/Environment.h"
#include "exodbc/Database.h"

#include "boost/format.hpp"

// Debug
#include "DebugNew.h"


using namespace exodbc;
using namespace std;

namespace ba = boost::algorithm;

void printUsage()
{
	WRITE_STDOUT_ENDL(u8"Usage: exodbcexec  [OPTION]... [-DSN <dsn> [-U <user>] [-P <pass>] | -CS <connectionString>] ");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL(u8"Opens a connection to a database to execute SQL against the database.");
	WRITE_STDOUT_ENDL(u8"Requires a Data Source Name (DSN) or Connection String (CS) to connect.");
	WRITE_STDOUT_ENDL(u8"A DSN or a CS must be passed as arguments. If no argument '-DSN' or '-CS' is");
	WRITE_STDOUT_ENDL(u8"passed or invalid arguments are passed, 2 is returned.");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL(u8"To connect using a DSN use the arguments:");
	WRITE_STDOUT_ENDL(u8" -DSN      <dsn>         Data Source Name");
	WRITE_STDOUT_ENDL(u8" -U        <user>        Username. Optional.");
	WRITE_STDOUT_ENDL(u8" -P        <pass>        Password. Optional.");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL(u8"To connect using a       CS use the argument:");
	WRITE_STDOUT_ENDL(u8" -CS       <cs>          Connection String");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL("OPTION can be:");
	WRITE_STDOUT_ENDL(u8" --silent                Hides all output.");
	WRITE_STDOUT_ENDL(u8" --odbcVersion <version> Set ODBC Version to use. Valid values are '2', '3'");
	WRITE_STDOUT_ENDL(u8"                         or '3.8'. Default is '3'.");
	WRITE_STDOUT_ENDL(u8" --help                  Show this text and return with -1.");
}

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	// if no args given print usage
	if (argc <= 1)
	{
		printUsage();
		return 2;
	}
	try
	{
		const std::string helpKey = u8"--help";
		const std::string userKey = u8"-U";
		const std::string passKey = u8"-P";
		const std::string dsnKey = u8"-DSN";
		const std::string csKey = u8"-CS";
		const std::string silentKey = u8"--silent";
		const std::string odbcVersionKey = u8"--odbcVersion";
		std::string userValue, passValue, dsnValue;
		std::string csValue;
		bool silent = false;
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
			if (ba::equals(arg, helpKey))
			{
				printUsage();
				return -1;
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
			if (ba::equals(arg, silentKey))
			{
				silent = true;
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
					return 2;
				}
			}
		}

		if ((dsnValue.empty() && csValue.empty()) 
			|| (!dsnValue.empty() && !csValue.empty()))
		{
			LOG_ERROR(u8"Muest use either '-CS' or '-DSN [-U <user>] [-P <pass>]', cannot use both");
			return 2;
		}

		if (silent)
		{
			LogManager::Get().SetGlobalLogLevel(LogLevel::None);
		}
		else
		{
			LogManager::Get().SetGlobalLogLevel(LogLevel::Info);
		}

		EnvironmentPtr pEnv = Environment::Create(odbcVersionValue);
		DatabasePtr pDb = Database::Create(pEnv);

		if (!csValue.empty())
		{
			LOG_INFO(boost::str(boost::format(u8"Trying to connect using Connection String '%s'") % csValue));
			pDb->Open(csValue);
		}
		else
		{
			LOG_INFO(boost::str(boost::format(u8"Trying to connect using DSN '%s', User '%s' and Password '%s'") % dsnValue % userValue % passValue));
			pDb->Open(dsnValue, userValue, passValue);
		}
		const DatabaseInfo& dbInfo = pDb->GetDbInfo();

		LOG_INFO(boost::str(boost::format(u8"Successfully connected to database system '%s' using driver '%s'") % dbInfo.GetDbmsName() % dbInfo.GetDriverName()));

		// Start exodbcexec on that db:
		exodbcexec::ExodbcExec exec(pDb);
		exodbcexec::InputGeneratorPtr pGen = std::make_shared<exodbcexec::StdInGenerator>();
		exec.Run(pGen);
	}
	catch (const Exception& ex)
	{
		LOG_ERROR(ex.ToString());
		return 1;
	}
	return 0;
}

namespace exodbcexec
{
	ExodbcExec::ExodbcExec(exodbc::DatabasePtr pDb)
		: m_pDb(pDb)
	{}

	void ExodbcExec::Run(InputGeneratorPtr pInGen)
	{

	}
}