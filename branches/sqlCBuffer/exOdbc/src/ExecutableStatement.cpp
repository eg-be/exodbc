/*!
* \file ExecutableStatement.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 27.12.2015
* \brief Source file for ExecutableStatement class.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
#include "ExecutableStatement.h"

// Same component headers
#include "SqlCBufferVisitors.h"
#include "SqlStatementCloser.h"

// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	ExecutableStatement::ExecutableStatement()
		: m_pDb(NULL)
		, m_isPrepared(false)
	{ }


	ExecutableStatement::ExecutableStatement(ConstDatabasePtr pDb, bool forwardOnlyCursor /* = false */)
		: m_pDb(NULL)
		, m_isPrepared(false)
	{
		Init(pDb, forwardOnlyCursor);
	}


	ExecutableStatement::~ExecutableStatement()
	{
		try
		{
			// we should not care? should we? Maybe better to wait and let it go out of scope?
			m_pHStmt->Free();
		}
		catch (const Exception& ex)
		{
			LOG_ERROR(ex.ToString());
		}
	}


	void ExecutableStatement::Init(ConstDatabasePtr pDb, bool forwardOnlyCursors)
	{
		exASSERT(m_pDb == NULL);
		exASSERT(pDb);
		exASSERT(pDb->IsOpen());

		m_pDb = pDb;
		m_pHStmt = std::make_shared<SqlStmtHandle>(m_pDb->GetSqlDbcHandle());

		if (DatabaseSupportsCursorOptions(m_pDb->GetDbms()))
		{
			SetCursorOptions(forwardOnlyCursors);
		}
	}


	void ExecutableStatement::SetCursorOptions(bool forwardOnlyCursors)
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(m_pDb);
		exASSERT(m_pDb->IsOpen());

		SQLRETURN ret = 0;
		if (forwardOnlyCursors || m_pDb->GetDbInfo().GetForwardOnlyCursors())
		{
			ret = SQLSetStmtAttr(m_pHStmt->GetHandle(), SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_NONSCROLLABLE, NULL);
			THROW_IFN_SUCCEEDED_MSG(SQLSetStmtAttr, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle(), L"Failed to set Statement Attr SQL_ATTR_CURSOR_SCROLLABLE to SQL_NONSCROLLABLE");
		}
		else
		{
			ret = SQLSetStmtAttr(m_pHStmt->GetHandle(), SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_SCROLLABLE, NULL);
			THROW_IFN_SUCCEEDED_MSG(SQLSetStmtAttr, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle(), L"Failed to set Statement Attr SQL_ATTR_CURSOR_SCROLLABLE to SQL_SCROLLABLE");
		}
	}


	void ExecutableStatement::ExecuteDirect(const std::wstring& sqlstmt)
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(!sqlstmt.empty());
		// Always discard pending results first
		SelectClose();

		SQLRETURN ret = SQLExecDirect(m_pHStmt->GetHandle(), (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCESS(SQLExecDirect, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
	}


	void ExecutableStatement::Prepare(const std::wstring& sqlstmt)
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(!sqlstmt.empty());

		SQLRETURN ret = SQLPrepare(m_pHStmt->GetHandle(), (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		m_isPrepared = true;
	}


	void ExecutableStatement::ExecutePrepared()
	{
		exASSERT(m_isPrepared);

		// Always discard pending results first
		SelectClose();

		SQLRETURN ret = SQLExecute(m_pHStmt->GetHandle());
		THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
	}


	void ExecutableStatement::BindColumn(ColumnBufferPtrVariant column, SQLUSMALLINT columnNr)
	{
		BindSelectVisitor sv(columnNr, m_pHStmt);
		boost::apply_visitor(sv, column);
	}


	void ExecutableStatement::BindParameter(ColumnBufferPtrVariant column, SQLUSMALLINT columnNr)
	{
		exASSERT(m_pDb);

		BindParamVisitor pv(columnNr, m_pHStmt, DatabaseSupportsCursorOptions(m_pDb->GetDbms()));
		boost::apply_visitor(pv, column);
	}


	void ExecutableStatement::UnbindColumns()
	{
		m_pHStmt->UnbindColumns();
	}


	void ExecutableStatement::SelectClose()
	{
		StatementCloser::CloseStmtHandle(m_pHStmt, StatementCloser::Mode::IgnoreNotOpen);
	}


	bool ExecutableStatement::SelectPrev()
	{
		return SelectFetchScroll(SQL_FETCH_PREV, NULL);
	}


	bool ExecutableStatement::SelectFirst()
	{
		return SelectFetchScroll(SQL_FETCH_FIRST, NULL);
	}


	bool ExecutableStatement::SelectLast()
	{
		return SelectFetchScroll(SQL_FETCH_LAST, NULL);
	}


	bool ExecutableStatement::SelectAbsolute(SQLLEN position)
	{
		return SelectFetchScroll(SQL_FETCH_ABSOLUTE, position);
	}


	bool ExecutableStatement::SelectRelative(SQLLEN offset)
	{
		return SelectFetchScroll(SQL_FETCH_RELATIVE, offset);
	}


	bool ExecutableStatement::SelectNext()
	{
		SQLRETURN ret = SQLFetch(m_pHStmt->GetHandle());
		if (!(SQL_SUCCEEDED(ret) || ret == SQL_NO_DATA))
		{
			SqlResultException sre(L"SQLFetch", ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
			SET_EXCEPTION_SOURCE(sre);
			throw sre;
		}
		if (ret == SQL_SUCCESS_WITH_INFO)
		{
			LOG_WARNING_STMT(m_pHStmt->GetHandle(), ret, SQLFetch);
		}

		return SQL_SUCCEEDED(ret);
	}


	bool ExecutableStatement::SelectFetchScroll(SQLSMALLINT fetchOrientation, SQLLEN fetchOffset)
	{
		SQLRETURN ret = SQLFetchScroll(m_pHStmt->GetHandle(), fetchOrientation, fetchOffset);
		if (!(SQL_SUCCEEDED(ret) || ret == SQL_NO_DATA))
		{
			wstring msg = boost::str(boost::wformat(L"Failed in SQLFetchScroll with FetchOrientation %d") % fetchOrientation);
			SqlResultException sre(L"SQLFetchScroll", ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle(), msg);
			SET_EXCEPTION_SOURCE(sre);
			throw sre;
		}
		if (ret == SQL_SUCCESS_WITH_INFO)
		{
			LOG_WARNING_STMT(m_pHStmt->GetHandle(), ret, SQLFetch);
		}

		return SQL_SUCCEEDED(ret);
	}
}
