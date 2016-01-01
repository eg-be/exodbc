/*!
* \file StatementCloser.h
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2015
* \brief Source file for the SqlStatementCloser
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlHandle.h"

// Other headers

// System headers

// Static consts
// -------------
namespace exodbc
{
	class EXODBCAPI StatementCloser
	{
	public:
		/*!
		* \enum	StmtCloseMode
		* \brief Hints for CloseStmtHandle() if it shall throw or not.
		* \see	CloseStmtHandle()
		*/
		enum class Mode
		{
			ThrowIfNotOpen,	///< Throw if trying to free a statement handle that is not open.
			IgnoreNotOpen	///< Do not throw if trying to close an already closed statement handle.
		};

		/*!
		* \brief	Close the cursor associated with the passed statement handle.
		* \details	Depending on StmtCloseMode, this will call SQLFreeStatement or SQLCloseCursor.
		*			Depending on StmtCloseMode, the function will throw if the cursor was not open
		*			before this function was called.
		* \param	hStmt		The statement handle.
		* \param	mode		Determine whether the function should fail if the cursor is not open.
		* \see		StmtCloseMode
		* \throw	Exception	Depending on StmtCloseMode.
		*/
		static void	CloseStmtHandle(ConstSqlStmtHandlePtr pHstmt, Mode mode);

		static void	CloseStmtHandle(SQLHANDLE hStmt, Mode mode);

		StatementCloser(ConstSqlStmtHandlePtr pHStmt, bool closeOnConstruction = false, bool closeOnDestruction = true);
		StatementCloser(SQLHSTMT hStmt, bool closeOnConstruction = false, bool closeOnDestruction = true);
		~StatementCloser();

	private:
		ConstSqlStmtHandlePtr m_pHStmt;
		SQLHSTMT m_hStmt;
		bool m_closeOnDestruction;
	};
}

// Interfaces
// ----------

#pragma once
