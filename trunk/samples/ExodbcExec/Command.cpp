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
// Other headers
#include "exodbc/exOdbc.h"
#include "exodbc/ColumnBufferWrapper.h"

// Debug
#include "DebugNew.h"

using namespace std;
using namespace exodbc;

namespace exodbcexec
{
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


	set<string> Select::GetAliases() const noexcept
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


	void Rollback::Execute(const std::vector<std::string>& args)
	{
		exASSERT(m_pDb);
		m_pDb->RollbackTrans();
	}


	set<string> Print::GetAliases() const noexcept
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
}