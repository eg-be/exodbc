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
#include "Helpers.h"

// Other headers
#include "boost/variant.hpp"
#include "boost/variant/polymorphic_get.hpp"
#include "boost/signals2.hpp"

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
		ExtendedColumnPropertiesHolder(SQLINTEGER columnSize, SQLSMALLINT decimalDigits, SQLSMALLINT sqlType)
			: m_columnSize(columnSize)
			, m_decimalDigits(decimalDigits)
			, m_sqlType(sqlType)
		{};

		ExtendedColumnPropertiesHolder()
			: m_columnSize(0)
			, m_decimalDigits(0)
			, m_sqlType(SQL_UNKNOWN_TYPE)
		{};

		virtual ~ExtendedColumnPropertiesHolder()
		{};

		void SetColumnSize(SQLINTEGER columnSize) noexcept { m_columnSize = columnSize; };
		void SetDecimalDigits(SQLSMALLINT decimalDigits) noexcept { m_decimalDigits = decimalDigits; };
		void SetSqlType(SQLSMALLINT sqlType) noexcept { m_sqlType = sqlType; };

		SQLINTEGER GetColumnSize() const noexcept { return m_columnSize; };
		SQLSMALLINT GetDecimalDigits() const noexcept { return m_decimalDigits; };
		SQLSMALLINT GetSqlType() const noexcept { return m_sqlType; };

	protected:
		SQLINTEGER m_columnSize;
		SQLSMALLINT m_decimalDigits;
		SQLSMALLINT m_sqlType;
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

		SqlCBuffer(const std::wstring& queryName, ColumnFlags flags = ColumnFlag::CF_NONE)
			: SqlCBufferLengthIndicator()
			, ColumnFlags(flags)
			, ExtendedColumnPropertiesHolder()
			, m_queryName(queryName)
		{
			m_pBuffer = std::make_shared<T>();
			SetNull();
		};

		SqlCBuffer& operator=(const SqlCBuffer& other) = default;
		SqlCBuffer(const SqlCBuffer& other) = default;

		virtual ~SqlCBuffer()
		{
			// If we are still bound to columns or params, release the bindings and disconnect the signals
			// Also disconnect all signals
			for (auto it = m_resetParamsConnections.begin(); it != m_resetParamsConnections.end(); ++it)
			{
				try
				{
					it->first->ResetParams();
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(ex.ToString());
				}
				it->second.disconnect();
			}
			for (auto it = m_unbindColumnsConnections.begin(); it != m_unbindColumnsConnections.end(); ++it)
			{
				try
				{
					it->first->UnbindColumns();
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(ex.ToString());
				}
				it->second.disconnect();
			}
		};

		static SQLSMALLINT GetSqlCType() noexcept { return sqlCType; };
		static SQLLEN GetBufferLength() noexcept { return sizeof(T); };

		void SetValue(const T& value) noexcept { SetValue(value, GetBufferLength()); };
		void SetValue(const T& value, SQLLEN cb) noexcept { *m_pBuffer = value; SetCb(cb); };
		const T& GetValue() const { if (IsNull()) { NullValueException nve(GetQueryName()); SET_EXCEPTION_SOURCE(nve); throw nve; } return *m_pBuffer; };
		std::shared_ptr<const T> GetBuffer() const noexcept { return m_pBuffer; };

		const std::wstring& GetQueryName() const noexcept { return m_queryName; };

		operator T() const noexcept { return GetValue(); };

		void BindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());

			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, (SQLPOINTER*)m_pBuffer.get(), GetBufferLength(), m_pCb.get());
			THROW_IFN_SUCCESS(SQLBindCol, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());

			// get a notification if unbound
			m_unbindColumnsConnections[pHStmt] = pHStmt->ConnectFreeSignal(boost::bind(&SqlCBuffer::OnUnbindColumns, this, _1));
		};


		void OnResetParams(const SqlStmtHandle& stmt)
		{
			// remove from our map, we are no longer interested in resetting params on destruction

			auto it = m_resetParamsConnections.begin(); 
			while(it != m_resetParamsConnections.end())
			{
				if (*(it->first) == stmt)
				{
					it = m_resetParamsConnections.erase(it);
				}
				else
				{
					++it;
				}
			}
		}


		void OnUnbindColumns(const SqlStmtHandle& stmt)
		{
			// remove from our map, we are no longer interested in resetting params on destruction
			auto it = m_unbindColumnsConnections.begin();
			while(it != m_unbindColumnsConnections.end())
			{
				if (*(it->first) == stmt)
				{
					it = m_unbindColumnsConnections.erase(it);
				}
				else
				{
					++it;
				}
			}
		}


		void BindParameter(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, bool useSqlDescribeParam = true)
		{
			exASSERT(paramNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());

			// Query the database about the parameter. Note: Some Drivers (access) do not support querying, then use the info set
			// on the extended properties (or fail, if those are not set)
			SQLSMALLINT paramSqlType = SQL_UNKNOWN_TYPE;
			SQLULEN paramCharSize = 0;
			SQLSMALLINT paramDecimalDigits = 0;
			SQLSMALLINT paramNullable = SQL_NULLABLE_UNKNOWN;
			if (useSqlDescribeParam)
			{
				SQLRETURN ret = SQLDescribeParam(pHStmt->GetHandle(), paramNr, &paramSqlType, &paramCharSize, &paramDecimalDigits, &paramNullable);
				THROW_IFN_SUCCESS(SQLDescribeParam, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
			}
			else
			{			
				paramSqlType = GetSqlType();
				exASSERT(paramSqlType != SQL_UNKNOWN_TYPE);
				paramDecimalDigits = GetDecimalDigits();
				paramCharSize = GetColumnSize();
			}

			// Check if we think its nullable, but the db does not think so
			if (Test(ColumnFlag::CF_NULLABLE))
			{
				exASSERT_MSG(paramNullable == SQL_NULLABLE || paramNullable == SQL_NULLABLE_UNKNOWN, L"Column is defined with flag CF_NULLABLE, but the Database has marked the parameter as not nullable");
			}

			// And bind using the information just read
			SQLRETURN ret = SQLBindParameter(pHStmt->GetHandle(),paramNr, SQL_PARAM_INPUT, sqlCType, paramSqlType, paramCharSize, paramDecimalDigits, (SQLPOINTER*) m_pBuffer.get(), GetBufferLength(), m_pCb.get());
			THROW_IFN_SUCCESS(SQLBindParameter, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());

			// Connect a signal that we are bound to this handle now and get notified if params get reseted
			m_resetParamsConnections[pHStmt] = pHStmt->ConnectFreeSignal(boost::bind(&SqlCBuffer::OnResetParams, this, _1));
		}

	private:
		std::shared_ptr<T> m_pBuffer;
		std::wstring m_queryName;
		std::map<ConstSqlStmtHandlePtr, boost::signals2::connection> m_unbindColumnsConnections;
		std::map<ConstSqlStmtHandlePtr, boost::signals2::connection> m_resetParamsConnections;
	};

	template<>
	void SqlCBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>::BindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
	{
		exASSERT(columnNr >= 1);
		exASSERT(pHStmt != NULL);
		exASSERT(pHStmt->IsAllocated());
		exASSERT(m_columnSize > 0);
		exASSERT(m_decimalDigits >= 0);

		SqlDescHandle hDesc(pHStmt, RowDescriptorType::ROW);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_TYPE, (SQLPOINTER) SQL_C_NUMERIC);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN) m_columnSize));
		SetDescriptionField(hDesc, columnNr, SQL_DESC_SCALE, (SQLPOINTER) m_decimalDigits);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_DATA_PTR, (SQLPOINTER) m_pBuffer.get());
		SetDescriptionField(hDesc, columnNr, SQL_DESC_INDICATOR_PTR, (SQLPOINTER) m_pCb.get());
		SetDescriptionField(hDesc, columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLPOINTER) m_pCb.get());

		// get a notification if unbound
		m_unbindColumnsConnections[pHStmt] = pHStmt->ConnectFreeSignal(boost::bind(&SqlCBuffer::OnUnbindColumns, this, _1));
	}


	template<>
	void SqlCBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>::BindParameter(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, bool useSqlDescribeParam)
	{
		//exASSERT_MSG(GetSqlType() != SQL_UNKNOWN_TYPE, L"Extended Property SqlType must be set if this Buffer shall be used as parameter");
		exASSERT(paramNr >= 1);
		exASSERT(pHStmt != NULL);
		exASSERT(pHStmt->IsAllocated());

		// Query the database about the parameter. Note: Some Drivers (access) do not support querying, then use the info set
		// on the extended properties (or fail, if those are not set)
		SQLSMALLINT paramSqlType = SQL_UNKNOWN_TYPE;
		SQLULEN paramCharSize = 0;
		SQLSMALLINT paramDecimalDigits = 0;
		SQLSMALLINT paramNullable = SQL_NULLABLE_UNKNOWN;
		if (useSqlDescribeParam)
		{
			SQLRETURN ret = SQLDescribeParam(pHStmt->GetHandle(), paramNr, &paramSqlType, &paramCharSize, &paramDecimalDigits, &paramNullable);
			THROW_IFN_SUCCESS(SQLDescribeParam, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
		}
		else
		{
			paramSqlType = GetSqlType();
			exASSERT(paramSqlType != SQL_UNKNOWN_TYPE);
			paramDecimalDigits = GetDecimalDigits();
			paramCharSize = GetColumnSize();
		}

		// Check if we think its nullable, but the db does not think so
		if (Test(ColumnFlag::CF_NULLABLE))
		{
			exASSERT_MSG(paramNullable == SQL_NULLABLE || paramNullable == SQL_NULLABLE_UNKNOWN, L"Column is defined with flag CF_NULLABLE, but the Database has marked the parameter as not nullable");
		}

		// And bind using the information just read
		SQLRETURN ret = SQLBindParameter(pHStmt->GetHandle(), paramNr, SQL_PARAM_INPUT, SQL_C_NUMERIC, paramSqlType, paramCharSize, paramDecimalDigits, (SQLPOINTER*)m_pBuffer.get(), GetBufferLength(), m_pCb.get());
		THROW_IFN_SUCCESS(SQLBindParameter, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());

		// Do some additional steps for numeric types
		SqlDescHandle hDesc(pHStmt, RowDescriptorType::PARAM);
		SetDescriptionField(hDesc, paramNr, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC);
		SetDescriptionField(hDesc, paramNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN)paramCharSize));
		SetDescriptionField(hDesc, paramNr, SQL_DESC_SCALE, (SQLPOINTER)paramDecimalDigits);
		SetDescriptionField(hDesc, paramNr, SQL_DESC_DATA_PTR, (SQLPOINTER)m_pBuffer.get());

		// Connect a signal that we are bound to this handle now and get notified if params get reseted
		m_resetParamsConnections[pHStmt] = pHStmt->ConnectFreeSignal(boost::bind(&SqlCBuffer::OnResetParams, this, _1));
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

		SqlCArrayBuffer(const std::wstring& queryName, SQLLEN nrOfElements, ColumnFlags flags = ColumnFlag::CF_NONE)
			: SqlCBufferLengthIndicator()
			, ColumnFlags(flags)
			, ExtendedColumnPropertiesHolder()
			, m_nrOfElements(nrOfElements)
			, m_pBuffer(std::make_shared<std::vector<T>>(nrOfElements))
			, m_queryName(queryName)
		{
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
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(boost::str(boost::wformat(L"Failed to unbind column %d from stmt-handle %d: %s") % bindInfo.m_columnNr %bindInfo.m_pHStmt->GetHandle() %ex.ToString()));
					++it;
				}
				// Unbind all params
				std::set<ColumnBoundHandle>::iterator itParams = m_boundParams.begin();
				while (itParams != m_boundParams.end())
				{
					try
					{
						ColumnBoundHandle bindInfo = *itParams;
						if(bindInfo.m_pHStmt && bindInfo.m_pHStmt->IsAllocated())
						{
							SQLRETURN ret = SQLFreeStmt(bindInfo.m_pHStmt->GetHandle(), SQL_RESET_PARAMS);
							THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, bindInfo.m_pHStmt->GetHandle());
						}
						itParams = m_boundParams.erase(itParams);
					}
					catch (const Exception& ex)
					{
						LOG_ERROR(boost::str(boost::wformat(L"Failed to unbind column %d from parameter stmt-handle %d: %s") % bindInfo.m_columnNr %bindInfo.m_pHStmt->GetHandle() % ex.ToString()));
						++itParams;
					}
				}
			}
		};

		static SQLSMALLINT GetSqlCType() noexcept { return sqlCType; };
		SQLLEN GetBufferLength() const noexcept { return sizeof(T) * GetNrOfElements(); };

		void SetValue(const std::vector<SQLCHAR>& value)
		{
			SetValue(value, value.size());
		}

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
			SetCb(cb);
		};
		const std::shared_ptr<std::vector<T>> GetBuffer() const noexcept { return m_pBuffer; };
		SQLLEN GetNrOfElements() const noexcept { return m_nrOfElements; };

		//operator T*() const noexcept { return m_pBuffer.get(); };

		const std::wstring& GetQueryName() const noexcept { return m_queryName; };

		std::wstring GetWString() const { if (IsNull()) { NullValueException nve(GetQueryName()); SET_EXCEPTION_SOURCE(nve); throw nve; } return m_pBuffer->data(); };
		std::string GetString() const { if (IsNull()) { NullValueException nve(GetQueryName()); SET_EXCEPTION_SOURCE(nve); throw nve; } return (char*)m_pBuffer->data(); };

		void SetWString(const std::wstring& ws) { std::vector<SQLWCHAR> vec(ws.begin(), ws.end()); SetValue(vec, SQL_NTS); };
		void SetString(const std::string& s) { std::vector<SQLCHAR> vec(s.begin(), s.end()); SetValue(vec, SQL_NTS); }

		void BindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			ColumnBoundHandle boundHandleInfo(pHStmt, columnNr);
			exASSERT_MSG(m_boundSelects.find(boundHandleInfo) == m_boundSelects.end(), L"Already bound to passed hStmt and column for Select on this buffer");

			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, (SQLPOINTER*) &(*m_pBuffer)[0], GetBufferLength(), m_pCb.get());
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
			// I see no way to unbind single params?
			std::set<ColumnBoundHandle>::iterator itParams = m_boundParams.begin();
			while (itParams != m_boundParams.end())
			{
				ColumnBoundHandle bindInfo = *itParams;
				exASSERT(bindInfo.m_pHStmt && bindInfo.m_pHStmt->IsAllocated());
				SQLRETURN ret = SQLFreeStmt(bindInfo.m_pHStmt->GetHandle(), SQL_RESET_PARAMS);
				THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, bindInfo.m_pHStmt->GetHandle());
				itParams = m_boundParams.erase(itParams);
			}
		}

		void BindParameter(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, bool useSqlDescribeParam = true)
		{
			exASSERT(paramNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			ColumnBoundHandle boundHandleInfo(pHStmt, paramNr);
			exASSERT_MSG(m_boundParams.find(boundHandleInfo) == m_boundParams.end(), L"Already bound to passed hStmt and paramNr as Parameter on this buffer");

			// Query the database about the parameter. Note: Some Drivers (access) do not support querying, then use the info set
			// on the extended properties (or fail, if those are not set)
			SQLSMALLINT paramSqlType = SQL_UNKNOWN_TYPE;
			SQLULEN paramCharSize = 0;
			SQLSMALLINT paramDecimalDigits = 0;
			SQLSMALLINT paramNullable = SQL_NULLABLE_UNKNOWN;
			if (useSqlDescribeParam)
			{
				SQLRETURN ret = SQLDescribeParam(pHStmt->GetHandle(), paramNr, &paramSqlType, &paramCharSize, &paramDecimalDigits, &paramNullable);
				THROW_IFN_SUCCESS(SQLDescribeParam, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
			}
			else
			{
				paramSqlType = GetSqlType();
				exASSERT(paramSqlType != SQL_UNKNOWN_TYPE);
				paramDecimalDigits = GetDecimalDigits();
				paramCharSize = GetColumnSize();
			}

			// Check if we think its nullable, but the db does not think so
			if (Test(ColumnFlag::CF_NULLABLE))
			{
				exASSERT_MSG((paramNullable == SQL_NULLABLE || paramNullable == SQL_NULLABLE_UNKNOWN), L"Column is defined with flag CF_NULLABLE, but the Database has marked the parameter as not nullable");
			}

			// And bind using the information just read
			SQLRETURN ret = SQLBindParameter(pHStmt->GetHandle(), paramNr, SQL_PARAM_INPUT, sqlCType, paramSqlType, paramCharSize, paramDecimalDigits, (SQLPOINTER*)&(*m_pBuffer)[0], GetBufferLength(), m_pCb.get());
			THROW_IFN_SUCCESS(SQLBindParameter, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
			m_boundParams.insert(boundHandleInfo);
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
		std::set<ColumnBoundHandle> m_boundSelects;
		std::set<ColumnBoundHandle> m_boundParams;
		std::wstring m_queryName;
	};

	// Array types
	typedef SqlCArrayBuffer<SQLWCHAR, SQL_C_WCHAR> SqlWCharArray;
	typedef SqlCArrayBuffer<SQLCHAR, SQL_C_CHAR> SqlCharArray;
	typedef SqlCArrayBuffer<SQLCHAR, SQL_C_BINARY> SqlBinaryArray;


	class SqlCPointerBuffer
		: public SqlCBufferLengthIndicator
		, public ColumnFlags
		, public ExtendedColumnPropertiesHolder
	{
	public:
		SqlCPointerBuffer() = delete;
		SqlCPointerBuffer(const std::wstring& queryName, SQLSMALLINT sqlType, SQLPOINTER pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags, SQLINTEGER columnSize, SQLSMALLINT decimalDigits)
			: SqlCBufferLengthIndicator()
			, ColumnFlags(flags)
			, ExtendedColumnPropertiesHolder(columnSize, decimalDigits, sqlType)
			, m_pBuffer(pBuffer)
			, m_sqlCType(sqlCType)
			, m_bufferLength(bufferSize)
			, m_queryName(queryName)
		{
			if (flags.Test(ColumnFlag::CF_NULLABLE))
			{
				SetNull();
			}
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
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(boost::str(boost::wformat(L"Failed to unbind column %d from stmt-handle %d: %s") % bindInfo.m_columnNr %bindInfo.m_pHStmt->GetHandle() % ex.ToString()));
					++it;
				}
			}
			// Unbind all params
			std::set<ColumnBoundHandle>::iterator itParams = m_boundParams.begin();
			while (itParams != m_boundParams.end())
			{
				ColumnBoundHandle bindInfo = *itParams;
				try
				{
					if (bindInfo.m_pHStmt && bindInfo.m_pHStmt->IsAllocated())
					{
						SQLRETURN ret = SQLFreeStmt(bindInfo.m_pHStmt->GetHandle(), SQL_RESET_PARAMS);
						THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, bindInfo.m_pHStmt->GetHandle());
					}
					itParams = m_boundParams.erase(itParams);
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(boost::str(boost::wformat(L"Failed to unbind column %d from parameter stmt-handle %d: %s") % bindInfo.m_columnNr %bindInfo.m_pHStmt->GetHandle() % ex.ToString()));
					++itParams;
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

			if (m_sqlCType == SQL_C_NUMERIC)
			{
				SqlDescHandle hDesc(pHStmt, RowDescriptorType::ROW);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_TYPE, (SQLPOINTER)m_sqlCType);
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

		void BindParameter(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, bool useSqlDescribeParam = true)
		{
			//exASSERT_MSG(GetSqlType() != SQL_UNKNOWN_TYPE, L"Extended Property SqlType must be set if this Buffer shall be used as parameter");
			exASSERT(paramNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			ColumnBoundHandle boundHandleInfo(pHStmt, paramNr);
			exASSERT_MSG(m_boundParams.find(boundHandleInfo) == m_boundParams.end(), L"Already bound to passed hStmt and paramNr as Parameter on this buffer");

			// Query the database about the parameter. Note: Some Drivers (access) do not support querying, then use the info set
			// on the extended properties (or fail, if those are not set)
			SQLSMALLINT paramSqlType = SQL_UNKNOWN_TYPE;
			SQLULEN paramCharSize = 0;
			SQLSMALLINT paramDecimalDigits = 0;
			SQLSMALLINT paramNullable = SQL_NULLABLE_UNKNOWN;
			if (useSqlDescribeParam)
			{
				SQLRETURN ret = SQLDescribeParam(pHStmt->GetHandle(), paramNr, &paramSqlType, &paramCharSize, &paramDecimalDigits, &paramNullable);
				THROW_IFN_SUCCESS(SQLDescribeParam, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
			}
			else
			{
				paramSqlType = GetSqlType();
				exASSERT(paramSqlType != SQL_UNKNOWN_TYPE);
				paramDecimalDigits = GetDecimalDigits();
				paramCharSize = GetColumnSize();
			}

			// Check if we think its nullable, but the db does not think so
			if (Test(ColumnFlag::CF_NULLABLE))
			{
				exASSERT_MSG(paramNullable == SQL_NULLABLE || paramNullable == SQL_NULLABLE_UNKNOWN, L"Column is defined with flag CF_NULLABLE, but the Database has marked the parameter as not nullable");
			}

			// And bind using the information just read
			SQLRETURN ret = SQLBindParameter(pHStmt->GetHandle(), paramNr, SQL_PARAM_INPUT, m_sqlCType, paramSqlType, paramCharSize, paramDecimalDigits, (SQLPOINTER*)m_pBuffer, GetBufferLength(), m_pCb.get());
			THROW_IFN_SUCCESS(SQLBindParameter, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());

			// Do some additional steps for numeric types
			if (m_sqlCType == SQL_C_NUMERIC)
			{
				SqlDescHandle hDesc(pHStmt, RowDescriptorType::PARAM);
				SetDescriptionField(hDesc, paramNr, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC);
				SetDescriptionField(hDesc, paramNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN)paramCharSize));
				SetDescriptionField(hDesc, paramNr, SQL_DESC_SCALE, (SQLPOINTER)paramDecimalDigits);
				SetDescriptionField(hDesc, paramNr, SQL_DESC_DATA_PTR, (SQLPOINTER)m_pBuffer);
			}

			m_boundParams.insert(boundHandleInfo);
		}


		const std::wstring& GetQueryName() const noexcept { return m_queryName; };

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
			// I see no way to unbind single params?
			std::set<ColumnBoundHandle>::iterator itParams = m_boundParams.begin();
			while (itParams != m_boundParams.end())
			{
				ColumnBoundHandle bindInfo = *itParams;
				exASSERT(bindInfo.m_pHStmt && bindInfo.m_pHStmt->IsAllocated());
				SQLRETURN ret = SQLFreeStmt(bindInfo.m_pHStmt->GetHandle(), SQL_RESET_PARAMS);
				THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, bindInfo.m_pHStmt->GetHandle());
				itParams = m_boundParams.erase(itParams);
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
		std::set<ColumnBoundHandle> m_boundParams;
		std::wstring m_queryName;
	};
	

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

	extern EXODBCAPI SqlCBufferVariant CreateBuffer(SQLSMALLINT sqlCType, const std::wstring& queryName);
	extern EXODBCAPI SqlCBufferVariant CreateArrayBuffer(SQLSMALLINT sqlCType, const std::wstring& queryName, const ColumnInfo& columnInfo);
	extern EXODBCAPI SQLLEN CalculateDisplaySize(SQLSMALLINT sqlType, SQLINTEGER columnSize, SQLSMALLINT numPrecRadix, SQLSMALLINT decimalDigits);
	extern EXODBCAPI bool IsArrayType(SQLSMALLINT sqlCType);

} // namespace exodbc
