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
	template<typename THANDLE, SQLSMALLINT tHandleType, typename TPARENTSQLHANDLE>
	class SqlHandle
	{
	public:
		/*!
		* \brief	Constructs an SQL_NULL_HANDLE. Call Allocate() or AllocateWithParent()
		*			later to allocate the handle.
		* \see		Allocate()
		* \see		AllocateWithParent()
		*/
		SqlHandle()
			: m_handle(SQL_NULL_HANDLE)
			, m_pParentHandle(NULL)
		{};


		/*!
		* \brief	Constructs a new handle using the passed handle as parent
		*			by calling AllocateWithParent() uppon construction.
		* \see		AllocateWithParent()
		*/
		SqlHandle(std::shared_ptr<const TPARENTSQLHANDLE> pParentHandle)
			: m_handle(SQL_NULL_HANDLE)
			, m_pParentHandle(NULL)
		{
			AllocateWithParent(pParentHandle);
		}

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

		void Allocate()
		{
			// Allowed only if not already allocated
			exASSERT(m_handle == SQL_NULL_HANDLE);
			exASSERT(m_pParentHandle == NULL);

			// This shall only be allowed for the environment handle so far. All others have a parent
			exASSERT_MSG(tHandleType == SQL_HANDLE_ENV, L"Only handles of type SQL_HANDLE_ENV can be Allocated without parent");
			SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_handle);
			if (!SQL_SUCCEEDED(ret))
			{
				SqlResultException ex(L"SQLAllocHandle", ret, L"Failed to allocated ODBC-Env Handle, no additional error information is available.");
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
		}


		/*!
		* \brief	Allocates a handle of type tHandleType, using pParentHandle as parent.
		*			The allocated tHandleType is stored internally ("the handle") for later use.
		*			If the tHandleType of this is SQL_HANDLE_ENV, parentHandle must be a
		*			NULL pointer.
		*			If this function succeeds, IsAllocated() returns true afterwards.
		* \see		Allocate()
		* \throw	AssertionException If this has already an allocated handle
		* \throw	SqlResultException If allocating fails.
		*/
		void AllocateWithParent(std::shared_ptr<const TPARENTSQLHANDLE> pParentHandle)
		{
			exASSERT(pParentHandle != NULL);
			exASSERT(pParentHandle->IsAllocated());

			// Allowed only if not already allocated
			exASSERT(m_handle == SQL_NULL_HANDLE);
			exASSERT(m_pParentHandle == NULL);

			// The environment handle has no parent handle
			exASSERT_MSG(tHandleType != SQL_HANDLE_ENV, L"Handles of type SQL_HANDLE_ENV must be allocated without parent");
			SQLRETURN ret = SQLAllocHandle(tHandleType, pParentHandle->GetHandle(), &m_handle);
			THROW_IFN_SUCCEEDED(SQLAllocHandle, ret, pParentHandle->GetHandleType(), pParentHandle->GetHandle());
			// success, remember parent
			m_pParentHandle = pParentHandle;
		}

		
		/*!
		* \brief	Frees the internal tHandleType. Sets the internal handle to
		*			SQL_NULL_HANDLE upon success, and resets the interanl 
		*			shared_ptr to the parent handle.
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
				SqlResultException ex(L"SQLFreeHandle", ret, tHandleType, m_handle, msg);
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			else if (ret == SQL_INVALID_HANDLE)
			{
				// If we've received INVALID_HANDLE our handle has probably already be deleted - anyway, its invalid, reset it.
				std::wstring msg = boost::str(boost::wformat(L"Freeing Handle %1% of type %2% failed with SQL_INVALID_HANDLE.") % m_handle %HandleType2s(tHandleType));
				m_handle = SQL_NULL_HANDLE;
				m_pParentHandle.reset();
				SqlResultException ex(L"SQLFreeHandle", ret, msg);
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}

			// successfully freed
			m_handle = SQL_NULL_HANDLE;
			m_pParentHandle.reset();
		}


		/*!
		* \brief	Get the handle. Throws if IsAllocated() returns false.
		* \throw	AssertionException If no handle is allocated.
		*/
		THANDLE GetHandle() const { exASSERT(IsAllocated()); return m_handle; };
		
		/*!
		* \brief	Returns true if a handle is allocated.
		*/
		bool IsAllocated() const noexcept { return m_handle != SQL_NULL_HANDLE; };


		/*!
		* \brief	Returns the type of the Handle.
		*/
		SQLSMALLINT GetHandleType() const noexcept { return tHandleType; };

		bool operator==(const SqlHandle& other) const noexcept
		{
			return GetHandleType() == other.GetHandleType()
				&& m_handle == other.m_handle;
		};

		bool operator!=(const SqlHandle& other) const noexcept
		{
			return !(*this == other);
		};

		bool operator<(const SqlHandle& other) const noexcept
		{
			return GetHandleType() < other.GetHandleType() && m_handle < other.m_handle;
		}

	private:
		THANDLE m_handle;
		std::shared_ptr<const TPARENTSQLHANDLE> m_pParentHandle;
	};

	/** Environment-handle */
	typedef SqlHandle<SQLHENV, SQL_HANDLE_ENV, SQLHANDLE> SqlEnvHandle;
	/** Environment-handle SharedPtr */
	typedef std::shared_ptr<SqlEnvHandle> SqlEnvHandlePtr;
	/** Environment-handle Const SharedPtr */
	typedef std::shared_ptr<const SqlEnvHandle> ConstSqlEnvHandlePtr;

	/** DatabaseConnection-handle */
	typedef SqlHandle<SQLHDBC, SQL_HANDLE_DBC, SqlEnvHandle> SqlDbcHandle;
	/** DatabaseConnection-handle SharedPtr */
	typedef std::shared_ptr<SqlDbcHandle> SqlDbcHandlePtr;
	/** DatabaseConnection-handle Const SharedPtr */
	typedef std::shared_ptr<const SqlDbcHandle> ConstSqlDbcHandlePtr;

	/** Statement-handle*/
	typedef SqlHandle<SQLHSTMT, SQL_HANDLE_STMT, SqlDbcHandle> SqlStmtHandle;
	/** Statement-handle SharedPtr */
	typedef std::shared_ptr<SqlStmtHandle> SqlStmtHandlePtr;
	/** Statement-handle Const SharedPtr */
	typedef std::shared_ptr<const SqlStmtHandle> ConstSqlStmtHandlePtr;

	// specialize for the Descriptor handle
	template<>
	class SqlHandle<SQLHDESC, SQL_HANDLE_DESC, SqlStmtHandle>
	{
	public:
		/*!
		* \brief	Creates a RowDescriptorHandle from the given Statement handle.
		* \see		Allocate()
		*/
		SqlHandle(ConstSqlStmtHandlePtr pStmt, RowDescriptorType type)
			: m_handle(SQL_NULL_HDESC)
			, m_pParentHandle(NULL)
		{
			AllocateWithParent(pStmt, type);
		};

		SqlHandle() = delete;
		SqlHandle(const SqlHandle& other) = delete;
		SqlHandle& operator=(const SqlHandle& other) = delete;

		/*!
		* \brief	Nothing to do on a desc-handle, deleted auto with its parent stmt-handle
		*/
		~SqlHandle()
		{ }

	private:
		void AllocateWithParent(ConstSqlStmtHandlePtr pStmtHandle, RowDescriptorType type)
		{
			exASSERT(pStmtHandle != NULL);
			exASSERT(pStmtHandle->IsAllocated());

			// Allowed only if not already allocated
			exASSERT(m_handle == SQL_NULL_HANDLE);
			exASSERT(m_pParentHandle == NULL);

			SQLRETURN ret = SQLGetStmtAttr(pStmtHandle->GetHandle(), (SQLINTEGER)type, &m_handle, 0, NULL);
			THROW_IFN_SUCCEEDED(SQLGetStmtAttr, ret, SQL_HANDLE_STMT, pStmtHandle->GetHandle());

			// success, remember parent
			m_pParentHandle = pStmtHandle;
		}

	public:
		/*!
		* \brief	Get the handle. Throws if IsAllocated() returns false.
		* \throw	AssertionException If no handle is allocated.
		*/
		SQLHDESC GetHandle() const { exASSERT(IsAllocated()); return m_handle; };

		/*!
		* \brief	Returns true if a handle is allocated.
		*/
		bool IsAllocated() const noexcept { return m_handle != SQL_NULL_HDESC; };


		/*!
		* \brief	Returns the type of the Handle.
		*/
		SQLSMALLINT GetHandleType() const noexcept { return SQL_HANDLE_DESC; };

		bool operator==(const SqlHandle& other) const noexcept
		{
			return GetHandleType() == other.GetHandleType()
				&& m_handle == other.m_handle;
		};

		bool operator!=(const SqlHandle& other) const noexcept
		{
			return !(*this == other);
		};

		bool operator<(const SqlHandle& other) const noexcept
		{
			return GetHandleType() < other.GetHandleType() && m_handle < other.m_handle;
		}

	private:
		SQLHDESC m_handle;
		ConstSqlStmtHandlePtr m_pParentHandle;
	};

	/** Descriptor-handle*/
	typedef SqlHandle<SQLHDESC, SQL_HANDLE_DESC, SqlStmtHandle> SqlDescHandle;
	/** Descriptor-handle SharedPtr */
	typedef std::shared_ptr<SqlDescHandle> SqlDescHandlePtr;
	/** Descriptor-handle Const SharedPtr */
	typedef std::shared_ptr<const SqlDescHandle> ConstSqlDescHandlePtr;

} // namespace exodbc

