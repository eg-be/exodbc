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
#include "InputGenerator.h"

// Other headers
#ifdef _WIN32
	#include <SDKDDKVer.h>
	#include <tchar.h>
	#include <io.h>
	#include <fcntl.h>
#endif
#include "exodbc/exOdbc.h"
#include "exodbc/LogManager.h"
#include "exodbc/LogHandler.h"
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/ColumnBufferWrapper.h"

#include "boost/format.hpp"
#include <boost/algorithm/string/iter_find.hpp>
#include <boost/algorithm/string/finder.hpp>
#include <boost/lexical_cast.hpp>

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
	WRITE_STDOUT_ENDL(u8"To connect using a CS use the argument:");
	WRITE_STDOUT_ENDL(u8" -CS       <cs>          Connection String");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL("OPTION can be:");
	WRITE_STDOUT_ENDL(u8" --addRowNr              When printing column values, add a column in front");
	WRITE_STDOUT_ENDL(u8"                         that prints the row number.");
	WRITE_STDOUT_ENDL(u8" --autoCommitOn          Enable auto commit. Default is to commit manual.");
	WRITE_STDOUT_ENDL(u8" --charColType    <type> The SQL C Type of the column buffers to create.");
	WRITE_STDOUT_ENDL(u8"                         Must be either 'SQLCHAR' or 'SQLWCHAR'.");
	WRITE_STDOUT_ENDL(u8"                         Default is to use 'SQLWCHAR' on windows and");
	WRITE_STDOUT_ENDL(u8"                         'SQLCHAR' else.");
	WRITE_STDOUT_ENDL(u8" --charColSize    <size> The size in number of characters for the column");
	WRITE_STDOUT_ENDL(u8"                         buffers to create for data exchange. If not set");
	WRITE_STDOUT_ENDL(u8"                         or set to 0, the database is queried about the size");
	WRITE_STDOUT_ENDL(u8"                         of a column before corresponding buffers are");
	WRITE_STDOUT_ENDL(u8"                         created.");
	WRITE_STDOUT_ENDL(u8" --columnSeparator <str> Character or String displayed to separate values");
	WRITE_STDOUT_ENDL(u8"                         of different columns when printing column data.");
	WRITE_STDOUT_ENDL(u8"                         Default is '||'.");
#ifdef _WIN32
	WRITE_STDOUT_ENDL(u8" --enableUtf16TextMode   On windows, sets the std::wcerr and std::wcout");
	WRITE_STDOUT_ENDL(u8"                         streams to UTF16 textmode. This will do a call to");
	WRITE_STDOUT_ENDL(u8"                         '_setmode(_fileno(stdout), _O_U16TEXT);' for both");
	WRITE_STDOUT_ENDL(u8"                         streams.");
	WRITE_STDOUT_ENDL(u8"                         Only available on windows and it is recommended");
	WRITE_STDOUT_ENDL(u8"                         to set this value. Else there is a risk of a broken");
	WRITE_STDOUT_ENDL(u8"                         wcerr / wcout stream if some utf 16 chars arrive on");
	WRITE_STDOUT_ENDL(u8"                         the stream.");
#endif
	WRITE_STDOUT_ENDL(u8" --exitOnError           Exits with a non-zero status if SQL execution");
	WRITE_STDOUT_ENDL(u8"                         fails or any SQL related call fails. Default is to");
	WRITE_STDOUT_ENDL(u8"                         log a warning and continue.");
	WRITE_STDOUT_ENDL(u8" --fixedPrintSize        When printing column values, format them with a");
	WRITE_STDOUT_ENDL(u8"                         fixed size. The print size of the field is equal");
	WRITE_STDOUT_ENDL(u8"                         to the number of characters that the corresponding");
	WRITE_STDOUT_ENDL(u8"                         buffer holds.");
	WRITE_STDOUT_ENDL(u8" --forwardOnlyCursors    Disables the commands '!prev', '!first' and");
	WRITE_STDOUT_ENDL(u8"                         '!last'. Only '!next' can be used to iterate");
	WRITE_STDOUT_ENDL(u8"                         records. If not set, positionable cursors are");
	WRITE_STDOUT_ENDL(u8"                         enabled by default.");
	WRITE_STDOUT_ENDL(u8" --help                  Show this text and return with -1.");
	WRITE_STDOUT_ENDL(u8" --logLevel <level>      Set the Log Level. <level> can be 'Debug', Info',");
	WRITE_STDOUT_ENDL(u8"                         'Output', 'Warning' or 'Error'. Default is 'Info'.");
	WRITE_STDOUT_ENDL(u8"                         On 'Output', only results from queries and warnings");
	WRITE_STDOUT_ENDL(u8"                         or errors are printed.");
	WRITE_STDOUT_ENDL(u8" --noHeader              Do not add a header row with column names when");
	WRITE_STDOUT_ENDL(u8"                         printing column data. Default is to add a header");
	WRITE_STDOUT_ENDL(u8" --odbcVersion <version> Set ODBC Version to use. Valid values are '2', '3'");
	WRITE_STDOUT_ENDL(u8"                         or '3.8'. Default is '3'.");
	WRITE_STDOUT_ENDL(u8" --silent                Hides all output, except '!help'.");
	WRITE_STDOUT_ENDL(u8" --sqlSeparator    <str> Character or String to separate SQL commands.");
	WRITE_STDOUT_ENDL(u8"                         If set, every SQL command is tokenized using the");
	WRITE_STDOUT_ENDL(u8"                         passed <str> as delimiter. Each token is then");
	WRITE_STDOUT_ENDL(u8"                         executed in its own SQLExecute call.");
	WRITE_STDOUT_ENDL(u8"                         Defaults to not set.");
	WRITE_STDOUT_ENDL(u8"                         Note: If one query fails, query execution is");
	WRITE_STDOUT_ENDL(u8"                         aborted.");
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
		// Pretty printing:
		StdErrLogHandlerPtr pStdLogger = std::make_shared<StdLogHandler>();
		pStdLogger->SetShowFileInfo(false);
		pStdLogger->SetHideLogLevel(LogLevel::Info);
		pStdLogger->SetHideLogLevel(LogLevel::Output);
		LogManager::Get().ClearLogHandlers();
		LogManager::Get().RegisterLogHandler(pStdLogger);
		
		// parse args:
		const std::string helpKey = u8"--help";
		const std::string userKey = u8"-U";
		const std::string passKey = u8"-P";
		const std::string dsnKey = u8"-DSN";
		const std::string csKey = u8"-CS";
		const std::string logLevelKey = u8"--logLevel";
		const std::string columnSeparatorKey = u8"--columnSeparator";
		const std::string charColSizeKey = u8"--charColSize";
		const std::string sqlSeparatorKey = u8"--sqlSeparator";
		const std::string charColTypeKey = u8"--charColType";
		const std::string noHeaderKey = u8"--noHeader";
		const std::string enableUtf16TextModeKey = u8"--enableUtf16TextMode";
		const std::string autoCommitKey = u8"--autoCommitOn";
		const std::string silentKey = u8"--silent";
		const std::string forwardOnlyCursorsKey = u8"--forwardOnlyCursors";
		const std::string fixedPrintSizeKey = u8"--fixedPrintSize";
		const std::string addRowNrKey = u8"--addRowNr";
		const std::string exitOnErrorKey = u8"--exitOnError";
		const std::string odbcVersionKey = u8"--odbcVersion";
		std::string userValue, passValue, dsnValue;
		std::string csValue;
		std::string columnSeparatorValue = u8"||";
		std::string sqlSeparatorValue = u8"";
		bool silent = false;
		bool exitOnError = false;
		bool forwardOnlyCursorsValue = false;
		bool autoCommitValue = false;
		bool noHeaderValue = false;
		bool fixedPrintSizeValue = false;
		bool addRowNrValue = false;
		bool forceCharCols = false;
		bool forceWCharCols = false;
		bool enableUtf16TextModeValue = false;
		SQLLEN charColSizeValue = 0;
		OdbcVersion odbcVersionValue = OdbcVersion::V_3;
		LogLevel logLevelValue = LogLevel::Info;
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
			if (ba::equals(arg, columnSeparatorKey) && i + 1 < argc)
			{
				columnSeparatorValue = argNext;
			}
			if (ba::equals(arg, sqlSeparatorKey) && i + 1 < argc)
			{
				sqlSeparatorValue = argNext;
			}
			if (ba::equals(arg, charColSizeKey) && i + 1 < argc)
			{
				string charColSizeStringValue = argNext;
				try
				{
					charColSizeValue = boost::lexical_cast<SQLINTEGER>(charColSizeStringValue);
				}
				catch (const boost::bad_lexical_cast&)
				{
					charColSizeValue = -1;
				}
				if (charColSizeValue < 0)
				{
					WRITE_STDOUT_ENDL(boost::str(boost::format(u8"Invalid charColSize value '%s'.") % charColSizeStringValue));
					return 2;
				}
			}
			if (ba::equals(arg, charColTypeKey) && i + 1 < argc)
			{
				string charColTypeValue = argNext;
				if (ba::iequals(u8"SQLCHAR", charColTypeValue))
					forceCharCols = true;
				else if (ba::iequals(u8"SQLWCHAR", charColTypeValue))
					forceWCharCols = true;
				else
				{
					WRITE_STDERR_ENDL(boost::str(boost::format(u8"Unknown charColType '%s'.") % charColTypeValue));
					return 2;
				}
			}
			if (ba::equals(arg, autoCommitKey))
			{
				autoCommitValue = true;
			}
			if (ba::equals(arg, enableUtf16TextModeKey))
			{
				enableUtf16TextModeValue = true;
			}
			if (ba::equals(arg, fixedPrintSizeKey))
			{
				fixedPrintSizeValue = true;
			}
			if (ba::equals(arg, addRowNrKey))
			{
				addRowNrValue = true;
			}
			if (ba::equals(arg, silentKey))
			{
				silent = true;
			}
			if (ba::equals(arg, noHeaderKey))
			{
				noHeaderValue = true;
			}
			if (ba::equals(arg, forwardOnlyCursorsKey))
			{
				forwardOnlyCursorsValue = true;
			}
			if (ba::equals(arg, exitOnErrorKey))
			{
				exitOnError = true;
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
					WRITE_STDERR_ENDL(boost::str(boost::format(u8"Unknown OdvcVersion '%s'.") % odbcVersionStringValue));
					return 2;
				}
			}
			if (ba::equals(arg, logLevelKey) && i + 1 < argc)
			{
				string logLevelString = argNext;
				if (ba::iequals(logLevelString, u8"Debug"))
					LogManager::Get().SetGlobalLogLevel(LogLevel::Debug);
				else if (ba::iequals(logLevelString, u8"Info"))
					LogManager::Get().SetGlobalLogLevel(LogLevel::Info);
				else if (ba::iequals(logLevelString, u8"Output"))
					LogManager::Get().SetGlobalLogLevel(LogLevel::Output);
				else if (ba::iequals(logLevelString, u8"Warning"))
					LogManager::Get().SetGlobalLogLevel(LogLevel::Warning);
				else if (ba::iequals(logLevelString, u8"Error"))
					LogManager::Get().SetGlobalLogLevel(LogLevel::Error);
				else
				{
					LOG_ERROR(boost::str(boost::format(u8"Unknown Log Level '%s'.") % logLevelString));
					return 2;
				}
			}
		}

		if ((dsnValue.empty() && csValue.empty()) 
			|| (!dsnValue.empty() && !csValue.empty()))
		{
			LOG_ERROR(u8"Must use either '-CS' or '-DSN [-U <user>] [-P <pass>]', cannot use both.");
			return 2;
		}

		if (silent)
		{
			LogManager::Get().SetGlobalLogLevel(LogLevel::None);
		}

#ifdef _WIN32
		if (enableUtf16TextModeValue)
		{
			_setmode(_fileno(stdout), _O_U16TEXT);
			_setmode(_fileno(stderr), _O_U16TEXT);
		}
#endif

		EnvironmentPtr pEnv = Environment::Create(odbcVersionValue);
		DatabasePtr pDb = Database::Create(pEnv);

		if (!csValue.empty())
		{
			LOG_INFO(boost::str(boost::format(u8"Trying to connect using Connection String '%s' ..") % csValue));
			pDb->Open(csValue);
		}
		else
		{
			LOG_INFO(boost::str(boost::format(u8"Trying to connect using DSN '%s', User '%s' and Password '%s' ..") % dsnValue % userValue % passValue));
			pDb->Open(dsnValue, userValue, passValue);
		}
		const DatabaseInfo& dbInfo = pDb->GetDbInfo();

		LOG_INFO(boost::str(boost::format(u8"Successfully connected to database system '%s' using driver '%s'.") % dbInfo.GetDbmsName() % dbInfo.GetDriverName()));

		if (autoCommitValue)
		{
			LOG_INFO(u8"Enabling auto commit ..");
			pDb->SetCommitMode(CommitMode::AUTO);
			LOG_INFO(u8"auto commit is on.");
		}

		// Start exodbcexec on that db:
		exodbcexec::ExodbcExec::CharColumnMode charColMode = exodbcexec::ExodbcExec::CharColumnMode::Auto;
		if (forceCharCols)
			charColMode = exodbcexec::ExodbcExec::CharColumnMode::Char;
		else if (forceWCharCols)
			charColMode = exodbcexec::ExodbcExec::CharColumnMode::WChar;

		exodbcexec::ExodbcExec exec(pDb, exitOnError, forwardOnlyCursorsValue, columnSeparatorValue, 
			noHeaderValue, fixedPrintSizeValue, addRowNrValue, charColMode, charColSizeValue, sqlSeparatorValue);
		exodbcexec::InputGeneratorPtr pGen = std::make_shared<exodbcexec::StdInGenerator>();
		return exec.Run(pGen);
	}
	catch (const Exception& ex)
	{
		LOG_ERROR(ex.ToString());
		return 1;
	}
}

namespace exodbcexec
{

	const std::set<std::string> ExodbcExec::COMMAND_EXIT = { u8"!exit", u8"!e", u8"!quit", u8"!q" };
	const std::set<std::string> ExodbcExec::COMMAND_HELP = { u8"!help", u8"!h" };
	const std::set<std::string> ExodbcExec::COMMAND_PRINT_ALL = { u8"!printAll", u8"!pa" };
	const std::set<std::string> ExodbcExec::COMMAND_PRINT_CURRENT = { u8"!printCurrent", u8"!pc" };
	const std::set<std::string> ExodbcExec::COMMAND_SELECT_NEXT = { u8"!next", u8"!sn" };
	const std::set<std::string> ExodbcExec::COMMAND_SELECT_PREV = { u8"!prev", u8"!sp" };
	const std::set<std::string> ExodbcExec::COMMAND_SELECT_FIRST = { u8"!first", u8"!sf" };
	const std::set<std::string> ExodbcExec::COMMAND_SELECT_LAST = { u8"!last", u8"!sl" };
	const std::set<std::string> ExodbcExec::COMMAND_COMMIT_TRANS = { u8"!commitTrans", u8"!ct" };
	const std::set<std::string> ExodbcExec::COMMAND_ROLLBACK_TRANS = { u8"!rollbackTrans", u8"!rt" };
	const std::set<std::string> ExodbcExec::COMMAND_FIND = { u8"!find", u8"!f" };
	const std::set<std::string> ExodbcExec::COMMAND_LIST_TABLES = { u8"!listTables", u8"!lt" };
	const std::set<std::string> ExodbcExec::COMMAND_LIST_SCHEMAS = { u8"!listSchemas", u8"!ls" };
	const std::set<std::string> ExodbcExec::COMMAND_LIST_CATALOGS = { u8"!listCatalogs", u8"!lc" };

	ExodbcExec::ExodbcExec(exodbc::DatabasePtr pDb, bool exitOnError, bool forwardOnlyCursors, const std::string& columnSeparator,
			bool printNoHeader, bool fixedPrintSize, bool printRowNr, CharColumnMode charColMode,
			SQLLEN charColSize,	const std::string& sqlSeparator)
		: m_pDb(pDb)
		, m_exitOnError(exitOnError)
		, m_forwardOnlyCursors(forwardOnlyCursors)
		, m_columnSeparator(columnSeparator)
		, m_printNoHeader(printNoHeader)
		, m_fixedPrintSize(fixedPrintSize)
		, m_printRowNr(printRowNr)
		, m_charColumnMode(charColMode)
		, m_charColSize(charColSize)
		, m_sqlSeparator(sqlSeparator)
	{
		exASSERT(charColSize >= 0);
		if (m_charColumnMode == CharColumnMode::Auto)
		{
#ifdef _WIN32
			m_charColumnMode = CharColumnMode::WChar;
#else
			m_charColumnMode = CharColumnMode::Char;
#endif
		}
		m_stmt.Init(m_pDb, m_forwardOnlyCursors);

		m_pStmt = make_shared<ExecutableStatement>(m_pDb, m_forwardOnlyCursors);

//		CreateCommands();
		RegisterCommand(make_shared<ExecuteSql>(m_pStmt));
		RegisterCommand(make_shared<Select>(Select::Mode::First, m_pStmt));
		RegisterCommand(make_shared<Select>(Select::Mode::Last, m_pStmt));
		RegisterCommand(make_shared<Select>(Select::Mode::Next, m_pStmt));
		RegisterCommand(make_shared<Select>(Select::Mode::Prev, m_pStmt));
		RegisterCommand(make_shared<Print>(Print::Mode::CurrentRecord, m_currentColumns, m_pStmt,
			columnSeparator, !printNoHeader, printRowNr, fixedPrintSize, 0));
		RegisterCommand(make_shared<Print>(Print::Mode::AllRecords, m_currentColumns, m_pStmt,
			columnSeparator, !printNoHeader, printRowNr, fixedPrintSize, 0));
		RegisterCommand(make_shared<Commit>(m_pDb));
		RegisterCommand(make_shared<Rollback>(m_pDb));
		RegisterCommand(make_shared<Help>(GetCommands()));
	}


	int ExodbcExec::Run(InputGeneratorPtr pInGen)
	{
		LOG_INFO(boost::str(boost::format(u8"Ready to execute SQL. Type '!exit' to exit, or '!help' for help.")));
		InputGenerator::GetCommandResult getCmdResult = InputGenerator::GetCommandResult::NO_COMMAND;
		auto GetInput = [&]() {
			string input;
			getCmdResult = pInGen->GetNextCommand(input);
			return input;
		};
		do
		{
			const string input = GetInput();

			if (input.empty())
				continue;

			if (COMMAND_EXIT.find(input) != COMMAND_EXIT.end())
				break;

			try
			{
				// Try to find a command
				CommandPtr pCommand = nullptr;
				vector<string> args;
				if (ba::starts_with(input, Command::COMMAND_PREFIX))
				{
					boost::split(args, input.substr(1), boost::is_any_of(u8" "));
					string cmd = args.front();
					args.erase(args.begin());
					pCommand = GetCommand(cmd);
				}
				if (pCommand)
				{
					pCommand->Execute(args);
				}
				else
				{
					if (ba::starts_with(input, u8"!"))
						LOG_WARNING(boost::str(boost::format(u8"Input starts with '!', but is not recognized as a command, executing as SQL.")));

					// no command found, pass original input to ExecuteSql
					CommandPtr pExec = GetCommand(ExecuteSql::NAME, true);
					exASSERT(pExec);
					if (!m_currentColumns.empty())
					{
						UnbindColumns();
					}
					pExec->Execute({ input });
					if (m_pStmt->GetNrOfColumns() > 0)
					{
						BindColumns();
					}
				}


				//std::string command;
				//std::vector<std::string> commandArgs;
				//if (ba::starts_with(input, u8"!"))
				//{
				//	// split arguments in command, get the argument part first
				//	size_t wsPos = input.find(u8" ");
				//	if (wsPos == string::npos)
				//	{
				//		command = input;
				//	}
				//	if (wsPos != string::npos)
				//	{
				//		string argsPart = boost::trim_copy(input.substr(wsPos));
				//		command = input.substr(0, wsPos);
				//		string currentArg;
				//		bool inFramedPart = false;
				//		// and iterate, but respect arguments in "", like "Hello world", or "   "
				//		for (string::const_iterator it = argsPart.begin(); it != argsPart.end(); ++it)
				//		{
				//			if (*it == '"')
				//			{
				//				if (!inFramedPart)
				//				{
				//					inFramedPart = true;
				//				}
				//				else
				//				{
				//					inFramedPart = false;
				//					commandArgs.push_back(currentArg);
				//					currentArg = u8"";
				//				}
				//			}
				//			else if (*it == ' ' && !inFramedPart && !currentArg.empty())
				//			{
				//				commandArgs.push_back(currentArg);
				//				currentArg = u8"";
				//			}
				//			else if(*it != ' ' || inFramedPart)
				//			{
				//				currentArg += *it;
				//			}
				//		}
				//		if (!currentArg.empty())
				//		{
				//			commandArgs.push_back(currentArg);
				//		}
				//	}
				//}

				//if (COMMAND_HELP.find(command) != COMMAND_HELP.end())
				//{
				//	PrintHelp();
				//}
				//else if (COMMAND_PRINT_ALL.find(command) != COMMAND_PRINT_ALL.end())
				//{
				//	Print(PrintMode::All);
				//}
				//else if(COMMAND_PRINT_CURRENT.find(command) != COMMAND_PRINT_CURRENT.end())
				//{
				//	Print(PrintMode::Current);
				//}
				//else if (COMMAND_SELECT_NEXT.find(command) != COMMAND_SELECT_NEXT.end())
				//{
				//	Select(SelectMode::Next);
				//}
				//else if (!m_forwardOnlyCursors && COMMAND_SELECT_PREV.find(command) != COMMAND_SELECT_PREV.end())
				//{
				//	Select(SelectMode::Prev);
				//}
				//else if (!m_forwardOnlyCursors && COMMAND_SELECT_FIRST.find(command) != COMMAND_SELECT_FIRST.end())
				//{
				//	Select(SelectMode::First);
				//}
				//else if (!m_forwardOnlyCursors && COMMAND_SELECT_LAST.find(command) != COMMAND_SELECT_LAST.end())
				//{
				//	Select(SelectMode::Last);
				//}
				//else if (COMMAND_COMMIT_TRANS.find(command) != COMMAND_COMMIT_TRANS.end())
				//{
				//	m_pDb->CommitTrans();
				//}
				//else if (COMMAND_ROLLBACK_TRANS.find(command) != COMMAND_ROLLBACK_TRANS.end())
				//{
				//	m_pDb->RollbackTrans();
				//}
				//else if (COMMAND_LIST_TABLES.find(command) != COMMAND_LIST_TABLES.end())
				//{
				//	List(ListMode::Types);
				//}
				//else if (COMMAND_LIST_SCHEMAS.find(command) != COMMAND_LIST_SCHEMAS.end())
				//{
				//	List(ListMode::Schemas);
				//}
				//else if (COMMAND_LIST_CATALOGS.find(command) != COMMAND_LIST_CATALOGS.end())
				//{
				//	List(ListMode::Catalogs);
				//}
				//else if (COMMAND_FIND.find(command) != COMMAND_FIND.end())
				//{
				//	string name = commandArgs.size() >= 1 ? commandArgs[0] : u8"";
				//	string schema = commandArgs.size() >= 2 ? commandArgs[1] : u8"";
				//	string catalog = commandArgs.size() >= 3 ? commandArgs[2] : u8"";
				//	string type = commandArgs.size() >= 4 ? commandArgs[3] : u8"";
				//	LOG_INFO(boost::str(boost::format(u8"Searching for tables with name: '%s'; schema: '%s'; catalog: '%s'; Type: '%s'") 
				//		% name % schema % catalog % type));
				//	auto start = std::chrono::high_resolution_clock::now();
				//	TableInfosVector tables = m_pDb->FindTables(name, schema, catalog, type);
				//	auto end = std::chrono::high_resolution_clock::now();
				//	auto elapsed = end - start;
				//	auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
				//	LOG_INFO(boost::str(boost::format(u8"Success, found %d table(s). Execution took %dms.")
				//		% tables.size() % millis.count()));

				//	if (tables.size() > 0)
				//	{
				//		LOG_OUTPUT(boost::str(boost::format(u8"%18s%s%18s%s%18s%s%18s")
				//			% u8"Name" % m_columnSeparator % u8"Schema" % m_columnSeparator
				//			% u8"Catalog" % m_columnSeparator % u8"type"));
				//		for (TableInfosVector::const_iterator it = tables.begin(); it != tables.end(); ++it)
				//		{
				//			LOG_OUTPUT(boost::str(boost::format(u8"%18s%s%18s%s%18s%s%18s")
				//				% it->GetPureName() % m_columnSeparator % it->GetSchema() % m_columnSeparator
				//				% it->GetCatalog() % m_columnSeparator % it->GetType()));
				//		}
				//		LOG_INFO(u8"No more results available");
				//	}
 			//	}
				//else
				//{
				//	if (ba::starts_with(input, u8"!"))
				//		LOG_WARNING(boost::str(boost::format(u8"Input starts with '!' but is not recognized as a command, executing as SQL.")));

				//	// Before executing, unbind any bound columns
				//	if( ! m_currentColumns.empty())
				//		UnbindColumns();

				//	// Tokenize commands if a sql separator is set
				//	vector<string> cmds;
				//	if ( ! m_sqlSeparator.empty())
				//	{
				//		iter_split(cmds, input, boost::algorithm::first_finder(m_sqlSeparator));
				//	}
				//	else
				//	{
				//		cmds.push_back(input);
				//	}

				//	for (vector<string>::const_iterator it = cmds.begin(); it != cmds.end(); ++it)
				//	{
				//		string cmd = *it;
				//		if (cmd.empty())
				//			continue;

				//		LOG_INFO(boost::str(boost::format(u8"Executing '%s'") % cmd));
				//		auto start = std::chrono::high_resolution_clock::now();
				//		m_stmt.ExecuteDirect(cmd);
				//		auto end = std::chrono::high_resolution_clock::now();
				//		auto elapsed = end - start;
				//		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
				//		LOG_INFO(boost::str(boost::format(u8"Success. Execution took %1%ms.")
				//			% millis.count()));
				//	}
				//	// And bind columns, if there are any
				//	if (m_stmt.GetNrOfColumns() > 0)
				//	{
				//		BindColumns();
				//	}
				//}
			}
			catch (const exodbc::SqlResultException& sre)
			{
				if (m_exitOnError)
				{
					LOG_ERROR(sre.ToString());
					return 10;
				}
				LOG_WARNING(sre.ToString());
			}
			catch (const exodbc::Exception& ex)
			{
				LOG_ERROR(ex.ToString());
				return 20;
			}
		}
		while (getCmdResult == InputGenerator::GetCommandResult::HAVE_COMMAND);

		return 0;
	}


	void ExodbcExec::CreateCommands()
	{
		//RegisterCommand(make_shared<ExecuteSql>(m_pStmt));
		//RegisterCommand(make_shared<Select>(Select::Mode::First, m_pStmt));
		//RegisterCommand(make_shared<Select>(Select::Mode::Last, m_pStmt));
		//RegisterCommand(make_shared<Select>(Select::Mode::Next, m_pStmt));
		//RegisterCommand(make_shared<Select>(Select::Mode::Prev, m_pStmt));
		//RegisterCommand(make_shared<Select>(Print::Mode::CurrentRecord, m_currentColumns));
		//RegisterCommand(make_shared<Select>(Print::Mode::AllRecords, m_currentColumns));
	}


	void ExodbcExec::RegisterCommand(CommandPtr pCommand)
	{
		exASSERT(pCommand);

		const vector<string>& aliases = pCommand->GetAliases();
		for (vector<string>::const_iterator it = aliases.begin(); it != aliases.end(); ++it)
		{
			exASSERT_MSG(m_commands.find(*it) == m_commands.end(), 
				boost::str(boost::format(u8"Command '%s' is already registered!") % *it));
		}
		for (vector<string>::const_iterator it = aliases.begin(); it != aliases.end(); ++it)
		{
			m_commands[*it] = pCommand;
		}
	}


	CommandPtr ExodbcExec::GetCommand(const std::string& name, bool includeHidden /* = false */) const
	{
		exASSERT(!name.empty());

		map<string, CommandPtr>::const_iterator it = m_commands.find(name);
		if (it != m_commands.end() && (!it->second->Hidden() || includeHidden))
		{
			return it->second;
		}
		return nullptr;
	}


	set<CommandPtr> ExodbcExec::GetCommands() const noexcept
	{
		set<CommandPtr> cmds;
		for (std::map<std::string, CommandPtr>::const_iterator it = m_commands.begin();
			it != m_commands.end(); ++it)
		{
			if (!it->second->Hidden())
			{
				cmds.insert(it->second);
			}
		}
		return cmds;
	}


	void ExodbcExec::BindColumns()
	{
		exASSERT(m_currentColumns.empty());

		SQLSMALLINT nrOfCols = m_pStmt->GetNrOfColumns();
		if (nrOfCols == 0)
		{
			LOG_WARNING(u8"No columns available in current result set.");
			return;
		}

		// Bind columns
		for (SQLSMALLINT colNr = 1; colNr <= nrOfCols; ++colNr)
		{
			SColumnDescription colDesc = m_pStmt->DescribeColumn(colNr);
			// add +3 chars to charsize: 1 for '\0', 1 for '.' and 1 for '-':
			exASSERT(m_charColumnMode != CharColumnMode::Auto);
			ColumnBufferPtrVariant pColBuffer;
			SQLLEN bufferSize = 0;
			if (m_charColSize > 0)
				bufferSize = m_charColSize + 1;
			else
				bufferSize = colDesc.m_charSize + 3;
			string bufferType;
			if (m_charColumnMode == CharColumnMode::WChar)
			{
				pColBuffer = WCharColumnBuffer::Create(bufferSize, colDesc.m_name, colDesc.m_sqlType, ColumnFlag::CF_SELECT);
				bufferType = u8"SQLWCHAR";
			}
			else if (m_charColumnMode == CharColumnMode::Char)
			{
				pColBuffer = CharColumnBuffer::Create(bufferSize, colDesc.m_name, colDesc.m_sqlType, ColumnFlag::CF_SELECT);
				bufferType = u8"SQLCHAR";
			}
			LOG_DEBUG(boost::str(boost::format(u8"Binding column nr %d: name: '%s'; charsize: %d; buffersize: %d; buffertype: %s") % colNr % colDesc.m_name % colDesc.m_charSize % bufferSize % bufferType));
			m_pStmt->BindColumn(pColBuffer, colNr);
			StringColumnWrapper wrapper(pColBuffer);
			m_currentColumns.push_back(wrapper);
		}
	}


	void ExodbcExec::UnbindColumns()
	{
		LOG_DEBUG(boost::str(boost::format(u8"Unbinding %d columns") % m_currentColumns.size()));
		m_pStmt->UnbindColumns();
		m_currentColumns.clear();
	}


	//void ExodbcExec::Select(SelectMode mode)
	//{
	//	switch (mode)
	//	{
	//	case SelectMode::First:
	//		m_stmt.SelectFirst();
	//		break;
	//	case SelectMode::Last:
	//		m_stmt.SelectLast();
	//		break;
	//	case SelectMode::Next:
	//		m_stmt.SelectNext();
	//		break;
	//	case SelectMode::Prev:
	//		m_stmt.SelectPrev();
	//		break;
	//	}
	//}


	void ExodbcExec::List(ExodbcExec::ListMode mode)
	{
		auto Lister = [&](ListMode mode) {
			switch (mode)
			{
			case ListMode::Types:
			case ListMode::Schemas:
				return m_pDb->ReadSchemas();
			case ListMode::Catalogs:
				return m_pDb->ReadCatalogs();
				break;
			}
			exASSERT(false);
		};

		vector<string> data;
		LOG_INFO(boost::str(boost::format(u8"Listing all %s ..") % ToString(mode)));
		auto start = std::chrono::high_resolution_clock::now();
		switch (mode)
		{
		case ListMode::Types:
			data = m_pDb->ReadTableTypes();
			break;
		case ListMode::Schemas:
			data = m_pDb->ReadSchemas();
			break;
		case ListMode::Catalogs:
			data = m_pDb->ReadCatalogs();
			break;
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = end - start;
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
		LOG_INFO(boost::str(boost::format(u8"Success, found %d %s. Execution took %dms.")
			% data.size() % ToString(mode) % millis.count()));

		if (!data.empty())
		{
			LOG_INFO(boost::str(boost::format(u8"Name")));
			LOG_INFO(boost::str(boost::format(u8"----")));
			for (vector<string>::const_iterator it = data.begin(); it != data.end(); ++it)
			{
				LOG_INFO(*it);
			}
		}
	}


	//void ExodbcExec::Print(ExodbcExec::PrintMode mode)
	//{
	//	if (m_currentColumns.empty())
	//	{
	//		LOG_WARNING(u8"No record set with bound columns is open.");
	//		return;
	//	}

	//	// print current or all
	//	if (!m_printNoHeader)
	//	{
	//		LOG_OUTPUT(GetHeaderString(m_currentColumns));
	//	}
	//	if (mode == PrintMode::Current)
	//	{
	//		stringstream ss;
	//		if (m_printRowNr && m_fixedPrintSize)
	//			ss << boost::str(boost::format(u8"%10d") % 1) << m_columnSeparator;
	//		else if (m_printRowNr)
	//			ss << boost::str(boost::format(u8"%d") % 1) << m_columnSeparator;
	//		ss << CurrentRecordToString(m_currentColumns);
	//		LOG_OUTPUT(ss.str());
	//	}
	//	else if (mode == PrintMode::All)
	//	{
	//		size_t rowCount = 1;

	//		bool haveNext = false;
	//		if (m_stmt.IsForwardOnlyCursor())
	//			haveNext = m_stmt.SelectNext();
	//		else
	//			haveNext = m_stmt.SelectFirst();
	//		while (haveNext)
	//		{
	//			stringstream ss;
	//			if (m_printRowNr && m_fixedPrintSize)
	//				ss << boost::str(boost::format(u8"%10d") % rowCount) << m_columnSeparator;
	//			else if(m_printRowNr)
	//				ss << boost::str(boost::format(u8"%d") % rowCount) << m_columnSeparator;

	//			ss << CurrentRecordToString(m_currentColumns);
	//			LOG_OUTPUT(ss.str());
	//			haveNext = m_stmt.SelectNext();
	//			++rowCount;
	//		}
	//		LOG_INFO(u8"No more results available.");
	//	}
	//}
	

	std::string ExodbcExec::CurrentRecordToString(const std::vector<exodbc::StringColumnWrapper>& columns) const
	{
		stringstream ss;
		std::vector<StringColumnWrapper>::const_iterator it = columns.begin();
		while (it != columns.end())
		{
			string sval;
			if (it->IsNull())
				sval = u8"NULL";
			else
				sval = it->GetValue<std::string>();
			if (m_fixedPrintSize)
			{
				ColumnBufferPtrVariant pCol = it->GetVariant();
				string sheader = boost::apply_visitor(QueryNameVisitor(), pCol);
				SQLLEN nrOfHeaderChars =(SQLLEN) sheader.length();
				SQLLEN nrOfElements = boost::apply_visitor(NrOfElementsVisitor(), pCol);
				// note that one element added was the terminating '\0', do not add that to print
				SQLLEN printSize = max(nrOfElements - 1, nrOfHeaderChars);
				exASSERT(printSize >= 1);
				stringstream fss;
				fss << u8"%" << printSize << "s";
				sval = boost::str(boost::format(fss.str()) % sval);
			}
			ss << sval;

			++it;
			if (it != columns.end())
				ss << m_columnSeparator;
		}
		return ss.str();
	}


	std::string ExodbcExec::GetHeaderString(const std::vector<exodbc::StringColumnWrapper>& columns) const
	{
		stringstream ss;
		std::vector<StringColumnWrapper>::const_iterator it = columns.begin();
		if (m_printRowNr)
			ss << boost::str(boost::format(u8"%10s") % u8"ROW") << m_columnSeparator;

		while (it != columns.end())
		{
			ColumnBufferPtrVariant pCol = it->GetVariant();
			string sheader = boost::apply_visitor(QueryNameVisitor(), pCol);
			if (m_fixedPrintSize)
			{
				SQLLEN nrOfElements = boost::apply_visitor(NrOfElementsVisitor(), pCol);
				SQLLEN nrOfHeaderChars = (SQLLEN) sheader.length();
				// note that one element added was the terminating '\0', do not add that to print
				SQLLEN printSize = max(nrOfElements - 1, nrOfHeaderChars);
				exASSERT(printSize >= 1);
				stringstream fss;
				fss << u8"%" << printSize << "s";
				sheader = boost::str(boost::format(fss.str()) % sheader);
			}
			ss << sheader;

			++it;
			if (it != columns.end())
				ss << m_columnSeparator;
		}
		return ss.str();
	}


	std::string ExodbcExec::ToString(ListMode mode) const noexcept
	{
		switch (mode)
		{
		case ListMode::Catalogs:
			return u8"Catalogs";
		case ListMode::Schemas:
			return u8"Schemas";
		case ListMode::Types:
			return u8"Types";
		}
		return u8"???";
	}


	void ExodbcExec::PrintHelp()
	{
		WRITE_STDOUT_ENDL(u8"Any input that is not recognized as a command will be executed as SQL");
		WRITE_STDOUT_ENDL(u8"against the database connected to.");
		WRITE_STDOUT_ENDL(u8"Commands can be abbreviated. For example the command 'Exit SQL");
		WRITE_STDOUT_ENDL(u8"execution', documented as '!exit,!e,!q', can be invoked using '!exit'");
		WRITE_STDOUT_ENDL(u8" or '!e' or '!q'.");
		WRITE_STDOUT_ENDL(u8"");
		WRITE_STDOUT_ENDL(u8"Commands are:");
		WRITE_STDOUT_ENDL(u8" !exit,!e,!q         Exit SQL execution.");
		WRITE_STDOUT_ENDL(u8" !quit               See !exit.");
		WRITE_STDOUT_ENDL(u8" !next,!sn           Select next record.");
		WRITE_STDOUT_ENDL(u8" !prev,!sp           Select previous record.");
		WRITE_STDOUT_ENDL(u8" !first,!sf          Select first record.");
		WRITE_STDOUT_ENDL(u8" !last,!sl           Select last record.");
		WRITE_STDOUT_ENDL(u8" !printAll,!pa       Print all records of the current recordset.");
		WRITE_STDOUT_ENDL(u8"                     If forward only cursors is set to false, !printAll");
		WRITE_STDOUT_ENDL(u8"                     will first execue a '!first' and then print and");
		WRITE_STDOUT_ENDL(u8"                     iterate all records by calling '!next'.");
		WRITE_STDOUT_ENDL(u8"                     If forward only cursors is set to true, all");
		WRITE_STDOUT_ENDL(u8"                     remaining records found using '!next' are printed");
		WRITE_STDOUT_ENDL(u8" !printCurrent,!pc   Print the current record.");
		WRITE_STDOUT_ENDL(u8" !listTables,!lt     List all tables.");
		WRITE_STDOUT_ENDL(u8" !listSchemas,!ls    List all schemas.");
		WRITE_STDOUT_ENDL(u8" !listCatalogs,!lc   List all catalogs.");
		WRITE_STDOUT_ENDL(u8" !commitTrans,!ct    Commit any ongoing transations.");
		WRITE_STDOUT_ENDL(u8" !rollbackTrans,!rt  Rollback all ongoing transactions.");
		WRITE_STDOUT_ENDL(u8"");
		WRITE_STDOUT_ENDL(u8" !find,!f          [name] [schema] [catalog] [type]");
		WRITE_STDOUT_ENDL(u8"                     Searches for objects.");
		WRITE_STDOUT_ENDL(u8"                     [name] [schema], [catalog] and [type] are optional.");
		WRITE_STDOUT_ENDL(u8"                     See the documentation of SQLTables for more");
		WRITE_STDOUT_ENDL(u8"                     information about the arguments.");
		WRITE_STDOUT_ENDL(u8"                     If an argument is not given, an empty string is");
		WRITE_STDOUT_ENDL(u8"                     used. Use \" to frame an argument value if it");
		WRITE_STDOUT_ENDL(u8"                     contains whitespaces.");
		WRITE_STDOUT_ENDL(u8" !help,!h            Show this help.");
	}
}