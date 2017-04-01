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

		const static std::set<std::string> COMMAND_EXIT;
		const static std::set<std::string> COMMAND_HELP;
		const static std::set<std::string> COMMAND_PRINT;
		const static std::set<std::string> COMMAND_SELECT_NEXT;
		const static std::set<std::string> COMMAND_SELECT_PREV;
		const static std::set<std::string> COMMAND_SELECT_FIRST;
		const static std::set<std::string> COMMAND_SELECT_LAST;

		ExodbcExec(exodbc::DatabasePtr pDb, bool exitOnError);

		int Run(InputGeneratorPtr pInGen);

		enum  class PrintMode
		{
			All,
			Current
		};

		void PrintHelp();
		void Print(PrintMode mode);

	private:
		std::string PrintCurrentRecord(const std::vector<exodbc::StringColumnWrapper>& columns) const;

		bool m_exitOnError;
		exodbc::DatabasePtr m_pDb;
		exodbc::ExecutableStatement m_stmt;
	};
}
