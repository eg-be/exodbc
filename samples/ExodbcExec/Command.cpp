﻿/*!
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
#include "exodbc/SqlInfoProperty.h"
#include <boost/tokenizer.hpp>

// Debug
#include "DebugNew.h"

using namespace std;
using namespace exodbc;

namespace exodbcexec
{
	const std::string Command::COMMAND_PREFIX = u8"!";

	const std::string ExecuteSql::NAME = u8"execSql";


	std::chrono::milliseconds Command::ExecuteTimed(std::function<void()> func)
	{
		auto start = std::chrono::high_resolution_clock::now();
		func();
		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = end - start;
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

		return millis;
	}


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
		// or, if the command has arguments, add the arguments syntax
		// to the first line and continue with the help on the second line:
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
		if (pCommand->HasArguments())
		{
			string firstLine = boost::str(boost::format(ssf.str()) % saliases % pCommand->GetArgumentsSyntax());
			lines.insert(lines.begin(), firstLine);
		}
		else
		{
			string firstLineTrimed = boost::trim_copy(lines[0]);
			lines[0] = boost::str(boost::format(ssf.str()) % saliases % firstLineTrimed);
		}
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
		stringstream ss2;
		ss.swap(ss2);
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
		auto dbms = props.GetSubset(SqlInfoProperty::InfoType::DBMS);
		LOG_OUTPUT(u8"");
		LOG_OUTPUT(u8"DBMS Product Information");
		LOG_OUTPUT(u8"========================");
		for (auto it = dbms.begin(); it != dbms.end(); ++it)
		{
			LOG_OUTPUT(boost::str(boost::format(u8"%-40s: %s") % it->GetName() % it->GetStringValue()));
		}
		auto dataSource = props.GetSubset(SqlInfoProperty::InfoType::DataSource);
		LOG_OUTPUT(u8"");
		LOG_OUTPUT(u8"Data Source Information");
		LOG_OUTPUT(u8"=======================");
		for (auto it = dataSource.begin(); it != dataSource.end(); ++it)
		{
			LOG_OUTPUT(boost::str(boost::format(u8"%-40s: %s") % it->GetName() % it->GetStringValue()));
		}
		auto driver = props.GetSubset(SqlInfoProperty::InfoType::Driver);
		LOG_OUTPUT(u8"");
		LOG_OUTPUT(u8"Driver Information");
		LOG_OUTPUT(u8"==================");
		for (auto it = driver.begin(); it != driver.end(); ++it)
		{
			LOG_OUTPUT(boost::str(boost::format(u8"%-40s: %s") % it->GetName() % it->GetStringValue()));
		}
		auto supportedSql = props.GetSubset(SqlInfoProperty::InfoType::SupportedSql);
		LOG_OUTPUT(u8"");
		LOG_OUTPUT(u8"Supported SQL Information");
		LOG_OUTPUT(u8"=========================");
		for (auto it = supportedSql.begin(); it != supportedSql.end(); ++it)
		{
			LOG_OUTPUT(boost::str(boost::format(u8"%-40s: %s") % it->GetName() % it->GetStringValue()));
		}
		auto sqlLimits = props.GetSubset(SqlInfoProperty::InfoType::SqlLimits);
		LOG_OUTPUT(u8"");
		LOG_OUTPUT(u8"SQL Limits Information");
		LOG_OUTPUT(u8"======================");
		for (auto it = sqlLimits.begin(); it != sqlLimits.end(); ++it)
		{
			LOG_OUTPUT(boost::str(boost::format(u8"%-40s: %s") % it->GetName() % it->GetStringValue()));
		}
		auto scalarFunction = props.GetSubset(SqlInfoProperty::InfoType::ScalarFunction);
		LOG_OUTPUT(u8"");
		LOG_OUTPUT(u8"Scalar Function Information");
		LOG_OUTPUT(u8"===========================");
		for (auto it = scalarFunction.begin(); it != scalarFunction.end(); ++it)
		{
			LOG_OUTPUT(boost::str(boost::format(u8"%-40s: %s") % it->GetName() % it->GetStringValue()));
		}
		auto conversion = props.GetSubset(SqlInfoProperty::InfoType::Conversion);
		LOG_OUTPUT(u8"");
		LOG_OUTPUT(u8"Conversion Information");
		LOG_OUTPUT(u8"======================");
		for (auto it = conversion.begin(); it != conversion.end(); ++it)
		{
			LOG_OUTPUT(boost::str(boost::format(u8"%-40s: %s") % it->GetName() % it->GetStringValue()));
		}
	}


	vector<string> ListCatalog::GetAliases() const noexcept
	{
		switch (m_mode)
		{
		case Mode::TableTypes:
			return{ u8"listTypes", u8"lt" };
		case Mode::Schemas:
			return{ u8"listSchemas", u8"ls" };
		case Mode::Catalogs:
			return{ u8"listCatalogs", u8"lc" };
		}
		exASSERT(false);
		return{};
	}


	string ListCatalog::GetHelp() const noexcept
	{
		switch (m_mode)
		{
		case Mode::TableTypes:
			return u8"List all table types.";
		case Mode::Schemas:
			return u8"List all schemas.";
		case Mode::Catalogs:
			return u8"List all catalogs.";
		}
		exASSERT(false);
		return{};
	}


	void ListCatalog::Execute(const std::vector<std::string> & args)
	{
		string modes = u8"Table Types";
		if (m_mode == Mode::Catalogs)
			modes = u8"Catalogs";
		else if (m_mode == Mode::Schemas)
			modes = u8"Schemas";
		vector<string> data;
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();
		LOG_INFO(boost::str(boost::format(u8"Listing all %s ..") % modes));
		auto start = std::chrono::high_resolution_clock::now();
		switch (m_mode)
		{
		case Mode::TableTypes:
			data = pDbCat->ListTableTypes();
			break;
		case Mode::Schemas:
			data = pDbCat->ListSchemas();
			break;
		case Mode::Catalogs:
			data = pDbCat->ListCatalogs();
			break;
		}
		auto end = std::chrono::high_resolution_clock::now();
		auto elapsed = end - start;
		auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);
		LOG_INFO(boost::str(boost::format(u8"Success, found %d %s. Execution took %dms.")
			% data.size() % modes % millis.count()));

		if (!data.empty())
		{
			if (m_printHeaderRow)
			{
				LOG_OUTPUT(boost::str(boost::format(modes)));
				string s(modes.length(), '=');
				LOG_OUTPUT(s);
			}
			for (vector<string>::const_iterator it = data.begin(); it != data.end(); ++it)
			{
				LOG_OUTPUT(*it);
			}
		}
	}


	void Find::Execute(const std::vector<std::string> & args)
	{
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();
		string name = u8"%";
		string type, cat, schem;
		bool haveType = false;
		bool haveCat = false;
		bool haveSchem = false;
		bool printColumns = false;
		if(!args.empty())
			printColumns = args.back() == u8"-pc";
		if (m_mode == Mode::Short)
		{
			haveType = args.size() >= (size_t)(printColumns ? 5 : 4);
			haveCat = args.size() >= (size_t)(printColumns ? 4 : 3);
			haveSchem = args.size() >= (size_t)(printColumns ?  3 : 2);
			if (args.size() >= (size_t)(printColumns ? 2 : 1))
				name = args[0];
			if (haveType)
				type = args[3];
			if (haveCat)
				cat = args[2];
			if (haveSchem)
				schem = args[1];
		}
		else
		{
			StdInGenerator in;
			LOG_OUTPUT(u8"Enter Name pattern: ");
			in.GetNextCommand(name);
			LOG_OUTPUT(u8"Enter Schema pattern or 'NULL' to ignore: ");
			in.GetNextCommand(schem);
			LOG_OUTPUT(u8"Enter Catalog pattern or 'NULL' to ignore: ");
			in.GetNextCommand(cat);
			LOG_OUTPUT(u8"Enter Type(s), separated by ',': ");
			in.GetNextCommand(type);
			if (schem != u8"NULL")
				haveSchem = true;
			if (cat != u8"NULL")
				haveCat = true;
			haveType = true;
		}

		LOG_INFO(boost::str(boost::format(u8"Searching using name: '%s', schema: '%s', catalog: '%s', type: '%s'") % name
			% (haveSchem ? schem : u8"NULL")
			% (haveCat ? cat : u8"NULL")
			% (haveType ? type : u8"NULL")));
		TableInfoVector tables;
		auto millis = ExecuteTimed([&]()
		{
			if (haveSchem && haveCat)
				tables = pDbCat->SearchTables(name, schem, cat, type);
			else if (haveSchem)
				tables = pDbCat->SearchTables(name, schem, DatabaseCatalog::SchemaOrCatalogType::Schema, type);
			else if (haveCat)
				tables = pDbCat->SearchTables(name, schem, DatabaseCatalog::SchemaOrCatalogType::Catalog, type);
			else
				tables = pDbCat->SearchTables(name, type);
		});
		LOG_INFO(boost::str(boost::format(u8"Success, found %d Tables. Execution took %dms.")
			% tables.size() % millis.count()));

		boost::format f(u8"%18s");
		if (!tables.empty() && m_printHeaderRow)
		{
			vector<string> headers = GetTableHeaderRows();
			for (vector<string>::const_iterator it = headers.begin(); it != headers.end(); ++it)
			{
				LOG_OUTPUT(*it);
			}
		}
		size_t rowNr = 0;
		for (TableInfoVector::const_iterator it = tables.begin(); it != tables.end(); ++it)
		{
			const TableInfo& ti = *it;
			LOG_OUTPUT(GetTableRecordRow(ti, rowNr));
			++rowNr;
		}
		if (printColumns)
		{
			for (TableInfoVector::const_iterator it = tables.begin(); it != tables.end(); ++it)
			{
				const TableInfo& ti = *it;
				ColumnInfoVector columns;
				LOG_INFO(boost::str(boost::format(u8"Reading Column Information for '%s'..") % ti.GetQueryName()));
				auto millis = ExecuteTimed([&]()
				{
					columns = pDbCat->ReadColumnInfo(ti);
				});
				LOG_INFO(boost::str(boost::format(u8"Success, found %d Columns. Execution took %dms.")
					% columns.size() % millis.count()));

				LOG_INFO(boost::str(boost::format(u8"Columns of '%s':") % ti.GetQueryName()));
				if (!columns.empty())
				{
					for (ColumnInfoVector::const_iterator it = columns.begin(); it != columns.end(); ++it)
					{
						const ColumnInfo& colInf = *it;
						size_t pos = it - columns.begin();
						LOG_OUTPUT(boost::str(boost::format(u8" [%d] Column Name:              %s") % pos % colInf.GetColumnName()));
						LOG_OUTPUT(boost::str(boost::format(u8" [%d] SQL Type:                 %d (%s)") % pos % colInf.GetSqlType() % Sql2StringHelper::SqlType2s(colInf.GetSqlType())));
						LOG_OUTPUT(boost::str(boost::format(u8" [%d] SQL Data Type [ODBC 3.0]: %d (%s)") % pos % colInf.GetSqlDataType() % Sql2StringHelper::SqlType2s(colInf.GetSqlDataType())));
						LOG_OUTPUT(boost::str(boost::format(u8" [%d] Type Name:                %s") % pos % colInf.GetTypeName()));
						LOG_OUTPUT(boost::str(boost::format(u8" [%d] Column Size:              %d") % pos % colInf.GetColumnSize()));
						LOG_OUTPUT(boost::str(boost::format(u8" [%d] Decimal Digits:           %d") % pos % colInf.GetDecimalDigits()));
						LOG_OUTPUT(boost::str(boost::format(u8" [%d] Nullable:                 %d (%s)") % pos % colInf.GetNullable() % Sql2StringHelper::SqlNullable2s(colInf.GetNullable())));
					}
				}
			}
		}
	}


	vector<string> Find::GetTableHeaderRows() const noexcept
	{
		stringstream ss;
		if (m_printRowNr)
		{
			stringstream ssRowColFormat;
			if (m_fixedPrintSize)
				ssRowColFormat << u8"%-" << DEFAULT_ROWNR_WIDTH << u8"s";
			else
				ssRowColFormat << u8"%s";
			string rowColFormat = ssRowColFormat.str();
			ss << boost::str(boost::format(rowColFormat) % u8"ROW");
			ss << m_columnSeparator;
		}

		int w = DEFAULT_FIELD_WIDTH;
		stringstream ssColNameFormat;
		if (m_fixedPrintSize)
		{
			ssColNameFormat << "%-" << w << "s" << m_columnSeparator << "%-" << w << "s" << m_columnSeparator;
			ssColNameFormat << "%-" << w << "s" << m_columnSeparator << "%-" << DEFAULT_TYPE_WIDTH << "s";
		}
		else
		{
			ssColNameFormat << "%s" << m_columnSeparator << "%s" << m_columnSeparator << "%s" << m_columnSeparator << "%s";
		}
		string cat = u8"Catalog";
		string schem = u8"Schema";
		string name = u8"Name";
		string type = u8"Type";
		string format = ssColNameFormat.str();
		ss << boost::str(boost::format(format) % cat % schem % name % type);

		string header1 = ss.str();
		vector<string> rows;
		rows.push_back(header1);
		rows.push_back(string(header1.length(), '='));
		return rows;
	}


	std::string Find::GetTableRecordRow(const TableInfo& ti, size_t rowNr) const noexcept
	{
		stringstream ss;
		if (m_printRowNr)
		{
			stringstream ssRowColFormat;
			if (m_fixedPrintSize)
				ssRowColFormat << u8"%" << DEFAULT_ROWNR_WIDTH << u8"d";
			else
				ssRowColFormat << u8"%d";
			string rowColFormat = ssRowColFormat.str();
			ss << boost::str(boost::format(rowColFormat) % rowNr);
			ss << m_columnSeparator;
		}

		int w = DEFAULT_FIELD_WIDTH;
		stringstream ssColNameFormat;
		if (m_fixedPrintSize)
		{
			ssColNameFormat << "%-" << w << "s" << m_columnSeparator << "%-" << w << "s" << m_columnSeparator;
			ssColNameFormat << "%-" << w << "s" << m_columnSeparator << "%-" << DEFAULT_TYPE_WIDTH << "s";
		}
		else
		{
			ssColNameFormat << "%s" << m_columnSeparator << "%s" << m_columnSeparator << "%s" << m_columnSeparator << "%s";
		}
		string format = ssColNameFormat.str();
		ss << boost::str(boost::format(format) % ti.GetCatalog() % ti.GetSchema() % ti.GetName() % ti.GetType());

		return ss.str();
	}


	vector<string> Find::GetAliases() const noexcept
	{
		switch (m_mode)
		{
		case Mode::Short:
			return { u8"find", u8"f" };
		case Mode::Interactive:
			return { u8"ifind", u8"if" };
		}
		exASSERT(false);
		return{};
	}


	std::string Find::GetArgumentsSyntax() const noexcept
	{ 
		switch (m_mode)
		{
		case Mode::Short:
			return u8"name [schema] [catalog] [type] [-pc]";
		case Mode::Interactive:
			return u8"[-pc]";
		}
		exASSERT(false);
		return u8"";
	}


	std::string Find::GetHelp() const noexcept
	{
		switch (m_mode)
		{
		case Mode::Short:
			return	u8"Search for tables, views, etc."
					u8"If any argument of schema, catalog or type is empty, the argument is "
					u8"ignored. Use '%' to match zero or more characters and '%' to match "
					u8"any single character. If argument '-pc' is passed, the column information "
					u8"for any found table is printed after the list of tables has been printed.";
		case Mode::Interactive:
			return	u8"Search for tables, views, etc."
					u8"Values for table, schema, catalog and type name are queried interactive."
					u8"If argument '-pc' is passed, the column information "
					u8"for any found table is printed after the list of tables has been printed.";
		}
		exASSERT(false);
		return u8"";
	}
}