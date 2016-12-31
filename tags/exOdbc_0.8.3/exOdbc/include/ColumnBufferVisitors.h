/*!
* \file ColumnBufferVisitors.h
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

	/*!
	* \class BindColumnVisitor
	* \brief Visitor to bind a ColumnBuffer to a statement handle as result set member.
	* \details On applying this visitor, it will call BindSelect(columnNr, pHStmt) on the variant.
	*/
	class BindColumnVisitor
		: public boost::static_visitor<void>
	{
	public:
		BindColumnVisitor() = delete;
		BindColumnVisitor(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
			: m_columnNr(columnNr)
			, m_pHStmt(pHStmt)
		{};

		template<typename T>
		void operator()(T& t) const
		{
			t->BindColumn(m_columnNr, m_pHStmt);
		}
	private:
		SQLUSMALLINT m_columnNr;
		ConstSqlStmtHandlePtr m_pHStmt;
	};


	/*!
	* \class BindParamVisitor
	* \brief Visitor to bind a ColumnBuffer to a statement handle as input parameter
	* \details On applying this visitor, it will call BindParameter(columnNr, pHStmt, m_useSqlDescribeParam) on the variant.
	*/
	class BindParamVisitor
		: public boost::static_visitor<void>
	{
	public:
		BindParamVisitor() = delete;
		BindParamVisitor(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, SParameterDescription paramDesc)
			: m_paramNr(paramNr)
			, m_pHStmt(pHStmt)
			, m_paramDesc(paramDesc)
		{};

		template<typename T>
		void operator()(T& t) const
		{
			t->BindParameter(m_paramNr, m_pHStmt, m_paramDesc);
		}

	private:
		SQLUSMALLINT m_paramNr;
		ConstSqlStmtHandlePtr m_pHStmt;
		SParameterDescription m_paramDesc;
	};

	
	/*!
	* \class QueryNameVisitor
	* \brief Visitor get the query name of a ColumnBuffer.
	* \details On applying this visitor, it will call GetQueryName() on the variant.
	*/
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


	/*!
	* \class IsNullVisitor
	* \brief Visitor to test if a ColumnBuffer is NULL.
	* \details On applying this visitor, it will call IsNull() on the variant.
	*/
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


	/*!
	* \class ColumnFlagsPtrVisitor
	* \brief Visitor to cast a variant to a ColumnFlagsPtr
	* \details	On applying this visitor, it will dynamic_pointer_cast the variant and throw
	*			an AssertionException if the cast fails.
	*/
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


	/*!
	* \class ExtendedColumnPropertiesHolderPtrVisitor
	* \brief Visitor to cast a variant to a ExtendedColumnPropertiesHolderPtr
	* \details	On applying this visitor, it will dynamic_pointer_cast the variant and throw
	*			an AssertionException if the cast fails.
	*/
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


	/*!
	* \class ColumnBufferLengthIndicatorPtrVisitor
	* \brief Visitor to cast a variant to a ColumnBufferLengthIndicatorPtr
	* \details	On applying this visitor, it will dynamic_pointer_cast the variant and throw
	*			an AssertionException if the cast fails.
	*/
	class ColumnBufferLengthIndicatorPtrVisitor
		: public boost::static_visitor < ColumnBufferLengthIndicatorPtr >
	{
	public:
		template<typename T>
		ColumnBufferLengthIndicatorPtr operator()(T& t) const
		{
			ColumnBufferLengthIndicatorPtr pCb = std::dynamic_pointer_cast<LengthIndicator>(t);
			exASSERT(pCb);
			return pCb;
		}
	};


	/*!
	* \class SqlCTypeVisitor
	* \brief Visitor get the SQL C Type of a ColumnBuffer.
	* \details On applying this visitor, it will call GetSqlCType() on the variant.
	*/
	class SqlCTypeVisitor
		: public boost::static_visitor<SQLSMALLINT>
	{
	public:
		template<typename T>
		SQLSMALLINT operator()(T& t) const
		{
			return t->GetSqlCType();
		}
	};


	/*!
	* \class SqlTypeVisitor
	* \brief Visitor get the SQL Type of a ColumnBuffer.
	* \details On applying this visitor, it will call GetSqlType() on the variant.
	*/
	class SqlTypeVisitor
		: public boost::static_visitor<SQLSMALLINT>
	{
	public:
		template<typename T>
		SQLSMALLINT operator()(T& t) const
		{
			return t->GetSqlType();
		}
	};


	/*!
	* \class SParamDescVisitor
	* \brief Visitor get create a parameter description from a ColumnBuffer
	* \details On applying this visitor, it will call CreateParamDescFromProps() on the variant.
	*/
	class SParamDescVisitor
		: public boost::static_visitor<SParameterDescription>
	{
	public:
		template<typename T>
		SParameterDescription operator()(T& t) const
		{
			return t->CreateParamDescFromProps();
		}
	};


} // namespace exodbc