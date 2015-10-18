/*!
* \file SqlCBuffer.h
* \author Elias Gerber <eg@elisium.ch>
* \date 01.10.2015
* \brief Header file for the CBufferType interface.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "Exception.h"

// Other headers
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
	//template<typename T, typename std::enable_if<!std::is_pointer<T>::value, T>::type arg = 0>
	//class Test
	//{

	//};

	//Test<int> g_test1;
	//Test<int*> g_test2;

	//class SqlCBufferAccessible
	//{
	//public:
	//	virtual SQLSMALLINT GetSqlCType() const throw() = 0;
	//	virtual const SQLPOINTER GetCBuffer() const throw() = 0;
	//	virtual SQLLEN GetBufferSize() const throw() = 0;
	//};


	//class SqlCBufferBindable
	//{
	//public:
	//	virtual void BindForSelect(SQLHSTMT hStmt, SQLSMALLINT columnNr, SQLINTEGER columnSize, SQLSMALLINT decimalDigits) const throw() = 0;
	//	virtual void BindAsParam(SQLHSTMT hStmt, SQLSMALLINT parameterNumber, SQLSMALLINT databaseSqlType, SQLSMALLINT decimalDigits) const throw() = 0;
	//};

	class SqlCBufferLengthIndicator
	{
	public:
		SqlCBufferLengthIndicator()
		{
			m_pCb = std::make_shared<SQLLEN>(0);
		}

		SqlCBufferLengthIndicator(const SqlCBufferLengthIndicator& other)
			: m_pCb(other.m_pCb)
		{};

		virtual ~SqlCBufferLengthIndicator()
		{};

		void SetCb(SQLLEN cb) throw() { *m_pCb = cb; };
		SQLLEN GetCb() const throw() { return *m_pCb; };

		void SetNull() throw() { *m_pCb = SQL_NULL_DATA; };
		bool IsNull() const throw() { return *m_pCb == SQL_NULL_DATA; };

	protected:
		std::shared_ptr<SQLLEN> m_pCb;
	};


	template<typename T, SQLSMALLINT sqlCType , typename std::enable_if<!std::is_pointer<T>::value, T>::type* = 0>
	class SqlCBuffer
		: public SqlCBufferLengthIndicator
	{
	public:
		SqlCBuffer()
			: SqlCBufferLengthIndicator()
		{
			m_pBuffer = std::make_shared<T>();
			SetNull();
		};

		SqlCBuffer(const SqlCBuffer& other)
			: SqlCBufferLengthIndicator(other)
			, m_pBuffer(other.m_pBuffer)
		{};

		virtual ~SqlCBuffer() 
		{};

		static SQLSMALLINT GetSqlCType() throw() { return sqlCType; };
		static SQLLEN GetBufferLength() throw() { return sizeof(T); };

		void SetValue(const T& value) throw() { SetValue(value, GetBufferLength()); };
		void SetValue(const T& value, SQLLEN cb) throw() { *m_pBuffer = value; SetCb(cb); };
		const T& GetValue() const throw() { return *m_pBuffer; };
		std::shared_ptr<const T> GetBuffer() const throw() { return m_pBuffer; };

	private:
		std::shared_ptr<T> m_pBuffer;
	};

	typedef SqlCBuffer<SQLSMALLINT, SQL_C_USHORT> SqlUShortBuffer;
	typedef SqlCBuffer<SQLINTEGER, SQL_C_ULONG> SqlULongBuffer;
	typedef SqlCBuffer<SQLBIGINT, SQL_C_UBIGINT> SqlUBigIntBuffer;
	typedef SqlCBuffer<SQLSMALLINT, SQL_C_SSHORT> SqlShortBuffer;
	typedef SqlCBuffer<SQLINTEGER, SQL_C_SLONG> SqlLongBuffer;
	typedef SqlCBuffer<SQLBIGINT, SQL_C_SBIGINT> SqlBigIntBuffer;
	typedef SqlCBuffer<SQL_TIME_STRUCT, SQL_C_TYPE_TIME> SqlTimeTypeStructBuffer;
	typedef SqlCBuffer<SQL_TIME_STRUCT, SQL_C_TIME> SqlTimeStructBuffer;



	template<typename T, SQLSMALLINT sqlCType, typename std::enable_if<!std::is_pointer<T>::value, T>::type* = 0>
	class SqlCArrayBuffer
		: public SqlCBufferLengthIndicator
	{
	public:
		SqlCArrayBuffer() 
			: SqlCBufferLengthIndicator()
			, m_nrOfElements(0)
		{ 
			static_assert(false, "Default Constructor is not supported, must use SqlCArrayBuffer(SQLLEN nrOfElements)");
		};

		SqlCArrayBuffer(SQLLEN nrOfElements)
			: SqlCBufferLengthIndicator()
			, m_nrOfElements(nrOfElements)
		{
			m_pBuffer = std::shared_ptr<T>(new T[m_nrOfElements], std::default_delete<T[]>());
			SetNull();
		};

		SqlCArrayBuffer(const SqlCArrayBuffer& other)
			: SqlCBufferLengthIndicator(other)
			, m_pBuffer(other.m_pBuffer)
			, m_nrOfElements(other.m_nrOfElements)
		{};

		~SqlCArrayBuffer() 
		{};

		void SetValue(const T* value, SQLLEN valueBufferLength, SQLLEN cb) throw() 
		{ 
			exASSERT(valueBufferLength <= GetBufferLength()); 
			memset(m_pBuffer.get(), 0, GetBufferLength()); 
			memcpy(m_pBuffer.get(), value, valueBufferLength);
			SetCb(cb);
		};
		const std::shared_ptr<T> GetBuffer() const throw() { return m_pBuffer; };
		SQLLEN GetNrOfElements() const throw() { return m_nrOfElements; };
		SQLLEN GetBufferLength() const throw() { return GetNrOfElements() * sizeof(T); };

		//virtual SQLSMALLINT GetSqlCType() const throw() { return sqlCType; };
		//virtual const SQLPOINTER GetCBuffer() const throw() { return (SQLPOINTER*)GetBuffer(); };
		//virtual SQLLEN GetBufferSize() const throw() { return sizeof(T) * GetNrOfElements(); };

	private:
		const SQLLEN m_nrOfElements;
		std::shared_ptr<T> m_pBuffer;
	};
	typedef SqlCArrayBuffer<SQLWCHAR, SQL_C_WCHAR> SqlWCharArray;
	typedef SqlCArrayBuffer<SQLCHAR, SQL_C_CHAR> SqlCharArray;
	typedef SqlCArrayBuffer<SQLCHAR, SQL_C_BINARY> SqlBinaryArray;


	/*
	TODO next: The CBufferType class or so must now handle the concrete buffers.
	and it must provide methods for Binding params and select buffers.

	and somehow access to the values.
	*/

	/*!
	* \class ExtendedTypes
	*
	* \brief Defines an interface to be implemented by extended types, so
	*		they can be used by the ColumnBuffer.
	*
	*/
	class EXODBCAPI CBufferType
	{
	public:
		virtual ~CBufferType() {};

		virtual CBufferType* CreateInstance(SQLSMALLINT sqlCType, SQLLEN nrOfElements) const = 0;


		/*!
		* \brief	Get the SQL C Types that are supported by this SqlCBufferType.
		*/
		virtual std::set<SQLSMALLINT> GetSupportedSqlCTypes() const = 0;


		/*!
		* \brief	Test if passed SQL C Type is supported by this SqlCBufferType.
		* \details	If this function returns true for the passed SQL C Type, this
		*			SQL C Type must also be included in the set retrieved by
		*			GetSupportedSqlCTypes().
		* \param sqlCType	The SQL C Type.
		* \return	True if passed SQL C Type is supported.
		*/
		virtual bool IsSqlCTypeSupported(SQLSMALLINT sqlCType) const = 0;


		///*!
		//* \brief	Get the SQL C Type for a SQL Type supported by this CBufferType.
		//* \param sqlType	The SQL Type.
		//* \return	The SQL C Type the passed SQL Type is mapped to.
		//* \throw	NotSupportedException If the passed SQL Type is not supported by
		//*			this CBufferType.
		//*/
		//virtual SQLSMALLINT GetSqlCType(SQLSMALLINT sqlType) const = 0;


		///*!
		//* \brief	Allocate a buffer for the column with passed column size.
		//* \param	columnSize	The column size value for the column.
		//* \param	bufferSize This in-out param is populated with the size (in bytes)
		//*			of the buffer allocated.
		//* \return	Pointer to the buffer allocated.
		//*/
		//virtual SQLPOINTER AllocateBuffer(SQLINTEGER columnSize, SQLLEN& bufferSize) const = 0;


		///*!
		//* \brief	Free the passed buffer (which must have been created by this CBufferType).
		//* \param	pBuffer Pointer to the buffer to be freed.
		//* \throw	Exception If passed pBuffer was not allocated by this CBufferType.
		//*/
		//virtual void FreeBuffer(SQLPOINTER pBuffer) const = 0;


		//virtual SQLSMALLINT GetSmallInt() const = 0;
		//virtual SQLINTEGER GetInt() const = 0;
		//virtual SQLBIGINT GetBigInt() { throw NotSupportedException(NotSupportedException::Type::SQL_C_TYPE, sqlCBufferType); };
		//virtual std::wstring GetAsWString(SQLPOINTER pBuffer, SQLSMALLINT sqlCBufferType) { throw NotSupportedException(NotSupportedException::Type::SQL_C_TYPE, sqlCBufferType); };
		//virtual std::string GetAsString(SQLPOINTER pBuffer, SQLSMALLINT sqlCBufferType) { throw NotSupportedException(NotSupportedException::Type::SQL_C_TYPE, sqlCBufferType); };
		//virtual SQLDOUBLE GetAsDouble(SQLPOINTER pBuffer, SQLSMALLINT sqlCBufferType) { throw NotSupportedException(NotSupportedException::Type::SQL_C_TYPE, sqlCBufferType); };
		//virtual SQLREAL GetAsReal(SQLPOINTER pBuffer, SQLSMALLINT sqlCBufferType) { throw NotSupportedException(NotSupportedException::Type::SQL_C_TYPE, sqlCBufferType); };
		//virtual SQL_TIMESTAMP_STRUCT GetAsTimestamp(SQLPOINTER pBuffer, SQLSMALLINT sqlCBufferType) { throw NotSupportedException(NotSupportedException::Type::SQL_C_TYPE, sqlCBufferType); };
		//virtual SQL_DATE_STRUCT GetAsDate(SQLPOINTER pBuffer, SQLSMALLINT sqlCBufferType) { throw NotSupportedException(NotSupportedException::Type::SQL_C_TYPE, sqlCBufferType); };
		//virtual SQL_TIME_STRUCT GetAsTime(SQLPOINTER pBuffer, SQLSMALLINT sqlCBufferType) { throw NotSupportedException(NotSupportedException::Type::SQL_C_TYPE, sqlCBufferType); };
		//virtual SQL_NUMERIC_STRUCT GetAsNumeric(SQLPOINTER pBuffer, SQLSMALLINT sqlCBufferType) { throw NotSupportedException(NotSupportedException::Type::SQL_C_TYPE, sqlCBufferType); };


		//virtual void SetSmallInt(SQLSMALLINT value);
	};

	typedef std::shared_ptr<CBufferType> CBufferTypePtr;
	typedef std::shared_ptr<const CBufferType> CBufferTypeConstPtr;

} // namespace exodbc
