/*!
* \file CreateTracPages.h
* \author Elias Gerber <eg@elisium.ch>
* \date 07.05.2017
* \brief Header file for CreateTracPages
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
	// Typedefs
	// --------


	// Structs
	// -------

	// Classes
	// -------



	class CreateTracPages
		: public Command
	{
	public:
		enum class Mode
		{
			DbInfo,
			TestTables
		};

		CreateTracPages(Mode mode, exodbc::DatabasePtr pDb)
			: m_pDb(pDb)
			, m_mode(mode)
		{};

		virtual std::vector<std::string> GetAliases() const noexcept;
		virtual void Execute(const std::vector<std::string> & args);
		virtual std::string GetHelp() const noexcept;
		virtual std::string GetArgumentsSyntax() const noexcept;

	private:
		// Common things
		std::vector<std::string> GetHeaderLines();
		std::vector<std::string> GetConnectionLines();

		// DbInfo
		std::vector<std::string> GetDbInfoLines(exodbc::SqlInfoProperty::InfoType infoType);
		std::vector<std::string> GetTypeLines();

		// TableInfo

		// Helpers
		void AddEmptyLine(std::vector<std::string>& lines) const noexcept;

		exodbc::DatabasePtr m_pDb;
		Mode m_mode;
	};



}
