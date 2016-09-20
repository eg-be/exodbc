/*!
* \file SqlStatementCloser.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2015
* \brief Source file for the SqlStatementCloser
* \copyright GNU Lesser General Public License Version 3
*/

// Own header
#include "SqlStatementCloser.h"

// Same component headers
// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------
namespace exodbc
{
	void StatementCloser::CloseStmtHandle(ConstSqlStmtHandlePtr pHStmt, Mode mode)
	{
		exASSERT(pHStmt);
		exASSERT(pHStmt->IsAllocated());

		SQLRETURN ret;
		if (mode == Mode::IgnoreNotOpen)
		{
			//  calling SQLFreeStmt with the SQL_CLOSE option has no effect on the application if no cursor is open on the statement
			ret = SQLFreeStmt(pHStmt->GetHandle(), SQL_CLOSE);
			THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
		}
		else
		{
			// SQLCloseCursor returns SQLSTATE 24000 (Invalid cursor state) if no cursor is open. 
			ret = SQLCloseCursor(pHStmt->GetHandle());
			THROW_IFN_SUCCEEDED(SQLCloseCursor, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
		}
	}


	StatementCloser::StatementCloser(ConstSqlStmtHandlePtr pHStmt, bool closeOnConstruction /* = false */, bool closeOnDestruction /* = true */)
		: m_pHStmt(pHStmt)
		, m_closeOnDestruction(closeOnDestruction)
	{
		if (closeOnConstruction)
		{
			exASSERT(m_pHStmt);
			exASSERT(m_pHStmt->IsAllocated());
			CloseStmtHandle(pHStmt, Mode::IgnoreNotOpen);
		}
	}


	StatementCloser::~StatementCloser()
	{
		try
		{
			if (m_closeOnDestruction)
			{
				if (m_pHStmt)
				{
					exASSERT(m_pHStmt->IsAllocated());
					CloseStmtHandle(m_pHStmt, Mode::IgnoreNotOpen);
				}

			}
		}
		catch (Exception& ex)
		{
			// Should never happen?
			LOG_ERROR(ex.ToString());
		}
	}
}