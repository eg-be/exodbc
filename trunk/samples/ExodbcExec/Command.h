﻿/*!
* \file Command.h
* \author Elias Gerber <eg@elisium.ch>
* \date 07.04.2017
* \brief Header file for CommandGenerator
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
// Other headers
#include "exodbc/ColumnBufferWrapper.h"
#include "exodbc/ExecutableStatement.h"

// System headers
#include <string>
#include <set>
#include <functional>
#include <vector>
#include <memory>

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
	* \class Command
	* \brief A single command
	*/
	class Command
	{
	public:
		static const std::string COMMAND_PREFIX;

		virtual std::vector<std::string> GetAliases() const noexcept = 0;

		virtual void Execute(const std::vector<std::string>& args) = 0;

		virtual bool Hidden() const noexcept { return false; };

		virtual std::string GetHelp() const noexcept = 0;

		//bool operator< (const Command& c) const
		//{
		//	const std::string& a1 = GetAliases().size() >= 1 ? *(GetAliases().begin()) : u8"";
		//	const std::string& c1 = c.GetAliases().size() >= 1 ? *(c.GetAliases().begin()) : u8"";
		//	return a1 < c1;
		//}
	};

	typedef std::shared_ptr<Command> CommandPtr;


	class ExecuteSql
		: public Command
	{
	public:

		static const std::string NAME;

		ExecuteSql(exodbc::ExecutableStatementPtr pStmt)
			: m_pStmt(pStmt)
		{
			exASSERT(m_pStmt);
			exASSERT(m_pStmt->IsInitialized());
		};

		virtual std::vector<std::string> GetAliases() const noexcept { return {NAME}; };

		virtual void Execute(const std::vector<std::string>& args);

		virtual bool Hidden() const noexcept { return true; };
		virtual std::string GetHelp() const noexcept { return u8"Execute SQL."; };

	private:
		exodbc::ExecutableStatementPtr m_pStmt;
	};


	class Select
		: public Command
	{
	public:
		enum class Mode
		{
			Next,
			Prev,
			First,
			Last
		};

		Select(Mode mode, exodbc::ExecutableStatementPtr pStmt)
			: m_pStmt(pStmt)
			, m_mode(mode)
		{};

		virtual std::vector<std::string> GetAliases() const noexcept;
		virtual void Execute(const std::vector<std::string> & args);
		virtual std::string GetHelp() const noexcept;

	private:
		Mode m_mode;
		exodbc::ExecutableStatementPtr m_pStmt;
	};
	typedef std::shared_ptr<Select> SelectPtr;


	class Commit
		: public Command
	{
	public:
		Commit(exodbc::DatabasePtr pDb)
			: m_pDb(pDb)
		{};
		virtual std::vector<std::string> GetAliases() const noexcept { return{ u8"commitTrans", u8"ct" }; };
		virtual void Execute(const std::vector<std::string>& args);
		virtual std::string GetHelp() const noexcept;

	private:
		exodbc::DatabasePtr m_pDb;
	};


	class Rollback
		: public Command
	{
	public:
		Rollback(exodbc::DatabasePtr pDb)
			: m_pDb(pDb)
		{};

		virtual std::vector<std::string> GetAliases() const noexcept { return{ u8"rollbackTrans", u8"rt" }; };
		virtual void Execute(const std::vector<std::string>& args);
		virtual std::string GetHelp() const noexcept;

	private:
		exodbc::DatabasePtr m_pDb;
	};


	class Help
		: public Command
	{
	public:
		Help(const std::set<CommandPtr>& cmds)
		{
			m_commands.insert(cmds.begin(), cmds.end());
		};

		virtual std::vector<std::string> GetAliases() const noexcept { return{ u8"help", u8"h" }; };
		virtual void Execute(const std::vector<std::string>& args);
		virtual std::string GetHelp() const noexcept { return u8"Show this help text."; };

	private:
		std::vector<std::string> Split(const std::string& input, size_t maxChars = 79) const noexcept;
		void Write(const std::string& str) const noexcept;
		void Write(std::stringstream& ss) const noexcept;
		
		struct SOrderCommands
		{
			bool operator()(CommandPtr pCmd1, CommandPtr pCmd2) const
			{
				const std::vector<std::string>& c1Aliases = pCmd1->GetAliases();
				const std::vector<std::string>& c2Aliases = pCmd2->GetAliases();
				const std::string c1Ali = c1Aliases.size() > 0 ? c1Aliases[0] : u8"";
				const std::string c2Ali = c2Aliases.size() > 0 ? c2Aliases[0] : u8"";
				return c1Ali < c2Ali;
			}
		};

		std::set<CommandPtr, SOrderCommands> m_commands;
	};


	class Print
		: public Command
	{
	public:
		enum class Mode
		{
			CurrentRecord,
			AllRecords
		};

		Print(Mode mode, const std::vector<exodbc::StringColumnWrapper>& columns, exodbc::ExecutableStatementPtr pStmt,
			const std::string& columnSeparator,
			bool printHeaderRow, bool printRowNr, bool fixedPrintSize, size_t fixedPrintSizeWidth)
			: m_mode(mode)
			, m_columns(columns)
			, m_pStmt(pStmt)
			, m_columnSeparator(columnSeparator)
			, m_printHeaderRow(printHeaderRow)
			, m_printRowNr(printRowNr)
			, m_fixedPrintSize(fixedPrintSize)
			, m_fixedPrintSizeWidth(fixedPrintSizeWidth)
		{};

		virtual std::vector<std::string> GetAliases() const noexcept;
		virtual void Execute(const std::vector<std::string> & args);
		virtual std::string GetHelp() const noexcept;

	private:
		static const size_t DEFAULT_ROWNR_WIDTH = 10;

		std::vector<std::string> GetHeaderRows() const noexcept;
		std::string GetHeaderRow() const noexcept;
		std::string CurrentRecordToString() const;
		std::string GetRecordRow(size_t rowNr) const;

		Mode m_mode;
		const std::vector<exodbc::StringColumnWrapper>& m_columns;
		exodbc::ExecutableStatementPtr m_pStmt;
		bool m_printHeaderRow;
		bool m_printRowNr;
		bool m_fixedPrintSize;
		size_t m_fixedPrintSizeWidth;
		std::string m_columnSeparator;
	};
}
