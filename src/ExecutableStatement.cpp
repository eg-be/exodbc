/*!
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
		, m_forwardOnlyCursors(false)
		, m_boundColumns(false)
		, m_boundParams(false)
	{ }


	ExecutableStatement::ExecutableStatement(ConstDatabasePtr pDb, bool forwardOnlyCursor /* = false */)
		: m_pDb(NULL)
		, m_isPrepared(false)
		, m_forwardOnlyCursors(false)
		, m_boundColumns(false)
		, m_boundParams(false)
	{
		Init(pDb, forwardOnlyCursor);
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
		else
		{
			m_forwardOnlyCursors = true;
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


	void ExecutableStatement::SetCursorOptions(bool forwardOnlyCursors)
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(m_pDb);
		exASSERT(m_pDb->IsOpen());

		try
		{
			SQLRETURN ret = 0;
			if (forwardOnlyCursors || m_pDb->GetDbInfo().GetForwardOnlyCursors())
			{
				ret = SQLSetStmtAttr(m_pHStmt->GetHandle(), SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_NONSCROLLABLE, 0);
				THROW_IFN_SUCCEEDED_MSG(SQLSetStmtAttr, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle(), u8"Failed to set Statement Attr SQL_ATTR_CURSOR_SCROLLABLE to SQL_NONSCROLLABLE");
				m_forwardOnlyCursors = true;
			}
			else
			{
				ret = SQLSetStmtAttr(m_pHStmt->GetHandle(), SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER)SQL_SCROLLABLE, 0);
				THROW_IFN_SUCCEEDED_MSG(SQLSetStmtAttr, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle(), u8"Failed to set Statement Attr SQL_ATTR_CURSOR_SCROLLABLE to SQL_SCROLLABLE");
				m_forwardOnlyCursors = false;
			}
		}
		catch (const SqlResultException& sre)
		{
			LOG_WARNING(boost::str(boost::format(u8"Failed to set Cursor Options, assuming forwardOnlyCursors: %s") % sre.ToString()));
			m_forwardOnlyCursors = true;
		}
	}


	void ExecutableStatement::ExecuteDirect(const std::string& sqlstmt)
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(!sqlstmt.empty());
		// Always discard pending results first
		SelectClose();

		SQLRETURN ret = SQLExecDirect(m_pHStmt->GetHandle(),  EXODBCSTR_TO_SQLAPICHARPTR(sqlstmt), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLExecDirect, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
	}


	void ExecutableStatement::Prepare(const std::string& sqlstmt)
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(!sqlstmt.empty());

		SQLRETURN ret = SQLPrepare(m_pHStmt->GetHandle(),  EXODBCSTR_TO_SQLAPICHARPTR(sqlstmt), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		m_isPrepared = true;
	}


	SParameterDescription ExecutableStatement::DescribeParameter(SQLUSMALLINT paramNr) const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(paramNr >= 1);

		SParameterDescription paramDesc;
		SQLRETURN ret = SQLDescribeParam(m_pHStmt->GetHandle(), paramNr, &paramDesc.m_sqlType, &paramDesc.m_charSize, &paramDesc.m_decimalDigits, &paramDesc.m_nullable);
		THROW_IFN_SUCCESS(SQLDescribeParam, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
		return paramDesc;
	}


	SColumnDescription ExecutableStatement::DescribeColumn(SQLUSMALLINT columnNr) const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(columnNr >= 1);
		exASSERT(m_pDb);

		SQLUSMALLINT maxColName = m_pDb->GetDbInfo().GetUSmallIntProperty(DatabaseInfo::USmallIntProperty::MaxColumnNameLen);
		if (maxColName <= 0)
		{
			maxColName = DB_MAX_COLUMN_NAME_LEN_DEFAULT;
		}
		std::unique_ptr<SQLAPICHARTYPE[]> nameBuffer(new SQLAPICHARTYPE[maxColName + 1]);
		SColumnDescription colDesc;
		SQLRETURN ret = SQLDescribeCol(m_pHStmt->GetHandle(), columnNr, nameBuffer.get(), maxColName + 1, NULL, &colDesc.m_sqlType, &colDesc.m_charSize, &colDesc.m_decimalDigits, &colDesc.m_nullable);
		THROW_IFN_SUCCEEDED(SQLDescribeCol, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		colDesc.m_name = SQLAPICHARPTR_TO_EXODBCSTR(nameBuffer.get());

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

		SParameterDescription paramDesc;
		if (!neverQueryParamDesc && IsPrepared() && DatabaseSupportsDescribeParam(m_pDb->GetDbms(), boost::apply_visitor(SqlCTypeVisitor(), column)))
		{
			paramDesc = DescribeParameter(paramNr);
		}
		else
		{
			paramDesc = boost::apply_visitor(SParamDescVisitor(), column);
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
		exASSERT(!m_forwardOnlyCursors);
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
