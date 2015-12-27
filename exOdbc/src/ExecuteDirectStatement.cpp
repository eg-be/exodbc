/*!
* \file ExecuteDirectStatement.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 27.12.2015
* \brief Source file for ExecuteDirectStatement class.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
#include "ExecuteDirectStatement.h"

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
	ExecuteDirectStatement::ExecuteDirectStatement()
		: m_pDb(NULL)
	{ }


	ExecuteDirectStatement::ExecuteDirectStatement(ConstDatabasePtr pDb)
		: m_pDb(NULL)
	{
		Init(pDb);
	}


	ExecuteDirectStatement::~ExecuteDirectStatement()
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


	void ExecuteDirectStatement::Init(ConstDatabasePtr pDb)
	{
		exASSERT(m_pDb == NULL);
		exASSERT(pDb);
		exASSERT(pDb->IsOpen());

		m_pDb = pDb;
		m_pHStmt = std::make_shared<SqlStmtHandle>(m_pDb->GetSqlDbcHandle());
	}


	void ExecuteDirectStatement::ExecuteDirect(const std::wstring& sqlstmt)
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(!sqlstmt.empty());
		// Always discard pending results first
		SelectClose();

		SQLRETURN ret = SQLExecDirect(m_pHStmt->GetHandle(), (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCESS(SQLExecDirect, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
	}


	void ExecuteDirectStatement::BindColumn(ColumnBufferPtrVariant column, SQLUSMALLINT columnNr)
	{
		BindSelectVisitor sv(columnNr, m_pHStmt);
		boost::apply_visitor(sv, column);
	}


	void ExecuteDirectStatement::UnbindColumns()
	{
		m_pHStmt->UnbindColumns();
	}


	void ExecuteDirectStatement::SelectClose()
	{
		StatementCloser::CloseStmtHandle(m_pHStmt, StatementCloser::Mode::IgnoreNotOpen);
	}


	bool ExecuteDirectStatement::SelectPrev()
	{
		return SelectFetchScroll(SQL_FETCH_PREV, NULL);
	}


	bool ExecuteDirectStatement::SelectFirst()
	{
		return SelectFetchScroll(SQL_FETCH_FIRST, NULL);
	}


	bool ExecuteDirectStatement::SelectLast()
	{
		return SelectFetchScroll(SQL_FETCH_LAST, NULL);
	}


	bool ExecuteDirectStatement::SelectAbsolute(SQLLEN position)
	{
		return SelectFetchScroll(SQL_FETCH_ABSOLUTE, position);
	}


	bool ExecuteDirectStatement::SelectRelative(SQLLEN offset)
	{
		return SelectFetchScroll(SQL_FETCH_RELATIVE, offset);
	}


	bool ExecuteDirectStatement::SelectNext()
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


	bool ExecuteDirectStatement::SelectFetchScroll(SQLSMALLINT fetchOrientation, SQLLEN fetchOffset)
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
