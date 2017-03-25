/*!
* \file ColumnBufferWrapper.h
* \author Elias Gerber <eg@elisium.ch>
* \date 25.03.2017
* \brief Header file for the ColumnBufferWrapper class.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "ColumnBuffer.h"
#include "ColumnBufferVisitors.h"

// Other headers

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
	* \class	ColumnBufferWrapper
	* \brief	A convenience wrapper to read some properties directly from a
	*			ColumnBufferPtrVariant.
	*/
	class EXODBCAPI ColumnBufferWrapper
	{
	public:
		ColumnBufferWrapper() = delete;

		ColumnBufferWrapper(const ColumnBufferPtrVariant& variant)
			: m_columnBufferVariant(variant)
		{};

		ColumnBufferWrapper(const ColumnBufferWrapper& other)
			: m_columnBufferVariant(other.m_columnBufferVariant)
		{};

		virtual ~ColumnBufferWrapper() 
		{};

		/*!
		* \brief Return true if ColumnBufferPtr is null.
		*/
		bool IsNull() const;


		/*!
		* \brief Set ColumnBufferPtr to NULL
		* \throw NotAllowedException If ColumnFlag::CF_NULLABLE is not set.
		*/
		void SetNull();


		/*!
		* \brief Get the SQL C Type of the ColumnBufferPtr.
		*/
		SQLSMALLINT GetSqlCType() const;

	protected:
		ColumnBufferPtrVariant m_columnBufferVariant;
	};


	/*!
	* \class	StringColumnWrapper
	* \brief	A convenience wrapper to set / get string values, without having to test for
	*			CHAR or WCHAR. Will only work for ColumnBuffer instances of types
	*			SQL_C_CHAR or SQL_C_WCHAR (asserts on Construction).
	*/
	class EXODBCAPI StringColumnWrapper
		: public ColumnBufferWrapper
	{
	public:
		StringColumnWrapper() = delete;

		StringColumnWrapper(const ColumnBufferPtrVariant& variant)
			: ColumnBufferWrapper(variant)
		{
			exASSERT(GetSqlCType() == SQL_C_CHAR || GetSqlCType() == SQL_C_WCHAR);
		};

		StringColumnWrapper(const StringColumnWrapper& other)
			: ColumnBufferWrapper(other)
		{};


		template<typename TDataType>
		typename std::enable_if<std::is_same<std::string, TDataType>::value ||
			std::is_same<char const *, TDataType>::value,
			TDataType>::type
			GetValue()
		{
			SQLSMALLINT sqlCType = GetSqlCType();

			try
			{
				if (sqlCType == SQL_C_CHAR)
				{
					CharColumnBufferPtr pCharColumn = boost::get<CharColumnBufferPtr>(m_columnBufferVariant);
					return pCharColumn->GetString();
				}
				else if (sqlCType == SQL_C_WCHAR)
				{
					WCharColumnBufferPtr pWCharColumn = boost::get<WCharColumnBufferPtr>(m_columnBufferVariant);
					return utf16ToUtf8(pWCharColumn->GetWString());
				}
				else
				{
					exASSERT(false);
					return u8""; // make compiler happy
				}
			}
			catch (const boost::bad_get& ex)
			{
				WrapperException we(ex);
				SET_EXCEPTION_SOURCE(we);
				throw we;
			}
		}


		template<typename TDataType>
		typename std::enable_if<std::is_same<std::wstring, TDataType>::value ||
			std::is_same<wchar_t const *, TDataType>::value,
			TDataType>::type
			GetValue()
		{
			SQLSMALLINT sqlCType = GetSqlCType();

			try
			{
				if (sqlCType == SQL_C_CHAR)
				{
					CharColumnBufferPtr pCharColumn = boost::get<CharColumnBufferPtr>(m_columnBufferVariant);
					return utf8ToUtf16(pCharColumn->GetString());
				}
				else if (sqlCType == SQL_C_WCHAR)
				{
					WCharColumnBufferPtr pWCharColumn = boost::get<WCharColumnBufferPtr>(m_columnBufferVariant);
					return pWCharColumn->GetWString();
				}
				else
				{
					exASSERT(false);
					return L""; // Make compiler happy
				}
			}
			catch (const boost::bad_get& ex)
			{
				WrapperException we(ex);
				SET_EXCEPTION_SOURCE(we);
				throw we;
			}
		}


		template<typename TDataType>
		typename std::enable_if<std::is_same<std::string, TDataType>::value ||
			std::is_same<char const *, TDataType>::value,
			void>::type
			SetValue(TDataType data)
		{
			SQLSMALLINT sqlCType = GetSqlCType();

			try
			{
				if (sqlCType == SQL_C_CHAR)
				{
					CharColumnBufferPtr pCharColumn = boost::get<CharColumnBufferPtr>(m_columnBufferVariant);
					pCharColumn->SetString(data);
				}
				else if (sqlCType == SQL_C_WCHAR)
				{
					WCharColumnBufferPtr pWCharColumn = boost::get<WCharColumnBufferPtr>(m_columnBufferVariant);
					pWCharColumn->SetWString(utf8ToUtf16(data));
				}
				else
				{
					exASSERT(false);
				}
			}
			catch (const boost::bad_get& ex)
			{
				WrapperException we(ex);
				SET_EXCEPTION_SOURCE(we);
				throw we;
			}
		}


		template<typename TDataType>
		typename std::enable_if<std::is_same<std::wstring, TDataType>::value ||
			std::is_same<wchar_t const *, TDataType>::value,
			void>::type
			SetValue(TDataType data)
		{
			SQLSMALLINT sqlCType = GetSqlCType();

			try
			{
				if (sqlCType == SQL_C_CHAR)
				{
					CharColumnBufferPtr pCharColumn = boost::get<CharColumnBufferPtr>(m_columnBufferVariant);
					pCharColumn->SetString(utf16ToUtf8(data));
				}
				else if (sqlCType == SQL_C_WCHAR)
				{
					WCharColumnBufferPtr pWCharColumn = boost::get<WCharColumnBufferPtr>(m_columnBufferVariant);
					pWCharColumn->SetWString(data);
				}
				else
				{
					exASSERT(false);
				}
			}
			catch (const boost::bad_get& ex)
			{
				WrapperException we(ex);
				SET_EXCEPTION_SOURCE(we);
				throw we;
			}
		}
	};

} // namespace exodbc
