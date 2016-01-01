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
#include "ColumnBuffer.h"

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
			t->BindSelect(m_columnNr, m_pHStmt);
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

		template<typename T>
		void operator()(T& t) const
		{
			t->BindParameter(m_paramNr, m_pHStmt, m_useSqlDescribeParam);
		}

	private:
		SQLUSMALLINT m_paramNr;
		ConstSqlStmtHandlePtr m_pHStmt;
		bool m_useSqlDescribeParam;
	};

	class QueryNameVisitor
		: public boost::static_visitor<const std::wstring&>
	{
	public:
		template<typename T>
		const std::wstring& operator()(T& t) const
		{
			return t->GetQueryName();
		}
	};

	class IsNullVisitor
		: public boost::static_visitor<bool>
	{
	public:
		template<typename T>
		bool operator()(T& t) const
		{
			return t->IsNull();
		}
	};

	class ColumnFlagsPtrVisitor
		: public boost::static_visitor<std::shared_ptr<ColumnFlags>>
	{
	public:
		template<typename T>
		std::shared_ptr<ColumnFlags> operator()(T& t) const
		{
			std::shared_ptr<ColumnFlags> pFlags = std::dynamic_pointer_cast<ColumnFlags>(t);
			exASSERT(pFlags);
			return pFlags;
		}
	};

	class ExtendedColumnPropertiesHolderPtrVisitor
		: public boost::static_visitor<std::shared_ptr<ExtendedColumnPropertiesHolder>>
	{
	public:
		template<typename T>
		std::shared_ptr<ExtendedColumnPropertiesHolder> operator()(T& t) const
		{
			std::shared_ptr<ExtendedColumnPropertiesHolder> pProps = std::dynamic_pointer_cast<ExtendedColumnPropertiesHolder>(t);
			exASSERT(pProps);
			return pProps;
		}
	};

	class ColumnBufferLengthIndicatorPtrVisitor
		: public boost::static_visitor < ColumnBufferLengthIndicatorPtr >
	{
	public:
		template<typename T>
		ColumnBufferLengthIndicatorPtr operator()(T& t) const
		{
			ColumnBufferLengthIndicatorPtr pCb = std::dynamic_pointer_cast<ColumnBufferLengthIndicator>(t);
			exASSERT(pCb);
			return pCb;
		}
	};

	class SqlCTypeVisitor
		: public boost::static_visitor<SQLSMALLINT>
	{
	public:
		template<typename T>
		const SQLSMALLINT operator()(T& t) const
		{
			return t->GetSqlCType();
		}
	};


	class SqlTypeVisitor
		: public boost::static_visitor<SQLSMALLINT>
	{
	public:
		template<typename T>
		const SQLSMALLINT operator()(T& t) const
		{
			return t->GetSqlType();
		}
	};


} // namespace exodbc
