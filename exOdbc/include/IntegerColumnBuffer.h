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
		static const int VAR_SMALLINT = 0;
		static const int VAR_INT = 1;
		static const int VAR_BIGINT = 2;

		typedef boost::variant<
			boost::recursive_wrapper<SqlSmallIntBuffer>, 
			boost::recursive_wrapper<SqlIntBuffer>,
			boost::recursive_wrapper<SqlBigIntBuffer>
		> IntegerVariant;

	public:
		static std::shared_ptr<IColumnBuffer> CreateBuffer(SQLSMALLINT sqlCType);

		IntegerColumnBuffer(SQLSMALLINT sqlCType);

		virtual void BindSelect(SQLHSTMT hStmt, SQLSMALLINT columnNr) const;
		virtual SQLSMALLINT GetSqlCType() const;

	private:
		// Allow creation only with sqlCType param
		IntegerColumnBuffer() { exASSERT(false);  };

		IntegerVariant m_intVariant;
	};
} // namespace exodbc
