/*!
* \file ColumnBuffer.h
* \author Elias Gerber <eg@elisium.ch>
* \date 23.11.2014
* \brief Header file for the ColumnBuffer class and its helpers.
* \copyright wxWindows Library Licence, Version 3.1
*
*/

#pragma once
#ifndef COLUMNBUFFER_H
#define COLUMNBUFFER_H

// Same component headers
#include "exOdbc.h"
#include "Helpers.h"
#include "InfoObject.h"
#include "Sql2BufferTypeMap.h"

// Other headers
#include "boost/variant.hpp"

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
	*  - SQLREAL*
	*  - SQL_DATE_STRUCT*
	*  - SQL_TIME_STRUCT*
	*  - SQL_TIME2_STRUCT* / SQL_TIME_STRUCT*
	*  - SQL_TIMESTAMP_STRUCT*
	*  - SQL_NUMERIC_STRUCT*
	* 
	*/
	typedef boost::variant<SQLSMALLINT*, SQLINTEGER*, SQLBIGINT*, 
		SQLCHAR*, SQLWCHAR*, 
		SQLDOUBLE*, SQLREAL*,
		SQL_DATE_STRUCT*, SQL_TIME_STRUCT*, SQL_TIMESTAMP_STRUCT*,
		SQL_NUMERIC_STRUCT*
#if HAVE_MSODBCSQL_H
		,SQL_SS_TIME2_STRUCT*
#endif
	> BufferPtrVariant;


	/*!
	* \enum NullValue
	* \brief A pseudo value to store in the BufferVariant to indicate a NULL value.
	*/
	enum class NullValue
	{
		IS_NULL = 1	///< Indicates that the value is NULL.
	};


	/*!
	* \typedef BufferVariant
	* \brief A helper for setting / getting values. If you like it, work with a variant
	* \details The following types can be stored:
	*  - NullValue A boost::variant will always hold a valid, as this is the first entry in the
	*              variant, every BufferVariant will default to a NULL value.
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
	typedef boost::variant<NullValue, SQLSMALLINT, SQLINTEGER, SQLBIGINT,
		std::string, std::wstring,
		SQLDOUBLE, SQLREAL,
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
	* If a ColumnBuffer is constructed by passing a ColumnInfo and a 
	* Sql2BufferTypeMap, the ColumnBuffer will query the passed Sql2BufferTypeMap
	* about the SQL C Type of the buffer to create. If the Sql2BufferTypeMap
	* returns a SQL C Type, a corresponding Buffer to transfer data is created.
	* This buffer will be managed by the ColumnBuffer.
	* 
	* If a ColumnBuffer is called by passing a ManualColumnInfo, an allocated
	* buffer must be passed. The passed buffer will be used to transfer data.
	* Do not free the buffer before the ColumnBuffer is destroyed.
	*
	*/
	class EXODBCAPI ColumnBuffer
	{
	public:
		/*!
		* \brief	Create a new ColumnBuffer that will allocate a corresponding buffer 
		*			using the data type information from the passed ColumnInfo and the passed Sql2BufferTypeMap.
		* \details	The constructor will try to allocate a corresponding buffer.
		*			Note: The constructor will examine ColumnInfo::m_isNullable and set the corresponding
		*			ColumnFlags, overriding an eventually set value in the passed flags.
		*			The ColumnBuffer will be set to SQL_NULL_DATA (even if the Column is not Nullable), so
		*			set or fetch some valid data into the ColumnBuffer soon.
		* \param columnInfo	The Information about the column we bind.
		* \param odbcVersion ODBC Version to work with.
		* \param pSql2BufferTypeMap The Sql2BufferTypeMap to be used to determine the BufferType (SQL C Type) during Construction.
		* \param flags		Define if a column shall be included in write-operations, is part of primary-key, etc.
		*
		* \see	HaveBuffer()
		* \see	Bind()
		* \throw Exception If creating a corresponding buffer fails.
		*/
		ColumnBuffer(const ColumnInfo& columnInfo, OdbcVersion odbcVersion, ConstSql2BufferTypeMapPtr pSql2BufferTypeMap, ColumnFlags flags = CF_SELECT);


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
		* \param columnInfo			The ManualColumnInfo for this Column.
		* \param sqlCType			The ODBC C Type of the buffer (like SQL_C_WCHAR, SQL_C_SLONG, etc.). This value will be forwarded to the driver during binding for select.
		* \param bufferPtrVariant	Pointer to allocated buffer for the given sqlCType. Must be a buffer that can be held by a exodbc::BufferPtrVariant .
		* \param bufferSize			Size of the allocated buffer.
		* \param odbcVersion		ODBC-Version supported.
		* \param	flags			Define if a column shall be included in write-operations, is part of primary-key, etc.
		* 
		* \see	HaveBuffer()
		* \see	Bind()
		*/
		ColumnBuffer(const ManualColumnInfo& columnInfo, SQLSMALLINT sqlCType, BufferPtrVariant bufferPtrVariant, SQLLEN bufferSize, OdbcVersion odbcVersion, ColumnFlags flags = CF_SELECT);


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
		SQLLEN GetBufferSize() const { exASSERT(HasBuffer());  return m_bufferSize; };


		/*!
		* \brief	Returns type of this buffer.
		* \return	Buffer-type.
		*/
		SQLSMALLINT GetBufferType() const throw() { return m_bufferType; };


		/*!
		* \brief	Returns SQL type of this ColumnBuffer.
		* \return	SQL type.
		*/
		SQLSMALLINT GetSqlType() const throw() { return m_sqlType; };


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
		* \brief	Access the length indicator passed to SQLBindCol / SQLBindParameter
		* \details	Fails if no buffer is allocated or not bound.
		* \return	Length indicator bound to column.
		* \throw	Exception
		*/
		SQLLEN GetCb() const { exASSERT(HasBuffer()); exASSERT(IsBound()); return  m_cb; };


		/*!
		* \brief	Set the value of the length indicator passed to SQLBindCol / SQLBIndParameter
		* \details	This is needed in case you directly work with a buffer that for example
		*			contains string-data, and you want to set the length indicator to SLQ_NTS,
		*			or the length of the data to be inserted, etc.
		*/
		void SetCb(SQLLEN cb) throw() {	m_cb = cb; };


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
		* \throw	Exception
		*/
		std::wstring GetQueryName() const;

		
		/*!
		* \brief	Get the Object Name of this ColumnBuffer.
		* \return	ObjectName created during construction, pointer is valid until this
		*			ColumnBuffer is destroyed.
		* \throw	Exception
		*/
		const ObjectName* GetName() const { exASSERT(m_pName); return m_pName; };


		/*!
		* \brief	Get the query name for this ColumnBuffer.
		* \return	The query name of the column matching this ColumnBuffer, or ??? in case of failure.
		*/
		std::wstring GetQueryNameNoThrow() const throw();


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
		bool IsPrimaryKey() const throw() { return IsColumnFlagSet(CF_PRIMARY_KEY); };


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
		*			If the column has the ColumnFlag::CF_NULLABLE set, and the value is NULL,
		*			NullValue::IS_NULL is returned.
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
		*			- NullValue::IS_NULL
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
		* \details	If the column is NULL, the string 'NULL' (without ') is
		*			returned, else a WStringVisitor is applied.
		*			Fails if not bound.
		* \return	Current value as std::wstring.
		* \throw	CastException If value cannot be casted to a std::wstring.
		* \see		WStringVisitor
		*/
		operator std::wstring() const;

		
		/*!
		* \brief	Cast the current value to a std::wstring if possible.
		* \details	If the column is NULL, the string 'NULL' (without ') is
		*			returned, else a StringVisitor is applied.
		*			Fails if not bound.
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
		* \brief	Cast the current value to a SQLREAL if possible.
		* \details	Fails if not bound.
		* \return	Current value as SQLREAL.
		* \throw	CastException If value cannot be casted to a SQLREAL.
		* \see		RealVisitor
		*/
		operator SQLREAL() const;


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
		SQLINTEGER DetermineBufferSize(const ColumnInfo& columnInfo) const;


		/*!
		* \brief	Tries to determine the size needed in chars if this ColumnBuffer is bound to a
		*			char-type plus the terminating 0 char. Uses the passed ColumnInfo.
		* \details Evaluates NUM_PREC_RADIX, COLUMN_SIZE and DECIMAL_DIGITS to determine the number
		*			of chars needed to store numeric-types.
		*			For date time types it uses ColumnSize.
		*			For char-sizes it is the column-size.
		*			For every type +1 is added for the terminating 0 char.
		* \see		http://msdn.microsoft.com/en-us/library/ms711683%28v=vs.85%29.aspx
		* \return	Needed buffer size according to SQL type from ColumnInfo.
		* \throw	NotSupportedException if the SQL Type from ColumnInfo is not supported.
		*/
		SQLINTEGER DetermineCharSize(const ColumnInfo& columnInfo) const;


		/*!
		* \brief	Get the allocated buffer as a void*.
		* \details Determines the type using the ColumnInfo and gets the pointer from the variant.
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
		* \param	bufferSize Used for types that are not as simple as a SQLSMALLINT, for example binary buffer. For SQLCHAR[] and SQLWCHAR it is 
		*			the number of characters.
		* \throw	NotSupportedException If the bufferType is unknown.
		* \throw	bad_alloc if allocating memory fails.
		*/
		void AllocateBuffer(SQLSMALLINT bufferType, SQLLEN bufferSize);


		/*!
		* \brief	Frees the Buffer in the variant.
		* \details	Set m_allocatedBuffer to false on success.
		* \throw	NotSupportedException If the bufferType is unknown.
		* \throw	WrapperException if boost::get fails
		*/
		void FreeBuffer();


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
		SQLREAL*				GetRealPtr() const;
		SQL_DATE_STRUCT*		GetDatePtr() const;
		SQL_TIME_STRUCT*		GetTimePtr() const;
		SQL_TIMESTAMP_STRUCT*	GetTimestampPtr() const;
#if HAVE_MSODBCSQL_H
		SQL_SS_TIME2_STRUCT*	GetTime2Ptr() const;
#endif
		SQL_NUMERIC_STRUCT*		GetNumericPtr() const;

		ColumnFlags				m_flags;				///< Flags, set during construction.
		SQLINTEGER				m_columnSize;			///< Column Size, either read from ColumnInfo during construction or set manually. -1 indicates unknown.
		SQLSMALLINT				m_decimalDigits;		///< Decimal digits, either read from ColumnInfo during construction or set manually. -1 indicates unkonwn.
		ObjectName*				m_pName;				///< The name of this object. Created during construction, freed on deletion.
		SQLUSMALLINT			m_columnNr;				///< Column number used during Bind(). Set to 0 during construction.
		SQLSMALLINT				m_sqlType;				///< The SQL Type of the Column, like SQL_SMALLINT. Either set on construction or read from ColumnInfo::m_sqlType.
		bool					m_haveBuffer;			///< True if a buffer is available, either because it was allocated or passed during construction.
		bool					m_allocatedBuffer;		///< True if Buffer has been allocated and must be deleted on destruction. Set from AllocateBuffer()
		SQLSMALLINT				m_bufferType;			///< ODBC C Type of the buffer allocated, as it was passed to the driver. like SQL_C_WCHAR, etc. Set from Ctor.
		SQLLEN					m_bufferSize;			///< Size of an allocated or set from constructor buffer.
		SQLHSTMT				m_hStmt;				///< Set to the statement handle this ColumnBuffer was bound to, initialized to SQL_NULL_HSTMT
		BoundParameterPtrsVector	m_boundParameters;	

		OdbcVersion m_odbcVersion;	///< OdbcVersion passed when creating this ColumnBuffer.

		BufferPtrVariant m_bufferPtr;	///< Variant that holds the pointer to the actual buffer

		SQLLEN		m_cb;	///< The length indicator set during Bind for this column

	};  // class ColumnBuffer

	typedef std::map<SQLUSMALLINT, ColumnBuffer*> ColumnBufferPtrMap;
}


#endif // COLUMNBUFFER_H
