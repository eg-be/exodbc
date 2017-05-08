/*!
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

		ExodbcExec(exodbc::DatabasePtr pDb, bool exitOnError, bool forwardOnlyCursors, const std::string& columnSeparator, 
					bool printNoHeader, bool fixedPrintSize, bool printRowNr, CharColumnMode charColMode,
					SQLLEN charColSize,	const std::string& sqlSeparator);

		int Run(InputGeneratorPtr pInGen);

//		void PrintHelp();
//		void List(ListMode mode);

		void RequestExit() { m_exitFlag = true; };

	private:
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

		void RegisterCommand(CommandPtr pCommand);
		CommandPtr GetCommand(const std::string& name, bool includeHidden = false) const;
		std::set<CommandPtr> GetCommands() const noexcept;

		exodbc::ExecutableStatementPtr m_pStmt;

		std::map<std::string, CommandPtr> m_commands;

		bool m_exitFlag;
	};
}
