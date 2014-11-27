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
#include "Table.h"

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
	// Typedefs
	// --------

	// The Variant we use to store pointers to the actual buffer
	typedef boost::variant<SQLSMALLINT*, SQLINTEGER*, SQLBIGINT*, SQLCHAR*, SQLWCHAR*> BufferPtrVariant;

	// Structs
	// -------

	// Classes
	// -------
	/*!
	* \class ColumnBuffer
	*
	* \brief Provides the buffer to transfer data from a column of a record.
	*
	* A ColumnBuffer can allocate a corresponding buffer-type automatically
	* by reading the sql-type info from the passed SColumnInfo, or by using a
	* buffer provided during construction. In that case you must also pass the
	* ODBC C-Type of the buffer.
	* Last there is an option to try to let the odbc-driver to convert everything
	* to a (w)string.
	*
	* If the buffer-information is read from the passed SColumnInfo, the driver will
	* create a buffer type depending on the value of the SqlDataType. The following is
	* a list of all supported SQL-Types and the buffer-type created.
	*
	* SQL-Type					| Buffer-Type
	* --------------------------|------------
	* SQL_SMALLINT				| SQLSMALLINT*
	* SQL_INTEGER				| SQLINTEGER*
	* SQL_BIGINT				| SQLBIGINT*
	* SQL_CHAR / SQL_VARCHAR	| SQLCHAR* [1]
	* SQL_WCHAR / SQL_WVARCHAR	| SQLWCHAR* [1]
	* 
	* [1] There are options about wide-chars: You can either enforce that a column
	* reported as CHAR / VARCHAR from the driver (or by you) is still bound to
	* a SQLWCHAR* buffer and the other way round, using the CharBindingMode
	* value.
	*/
	class EXODBCAPI ColumnBuffer
	{
	public:
		/*!
		* \brief	Create a new ColumnBuffer that will allocate a corresponding buffer 
		*			using the datatype-information from the passed SColumnInfo.
		* \detailed	Might fail if SQL-type is not supported. 
		*		TODO: Some way to check that
		*
		* \see		???()
		*/
		ColumnBuffer(const SColumnInfo& columnInfo, CharBindingMode mode);

		//		ColumnBuffer(const SColumnInfo& columnInfo, boost::any* pBuffer);
//		ColumnBuffer(const STableColumnInfo& columnInfo, void* pBuffer);
	private:
		// We cannot be copied
		ColumnBuffer(const ColumnBuffer& other) {};
		ColumnBuffer() {};

	public:

		bool BindColumnBuffer(HSTMT hStmt);

		//Note: All These Get-Methods could throw a boost::bad_get
		SQLLEN GetCb() const {	return  m_cb;	};

		bool IsNull() const { return m_cb == SQL_NULL_DATA; };
		bool NoTotal() const { return m_cb == SQL_NO_TOTAL; };

		SColumnInfo GetColumnInfo() const { return m_columnInfo; };

		void SetCharBindingMode(CharBindingMode mode) { exASSERT(!m_isBound); m_charBindingMode = mode; }
		CharBindingMode GetCharBindingMode() const { return m_charBindingMode; };

		//// Type Information
		//bool IsSmallInt() const { return m_columnInfo.m_sqlDataType == SQL_SMALLINT; };
		//bool IsInt() const { return m_columnInfo.m_sqlDataType == SQL_INTEGER; };
		//bool IsBigInt() const { return m_columnInfo.m_sqlDataType == SQL_BIGINT; };
		//bool IsIntType() const { return (IsSmallInt() || IsInt() || IsBigInt()); };

		// Operators
		operator SQLSMALLINT() const;
		operator SQLINTEGER() const;
		operator SQLBIGINT() const;
		operator std::wstring() const;
		operator std::string() const;

	public:
		~ColumnBuffer();

	private:
		size_t GetBufferSize() const;
		void* GetBuffer();

		bool AllocateBuffer(const SColumnInfo& columnInfo);

		// Access to ptrs inside variant
		SQLSMALLINT*	GetSmallIntPtr() const;
		SQLINTEGER*		GetIntPtr() const;
		SQLBIGINT*		GetBigIntPtr() const;
		SQLCHAR*		GetCharPtr() const;
		SQLWCHAR*		GetWCharPtr() const;

		SColumnInfo m_columnInfo;
		bool m_allocatedBuffer;
		bool m_isBound;
		
		CharBindingMode m_charBindingMode;

		BufferPtrVariant m_intPtrVar;	///< Variant that holds the actual buffer

		SQLLEN		m_cb;	///< The length indicator set during Bind for this column

	};  // class ColumnBuffer


	class CastException :
		public boost::exception,
		public std::exception
	{
	public:
		CastException(SQLSMALLINT sqlSourceType, SQLSMALLINT sqlCType)
			: m_sqlSourceType(sqlSourceType)
			, m_sqlCType(sqlCType)
		{
			m_what = (boost::format("Cannot cast from SQL Type %s (%d) to ODBC C Type %s (%d)") % w2s(SqlType2s(m_sqlSourceType)) % m_sqlSourceType % w2s(SqlCType2OdbcS(sqlCType)) % m_sqlCType).str();
		}

		virtual const char* what() const throw() { return m_what.c_str(); };

	private:
		std::string m_what;
	public:
		const SQLSMALLINT m_sqlSourceType;
		const SQLSMALLINT m_sqlCType;
	};


	class BigintVisitor
		: public boost::static_visitor < SQLBIGINT >
	{
	public:
		BigintVisitor(SQLSMALLINT sqlDataType)
			: m_sqlDataType(sqlDataType) {};

		SQLBIGINT operator()(SQLSMALLINT* smallInt) const { return *smallInt;  };
		SQLBIGINT operator()(SQLINTEGER* i) const { return *i; };
		SQLBIGINT operator()(SQLBIGINT* bigInt) const { return *bigInt; };
		SQLBIGINT operator()(SQLCHAR* pChar) const { throw CastException(m_sqlDataType, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQLWCHAR* pWChar) const { throw CastException(m_sqlDataType, SQL_C_SBIGINT); };
	
	private:
		SQLSMALLINT m_sqlDataType;
	};


	class WStringVisitor
		: public boost::static_visitor < std::wstring >
	{
	public:
		WStringVisitor(SQLSMALLINT sqlDataType)
			: m_sqlDataType(sqlDataType) {};

		std::wstring operator()(SQLSMALLINT* smallInt) const { return (boost::wformat(L"%d") % *smallInt).str(); };
		std::wstring operator()(SQLINTEGER* i) const { return (boost::wformat(L"%d") % *i).str(); };
		std::wstring operator()(SQLBIGINT* bigInt) const { return (boost::wformat(L"%d") % *bigInt).str(); };
		std::wstring operator()(SQLCHAR* pChar) const{ throw CastException(m_sqlDataType, SQL_C_WCHAR); };
		std::wstring operator()(SQLWCHAR* pWChar) const { return pWChar; };

	private:
		SQLSMALLINT m_sqlDataType;
	};


	class StringVisitor
		: public boost::static_visitor < std::string >
	{
	public:
		StringVisitor(SQLSMALLINT sqlDataType)
			: m_sqlDataType(sqlDataType) {};

		std::string operator()(SQLSMALLINT* smallInt) const { return (boost::format("%d") % *smallInt).str(); };
		std::string operator()(SQLINTEGER* i) const { return (boost::format("%d") % *i).str(); };
		std::string operator()(SQLBIGINT* bigInt) const { return (boost::format("%d") % *bigInt).str(); };
		std::string operator()(SQLCHAR* pChar) const { return (char*)pChar; };
		std::string operator()(SQLWCHAR* pWChar) const { throw CastException(m_sqlDataType, SQL_C_CHAR); };

	private:
		SQLSMALLINT m_sqlDataType;
	};

}


#endif // ColumnBuffer_H
