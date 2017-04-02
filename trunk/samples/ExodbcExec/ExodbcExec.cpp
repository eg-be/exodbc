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
#endif
#include "exodbc/exOdbc.h"
#include "exodbc/LogManager.h"
#include "exodbc/LogHandler.h"
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/ColumnBufferWrapper.h"

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
	WRITE_STDOUT_ENDL(u8"To connect using a CS use the argument:");
	WRITE_STDOUT_ENDL(u8" -CS       <cs>          Connection String");
	WRITE_STDOUT_ENDL(u8"");
	WRITE_STDOUT_ENDL("OPTION can be:");
	WRITE_STDOUT_ENDL(u8" --silent                Hides all output.");
	WRITE_STDOUT_ENDL(u8" --odbcVersion <version> Set ODBC Version to use. Valid values are '2', '3'");
	WRITE_STDOUT_ENDL(u8"                         or '3.8'. Default is '3'.");
	WRITE_STDOUT_ENDL(u8" --autoCommitOn          Enable auto commit. Default is to commit manual.");
	WRITE_STDOUT_ENDL(u8" --forwardOnlyCursors    Disables the commands '!prev', '!first' and");
	WRITE_STDOUT_ENDL(u8"                         '!last'. Only '!next' can be used to iterate");
	WRITE_STDOUT_ENDL(u8"                         records. If not set, positionable cursors are");
	WRITE_STDOUT_ENDL(u8"                         enabled by default.");
	WRITE_STDOUT_ENDL(u8" --exitOnError           Exits with a non-zero status if SQL execution");
	WRITE_STDOUT_ENDL(u8"                         fails or any SQL related call fails. Default is to");
	WRITE_STDOUT_ENDL(u8"                         log a warning and continue.");
	WRITE_STDOUT_ENDL(u8" --logLevel <level>      Set the Log Level. <level> can be 'Debug', Info',");
	WRITE_STDOUT_ENDL(u8"                         'Warning' or 'Error'. Default is 'Info'.");
	WRITE_STDOUT_ENDL(u8" --columnSeparator <str> Character or String displayed to separate values");
	WRITE_STDOUT_ENDL(u8"                         of different columns when printing column data.");
	WRITE_STDOUT_ENDL(u8"                         Default is '||'.");
	WRITE_STDOUT_ENDL(u8" --noHeader              Do not add a header row with column names when");
	WRITE_STDOUT_ENDL(u8"                         printing column data. Default is to add a header");
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
		// Pretty printing:
		StdErrLogHandlerPtr pStdLogger = std::make_shared<StdLogHandler>();
		pStdLogger->SetShowFileInfo(false);
		pStdLogger->SetHideLogLevel(LogLevel::Info);
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
		const std::string noHeaderKey = u8"--noHeader";
		const std::string autoCommitKey = u8"--autoCommitOn";
		const std::string silentKey = u8"--silent";
		const std::string forwardOnlyCursorsKey = u8"--forwardOnlyCursors";
		const std::string exitOnErrorKey = u8"--exitOnError";
		const std::string odbcVersionKey = u8"--odbcVersion";
		std::string userValue, passValue, dsnValue;
		std::string csValue;
		std::string columnSeparatorValue = u8"||";
		bool silent = false;
		bool exitOnError = false;
		bool forwardOnlyCursorsValue = false;
		bool autoCommitValue = false;
		bool noHeaderValue = false;
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
			if (ba::equals(arg, autoCommitKey))
			{
				autoCommitValue = true;
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

		EnvironmentPtr pEnv = Environment::Create(odbcVersionValue);
		DatabasePtr pDb = Database::Create(pEnv);

		if (!csValue.empty())
		{
			LOG_INFO(boost::str(boost::format(u8"Trying to connect using Connection String '%s'...") % csValue));
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
		exodbcexec::ExodbcExec exec(pDb, exitOnError, forwardOnlyCursorsValue, columnSeparatorValue, noHeaderValue);
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

	ExodbcExec::ExodbcExec(exodbc::DatabasePtr pDb, bool exitOnError, bool forwardOnlyCursors, const std::string& columnSeparator,
			bool printNoHeader)
		: m_pDb(pDb)
		, m_exitOnError(exitOnError)
		, m_forwardOnlyCursors(forwardOnlyCursors)
		, m_columnSeparator(columnSeparator)
		, m_printNoHeader(printNoHeader)
	{
		m_stmt.Init(m_pDb, m_forwardOnlyCursors);
	}


	int ExodbcExec::Run(InputGeneratorPtr pInGen)
	{
		std::string command;
		LOG_INFO(boost::str(boost::format(u8"Ready to execute SQL. Type '!exit' to exit, or '!help' for help.")));
		while (pInGen->GetNextCommand(command) == InputGenerator::GetCommandResult::HAVE_COMMAND)
		{
			if (COMMAND_EXIT.find(command) != COMMAND_EXIT.end())
				break;

			try
			{
				if (COMMAND_HELP.find(command) != COMMAND_HELP.end())
				{
					PrintHelp();
				}
				else if (COMMAND_PRINT_ALL.find(command) != COMMAND_PRINT_ALL.end())
				{
					Print(PrintMode::All);
				}
				else if(COMMAND_PRINT_CURRENT.find(command) != COMMAND_PRINT_CURRENT.end())
				{
					Print(PrintMode::Current);
				}
				else if (COMMAND_SELECT_NEXT.find(command) != COMMAND_SELECT_NEXT.end())
				{
					Select(SelectMode::Next);
				}
				else if (!m_forwardOnlyCursors && COMMAND_SELECT_PREV.find(command) != COMMAND_SELECT_PREV.end())
				{
					Select(SelectMode::Prev);
				}
				else if (!m_forwardOnlyCursors && COMMAND_SELECT_FIRST.find(command) != COMMAND_SELECT_FIRST.end())
				{
					Select(SelectMode::First);
				}
				else if (!m_forwardOnlyCursors && COMMAND_SELECT_LAST.find(command) != COMMAND_SELECT_LAST.end())
				{
					Select(SelectMode::Last);
				}
				else if (COMMAND_COMMIT_TRANS.find(command) != COMMAND_COMMIT_TRANS.end())
				{
					m_pDb->CommitTrans();
				}
				else if (COMMAND_ROLLBACK_TRANS.find(command) != COMMAND_ROLLBACK_TRANS.end())
				{
					m_pDb->RollbackTrans();
				}
				else
				{
					if (ba::starts_with(command, u8"!"))
						LOG_WARNING(boost::str(boost::format(u8"Input starts with '!' but is not recognized as a command, executing as SQL.")));

					// Before executing, unbind any bound columns
					if( ! m_currentColumns.empty())
						UnbindColumns();

					LOG_INFO(boost::str(boost::format(u8"Executing '%s'") % command));
					auto start = std::chrono::high_resolution_clock::now();
					m_stmt.ExecuteDirect(command);
					auto end = std::chrono::high_resolution_clock::now();
					auto elapsed = end - start;
					auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
					LOG_INFO(boost::str(boost::format(u8"Success. Execution took %1%ms.")
						% millis.count()));
					// And bind columns, if there are any
					if (m_stmt.GetNrOfColumns() > 0)
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
		return 0;
	}


	void ExodbcExec::BindColumns()
	{
		exASSERT(m_currentColumns.empty());

		SQLSMALLINT nrOfCols = m_stmt.GetNrOfColumns();
		if (nrOfCols == 0)
		{
			LOG_WARNING(u8"No columns available in current result set.");
			return;
		}

		// Bind columns
		for (SQLSMALLINT colNr = 1; colNr <= nrOfCols; ++colNr)
		{
			SColumnDescription colDesc = m_stmt.DescribeColumn(colNr);
			LOG_DEBUG(boost::str(boost::format(u8"Binding column nr %d: name: '%s'; charsize: %d.") % colNr % colDesc.m_name % colDesc.m_charSize));
			// add +3 chars to charsize: 1 for '\0', 1 for '.' and 1 for '-':
#ifdef _WIN32
			WCharColumnBufferPtr pColBuffer = WCharColumnBuffer::Create(colDesc.m_charSize + 3, colDesc.m_name, colDesc.m_sqlType, ColumnFlag::CF_SELECT);
#else
			CharColumnBufferPtr pColBuffer = CharColumnBufferPtr::Create(colDesc.m_charSize + 3, colDesc.m_name, colDesc.m_sqlType, ColumnFlag::CF_SELECT);
#endif
			m_stmt.BindColumn(pColBuffer, colNr);
			StringColumnWrapper wrapper(pColBuffer);
			m_currentColumns.push_back(wrapper);
		}
	}


	void ExodbcExec::UnbindColumns()
	{
		LOG_DEBUG(boost::str(boost::format(u8"Unbinding %d columns") % m_currentColumns.size()));
		m_stmt.UnbindColumns();
		m_currentColumns.clear();
	}


	void ExodbcExec::Select(SelectMode mode)
	{
		switch (mode)
		{
		case SelectMode::First:
			m_stmt.SelectFirst();
			break;
		case SelectMode::Last:
			m_stmt.SelectLast();
			break;
		case SelectMode::Next:
			m_stmt.SelectNext();
			break;
		case SelectMode::Prev:
			m_stmt.SelectPrev();
			break;
		}
	}


	void ExodbcExec::Print(ExodbcExec::PrintMode mode)
	{
		if (m_currentColumns.empty())
		{
			LOG_WARNING(u8"No record set with bound columns is open.");
			return;
		}

		// print current or all
		if (!m_printNoHeader)
		{
			LOG_INFO(GetHeaderString(m_currentColumns));
		}
		if (mode == PrintMode::Current)
		{
			LOG_INFO(CurrentRecordToString(m_currentColumns));
		}
		else if (mode == PrintMode::All)
		{
			size_t rowCount = 1;

			bool haveNext = false;
			if (m_stmt.IsForwardOnlyCursor())
				haveNext = m_stmt.SelectNext();
			else
				haveNext = m_stmt.SelectFirst();
			while (haveNext)
			{
				stringstream ss;
				ss << u8"row: " << rowCount << m_columnSeparator;
				ss << CurrentRecordToString(m_currentColumns);
				LOG_INFO(ss.str());
				haveNext = m_stmt.SelectNext();
				++rowCount;
			}
			LOG_INFO(u8"No more results available");
		}
	}
	

	std::string ExodbcExec::CurrentRecordToString(const std::vector<exodbc::StringColumnWrapper>& columns) const
	{
		stringstream ss;
		std::vector<StringColumnWrapper>::const_iterator it = columns.begin();
		while (it != columns.end())
		{
			if (it->IsNull())
				ss << u8"NULL";
			else
				ss << it->GetValue<std::string>();
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
		while (it != columns.end())
		{
			ColumnBufferPtrVariant pCol = it->GetVariant();
			ss << boost::apply_visitor(QueryNameVisitor(), pCol);
			++it;
			if (it != columns.end())
				ss << m_columnSeparator;
		}
		return ss.str();
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
		WRITE_STDOUT_ENDL(u8" !commitTrans,!ct    Commit any ongoing transations.");
		WRITE_STDOUT_ENDL(u8" !rollbackTrans,!rt  Rollback all ongoing transactions.");

		WRITE_STDOUT_ENDL(u8" !help,!h            Show this help.");
	}
}