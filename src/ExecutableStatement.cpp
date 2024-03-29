﻿/*!
* \file ExecutableStatement.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 27.12.2015
* \brief Source file for ExecutableStatement class.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "ExecutableStatement.h"

// Same component headers
#include "ColumnBufferVisitors.h"
#include "SqlStatementCloser.h"
#include "LogManagerOdbcMacros.h"

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
		, m_scrollableCursor(false)
		, m_boundColumns(false)
		, m_boundParams(false)
	{ }


	ExecutableStatement::ExecutableStatement(ConstDatabasePtr pDb, bool scrollableCursor)
		: m_pDb(NULL)
		, m_isPrepared(false)
		, m_scrollableCursor(false)
		, m_boundColumns(false)
		, m_boundParams(false)
	{
		Init(pDb, scrollableCursor);
	}


	ExecutableStatement::ExecutableStatement(ConstDatabasePtr pDb)
		: m_pDb(NULL)
		, m_isPrepared(false)
		, m_scrollableCursor(false)
		, m_boundColumns(false)
		, m_boundParams(false)
	{
		Init(pDb, m_scrollableCursor);
	}


	ExecutableStatement::~ExecutableStatement()
	{
		// We do not free the handle explicitly. Let it go out of scope, it will destroy itself once no one needs it.
		// but unbind things as long as the ExecutableStatement cannot be copied.
		if (m_boundParams)
		{
			try
			{
				m_pHStmt->ResetParams();
			}
			catch (const Exception& ex)
			{
				LOG_ERROR(ex.ToString());
			}
		}
		if (m_boundColumns)
		{
			try
			{
				m_pHStmt->UnbindColumns();
			}
			catch (const Exception& ex)
			{
				LOG_ERROR(ex.ToString());
			}
		}
	}


	void ExecutableStatement::Init(ConstDatabasePtr pDb, bool scrollableCursor)
	{
		exASSERT(m_pDb == NULL);
		exASSERT(pDb);
		exASSERT(pDb->IsOpen());

		m_pDb = pDb;
		m_pHStmt = std::make_shared<SqlStmtHandle>(m_pDb->GetSqlDbcHandle());

		// If we fail during init, go back into state before init was called
		try
		{
			SetCursorOptions(scrollableCursor);
		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			Reset();
			throw;
		}
	}


	void ExecutableStatement::Reset()
	{
		if (m_boundColumns)
		{
			m_pHStmt->UnbindColumns();
			m_boundColumns = false;
		}
		if(m_boundParams)
		{
			m_pHStmt->ResetParams();
			m_boundParams = false;
		}
		m_pHStmt.reset();
		m_pDb.reset();
	}


	void ExecutableStatement::SetCursorOptions(bool scrollableCursor)
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(m_pDb);
		exASSERT(m_pDb->IsOpen());

		try
		{
			SQLRETURN ret = 0;
			// Try to read the currently active value first, and only try to change if a change is required:
			SQLULEN currentValue;
			ret = SQLGetStmtAttr(m_pHStmt->GetHandle(), SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)&currentValue, sizeof(currentValue), 0);
			THROW_IFN_SUCCEEDED_MSG(SQLGetStmtAttr, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle(), u8"Failed to get Statement Attr SQL_ATTR_CURSOR_SCROLLABLE");
			if (!scrollableCursor && currentValue != SQL_NONSCROLLABLE)
			{
				ret = SQLSetStmtAttr(m_pHStmt->GetHandle(), SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_NONSCROLLABLE, 0);
				THROW_IFN_SUCCEEDED_MSG(SQLSetStmtAttr, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle(), u8"Failed to set Statement Attr SQL_ATTR_CURSOR_SCROLLABLE to SQL_NONSCROLLABLE");
				m_scrollableCursor = false;
			}
			else if(scrollableCursor && currentValue != SQL_SCROLLABLE)
			{
				ret = SQLSetStmtAttr(m_pHStmt->GetHandle(), SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_SCROLLABLE, 0);
				THROW_IFN_SUCCEEDED_MSG(SQLSetStmtAttr, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle(), u8"Failed to set Statement Attr SQL_ATTR_CURSOR_SCROLLABLE to SQL_SCROLLABLE");
				m_scrollableCursor = true;
			}
		}
		catch (const SqlResultException& sre)
		{
			// If we failed because the driver does not support scrollable cursors and
			// we only need forward-only cursors, simply log a warning and assume forward-only
			if (sre.HasErrorInfo(ErrorHelper::SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED) && !scrollableCursor)
			{
				LOG_WARNING(boost::str(boost::format(u8"Failed to read or set Cursor Option SQL_ATTR_CURSOR_SCROLLABLE because driver does not support that atribute, assuming forwardOnlyCursors: %s") % sre.ToString()));
				m_scrollableCursor = false;
			}
			else
			{
				throw;
			}
		}
	}


	void ExecutableStatement::ExecuteDirect(const std::string& sqlstmt)
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(!sqlstmt.empty());
		// Always discard pending results first
		SelectClose();

		SQLRETURN ret = SQLExecDirect(m_pHStmt->GetHandle(),  
			(SQLAPICHARTYPE*) EXODBCSTR_TO_SQLAPISTR(sqlstmt).c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLExecDirect, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
	}


	void ExecutableStatement::Prepare(const std::string& sqlstmt)
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(!sqlstmt.empty());

		SQLRETURN ret = SQLPrepare(m_pHStmt->GetHandle(),  
			(SQLAPICHARTYPE*) EXODBCSTR_TO_SQLAPISTR(sqlstmt).c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		m_isPrepared = true;
	}


	ParameterDescription ExecutableStatement::DescribeParameter(SQLUSMALLINT paramNr) const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(paramNr >= 1);

		ParameterDescription paramDesc(m_pHStmt, paramNr);
		return paramDesc;
	}


	ColumnDescription ExecutableStatement::DescribeColumn(SQLUSMALLINT columnNr) const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(columnNr >= 1);
		exASSERT(m_pDb);

		ColumnDescription colDesc(m_pHStmt, columnNr, m_pDb->GetProperties());

		return colDesc;
	}


	SQLSMALLINT ExecutableStatement::GetNrOfColumns() const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		SQLSMALLINT columnCount = 0;
		SQLRETURN ret = SQLNumResultCols(m_pHStmt->GetHandle(), &columnCount);
		THROW_IFN_SUCCEEDED(SQLNumResultCols, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		return columnCount;
	}


	void ExecutableStatement::ExecutePrepared() const
	{
		exASSERT(m_isPrepared);

		// Always discard pending results first
		SelectClose();

		SQLRETURN ret = SQLExecute(m_pHStmt->GetHandle());
		THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
	}


	void ExecutableStatement::BindColumn(ColumnBufferPtrVariant column, SQLUSMALLINT columnNr)
	{
		BindColumnVisitor sv(columnNr, m_pHStmt);
		boost::apply_visitor(sv, column);
		m_boundColumns = true;
	}


	void ExecutableStatement::BindParameter(ColumnBufferPtrVariant column, SQLUSMALLINT paramNr, bool neverQueryParamDesc /* = false */)
	{
		exASSERT(m_pDb);

		ParameterDescription paramDesc;
		if (!neverQueryParamDesc && IsPrepared() && DatabaseSupportsDescribeParam(m_pDb->GetDbms(), boost::apply_visitor(SqlCTypeVisitor(), column)))
		{
			paramDesc = DescribeParameter(paramNr);
		}
		else
		{
			paramDesc = boost::apply_visitor(ParamDescVisitor(), column);
		}

		BindParamVisitor pv(paramNr, m_pHStmt, paramDesc);
		boost::apply_visitor(pv, column);
		m_boundParams = true;
	}


	void ExecutableStatement::UnbindColumns()
	{
		exASSERT(m_pHStmt);

		m_pHStmt->UnbindColumns();
	}


	void ExecutableStatement::UnbindParams()
	{
		exASSERT(m_pHStmt);

		m_pHStmt->ResetParams();
	}


	void ExecutableStatement::SelectClose() const
	{
		StatementCloser::CloseStmtHandle(m_pHStmt, StatementCloser::Mode::IgnoreNotOpen);
	}


	bool ExecutableStatement::SelectPrev()
	{
		return SelectFetchScroll(SQL_FETCH_PREV, 0);
	}


	bool ExecutableStatement::SelectFirst()
	{
		return SelectFetchScroll(SQL_FETCH_FIRST, 0);
	}


	bool ExecutableStatement::SelectLast()
	{
		return SelectFetchScroll(SQL_FETCH_LAST, 0);
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
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		SQLRETURN ret = SQLFetch(m_pHStmt->GetHandle());
		if (!(SQL_SUCCEEDED(ret) || ret == SQL_NO_DATA))
		{
			SqlResultException sre(u8"SQLFetch", ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
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
		exASSERT(m_scrollableCursor);
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		SQLRETURN ret = SQLFetchScroll(m_pHStmt->GetHandle(), fetchOrientation, fetchOffset);
		if (!(SQL_SUCCEEDED(ret) || ret == SQL_NO_DATA))
		{
			std::string msg = boost::str(boost::format(u8"Failed in SQLFetchScroll with FetchOrientation %d") % fetchOrientation);
			SqlResultException sre(u8"SQLFetchScroll", ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle(), msg);
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
