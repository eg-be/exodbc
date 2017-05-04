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
	WRITE_STDOUT_ENDL(u8"                         that prints the row number. Default is false.");
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
		SqlInfoProperties props = pDb->GetProperties();

		LOG_INFO(boost::str(boost::format(u8"Successfully connected to database system '%s' using driver '%s'.") % props.GetDbmsName() % props.GetDriverName()));

		if (autoCommitValue)
		{
			LOG_INFO(u8"Enabling auto commit ..");
			pDb->SetCommitMode(Database::CommitMode::AUTO);
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
		, m_exitFlag(false)
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
		RegisterCommand(make_shared<Exit>(this));
		RegisterCommand(make_shared<DbInfo>(m_pDb));
		RegisterCommand(make_shared<ListCatalog>(ListCatalog::Mode::TableTypes, m_pDb, !printNoHeader));
		RegisterCommand(make_shared<ListCatalog>(ListCatalog::Mode::Schemas, m_pDb, !printNoHeader));
		RegisterCommand(make_shared<ListCatalog>(ListCatalog::Mode::Catalogs, m_pDb, !printNoHeader));
		RegisterCommand(make_shared<Find>(Find::Mode::Short, m_pDb, !printNoHeader, printRowNr, fixedPrintSize, columnSeparator));
		RegisterCommand(make_shared<Find>(Find::Mode::Interactive, m_pDb, !printNoHeader, printRowNr, fixedPrintSize, columnSeparator));
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
		while (getCmdResult == InputGenerator::GetCommandResult::HAVE_COMMAND && !m_exitFlag);

		return 0;
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
			ColumnDescription colDesc = m_pStmt->DescribeColumn(colNr);
			// add +3 chars to charsize: 1 for '\0', 1 for '.' and 1 for '-':
			exASSERT(m_charColumnMode != CharColumnMode::Auto);
			ColumnBufferPtrVariant pColBuffer;
			SQLLEN bufferSize = 0;
			if (m_charColSize > 0)
				bufferSize = m_charColSize + 1;
			else
				bufferSize = colDesc.GetCharSize() + 3;
			string bufferType;
			if (m_charColumnMode == CharColumnMode::WChar)
			{
				pColBuffer = WCharColumnBuffer::Create(bufferSize, colDesc.GetName(), colDesc.GetSqlType(), ColumnFlag::CF_SELECT);
				bufferType = u8"SQLWCHAR";
			}
			else if (m_charColumnMode == CharColumnMode::Char)
			{
				pColBuffer = CharColumnBuffer::Create(bufferSize, colDesc.GetName(), colDesc.GetSqlType(), ColumnFlag::CF_SELECT);
				bufferType = u8"SQLCHAR";
			}
			LOG_DEBUG(boost::str(boost::format(u8"Binding column nr %d: name: '%s'; charsize: %d; buffersize: %d; buffertype: %s") % colNr % colDesc.GetName() % colDesc.GetCharSize() % bufferSize % bufferType));
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



	//void ExodbcExec::PrintHelp()
	//{
	//	WRITE_STDOUT_ENDL(u8"Any input that is not recognized as a command will be executed as SQL");
	//	WRITE_STDOUT_ENDL(u8"against the database connected to.");
	//	WRITE_STDOUT_ENDL(u8"Commands can be abbreviated. For example the command 'Exit SQL");
	//	WRITE_STDOUT_ENDL(u8"execution', documented as '!exit,!e,!q', can be invoked using '!exit'");
	//	WRITE_STDOUT_ENDL(u8" or '!e' or '!q'.");
	//	WRITE_STDOUT_ENDL(u8"");
	//	WRITE_STDOUT_ENDL(u8"Commands are:");
	//	WRITE_STDOUT_ENDL(u8" !exit,!e,!q         Exit SQL execution.");
	//	WRITE_STDOUT_ENDL(u8" !quit               See !exit.");
	//	WRITE_STDOUT_ENDL(u8" !next,!sn           Select next record.");
	//	WRITE_STDOUT_ENDL(u8" !prev,!sp           Select previous record.");
	//	WRITE_STDOUT_ENDL(u8" !first,!sf          Select first record.");
	//	WRITE_STDOUT_ENDL(u8" !last,!sl           Select last record.");
	//	WRITE_STDOUT_ENDL(u8" !printAll,!pa       Print all records of the current recordset.");
	//	WRITE_STDOUT_ENDL(u8"                     If forward only cursors is set to false, !printAll");
	//	WRITE_STDOUT_ENDL(u8"                     will first execue a '!first' and then print and");
	//	WRITE_STDOUT_ENDL(u8"                     iterate all records by calling '!next'.");
	//	WRITE_STDOUT_ENDL(u8"                     If forward only cursors is set to true, all");
	//	WRITE_STDOUT_ENDL(u8"                     remaining records found using '!next' are printed");
	//	WRITE_STDOUT_ENDL(u8" !printCurrent,!pc   Print the current record.");
	//	WRITE_STDOUT_ENDL(u8" !listTables,!lt     List all tables.");
	//	WRITE_STDOUT_ENDL(u8" !listSchemas,!ls    List all schemas.");
	//	WRITE_STDOUT_ENDL(u8" !listCatalogs,!lc   List all catalogs.");
	//	WRITE_STDOUT_ENDL(u8" !commitTrans,!ct    Commit any ongoing transations.");
	//	WRITE_STDOUT_ENDL(u8" !rollbackTrans,!rt  Rollback all ongoing transactions.");
	//	WRITE_STDOUT_ENDL(u8"");
	//	WRITE_STDOUT_ENDL(u8" !find,!f          [name] [schema] [catalog] [type]");
	//	WRITE_STDOUT_ENDL(u8"                     Searches for objects.");
	//	WRITE_STDOUT_ENDL(u8"                     [name] [schema], [catalog] and [type] are optional.");
	//	WRITE_STDOUT_ENDL(u8"                     See the documentation of SQLTables for more");
	//	WRITE_STDOUT_ENDL(u8"                     information about the arguments.");
	//	WRITE_STDOUT_ENDL(u8"                     If an argument is not given, an empty string is");
	//	WRITE_STDOUT_ENDL(u8"                     used. Use \" to frame an argument value if it");
	//	WRITE_STDOUT_ENDL(u8"                     contains whitespaces.");
	//	WRITE_STDOUT_ENDL(u8" !help,!h            Show this help.");
	//}
}