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
	// Typedefs
	// --------

	// The Variant we use to store pointers to the actual buffer
	typedef boost::variant<SQLSMALLINT*, SQLINTEGER*, SQLBIGINT*, SQLCHAR*, SQLWCHAR*, SQLDOUBLE*> BufferPtrVariant;

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
	* by reading the SQL-type info from the passed SColumnInfo, or by using a
	* buffer provided during construction. In that case you must also pass the
	* ODBC C-Type of the buffer and the query name of the corresponding column.
	*
	* Last there is an option to try to let the ODBC-driver to convert everything
	* to a (w)string.
	*
	* If the buffer-information is read from the passed SColumnInfo, the ColumnBuffer will
	* allocate a buffer type depending on the value of the SqlDataType. The following is
	* a list of all supported SQL-Types and the buffer-type created.
	*
	* SQL-Type					| Buffer-Type
	* --------------------------|------------
	* SQL_SMALLINT				| SQLSMALLINT*
	* SQL_INTEGER				| SQLINTEGER*
	* SQL_BIGINT				| SQLBIGINT*
	* SQL_CHAR / SQL_VARCHAR	| SQLCHAR* [1]
	* SQL_WCHAR / SQL_WVARCHAR	| SQLWCHAR* [1]
	* SQL_DOUBLE				| SQLDOUBLE*
	* SQL_FLOAT					| SQLDOUBLE*
	* SQL_REAL					| SQLDOUBLE*
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
		*			using the data type information from the passed SColumnInfo.
		* \detailed	The constructor will try to allocate a corresponding buffer.
		* \param columnInfo	The Information about the column we bind.
		* \param mode		How Character columns should be bound
		*
		* \see	HaveBuffer()
		* \see	Bind()
		*/
		ColumnBuffer(const SColumnInfo& columnInfo, CharBindingMode mode);


		/*!
		* \brief	Create a new ColumnBuffer that will use the buffer given inside BufferPtrVariant.
		* \detailed	The constructor will not try to allocate a buffer on its own but update
		*			its internal variant with the value of the passed bufferVariant and the corresponding
		*			information.
		*			The ColumnBuffer will not take ownership of the passed bufferVariant and will
		*			not delete it.
		*			Note that using this constructor you can pass almost any combination of conversions 
		*			from SQL Types to ODBC C Types to the driver. Bind() will fail if the driver does not
		*			support the given conversion.
		* \param sqlCType	The ODBC C Type of the buffer. This value will be forwarded to the driver during SQLBindCol. 
		* \param ordinalPosition The ordinal position of the column in the table. Numbering starts at 1 (!)
		* \param bufferPtrVarian Pointer to allocated buffer for the given sqlCType.
		* \param bufferSize	Size of the allocated buffer.
		* \param queryName Name of the column that corresponds to this buffer.
		*
		* \see	HaveBuffer()
		* \see	Bind()
		*/
		ColumnBuffer(SQLSMALLINT sqlCType, SQLUSMALLINT ordinalPosition, BufferPtrVariant bufferPtrVariant, SQLLEN bufferSize, const std::wstring& queryName);


		/*!
		* \brief	Create a new ColumnBuffer that will convert everything to a string (bind to a char* or wchar* buffer).
		* \detailed	The constructor will try to allocate a char* or wchar* buffer that has enough space
		*			for the passed stringLen (+ trailing zero).
		* \param ordinalPosition	The ordinal position of the column in the table. Numbering starts at 1 (!)
		* \param stringLen			The String-length of the buffer. This is not byte-length!
		* \param mode				Determines if a char* or a wchar* buffer is allocated. You are not allowed
		*							to use CharBindingMode::BIND_AS_REPORTED .
		* \param queryName			Name of the column that corresponds to this buffer.
		*
		* \see	HaveBuffer()
		* \see	Bind()
		*/
		ColumnBuffer(SQLUSMALLINT ordinalPosition, SQLLEN stringLen, CharBindingMode mode, const std::wstring& queryName);


		~ColumnBuffer();

	private:
		// We cannot be copied
		ColumnBuffer(const ColumnBuffer& other) { exASSERT(false);  };
		ColumnBuffer() {};

	public:
		/*!
		* \brief	Returns true if this ColumnBuffer has a buffer ready.
		* \detailed	This can be either true because during construction a buffer
		*			was allocated or because you've manually set a buffer.
		* \return	True if buffer is ready.
		*/
		bool HaveBuffer() const { return m_haveBuffer; };


		/*!
		* \brief	Tries to bind the buffer to the column using SQLBindCol.
		* \detailed	Fails if no buffer is allocated or if already bound.
		*			The driver might fail to bind the column to the type.
		* \param	hStmt ODBC Statement handle to bind this ColumnBuffer to.
		* \return	True if bound successful.
		*/
		bool Bind(HSTMT hStmt);


		/*!
		* \brief	Returns true if this ColumnBuffer is bound.
		* \return	True if Column is bound.
		*/
		bool IsBound() const { return m_bound; };


		/*!
		* \brief	Access the length indicator passed to SQLBindCol.
		* \detailed	Fails if no buffer is allocated or not bound.
		* \return	Length indicator bound to column.
		*/
		SQLLEN GetCb() const { exASSERT(HaveBuffer()); exASSERT(IsBound()); return  m_cb; };


		/*!
		* \brief	Check if the current value is NULL.
		* \detailed	Fails if no buffer is allocated or not bound.
		* \return	True if current value is Null.
		*/
		bool IsNull() const { exASSERT(HaveBuffer()); exASSERT(IsBound()); return m_cb == SQL_NULL_DATA; };

		
		/*!
		* \brief	Check if the is SQL_NO_TOTAL.
		* \detailed	Fails if no buffer is allocated or not bound.
		* \return	True if current value is SQL_NO_TOTAL.
		*/
		bool NoTotal() const { exASSERT(HaveBuffer()); exASSERT(IsBound()); return m_cb == SQL_NO_TOTAL; };


		/*!
		* \brief	Check if a valid SColumnInfo is available for this ColumnBuffer.
		* \detailed	True if this ColumnBuffer was created by passing a SColumnInfo object.
		* \return	True if a SColumnInfo can be fetched using GetColumnInfo.
		* \see		GetColumnInfo()
		*/
		bool HaveColumnInfo() const { return  m_haveColumnInfo; };


		/*!
		* \brief	Get the SColumnInfo used within this ColumnBuffer.
		* \detailed	Fails if no SColumnInfo has been set on this ColumnBuffer.
		* \return	SColumnInfo set on this ColumnBuffer.
		* \see		HaveColumnInfo()
		*/
		SColumnInfo GetColumnInfo() const { exASSERT(m_haveColumnInfo);  return m_columnInfo; };


		/*!
		* \brief	Get the query name for this ColumnBuffer.
		* \return	The query name of the column matching this ColumnBuffer.
		*/
		const std::wstring& GetQueryName() const { return m_queryName; };


		/*!
		* \brief	Set how Chars should be bound.
		* \detailed	Fails if already bound.
		* \see		CharBindingMode
		*/
		void SetCharBindingMode(CharBindingMode mode) { exASSERT(!m_bound); m_charBindingMode = mode; }


		/*!
		* \brief	Get the currently set CharBindingMode.
		* \return	CharBindingMode set.
		*/
		CharBindingMode GetCharBindingMode() const { return m_charBindingMode; };

		// Operators
		// ---------

		/*!
		* \brief	Cast the current value to a SQLSMALLINT if possible.
		* \detailed	Fails if not bound.
		* \return	Current value as SQLSMALLINT.
		* \throw	CastException If value cannot be casted to an SQLSMALLINT or if data loss would occur.
		*/
		operator SQLSMALLINT() const;


		/*!
		* \brief	Cast the current value to a SQLINTEGER if possible.
		* \detailed	Fails if not bound.
		* \return	Current value as SQLINTEGER.
		* \throw	CastException If value cannot be casted to an SQLINTEGER or if data loss would occur.
		*/
		operator SQLINTEGER() const;


		/*!
		* \brief	Cast the current value to a SQLBIGINT if possible.
		* \detailed	Fails if not bound.
		* \return	Current value as SQLBIGINT.
		* \throw	CastException If value cannot be casted to an SQLBIGINT.
		*/
		operator SQLBIGINT() const;


		/*!
		* \brief	Cast the current value to a std::wstring if possible.
		* \detailed	Fails if not bound.
		* \return	Current value as std::wstring.
		* \throw	CastException If value cannot be casted to a std::wstring.
		*/
		operator std::wstring() const;

		
		/*!
		* \brief	Cast the current value to a std::wstring if possible.
		* \detailed	Fails if not bound.
		* \return	Current value as std::wstring.
		* \throw	CastException If value cannot be casted to a std::string.
		*/
		operator std::string() const;


		/*!
		* \brief	Cast the current value to a SQLDOUBLE if possible.
		* \detailed	Fails if not bound.
		* \return	Current value as SQLDOUBLE.
		* \throw	CastException If value cannot be casted to a SQLDOUBLE.
		*/
		operator double() const;

	private:

		/*!
		* \brief	Determine the buffer size needed for the current SQL-Type given in SColumnInfo.
		* \detailed This is used internally if no buffer-size and buffer is given and the buffer must
		*			thus be allocated automatically.
		*			Must be called after m_bufferType is set.
		*			The size of types with fixed lengths is given by sizeof() (like sizeof(SQLINTEGER).
		*			For char-types the size is calculated by '(fieldlength + 1) * sizeof(char-type)',
		*			where char-type is SQLCCHAR or SQLWCHAR. One extra space is allocated for the 
		*			terminating \0.
		* \return	Size of the allocated buffer
		*/
		size_t DetermineBufferSize() const;


		/*!
		* \brief	Determine the buffer type for the current SQL type given in SColumnInfo.
		* \detailed This is used internally if no buffer-size and buffer is given and the buffer must
		*			thus be allocated automatically.
		*			See ColumnBuffer description for what SQL type is mapped to what buffer.
		* \return	Needed buffer size according to SQL type form SColumnInfo or 0 in case of failure.
		*/
		SQLSMALLINT DetermineBufferType() const;


		/*!
		* \brief	Get the allocated buffer as a void*.
		* \detailed Determines the type using the SColumnInfo and gets the pointer from the variant.
		*			Fails if no buffer is allocated.
		* \return	void* to the current buffer.
		* \throw	boost::bad_get If SQL-type does not match type in SQColumnInfo.
		*/
		void* GetBuffer();


		/*!
		* \brief	Get the allocated buffer as a void*.
		* \detailed Determines the type using the SColumnInfo and gets the pointer from the variant.
		* \return	true if buffer is allocated.
		* \throw	boost::bad_get If SQL-type does not match type in SQColumnInfo.
		*/
		bool AllocateBuffer(const SColumnInfo& columnInfo);


		// Helpers to quickly access the pointers inside the variant.
		// All of these could throw a boost::bad_get
		SQLSMALLINT*	GetSmallIntPtr() const;
		SQLINTEGER*		GetIntPtr() const;
		SQLBIGINT*		GetBigIntPtr() const;
		SQLCHAR*		GetCharPtr() const;
		SQLWCHAR*		GetWCharPtr() const;
		SQLDOUBLE*		GetDoublePtr() const;

		SColumnInfo m_columnInfo;	///< ColumnInformation matching this Buffer, only available if m_haveColumnInfo is true.
		bool		m_haveColumnInfo;	///< True if m_columnInfo contains a valid info-object.
		std::wstring m_queryName;	///< Name to use to query this Column. Either passed during construction, or read from m_columnInfo during construction.
		SQLUSMALLINT m_columnNr;	///< Either set on construction or read from SColumnInfo::m_ordinalPosition
		bool m_haveBuffer;			///< True if a buffer is available, either because it was allocated or passed during construction.
		bool m_allocatedBuffer;		///< True if Buffer has been allocated and must be deleted on destruction. Set from AllocateBuffer()
		SQLSMALLINT m_bufferType;	///< ODBC C Type of the buffer allocated, as it was passed to the driver. like SQL_C_WCHAR, etc. Set from ctor or during AllocateBuffer()
		SQLINTEGER	m_bufferSize;	///< Size of an allocated or set from constructor buffer.
		bool m_bound;				///< True if Bind() was successful.
		
		CharBindingMode m_charBindingMode;	///< Determine if chars shall be bound as wchars, etc. Cannot be changed after bound.

		BufferPtrVariant m_bufferPtr;	///< Variant that holds the pointer to the actual buffer

		SQLLEN		m_cb;	///< The length indicator set during Bind for this column

	};  // class ColumnBuffer


	/*!
	* \class CastException
	*
	* \brief Thrown if a Visitor cannot cast a value.
	*
	* Contains information about the source-type and the cast-target-type.
	*/
	class CastException :
		public boost::exception,
		public std::exception
	{
	public:
		CastException(SQLSMALLINT cSourceType, SQLSMALLINT cDestType)
			: m_cSourceType(cSourceType)
			, m_cDestType(cDestType)
		{
			m_what = (boost::format("Cannot cast from ODBC C Type %s (%d) to ODBC C Type %s (%d)") % w2s(SqlCType2OdbcS(m_cSourceType)) % m_cSourceType % w2s(SqlCType2OdbcS(m_cDestType)) % m_cDestType).str();
		}

		virtual const char* what() const throw() { return m_what.c_str(); };

	private:
		std::string m_what;
	public:
		const SQLSMALLINT m_cSourceType;	///< Sql Type used as source value for the cast.
		const SQLSMALLINT m_cDestType;		///< ODBC C Type used as destination value for the cast.
	};	// class CastException
	

	/*!
	* \class BigintVisitor
	*
	* \brief Visitor to cast current value to a SQLBIGINT.
	*
	* This Visitor can cast the following sources to a SQLBIGINT:
	* 
	* - SQLSMALLINT*
	* - SQLINTEGER*
	* - SQLBIGINT*
	* 
	* It will throw a CastException on the following source-types:
	* - SQLCHAR*
	* - SQLWCHAR*
	* - SQLDOUBLE*
	* 
	*/
	class BigintVisitor
		: public boost::static_visitor < SQLBIGINT >
	{
	public:		
		SQLBIGINT operator()(SQLSMALLINT* smallInt) const { return *smallInt;  };
		SQLBIGINT operator()(SQLINTEGER* i) const { return *i; };
		SQLBIGINT operator()(SQLBIGINT* bigInt) const { return *bigInt; };
		SQLBIGINT operator()(SQLCHAR* pChar) const { throw CastException(SQL_C_CHAR, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQLDOUBLE* pDouble) const { throw CastException(SQL_C_DOUBLE, SQL_C_SBIGINT); };
	};	// class BigintVisitor


	/*!
	* \class WStringVisitor
	*
	* \brief Visitor to cast current value to a std::wstring .
	*
	* This Visitor can cast the following sources to a std::wstring :
	*
	* - SQLSMALLINT*
	* - SQLINTEGER*
	* - SQLBIGINT*
	* - SQLWCHAR*
	* - SQLDOUBLE*
	* 
	* It will throw a CastException on the following source-types:
	* - SQLCHAR*
	*
	*/
	class WStringVisitor
		: public boost::static_visitor < std::wstring >
	{
	public:
		std::wstring operator()(SQLSMALLINT* smallInt) const { return (boost::wformat(L"%d") % *smallInt).str(); };
		std::wstring operator()(SQLINTEGER* i) const { return (boost::wformat(L"%d") % *i).str(); };
		std::wstring operator()(SQLBIGINT* bigInt) const { return (boost::wformat(L"%d") % *bigInt).str(); };
		std::wstring operator()(SQLCHAR* pChar) const{ throw CastException(SQL_C_CHAR, SQL_C_WCHAR); };
		std::wstring operator()(SQLWCHAR* pWChar) const { return pWChar; };
		std::wstring operator()(SQLDOUBLE* pDouble) const { return (boost::wformat(L"%f") % *pDouble).str(); };
	};	// class WStringVisitor


	/*!
	* \class StringVisitor
	*
	* \brief Visitor to cast current value to a std::string .
	*
	* This Visitor can cast the following sources to a std::string :
	*
	* - SQLSMALLINT*
	* - SQLINTEGER*
	* - SQLBIGINT*
	* - SQLCHAR*
	* - SQLDOUBLE*
	*
	* It will throw a CastException on the following source-types:
	* - SQLWCHAR*
	*
	*/
	class StringVisitor
		: public boost::static_visitor < std::string >
	{
	public:
		std::string operator()(SQLSMALLINT* smallInt) const { return (boost::format("%d") % *smallInt).str(); };
		std::string operator()(SQLINTEGER* i) const { return (boost::format("%d") % *i).str(); };
		std::string operator()(SQLBIGINT* bigInt) const { return (boost::format("%d") % *bigInt).str(); };
		std::string operator()(SQLCHAR* pChar) const { return (char*)pChar; };
		std::string operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_CHAR); };
		std::string operator()(SQLDOUBLE* pDouble) const { return (boost::format("%f") % *pDouble).str(); };
	};	// class StringVisitor


	/*!
	* \class DoubleVisitor
	*
	* \brief Visitor to cast current value to a SQLDOUBLE.
	*
	* This Visitor can cast the following sources to a SQLDOUBLE :
	*
	* - SQLSMALLINT*
	* - SQLINTEGER*
	* - SQLDOUBLE*
	*
	* It will throw a CastException on the following source-types:
	* - SQLWCHAR*
	* - SQLCHAR*
	* - SQLBIGINT*
	*
	*/
	class DoubleVisitor
		: public boost::static_visitor < SQLDOUBLE >
	{
	public:
		SQLDOUBLE operator()(SQLSMALLINT* smallInt) const { return *smallInt; };
		SQLDOUBLE operator()(SQLINTEGER* i) const { return *i; };
		SQLDOUBLE operator()(SQLBIGINT* bigInt) const { throw CastException(SQL_C_SBIGINT, SQL_C_DOUBLE); };
		SQLDOUBLE operator()(SQLCHAR* pChar) const { throw CastException(SQL_C_CHAR, SQL_C_DOUBLE); };
		SQLDOUBLE operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_DOUBLE); };
		SQLDOUBLE operator()(SQLDOUBLE* pDouble) const { return *pDouble; };
	};	// class DoubleVisitor
}


#endif // ColumnBuffer_H
