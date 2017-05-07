/*!
* \file CreateTestDbPageCommand.h
* \author Elias Gerber <eg@elisium.ch>
* \date 07.05.2017
* \brief Header file for CreateTestDbPageCommand
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "Command.h"

// Other headers
#include "exodbc/ColumnBufferWrapper.h"
#include "exodbc/ExecutableStatement.h"
#include "exodbc/SqlInfoProperty.h"

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
	class ExodbcExec;

	// Typedefs
	// --------


	// Structs
	// -------

	// Classes
	// -------



	class CreateTestDbPageCommand
		: public Command
	{
	public:
		CreateTestDbPageCommand(exodbc::DatabasePtr pDb)
			: m_pDb(pDb)
		{};

		virtual std::vector<std::string> GetAliases() const noexcept { return{u8"createInfoPage", u8"cip"}; };
		virtual void Execute(const std::vector<std::string> & args);
		virtual std::string GetHelp() const noexcept { return u8"Outputs trac markup to create a page about the connected Test-Database.";	};
		virtual std::string GetArgumentsSyntax() const noexcept;

	private:
		std::vector<std::string> GetDbInfoLines(exodbc::SqlInfoProperty::InfoType infoType);
		std::string GetNameHeader();
		std::string GetPropertyTableNameHeader();

		void AddEmptyLine(std::vector<std::string>& lines) const noexcept;

		exodbc::DatabasePtr m_pDb;
	};



}
