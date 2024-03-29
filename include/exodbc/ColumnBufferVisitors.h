﻿/*!
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
#include "ParameterDescription.h"

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
	* \details On applying this visitor, it will call BindColumn(columnNr, pHStmt) on the ColumnBuffer.
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
	* \brief Visitor to bind a ColumnBuffer to a statement handle as input parameter.
	* \details On applying this visitor, it will call BindParameter(columnNr, pHStmt, paramDesc) on the ColumnBuffer.
	*/
	class BindParamVisitor
		: public boost::static_visitor<void>
	{
	public:
		BindParamVisitor() = delete;
		BindParamVisitor(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, const ParameterDescription& paramDesc)
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
		ParameterDescription m_paramDesc;
	};

	
	/*!
	* \class QueryNameVisitor
	* \brief Visitor get the query name of a ColumnBuffer.
	* \details On applying this visitor, it will call GetQueryName() on the ColumnBuffer.
	*/
	class QueryNameVisitor
		: public boost::static_visitor<std::string>
	{
	public:
		template<typename T>
		std::string operator()(T& t) const
		{
			return t->GetQueryName();
		}
	};


	/*!
	* \class IsNullVisitor
	* \brief Visitor to test if a ColumnBuffer is NULL.
	* \details On applying this visitor, it will call IsNull() on the ColumnBuffer.
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
	* \class SetNullVisitor
	* \brief Visitor to set a ColumnBuffer to NULL
	* \details On applying this visitor, it will call SetNull() on the ColumnBuffer.
	* \throw NotAllowedException If the flag ColumnFlag::CF_NULLABLE is not set.
	*/
	class SetNullVisitor
		: public boost::static_visitor<void>
	{
	public:
		template<typename T>
		void operator()(T& t) const
		{
			// Test if we are allowed to set the column to NULL
			ColumnFlagsPtr pFlags = std::dynamic_pointer_cast<ColumnFlags>(t);
			exASSERT(pFlags);
			if (!pFlags->Test(ColumnFlag::CF_NULLABLE))
			{
				std::string msg = boost::str(boost::format("ColumnFlag::CF_NULLABLE is not set on column '%s'") % t->GetQueryName());
				THROW_WITH_SOURCE(NotAllowedException, msg);
			}
			t->SetNull();
		}

	};


	/*!
	* \class ColumnFlagsPtrVisitor
	* \brief Visitor to cast a ColumnBuffer to a ColumnFlagsPtr
	* \details	On applying this visitor, it will dynamic_pointer_cast the ColumnBuffer and throw
	*			an AssertionException if the cast fails.
	*/
	class ColumnFlagsPtrVisitor
		: public boost::static_visitor<ColumnFlagsPtr>
	{
	public:
		template<typename T>
		ColumnFlagsPtr operator()(T& t) const
		{
			ColumnFlagsPtr pFlags = std::dynamic_pointer_cast<ColumnFlags>(t);
			exASSERT(pFlags);
			return pFlags;
		}
	};


	/*!
	* \class ColumnPropertiesPtrVisitor
	* \brief Visitor to cast a ColumnBuffer to a ColumnPropertiesPtr
	* \details	On applying this visitor, it will dynamic_pointer_cast the ColumnBuffer and throw
	*			an AssertionException if the cast fails.
	*/
	class ColumnPropertiesPtrVisitor
		: public boost::static_visitor<ColumnPropertiesPtr>
	{
	public:
		template<typename T>
		ColumnPropertiesPtr operator()(T& t) const
		{
			ColumnPropertiesPtr pProps = std::dynamic_pointer_cast<ColumnProperties>(t);
			exASSERT(pProps);
			return pProps;
		}
	};


	/*!
	* \class LengthIndicatorPtrVisitor
	* \brief Visitor to cast a ColumnBuffer to a LengthIndicatorPtr
	* \details	On applying this visitor, it will dynamic_pointer_cast the ColumnBuffer and throw
	*			an AssertionException if the cast fails.
	*/
	class LengthIndicatorPtrVisitor
		: public boost::static_visitor < LengthIndicatorPtr >
	{
	public:
		template<typename T>
		LengthIndicatorPtr operator()(T& t) const
		{
			LengthIndicatorPtr pCb = std::dynamic_pointer_cast<LengthIndicator>(t);
			exASSERT(pCb);
			return pCb;
		}
	};


	/*!
	* \class SqlCTypeVisitor
	* \brief Visitor get the SQL C Type of a ColumnBuffer as SQLSMALLINT.
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
	* \class ParamDescVisitor
	* \brief Visitor to create a parameter description from a ColumnBuffer
	* \details On applying this visitor, it will call CreateParamDescFromProps() on the ColumnBuffer.
	*/
	class ParamDescVisitor
		: public boost::static_visitor<ParameterDescription>
	{
	public:
		template<typename T>
		ParameterDescription operator()(T& t) const
		{
			return t->CreateParamDescFromProps();
		}
	};


	/*!
	* \class NrOfElementsVisitor
	* \brief Visitor to get the number of elements this buffer is holding (of its type) as SQLLEN.
	* \details On applying this visitor, it will call GetNrOfElements() on the ColumnBuffer.
	*/
	class NrOfElementsVisitor
		: public boost::static_visitor<SQLLEN>
	{
	public:
		template<typename T>
		SQLLEN operator()(T& t) const
		{
			return t->GetNrOfElements();
		}
	};

} // namespace exodbc
