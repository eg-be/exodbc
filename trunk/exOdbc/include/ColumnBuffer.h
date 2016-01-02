/*!
* \file ColumnBuffer.h
* \author Elias Gerber <eg@elisium.ch>
* \date 01.10.2015
* \brief Header file for the ColumnBuffer class and all its helpers.
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
	class ColumnBufferLengthIndicator
	{
	public:
		ColumnBufferLengthIndicator()
		{
			m_cb = 0;
		}

		ColumnBufferLengthIndicator(const ColumnBufferLengthIndicator& other) = delete;
		ColumnBufferLengthIndicator& operator=(const ColumnBufferLengthIndicator& other) = delete;

		virtual ~ColumnBufferLengthIndicator()
		{};

		void SetCb(SQLLEN cb) noexcept { m_cb = cb; };
		SQLLEN GetCb() const noexcept { return m_cb; };

		void SetNull() noexcept { m_cb = SQL_NULL_DATA; };
		bool IsNull() const noexcept { return m_cb == SQL_NULL_DATA; };

	protected:
		SQLLEN m_cb;
	};
	typedef std::shared_ptr<ColumnBufferLengthIndicator> ColumnBufferLengthIndicatorPtr;


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
	typedef std::shared_ptr<ExtendedColumnPropertiesHolder> ExtendedColumnPropertiesHolderPtr;


	class ColumnBufferBindInfoHolder
	{
	public:
		ColumnBufferBindInfoHolder() = default;
		ColumnBufferBindInfoHolder(const ColumnBufferBindInfoHolder& other) = delete;
		ColumnBufferBindInfoHolder& operator=(const ColumnBufferBindInfoHolder& other) = delete;

		virtual ~ColumnBufferBindInfoHolder()
		{
			// If we are still bound to columns or params, release the bindings and disconnect the signals
			for (auto it = m_resetParamsConnections.begin(); it != m_resetParamsConnections.end(); ++it)
			{
				// disconnect first, we know its being reseted
				it->second.disconnect();
				try
				{
					it->first->ResetParams();
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(ex.ToString());
				}
			}
			for (auto it = m_unbindColumnsConnections.begin(); it != m_unbindColumnsConnections.end(); ++it)
			{
				// disconnect first, we know its being unbound now.
				it->second.disconnect();
				try
				{
					it->first->UnbindColumns();
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(ex.ToString());
				}
			}
		};

		void OnResetParams(const SqlStmtHandle& stmt)
		{
			// remove from our map, we are no longer interested in resetting params on destruction
			auto it = m_resetParamsConnections.begin();
			while (it != m_resetParamsConnections.end())
			{
				if (*(it->first) == stmt)
				{
					it->second.disconnect();
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
			while (it != m_unbindColumnsConnections.end())
			{
				if (*(it->first) == stmt)
				{
					it->second.disconnect();
					it = m_unbindColumnsConnections.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

	protected:
		std::map<ConstSqlStmtHandlePtr, boost::signals2::connection> m_unbindColumnsConnections;
		std::map<ConstSqlStmtHandlePtr, boost::signals2::connection> m_resetParamsConnections;
	};


	template<typename T, SQLSMALLINT sqlCType , typename std::enable_if<!std::is_pointer<T>::value, T>::type* = 0>
	class ColumnBuffer
		: public ColumnBufferLengthIndicator
		, public ColumnFlags
		, public ExtendedColumnPropertiesHolder
		, public ColumnBufferBindInfoHolder
	{
	public:
		ColumnBuffer()
			: ColumnBufferLengthIndicator()
			, ColumnFlags()
			, ExtendedColumnPropertiesHolder()
			, ColumnBufferBindInfoHolder()
		{
			::ZeroMemory(&m_buffer, sizeof(T));
			SetNull();
		};

		ColumnBuffer(const std::wstring& queryName, ColumnFlags flags = ColumnFlag::CF_NONE)
			: ColumnBufferLengthIndicator()
			, ColumnFlags(flags)
			, ExtendedColumnPropertiesHolder()
			, ColumnBufferBindInfoHolder()
			, m_queryName(queryName)
		{
			::ZeroMemory(&m_buffer, sizeof(T));
			SetNull();
		};


		/*!
		* \brief: Create a new instance wrapped into a shared_ptr
		*/
		static std::shared_ptr<ColumnBuffer> Create(const std::wstring& queryName, ColumnFlags flags = ColumnFlag::CF_NONE)
		{
			return std::make_shared<ColumnBuffer>(queryName, flags);
		}


		ColumnBuffer& operator=(const ColumnBuffer& other) = delete;
		ColumnBuffer(const ColumnBuffer& other) = delete;

		virtual ~ColumnBuffer()
		{ };

		static SQLSMALLINT GetSqlCType() noexcept { return sqlCType; };
		static SQLLEN GetBufferLength() noexcept { return sizeof(T); };

		void SetValue(const T& value) noexcept { SetValue(value, GetBufferLength()); };
		void SetValue(const T& value, SQLLEN cb) noexcept { m_buffer = value; SetCb(cb); };
		const T& GetValue() const { if (IsNull()) { NullValueException nve(GetQueryName()); SET_EXCEPTION_SOURCE(nve); throw nve; } return m_buffer; };

		const std::wstring& GetQueryName() const noexcept { return m_queryName; };

		operator T() const noexcept { return GetValue(); };

		void BindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());

			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, (SQLPOINTER*)&m_buffer, GetBufferLength(), &m_cb);
			THROW_IFN_SUCCESS(SQLBindCol, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());

			// get a notification if unbound
			m_unbindColumnsConnections[pHStmt] = pHStmt->ConnectUnbindColumnsSignal(boost::bind(&ColumnBuffer::OnUnbindColumns, this, _1));
		};


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
			SQLRETURN ret = SQLBindParameter(pHStmt->GetHandle(),paramNr, SQL_PARAM_INPUT, sqlCType, paramSqlType, paramCharSize, paramDecimalDigits, (SQLPOINTER*) &m_buffer, GetBufferLength(), &m_cb);
			THROW_IFN_SUCCESS(SQLBindParameter, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());

			// Connect a signal that we are bound to this handle now and get notified if params get reseted
			m_resetParamsConnections[pHStmt] = pHStmt->ConnectResetParamsSignal(boost::bind(&ColumnBuffer::OnResetParams, this, _1));
		}

	private:
		T m_buffer;
		std::wstring m_queryName;
	};

	template<>
	void ColumnBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>::BindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
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
		SetDescriptionField(hDesc, columnNr, SQL_DESC_DATA_PTR, (SQLPOINTER) &m_buffer);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_INDICATOR_PTR, (SQLPOINTER) &m_cb);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLPOINTER) &m_cb);

		// get a notification if unbound
		m_unbindColumnsConnections[pHStmt] = pHStmt->ConnectUnbindColumnsSignal(boost::bind(&ColumnBuffer::OnUnbindColumns, this, _1));
	}


	template<>
	void ColumnBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>::BindParameter(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, bool useSqlDescribeParam)
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
		SQLRETURN ret = SQLBindParameter(pHStmt->GetHandle(), paramNr, SQL_PARAM_INPUT, SQL_C_NUMERIC, paramSqlType, paramCharSize, paramDecimalDigits, (SQLPOINTER*)&m_buffer, GetBufferLength(), &m_cb);
		THROW_IFN_SUCCESS(SQLBindParameter, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());

		// Do some additional steps for numeric types
		SqlDescHandle hDesc(pHStmt, RowDescriptorType::PARAM);
		SetDescriptionField(hDesc, paramNr, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC);
		SetDescriptionField(hDesc, paramNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN)paramCharSize));
		SetDescriptionField(hDesc, paramNr, SQL_DESC_SCALE, (SQLPOINTER)paramDecimalDigits);
		SetDescriptionField(hDesc, paramNr, SQL_DESC_DATA_PTR, (SQLPOINTER)&m_buffer);

		// Connect a signal that we are bound to this handle now and get notified if params get reseted
		m_resetParamsConnections[pHStmt] = pHStmt->ConnectResetParamsSignal(boost::bind(&ColumnBuffer::OnResetParams, this, _1));
	}


	// Integer types
	typedef ColumnBuffer<SQLSMALLINT, SQL_C_USHORT> UShortColumnBuffer;
	typedef std::shared_ptr<UShortColumnBuffer> UShortColumnBufferPtr;
	typedef ColumnBuffer<SQLINTEGER, SQL_C_ULONG> ULongColumnBuffer;
	typedef std::shared_ptr<ULongColumnBuffer> ULongColumnBufferPtr;
	typedef ColumnBuffer<SQLBIGINT, SQL_C_UBIGINT> UBigIntColumnBuffer;
	typedef std::shared_ptr<UBigIntColumnBuffer> UBigIntColumnBufferPtr;
	typedef ColumnBuffer<SQLSMALLINT, SQL_C_SSHORT> ShortColumnBuffer;
	typedef std::shared_ptr<ShortColumnBuffer> ShortColumnBufferPtr;
	typedef ColumnBuffer<SQLINTEGER, SQL_C_SLONG> LongColumnBuffer;
	typedef std::shared_ptr<LongColumnBuffer> LongColumnBufferPtr;
	typedef ColumnBuffer<SQLBIGINT, SQL_C_SBIGINT> BigIntColumnBuffer;
	typedef std::shared_ptr<BigIntColumnBuffer> BigIntColumnBufferPtr;

	// datetime types
	typedef ColumnBuffer<SQL_TIME_STRUCT, SQL_C_TYPE_TIME> TypeTimeColumnBuffer;
	typedef std::shared_ptr<TypeTimeColumnBuffer> TypeTimeColumnBufferPtr;
	typedef ColumnBuffer<SQL_TIME_STRUCT, SQL_C_TIME> TimeColumnBuffer;
	typedef std::shared_ptr<TimeColumnBuffer> TimeColumnBufferPtr;
	typedef ColumnBuffer<SQL_DATE_STRUCT, SQL_C_TYPE_DATE> TypeDateColumnBuffer;
	typedef std::shared_ptr<TypeDateColumnBuffer> TypeDateColumnBufferPtr;
	typedef ColumnBuffer<SQL_DATE_STRUCT, SQL_C_DATE> DateColumnBuffer;
	typedef std::shared_ptr<DateColumnBuffer> DateColumnBufferPtr;
	typedef ColumnBuffer<SQL_TIMESTAMP_STRUCT, SQL_C_TYPE_TIMESTAMP> TypeTimestampColumnBuffer;
	typedef std::shared_ptr<TypeTimestampColumnBuffer> TypeTimestampColumnBufferPtr;
	typedef ColumnBuffer<SQL_TIMESTAMP_STRUCT, SQL_C_TIMESTAMP> TimestampColumnBuffer;
	typedef std::shared_ptr<TimestampColumnBuffer> TimestampColumnBufferPtr;

	// Numeric types
	typedef ColumnBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC> NumericColumnBuffer;
	typedef std::shared_ptr<NumericColumnBuffer> NumericColumnBufferPtr;

	// Floating types
	typedef ColumnBuffer<SQLDOUBLE, SQL_C_DOUBLE> DoubleColumnBuffer;
	typedef std::shared_ptr<DoubleColumnBuffer> DoubleColumnBufferPtr;
	typedef ColumnBuffer<SQLREAL, SQL_C_FLOAT> RealColumnBuffer;
	typedef std::shared_ptr<RealColumnBuffer> RealColumnBufferPtr;

	template<typename T, SQLSMALLINT sqlCType, typename std::enable_if<!std::is_pointer<T>::value, T>::type* = 0>
	class ColumnArrayBuffer
		: public ColumnBufferLengthIndicator
		, public ColumnFlags
		, public ExtendedColumnPropertiesHolder
		, public ColumnBufferBindInfoHolder
	{
	public:
		ColumnArrayBuffer() = delete;

		ColumnArrayBuffer(const std::wstring& queryName, SQLLEN nrOfElements, ColumnFlags flags = ColumnFlag::CF_NONE)
			: ColumnBufferLengthIndicator()
			, ColumnFlags(flags)
			, ExtendedColumnPropertiesHolder()
			, m_nrOfElements(nrOfElements)
			, m_buffer(nrOfElements)
			, m_queryName(queryName)
		{
			SetNull();
		};

		ColumnArrayBuffer(const ColumnArrayBuffer& other) = delete;
		ColumnArrayBuffer& operator=(const ColumnArrayBuffer& other) = delete;

		virtual ~ColumnArrayBuffer() 
		{
		};


		/*!
		* \brief: Create a new instance wrapped into a shared_ptr
		*/
		static std::shared_ptr<ColumnArrayBuffer> Create(const std::wstring& queryName, SQLLEN nrOfElements, ColumnFlags flags = ColumnFlag::CF_NONE)
		{
			return std::make_shared<ColumnArrayBuffer>(queryName, nrOfElements, flags);
		}


		static SQLSMALLINT GetSqlCType() noexcept { return sqlCType; };
		SQLLEN GetBufferLength() const noexcept { return sizeof(T) * GetNrOfElements(); };

		void SetValue(const std::vector<SQLCHAR>& value)
		{
			SetValue(value, value.size());
		}

		void SetValue(const std::vector<T>& value, SQLLEN cb)
		{ 
			exASSERT(value.size() <= m_buffer.capacity()); 
			size_t index = 0;
			for (auto it = value.begin(); it != value.end(); ++it)
			{
				m_buffer[index] = *it;
				++index;
			}
			// null-terminate if last element added not already was a '0'
			// and if there is still some space for the last '0'
			// if there is no space, fail
			if (cb == SQL_NTS && value.back() != 0)
			{
				exASSERT(index < m_buffer.capacity());
				m_buffer[index] = 0;
			}
			SetCb(cb);
		};
		const std::vector<T>& GetBuffer() const noexcept { return m_buffer; };
		SQLLEN GetNrOfElements() const noexcept { return m_nrOfElements; };

		const std::wstring& GetQueryName() const noexcept { return m_queryName; };

		std::wstring GetWString() const { if (IsNull()) { NullValueException nve(GetQueryName()); SET_EXCEPTION_SOURCE(nve); throw nve; } return m_buffer.data(); };
		std::string GetString() const { if (IsNull()) { NullValueException nve(GetQueryName()); SET_EXCEPTION_SOURCE(nve); throw nve; } return (char*)m_buffer.data(); };

		void SetWString(const std::wstring& ws) { std::vector<SQLWCHAR> vec(ws.begin(), ws.end()); SetValue(vec, SQL_NTS); };
		void SetString(const std::string& s) { std::vector<SQLCHAR> vec(s.begin(), s.end()); SetValue(vec, SQL_NTS); }

		void BindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());

			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, (SQLPOINTER*) &m_buffer[0], GetBufferLength(), &m_cb);
			THROW_IFN_SUCCESS(SQLBindCol, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());

			// get a notification if unbound
			m_unbindColumnsConnections[pHStmt] = pHStmt->ConnectUnbindColumnsSignal(boost::bind(&ColumnArrayBuffer::OnUnbindColumns, this, _1));
		};


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
				exASSERT_MSG((paramNullable == SQL_NULLABLE || paramNullable == SQL_NULLABLE_UNKNOWN), L"Column is defined with flag CF_NULLABLE, but the Database has marked the parameter as not nullable");
			}

			// And bind using the information just read
			SQLRETURN ret = SQLBindParameter(pHStmt->GetHandle(), paramNr, SQL_PARAM_INPUT, sqlCType, paramSqlType, paramCharSize, paramDecimalDigits, (SQLPOINTER*)&m_buffer[0], GetBufferLength(), &m_cb);
			THROW_IFN_SUCCESS(SQLBindParameter, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());

			// Connect a signal that we are bound to this handle now and get notified if params get reseted
			m_resetParamsConnections[pHStmt] = pHStmt->ConnectResetParamsSignal(boost::bind(&ColumnArrayBuffer::OnResetParams, this, _1));
		}

	private:
		SQLLEN m_nrOfElements;
		std::vector<T> m_buffer;
		std::wstring m_queryName;
	};

	// Array types
	typedef ColumnArrayBuffer<SQLWCHAR, SQL_C_WCHAR> WCharColumnArrayBuffer;
	typedef std::shared_ptr<WCharColumnArrayBuffer> WCharColumnBufferPtr;
	typedef ColumnArrayBuffer<SQLCHAR, SQL_C_CHAR> CharColumnArrayBuffer;
	typedef std::shared_ptr<CharColumnArrayBuffer> CharColumnBufferPtr;
	typedef ColumnArrayBuffer<SQLCHAR, SQL_C_BINARY> BinaryColumnArrayBuffer;
	typedef std::shared_ptr<BinaryColumnArrayBuffer> BinaryColumnBufferPtr;


	class SqlCPointerBuffer
		: public ColumnBufferLengthIndicator
		, public ColumnFlags
		, public ExtendedColumnPropertiesHolder
		, public ColumnBufferBindInfoHolder
	{
	public:
		SqlCPointerBuffer() = delete;
		SqlCPointerBuffer(const std::wstring& queryName, SQLSMALLINT sqlType, SQLPOINTER pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags, SQLINTEGER columnSize, SQLSMALLINT decimalDigits)
			: ColumnBufferLengthIndicator()
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

		SqlCPointerBuffer& operator=(const SqlCPointerBuffer& other) = delete;
		SqlCPointerBuffer(const SqlCPointerBuffer& other) = delete;

		virtual ~SqlCPointerBuffer()
		{ };


		/*!
		* \brief: Create a new instance wrapped into a shared_ptr
		*/
		static std::shared_ptr<SqlCPointerBuffer> Create(const std::wstring& queryName, SQLSMALLINT sqlType, SQLPOINTER pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags, SQLINTEGER columnSize, SQLSMALLINT decimalDigits)
		{
			return std::make_shared<SqlCPointerBuffer>(queryName, sqlType, pBuffer, sqlCType, bufferSize, flags, columnSize, decimalDigits);
		}


		SQLSMALLINT GetSqlCType() const noexcept { return m_sqlCType; };
		SQLLEN GetBufferLength() const noexcept { return m_bufferLength; };

		void BindSelect(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			exASSERT(m_columnSize >= 0);
			exASSERT(m_decimalDigits >= 0);

			if (m_sqlCType == SQL_C_NUMERIC)
			{
				SqlDescHandle hDesc(pHStmt, RowDescriptorType::ROW);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_TYPE, (SQLPOINTER)m_sqlCType);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN)m_columnSize));
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_SCALE, (SQLPOINTER)m_decimalDigits);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_DATA_PTR, (SQLPOINTER)m_pBuffer);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_INDICATOR_PTR, (SQLPOINTER)&m_cb);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLPOINTER)&m_cb);
			}
			else
			{
				SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, m_sqlCType, (SQLPOINTER*)m_pBuffer, GetBufferLength(), &m_cb);
				THROW_IFN_SUCCESS(SQLBindCol, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());
			}
			
			// get a notification if unbound
			m_unbindColumnsConnections[pHStmt] = pHStmt->ConnectUnbindColumnsSignal(boost::bind(&SqlCPointerBuffer::OnUnbindColumns, this, _1));
		}

		void BindParameter(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, bool useSqlDescribeParam = true)
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
			SQLRETURN ret = SQLBindParameter(pHStmt->GetHandle(), paramNr, SQL_PARAM_INPUT, m_sqlCType, paramSqlType, paramCharSize, paramDecimalDigits, (SQLPOINTER*)m_pBuffer, GetBufferLength(), &m_cb);
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

			// Connect a signal that we are bound to this handle now and get notified if params get reseted
			m_resetParamsConnections[pHStmt] = pHStmt->ConnectResetParamsSignal(boost::bind(&SqlCPointerBuffer::OnResetParams, this, _1));
		}


		const std::wstring& GetQueryName() const noexcept { return m_queryName; };


	private:
		SQLPOINTER m_pBuffer;
		SQLSMALLINT m_sqlCType;
		SQLLEN m_bufferLength;

		std::wstring m_queryName;
	};

	typedef std::shared_ptr<SqlCPointerBuffer> SqlCPointerBufferPtr;
	

	// the variant
	typedef boost::variant<
		UShortColumnBufferPtr, ShortColumnBufferPtr, ULongColumnBufferPtr, LongColumnBufferPtr, UBigIntColumnBufferPtr, BigIntColumnBufferPtr,
		TypeTimeColumnBufferPtr, TimeColumnBufferPtr,
		TypeDateColumnBufferPtr, DateColumnBufferPtr,
		TypeTimestampColumnBufferPtr, TimestampColumnBufferPtr,
		NumericColumnBufferPtr,
		DoubleColumnBufferPtr, RealColumnBufferPtr,
		WCharColumnBufferPtr, CharColumnBufferPtr,
		BinaryColumnBufferPtr,
		SqlCPointerBufferPtr
	> ColumnBufferPtrVariant;
	
	typedef std::map<SQLUSMALLINT, ColumnBufferPtrVariant> ColumnBufferPtrVariantMap;

	extern EXODBCAPI ColumnBufferPtrVariant CreateColumnBufferPtr(SQLSMALLINT sqlCType, const std::wstring& queryName);
	extern EXODBCAPI ColumnBufferPtrVariant CreateColumnArrayBufferPtr(SQLSMALLINT sqlCType, const std::wstring& queryName, const ColumnInfo& columnInfo);
	extern EXODBCAPI SQLLEN CalculateDisplaySize(SQLSMALLINT sqlType, SQLINTEGER columnSize, SQLSMALLINT numPrecRadix, SQLSMALLINT decimalDigits);
	extern EXODBCAPI bool IsArrayType(SQLSMALLINT sqlCType);

} // namespace exodbc
