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
	typedef boost::variant<SQLSMALLINT*, SQLINTEGER*, SQLBIGINT*, 
		SQLCHAR*, SQLWCHAR*, 
		SQLDOUBLE*,
		SQL_DATE_STRUCT*, SQL_TIME_STRUCT*, SQL_TIMESTAMP_STRUCT*
#if HAVE_MSODBCSQL_H
		,SQL_SS_TIME2_STRUCT*
#endif
	> BufferPtrVariant;

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
	* a list of all supported SQL-Types and the buffer-type created:
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
	* SQL_DATE					| SQL_DATE_STRUCT*
	* SQL_TIME					| SQL_TIME_STRUCT*
	* SQL_TIME2					| SQL_TIME2_STRUCT* / SQL_TIME_STRUCT* [2]
	* SQL_TIMESTAMP				| SQL_TIMESTAMP_STRUCT*
	* SQL_BINARY				| SQL_CHAR*
	* SQL_VARBINARY				| SQL_CHAR*
	* SQL_LONGVARBINARY			| SQL_CHAR*
	*
	* [1] There are options about wide-chars: You can either enforce that a column
	* reported as CHAR / VARCHAR from the driver (or by you) is still bound to
	* a SQLWCHAR* buffer and the other way round, using the AutoBindingMode
	* value.
	*
	* [2] The SQL_TIME2 is a Microsoft SQL Server specific extension. It is only available
	* if HAVE_MSODBCSQL_H is defined to 1. If HAVE_MSODBCSQL_H
	* is not set to 1, the SQL-Type SQL_TIME2 is not supported. If HAVE_MSODBC_SQL_H is defined to 1 and
	* the ODBC version is >= 3.8, SQL_TIME2 is mapped to a SQL_TIME2_STRUCT, else it is mapped
	* to an SQL_TIME_STRUCT.
	*/
	class EXODBCAPI ColumnBuffer
	{
	public:
		/*!
		* \brief	Keeps only the first decimalDigits in value.
		* \detailed	Reads number decimalDigits from the left of the passed value and
		*			removes the rest to the right. So, for example (3, 12345) becomes
		*			123.
		* \param	decimalDigits How many Digits to read from the left of value.
		*
		*/
		static void TrimValue(SQLSMALLINT decimalDigits, SQLUINTEGER& value);

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
		ColumnBuffer(const SColumnInfo& columnInfo, AutoBindingMode mode, OdbcVersion odbcVersion);


		/*!
		* \brief	Create a new ColumnBuffer that will use the buffer given inside BufferPtrVariant.
		* \detailed	The constructor will not try to allocate a buffer on its own but update
		*			its internal variant with the value of the passed bufferVariant and the corresponding
		*			information.
		*			The ColumnBuffer will not take ownership of the passed bufferVariant and will
		*			not delete it.
		*			Note that using this constructor you can pass almost any combination of conversions 
		*			from SQL Types to ODBC C Types to the driver, as long as the buffer-type exists in the
		*			BufferPtrVariant. Bind() will fail if the driver does not support the given conversion.
		* \param sqlCType	The ODBC C Type of the buffer. This value will be forwarded to the driver during SQLBindCol. 
		* \param ordinalPosition The ordinal position of the column in the table. Numbering starts at 1 (!)
		* \param bufferPtrVarian Pointer to allocated buffer for the given sqlCType.
		* \param bufferSize	Size of the allocated buffer.
		* \param queryName Name of the column that corresponds to this buffer.
		* \param decimalDigits		Number of decimal digits. Set to -1 to indicate unknown.
		*
		* \see	HaveBuffer()
		* \see	Bind()
		*/
		ColumnBuffer(SQLSMALLINT sqlCType, SQLUSMALLINT ordinalPosition, BufferPtrVariant bufferPtrVariant, SQLLEN bufferSize, const std::wstring& queryName, SQLSMALLINT decimalDigits = -1);


		/*!
		* \brief	Create a new ColumnBuffer that will convert everything to a string (bind to a char* or wchar* buffer).
		* \detailed	The constructor will try to allocate a char* or wchar* buffer that has enough space
		*			for the passed stringLen (+ trailing zero).
		* \param ordinalPosition	The ordinal position of the column in the table. Numbering starts at 1 (!)
		* \param stringLen			The String-length of the buffer. This is not byte-length!
		* \param mode				Determines if a char* or a wchar* buffer is allocated. You are not allowed
		*							to use AutoBindingMode::BIND_AS_REPORTED .
		* \param queryName			Name of the column that corresponds to this buffer.
		*
		* \see	HaveBuffer()
		* \see	Bind()
		*/
		ColumnBuffer(SQLUSMALLINT ordinalPosition, SQLLEN stringLen, AutoBindingMode mode, const std::wstring& queryName);


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
		bool HasBuffer() const { return m_haveBuffer; };


		/*!
		* \brief	Returns size of this buffer, fail if no buffer is allocated.
		* \see		HasBuffer()
		* \return	Buffer-size
		*/
		SQLINTEGER GetBufferSize() const { exASSERT(HasBuffer());  return m_bufferSize; };


		/*!
		* \brief	Returns type of this buffer.
		* \return	Buffer-type.
		*/
		SQLSMALLINT GetBufferType() const { return m_bufferType; };


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
		SQLLEN GetCb() const { exASSERT(HasBuffer()); exASSERT(IsBound()); return  m_cb; };


		/*!
		* \brief	Check if the current value is NULL.
		* \detailed	Fails if no buffer is allocated or not bound.
		* \return	True if current value is Null.
		*/
		bool IsNull() const { exASSERT(HasBuffer()); exASSERT(IsBound()); return m_cb == SQL_NULL_DATA; };

		
		/*!
		* \brief	Check if the is SQL_NO_TOTAL.
		* \detailed	Fails if no buffer is allocated or not bound.
		* \return	True if current value is SQL_NO_TOTAL.
		*/
		bool NoTotal() const { exASSERT(HasBuffer()); exASSERT(IsBound()); return m_cb == SQL_NO_TOTAL; };


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
		* \see		AutoBindingMode
		*/
		void SetAutoBindingMode(AutoBindingMode mode) { exASSERT(!m_bound); m_autoBindingMode = mode; }


		/*!
		* \brief	Get the currently set AutoBindingMode.
		* \return	AutoBindingMode set.
		*/
		AutoBindingMode GetAutoBindingMode() const { return m_autoBindingMode; };

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
		* \see		WStringVisitor
		*/
		operator std::wstring() const;

		
		/*!
		* \brief	Cast the current value to a std::wstring if possible.
		* \detailed	Fails if not bound.
		* \return	Current value as std::wstring.
		* \throw	CastException If value cannot be casted to a std::string.
		* \see		StringVisitor
		*/
		operator std::string() const;


		/*!
		* \brief	Cast the current value to a SQLDOUBLE if possible.
		* \detailed	Fails if not bound.
		* \return	Current value as SQLDOUBLE.
		* \throw	CastException If value cannot be casted to a SQLDOUBLE.
		* \see		DoubleVisitor
		*/
		operator double() const;


		/*!
		* \brief	Cast the current value to a SQL_DATE_STRUCT if possible.
		* \detailed	Fails if not bound. If the value is a Timestamp, the time-part is ignored.
		* \return	Current value as SQL_DATE_STRUCT.
		* \throw	CastException If value cannot be casted to a SQL_DATE_STRUCT.
		* \see		TimestampVisitor
		*/
		operator SQL_DATE_STRUCT() const;


		/*!
		* \brief	Cast the current value to a SQL_TIME_STRUCT if possible.
		* \detailed	Fails if not bound. If the value is a Timestamp, the date-part is ignored.
		* \return	Current value as SQL_TIME_STRUCT.
		* \throw	CastException If value cannot be casted to a SQL_TIME_STRUCT.
		* \see		TimestampVisitor
		*/
		operator SQL_TIME_STRUCT() const;


#if HAVE_MSODBCSQL_H
		/*!
		* \brief	Cast the current value to a SQL_SS_TIME2_STRUCT if possible.
		* \detailed	Fails if not bound. If the value is a Timestamp, the date-part is ignored.
		*			Only available if HAVE_MSODBCSQL_H is defined to 1.
		* \return	Current value as SQL_SS_TIME2_STRUCT.
		* \throw	CastException If value cannot be casted to a SQL_SS_TIME2_STRUCT.
		* \see		TimestampVisitor
		*/
		operator SQL_SS_TIME2_STRUCT() const;
#endif


		/*!
		* \brief	Cast the current value to a SQL_TIMESTAMP_STRUCT if possible.
		* \detailed	Fails if not bound.
		* \return	Current value as SQL_TIMESTAMP_STRUCT.
		* \throw	CastException If value cannot be casted to a SQL_TIMESTAMP_STRUCT.
		* \see		TimestampVisitor
		*/
		operator SQL_TIMESTAMP_STRUCT() const;


		/*!
		* \brief	Access the current buffer value as a const SQLCHAR*
		* \detailed	Returns the same pointer as it is stored in here. This is mainly used
		*			for accessing binary data, to avoid to copy the binary buffer.
		* \return	Const SQLCHAR* to the buffer-content.
		* \see		TimestampVisitor
		*/
		operator const SQLCHAR*() const;


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
		SQLINTEGER DetermineBufferSize() const;


		/*!
		* \brief	Tries to determine the size needed in chars if this ColumnBuffer is bound to a
		*			char-type plus the terminating 0 char. Uses the passed SColumnInfo.
		* \detailed Evaluates NUM_PREC_RADIX, COLUMN_SIZE and DECIMAL_DIGITS to determine the number
		*			of chars needed to store numeric-types.
		*			For date time types it uses ColumnSize.
		*			For char-sizes it is the column-size.
		*			For every type +1 is added for the terminating 0 char.
		* \see		http://msdn.microsoft.com/en-us/library/ms711683%28v=vs.85%29.aspx
		* \return	Needed buffer size according to SQL type form SColumnInfo or 0 in case of failure.
		*/
		SQLINTEGER DetermineCharSize() const;


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
		* \brief	Allocate a buffer in the variant.
		* \detailed Determines the buffer type and sets m_bufferType, m_bufferSize and allocates corresponding buffer. 
		*			Sets m_allocatedBuffer to true on success.
		* \return	true if buffer is allocated.
		*/
		bool AllocateBuffer();


		/*!
		* \brief	Return the number of decimal digits if known or -1 otherwise.
		* \detailed If a SColumnInfo is available returns the value SColumnInfo::m_decimalDigits
		*			if the value is not set to NULL. Else the value of m_decimalDigits is returned,
		*			which defaults to -1.
		* \return	Number of decimal digits if known or -1.
		*/
		SQLSMALLINT GetDecimalDigitals() const;


		// Helpers to quickly access the pointers inside the variant.
		// All of these could throw a boost::bad_get
		SQLSMALLINT*	GetSmallIntPtr() const;
		SQLINTEGER*		GetIntPtr() const;
		SQLBIGINT*		GetBigIntPtr() const;
		SQLCHAR*		GetCharPtr() const;
		SQLWCHAR*		GetWCharPtr() const;
		SQLDOUBLE*		GetDoublePtr() const;
		SQL_DATE_STRUCT* GetDatePtr() const;
		SQL_TIME_STRUCT* GetTimePtr() const;
		SQL_TIMESTAMP_STRUCT* GetTimestampPtr() const;
#if HAVE_MSODBCSQL_H
		SQL_SS_TIME2_STRUCT* GetTime2Ptr() const;
#endif

		SColumnInfo m_columnInfo;	///< ColumnInformation matching this Buffer, only available if m_haveColumnInfo is true.
		bool		m_haveColumnInfo;	///< True if m_columnInfo contains a valid info-object.
		SQLSMALLINT m_decimalDigits;	///< Decimal digits, set from manual constructor.
		std::wstring m_queryName;	///< Name to use to query this Column. Either passed during construction, or read from m_columnInfo during construction.
		SQLUSMALLINT m_columnNr;	///< Either set on construction or read from SColumnInfo::m_ordinalPosition
		bool m_haveBuffer;			///< True if a buffer is available, either because it was allocated or passed during construction.
		bool m_allocatedBuffer;		///< True if Buffer has been allocated and must be deleted on destruction. Set from AllocateBuffer()
		SQLSMALLINT m_bufferType;	///< ODBC C Type of the buffer allocated, as it was passed to the driver. like SQL_C_WCHAR, etc. Set from ctor or during AllocateBuffer()
		SQLINTEGER	m_bufferSize;	///< Size of an allocated or set from constructor buffer.
		bool m_bound;				///< True if Bind() was successful.
		OdbcVersion m_odbcVersion;	///< OdbcVersion passed when creating this ColumnBuffer.
		AutoBindingMode m_autoBindingMode;	///< Determine if chars shall be bound as wchars, etc. Cannot be changed after bound.

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
		SQLBIGINT operator()(SQL_DATE_STRUCT* pTime) const { throw CastException(SQL_C_TYPE_DATE, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQL_TIME_STRUCT* pDate) const { throw CastException(SQL_C_TYPE_TIME, SQL_C_SBIGINT); };
		SQLBIGINT operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { throw CastException(SQL_C_TYPE_TIMESTAMP, SQL_C_SBIGINT); };
#if HAVE_MSODBCSQL_H
		SQLBIGINT operator()(SQL_SS_TIME2_STRUCT* pTime) const { throw CastException(SQL_C_SS_TIME2, SQL_C_SBIGINT); };
#endif
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
		std::wstring operator()(SQL_DATE_STRUCT* pTime) const { throw CastException(SQL_C_TYPE_DATE, SQL_C_WCHAR); };
		std::wstring operator()(SQL_TIME_STRUCT* pDate) const { throw CastException(SQL_C_TYPE_TIME, SQL_C_WCHAR); };
		std::wstring operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { throw CastException(SQL_C_TYPE_TIMESTAMP, SQL_C_WCHAR); };
#if HAVE_MSODBCSQL_H
		std::wstring operator()(SQL_SS_TIME2_STRUCT* pTime) const { throw CastException(SQL_C_SS_TIME2, SQL_C_WCHAR); };
#endif
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
		std::string operator()(SQL_DATE_STRUCT* pTime) const { throw CastException(SQL_C_TYPE_DATE, SQL_C_CHAR); };
		std::string operator()(SQL_TIME_STRUCT* pDate) const { throw CastException(SQL_C_TYPE_TIME, SQL_C_CHAR); };
		std::string operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { throw CastException(SQL_C_TYPE_TIMESTAMP, SQL_C_CHAR); };
#if HAVE_MSODBCSQL_H
		std::string operator()(SQL_SS_TIME2_STRUCT* pTime) const { throw CastException(SQL_C_SS_TIME2, SQL_C_CHAR); };
#endif
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
		SQLDOUBLE operator()(SQL_DATE_STRUCT* pTime) const { throw CastException(SQL_C_TYPE_DATE, SQL_C_DOUBLE); };
		SQLDOUBLE operator()(SQL_TIME_STRUCT* pDate) const { throw CastException(SQL_C_TYPE_TIME, SQL_C_DOUBLE); };
		SQLDOUBLE operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { throw CastException(SQL_C_TYPE_TIMESTAMP, SQL_C_DOUBLE); };
#if HAVE_MSODBCSQL_H
		SQLDOUBLE operator()(SQL_SS_TIME2_STRUCT* pTime) const { throw CastException(SQL_C_SS_TIME2, SQL_C_DOUBLE); };
#endif
	};	// class DoubleVisitor


	/*!
	* \class TimestampVisitor
	*
	* \brief Visitor to cast current value to a SQL_TIMESTAMP_STRUCT.
	*
	* This Visitor can cast the following sources to a SQL_TIMESTAMP_STRUCT :
	*
	* - SQLDATE* (time is set to 00:00:00)
	* - SQLTIME* (date is set to 00.00.0000)
	* - SQLTIMESTAMP*
	*
	*/
	class TimestampVisitor
		: public boost::static_visitor < SQL_TIMESTAMP_STRUCT >
	{
	public:
		TimestampVisitor(SQLSMALLINT decimalDigits)
			: m_decimalDigits(decimalDigits) {};

		SQL_TIMESTAMP_STRUCT operator()(SQLSMALLINT* smallInt) const { throw CastException(SQL_C_SSHORT, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLINTEGER* i) const { throw CastException(SQL_C_SSHORT, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLBIGINT* bigInt) const { throw CastException(SQL_C_SBIGINT, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLCHAR* pChar) const { throw CastException(SQL_C_CHAR, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLDOUBLE* pDouble) const { throw CastException(SQL_C_DOUBLE, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQL_DATE_STRUCT* pDate) const;
		SQL_TIMESTAMP_STRUCT operator()(SQL_TIME_STRUCT* pTime) const;
		SQL_TIMESTAMP_STRUCT operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const;
#if HAVE_MSODBCSQL_H
		SQL_TIMESTAMP_STRUCT operator()(SQL_SS_TIME2_STRUCT* pTime) const;
#endif

	private:
		SQLSMALLINT m_decimalDigits;
	};	// class TimestampVisitor


	/*!
	* \class CharPtrVisitor
	*
	* \brief Visitor to access the buffer.
	*
	* This Visitor extracts the buffer-pointer and returns it as const SQLCHAR*,
	* which works for every datatype.
	*
	*/
	class CharPtrVisitor
		: public boost::static_visitor < const SQLCHAR* >
	{
	public:

		const SQLCHAR* operator()(SQLSMALLINT* smallInt) const { return (const SQLCHAR*)(smallInt); };
		const SQLCHAR* operator()(SQLINTEGER* i) const { return (const SQLCHAR*)(i); };
		const SQLCHAR* operator()(SQLBIGINT* bigInt) const { return (const SQLCHAR*)(bigInt); };
		const SQLCHAR* operator()(SQLCHAR* pChar) const { return (const SQLCHAR*)(pChar); };
		const SQLCHAR* operator()(SQLWCHAR* pWChar) const { return (const SQLCHAR*)(pWChar); };
		const SQLCHAR* operator()(SQLDOUBLE* pDouble) const { return (const SQLCHAR*)(pDouble); };
		const SQLCHAR* operator()(SQL_DATE_STRUCT* pDate) const { return (const SQLCHAR*)(pDate); };
		const SQLCHAR* operator()(SQL_TIME_STRUCT* pTime) const { return (const SQLCHAR*)(pTime); };
		const SQLCHAR* operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { return (const SQLCHAR*)(pTimestamp); };
#if HAVE_MSODBCSQL_H
		const SQLCHAR* operator()(SQL_SS_TIME2_STRUCT* pTime) const { return (const SQLCHAR*)(pTime); };
#endif
	};
}


#endif // ColumnBuffer_H
