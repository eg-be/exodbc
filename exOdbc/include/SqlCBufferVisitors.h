/*!
* \file SqlCBufferVisitors.h
* \author Elias Gerber <eg@elisium.ch>
* \date 20.12.2015
* \brief Header file for the Visitors to visit SqlCBuffer.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlCBuffer.h"
#include "PreparedStatement.h"

// Other headers
// System headers

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

	// Visitors
	// --------
	class BindSelectVisitor
		: public boost::static_visitor<void>
	{
	public:
		BindSelectVisitor() = delete;
		BindSelectVisitor(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
			: m_columnNr(columnNr)
			, m_pHStmt(pHStmt)
		{};

		template<typename T>
		void operator()(T& t) const
		{
			t.BindSelect(m_columnNr, m_pHStmt);
		}
	private:
		SQLUSMALLINT m_columnNr;
		ConstSqlStmtHandlePtr m_pHStmt;
	};

	class BindParamVisitor
		: public boost::static_visitor<void>
	{
	public:
		BindParamVisitor() = delete;
		BindParamVisitor(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, bool useSqlDescribeParam)
			: m_paramNr(paramNr)
			, m_pHStmt(pHStmt)
			, m_useSqlDescribeParam(useSqlDescribeParam)
		{};
		BindParamVisitor(SQLUSMALLINT paramNr, const PreparedStatement& prepStmt)
			: m_paramNr(paramNr)
			, m_pHStmt(prepStmt.GetStmt())
			, m_useSqlDescribeParam(prepStmt.GetUseSqlDescribeParam())
		{};

		template<typename T>
		void operator()(T& t) const
		{
			t.BindParameter(m_paramNr, m_pHStmt, m_useSqlDescribeParam);
		}

	private:
		SQLUSMALLINT m_paramNr;
		ConstSqlStmtHandlePtr m_pHStmt;
		bool m_useSqlDescribeParam;
	};

	class UnbindVisitor
		: public boost::static_visitor<void>
	{
	public:

		template<typename T>
		void operator()(T& t) const
		{
			t.Unbind();
		}
	};

	class QueryNameVisitor
		: public boost::static_visitor<const std::wstring&>
	{
	public:
		template<typename T>
		const std::wstring& operator()(T& t) const
		{
			return t.GetQueryName();
		}
	};

	class SqlCTypeVisitor
		: public boost::static_visitor<SQLSMALLINT>
	{
	public:
		template<typename T>
		const SQLSMALLINT operator()(T& t) const
		{
			return t.GetSqlCType();
		}
	};

} // namespace exodbc
