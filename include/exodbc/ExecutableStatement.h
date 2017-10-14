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
#include "ParameterDescription.h"
#include "ColumnDescription.h"

// Other headers
// System headers
#include <string>
#include <memory>

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
	*			On destruction, the columns and or params will be resetted
	*			on the underlying handle and the handle will be freed.
	*/
	class EXODBCAPI ExecutableStatement
	{
	public:
		/*!
		* A static lists of drivers that do not support SqlDescribeParam.
		* Unknown databases are expected to support it, so true is returned.
		* Access and Excel are known for not supporting.
		* MySql returns strange results for numeric types, see #206
		*/
		static bool DatabaseSupportsDescribeParam(DatabaseProduct dbms, SQLSMALLINT sqlCType) noexcept
		{
			if (dbms == DatabaseProduct::ACCESS || dbms == DatabaseProduct::EXCEL)
				return false;
			if (dbms == DatabaseProduct::MY_SQL && sqlCType == SQL_C_NUMERIC)
				return false;
			return true;
		};


		/*!
		* \brief Construct an empty ExecutableStatement. You must call Init() before
		* you can start using the ExecutableStatement.
		*/
		ExecutableStatement();


		/*!
		* \brief Constructs a statement from the given Database, using passed value 
		* to set scrollable cursors
		*/
		ExecutableStatement(ConstDatabasePtr pDb, bool scrollableCursor);
		

		/*!
		* \brief Constructs a statement from the given Database. Scrollable cursor is disabled.
		*/
		ExecutableStatement(ConstDatabasePtr pDb);

		
		// Prevent copies.
		ExecutableStatement(const ExecutableStatement& other) = delete;
		ExecutableStatement& operator=(const ExecutableStatement& other) = delete;

		/*!
		* \brief	If parameters or columns have been bound on this statement, call UnbindColumns() or
		*			ResetParams() on the internal statement on destruction.
		*/
		virtual ~ExecutableStatement();


		/*!
		* \brief	Initialize the ExecutableStatement. Must be called only once, and only
		*			if the default Constructor has been used, or after Reset() has been called.
		*/
		void Init(ConstDatabasePtr pDb, bool scrollableCursor);


		/*!
		* \brief	Returns true if Init() was called and m_pDb is not null.
		*/
		bool IsInitialized() const noexcept { return m_pDb != NULL; };


		/*!
		* \brief	Resets this ExecutableStatement: It will be in the same state as if it has
		*			been constructed using the default constructor. Call Init() after you've called
		*			Reset() and you want to re-use the ExectuableStatement again.
		* \details	If any params or columns have been bound using this ExcecutableStatement, 
		*			the corresponding Unbind function is called on the Stmt-handle.
		*/
		void Reset();


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
		*			If the statement has been prepared, and the Database supports querying
		*			about parameter descriptions, the values for column size and decimal digits
		*			are queried from the database.
		*			
		*			If the flag neverQueryParamDesc is true, params are never queried.
		*			
		*			If param description is not queried, one is created from the values
		*			stored in the ColumnBuffer.
		*/
		void BindParameter(ColumnBufferPtrVariant column, SQLUSMALLINT paramNr, bool neverQueryParamDesc = false);


		/*!
		* \brief	Execute the passed statement using SQLExecDirect.
		* \details	Before the statement is executed an eventually open Cursor is closed.
		*/
		void ExecuteDirect(const std::string& sqlstmt);


		/*!
		* \brief	Prepares the statement for multiple execution by calling SQLPrepare
		*			with the passed sqlstmt.
		*/
		void Prepare(const std::string& sqlstmt);


		/*!
		* \brief	Returns true if a statement has been prepared for execution using Prepare().
		*/
		bool IsPrepared() const noexcept { return m_isPrepared; };


		/*!
		* \brief	Returns true if scrollable cursors are enabled
		*/
		bool IsForwardOnlyCursor() const noexcept { return m_scrollableCursor; };


		/*!
		* \brief	Calls SQLDescribeParam for parameter at paramNr and the handle of this ExecutableStatement.
		*/
		ParameterDescription DescribeParameter(SQLUSMALLINT paramNr) const;


		/*!
		* \brief	Calls SQLDescribeCol for column at columnNr and the handle of this ExecutableStatement.
		*/
		ColumnDescription DescribeColumn(SQLUSMALLINT columnNr) const;


		/*!
		* Calls SQLNumResultsCols to get the number of columns in the current result set of this ExecutableStatement.
		*/
		SQLSMALLINT GetNrOfColumns() const;


		/*!
		* \brief	Executes a statement that has been prepared using Prepare() using SQLExecute.
		*/
		void ExecutePrepared() const;


		/*!
		* \brief	Unbinds all columns bound to the handle held by this ExecutableStatement.
		*/
		void UnbindColumns();


		/*!
		* \brief	Unbinds all parameters bound to the handle held by this ExecutableStatement.
		*/
		void UnbindParams();


		/*!
		* \brief	Closes an eventually open cursor (result set). Save to call even if no
		*			result set is open.
		*/
		void SelectClose() const;


		/*!
		* \brief	Selects the next record in a result set.
		*/
		bool SelectNext();


		/*!
		* \brief	Selects the previous record in a result set.
		*/
		bool SelectPrev();


		/*!
		* \brief	Selects the first record in a result set.
		*/
		bool SelectFirst();


		/*!
		* \brief	Selects the last record in a result set.
		*/
		bool SelectLast();


		/*!
		* \brief	Fetches the record at absolute position fromt the current active recordset.
		*/
		bool SelectAbsolute(SQLLEN position);


		/*!
		* \brief	Fetches the record at relative position to the currently selected record.
		* \return	True if record at relative position has been fetched, false if no record available.
		* \note		MySql seems to get confused if no record is selected before doing the first SelectRelative():
		*			If just a statement is executed, I would expect the cursor to be positioned before the
		*			first row. So a SelectRelative(3) should select the 3rd record. But on MySql a SelectRelative(3)
		*			selects the 4th record. If you do a SelectNext() first and then a SelectRelative(2), the
		*			3rd record is selected correctly.
		*/
		bool SelectRelative(SQLLEN offset);


	protected:
		void SetCursorOptions(bool scrollableCursor);

		bool SelectFetchScroll(SQLSMALLINT fetchOrientation, SQLLEN fetchOffset);

		SqlStmtHandlePtr m_pHStmt;	///< The statement we operate on
		ConstDatabasePtr m_pDb;
		bool m_isPrepared;
		bool m_scrollableCursor;

		bool m_boundColumns;
		bool m_boundParams;
	};

	typedef std::shared_ptr<ExecutableStatement> ExecutableStatementPtr;
} // namespace exodbc

