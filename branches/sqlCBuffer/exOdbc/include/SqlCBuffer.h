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
#include "boost/variant.hpp"

// System headers

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

		SqlCBufferLengthIndicator& operator=(const SqlCBufferLengthIndicator& other)
		{
			m_pCb = other.m_pCb;
			return *this;
		}

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

		SqlCBuffer& operator=(const SqlCBuffer& other)
		{
			SqlCBufferLengthIndicator::operator=(other);
			m_pBuffer = other.m_pBuffer;
			return *this;
		}

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

		void BindSelect() const {};

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

		SqlCArrayBuffer& operator=(const SqlCArrayBuffer& other)
		{
			SqlCBufferLengthIndicator::operator=(other);
			m_nrOfElements = other.m_nrOfElements;
			m_pBuffer = other.m_pBuffer;
			return *this;
		}

		virtual ~SqlCArrayBuffer() 
		{};

		static SQLSMALLINT GetSqlCType() throw() { return sqlCType; };
		SQLLEN GetBufferLength() const throw() { return sizeof(T) * GetNrOfElements(); };

		// \todo we should rather use std::copy here.
		void SetValue(const T* value, SQLLEN valueBufferLength, SQLLEN cb) throw() 
		{ 
			exASSERT(valueBufferLength <= GetBufferLength()); 
			memset(m_pBuffer.get(), 0, GetBufferLength()); 
			memcpy(m_pBuffer.get(), value, valueBufferLength);
			SetCb(cb);
		};
		const std::shared_ptr<T> GetBuffer() const throw() { return m_pBuffer; };
		SQLLEN GetNrOfElements() const throw() { return m_nrOfElements; };

		void BindSelect() const {};

	private:
		SQLLEN m_nrOfElements;
		std::shared_ptr<T> m_pBuffer;
	};
	
	typedef SqlCArrayBuffer<SQLWCHAR, SQL_C_WCHAR> SqlWCharArray;
	typedef SqlCArrayBuffer<SQLCHAR, SQL_C_CHAR> SqlCharArray;
	typedef SqlCArrayBuffer<SQLCHAR, SQL_C_BINARY> SqlBinaryArray;

	typedef boost::variant<SqlUShortBuffer, SqlShortBuffer, SqlULongBuffer, SqlLongBuffer, SqlUBigIntBuffer, SqlBigIntBuffer,
		SqlTimeTypeStructBuffer, SqlTimeStructBuffer,
		SqlWCharArray, SqlCharArray, SqlBinaryArray> SqlCBufferVariant;
	
	typedef std::map<SQLUSMALLINT, SqlCBufferVariant> SqlCBufferVariantMap;

	class BindSelectVisitor
		: public boost::static_visitor<void>
	{
	public:

		template<typename T>
		void operator()(const T& t) const
		{
			t.BindSelect();
		}
	};

	extern EXODBCAPI SqlCBufferVariant CreateBuffer(SQLSMALLINT sqlCType);

} // namespace exodbc
