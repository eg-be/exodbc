/*!
* \file IntegerColumnBuffer.h
* \author Elias Gerber <eg@elisium.ch>
* \date 18.10.2015
* \brief Header file for the CBufferType interface.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "Exception.h"
#include "ColumnBufferFactory.h"
#include "SqlCBuffer.h"

// Other headers
#include "boost/variant.hpp"

// System headers
#include <set>

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
	class IntegerColumnBuffer
		: public IColumnBuffer
	{
	public:
		static const std::set<SQLSMALLINT> s_supportedCTypes;

	private:
		// must match position of variant.which()
		static const int VAR_USHORT = 0;
		static const int VAR_ULONG = 1;
		static const int VAR_UBIGINT = 2;
		static const int VAR_SSHORT = 3;
		static const int VAR_SLONG = 4;
		static const int VAR_SBIGINT = 5;

		typedef boost::variant<
			boost::recursive_wrapper<SqlUShortBuffer>,
			boost::recursive_wrapper<SqlULongBuffer>,
			boost::recursive_wrapper<SqlUBigIntBuffer>,
			boost::recursive_wrapper<SqlShortBuffer>, 
			boost::recursive_wrapper<SqlLongBuffer>,
			boost::recursive_wrapper<SqlBigIntBuffer>
		> IntegerVariant;

	public:
		static IColumnBufferPtr CreateBuffer(SQLSMALLINT sqlCType);

		IntegerColumnBuffer(SQLSMALLINT sqlCType);

		template<typename T>
		void SetValue(const T& value);
		template<typename T>
		void GetValue(T& value) const;

		virtual void BindSelect(SQLHSTMT hStmt, SQLSMALLINT columnNr) const;
		virtual SQLSMALLINT GetSqlCType() const;

		virtual bool IsNull() const;
		virtual void SetNull();

	private:
		// Allow creation only with sqlCType param
		IntegerColumnBuffer() { exASSERT(false);  };
		IntegerColumnBuffer(const IntegerColumnBuffer& other) { exASSERT(false); };

		IntegerVariant m_intVariant;
	};


	template<typename T>
	void IntegerColumnBuffer::SetValue(const T& value)
	{
		try
		{
			switch (m_intVariant.which())
			{
			case VAR_USHORT:
				boost::get<SqlUShortBuffer>(m_intVariant).SetValue(value);
				break;
			case VAR_ULONG:
				boost::get<SqlULongBuffer>(m_intVariant).SetValue(value);
				break;
			case VAR_UBIGINT:
				boost::get<SqlUBigIntBuffer>(m_intVariant).SetValue(value);
				break;
			case VAR_SSHORT:
				boost::get<SqlShortBuffer>(m_intVariant).SetValue(value);
				break;
			case VAR_SLONG:
				boost::get<SqlLongBuffer>(m_intVariant).SetValue(value);
				break;
			case VAR_SBIGINT:
				boost::get<SqlBigIntBuffer>(m_intVariant).SetValue(value);
				break;
			default:
				exASSERT(false);
			}
		}
		catch (const boost::bad_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw;
		}
	}


	template<typename T>
	void IntegerColumnBuffer::GetValue(T& value) const
	{
		try
		{
			switch (m_intVariant.which())
			{
			case VAR_USHORT:
				value = boost::get<SqlUShortBuffer>(m_intVariant).GetValue();
				break;
			case VAR_ULONG:
				value = boost::get<SqlULongBuffer>(m_intVariant).GetValue();
				break;
			case VAR_UBIGINT:
				value = boost::get<SqlUBigIntBuffer>(m_intVariant).GetValue();
				break;
			case VAR_SSHORT:
				value = boost::get<SqlShortBuffer>(m_intVariant).GetValue();
				break;
			case VAR_SLONG:
				value = boost::get<SqlLongBuffer>(m_intVariant).GetValue();
				break;
			case VAR_SBIGINT:
				value = boost::get<SqlBigIntBuffer>(m_intVariant).GetValue();
				break;
			default:
				exASSERT(false);
			}
		}
		catch (const boost::bad_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw;
		}
	}
} // namespace exodbc
