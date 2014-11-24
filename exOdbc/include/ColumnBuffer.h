/*!
* \file ColumnBuffer.h
* \author Elias Gerber <eg@zame.ch>
* \date 23.11.2014
* \brief Header file for the ColumnBuffer class and its helpers.
*
*/

#pragma once
#ifndef ColumnBuffer_H
#define ColumnBuffer_H

// Same component headers
#include "exOdbc.h"
#include "Helpers.h"

// Other headers
#include "boost/variant.hpp"

// System headers
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>


// Forward declarations
// --------------------

namespace exodbc
{
//	typedef boost::variant<SQLUSMALLINT*, SQLSMALLINT*, SQLUINTEGER*, SQLINTEGER*, SQLUBIGINT*, SQLBIGINT*> IntegerVariant;
	// We could also use just one variant for all types that are simple (not binary)?
	typedef boost::variant<SQLUSMALLINT, SQLSMALLINT, SQLUINTEGER, SQLINTEGER, SQLUBIGINT, SQLBIGINT> IntegerVariant;
	typedef boost::variant<SQL_DATE_STRUCT, SQL_TIME_STRUCT, SQL_TIMESTAMP_STRUCT> TimestampVariant;
	
	typedef boost::variant<IntegerVariant, TimestampVariant> BufferVariant;
	// Structs
	// -------

	// Classes
	// -------
	/*!
	* \class ColumnBuffer
	*
	* \brief Provides the buffer to transfer date from a column of a record.
	*
	*
	*/
	class EXODBCAPI ColumnBuffer
	{
	public:
		ColumnBuffer(const SColumnInfo& columnInfo);
//		ColumnBuffer(const SColumnInfo& columnInfo, boost::any* pBuffer);
//		ColumnBuffer(const STableColumnInfo& columnInfo, void* pBuffer);
	private:
		// We cannot be copied
		ColumnBuffer(const ColumnBuffer& other) {};
		ColumnBuffer() {};

	public:

		bool BindColumnBuffer(HSTMT hStmt);

		//Note: All These Get-Methods could throw a boost::bad_get
		SQLSMALLINT GetSmallInt() const;
		SQLINTEGER GetInt() const;
		SQLBIGINT GetBigInt() const;

		SQLLEN GetCb() const {	return  m_cb;	};

		bool IsNull() const { return m_cb == SQL_NULL_DATA; };
		bool NoTotal() const { return m_cb == SQL_NO_TOTAL; };

		SColumnInfo GetColumnInfo() const { return m_columnInfo; };

		bool IsIntType() const;

		bool IsSmallInt() const { return m_columnInfo.m_sqlDataType == SQL_SMALLINT; };
		bool IsInt() const { return m_columnInfo.m_sqlDataType == SQL_INTEGER; };
		bool IsBigInt() const { return m_columnInfo.m_sqlDataType == SQL_BIGINT; };

	public:
		~ColumnBuffer();

	private:
		size_t GetBufferSize() const;
		void* GetBuffer();

		bool AllocateBuffer(const SColumnInfo& columnInfo);

		SColumnInfo m_columnInfo;
		bool m_allocatedBuffer;
		
		//BufferVariant m_buffer;	///< Maybe we want one variant for all types?
		IntegerVariant m_intVar;	///< Or a logical variant?
		char*		m_pBinaryBuffer; ///< Allocated if this column has binary data
		SQLWCHAR*	m_pWCharBuffer;	///< Allocated if this column has char-data

		SQLLEN		m_cb;	///< The length indicator set during Bind for this column

	};  // class ColumnBuffer
}


#endif // ColumnBuffer_H
