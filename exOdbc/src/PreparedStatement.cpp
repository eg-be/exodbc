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

// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	PreparedStatement::PreparedStatement(ConstDatabasePtr pDb, const std::wstring& sqlstmt)
		: m_sqlstmt(sqlstmt)
		, m_useSqlDescribeParam(false)
	{
		exASSERT(pDb);
		exASSERT(pDb->IsOpen());

		m_useSqlDescribeParam = DatabaseSupportsDescribeParam(pDb->GetDbms());
		m_pHStmt = std::make_shared<SqlStmtHandle>(pDb->GetSqlDbcHandle());
		Prepare();
	}


	PreparedStatement::PreparedStatement(ConstSqlDbcHandlePtr pHDbc, DatabaseProduct dbc, const std::wstring& sqlstmt)
		: m_sqlstmt(sqlstmt)
		, m_useSqlDescribeParam(false)
	{
		m_useSqlDescribeParam = DatabaseSupportsDescribeParam(dbc);
		m_pHStmt = std::make_shared<SqlStmtHandle>(pHDbc);
		Prepare();
	}


	PreparedStatement::PreparedStatement(ConstSqlDbcHandlePtr pHDbc, bool useSqlDescribeParam, const std::wstring& sqlstmt)
		: m_sqlstmt(sqlstmt)
		, m_useSqlDescribeParam(useSqlDescribeParam)
	{
		m_pHStmt = std::make_shared<SqlStmtHandle>(pHDbc);
		Prepare();
	}


	PreparedStatement::~PreparedStatement()
	{
		try
		{
			// we should not care? should we?
			m_pHStmt->Free();
		}
		catch (const Exception& ex)
		{
			LOG_ERROR(ex.ToString());
		}
	}


	void PreparedStatement::Execute()
	{
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


	void PreparedStatement::Prepare()
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		SQLRETURN ret = SQLPrepare(m_pHStmt->GetHandle(), (SQLWCHAR*)m_sqlstmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
	}
}
