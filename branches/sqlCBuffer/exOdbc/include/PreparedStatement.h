/*!
* \file PreparedStatement.h
* \author Elias Gerber <eg@elisium.ch>
* \date 20.12.2015
* \brief Header file for the PreparedStatement classe.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlHandle.h"
#include "ColumnBuffer.h"
#include "Database.h"
#include "Table.h"

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
	* \class PreparedStatement
	*
	* \brief Takes an SQL Statement that can contain parameter markers ('?').
	*		 ColumnBuffer classes matching those markers can be bound to
	*		 execute the statement once or multiple times.
	*/
	class EXODBCAPI PreparedStatement
	{
	public:

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
		* A static lists of drivers that do not support to set Cursor Options.
		* Unknown databases are expected to support it, so true is returned.
		* Access and Excel are known for not supporting.
		*/
		static bool DatabaseSupportsCursorOptions(DatabaseProduct dbms) noexcept
		{
			return !(dbms == DatabaseProduct::ACCESS || dbms == DatabaseProduct::EXCEL);
		}

		/*!
		* \brief Construct an empty PreparedStatement. You must call Init() before
		* you can start using the PreparedStatement.
		*/
		PreparedStatement();
		
		PreparedStatement(const PreparedStatement& other) = delete;

		~PreparedStatement();

		/*!
		* \brief Constructs a statement from the given Database.
		* \details Call Prepare() afterwards to set the sql statement.
		* \see DatabseSupportsDescribeParam()
		*/
		PreparedStatement(ConstDatabasePtr pDb);


		/*!
		* \brief Constructs a statement from the given Database, using
		* the given sqlstmt. The statement will be prepared during construction.
		* \see DatabseSupportsDescribeParam()
		*/
		PreparedStatement(ConstDatabasePtr pDb, const std::wstring& sqlstmt);


		/*!
		* Initialize the PreparedStatement. Must be called only once, and only
		* if the default Constructor has been used.
		*/
		void Init(ConstDatabasePtr pDb);


		/*!
		* \brief	Bind a ColumnBufferPtrVariant to a parameter marker ('?').
		*/
		void BindParameter(ColumnBufferPtrVariant column, SQLUSMALLINT columnNr);


		/*!
		* \brief	Bind a ColumnBufferPtrVariant to a column of a result set (for
		*			example a SELECT query).
		*/
		void BindColumn(ColumnBufferPtrVariant column, SQLUSMALLINT columnNr);



		/*!
		* Execute PreparedStatement using the bound Parameters / Columns
		*/
		void Execute();


		/*!
		* \brief	Returns m_useSqlDescribeParam
		*/
		bool GetUseSqlDescribeParam() const noexcept { return m_useSqlDescribeParam; };


		/*!
		* \brief	Returns the statement used (and managed) by this PreparedStatement.
		*/
		ConstSqlStmtHandlePtr GetStmt() const noexcept { return m_pHStmt; };


		/*!
		* \brief	Calls SQLPrepare using m_sqlstmt and m_pHStmt.
		*/
		void Prepare(const std::wstring& sqlstmt);


		bool IsPrepared() const noexcept { return m_isPrepared; };


		void UnbindColumns();


		void UnbindParams();


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
		std::wstring m_sqlstmt;	///< The SQL for our statement.
		bool m_isPrepared;
		bool m_useSqlDescribeParam;
		ConstDatabasePtr m_pDb;
	};


	class EXODBCAPI PreparedInsertStatement
		: public PreparedStatement
	{
	public:
		PreparedInsertStatement() = delete;
		PreparedInsertStatement(const PreparedInsertStatement& other) = delete;
		PreparedInsertStatement(const Table& table);
	};

} // namespace exodbc

