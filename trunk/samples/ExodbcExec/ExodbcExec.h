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

// System headers
#include <string>
#include <vector>

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

		const static std::string COMMAND_EXIT;
		const static std::string COMMAND_EXIT_SHORT;
		const static std::string COMMAND_HELP;
		const static std::string COMMAND_HELP_SHORT;
		const static std::string COMMAND_PRINT;
		const static std::string COMMAND_PRINT_SHORT;

		ExodbcExec(exodbc::DatabasePtr pDb, bool exitOnError);

		int Run(InputGeneratorPtr pInGen);

		void PrintHelp();
		void PrintAll();

	private:
		bool m_exitOnError;
		exodbc::DatabasePtr m_pDb;
		exodbc::ExecutableStatement m_stmt;
	};
}
