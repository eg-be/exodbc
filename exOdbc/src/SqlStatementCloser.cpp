/*!
* \file SqlStatementCloser.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2015
* \brief Source file for the SqlStatementCloser
* \copyright GNU Lesser General Public License Version 3
*/

#include "stdafx.h"

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
	void StatementCloser::CloseStmtHandle(SQLHANDLE hStmt, Mode mode)
	{
		exASSERT(hStmt != SQL_NULL_HSTMT);

		SQLRETURN ret;
		if (mode == Mode::IgnoreNotOpen)
		{
			//  calling SQLFreeStmt with the SQL_CLOSE option has no effect on the application if no cursor is open on the statement
			ret = SQLFreeStmt(hStmt, SQL_CLOSE);
			THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, hStmt);
		}
		else
		{
			// SQLCloseCursor returns SQLSTATE 24000 (Invalid cursor state) if no cursor is open. 
			ret = SQLCloseCursor(hStmt);
			THROW_IFN_SUCCEEDED(SQLCloseCursor, ret, SQL_HANDLE_STMT, hStmt);
		}
	}


	void StatementCloser::CloseStmtHandle(ConstSqlStmtHandlePtr pHStmt, Mode mode)
	{
		exASSERT(pHStmt);
		exASSERT(pHStmt->IsAllocated());
		CloseStmtHandle(pHStmt->GetHandle(), mode);
	}


	StatementCloser::StatementCloser(SQLHSTMT hStmt, bool closeOnConstruction /* = false */, bool closeOnDestruction /* = true */)
		: m_hStmt(hStmt)
		, m_closeOnDestruction(closeOnDestruction)
		, m_pHStmt(NULL)
	{
		if (closeOnConstruction)
		{
			CloseStmtHandle(m_hStmt, Mode::IgnoreNotOpen);
		}
	}


	StatementCloser::StatementCloser(ConstSqlStmtHandlePtr pHStmt, bool closeOnConstruction /* = false */, bool closeOnDestruction /* = true */)
		: m_pHStmt(pHStmt)
		, m_closeOnDestruction(closeOnDestruction)
		, m_hStmt(SQL_NULL_HSTMT)
	{
		if (closeOnConstruction)
		{
			exASSERT(m_pHStmt);
			exASSERT(m_pHStmt->IsAllocated());
			CloseStmtHandle(pHStmt->GetHandle(), Mode::IgnoreNotOpen);
		}
	}


	StatementCloser::~StatementCloser()
	{
		try
		{
			if (m_closeOnDestruction)
			{
				if (m_hStmt)
				{
					CloseStmtHandle(m_hStmt, Mode::IgnoreNotOpen);
				}
				else if (m_pHStmt)
				{
					exASSERT(m_pHStmt->IsAllocated());
					CloseStmtHandle(m_pHStmt->GetHandle(), Mode::IgnoreNotOpen);
				}

			}
		}
		catch (Exception& ex)
		{
			// Should never happen?
			// \todo Ticket #100
			LOG_ERROR(ex.ToString());
		}
	}
}