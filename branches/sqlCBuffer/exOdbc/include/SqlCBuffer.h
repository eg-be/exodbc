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
#include "InfoObject.h"

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

		SqlCBufferLengthIndicator(const SqlCBufferLengthIndicator& other) = default;
		SqlCBufferLengthIndicator& operator=(const SqlCBufferLengthIndicator& other) = default;

		virtual ~SqlCBufferLengthIndicator()
		{};

		void SetCb(SQLLEN cb) noexcept { *m_pCb = cb; };
		SQLLEN GetCb() const noexcept { return *m_pCb; };

		void SetNull() noexcept { *m_pCb = SQL_NULL_DATA; };
		bool IsNull() const noexcept { return *m_pCb == SQL_NULL_DATA; };

	protected:
		std::shared_ptr<SQLLEN> m_pCb;
	};

	struct EXODBCAPI ColumnBoundHandle
	{
		ColumnBoundHandle(SQLHSTMT hStmt, SQLSMALLINT columnNr)
			: m_hStmt(hStmt)
			, m_columnNr(columnNr)
		{};

		ColumnBoundHandle(const ColumnBoundHandle& other) = default;

		~ColumnBoundHandle()
		{};

		bool operator==(const ColumnBoundHandle& other) const noexcept
		{
			return other.m_hStmt == m_hStmt && other.m_columnNr == m_columnNr;
		};

		bool operator!=(const ColumnBoundHandle& other) const noexcept
		{
			return !(*this == other);
		};

		bool operator<(const ColumnBoundHandle& other) const noexcept
		{
			return m_hStmt < other.m_hStmt && m_columnNr < other.m_columnNr;
		}

		const SQLHSTMT m_hStmt;
		const SQLSMALLINT m_columnNr;
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

		SqlCBuffer& operator=(const SqlCBuffer& other) = default;
		SqlCBuffer(const SqlCBuffer& other) = default;

		~SqlCBuffer()
		{
			std::set<ColumnBoundHandle>::iterator it = m_boundSelects.begin();
			while (it != m_boundSelects.end())
			{
				// unbind
				ColumnBoundHandle bindInfo = *it;
				try
				{
					UnbindSelect(bindInfo.m_columnNr, bindInfo.m_hStmt);
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(boost::str(boost::wformat(L"Failed to unbind column %d from handle %d: %s") % bindInfo.m_columnNr %bindInfo.m_hStmt %ex.ToString()));
				}
				it = m_boundSelects.erase(it);
			}
		};

		static SQLSMALLINT GetSqlCType() noexcept { return sqlCType; };
		static SQLLEN GetBufferLength() noexcept { return sizeof(T); };

		void SetValue(const T& value) noexcept { SetValue(value, GetBufferLength()); };
		void SetValue(const T& value, SQLLEN cb) noexcept { *m_pBuffer = value; SetCb(cb); };
		const T& GetValue() const noexcept { return *m_pBuffer; };
		std::shared_ptr<const T> GetBuffer() const noexcept { return m_pBuffer; };

		void BindSelect(const ColumnBindInfo& boundHandleInfo, SQLUSMALLINT columnNr, SQLHSTMT hStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(hStmt != SQL_NULL_HSTMT);
			ColumnBoundHandle boundHandleInfo(hStmt, columnNr);
			exASSERT_MSG(m_boundSelects.find(boundHandleInfo) == m_boundSelects.end(), L"Already bound to passed hStmt and column for Select on this buffer");

			SQLRETURN ret = SQLBindCol(hStmt, columnNr, sqlCType, (SQLPOINTER*)m_pBuffer.get(), GetBufferLength(), m_pCb.get());
			THROW_IFN_SUCCESS(SQLBindCol, ret, SQL_HANDLE_STMT, hStmt);
			m_boundSelects.insert(boundHandleInfo);
		};

		void UnbindSelect(SQLUSMALLINT columnNr, SQLHSTMT hStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(hStmt != SQL_NULL_HSTMT);
			ColumnBoundHandle boundHandleInfo(hStmt, columnNr);
			std::set<ColumnBoundHandle>::iterator it = m_boundSelects.find(boundHandleInfo);
			exASSERT_MSG(it != m_boundSelects.end(), L"Not bound to passed hStmt and column for Select on this buffer");
			
			SQLRETURN ret = SQLBindCol(hStmt, columnNr, sqlCType, NULL, 0, NULL);
			THROW_IFN_SUCCEEDED(SQLBindCol, ret, SQL_HANDLE_STMT, hStmt);
			m_boundSelects.erase(it);
		}

		private:
		std::shared_ptr<T> m_pBuffer;

		std::set<ColumnBoundHandle> m_boundSelects;
	};

	template<>
	void SqlCBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>::BindSelect(const ColumnBindInfo& bindInfo, SQLUSMALLINT columnNr, SQLHSTMT hStmt)
	{
		exASSERT(columnNr >= 1);
		exASSERT(hStmt != SQL_NULL_HSTMT);
		exASSERT(bindInfo.GetColumnSize() > 0);
		exASSERT(bindInfo.GetDecimalDigits() >= 0);
		ColumnBoundHandle boundHandleInfo(hStmt, columnNr);
		exASSERT_MSG(m_boundSelects.find(boundHandleInfo) == m_boundSelects.end(), L"Already bound to passed hStmt and column for Select on this buffer");

		SQLHDESC hDesc = GetRowDescriptorHandle(hStmt, RowDescriptorType::ROW);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_TYPE, (SQLPOINTER) SQL_C_NUMERIC);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN) bindInfo.GetColumnSize()));
		SetDescriptionField(hDesc, columnNr, SQL_DESC_SCALE, (SQLPOINTER) bindInfo.GetDecimalDigits());
		SetDescriptionField(hDesc, columnNr, SQL_DESC_DATA_PTR, (SQLPOINTER) m_pBuffer.get());
		SetDescriptionField(hDesc, columnNr, SQL_DESC_INDICATOR_PTR, (SQLPOINTER) m_pCb.get());
		SetDescriptionField(hDesc, columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLPOINTER) m_pCb.get());
		m_boundSelects.insert(boundHandleInfo);
	}

	template<>
	void SqlCBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>::UnbindSelect(SQLUSMALLINT columnNr, SQLHSTMT hStmt)
	{
		exASSERT(columnNr >= 1);
		exASSERT(hStmt != SQL_NULL_HSTMT);
		ColumnBoundHandle boundHandleInfo(hStmt, columnNr);
		std::set<ColumnBoundHandle>::iterator it = m_boundSelects.find(boundHandleInfo);
		exASSERT_MSG(it != m_boundSelects.end(), L"Not bound to passed hStmt and column for Select on this buffer");

		SQLHDESC hDesc = GetRowDescriptorHandle(hStmt, RowDescriptorType::ROW);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_TYPE, (SQLPOINTER) SQL_C_NUMERIC);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_DATA_PTR, (SQLINTEGER)NULL);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_INDICATOR_PTR, (SQLINTEGER)NULL);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLINTEGER)NULL);
		m_boundSelects.erase(it);
	}

	typedef SqlCBuffer<SQLSMALLINT, SQL_C_USHORT> SqlUShortBuffer;
	typedef SqlCBuffer<SQLINTEGER, SQL_C_ULONG> SqlULongBuffer;
	typedef SqlCBuffer<SQLBIGINT, SQL_C_UBIGINT> SqlUBigIntBuffer;
	typedef SqlCBuffer<SQLSMALLINT, SQL_C_SSHORT> SqlShortBuffer;
	typedef SqlCBuffer<SQLINTEGER, SQL_C_SLONG> SqlLongBuffer;
	typedef SqlCBuffer<SQLBIGINT, SQL_C_SBIGINT> SqlBigIntBuffer;
	typedef SqlCBuffer<SQL_TIME_STRUCT, SQL_C_TYPE_TIME> SqlTimeTypeStructBuffer;
	typedef SqlCBuffer<SQL_TIME_STRUCT, SQL_C_TIME> SqlTimeStructBuffer;

	typedef SqlCBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC> SqlNumericStructBuffer;


	template<typename T, SQLSMALLINT sqlCType, typename std::enable_if<!std::is_pointer<T>::value, T>::type* = 0>
	class SqlCArrayBuffer
		: public SqlCBufferLengthIndicator
	{
	public:
		SqlCArrayBuffer() = delete;

		SqlCArrayBuffer(SQLLEN nrOfElements)
			: SqlCBufferLengthIndicator()
			, m_nrOfElements(nrOfElements)
		{
			m_pBuffer = std::shared_ptr<T>(new T[m_nrOfElements], std::default_delete<T[]>());
			SetNull();
		};

		SqlCArrayBuffer(const SqlCArrayBuffer& other) = default;
		SqlCArrayBuffer& operator=(const SqlCArrayBuffer& other) = default;

		virtual ~SqlCArrayBuffer() 
		{};

		static SQLSMALLINT GetSqlCType() noexcept { return sqlCType; };
		SQLLEN GetBufferLength() const noexcept { return sizeof(T) * GetNrOfElements(); };

		// \todo we should rather use std::copy here.
		void SetValue(const T* value, SQLLEN valueBufferLength, SQLLEN cb)
		{ 
			exASSERT(valueBufferLength <= GetBufferLength()); 
			memset(m_pBuffer.get(), 0, GetBufferLength()); 
			memcpy(m_pBuffer.get(), value, valueBufferLength);
			SetCb(cb);
		};
		const std::shared_ptr<T> GetBuffer() const noexcept { return m_pBuffer; };
		SQLLEN GetNrOfElements() const noexcept { return m_nrOfElements; };

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
//			t.BindSelect();
		}
	};

	extern EXODBCAPI SqlCBufferVariant CreateBuffer(SQLSMALLINT sqlCType);

} // namespace exodbc
