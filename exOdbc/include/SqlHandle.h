/*!
* \file SqlHandle.h
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2015
* \brief Header file for the SqlHandle classes.
* \copyright GNU Lesser General Public License Version 3
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "Exception.h"

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
	* \class SqlHandle
	*
	* \brief A wrapper around a SQLHANDLE. Handle gets freed on destruction.
	*/
	template<typename THANDLE, SQLSMALLINT tHandleType, typename PHANDLE, SQLSMALLINT pHandleType>
	class SqlHandle
	{
	public:
		/*!
		* \brief	Constructs an SQL_NULL_HANDLE. Call Allocate() later to allocate the handle.
		* \see		Allocate()
		*/
		SqlHandle()
			: m_handle(SQL_NULL_HANDLE)
		{};


		/*!
		* \brief	Constructs an handle of typename THANDLE, using PHANDLE as parent handle,
		*			by calling Allocate.
		* \see		Allocate()
		* \throw	Exception
		*/
		SqlHandle(PHANDLE parentHandle)
			: m_handle(SQL_NULL_HANDLE)
		{
			Allocate(parentHandle);
		};

		SqlHandle(const SqlHandle& other) = delete;
		SqlHandle& operator=(const SqlHandle& other) = delete;


		/*!
		* \brief	If the internal tHandleType is not a SQL_NULL_HANDLE,
		*			Free() is called. 
		*/
		~SqlHandle()
		{
			try
			{
				if (IsAllocated())
				{
					Free();
				}
			}
			catch (const Exception& ex)
			{
				LOG_ERROR(ex.ToString());
			}
		}


		/*!
		* \brief	Allocates a handle of type tHandleType, using PHANDLE as parent handle.
		*			The allocated tHandleType is stored internally ("the handle") for later use.
		*			If the tHandleType of this is SQL_HANDLE_ENV, parentHandle must be a
		*			SQL_NULL_HANDLE.
		* \see		Allocate()
		* \throw	AssertionException If this has already an allocated handle
		* \throw	SqlResultException If allocating fails.
		*/
		void Allocate(PHANDLE parentHandle)
		{
			// Allowed only if not already allocated
			exASSERT(m_handle == SQL_NULL_HANDLE);

			// The environment handle has no parent handle
			if(tHandleType == SQL_HANDLE_ENV)
			{
				exASSERT(parentHandle == SQL_NULL_HANDLE);
				SQLRETURN  = SQLAllocHandle(tHandleType, SQL_NULL_HANDLE, &m_handle);
				if (!SQL_SUCCEEDED(ret))
				{
					SqlResultException ex(L"SQLAllocHandle", ret, L"Failed to allocated ODBC-Env Handle, no additional error information is available.");
					SET_EXCEPTION_SOURCE(ex);
					throw ex;
				}
			}
			else
			{
				exASSERT(parentHandle != SQL_NULL_HANDLE);
				SQLRETURN ret = SQLAllocHandle(tHandleType, parentHandle, &m_handle);
				THROW_IFN_SUCCEEDED(SQLAllocHandle, ret, pHandleType, parentHandle);
			}
		}

		
		/*!
		* \brief	Frees the internal tHandleType. Sets the internal handle to
		*			SQL_NULL_HANDLE upon success.
		* \throw	AssertionException If no handle is allocated.
		* \throw	SqlResultException If freeing fails.
		*/
		void Free()
		{
			exASSERT(m_handle != SQL_NULL_HANDLE);

			// Returns only SQL_SUCCESS, SQL_ERROR, or SQL_INVALID_HANDLE.
			SQLRETURN ret = SQLFreeHandle(tHandleType, m_handle);

			// if SQL_ERROR is returned, the handle is still valid, and error information can be fetched
			if (ret == SQL_ERROR)
			{
				std::wstring msg = boost::str(boost::wformat(L"Freeing Handle %1% of type %2% failed with SQL_ERROR, handle is still valid.") % m_handle %HandleType2s(tHandleType));
				SqlResultException ex(L"SQLFreeHandle", ret, SQL_HANDLE_DBC, m_hdbc, msg);
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			else if (ret == SQL_INVALID_HANDLE)
			{
				// If we've received INVALID_HANDLE our handle has probably already be deleted - anyway, its invalid, reset it.
				std::wstring msg = boost::str(boost::wformat(L"Freeing Handle %1% of type %2% failed with SQL_INVALID_HANDLE.") % m_handle %HandleType2s(tHandleType));
				m_handle = SQL_NULL_HANDLE
				SqlResultException ex(L"SQLFreeHandle", ret, msg);
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}

			// successfully freed
			m_handle = SQL_NULL_HANDLE;
		}


		/*!
		* \brief	Get the handle. Throws if IsAllocated() returns false.
		* \throw	AssertionException If no handle is allocated.
		*/
		THANDLE Get() const { exASSERT(IsAllocated()); m_handle; };

		
		/*!
		* \brief	Returns true if a handle is allocated.
		*/
		bool IsAllocated() const noexcept { return m_handle != SQL_NULL_HANDLE; };

	private:
		THANDLE m_handle;
	};

	/** Environment-handle */
	typedef SqlHandle<SQLHENV, SQL_HANDLE_ENV, SQLHENV, NULL> SqlEnvHandle;
	
	/** Environment-handle SharedPtr */
	typedef std::shared_ptr<SqlEnvHandle> SqlEnvHandlePtr;

	/** DatabaseConnection-handle */
	typedef SqlHandle<SQLHDBC, SQL_HANDLE_DBC, SQLHENV, SQL_HANDLE_ENV> SqlDbcHandle;

	/** DatabaseConnection-handle SharedPtr */
	typedef std::shared_ptr<SqlDbcHandle> SqlDbcHandlePtr;

	/** Statement-handle*/
	typedef SqlHandle<SQLHSTMT, SQL_HANDLE_STMT, SQLHDBC, SQL_HANDLE_DBC> SqlStmtHandle;

	/** Statement-handle SharedPtr */
	typedef std::shared_ptr<SqlStmtHandle> SqlStmtHandlePtr;


} // namespace exodbc

