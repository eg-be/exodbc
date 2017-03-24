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
#include "SpecializedExceptions.h"
#include "InfoObject.h"
#include "EnumFlags.h"
#include "SqlHandle.h"
#include "Helpers.h"
#include "LogManagerOdbcMacros.h"

// Other headers
#include <boost/variant.hpp>
#include <boost/signals2.hpp>

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
	
	/*!
	* \class	LengthIndicator
	* \brief	Wraps around a SQLLEN member to manage the length indicator of a ColumnBuffer.
	* \details	Cannot be copied.
	*/
	class LengthIndicator
	{
	public:
		/*!
		* \brief	Create new Instance, where Length Indicator is set to 0.
		*/
		LengthIndicator()
		{
			m_cb = 0;
		}

		LengthIndicator(const LengthIndicator& other) = delete;
		LengthIndicator& operator=(const LengthIndicator& other) = delete;

		virtual ~LengthIndicator()
		{};
		
		/*!
		* \brief	Set length indicator to passed value.
		*/
		void SetCb(SQLLEN cb) noexcept { m_cb = cb; };
		
		/*!
		* \brief	Get length indicator value.
		*/
		SQLLEN GetCb() const noexcept { return m_cb; };
		
		/*!
		* \brief	Sets the length indicator to SQL_NULL_DATA
		*/
		void SetNull() noexcept { m_cb = SQL_NULL_DATA; };

		/*!
		* \brief	Returns true if length indicator is set to SQL_NULL_DATA
		*/
		bool IsNull() const noexcept { return m_cb == SQL_NULL_DATA; };

	protected:
		SQLLEN m_cb;
	};
	typedef std::shared_ptr<LengthIndicator> LengthIndicatorPtr;


	/*!
	* \class	ColumnProperties
	* \brief	Helper class to store various all properties of a ColumnBuffer that are not related to 
	*			the ColumnBuffers type T.
	* \details	Can store additional information about:
	*			- Column size
	*			- Decimal digits
	*			- Sql Type
	*			- Query name
	*/
	class ColumnProperties
	{
	public:

		/*!
		* \brief	Create new instance with passed values.
		*/
		ColumnProperties(SQLINTEGER columnSize, SQLSMALLINT decimalDigits, SQLSMALLINT sqlType, std::string queryName)
			: m_columnSize(columnSize)
			, m_decimalDigits(decimalDigits)
			, m_sqlType(sqlType)
			, m_queryName(queryName)
		{};

		/*!
		* \brief	Create new instance, column size and decimal digits are set to 0, SQL type is set to SQL_UNKNOWN_TYPE
		*/
		ColumnProperties()
			: m_columnSize(0)
			, m_decimalDigits(0)
			, m_sqlType(SQL_UNKNOWN_TYPE)
		{};

		virtual ~ColumnProperties()
		{};

		/*!
		* \brief	Set Column size to passed value.
		*/
		void SetColumnSize(SQLINTEGER columnSize) noexcept { m_columnSize = columnSize; };

		/*!
		* \brief Set Decimal digits to passed value.
		*/
		void SetDecimalDigits(SQLSMALLINT decimalDigits) noexcept { m_decimalDigits = decimalDigits; };

		/*!
		* \brief	Set SQL Type to passed value.
		*/
		void SetSqlType(SQLSMALLINT sqlType) noexcept { m_sqlType = sqlType; };

		/*!
		* \brief	Set the query name.
		*/
		void SetQueryName(const std::string& queryName) noexcept { m_queryName = queryName; };

		/*!
		* \brief	Returns Column Size.
		*/
		SQLINTEGER GetColumnSize() const noexcept { return m_columnSize; };

		/*!
		* \brief	Returns decimal digits.
		*/
		SQLSMALLINT GetDecimalDigits() const noexcept { return m_decimalDigits; };

		/*!
		* \brief Returns SQL Type.
		*/
		SQLSMALLINT GetSqlType() const noexcept { return m_sqlType; };

		/*!
		* \brief	Get query name.
		* \throw	AssertionException if no query name is set.
		* \see		HasQueryName()
		*/
		std::string GetQueryName() const { exASSERT(HasQueryName()); return m_queryName; };

		/*!
		* \brief	Returns true if a query name is set (is not empty).
		*/
		bool HasQueryName() const noexcept { return !m_queryName.empty(); };

	protected:
		SQLINTEGER m_columnSize;
		SQLSMALLINT m_decimalDigits;
		SQLSMALLINT m_sqlType;
		std::string m_queryName;
	};
	typedef std::shared_ptr<ColumnProperties> ColumnPropertiesPtr;


	/*!
	* \class	ColumnBindable
	* \brief	Provides helpers to bind a buffer to a column as Parameter or Result.
	*			Stores information on which statement handle a ColumnBuffer is bound to and
	*			releases binding upon destruction.
	* \details	Provides two protected maps indexed by a ConstSqlStmtHandlePtr with a value
	*			of a boost::signal2::connection. ColumnBuffers that are being bound to
	*			can register a signal on the SqlHandle object to be notified if the parameters
	*			and/or columns get unbound on the SqlHandle (connect them to OnResetParams() /
	*			OnUndindColumns(). 
	*
	*			If the ColumnBindInfoHolder gets destroyed, it will call the ResetParams() /
	*			UnbindColumns() function on all SqlHandle entries in its map, so that ColumnBuffers
	*			can subclass from this BindInfoHolder: If the ColumnBuffer gets destroyed it will
	*			unbind itself from the SqlHandles.
	*
	*			If the signal handlers are notified that a SqlHandle has unbound its parameters or
	*			columns, it will remove entries matching that SqlHandle from its internal maps.
	*			
	*			Cannot be copied.
	*/
	class ColumnBindable
	{
	public:
		ColumnBindable() = default;
		ColumnBindable(const ColumnBindable& other) = delete;
		ColumnBindable& operator=(const ColumnBindable& other) = delete;

		/*!
		* \brief	On destruction, call ResetParams() and UnbindColumns() on all SqlHandles 
		*			we are bound to as param or column
		*/
		virtual ~ColumnBindable()
		{
			// If we are still bound to columns or params, release the bindings and disconnect the signals
			for (auto it = m_resetParamsConnections.begin(); it != m_resetParamsConnections.end(); ++it)
			{
				// disconnect first, we know its being reseted and dont want to get signaled that we reset params now
				std::pair<boost::signals2::connection, ConstSqlStmtHandlePtr> p = it->second;
				p.first.disconnect();
				try
				{
					p.second->ResetParams();
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(ex.ToString());
				}
			}
			for (auto it = m_unbindColumnsConnections.begin(); it != m_unbindColumnsConnections.end(); ++it)
			{
				// disconnect first, we know its being unbound now.
				std::pair<boost::signals2::connection, ConstSqlStmtHandlePtr> p = it->second;
				p.first.disconnect();
				try
				{
					p.second->UnbindColumns();
				}
				catch (const Exception& ex)
				{
					LOG_ERROR(ex.ToString());
				}
			}
		};

		/*!
		* \brief	If notified that passed SqlStmtHandle has reseted its bound parameters,
		*			remove any entry about that SqlStmtHandle from internal map.
		*/
		void OnResetParams(const SqlStmtHandle& stmt)
		{
			// remove from our map, we are no longer interested in resetting params on destruction
			auto it = m_resetParamsConnections.find(stmt.GetHandle());
			if (it != m_resetParamsConnections.end())
			{
				it->second.first.disconnect();
				m_resetParamsConnections.erase(it);
			}
		}

		/*!
		* \brief	If notified that passed SqlStmtHandle has unbound its columns,
		*			remove any entry about that SqlStmtHandle from internal map.
		*/
		void OnUnbindColumns(const SqlStmtHandle& stmt)
		{
			// remove from our map, we are no longer interested in resetting params on destruction
			auto it = m_unbindColumnsConnections.find(stmt.GetHandle());
			if (it != m_unbindColumnsConnections.end())
			{
				it->second.first.disconnect();
				m_unbindColumnsConnections.erase(it);
			}
		}

	protected:
		/*!
		* \brief	Calls SqlBindCol to bind this ColumnBuffer to the result set of the passed statement handle.
		* \details	Connects a signal on the passed statement handle to be notified if the columns of the
		*			handle gets unbound.
		* \param	columnNr 1-indexed column of the result set.
		* \param	pHStmt	Statement handle to bind against.
		* \param	sqlCType SQL C Type of the buffer.
		* \param	pBuffer Buffer to bind against.
		* \param	bufferLen length of pBuffer in bytes.
		* \param	pCb Length indicator to bind against.
		*/
		void BindColumnImpl(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt, SQLSMALLINT sqlCType, SQLPOINTER pBuffer, SQLLEN bufferLen, SQLLEN* pCb)
		{
			exASSERT(columnNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			exASSERT(pBuffer != NULL);
			exASSERT(bufferLen > 0);
			exASSERT(pCb != NULL);

			SQLRETURN ret = SQLBindCol(pHStmt->GetHandle(), columnNr, sqlCType, pBuffer, bufferLen, pCb);
			THROW_IFN_SUCCESS(SQLBindCol, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());

			// get a notification if unbound
			if (m_unbindColumnsConnections.find(pHStmt->GetHandle()) == m_unbindColumnsConnections.end())
			{
				std::pair<boost::signals2::connection, ConstSqlStmtHandlePtr> p(pHStmt->ConnectUnbindColumnsSignal(boost::bind(&ColumnBindable::OnUnbindColumns, this, _1)), pHStmt);
				m_unbindColumnsConnections[pHStmt->GetHandle()] = p;
			}
		};


		/*!
		* \brief Binds passed buffer as parameter using information passed.
		*/
		void BindParamImpl(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, SQLSMALLINT sqlCType, SQLPOINTER pBuffer, SQLLEN bufferLen, SQLLEN* pCb, SParameterDescription paramDesc)
		{
			exASSERT(paramNr >= 1);
			exASSERT(pHStmt != NULL);
			exASSERT(pHStmt->IsAllocated());
			exASSERT(pBuffer != NULL);
			exASSERT(bufferLen > 0);
			exASSERT(pCb != NULL);
			exASSERT(paramDesc.m_sqlType != SQL_UNKNOWN_TYPE);

			// bind using the information passed
			SQLRETURN ret = SQLBindParameter(pHStmt->GetHandle(), paramNr, SQL_PARAM_INPUT, sqlCType, paramDesc.m_sqlType, paramDesc.m_charSize, paramDesc.m_decimalDigits, pBuffer, bufferLen, pCb);
			THROW_IFN_SUCCESS(SQLBindParameter, ret, SQL_HANDLE_STMT, pHStmt->GetHandle());

			// Connect a signal that we are bound to this handle now and get notified if params get reseted
			if (m_resetParamsConnections.find(pHStmt->GetHandle()) == m_resetParamsConnections.end())
			{
				std::pair<boost::signals2::connection, ConstSqlStmtHandlePtr> p(pHStmt->ConnectResetParamsSignal(boost::bind(&ColumnBindable::OnResetParams, this, _1)), pHStmt);
				m_resetParamsConnections[pHStmt->GetHandle()] = p;
			}
		}


	protected:
		std::map<SQLHSTMT, std::pair<boost::signals2::connection, ConstSqlStmtHandlePtr>> m_unbindColumnsConnections;
		std::map<SQLHSTMT, std::pair<boost::signals2::connection, ConstSqlStmtHandlePtr>> m_resetParamsConnections;
	};


	/*!
	* \class	ColumnBuffer
	* \brief	Manages a buffer that is used to exchange data between the ODBC driver and exOdbc.
	* \details	Cannot be copied and should probably be wrapped into a shared_ptr.
	*			A ColumnBuffer can allocate exactly one element of type T, or a series of Ts (array).
	*/
	template<typename T, SQLSMALLINT sqlCType, typename std::enable_if<!std::is_pointer<T>::value, T>::type* = nullptr>
	class ColumnBuffer
		: public LengthIndicator
		, public ColumnFlags
		, public ColumnProperties
		, public ColumnBindable
	{
	public:
		ColumnBuffer() = delete;

		/*!
		* \brief	Create a new buffer with one element of type T, passed queryName, sqlType and flags.
		*			The buffer is set to NULL.
		*/
		ColumnBuffer(const std::string& queryName, SQLSMALLINT sqlType, ColumnFlags flags = ColumnFlag::CF_NONE)
			: LengthIndicator()
			, ColumnFlags(flags)
			, ColumnProperties()
			, m_nrOfElements(1)
			, m_buffer(1)
		{
			SetNull();
			SetQueryName(queryName);
			SetSqlType(sqlType);
		};


		/*!
		* \brief	Create a new buffer with one element of type T, passed sqlType and flags.
		*			The buffer is set to NULL.
		*/
		ColumnBuffer(SQLSMALLINT sqlType, ColumnFlags flags = ColumnFlag::CF_NONE)
			: LengthIndicator()
			, ColumnFlags(flags)
			, ColumnProperties()
			, m_nrOfElements(1)
			, m_buffer(1)
		{
			SetNull();
			SetSqlType(sqlType);
		};


		/*!
		* \brief	Create a new array buffer with passed queryName, nrOfElements and flags.
		*			The buffer is set to NULL.
		*/
		ColumnBuffer(SQLLEN nrOfElements, const std::string& queryName, SQLSMALLINT sqlType, ColumnFlags flags = ColumnFlag::CF_NONE)
			: LengthIndicator()
			, ColumnFlags(flags)
			, ColumnProperties()
			, m_nrOfElements(nrOfElements)
		{
			exASSERT(m_nrOfElements > 0);
			m_buffer = std::vector<T>(m_nrOfElements);
			SetNull();
			SetQueryName(queryName);
			SetSqlType(sqlType);
		};


		/*!
		* \brief	Create a new array buffer with passed sqlType, nrOfElements and flags.
		*			The buffer is set to NULL.
		*/
		ColumnBuffer(SQLLEN nrOfElements, SQLSMALLINT sqlType, ColumnFlags flags = ColumnFlag::CF_NONE)
			: LengthIndicator()
			, ColumnFlags(flags)
			, ColumnProperties()
			, m_nrOfElements(nrOfElements)
		{
			exASSERT(m_nrOfElements > 0);
			m_buffer = std::vector<T>(m_nrOfElements);
			SetNull();
			SetSqlType(sqlType);
		};

		ColumnBuffer(const ColumnBuffer& other) = delete;
		ColumnBuffer& operator=(const ColumnBuffer& other) = delete;

		virtual ~ColumnBuffer() 
		{ };


		/*!
		* \brief: Create a new instance wrapped into a shared_ptr
		*/
		static std::shared_ptr<ColumnBuffer> Create(const std::string& queryName, SQLSMALLINT sqlType, ColumnFlags flags = ColumnFlag::CF_NONE)
		{
			return std::make_shared<ColumnBuffer>(queryName, sqlType, flags);
		}


		/*!
		* \brief: Create a new instance wrapped into a shared_ptr
		*/
		static std::shared_ptr<ColumnBuffer> Create(SQLSMALLINT sqlType, ColumnFlags flags = ColumnFlag::CF_NONE)
		{
			return std::make_shared<ColumnBuffer>(sqlType, flags);
		}


		/*!
		* \brief: Create a new array instance wrapped into a shared_ptr
		*/
		static std::shared_ptr<ColumnBuffer> Create(SQLLEN nrOfElements, const std::string& queryName, SQLSMALLINT sqlType, ColumnFlags flags = ColumnFlag::CF_NONE)
		{
			return std::make_shared<ColumnBuffer>(nrOfElements, queryName, sqlType, flags);
		}


		/*!
		* \brief: Create a new array instance wrapped into a shared_ptr
		*/
		static std::shared_ptr<ColumnBuffer> Create(SQLLEN nrOfElements, SQLSMALLINT sqlType, ColumnFlags flags = ColumnFlag::CF_NONE)
		{
			return std::make_shared<ColumnBuffer>(nrOfElements, sqlType, flags);
		}


		/*!
		* \brief Return the SQL C Type of this ColumnBuffer.
		*/
		static SQLSMALLINT GetSqlCType() noexcept { return sqlCType; };

		/*!
		* \brief	Get the length in bytes of the internal buffer. This is the size of
		*			a single element multiplied with the number of elements.
		*/
		SQLLEN GetBufferLength() const noexcept { return sizeof(T) * GetNrOfElements(); };

		/*!
		* \brief	Set a binary value of the buffer. The length indicator is set to the size() of
		*			the vector passed.
		* \see		SetValue(const std::vector<T>& value, SQLLEN cb)
		*/
		void SetValue(const std::vector<SQLCHAR>& value)
		{
			SetValue(value, value.size());
		}

		/*!
		* \brief	Set the value of the array buffer. 
		* \details	internal buffer is filled with the elements of value. 
		*			If value contains less elements than the internal buffer, only the 
		*			number of elements passed are set in the internal buffer (beginning from
		*			0). The other elements are not modified.
		*			Length indicator is set to passed value.
		*			If cb is set SQL_NTS and the last element of values is not a '0',
		*			and if there is space available in the internal buffer, a terminating '0' is
		*			added.
		* \throw	Exception If value.size() is greated than the capacity of the buffer.
		* \throw	AssertionException if cb is set to SQL_NTS but there is not enough space
		*			in the internal buffer to null terminate and the data passed in value
		*			is not already null-terminated.
		*/
		void SetValue(const std::vector<T>& value, SQLLEN cb)
		{ 
			exASSERT_MSG(value.size() <= m_buffer.capacity(), u8"Passed value exceeds size of buffer allocated.");
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
				exASSERT_MSG(index < m_buffer.capacity(), u8"Not enough space to null-terminate the buffer.");
				m_buffer[index] = 0;
			}
			SetCb(cb);
		};

		/*!
		* \brief	Set the first element of the buffer to passed value. The length indicator is set to
		*			the length of one element of the buffer.
		*/
		void SetValue(const T& value) noexcept { SetValue(value, sizeof(T)); };

		/*!
		* \brief	Set the first element of the buffer and the length indicator to the passed values.
		*/
		void SetValue(const T& value, SQLLEN cb) noexcept { m_buffer[0] = value; SetCb(cb); };

		/*!
		* \brief	Return the value held by the first element of the buffer, if the buffer is not set to NULL.
		* \throw	NullValueException if buffer contains a NULL value.
		*/
		const T& GetValue() const { if (IsNull()) { NullValueException nve(GetQueryName()); SET_EXCEPTION_SOURCE(nve); throw nve; } return m_buffer[0]; };

		/*!
		* \brief	Wrapper for GetValue().
		* \see		GetValue()
		*/
		operator T() const noexcept { return GetValue(); };

		/*!
		* \brief	Get the array buffer.
		*/
		const std::vector<T>& GetBuffer() const noexcept { return m_buffer; };

		/*!
		* \brief	Get the number of elements T allocated.
		*/
		SQLLEN GetNrOfElements() const noexcept { return m_nrOfElements; };


		///*!
		//* \brief	Get the data of the buffer as std::wstring
		//* \throw	NullValueException if buffer is set to NULL.
		//*/
		template<class Q = T>
		typename std::enable_if<std::is_same<Q, SQLWCHAR>::value, std::wstring>::type GetWString() const
		{
			if (IsNull()) {
				NullValueException nve(GetQueryName());
				SET_EXCEPTION_SOURCE(nve);
				throw nve;
			}
			return reinterpret_cast<const wchar_t*>(m_buffer.data());
		};


		///*!
		//* \brief	Get the data of the buffer as std::string
		//* \throw	NullValueException if buffer is set to NULL.
		//*/
		template<class Q = T>
		typename std::enable_if<std::is_same<Q, SQLCHAR>::value, std::string>::type GetString() const
		{
			if (IsNull()) {
				NullValueException nve(GetQueryName());
				SET_EXCEPTION_SOURCE(nve);
				throw nve;
			}
			return reinterpret_cast<const char*>(m_buffer.data());
		};


		/*!
		* \brief	Set passed std::wstring as value on the buffer. Null terminates the buffer.
		*/
		void SetWString(const std::wstring& ws) { std::vector<SQLWCHAR> vec(ws.begin(), ws.end()); SetValue(vec, SQL_NTS); };

		/*!
		* \brief	Set passed std::string as value on the buffer. Null terminates the buffer.
		*/
		void SetString(const std::string& s) { std::vector<SQLCHAR> vec(s.begin(), s.end()); SetValue(vec, SQL_NTS); }

		/*!
		* \brief	Calls SqlBindCol to bind this ColumnBuffer to the result set of the passed statement handle.
		* \details	Connects a signal on the passed statement handle to be notified if the columns of the
		*			handle gets unbound.
		* \param	columnNr 1-indexed column of the result set.
		* \param	pHStmt	Statement handle to bind against.
		*/
		void BindColumn(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			BindColumnImpl(columnNr, pHStmt, sqlCType, (SQLPOINTER*)&m_buffer[0], GetBufferLength(), &m_cb);
		};

		/*!
		* \brief	Calls SqlBindParameter with SQL_PARAM_INPUT to bind the passed parameter as input parameter
		*			on the passed statement handle.
		* \details	Optionally queries the database about the parameter, using SQLDescribeParam.
		*			Connects a signal on the passed statement handle to be notified if the params of the
		*			handle get reseted.
		* \param	paramNr 1-indexed parameter of the statement to be executed.
		* \param	pHStmt Statement to bind against.
		* \param	paramDesc	Description of the parameter.
		*/
		void BindParameter(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, SParameterDescription paramDesc)
		{
			BindParamImpl(paramNr, pHStmt, sqlCType, (SQLPOINTER*)&m_buffer[0], GetBufferLength(), &m_cb, paramDesc);
		}


		/*!
		* \brief Create a SParameterDescription from the information in this ColumnBuffer
		*			without querying the database.
		*/
		SParameterDescription CreateParamDescFromProps()
		{
			SParameterDescription paramDesc;
			paramDesc.m_sqlType = GetSqlType();
			paramDesc.m_charSize = GetColumnSize();
			paramDesc.m_decimalDigits = GetDecimalDigits();
			if (Test(ColumnFlag::CF_NULLABLE))
			{
				paramDesc.m_nullable = SQL_NULLABLE;
			}
			return paramDesc;
		}


	private:
		SQLLEN m_nrOfElements;
		std::vector<T> m_buffer;
	};

	template<> inline
	void ColumnBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>::BindColumn(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
	{
		// Do this totally different than else. Does not work to first bind as usual and then set attrs... ?
		exASSERT(columnNr >= 1);
		exASSERT(pHStmt != NULL);
		exASSERT(pHStmt->IsAllocated());
		exASSERT(m_columnSize > 0);
		exASSERT(m_decimalDigits >= 0);

		SqlDescHandle hDesc(pHStmt, RowDescriptorType::ROW);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN)m_columnSize));
		SetDescriptionField(hDesc, columnNr, SQL_DESC_SCALE, (SQLPOINTER)((SQLLEN)m_decimalDigits));
		SetDescriptionField(hDesc, columnNr, SQL_DESC_DATA_PTR, (SQLPOINTER)&m_buffer[0]);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_INDICATOR_PTR, (SQLPOINTER)&m_cb);
		SetDescriptionField(hDesc, columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLPOINTER)&m_cb);

		// get a notification if unbound
		if (m_unbindColumnsConnections.find(pHStmt->GetHandle()) == m_unbindColumnsConnections.end())
		{
			std::pair<boost::signals2::connection, ConstSqlStmtHandlePtr> p(pHStmt->ConnectUnbindColumnsSignal(boost::bind(&ColumnBindable::OnUnbindColumns, this, _1)), pHStmt);
			m_unbindColumnsConnections[pHStmt->GetHandle()] = p;
		}
	}


	template<> inline
	void ColumnBuffer<SQL_NUMERIC_STRUCT, SQL_C_NUMERIC>::BindParameter(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, SParameterDescription paramDesc)
	{
		BindParamImpl(paramNr, pHStmt, SQL_C_NUMERIC, (SQLPOINTER*)&m_buffer, GetBufferLength(), &m_cb, paramDesc);

		// Do some additional steps for numeric types
		SqlDescHandle hDesc(pHStmt, RowDescriptorType::PARAM);
		SetDescriptionField(hDesc, paramNr, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC);
		SetDescriptionField(hDesc, paramNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN)paramDesc.m_charSize));
		SetDescriptionField(hDesc, paramNr, SQL_DESC_SCALE, (SQLPOINTER)((SQLLEN)paramDesc.m_decimalDigits));
		SetDescriptionField(hDesc, paramNr, SQL_DESC_DATA_PTR, (SQLPOINTER)&m_buffer[0]);
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

	// Array types
	typedef ColumnBuffer<SQLWCHAR, SQL_C_WCHAR> WCharColumnBuffer;
	typedef std::shared_ptr<WCharColumnBuffer> WCharColumnBufferPtr;
	typedef ColumnBuffer<SQLCHAR, SQL_C_CHAR> CharColumnBuffer;
	typedef std::shared_ptr<CharColumnBuffer> CharColumnBufferPtr;
	typedef ColumnBuffer<SQLCHAR, SQL_C_BINARY> BinaryColumnBuffer;
	typedef std::shared_ptr<BinaryColumnBuffer> BinaryColumnBufferPtr;

	/*!
	* \class	SqlCPointerBuffer
	* \brief	Class to hold any user created (and managed) buffer that is given as a raw SQLPOINTER.
	* \details	Basically allows any user-defined type to be used as a buffer. This buffer is not freed
	*			on destruction.
	*/
	class SqlCPointerBuffer
		: public LengthIndicator
		, public ColumnFlags
		, public ColumnProperties
		, public ColumnBindable
	{
	public:
		SqlCPointerBuffer() = delete;

		/*!
		* \brief Create a new SqlCPointerBuffer.
		* \details Sets the buffer to NULL on construction.
		* \param queryName	Name to be used in queries.
		* \param sqlType	The SQL Type of the corresponding database column.
		* \param pBuffer	SQLPOINTER to a buffer already allocated. Do not free this buffer before this SqlCPointerBuffer
		*					is destroyed.
		* \param sqlCType	The SQL C Type of the buffer pointer to by pBuffer.
		* \param bufferLength	Size of pBuffer in bytes
		* \param flags		ColumnFlags of the column.
		* \param columnSize Column size.
		* \param decimalDigits Decimal digits.
		* \param nrOfElements Count how many elements of sqlType are allocated within pBuffer.
		*/
		SqlCPointerBuffer(const std::string& queryName, SQLSMALLINT sqlType, SQLPOINTER pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferLength, ColumnFlags flags, SQLINTEGER columnSize, SQLSMALLINT decimalDigits, SQLLEN nrOfElements = 1)
			: LengthIndicator()
			, ColumnFlags(flags)
			, ColumnProperties(columnSize, decimalDigits, sqlType, queryName)
			, m_pBuffer(pBuffer)
			, m_sqlCType(sqlCType)
			, m_bufferLength(bufferLength)
			, m_nrOfElements(nrOfElements)
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
		static std::shared_ptr<SqlCPointerBuffer> Create(const std::string& queryName, SQLSMALLINT sqlType, SQLPOINTER pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags, SQLINTEGER columnSize, SQLSMALLINT decimalDigits, SQLLEN nrOfElements = 1)
		{
			return std::make_shared<SqlCPointerBuffer>(queryName, sqlType, pBuffer, sqlCType, bufferSize, flags, columnSize, decimalDigits, nrOfElements);
		}

		/*!
		* \brief Return the SQL C Type of this ColumnBuffer.
		*/
		SQLSMALLINT GetSqlCType() const noexcept { return m_sqlCType; };

		/*!
		* \brief	Get the length in bytes of the internal buffer.
		*/
		SQLLEN GetBufferLength() const noexcept { return m_bufferLength; };


		/*!
		* \brief	Get the number of elements allocated.
		*/
		SQLLEN GetNrOfElements() const noexcept { return m_nrOfElements; };


		/*!
		* \brief	Calls SqlBindCol to bind this ColumnBuffer to the result set of the passed statement handle.
		* \details	Connects a signal on the passed statement handle to be notified if the columns of the
		*			handle gets unbound.
		*
		*			If the SQL C Type is SQL_C_NUMERIC, binding is done using the description fields to properly set
		*			DESC_PRESISION (passed Column Size) and DESC_SCALE (passed Decimal Digits).
		* \param	columnNr 1-indexed column of the result set.
		* \param	pHStmt	Statement handle to bind against.
		*/
		void BindColumn(SQLUSMALLINT columnNr, ConstSqlStmtHandlePtr pHStmt)
		{
			if (m_sqlCType == SQL_C_NUMERIC)
			{
				// totally different than usually. do not forget to register unbind notification
				exASSERT(columnNr >= 1);
				exASSERT(pHStmt != NULL);
				exASSERT(pHStmt->IsAllocated());
				exASSERT(m_columnSize >= 0);
				exASSERT(m_decimalDigits >= 0);

				SqlDescHandle hDesc(pHStmt, RowDescriptorType::ROW);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_TYPE, (SQLPOINTER)((SQLLEN)m_sqlCType));
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN)m_columnSize));
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_SCALE, (SQLPOINTER)((SQLLEN)m_decimalDigits));
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_DATA_PTR, (SQLPOINTER)m_pBuffer);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_INDICATOR_PTR, (SQLPOINTER)&m_cb);
				SetDescriptionField(hDesc.GetHandle(), columnNr, SQL_DESC_OCTET_LENGTH_PTR, (SQLPOINTER)&m_cb);

				// get a notification if unbound
				if (m_unbindColumnsConnections.find(pHStmt->GetHandle()) == m_unbindColumnsConnections.end())
				{
					std::pair<boost::signals2::connection, ConstSqlStmtHandlePtr> p(pHStmt->ConnectUnbindColumnsSignal(boost::bind(&SqlCPointerBuffer::OnUnbindColumns, this, _1)), pHStmt);
					m_unbindColumnsConnections[pHStmt->GetHandle()] = p;
				}
			}
			else
			{
				// Bind using base-class
				BindColumnImpl(columnNr, pHStmt, m_sqlCType, m_pBuffer, GetBufferLength(), &m_cb);
			}

		}

		/*!
		* \brief	Calls SqlBindParameter with SQL_PARAM_INPUT to bind the passed parameter as input parameter
		*			on the passed statement handle.
		* \details	Optionally queries the database about the parameter, using SQLDescribeParam.
		*			Connects a signal on the passed statement handle to be notified if the params of the
		*			handle get reseted.
		*
		*			If the SQL C Type is SQL_C_NUMERIC, binding is done using the description fields to properly set
		*			DESC_PRESISION (passed Column Size) and DESC_SCALE (passed Decimal Digits).
		* \param	paramNr 1-indexed parameter of the statement to be executed.
		* \param	pHStmt Statement to bind against.
		* \param	paramDesc	Description of the parameter.
		*/
		void BindParameter(SQLUSMALLINT paramNr, ConstSqlStmtHandlePtr pHStmt, SParameterDescription paramDesc)
		{
			BindParamImpl(paramNr, pHStmt, GetSqlCType(), m_pBuffer, GetBufferLength(), &m_cb, paramDesc);

			if (m_sqlCType == SQL_C_NUMERIC)
			{
				// Do some additional steps for numeric types
				SqlDescHandle hDesc(pHStmt, RowDescriptorType::PARAM);
				SetDescriptionField(hDesc, paramNr, SQL_DESC_TYPE, (SQLPOINTER)SQL_C_NUMERIC);
				SetDescriptionField(hDesc, paramNr, SQL_DESC_PRECISION, (SQLPOINTER)((SQLLEN)paramDesc.m_charSize));
				SetDescriptionField(hDesc, paramNr, SQL_DESC_SCALE, (SQLPOINTER)((SQLLEN)paramDesc.m_decimalDigits));
				SetDescriptionField(hDesc, paramNr, SQL_DESC_DATA_PTR, m_pBuffer);
			}
		}


		/*!
		* \brief Create a SParameterDescription from the information in this SqlCPointerBuffer
		*			without querying the database.
		*/
		SParameterDescription CreateParamDescFromProps()
		{
			SParameterDescription paramDesc;
			paramDesc.m_sqlType = GetSqlType();
			paramDesc.m_charSize = GetColumnSize();
			paramDesc.m_decimalDigits = GetDecimalDigits();
			if (Test(ColumnFlag::CF_NULLABLE))
			{
				paramDesc.m_nullable = SQL_NULLABLE;
			}
			return paramDesc;
		}


	private:
		SQLPOINTER m_pBuffer;
		SQLSMALLINT m_sqlCType;
		SQLLEN m_bufferLength;
		SQLLEN m_nrOfElements;
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

	/*!
	* \brief Depending on passed SQL C Type, a corresponding ColumnBuffer is created, wrapped into a shared_ptr and returned as variant.
	*/
	extern EXODBCAPI ColumnBufferPtrVariant CreateColumnBufferPtr(SQLSMALLINT sqlCType, const std::string& queryName);

	/*!
	* \brief Depending on passed SQL C Type, a corresponding ColumnBuffer is created, wrapped into a shared_ptr and returned as variant.
	*/
	extern EXODBCAPI ColumnBufferPtrVariant CreateColumnArrayBufferPtr(SQLSMALLINT sqlCType, const std::string& queryName, const ColumnInfo& columnInfo);
	
	/*!
	* \brief	Calculate the "DisplaySize" of a given column: This is the number of characters required if the column
	*			is bound to a SQL C character type.
	* \details	See https://msdn.microsoft.com/en-us/library/ms713974%28v=vs.85%29.aspx for the details.
	*/
	extern EXODBCAPI SQLLEN CalculateDisplaySize(SQLSMALLINT sqlType, SQLINTEGER columnSize, SQLSMALLINT numPrecRadix, SQLSMALLINT decimalDigits);
	
	/*!
	* \brief	Returns true if the passed SQL C Type should be used together with an ColumnBuffer that contains multiple elements.
	* \details	Returns true for:
	*			- SQL_C_CHAR
	*			- SQL_C_WCHAR
	*			- SQL_C_BINARY
	*/
	extern EXODBCAPI bool IsArrayType(SQLSMALLINT sqlCType);

} // namespace exodbc
