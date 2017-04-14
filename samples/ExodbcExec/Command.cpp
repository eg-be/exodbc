/*!
* \file Command.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 07.04.2017
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "Command.h"

// Same component headers
#include "ExodbcExec.h"

// Other headers
#include "exodbc/exOdbc.h"
#include "exodbc/ColumnBufferWrapper.h"
//#include "exodbc/InfoObject.h"
#include "exodbc/SqlInfoProperty.h"

// Debug
#include "DebugNew.h"

using namespace std;
using namespace exodbc;

namespace exodbcexec
{
	const std::string Command::COMMAND_PREFIX = u8"!";

	const std::string ExecuteSql::NAME = u8"execSql";

	void ExecuteSql::Execute(const std::vector<std::string>& args)
	{
		// Execute every argument as SQL:
		for (vector<string>::const_iterator it = args.begin(); it != args.end(); ++it)
		{
			const string& cmd = *it;
			if (cmd.empty())
				continue;

			LOG_INFO(boost::str(boost::format(u8"Executing '%s'") % cmd));
			auto start = std::chrono::high_resolution_clock::now();
			m_pStmt->ExecuteDirect(cmd);
			auto end = std::chrono::high_resolution_clock::now();
			auto elapsed = end - start;
			auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
			LOG_INFO(boost::str(boost::format(u8"Success. Execution took %1%ms.")
				% millis.count()));
		}
	}


	void Exit::Execute(const std::vector<std::string> & args)
	{
		exASSERT(m_pExodbcExec);
		m_pExodbcExec->RequestExit();
	}


	vector<string> Select::GetAliases() const noexcept
	{
		switch (m_mode)
		{
		case Mode::First:
			return{ u8"first", u8"sf" };
		case Mode::Last:
			return{ u8"last", u8"sl" };
		case Mode::Next:
			return{ u8"next", u8"sn" };
		case Mode::Prev:
			return{ u8"prev", u8"sp" };
		}
		exASSERT(false);
		return {};
	}


	string Select::GetHelp() const noexcept
	{
		switch (m_mode)
		{
		case Mode::First:
			return u8"Select first record.";
		case Mode::Last:
			return u8"Select last record.";
		case Mode::Next:
			return u8"Select next record.";
		case Mode::Prev:
			return u8"Select previous record.";
		}
		exASSERT(false);
		return{};
	}


	void Select::Execute(const std::vector<std::string> & args)
	{
		exASSERT(m_pStmt);
		bool res = false;
		switch (m_mode)
		{
		case Mode::First:
			res = m_pStmt->SelectFirst();
			break;
		case Mode::Last:
			res = m_pStmt->SelectLast();
			break;
		case Mode::Next:
			res = m_pStmt->SelectNext();
			break;
		case Mode::Prev:
			res = m_pStmt->SelectPrev();
			break;
		}
		if (!res)
		{
			LOG_WARNING(u8"No Record selected");
		}
	}


	void Commit::Execute(const std::vector<std::string>& args)
	{
		exASSERT(m_pDb);
		m_pDb->CommitTrans();
	}


	string Commit::GetHelp() const noexcept
	{
		return u8"Commit any ongoing transations.";
	}


	string Rollback::GetHelp() const noexcept
	{
		return u8"Rollback all ongoing transactions.";
	}


	void Rollback::Execute(const std::vector<std::string>& args)
	{
		exASSERT(m_pDb);
		m_pDb->RollbackTrans();
	}


	vector<string> Print::GetAliases() const noexcept
	{
		switch (m_mode)
		{
		case Mode::CurrentRecord:
			return{ u8"printCurrent", u8"pc" };
		case Mode::AllRecords:
			return{ u8"printAll", u8"pa" };
		}
		exASSERT(false);
		return{};
	}


	string Print::GetHelp() const noexcept
	{
		switch (m_mode)
		{
		case Mode::CurrentRecord:
			return u8"Print the current record.";
		case Mode::AllRecords:
			return	u8"Print all records of the current record set. "
					u8"If forward only cursors is set to false, !printAll "
					u8"will first execute a '!first' and then print and "
					u8"iterate all records by calling '!next'. "
					u8"If forward only cursors is set to true, all "
					u8"remaining records found using '!next' are printed.";
		}
		exASSERT(false);
		return{};
	}


	void Print::Execute(const std::vector<std::string> & args)
	{
		if (m_columns.empty())
		{
			LOG_WARNING(u8"No record set with bound columns is open.");
			return;
		}

		if (m_printHeaderRow)
		{
			vector<string> headers = GetHeaderRows();
			for (vector<string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
			{
				LOG_OUTPUT(*it);
			}
		}

		if (m_mode == Mode::CurrentRecord)
		{
			LOG_OUTPUT(GetRecordRow(1));
		}
		else
		{
			size_t rowCount = 1;

			bool haveNext = false;
			if (m_pStmt->IsForwardOnlyCursor())
				haveNext = m_pStmt->SelectNext();
			else
				haveNext = m_pStmt->SelectFirst();
			while (haveNext)
			{
				LOG_OUTPUT(GetRecordRow(rowCount));
				haveNext = m_pStmt->SelectNext();
				++rowCount;
			}
			LOG_INFO(u8"No more results available.");
		}
	}


	vector<string> Print::GetHeaderRows() const noexcept
	{
		vector<string> rows;
		string s1 = GetHeaderRow();
		string s2(s1.length(), '=');
		rows.push_back(s1);
		rows.push_back(s2);
		return rows;
	}


	string Print::GetHeaderRow() const noexcept
	{
		stringstream ss;
		if (m_printRowNr)
		{
			stringstream ssRowColFormat;
			if (m_fixedPrintSize && m_fixedPrintSizeWidth > 0)
				ssRowColFormat << u8"%" << m_fixedPrintSizeWidth << u8"s";
			else if (m_fixedPrintSize && m_fixedPrintSizeWidth == 0)
				ssRowColFormat << u8"%" << DEFAULT_ROWNR_WIDTH << u8"s";
			else
				ssRowColFormat << u8"%s";
			string rowColFormat = ssRowColFormat.str();
			ss << boost::str(boost::format(rowColFormat) % u8"ROW");
			ss << m_columnSeparator;
		}

		std::vector<StringColumnWrapper>::const_iterator it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant pCol = it->GetVariant();
			string sheader = boost::apply_visitor(QueryNameVisitor(), pCol);
			if (m_fixedPrintSize)
			{
				SQLLEN nrOfElements = boost::apply_visitor(NrOfElementsVisitor(), pCol);
				SQLLEN nrOfHeaderChars = (SQLLEN)sheader.length();
				// note that one element added was the terminating '\0', do not add that to print
				SQLLEN printSize = max(nrOfElements - 1, nrOfHeaderChars);
				exASSERT(printSize >= 1);
				stringstream ssColFormat;
				ssColFormat << u8"%" << printSize << "s";
				sheader = boost::str(boost::format(ssColFormat.str()) % sheader);
			}
			ss << sheader;

			++it;
			if (it != m_columns.end())
				ss << m_columnSeparator;
		}
		return ss.str();
	}


	std::string Print::CurrentRecordToString() const
	{
		stringstream ss;
		std::vector<StringColumnWrapper>::const_iterator it = m_columns.begin();
		while (it != m_columns.end())
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
				SQLLEN nrOfHeaderChars = (SQLLEN)sheader.length();
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
			if (it != m_columns.end())
				ss << m_columnSeparator;
		}
		return ss.str();
	}


	std::string Print::GetRecordRow(size_t rowNr) const
	{
		if (!m_printRowNr)
			return CurrentRecordToString();

		stringstream ss;
		stringstream ssRowColFormat;
		if (m_fixedPrintSize && m_fixedPrintSizeWidth > 0)
			ssRowColFormat << u8"%" << m_fixedPrintSizeWidth << u8"d";
		else if (m_fixedPrintSize && m_fixedPrintSizeWidth == 0)
			ssRowColFormat << u8"%" << DEFAULT_ROWNR_WIDTH << u8"d";
		else
			ssRowColFormat << u8"%d";
		ss << boost::str(boost::format(ssRowColFormat.str()) % rowNr);
		ss << m_columnSeparator;
		ss << CurrentRecordToString();
		return ss.str();
	}


	void Help::Execute(const std::vector<std::string>& args)
	{
		stringstream ss;
		ss << u8"The following commands are available:";
		Write(ss);
		// print commands:
		for (set<CommandPtr>::const_iterator it = m_commands.begin(); it != m_commands.end(); ++it)
		{
			CommandPtr pCommand = *it;
			Write(pCommand);
		}
		ss <<	u8"Commands can be abbreviated. For example the command 'Exit SQL "
				u8"execution', documented as '!exit,!e,!q', can be invoked using '!exit' "
				u8" or '!e' or '!q'.";
		Write(ss);
		ss <<	u8"Any input that is not recognized as a command will be executed as SQL "
				u8"against the database connected to.";
		Write(ss);
	}


	void Help::Write(CommandPtr pCommand, size_t maxChars /* = DEFAULT_MAXCHARS */) const noexcept
	{
		exASSERT(pCommand);
		size_t maxAliasesWidth = 20;
		vector<string> lines = Split(pCommand->GetHelp(), maxChars, maxAliasesWidth);
		// modify and add alias to the first line
		exASSERT(!lines.empty());
		vector<string> aliases = pCommand->GetAliases();
		string saliases = u8" ";
		vector<string>::const_iterator it = aliases.begin();
		while (it != aliases.end())
		{
			saliases += Command::COMMAND_PREFIX;
			saliases += *it;
			++it;
			if (it != aliases.end())
				saliases += u8",";
		}
		stringstream ssf;
		ssf << u8"%-" << maxAliasesWidth << u8"s%s";
		string firstLineTrimed = boost::trim_copy(lines[0]);
		lines[0] = boost::str(boost::format(ssf.str()) % saliases % firstLineTrimed );
		for (vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			WRITE_STDOUT_ENDL(*it);
		}
	}


	void Help::Write(const std::string& str, size_t maxChars /* = DEFAULT_MAXCHARS */) const noexcept
	{
		vector<string> lines = Split(str, maxChars);
		for (vector<string>::const_iterator it = lines.begin(); it != lines.end(); ++it)
		{
			WRITE_STDOUT_ENDL(*it);
		}
	}


	void Help::Write(std::stringstream& ss, size_t maxChars /* = DEFAULT_MAXCHARS */) const noexcept
	{
		Write(ss.str());
		ss.swap(stringstream());
	}


	vector<string> Help::Split(const std::string& str, size_t maxChars, size_t indent /* = 0 */) const noexcept
	{
		vector<string> lines;
		string tmp;
		string sindent(indent, ' ');
		tmp += sindent;
		// tokenize into words
		vector<string> words;
		boost::split(words, str, boost::is_any_of(" "));
		bool firstWord = true;
		for (vector<string>::const_iterator it = words.begin(); it != words.end(); ++it)
		{
			const string& word = *it;
			if (tmp.length() + (firstWord ? 0 : 1) + word.length() <= maxChars)
			{
				if (firstWord)
					firstWord = false;
				else
					tmp += u8" ";
				tmp += word;
			}
			else
			{
				lines.push_back(tmp);
				tmp = sindent;
				tmp += *it;
			}
		}
		if (!tmp.empty())
		{
			lines.push_back(tmp);
		}
		return lines;
	}


	void DbInfo::Execute(const std::vector<std::string> & args)
	{
		SqlInfoProperties props;
		props.Init(m_pDb->GetSqlDbcHandle());
		auto dbms = props.GetProperties(SqlInfoProperty::InfoType::DBMS);
		LOG_OUTPUT(u8"DBMS Product Information");
		LOG_OUTPUT(u8"========================");
		for (auto it = dbms.begin(); it != dbms.end(); ++it)
		{
			LOG_OUTPUT(boost::str(boost::format(u8"%-30s: %s") % it->GetName() % it->GetStringValue()));
		}
		auto dataSource = props.GetProperties(SqlInfoProperty::InfoType::DataSource);
		LOG_OUTPUT(u8"Data Source Information");
		LOG_OUTPUT(u8"=======================");
		for (auto it = dataSource.begin(); it != dataSource.end(); ++it)
		{
			LOG_OUTPUT(boost::str(boost::format(u8"%-30s: %s") % it->GetName() % it->GetStringValue()));
		}
	}
}