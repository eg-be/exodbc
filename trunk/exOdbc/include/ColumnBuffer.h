/*!
* \file ColumnBuffer.h
* \author Elias Gerber <eg@elisium.ch>
* \date 23.11.2014
* \brief Header file for the ColumnBuffer class and its helpers.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#pragma once
#ifndef ColumnBuffer_H
#define ColumnBuffer_H

// Same component headers
#include "exOdbc.h"
#include "Helpers.h"
#include "Exception.h"

// Other headers
#include "boost/variant.hpp"
#include "boost/lexical_cast.hpp"

// System headers
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>
#include <map>


// Forward declarations
// --------------------

namespace exodbc
{
	// Typedefs
	// --------

	/*!
	* \typedef BufferPtrVariant
	* \brief The Variant we use to store pointers to the actual buffer
	* \details The following types can be stored:
	*  - SQLSMALLINT*
	*  - SQLINTEGER*
	*  - SQLBIGINT*
	*  - SQLCHAR*
	*  - SQLWCHAR*
	*  - SQLDOUBLE*
	*  - SQL_DATE_STRUCT*
	*  - SQL_TIME_STRUCT*
	*  - SQL_TIME2_STRUCT* / SQL_TIME_STRUCT*
	*  - SQL_TIMESTAMP_STRUCT*
	*  - SQL_NUMERIC_STRUCT*
	* 
	*/
	typedef boost::variant<SQLSMALLINT*, SQLINTEGER*, SQLBIGINT*, 
		SQLCHAR*, SQLWCHAR*, 
		SQLDOUBLE*,
		SQL_DATE_STRUCT*, SQL_TIME_STRUCT*, SQL_TIMESTAMP_STRUCT*,
		SQL_NUMERIC_STRUCT*
#if HAVE_MSODBCSQL_H
		,SQL_SS_TIME2_STRUCT*
#endif
	> BufferPtrVariant;

	/*!
	* \typedef BufferVariant
	* \brief A helper for setting / getting values. If you like it, work with a variant
	* \details The following types can be stored:
	*  - SQLSMALLINT
	*  - SQLINTEGER
	*  - SQLBIGINT
	*  - std::string
	*  - std::wstring
	*  - SQLDOUBLE
	*  - SQL_DATE_STRUCT
	*  - SQL_TIME_STRUCT
	*  - SQL_TIME2_STRUCT / SQL_TIME_STRUCT
	*  - SQL_TIMESTAMP_STRUCT
	*  - SQL_NUMERIC_STRUCT
	*/
	typedef boost::variant<SQLSMALLINT, SQLINTEGER, SQLBIGINT,
		std::string, std::wstring,
		SQLDOUBLE,
		SQL_DATE_STRUCT, SQL_TIME_STRUCT, SQL_TIMESTAMP_STRUCT,
		SQL_NUMERIC_STRUCT
#if HAVE_MSODBCSQL_H
		, SQL_SS_TIME2_STRUCT
#endif
	> BufferVariant;


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
	* by reading the SQL-type info from the passed SColumnInfo, or use a
	* buffer provided during construction. In that case you must also pass the
	* ODBC C-Type of the buffer and the query name of the corresponding column.
	*
	* There is an option to try to let the ODBC-driver to convert everything
	* to a (w)string, see AutoBindingMode.
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
	* SQL_NUMERIC				| SQL_NUMERIC_STRUCT*
	* SQL_DECIMAL				| SQL_NUMERIC_STRUCT*
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
		* \brief	Create a new ColumnBuffer that will allocate a corresponding buffer 
		*			using the data type information from the passed SColumnInfo.
		* \details	The constructor will try to allocate a corresponding buffer.
		*			Note: The constructor will examine SColumnInfo::m_isNullable and set the corresponding
		*			ColumnFlags, overriding an eventually set value in the passed flags.
		*			The ColumnBuffer will be set to SQL_NULL_DATA.
		* \param columnInfo	The Information about the column we bind.
		* \param mode		How Character columns should be bound.
		* \param odbcVersion ODBC Version to work with.
		* \param flags		Define if a column shall be included in write-operations, is part of primary-key, etc.
		*
		* \see	HaveBuffer()
		* \see	Bind()
		* \throw Exception If creating a corresponding buffer fails.
		*/
		ColumnBuffer(const SColumnInfo& columnInfo, AutoBindingMode mode, OdbcVersion odbcVersion, ColumnFlags flags = CF_SELECT);


		/*!
		* \brief	Create a new ColumnBuffer that will use the buffer given inside BufferPtrVariant.
		* \details	The constructor will not try to allocate a buffer on its own but update
		*			its internal variant with the value of the passed bufferVariant and the corresponding
		*			information.
		*			The ColumnBuffer will not take ownership of the passed bufferVariant and will
		*			not delete it.
		*			Note that using this constructor you can pass almost any combination of conversions 
		*			from SQL Types to ODBC C Types to the driver, as long as the buffer-type exists in the
		*			BufferPtrVariant. Bind() will fail if the driver does not support the given conversion.
		*			The SQL_NULL_DATA flag will not be set if you create a ColumnBuffer using this constructor
		*			as there already is a buffer available containing some data.
		* \param sqlCType			The ODBC C Type of the buffer (like SQL_C_WCHAR, SQL_C_SLONG, etc.). This value will be forwarded to the driver during binding for select.
		* \param bufferPtrVariant	Pointer to allocated buffer for the given sqlCType. Must be a buffer that can be held by a exodbc::BufferPtrVariant .
		* \param bufferSize			Size of the allocated buffer.
		* \param sqlType			The SQL Type of the buffer (like SQL_VARCHAR, SQL_INTEGER, etc.). This value will be forwarded to the driver during Binding parameters.
		*							If this ColumnBuffer is never bound as a parameter it is save to set this to SQL_UNKNOWN.
		* \param queryName			Name of the column that corresponds to this buffer.
		* \param	columnSize		The number of digits of a decimal value (including the fractional part).
		*							This is only used if the sqlCType is SQL_C_NUMERIC, to set SQL_DESC_PRECISION.
		* \param	decimalDigits	The number of digits of the fractional part of a decimal value.
		*							This is only used if the sqlCType is SQL_C_NUMERIC, to set SQL_DESC_SCALE.
		* \param	sqlType			The SQL Type of the column. This is only used if Parameter Markers are bound to this buffer, see BindParameter().
		* \param	flags		Define if a column shall be included in write-operations, is part of primary-key, etc.
		*
		* 
		* \see	HaveBuffer()
		* \see	Bind()
		*/
		ColumnBuffer(SQLSMALLINT sqlCType, BufferPtrVariant bufferPtrVariant, SQLLEN bufferSize, SQLSMALLINT sqlType, const std::wstring& queryName, ColumnFlags flags = CF_SELECT, SQLINTEGER columnSize = -1, SQLSMALLINT decimalDigits = -1);


		~ColumnBuffer();

	private:
		// We cannot be copied
		ColumnBuffer(const ColumnBuffer& other) { exASSERT(false);  };
		ColumnBuffer() {};

	public:
		/*!
		* \brief	Returns true if this ColumnBuffer has a buffer ready.
		* \details	This can be true either because during construction a buffer
		*			was allocated or because you've manually set a buffer.
		* \return	True if buffer is ready.
		*/
		bool HasBuffer() const throw() { return m_haveBuffer; };


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
		SQLSMALLINT GetBufferType() const throw() { return m_bufferType; };


		/*!
		* \brief	Tries to bind the buffer to the column using SQLBindCol
		*			for non-numeric types, or SQLSetDescField for numeric types.
		* \details	Fails if no buffer is allocated or if already bound.
		*			The driver might fail to bind the column to the type.
		*			On success, sets m_bound to true.
		* \param	hStmt ODBC Statement handle to bind this ColumnBuffer to.
		* \param	columnNr Column number that must match the column number of the
		*			statement to bind to. Must be >= 1 (1-indexed).
		* \throw	Exception If binding fails.
		*/
		void Bind(SQLHSTMT hStmt, SQLSMALLINT columnNr);


		/*!
		* \brief	Unbinds the buffer
		* \details	Tries to unbind the buffer to the column using SQLBindCol
		*			for non-numeric types, or SQLSetDescField for numeric types.
		*			On success, sets m_bound to false.
		*			Fails if not bound.
		* \param	hStmt ODBC Statement handle to unbind this ColumnBuffer from.
		* \throw	Exception If unbinding fails.
		*/
		void Unbind(SQLHSTMT hStmt);


		/*!
		* \brief	Tries to bind the buffer as parameter using SQLBindParameter.
		*			If the buffer type is a numeric-type, the needed attributes are set.
		* \details	Fails if no buffer is allocated or if already bound.
		*			The driver might fail to bind.
		* \param	hStmt ODBC Prepared Statement handle to bind this ColumnBuffer as parameter to.
		* \param	parameterNumber Parameter number corresponding to a parameter marker in the 
		*			prepared statement handle. Must be >= 1.
		* \throw	Exception If binding fails.
		*/
		void BindParameter(SQLHSTMT hStmt, SQLSMALLINT parameterNumber);


		/*!
		* \brief	Tries to unbind the buffer as parameter using SQLFreeStmt.
		*			Note that this will unbind ALL parameters bound to passed statement, see Ticket #59
		* \details	Fails if no buffer is allocated or if already bound.
		*			The driver might fail to bind.
		* \param	hStmt ODBC Prepared Statement handle to unbind this ColumnBuffer as parameter from.
		* \param	parameterNumber Parameter number corresponding to a parameter marker in the
		*			prepared statement handle. Must be >= 1.
		* \throw	Exception If unbinding fails.
		*/
		void UnbindParameter(SQLHSTMT hStmt, SQLSMALLINT parameterNumber);


		/*!
		* \brief	Returns true if this ColumnBuffer is bound.
		* \return	True if Column is bound.
		*/
		bool IsBound() const throw() { return m_hStmt != SQL_NULL_HSTMT; };


		/*!
		* \brief	Access the length indicator passed to SQLBindCol.
		* \details	Fails if no buffer is allocated or not bound.
		* \return	Length indicator bound to column.
		* \throw	Exception
		*/
		SQLLEN GetCb() const { exASSERT(HasBuffer()); exASSERT(IsBound()); return  m_cb; };


		/*!
		* \brief	Check if the current value is NULL.
		* \details	Fails if no buffer is allocated or not bound.
		* \return	True if current value is Null.
		* \throw	Exception
		*/
		bool IsNull() const { exASSERT(HasBuffer()); exASSERT(IsBound()); return m_cb == SQL_NULL_DATA; };


		/*!
		* \brief	Sets the current value to NULL.
		* \details	Fails if not NULLable.
		* \see IsNullable()
		* \throw Exception If not Nullable.
		*/
		void SetNull();


		/*!
		* \brief	Tests if ColumnFlags::CF_NULLABLE is set.
		* \see SetNull()
		* \return	True if current value can be set to Null.
		*/
		bool IsNullable() const throw() { return IsColumnFlagSet(CF_NULLABLE); };

		
		/*!
		* \brief	Check if the is SQL_NO_TOTAL.
		* \details	Fails if no buffer is allocated or not bound.
		* \return	True if current value is SQL_NO_TOTAL.
		* \throw	Exception
		*/
		bool NoTotal() const { exASSERT(HasBuffer()); exASSERT(IsBound()); return m_cb == SQL_NO_TOTAL; };


		/*!
		* \brief	Get the query name for this ColumnBuffer.
		* \return	The query name of the column matching this ColumnBuffer.
		*/
		const std::wstring& GetQueryName() const throw() { return m_queryName; };


		/*!
		* \brief	Set how Chars should be bound.
		* \details	Fails if already bound.
		* \param	mode Mode to set.
		* \see		AutoBindingMode
		* \throw	Exception If already bound.
		*/
		void SetAutoBindingMode(AutoBindingMode mode) { exASSERT(!IsBound()); m_autoBindingMode = mode; }


		/*!
		* \brief	Get the currently set AutoBindingMode.
		* \return	AutoBindingMode set.
		*/
		AutoBindingMode GetAutoBindingMode() const throw() { return m_autoBindingMode; };


		/*!
		* \brief	Set or clear the PrimaryKey flag.
		* \details	Must be called before any parameters are bound.
		* \param	isPrimaryKey True if this column is a primary key.
		* \throw	Exception If parameters are already bound.
		*/
		void SetPrimaryKey(bool isPrimaryKey = true);


		/*!
		* \brief	Read the PrimaryKey flag.
		*/
		bool IsPrimaryKey() const throw() { return (m_flags & CF_PRIMARY_KEY) == CF_PRIMARY_KEY; };


		/*!
		* \brief	Test if a ColumnFlags is set.
		*/
		bool IsColumnFlagSet(ColumnFlag columnFlag) const throw() { return (m_flags & columnFlag) == columnFlag; };


		/*!
		* \brief	Set a ColumnFlags.
		* \details	Flags must be set before the ColumnBuffer is Bound to the buffer!
		* \throw	Exception
		*/
		void SetColumnFlag(ColumnFlag columnFlag) { exASSERT(!IsBound());  m_flags |= columnFlag; };


		/*!
		* \brief	Clear a ColumnFlags.
		* \details	Flags must be cleared before the ColumnBuffer is Bound!
		* \throw	Exception
		*/
		void ClearColumnFlag(ColumnFlag columnFlag) { exASSERT(!IsBound());  m_flags &= ~columnFlag; };


		/*!
		* \brief	Set the value of a binary value. Copies the value into this ColumnBuffer.
		* \details	If this ColumnBuffer has the type SQL_C_BINARY the value of pBuff is
		*			copied into the Buffer of this ColumnBuffer. The Buffer is first filled with zeros,
		*			so that the buffer will be zero-padded if bufferSize is small than the size
		*			of the buffer allocated by this ColumnBuffer.
		* \param	pBuff Pointer to the value to be copied.
		* \param	bufferSize Size of the buffer pointed to by pBuff. Must be smaller or equal than
		*			the size of the buffer allocated by this ColumnBuffer.
		* \throw	Exception If not a binary buffer, or on any other error.
		*/
		void SetBinaryValue(const SQLCHAR* pBuff, SQLINTEGER bufferSize);


		/*!
		* \brief	Get the value as BufferVariant.
		* \details	Return a BufferVariant with the value of this ColumnBuffer.
		*			No conversion is done, depending on the type of the internal buffer allocated
		*			a corresponding value object is created in BufferVariant.
		* \throw	Exception If the internal buffer cannot be assigned to a value held by BufferVariant.
		*			This is the case mostly for binary data.
		*/
		BufferVariant GetValue() const;


		// Operators
		// ---------
		/*!
		* \brief	Copies the passed value into this ColumnBuffer. Does not work for all types, see Details!
		* \details	Copies the value of the passed BufferVariant into this Buffer.
		*			This does not work for BinaryData - we need to know the length of the data.
		*			The following types work:
		*			- SQLSMALLINT
		*			- SQLINTEGER
		*			- SQLBIGINT
		*			- SQLDOUBLE
		*			- SQL_DATE_STRUCT
		*			- SQL_TIME_STRUCT
		*			- SQL_TIMESTAMP_STRUCT
		*			- SQL_NUMERIC_STRUCT
		*			- SQL_SS_TIME2_STRUCT
		*
		* \param	var Variant containing the value that shall be assigned to this Buffer.
		* \see		SetBinaryValue()
		* \throw	Exception If type of buffer does not match the value passed.
		*/
		void operator=(const BufferVariant& var);


		/*!
		* \brief	Cast the current value to a SQLSMALLINT if possible.
		* \details	Fails if not bound.
		* \return	Current value as SQLSMALLINT.
		* \throw	CastException If value cannot be casted to an SQLSMALLINT or if data loss would occur.
		*/
		operator SQLSMALLINT() const;


		/*!
		* \brief	Cast the current value to a SQLINTEGER if possible.
		* \details	Fails if not bound.
		* \return	Current value as SQLINTEGER.
		* \throw	CastException If value cannot be casted to an SQLINTEGER or if data loss would occur.
		*/
		operator SQLINTEGER() const;


		/*!
		* \brief	Cast the current value to a SQLBIGINT if possible.
		* \details	Fails if not bound.
		* \return	Current value as SQLBIGINT.
		* \throw	CastException If value cannot be casted to an SQLBIGINT.
		*/
		operator SQLBIGINT() const;


		/*!
		* \brief	Cast the current value to a std::wstring if possible.
		* \details	Fails if not bound.
		* \return	Current value as std::wstring.
		* \throw	CastException If value cannot be casted to a std::wstring.
		* \see		WStringVisitor
		*/
		operator std::wstring() const;

		
		/*!
		* \brief	Cast the current value to a std::wstring if possible.
		* \details	Fails if not bound.
		* \return	Current value as std::wstring.
		* \throw	CastException If value cannot be casted to a std::string.
		* \see		StringVisitor
		*/
		operator std::string() const;


		/*!
		* \brief	Cast the current value to a SQLDOUBLE if possible.
		* \details	Fails if not bound.
		* \return	Current value as SQLDOUBLE.
		* \throw	CastException If value cannot be casted to a SQLDOUBLE.
		* \see		DoubleVisitor
		*/
		operator SQLDOUBLE() const;


		/*!
		* \brief	Cast the current value to a SQL_DATE_STRUCT if possible.
		* \details	Fails if not bound. If the value is a Timestamp, the time-part is ignored.
		* \return	Current value as SQL_DATE_STRUCT.
		* \throw	CastException If value cannot be casted to a SQL_DATE_STRUCT.
		* \see		TimestampVisitor
		*/
		operator SQL_DATE_STRUCT() const;


		/*!
		* \brief	Cast the current value to a SQL_TIME_STRUCT if possible.
		* \details	Fails if not bound. If the value is a Timestamp, the date-part is ignored.
		* \return	Current value as SQL_TIME_STRUCT.
		* \throw	CastException If value cannot be casted to a SQL_TIME_STRUCT.
		* \see		TimestampVisitor
		*/
		operator SQL_TIME_STRUCT() const;


#if HAVE_MSODBCSQL_H
		/*!
		* \brief	Cast the current value to a SQL_SS_TIME2_STRUCT if possible.
		* \details	Fails if not bound. If the value is a Timestamp, the date-part is ignored.
		*			Only available if HAVE_MSODBCSQL_H is defined to 1.
		* \return	Current value as SQL_SS_TIME2_STRUCT.
		* \throw	CastException If value cannot be casted to a SQL_SS_TIME2_STRUCT.
		* \see		TimestampVisitor
		*/
		operator SQL_SS_TIME2_STRUCT() const;
#endif


		/*!
		* \brief	Cast the current value to a SQL_TIMESTAMP_STRUCT if possible.
		* \details	Fails if not bound.
		* \return	Current value as SQL_TIMESTAMP_STRUCT.
		* \throw	CastException If value cannot be casted to a SQL_TIMESTAMP_STRUCT.
		* \see		TimestampVisitor
		*/
		operator SQL_TIMESTAMP_STRUCT() const;


		/*!
		* \brief	Cast the current value to a SQL_TIMESTAMP_STRUCT if possible.
		* \details	Fails if not bound.
		* \return	Current value as SQL_TIMESTAMP_STRUCT.
		* \throw	CastException If value cannot be casted to a SQL_TIMESTAMP_STRUCT.
		* \see		TimestampVisitor
		*/
		operator SQL_NUMERIC_STRUCT() const;


		/*!
		* \brief	Access the current buffer value as a const SQLCHAR*
		* \details	Returns the same pointer as it is stored in here. This is mainly used
		*			for accessing binary data, to avoid to copy the binary buffer.
		*			Do NOT delete the pointer returned by this operator, the ColumnBuffer will.
		* \return	Const SQLCHAR* to the buffer-content.
		* \throw	Exception
		* \see		TimestampVisitor
		*/
		operator const SQLCHAR*() const;


	private:
		/*!
		* \brief	Determine the buffer size for the buffer type given by m_bufferType 
		* \details This is used internally if no buffer-size and buffer is given and the buffer must
		*			thus be allocated automatically.
		*			Must be called after m_bufferType is set.
		*			The size of types with fixed lengths is given by sizeof() (like sizeof(SQLINTEGER).
		*			For char-types the size is calculated by '(fieldlength + 1) * sizeof(char-type)',
		*			where char-type is SQLCCHAR or SQLWCHAR. One extra space is allocated for the 
		*			terminating \0.
		*			In this case the passed columnInfo is needed to read the length of the column.
		* \return	Size of buffer for type given by m_bufferType.
		* \throw	NotSupportedException If m_bufferType has not supported value.
		*/
		SQLINTEGER DetermineBufferSize(const SColumnInfo& columnInfo) const;


		/*!
		* \brief	Tries to determine the size needed in chars if this ColumnBuffer is bound to a
		*			char-type plus the terminating 0 char. Uses the passed SColumnInfo.
		* \details Evaluates NUM_PREC_RADIX, COLUMN_SIZE and DECIMAL_DIGITS to determine the number
		*			of chars needed to store numeric-types.
		*			For date time types it uses ColumnSize.
		*			For char-sizes it is the column-size.
		*			For every type +1 is added for the terminating 0 char.
		* \see		http://msdn.microsoft.com/en-us/library/ms711683%28v=vs.85%29.aspx
		* \return	Needed buffer size according to SQL type from SColumnInfo.
		* \throw	NotSupportedException if the SQL Type from SColumnInfo is not supported.
		*/
		SQLINTEGER DetermineCharSize(const SColumnInfo& columnInfo) const;


		/*!
		* \brief	Determine the buffer type for the given SQL type .
		* \details This is used internally if no buffer-size and buffer is given and the buffer must
		*			thus be allocated automatically.
		*			See ColumnBuffer description for what SQL type is mapped to what buffer.
		* \return	Needed buffer size according to SQL type form SColumnInfo
		* \throw	NotSupportedException If SQL Type is not implemented.
		*/
		SQLSMALLINT DetermineBufferType(SQLSMALLINT sqlType) const;


		/*!
		* \brief	Get the allocated buffer as a void*.
		* \details Determines the type using the SColumnInfo and gets the pointer from the variant.
		*			Fails if no buffer is allocated.
		* \return	void* to the current buffer.
		* \throw	boost::bad_get If SQL-type does not match type in SQColumnInfo.
		* \throw	NotSupportedException if type of m_bufferType is unknown.
		*/
		void* GetBuffer() const;


		/*!
		* \brief	Allocate a buffer in the variant.
		* \details Allocates corresponding buffer. 
		*			Sets m_allocatedBuffer to true on success.
		* \param	bufferType An SQL_C_TYPE like SQL_C_SSHORT to describe the type to allocate
		* \param	bufferSize Used for types that are not as simple as a SQLSMALLINT, for example SQLWCHAR*
		* \throw	NotSupportedException If the bufferType is unknown.
		* \throw	bad_alloc if allocating memory fails.
		*/
		void AllocateBuffer(SQLSMALLINT bufferType, SQLINTEGER bufferSize);


		/*!
		* \brief	Return the number of decimal digits set during construction.
		* \return	Number of decimal digits if known or -1.
		*/
		SQLSMALLINT GetDecimalDigitals() const throw() {	return m_decimalDigits;	};


		struct BoundParameter
		{
			BoundParameter() : m_hStmt(SQL_NULL_HSTMT), m_parameterNumber(0) {};
			BoundParameter(SQLHSTMT hStmt, SQLUSMALLINT parameterNr) : m_hStmt(hStmt), m_parameterNumber(parameterNr) {};
			SQLHSTMT		m_hStmt;
			SQLUSMALLINT	m_parameterNumber;
		};
		typedef std::vector<BoundParameter*> BoundParameterPtrsVector;

		// Helpers to quickly access the pointers inside the variant.
		// All of these could throw a boost::bad_get
		SQLSMALLINT*			GetSmallIntPtr() const;
		SQLINTEGER*				GetIntPtr() const;
		SQLBIGINT*				GetBigIntPtr() const;
		SQLCHAR*				GetCharPtr() const;
		SQLWCHAR*				GetWCharPtr() const;
		SQLDOUBLE*				GetDoublePtr() const;
		SQL_DATE_STRUCT*		GetDatePtr() const;
		SQL_TIME_STRUCT*		GetTimePtr() const;
		SQL_TIMESTAMP_STRUCT*	GetTimestampPtr() const;
#if HAVE_MSODBCSQL_H
		SQL_SS_TIME2_STRUCT*	GetTime2Ptr() const;
#endif
		SQL_NUMERIC_STRUCT*		GetNumericPtr() const;

		ColumnFlags				m_flags;				///< Flags, set during construction.
		SQLINTEGER				m_columnSize;			///< Column Size, either read from SColumnInfo during construction or set manually. -1 indicates unknown.
		SQLSMALLINT				m_decimalDigits;		///< Decimal digits, either read from SColumnInfo during construction or set manually. -1 indicates unkonwn.
		std::wstring			m_queryName;			///< Name to use to query this Column. Either passed during construction, or read from m_columnInfo during construction.
		SQLUSMALLINT			m_columnNr;				///< Column number used during Bind(). Set to 0 during construction.
		SQLSMALLINT				m_sqlType;				///< The SQL Type of the Column, like SQL_SMALLINT. Either set on construction or read from SColumnInfo::m_sqlType.
		bool					m_haveBuffer;			///< True if a buffer is available, either because it was allocated or passed during construction.
		bool					m_allocatedBuffer;		///< True if Buffer has been allocated and must be deleted on destruction. Set from AllocateBuffer()
		SQLSMALLINT				m_bufferType;			///< ODBC C Type of the buffer allocated, as it was passed to the driver. like SQL_C_WCHAR, etc. Set from ctor or during AllocateBuffer()
		SQLINTEGER				m_bufferSize;			///< Size of an allocated or set from constructor buffer.
		SQLHSTMT				m_hStmt;				///< Set to the statement handle this ColumnBuffer was bound to, initialized to SQL_NULL_HSTMT
		BoundParameterPtrsVector	m_boundParameters;	

		OdbcVersion m_odbcVersion;	///< OdbcVersion passed when creating this ColumnBuffer.
		AutoBindingMode m_autoBindingMode;	///< Determine if chars shall be bound as wchars, etc. Cannot be changed after bound.

		BufferPtrVariant m_bufferPtr;	///< Variant that holds the pointer to the actual buffer

		SQLLEN		m_cb;	///< The length indicator set during Bind for this column

	};  // class ColumnBuffer

	typedef std::map<SQLUSMALLINT, ColumnBuffer*> ColumnBufferPtrMap;

	

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
	* \todo SQL_NUMERIC_STRUCT* if it fits into the bigint
	*/
	class EXODBCAPI BigintVisitor
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
		SQLBIGINT operator()(SQL_NUMERIC_STRUCT* pNumeric) const { throw CastException(SQL_C_NUMERIC, SQL_C_SBIGINT); };
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
	* - SQLSMALLINT* (Note: This will just format using %d)
	* - SQLINTEGER* (Note: This will just format using %ld)
	* - SQLBIGINT* (Note: This will just format using %lld)
	* - SQLCHAR*
	* - SQLDOUBLE* (Note: This will just format using boost::lexical_cast<std::wstring>)
	* - SQLDATE* Returns a date in the format YYYY-MM-DD
	* - SQLTIME_STRUCT* Returns a time in the format hh:mm:ss
	* - SQL_TIMESTAMP_STRUCT* Returns a timestamp in the format YYYY-MM-DDThh:mm:ss.fffffffff
	* \todo NUMERIC_STRUCT*
	*
	*/
	class EXODBCAPI WStringVisitor
		: public boost::static_visitor < std::wstring >
	{
	public:
		std::wstring operator()(SQLSMALLINT* smallInt) const { return (boost::wformat(L"%d") % *smallInt).str(); };
		std::wstring operator()(SQLINTEGER* i) const { return (boost::wformat(L"%ld") % *i).str(); };
		std::wstring operator()(SQLBIGINT* bigInt) const { return (boost::wformat(L"%lld") % *bigInt).str(); };
		std::wstring operator()(SQLCHAR* pChar) const{ throw CastException(SQL_C_CHAR, SQL_C_WCHAR); };
		std::wstring operator()(SQLWCHAR* pWChar) const { return pWChar; };
		std::wstring operator()(SQLDOUBLE* pDouble) const { return boost::lexical_cast<std::wstring>(*pDouble); };
		std::wstring operator()(SQL_DATE_STRUCT* pDate) const { return boost::str(boost::wformat(L"%04d-%02d-%02d") % pDate->year % pDate->month %pDate->day); };
		std::wstring operator()(SQL_TIME_STRUCT* pTime) const { return boost::str(boost::wformat(L"%02d:%02d:%02d") % pTime->hour % pTime->minute %pTime->second); };
		std::wstring operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { return boost::str(boost::wformat(L"%04d-%02d-%02dT%02d:%02d:%012.9f") % pTimestamp->year %pTimestamp->month %pTimestamp->day %pTimestamp->hour %pTimestamp->minute % ((SQLDOUBLE)pTimestamp->second + ((SQLDOUBLE)pTimestamp->fraction / (SQLDOUBLE)1000000000.0))); };
		std::wstring operator()(SQL_NUMERIC_STRUCT* pNumeric) const { throw CastException(SQL_C_NUMERIC, SQL_C_WCHAR); };
#if HAVE_MSODBCSQL_H
		std::wstring operator()(SQL_SS_TIME2_STRUCT* pTime2) const { return boost::str(boost::wformat(L"%02d:%02d:%012.9f") % pTime2->hour % pTime2->minute % ((SQLDOUBLE)pTime2->second + ((SQLDOUBLE)pTime2->fraction / (SQLDOUBLE)1000000000.0))); };
#endif
	};	// class WStringVisitor


	/*!
	* \class StringVisitor
	*
	* \brief Visitor to cast current value to a std::string .
	*
	* This Visitor can cast the following sources to a std::string :
	*
	* - SQLSMALLINT* (Note: This will just format using %d)
	* - SQLINTEGER* (Note: This will just format using %ld)
	* - SQLBIGINT* (Note: This will just format using %lld)
	* - SQLCHAR*
	* - SQLDOUBLE* (Note: This will just format using boost::lexical_cast<std::wstring>)
	* - SQLDATE* Returns a date in the format YYYY-MM-DD
	* - SQLTIME_STRUCT* Returns a time in the format hh:mm:ss
	* - SQL_TIMESTAMP_STRUCT* Returns a timestamp in the format YYYY-MM-DDThh:mm:ss.fffffffff
	* - SQL_SS_TIME2_STRUCT* Returns a time in the format hh:mm:ss.fffffffff Only available if HAVE_MSODBCSQL_His defined.
	* \todo NUMERIC_STRUCT*
	*/
	class EXODBCAPI StringVisitor
		: public boost::static_visitor < std::string >
	{
	public:
		std::string operator()(SQLSMALLINT* smallInt) const { return (boost::format("%d") % *smallInt).str(); };
		std::string operator()(SQLINTEGER* i) const { return (boost::format("%ld") % *i).str(); };
		std::string operator()(SQLBIGINT* bigInt) const { return (boost::format("%lld") % *bigInt).str(); };
		std::string operator()(SQLCHAR* pChar) const { return (char*)pChar; };
		std::string operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_CHAR); };
		std::string operator()(SQLDOUBLE* pDouble) const { return boost::lexical_cast<std::string>(*pDouble); };
		std::string operator()(SQL_DATE_STRUCT* pDate) const { return boost::str(boost::format("%04d-%02d-%02d") % pDate->year % pDate->month %pDate->day); };
		std::string operator()(SQL_TIME_STRUCT* pTime) const { return boost::str(boost::format("%02d:%02d:%02d") % pTime->hour % pTime->minute %pTime->second); };
		std::string operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { return boost::str(boost::format("%04d-%02d-%02dT%02d:%02d:%012.9f") % pTimestamp->year %pTimestamp->month %pTimestamp->day %pTimestamp->hour %pTimestamp->minute % ((SQLDOUBLE)pTimestamp->second + ((SQLDOUBLE)pTimestamp->fraction / (SQLDOUBLE)1000000000.0))); };
		std::string operator()(SQL_NUMERIC_STRUCT* pNumeric) const { throw CastException(SQL_C_NUMERIC, SQL_C_CHAR); };
#if HAVE_MSODBCSQL_H
		std::string operator()(SQL_SS_TIME2_STRUCT* pTime2) const { return boost::str(boost::format("%02d:%02d:%012.9f") % pTime2->hour % pTime2->minute % ((SQLDOUBLE)pTime2->second + ((SQLDOUBLE)pTime2->fraction / (SQLDOUBLE)1000000000.0))); };
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
	* \todo: SQL_NUMERIC_STRUCT*
	*/
	class EXODBCAPI DoubleVisitor
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
		SQLDOUBLE operator()(SQL_NUMERIC_STRUCT* pNumeric) const { throw CastException(SQL_C_NUMERIC, SQL_C_DOUBLE); };
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
	* - SQL_SS_TIME2_STRUCT* (date is set to 00.00.0000). Only available if HAVE_MSODBCSQL_H is defined.
	*
	*/
	class EXODBCAPI TimestampVisitor
		: public boost::static_visitor < SQL_TIMESTAMP_STRUCT >
	{
	public:

		SQL_TIMESTAMP_STRUCT operator()(SQLSMALLINT* smallInt) const { throw CastException(SQL_C_SSHORT, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLINTEGER* i) const { throw CastException(SQL_C_SSHORT, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLBIGINT* bigInt) const { throw CastException(SQL_C_SBIGINT, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLCHAR* pChar) const { throw CastException(SQL_C_CHAR, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQLDOUBLE* pDouble) const { throw CastException(SQL_C_DOUBLE, SQL_C_TYPE_TIMESTAMP); };
		SQL_TIMESTAMP_STRUCT operator()(SQL_DATE_STRUCT* pDate) const;
		SQL_TIMESTAMP_STRUCT operator()(SQL_TIME_STRUCT* pTime) const;
		SQL_TIMESTAMP_STRUCT operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const;
		SQL_TIMESTAMP_STRUCT operator()(SQL_NUMERIC_STRUCT* pNumeric) const { throw CastException(SQL_C_NUMERIC, SQL_C_TYPE_TIMESTAMP); };
#if HAVE_MSODBCSQL_H
		SQL_TIMESTAMP_STRUCT operator()(SQL_SS_TIME2_STRUCT* pTime) const;
#endif

	private:
	};	// class TimestampVisitor


	/*!
	* \class NumericVisitor
	*
	* \brief Visitor to cast current value to a SQL_NUMERIC_STRUCT.
	*
	* This Visitor can cast the following sources to a SQL_NUMERIC_STRUCT :
	*
	* - SQL_NUMERIC_STRUCT*
	* \todo SMALLINT, INTEGER, BIGINT
	*/
	class EXODBCAPI NumericVisitor
		: public boost::static_visitor < SQL_NUMERIC_STRUCT >
	{
	public:

		SQL_NUMERIC_STRUCT operator()(SQLSMALLINT* smallInt) const { throw CastException(SQL_C_SSHORT, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQLINTEGER* i) const { throw CastException(SQL_C_SSHORT, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQLBIGINT* bigInt) const { throw CastException(SQL_C_SBIGINT, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQLCHAR* pChar) const { throw CastException(SQL_C_CHAR, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQLWCHAR* pWChar) const { throw CastException(SQL_C_WCHAR, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQLDOUBLE* pDouble) const { throw CastException(SQL_C_DOUBLE, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQL_DATE_STRUCT* pDate) const { throw CastException(SQL_TYPE_DATE, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQL_TIME_STRUCT* pTime) const { throw CastException(SQL_TYPE_TIME, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQL_TIMESTAMP_STRUCT* pTimestamp) const { throw CastException(SQL_TYPE_TIMESTAMP, SQL_C_NUMERIC); };
		SQL_NUMERIC_STRUCT operator()(SQL_NUMERIC_STRUCT* pNumeric) const { return *pNumeric; };
#if HAVE_MSODBCSQL_H
		SQL_NUMERIC_STRUCT operator()(SQL_SS_TIME2_STRUCT* pTime) const { throw CastException(SQL_C_SS_TIME2, SQL_C_NUMERIC); };
#endif

	private:
	};	// class NumericVisitor


	/*!
	* \class CharPtrVisitor
	*
	* \brief Visitor to access the buffer.
	*
	* This Visitor extracts the buffer-pointer and returns it as const SQLCHAR*,
	* which works for every datatype.
	*
	*/
	class EXODBCAPI CharPtrVisitor
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
		const SQLCHAR* operator()(SQL_NUMERIC_STRUCT* pNumeric) const { return (const SQLCHAR*)(pNumeric); };
#if HAVE_MSODBCSQL_H
		const SQLCHAR* operator()(SQL_SS_TIME2_STRUCT* pNumeric) const { return (const SQLCHAR*)(pNumeric); };
#endif
	};
}


#endif // ColumnBuffer_H
