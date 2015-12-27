/*!
* \file ExecutableStatement.h
* \author Elias Gerber <eg@elisium.ch>
* \date 26712.2015
* \brief Header file for the ExecutableStatement class.
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
	* \class ExecutableStatement
	*
	* \brief A class to execute a SQL statement against the Database.
	* \details	Provides a statement handle to execute arbitrary SQL.
	*			ColumnBuffer classes can be bound to retrieve the results of
	*			that SQL statement, and / or as parameters for the statement.
	*/
	class EXODBCAPI ExecutableStatement
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
		* A static lists of drivers that do not support SqlDescribeParam.
		* Unknown databases are expected to support it, so true is returned.
		* Access and Excel are known for not supporting.
		*/
		static bool DatabaseSupportsDescribeParam(DatabaseProduct dbms) noexcept
		{
			return !(dbms == DatabaseProduct::ACCESS || dbms == DatabaseProduct::EXCEL);
		};


		/*!
		* \brief Construct an empty ExecutableStatement. You must call Init() before
		* you can start using the ExecutableStatement.
		*/
		ExecutableStatement();

		/*!
		* \brief Constructs a statement from the given Database.
		*/
		ExecutableStatement(ConstDatabasePtr pDb, bool forwardOnlyCursors = false);		
		
		
		/*!
		* \brief Prevent copies.
		*/
		ExecutableStatement(const ExecutableStatement& other) = delete;


		/*!
		* \brief Explicitly frees the handle on destruction.
		*/
		virtual ~ExecutableStatement();


		/*!
		* \brief	Initialize the ExecutableStatement. Must be called only once, and only
		*			if the default Constructor has been used.
		*/
		void Init(ConstDatabasePtr pDb, bool forwardOnlyCursors);


		/*!
		* \brief	Bind a ColumnBufferPtrVariant to a column of a result set (for
		*			example a SELECT query).
		* \details	The columnNr is 1-indexed and must match the columns of the result-set
		*			produced by the SQL statement that is going to be executed.
		*/
		void BindColumn(ColumnBufferPtrVariant column, SQLUSMALLINT columnNr);


		/*!
		* \brief	Bind a ColumnBufferPtrVariant to a parameter marker ('?').
		* \details	The columnNr is 1-indexed and must match the parameter markers of 
		*			the result-set SQL statement that is going to be executed.
		*/
		void BindParameter(ColumnBufferPtrVariant column, SQLUSMALLINT columnNr);


		/*!
		* \brief	Execute the passed statement using SQLExecDirect.
		* \details	Before the statement is executed an eventually open Cursor is closed.
		*/
		void ExecuteDirect(const std::wstring& sqlstmt);


		/*!
		* \brief	Prepares the statement for multiple execution by calling SQLPrepare
		*			with the passed sqlstmt.
		*/
		void Prepare(const std::wstring& sqlstmt);


		/*!
		* \brief	Returns true if a statement has been prepared for execution using Prepare().
		*/
		bool IsPrepared() const noexcept { return m_isPrepared; };


		/*!
		* \brief	Executes a statement that has been prepared using Prepare() using SQLExecute.
		*/
		void ExecutePrepared();


		/*!
		* \brief	Unbinds all columns bound to the handle held by this ExecutableStatement.
		*/
		void UnbindColumns();


		/*!
		* \brief	Closes an eventually open cursor (result set). Save to call even if no
		*			result set is open.
		*/
		void SelectClose();


		/*!
		* \brief	Closes an eventually open cursor (result set). Save to call even if no
		*			result set is open.
		*/
		bool SelectNext();


		bool SelectPrev();


		bool SelectFirst();


		bool SelectLast();


		bool SelectAbsolute(SQLLEN position);


		bool SelectRelative(SQLLEN offset);


	protected:
		void SetCursorOptions(bool forwardOnlyCursors);

		bool SelectFetchScroll(SQLSMALLINT fetchOrientation, SQLLEN fetchOffset);

		SqlStmtHandlePtr m_pHStmt;	///< The statement we operate on
		ConstDatabasePtr m_pDb;
		bool m_isPrepared;
	};
} // namespace exodbc

