/*!
* \file ExecuteDirectStatement.h
* \author Elias Gerber <eg@elisium.ch>
* \date 26712.2015
* \brief Header file for the ExecuteDirectStatement class.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlHandle.h"
#include "ColumnBuffer.h"
#include "Database.h"

// Other headers
// System headers
#include <string>

// Forward declarations
// --------------------

namespace exodbc
{
	// Consts
	// ------

	// Structs
	// -------

	// Classes
	// -------
	/*!
	* \class ExecuteDirectStatement
	*
	* \brief A class to execute a single SQL statement without parameters.
	* \details Wraps around a statement handle to call SQLExecDirect using a
	*			passed SQL statement. 
	*			ColumnBuffer classes can be bound to retrieve the results of
	*			that SQL statement.
	*/
	class EXODBCAPI ExecuteDirectStatement
	{
	public:

		/*!
		* A static lists of drivers that do not support to set Cursor Options.
		* Unknown databases are expected to support it, so true is returned.
		* Access and Excel are known for not supporting.
		*/
		static bool DatabaseSupportsCursorOptions(DatabaseProduct dbms) noexcept
		{
			return !(dbms == DatabaseProduct::ACCESS || dbms == DatabaseProduct::EXCEL);
		}

		/*!
		* \brief Construct an empty ExecuteDirectStatement. You must call Init() before
		* you can start using the ExecuteDirectStatement.
		*/
		ExecuteDirectStatement();

		/*!
		* \brief Constructs a statement from the given Database.
		*/
		ExecuteDirectStatement(ConstDatabasePtr pDb);		
		
		ExecuteDirectStatement(const ExecuteDirectStatement& other) = delete;

		virtual ~ExecuteDirectStatement();




		/*!
		* Initialize the ExecuteDirectStatement. Must be called only once, and only
		* if the default Constructor has been used.
		*/
		void Init(ConstDatabasePtr pDb);


		/*!
		* \brief	Bind a ColumnBufferPtrVariant to a column of a result set (for
		*			example a SELECT query).
		*/
		void BindColumn(ColumnBufferPtrVariant column, SQLUSMALLINT columnNr);



		/*!
		* \brief	Execute the given statement using SQLExecDirect.
		* \details	Before the statement is executed an eventually open Cursor is closed.
		*/
		void ExecuteDirect(const std::wstring& sqlstmt);


		/*!
		* \brief	Returns the statement used.
		*/
		ConstSqlStmtHandlePtr GetStmt() const noexcept { return m_pHStmt; };


		void UnbindColumns();


		void SelectClose();


		bool SelectNext();


		bool SelectPrev();


		bool SelectFirst();


		bool SelectLast();


		bool SelectAbsolute(SQLLEN position);


		bool SelectRelative(SQLLEN offset);


	protected:
		bool SelectFetchScroll(SQLSMALLINT fetchOrientation, SQLLEN fetchOffset);

	private:
		SqlStmtHandlePtr m_pHStmt;	///< The statement we operate on
		ConstDatabasePtr m_pDb;
	};
} // namespace exodbc

