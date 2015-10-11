/*!
* \file CBufferType.h
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

	template<typename T, SQLSMALLINT sqlCType , typename std::enable_if<!std::is_pointer<T>::value, T>::type* = 0>
	class SqlCBuffer
	{
	public:
		SqlCBuffer()
		{
			//typename std::enable_if<!std::is_pointer<T>::value, T>::type arg

			//static_assert(!std::is_pointer<T>::value,
			//	"The argument to func must not be a pointer.");
			ZeroMemory(&m_buffer, GetBufferSize());
		};

		~SqlCBuffer() {};

		static SQLSMALLINT GetSqlCType() throw() { return sqlCType; };
		void SetValue(const T& value) throw() { m_buffer = value; };
		T GetValue() const throw() { return m_buffer; };
		SQLLEN GetBufferSize() const throw() { return sizeof(m_buffer); };

	private:
		T m_buffer;
	};

	typedef SqlCBuffer<SQLSMALLINT, SQL_C_SSHORT> SqlSmallIntBuffer;
	typedef SqlCBuffer<SQLINTEGER, SQL_C_SLONG> SqlIntBuffer;
	typedef SqlCBuffer<SQLBIGINT, SQL_C_SBIGINT> SqlBigIntBuffer;
	typedef SqlCBuffer<SQL_TIME_STRUCT, SQL_C_TYPE_TIME> SqlTimeTypeStructBuffer;
	typedef SqlCBuffer<SQL_TIME_STRUCT, SQL_C_TIME> SqlTimeStructBuffer;

	template<typename T, SQLSMALLINT sqlCType, typename std::enable_if<!std::is_pointer<T>::value, T>::type* = 0>
	class SqlCArrayBuffer
	{
	private:
		SqlCArrayBuffer() 
		{ 
			exASSERT(false); 
		};

	public:
		SqlCArrayBuffer(SQLLEN nrOfElements)
			: m_buffer(nrOfElements)
		{
			ZeroMemory((void*) m_buffer.data(), GetBufferSize());
		};

		~SqlCArrayBuffer() 
		{};

		static SQLSMALLINT GetSqlCType() throw() { return sqlCType; };
		void SetValue(const T* value, SQLLEN valueSize) throw() { exASSERT(valueSize == GetBufferSize()); memcpy(m_buffer.data(), value, GetBufferSize()); };
		void SetValue(const std::vector<T>& value) { exASSERT(value.size() == m_buffer.size()); m_buffer = value; };
		const T* GetValue(SQLLEN& bufferSize) const throw() { bufferSize = GetBufferSize(); return m_buffer.data(); };
		SQLLEN GetBufferSize() const throw() { return sizeof(T) * GetNrOfElements(); };
		SQLLEN GetNrOfElements() const throw() { return m_buffer.size(); };

	private:
		std::vector<T> m_buffer;
	};
	typedef SqlCArrayBuffer<SQLWCHAR, SQL_C_WCHAR> SqlWCharArray;
	typedef SqlCArrayBuffer<SQLCHAR, SQL_C_CHAR> SqlCharArray;
	typedef SqlCArrayBuffer<SQLCHAR, SQL_C_BINARY> SqlBinaryArray;


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
