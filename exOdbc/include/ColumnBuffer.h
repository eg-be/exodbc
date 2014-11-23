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
#include "boost/any.hpp"
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
		ColumnBuffer(const SColumnInfo& columnInfo, boost::any* pBuffer);
//		ColumnBuffer(const STableColumnInfo& columnInfo, void* pBuffer);

		ColumnBuffer(const ColumnBuffer& other);

	private:
		ColumnBuffer() {};

	public:
		~ColumnBuffer();

	private:
		bool AllocateBuffer(const SColumnInfo& columnInfo);

		SColumnInfo m_columnInfo;
		bool m_allocatedBuffer;
		
		boost::any* m_pBuffer;
		BufferVariant m_buffer;
		IntegerVariant m_intVar;

	};  // class ColumnBuffer
}


#endif // ColumnBuffer_H
