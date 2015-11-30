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
#include "EnumFlags.h"
#include "SqlHandle.h"

// Other headers
#include "boost/variant.hpp"
#include "boost/variant/polymorphic_get.hpp"

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
	struct ColumnBoundHandle
	{
		ColumnBoundHandle(ConstSqlStmtHandlePtr pHStmt, SQLSMALLINT columnNr)
			: m_pHStmt(pHStmt)
			, m_columnNr(columnNr)
		{
			exASSERT(m_pHStmt != NULL);
			exASSERT(m_pHStmt->IsAllocated());
		};

		ColumnBoundHandle() = delete;
		ColumnBoundHandle(const ColumnBoundHandle& other) = default;
		ColumnBoundHandle& operator=(const ColumnBoundHandle& other) = default;
		~ColumnBoundHandle()
		{};

		bool operator==(const ColumnBoundHandle& other) const noexcept
		{
			return *m_pHStmt == *(other.m_pHStmt) && other.m_columnNr == m_columnNr;
		};

		bool operator!=(const ColumnBoundHandle& other) const noexcept
		{
			return !(*this == other);
		};

		bool operator<(const ColumnBoundHandle& other) const noexcept
		{
			return *m_pHStmt < *(other.m_pHStmt) && m_columnNr < other.m_columnNr;
		}

		ConstSqlStmtHandlePtr m_pHStmt;
		const SQLSMALLINT m_columnNr;
	};


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


	class ExtendedColumnPropertiesHolder
	{
	public:
		ExtendedColumnPropertiesHolder()
			: m_columnSize(0)
			, m_decimalDigits(0)
			, m_pObjectName(NULL)
			, m_sqlType(SQL_UNKNOWN_TYPE)
		{};

		virtual ~ExtendedColumnPropertiesHolder()
		{};

		void SetColumnSize(SQLINTEGER columnSize) noexcept { m_columnSize = columnSize; };
		void SetDecimalDigits(SQLSMALLINT decimalDigits) noexcept { m_decimalDigits = decimalDigits; };
		void SetObjectName(std::shared_ptr<ObjectName> pObjectName) { exASSERT(pObjectName != NULL); m_pObjectName = pObjectName; };
		void SetSqlType(SQLSMALLINT sqlType) noexcept { m_sqlType = sqlType; };

		SQLINTEGER GetColumnSize() const noexcept { return m_columnSize; };
		SQLSMALLINT GetDecimalDigits() const noexcept { return m_decimalDigits; };
		std::shared_ptr<ObjectName> GetObjectName() const noexcept { return m_pObjectName; };
		SQLSMALLINT GetSqlType() const noexcept { return m_sqlType; };

	protected:
		SQLINTEGER m_columnSize;
		SQLSMALLINT m_decimalDigits;
		SQLSMALLINT m_sqlType;
		std::shared_ptr<ObjectName> m_pObjectName;
	};

	template<typename T, SQLSMALLINT sqlCType , typename std::enable_if<!std::is_pointer<T>::value, T>::type* = 0>
	class SqlCBuffer
		: public SqlCBufferLengthIndicator
		, public ColumnFlags
		, public ExtendedColumnPropertiesHolder
	{
	public:
		SqlCBuffer()
			: SqlCBufferLengthIndicator()
			, ColumnFlags()
			, ExtendedColumnPropertiesHolder()
		{
			m_pBuffer = std::make_shared<T>();
			SetNull();
		};

		SqlCBuffer& operator=(const SqlCBuffer& other) = default;
		SqlCBuffer(const SqlCBuffer& other) = default;

		virtual ~SqlCBuffer()
		{
			// unbind selects, one by one, do not stop if one fails but try the next one
			// (thats why this is not done using Unbind() )
			std::set<ColumnBoundHandle>::iterator it = m_boundSelects.begin();
			while (it != m_boundSelects.end())
			{
				ColumnBoundHandle bindInfo = *it;
				try
				{
					if (bindInfo.m_pHStmt && bindInfo.m_pHStmt->IsAllocated())
					{
						it = UnbindSelect(bindInfo.m_columnNr, bindInfo.m_pHStmt);
					}
					else
					{
						LOG_ERROR(L"Unexpected ColumnBoundHandle bindInfo: Either m_pHStmt not set, or m_pHStmt not allocated.");
					}
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(boost::str(boost::wformat(L"Failed to unbind column %d from stmt-handle %d: %s") % bindInfo.m_columnNr %bindInfo.m_pHStmt->GetHandle() %ex.ToString()));
					++it;
				}
			}
		};

		static SQLSMALLINT GetSqlCType() noexcept { return sqlCType; };
		static SQLLEN GetBufferLength() noexcept { return sizeof(T); };

		void SetValue(const T& value) noexcept { SetValue(value, GetBufferLength()); };
		void SetValue(const T& value, SQLLEN cb) noexcept { *m_pBuffer = value; SetCb(cb); };
		const T& GetValue() const { exASSERT(!IsNull()); return *m_pBuffer; };
		std::shared_ptr<const T> GetBuffer() const noexcept { return m_pBuffer; };

		operator T() const noexcept { return GetValue(); };

		void BindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			ColumnBoundHandle boundHandleInfo(pHStmt, columnNr);
			exASSERT_MSG(m_boundSelects.find(boundHandleInfo) == m_boundSelects.end(), L"Already bound to passed hStmt and column for Select on this buffer");

			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, (SQLPOINTER*)m_pBuffer.get(), GetBufferLength(), m_pCb.get());
			THROW_IFN_SUCCESS(SQLBindCol, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
			m_boundSelects.insert(boundHandleInfo);
		};


		/*!
		* \brief	Unbinds from all handles bound.
		* \throw	Exception
		*/
		void Unbind()
		{
			// unbind selects
			std::set<ColumnBoundHandle>::iterator it = m_boundSelects.begin();
			while (it != m_boundSelects.end())
			{
				ColumnBoundHandle bindInfo = *it;
				exASSERT(bindInfo.m_pHStmt && bindInfo.m_pHStmt->IsAllocated());
				it = UnbindSelect(bindInfo.m_columnNr, bindInfo.m_pHStmt);
			}
		}


	protected:
		std::set<ColumnBoundHandle>::iterator UnbindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			ColumnBoundHandle boundHandleInfo(pHStmt, columnNr);
			std::set<ColumnBoundHandle>::iterator it = m_boundSelects.find(boundHandleInfo);
			exASSERT_MSG(it != m_boundSelects.end(), L"Not bound to passed hStmt and column for Select on this buffer");

			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, NULL, 0, NULL);
			THROW_IFN_SUCCEEDED(SQLBindCol, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
			return m_boundSelects.erase(it);
		}

	private:
		std::shared_ptr<T> m_pBuffer;
		std::set<ColumnBoundHandle> m_boundSelects;
	};

	template<>
	void SqlCBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>::BindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
	{
		exASSERT(columnNr >= 1);
		exASSERT(pHStmt != NULL);
		exASSERT(pHStmt->IsAllocated());
		exASSERT(m_columnSize > 0);
		exASSERT(m_decimalDigits >= 0);
		ColumnBoundHandle boundHandleInfo(pHStmt, columnNr);
		exASSERT_MSG(m_boundSelects.find(boundHandleInfo) == m_boundSelects.end(), L"Already bound to passed hStmt and column for Select on this buffer");

		SQLHDESC hDesc = GetRowDescriptorHandle(pHStmt->GetHandle(), RowDescriptorType::ROW);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_TYPE, (SQLPOINTER) SQL_C_NUMERIC);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN) m_columnSize));
		SetDescriptionField(hDesc, columnNr, SQL_DESC_SCALE, (SQLPOINTER) m_decimalDigits);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_DATA_PTR, (SQLPOINTER) m_pBuffer.get());
		SetDescriptionField(hDesc, columnNr, SQL_DESC_INDICATOR_PTR, (SQLPOINTER) m_pCb.get());
		SetDescriptionField(hDesc, columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLPOINTER) m_pCb.get());
		m_boundSelects.insert(boundHandleInfo);
	}

	template<>
	std::set<ColumnBoundHandle>::iterator SqlCBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>::UnbindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
	{
		exASSERT(columnNr >= 1);
		exASSERT(pHStmt != NULL);
		exASSERT(pHStmt->IsAllocated());
		ColumnBoundHandle boundHandleInfo(pHStmt, columnNr);
		std::set<ColumnBoundHandle>::iterator it = m_boundSelects.find(boundHandleInfo);
		exASSERT_MSG(it != m_boundSelects.end(), L"Not bound to passed hStmt and column for Select on this buffer");

		SQLHDESC hDesc = GetRowDescriptorHandle(pHStmt->GetHandle(), RowDescriptorType::ROW);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_TYPE, (SQLPOINTER) SQL_C_NUMERIC);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_DATA_PTR, (SQLINTEGER)NULL);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_INDICATOR_PTR, (SQLINTEGER)NULL);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLINTEGER)NULL);
		return m_boundSelects.erase(it);
	}

	// Integer types
	typedef SqlCBuffer<SQLSMALLINT, SQL_C_USHORT> SqlUShortBuffer;
	typedef SqlCBuffer<SQLINTEGER, SQL_C_ULONG> SqlULongBuffer;
	typedef SqlCBuffer<SQLBIGINT, SQL_C_UBIGINT> SqlUBigIntBuffer;
	typedef SqlCBuffer<SQLSMALLINT, SQL_C_SSHORT> SqlSShortBuffer;
	typedef SqlCBuffer<SQLINTEGER, SQL_C_SLONG> SqlSLongBuffer;
	typedef SqlCBuffer<SQLBIGINT, SQL_C_SBIGINT> SqlSBigIntBuffer;
	
	// datetime types
	typedef SqlCBuffer<SQL_TIME_STRUCT, SQL_C_TYPE_TIME> SqlTypeTimeStructBuffer;
	typedef SqlCBuffer<SQL_TIME_STRUCT, SQL_C_TIME> SqlTimeStructBuffer;
	typedef SqlCBuffer<SQL_DATE_STRUCT, SQL_C_TYPE_DATE> SqlTypeDateStructBuffer;
	typedef SqlCBuffer<SQL_DATE_STRUCT, SQL_C_DATE> SqlDateStructBuffer;
	typedef SqlCBuffer<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP> SqlTypeTimestampStructBuffer;
	typedef SqlCBuffer<SQL_TIMESTAMP_STRUCT, SQL_C_TIMESTAMP> SqlTimestamptStructBuffer;

	// Numeric types
	typedef SqlCBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC> SqlNumericStructBuffer;

	// Floating types
	typedef SqlCBuffer<SQLDOUBLE, SQL_C_DOUBLE> SqlDoubleBuffer;
	typedef SqlCBuffer<SQLREAL, SQL_C_FLOAT> SqlRealBuffer;

	template<typename T, SQLSMALLINT sqlCType, typename std::enable_if<!std::is_pointer<T>::value, T>::type* = 0>
	class SqlCArrayBuffer
		: public SqlCBufferLengthIndicator
		, public ColumnFlags
		, public ExtendedColumnPropertiesHolder
	{
	public:
		SqlCArrayBuffer() = delete;

		SqlCArrayBuffer(SQLLEN nrOfElements)
			: SqlCBufferLengthIndicator()
			, ColumnFlags()
			, ExtendedColumnPropertiesHolder()
			, m_nrOfElements(nrOfElements)
			//, m_vect(nrOfElements)
			, m_pBuffer(std::make_shared<std::vector<T>>(nrOfElements))
		{
			//m_vect = std::vector<T>(m_nrOfElements);

			//m_pBuffer = std::make_shared<std::vector<T>>(m_nrOfElements);
			//m_pBuffer = std::shared_ptr<T>(new T[m_nrOfElements], std::default_delete<T[]>());
			// [](double* arr) { delete[] arr; }
			//m_pBuffer = new T[m_nrOfElements];
			SetNull();
		};

		SqlCArrayBuffer(const SqlCArrayBuffer& other) = default;
		SqlCArrayBuffer& operator=(const SqlCArrayBuffer& other) = default;

		virtual ~SqlCArrayBuffer() 
		{
			std::set<ColumnBoundHandle>::iterator it = m_boundSelects.begin();
			while (it != m_boundSelects.end())
			{
				// unbind
				ColumnBoundHandle bindInfo = *it;
				try
				{
					if (bindInfo.m_pHStmt && bindInfo.m_pHStmt->IsAllocated())
					{
						it = UnbindSelect(bindInfo.m_columnNr, bindInfo.m_pHStmt);
					}
					else
					{
						LOG_ERROR(L"Unexpected ColumnBoundHandle bindInfo: Either m_pHStmt not set, or m_pHStmt not allocated.");
					}
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(boost::str(boost::wformat(L"Failed to unbind column %d from stmt-handle %d: %s") % bindInfo.m_columnNr %bindInfo.m_pHStmt->GetHandle() %ex.ToString()));
					++it;
				}
			}
		};

		static SQLSMALLINT GetSqlCType() noexcept { return sqlCType; };
		SQLLEN GetBufferLength() const noexcept { return sizeof(T) * GetNrOfElements(); };

		void SetValue(const std::vector<T>& value, SQLLEN cb)
		{ 
			exASSERT(value.size() <= m_pBuffer->capacity()); 
			size_t index = 0;
			for (std::vector<T>::const_iterator it = value.begin(); it != value.end(); ++it)
			{
				(*m_pBuffer)[index] = *it;
				++index;
			}
			// null-terminate if last element added not already was a '0'
			// and if there is still some space for the last '0'
			// if there is no space, fail
			if (cb == SQL_NTS && value.back() != 0)
			{
				exASSERT(index < m_pBuffer->capacity());
				(*m_pBuffer)[index] = 0;
			}
			//memset(m_pBuffer.get(), 0, GetBufferLength()); 
			//memcpy(m_pBuffer.get(), value, valueBufferLength);
			SetCb(cb);
		};
		const std::shared_ptr<std::vector<T>> GetBuffer() const noexcept { return m_pBuffer; };
		//const std::shared_ptr<T> GetBuffer() const noexcept { return m_pBuffer; };
		SQLLEN GetNrOfElements() const noexcept { return m_nrOfElements; };

		//operator T*() const noexcept { return m_pBuffer.get(); };

		void BindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			ColumnBoundHandle boundHandleInfo(pHStmt, columnNr);
			exASSERT_MSG(m_boundSelects.find(boundHandleInfo) == m_boundSelects.end(), L"Already bound to passed hStmt and column for Select on this buffer");

			//SQLLEN l = GetBufferLength();
			//SQLPOINTER* pVec = (SQLPOINTER*) &(m_pBuffer.get()[0]);
			//SQLSMALLINT targetType = sqlCType;
			//SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, (SQLPOINTER*) &m_vect[0], l, m_pCb.get());
			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, (SQLPOINTER*) &(*m_pBuffer)[0], GetBufferLength(), m_pCb.get());
//			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, (SQLPOINTER*)(m_pBuffer.get()), 4, m_pCb.get());
//			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, (SQLPOINTER*)(m_pBuffer.get()), GetBufferLength(), m_pCb.get());
//			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, (SQLPOINTER*)m_pBuffer, GetBufferLength(), m_pCb.get());
			THROW_IFN_SUCCESS(SQLBindCol, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
			m_boundSelects.insert(boundHandleInfo);
		};

		/*!
		* \brief	Unbinds from all handles bound.
		* \throw	Exception
		*/
		void Unbind()
		{
			// unbind selects
			std::set<ColumnBoundHandle>::iterator it = m_boundSelects.begin();
			while (it != m_boundSelects.end())
			{
				ColumnBoundHandle bindInfo = *it;
				exASSERT(bindInfo.m_pHStmt && bindInfo.m_pHStmt->IsAllocated());
				it = UnbindSelect(bindInfo.m_columnNr, bindInfo.m_pHStmt);
			}
		}

	protected:
		std::set<ColumnBoundHandle>::iterator UnbindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			ColumnBoundHandle boundHandleInfo(pHStmt, columnNr);
			std::set<ColumnBoundHandle>::iterator it = m_boundSelects.find(boundHandleInfo);
			exASSERT_MSG(it != m_boundSelects.end(), L"Not bound to passed hStmt and column for Select on this buffer");

			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, NULL, 0, NULL);
			THROW_IFN_SUCCEEDED(SQLBindCol, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
			return m_boundSelects.erase(it);
		}

	private:
		SQLLEN m_nrOfElements;
		std::shared_ptr<std::vector<T>> m_pBuffer;
		//std::vector<T> m_vect;
		//std::shared_ptr<std::vector<T>> m_pBuffer;
		//std::shared_ptr<T> m_pBuffer;
		//T* m_pBuffer;
		std::set<ColumnBoundHandle> m_boundSelects;
	};

	class SqlCPointerBuffer
		: public SqlCBufferLengthIndicator
		, public ColumnFlags
		, public ExtendedColumnPropertiesHolder
	{
	public:
		SqlCPointerBuffer() = delete;
		SqlCPointerBuffer(const std::wstring& queryName, SQLSMALLINT sqlType, SQLPOINTER pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags, SQLINTEGER columnSize, SQLSMALLINT decimalDigits)
			: m_pBuffer(pBuffer)
			, m_sqlCType(sqlCType)
			, m_bufferLength(bufferSize)
		{
			ManualColumnInfo colInfo(sqlType, queryName);
			SetObjectName(std::make_shared <ManualColumnInfo> (colInfo));
			Set(flags);
			SetDecimalDigits(decimalDigits);
			SetColumnSize(columnSize);
			SetSqlType(sqlType);
		};

		SqlCPointerBuffer& operator=(const SqlCPointerBuffer& other) = default;
		SqlCPointerBuffer(const SqlCPointerBuffer& other) = default;

		virtual ~SqlCPointerBuffer()
		{
			// unbind selects, one by one, do not stop if one fails but try the next one
			// (thats why this is not done using Unbind() )
			std::set<ColumnBoundHandle>::iterator it = m_boundSelects.begin();
			while (it != m_boundSelects.end())
			{
				ColumnBoundHandle bindInfo = *it;
				try
				{
					if (bindInfo.m_pHStmt && bindInfo.m_pHStmt->IsAllocated())
					{
						it = UnbindSelect(bindInfo.m_columnNr, bindInfo.m_pHStmt);
					}
					else
					{
						LOG_ERROR(L"Unexpected ColumnBoundHandle bindInfo: Either m_pHStmt not set, or m_pHStmt not allocated.");
					}
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(boost::str(boost::wformat(L"Failed to unbind column %d from stmt-handle %d: %s") % bindInfo.m_columnNr %bindInfo.m_pHStmt->GetHandle() % ex.ToString()));
					++it;
				}
			}
		};

		SQLSMALLINT GetSqlCType() const noexcept { return m_sqlCType; };
		SQLLEN GetBufferLength() const noexcept { return m_bufferLength; };

		void BindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			exASSERT(m_columnSize >= 0);
			exASSERT(m_decimalDigits >= 0);
			ColumnBoundHandle boundHandleInfo(pHStmt, columnNr);
			exASSERT_MSG(m_boundSelects.find(boundHandleInfo) == m_boundSelects.end(), L"Already bound to passed hStmt and column for Select on this buffer");

			if (m_columnSize > 0 || m_decimalDigits > 0)
			{
				SqlDescHandle hDesc(pHStmt, RowDescriptorType::ROW);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN)m_columnSize));
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_SCALE, (SQLPOINTER)m_decimalDigits);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_DATA_PTR, (SQLPOINTER)m_pBuffer);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_INDICATOR_PTR, (SQLPOINTER)m_pCb.get());
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLPOINTER)m_pCb.get());
			}
			else
			{
				SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, m_sqlCType, (SQLPOINTER*)m_pBuffer, GetBufferLength(), m_pCb.get());
				THROW_IFN_SUCCESS(SQLBindCol, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
			}
			m_boundSelects.insert(boundHandleInfo);
		}


		/*!
		* \brief	Unbinds from all handles bound.
		* \throw	Exception
		*/
		void Unbind()
		{
			// unbind selects
			std::set<ColumnBoundHandle>::iterator it = m_boundSelects.begin();
			while (it != m_boundSelects.end())
			{
				ColumnBoundHandle bindInfo = *it;
				exASSERT(bindInfo.m_pHStmt && bindInfo.m_pHStmt->IsAllocated());
				it = UnbindSelect(bindInfo.m_columnNr, bindInfo.m_pHStmt);
			}
		}

	protected:
		std::set<ColumnBoundHandle>::iterator UnbindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			ColumnBoundHandle boundHandleInfo(pHStmt, columnNr);
			std::set<ColumnBoundHandle>::iterator it = m_boundSelects.find(boundHandleInfo);
			exASSERT_MSG(it != m_boundSelects.end(), L"Not bound to passed hStmt and column for Select on this buffer");

			if (m_columnSize > 0 || m_decimalDigits > 0)
			{
				SqlDescHandle hDesc(pHStmt, RowDescriptorType::ROW);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_DATA_PTR, (SQLINTEGER)NULL);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_INDICATOR_PTR, (SQLINTEGER)NULL);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLINTEGER)NULL);
			}
			else
			{
				SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, m_sqlCType, NULL, 0, NULL);
				THROW_IFN_SUCCEEDED(SQLBindCol, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
			}
			return m_boundSelects.erase(it);
		}

	private:
		SQLPOINTER m_pBuffer;
		SQLSMALLINT m_sqlCType;
		SQLLEN m_bufferLength;

		std::set<ColumnBoundHandle> m_boundSelects;
	};
	
	// Array types
	typedef SqlCArrayBuffer<SQLWCHAR, SQL_C_WCHAR> SqlWCharArray;
	typedef SqlCArrayBuffer<SQLCHAR, SQL_C_CHAR> SqlCharArray;
	typedef SqlCArrayBuffer<SQLCHAR, SQL_C_BINARY> SqlBinaryArray;

	// the variant
	typedef boost::variant<
		SqlUShortBuffer, SqlSShortBuffer, SqlULongBuffer, SqlSLongBuffer, SqlUBigIntBuffer, SqlSBigIntBuffer,
		SqlTypeTimeStructBuffer, SqlTimeStructBuffer,
		SqlTypeDateStructBuffer, SqlDateStructBuffer,
		SqlTypeTimestampStructBuffer, SqlTimestamptStructBuffer,
		SqlNumericStructBuffer,
		SqlDoubleBuffer, SqlRealBuffer,
		SqlWCharArray, SqlCharArray, SqlBinaryArray,
		SqlCPointerBuffer
	> SqlCBufferVariant;
	
	typedef std::map<SQLUSMALLINT, SqlCBufferVariant> SqlCBufferVariantMap;


	class BindSelectVisitor
		: public boost::static_visitor<void>
	{
	public:
		BindSelectVisitor() = delete;
		BindSelectVisitor(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
			: m_columnNr(columnNr)
			, m_pHStmt(pHStmt)
		{};

		template<typename T>
		void operator()(T& t) const
		{
			t.BindSelect(m_columnNr, m_pHStmt);
		}
	private:
		SQLUSMALLINT m_columnNr;
		ConstSqlStmtHandlePtr m_pHStmt;
	};

	class UnbindVisitor
		: public boost::static_visitor<void>
	{
	public:

		template<typename T>
		void operator()(T& t) const
		{
			t.Unbind();
		}
	};

	extern EXODBCAPI SqlCBufferVariant CreateBuffer(SQLSMALLINT sqlCType, const ColumnBindInfo& bindInfo);
	extern EXODBCAPI SqlCBufferVariant CreateArrayBuffer(SQLSMALLINT sqlCType, const ColumnInfo& columnInfo);
	extern EXODBCAPI SQLLEN CalculateDisplaySize(SQLSMALLINT sqlType, SQLINTEGER columnSize, SQLSMALLINT numPrecRadix, SQLSMALLINT decimalDigits);
	extern EXODBCAPI bool IsArrayType(SQLSMALLINT sqlCType);

} // namespace exodbc
