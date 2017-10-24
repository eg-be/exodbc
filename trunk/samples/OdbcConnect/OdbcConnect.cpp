/*!
* \file OdbcConnectionTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 29.03.2017
* \copyright GNU Lesser General Public License Version 3
*
* OdbcConnectionTest Sample.
*/

#ifdef _WIN32
#include <SDKDDKVer.h>
#include <tchar.h>
#endif


// And the rest
#include "exodbc/exOdbc.h"
#include "exodbc/Exception.h"
#include "exodbc/Database.h"
#include "exodbc/LogManager.h"
#include "exodbc/Environment.h"

#include "boost/format.hpp"
#include "boost/program_options.hpp"

using namespace exodbc;
using namespace std;

namespace ba = boost::algorithm;
namespace po = boost::program_options;

void printUsage()
{
	WRITE_STDOUT_ENDL(u8"Usage: odbcconnect  [OPTION]... [-DSN <dsn> [-U <user>] [-P <pass>] | -CS <connectionString>] ");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL(u8"Tests if connecting to a database succeeds.");
	WRITE_STDOUT_ENDL(u8"On success prints the database system name to stdout, or on failure an error");
	WRITE_STDOUT_ENDL(u8"message to stderr. Returns 0 in case of success or 1 in case of failure.");  
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






/* Function used to check that 'opt1' and 'opt2' are not specified
at the same time. */
void conflicting_options(const po::variables_map& vm,
	const char* opt1, const char* opt2)
{
	if (vm.count(opt1) && !vm[opt1].defaulted()
		&& vm.count(opt2) && !vm[opt2].defaulted())
		throw logic_error(string("Conflicting options '")
			+ opt1 + "' and '" + opt2 + "'.");
}


void option_dependency(const boost::program_options::variables_map & vm,
	const std::string & for_what, const std::string & required_option)
{
	if (vm.count(for_what) && !vm[for_what].defaulted())
		if (vm.count(required_option) == 0 || vm[required_option].defaulted())
			throw std::logic_error(std::string("Option '") + for_what
				+ "' requires option '" + required_option + "'.");
}


void printUsage(const std::vector<po::options_description>& optDescs)
{
	WRITE_STDOUT_ENDL(u8"Usage: odbcconnect [OPTION] ConnectionString");
	for (auto it = optDescs.begin(); it != optDescs.end(); ++it)
	{
		const po::options_description& optDesc = *it;
		std::stringstream ss;
		ss << optDesc;
		WRITE_STDOUT_ENDL(ss.str());
	}
}


#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	std::string connectionString;

	// Make ConnectionString a hidden option and use it as a positional option
	po::options_description descConnStr(u8"Connection Parameter");
	descConnStr.add_options()(u8"ConnectionString", po::value<std::string>(&connectionString), u8"Connection String");
	po::positional_options_description posConnStr;
	posConnStr.add("ConnectionString", 1);

	bool silent = false;
	bool dsn = false;
	std::string user, pass;
	exodbc::OdbcVersion ov;
	po::options_description descOptions("Options");
	descOptions.add_options()
		(u8"dsn", po::bool_switch(&dsn), u8"Use a Data Source Name (DSN) instead of a Connection String. The argument ConnectionString will be treated as DSN.")
		(u8"user", po::value<std::string>(&user)->default_value(u8""), u8"A username to be used with a DSN. Can only be used if flag dsn is set.")
		(u8"pass", po::value<std::string>(&pass)->default_value(u8""), u8"A password to be used with a DSN. Can only be used if flag dsn is set.")
		(u8"ov", po::value<exodbc::OdbcVersion>(&ov)->default_value(exodbc::OdbcVersion::V_3), u8"Set ODBC Version to use. Valid values are:\n"
			u8"  2:   \tVersion 2\n"
			u8"  3:   \tVersion 3\n"
			u8"  3.8: \tVersion 3.8")
		(u8"silent", po::bool_switch(&silent), u8"Hide all output, including any error information")
		(u8"help", "Print help and exit")
		;

	po::options_description descAll(u8"All Options");
	descAll.add(descConnStr).add(descOptions);

	try
	{
		po::variables_map vm;
		po::store(po::wcommand_line_parser(argc, argv).options(descAll).positional(posConnStr).run(), vm);

		// Parse help first
		if (vm.count(u8"help"))
		{
			printUsage({ descOptions });
			return 1;
		}

		po::notify(vm);

		option_dependency(vm, u8"user", u8"dsn");
		option_dependency(vm, u8"pass", u8"dsn");

		if (connectionString.empty())
		{
			throw po::error(u8"A ConnectionString must be passed");
		}
	}
	catch (const exception& ex)
	{
		WRITE_STDERR_ENDL(ex.what());
		WRITE_STDERR_ENDL(u8"");
		WRITE_STDERR_ENDL(u8"see --help for more information");
		return 1;
	}

	return 0;


	//boost::optional<int> silent;

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

		if (silent)
		{
			LogManager::Get().SetGlobalLogLevel(LogLevel::None);
		}
		else
		{
			LogManager::Get().SetGlobalLogLevel(LogLevel::Info);
		}

		if ((dsnValue.empty() && csValue.empty()) 
			|| (!dsnValue.empty() && !csValue.empty()))
		{
			LOG_ERROR(u8"Muest use either '-CS' or '-DSN [-U <user>] [-P <pass>]', cannot use both");
			return 2;
		}

		EnvironmentPtr pEnv = Environment::Create(odbcVersionValue);
		Database db(pEnv);

		if (!csValue.empty())
		{
			LOG_INFO(boost::str(boost::format(u8"Trying to connect using Connection String '%s'") % csValue));
			db.Open(csValue);
		}
		else
		{
			LOG_INFO(boost::str(boost::format(u8"Trying to connect using DSN '%s', User '%s' and Password '%s'") % dsnValue % userValue % passValue));
			db.Open(dsnValue, userValue, passValue);
		}
		string dbmsName = db.GetProperties().GetDbmsName();
		string driverName = db.GetProperties().GetDriverName();

		LOG_INFO(boost::str(boost::format(u8"Successfully connected to database system '%s' using driver '%s'") % dbmsName % driverName));
	}
	catch (const Exception& ex)
	{
		LOG_ERROR(ex.ToString());
		return 1;
	}
	return 0;
}

