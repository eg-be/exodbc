/*!
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
		virtual std::set<std::string> GetAliases() const noexcept = 0;

		virtual void Execute(const std::vector<std::string>& args) = 0;

		virtual bool Hidden() const noexcept { return false; };
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

		virtual std::set<std::string> GetAliases() const noexcept { return {NAME}; };

		virtual void Execute(const std::vector<std::string>& args);

		virtual bool Hidden() const noexcept { return true; };

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

		virtual std::set<std::string> GetAliases() const noexcept;
		virtual void Execute(const std::vector<std::string> & args);

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

		virtual std::set<std::string> GetAliases() const noexcept { return{ u8"commitTrans", u8"ct" }; };
		virtual void Execute(const std::vector<std::string>& args);

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

		virtual std::set<std::string> GetAliases() const noexcept { return{ u8"rollbackTrans", u8"rt" }; };
		virtual void Execute(const std::vector<std::string>& args);

	private:
		exodbc::DatabasePtr m_pDb;
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

		virtual std::set<std::string> GetAliases() const noexcept;
		virtual void Execute(const std::vector<std::string> & args);

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
