﻿/*!
* \file OdbcExec.h
* \author Elias Gerber <eg@elisium.ch>
* \date 31.03.2017
* \brief Header file for odbcexec
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "InputGenerator.h"
#include "Command.h"

// Other headers
#include "exodbc/Database.h"
#include "exodbc/ExecutableStatement.h"
#include "exodbc/ColumnBufferWrapper.h"

// System headers
#include <string>
#include <vector>
#include <set>

// Forward declarations
// --------------------

namespace exodbcexec
{
	// Typedefs
	// --------

	// Structs
	// -------

	// Classes
	// -------

	/*!
	* \class ExodbcExec
	* \brief An e
	*/
	class ExodbcExec
	{
	public:

		enum class CharColumnMode
		{
			Auto,
			Char,
			WChar
		};

		const static std::set<std::string> COMMAND_EXIT;
		const static std::set<std::string> COMMAND_HELP;
		const static std::set<std::string> COMMAND_PRINT_ALL;
		const static std::set<std::string> COMMAND_PRINT_CURRENT;
		const static std::set<std::string> COMMAND_SELECT_NEXT;
		const static std::set<std::string> COMMAND_SELECT_PREV;
		const static std::set<std::string> COMMAND_SELECT_FIRST;
		const static std::set<std::string> COMMAND_SELECT_LAST;
		const static std::set<std::string> COMMAND_COMMIT_TRANS;
		const static std::set<std::string> COMMAND_ROLLBACK_TRANS;
		const static std::set<std::string> COMMAND_FIND;
		const static std::set<std::string> COMMAND_LIST_TABLES;
		const static std::set<std::string> COMMAND_LIST_SCHEMAS;
		const static std::set<std::string> COMMAND_LIST_CATALOGS;

		ExodbcExec(exodbc::DatabasePtr pDb, bool exitOnError, bool forwardOnlyCursors, const std::string& columnSeparator, 
					bool printNoHeader, bool fixedPrintSize, bool printRowNr, CharColumnMode charColMode,
					SQLLEN charColSize,	const std::string& sqlSeparator);

		int Run(InputGeneratorPtr pInGen);

		enum class PrintMode
		{
			All,
			Current
		};

		enum class SelectMode
		{
			Next,
			Prev,
			First,
			Last
		};

		enum class ListMode
		{
			Types,
			Schemas,
			Catalogs
		};
		std::string ToString(ListMode mode) const noexcept;

		void PrintHelp();
		//void Print(PrintMode mode);
		//void Select(SelectMode mode);
		void List(ListMode mode);

	private:
		std::string CurrentRecordToString(const std::vector<exodbc::StringColumnWrapper>& columns) const;
		std::string GetHeaderString(const std::vector<exodbc::StringColumnWrapper>& columns) const;
		void BindColumns();
		void UnbindColumns();

		bool m_exitOnError;
		exodbc::DatabasePtr m_pDb;
		exodbc::ExecutableStatement m_stmt;
		bool m_forwardOnlyCursors;
		std::vector<exodbc::StringColumnWrapper> m_currentColumns;
		std::string m_columnSeparator;
		bool m_printNoHeader;
		bool m_fixedPrintSize;
		bool m_printRowNr;
		CharColumnMode m_charColumnMode;
		SQLLEN m_charColSize;
		std::string m_sqlSeparator;

		void CreateCommands();
		void RegisterCommand(CommandPtr pCommand);

		exodbc::ExecutableStatementPtr m_pStmt;

		CommandPtr GetCommand(const std::string& name, bool includeHidden = false) const;
		std::map<std::string, CommandPtr> m_commands;
	};
}