/*!
* \file PreparedStatement.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 20.12.2015
* \brief Source file for SqlCBuffer.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
#include "PreparedStatement.h"

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
	PreparedStatement::PreparedStatement()
		: m_isPrepared(false)
		, m_useSqlDescribeParam(false)
		, m_pDb(NULL)
	{ }


	PreparedStatement::PreparedStatement(ConstDatabasePtr pDb)
		: m_isPrepared(false)
		, m_useSqlDescribeParam(false)
		, m_pDb(NULL)
	{
		Init(pDb);
	}


	PreparedStatement::PreparedStatement(ConstDatabasePtr pDb, const std::wstring& sqlstmt)
		: m_isPrepared(false)
		, m_useSqlDescribeParam(false)
		, m_pDb(NULL)
	{
		Init(pDb);

		Prepare(sqlstmt);
	}


	PreparedStatement::~PreparedStatement()
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


	void PreparedStatement::Init(ConstDatabasePtr pDb)
	{
		exASSERT(m_pDb == NULL);
		exASSERT(pDb);
		exASSERT(pDb->IsOpen());

		m_pDb = pDb;
		m_useSqlDescribeParam = DatabaseSupportsDescribeParam(m_pDb->GetDbms());
		m_pHStmt = std::make_shared<SqlStmtHandle>(m_pDb->GetSqlDbcHandle());
	}


	void PreparedStatement::Execute()
	{
		exASSERT(m_isPrepared);

		// Always discard pending results first
		SelectClose();

		SQLRETURN ret = SQLExecute(m_pHStmt->GetHandle());
		THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
	}


	void PreparedStatement::BindParameter(ColumnBufferPtrVariant column, SQLUSMALLINT columnNr)
	{
		BindParamVisitor pv(columnNr, m_pHStmt, m_useSqlDescribeParam);
		boost::apply_visitor(pv, column);
	}


	void PreparedStatement::BindColumn(ColumnBufferPtrVariant column, SQLUSMALLINT columnNr)
	{
		BindSelectVisitor sv(columnNr, m_pHStmt);
		boost::apply_visitor(sv, column);
	}


	void PreparedStatement::Prepare(const std::wstring& sqlstmt)
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		m_sqlstmt = sqlstmt;

		SQLRETURN ret = SQLPrepare(m_pHStmt->GetHandle(), (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		m_isPrepared = true;
	}


	void PreparedStatement::UnbindColumns()
	{
		m_pHStmt->UnbindColumns();
	}

	
	void PreparedStatement::UnbindParams()
	{
		m_pHStmt->ResetParams();
	}


	void PreparedStatement::SelectClose()
	{
		StatementCloser::CloseStmtHandle(m_pHStmt, StatementCloser::Mode::IgnoreNotOpen);
	}


	bool PreparedStatement::SelectPrev()
	{
		return SelectFetchScroll(SQL_FETCH_PREV, NULL);
	}


	bool PreparedStatement::SelectFirst()
	{
		return SelectFetchScroll(SQL_FETCH_FIRST, NULL);
	}


	bool PreparedStatement::SelectLast()
	{
		return SelectFetchScroll(SQL_FETCH_LAST, NULL);
	}


	bool PreparedStatement::SelectAbsolute(SQLLEN position)
	{
		return SelectFetchScroll(SQL_FETCH_ABSOLUTE, position);
	}


	bool PreparedStatement::SelectRelative(SQLLEN offset)
	{
		return SelectFetchScroll(SQL_FETCH_RELATIVE, offset);
	}


	bool PreparedStatement::SelectNext()
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


	bool PreparedStatement::SelectFetchScroll(SQLSMALLINT fetchOrientation, SQLLEN fetchOffset)
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
