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
#include "ColumnBuffer.h"

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
	*		 SqlCBuffer classes matching those markers can be bound to
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

		PreparedStatement() = delete;
		PreparedStatement(const PreparedStatement& other) = delete;

		~PreparedStatement();

		/*!
		* \brief Constructs a statement from the given Database.
		* \see DatabseSupportsDescribeParam()
		*/
		PreparedStatement(ConstDatabasePtr pDb, const std::wstring& sqlstmt);


		/*!
		* \brief	Constructs a statement handle using the passed pHDbc to
		*			prepare the statement for execution.
		* \see DatabseSupportsDescribeParam()
		*/
		PreparedStatement(ConstSqlDbcHandlePtr pHDbc, DatabaseProduct dbc, const std::wstring& sqlstmt);


		/*!
		* \brief	Constructs a statement handle using the passed pHDbc to
		*			prepare the statement for execution.
		*			If useSqlDescribeParam is true, the function SqlDescribeParams is
		*			used during parameter binding (not supported by all dbs).
		*/
		PreparedStatement(ConstSqlDbcHandlePtr pHDbc, bool useSqlDescribeParam, const std::wstring& sqlstmt);		


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

	private:

		/*!
		* \brief	Calls SQLPrepare using m_sqlstmt and m_pHStmt.
		*/
		void Prepare();

		SqlStmtHandlePtr m_pHStmt;	///< The statement we operate on
		const std::wstring m_sqlstmt;	///< The SQL for our statement.
		bool m_useSqlDescribeParam;
	};

} // namespace exodbc

