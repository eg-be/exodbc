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

	/*!
	* \class SqlHandle
	*
	* \brief A wrapper around a SQLHANDLE. Handle gets freed on destruction.
	*/
	template<typename THANDLE, SQLSMALLINT tHandleType, typename TPARENTSQLHANDLE>
	class SqlHandle
	{
		typedef boost::signals2::signal<void(const SqlHandle&)> FreeSignal;
		typedef typename FreeSignal::slot_type FreeSignalSlot;

		typedef boost::signals2::signal<void(const SqlHandle&)> ResetParamsSignal;
		typedef typename ResetParamsSignal::slot_type ResetParamsSignalSlot;

		typedef boost::signals2::signal<void(const SqlHandle&)> UnbindColumnsSignal;
		typedef typename UnbindColumnsSignal::slot_type UnbindColumnsSignalSlot;

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
		* \brief: Create a new instance wrapped into a shared_ptr
		*/
		static std::shared_ptr<SqlHandle> Create(std::shared_ptr<const TPARENTSQLHANDLE> pParentHandle)
		{
			return std::make_shared<SqlHandle>(pParentHandle);
		}


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
		* \brief	Allocates the internal handle.
		* \details	Can be only be used if tHandleType is SQL_HANDLE_ENV, all other handles need a parent
		*			to be allocated.
		* \throw	Exception if already allocated, or if allocating fails.
		*/
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
		*			SQL_NULL_HANDLE upon success, and resets the internal 
		*			shared_ptr to the parent handle.
		*
		*			Before freeing, fires the FreeSignal.
		*			After freeing, fires the ResetParamsSignal and UnbindColumnsSignal.
		* \throw	AssertionException If no handle is allocated.
		* \throw	SqlResultException If freeing fails.
		*/
		void Free()
		{
			exASSERT(m_handle != SQL_NULL_HANDLE);

			// Before freeing, trigger signal that we are going to be freed
			m_freeSignal(*this);

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

			// notify that params have been reseted and columns are unbound now
			m_resetParamsSignal(*this);
			m_unbindColumnsSignal(*this);
		}


		/*!
		* \brief Resets all parameters bound to this Handle by calling SQLFreeStmt() with SQL_RESET_PARAMS.
		*		On Success, the ResetParamsSignal is fired.
		*/
		void ResetParams() const
		{
			exASSERT(m_handle != SQL_NULL_HANDLE);
			exASSERT(tHandleType == SQL_HANDLE_STMT);

			SQLRETURN ret = SQLFreeStmt(m_handle, SQL_RESET_PARAMS);
			THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_handle);

			// trigger signal
			m_resetParamsSignal(*this);
		}


		/*!
		* \brief Resets all columns bound to this Handle by calling SQLFreeStmt() with SQL_UNBIND.
		*		On Success, the UnbindColumnsSignal is fired.
		*/
		void UnbindColumns() const
		{
			exASSERT(m_handle != SQL_NULL_HANDLE);
			exASSERT(tHandleType == SQL_HANDLE_STMT);

			SQLRETURN ret = SQLFreeStmt(m_handle, SQL_UNBIND);
			THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_handle);

			// trigger signal
			m_unbindColumnsSignal(*this);
		}


		/*!
		* \brief	Connect a signal that gets called whenever Params have been reseted.
		*/
		boost::signals2::connection ConnectResetParamsSignal(const ResetParamsSignalSlot& slot) const { return m_resetParamsSignal.connect(slot); };

		
		/*!
		* \brief	Connect a signal that gets called whenever Columns have been unbound.
		*/
		boost::signals2::connection ConnectUnbindColumnsSignal(const UnbindColumnsSignalSlot& slot) const { return m_unbindColumnsSignal.connect(slot); };


		/*!
		* \brief	Connect a signal that gets called before trying to free the handle.
		*/
		boost::signals2::connection ConnectFreeSignal(const FreeSignalSlot& slot) const { return m_freeSignal.connect(slot); };


		/*!
		* \brief	Get the handle. Does not throw could return SQL_NULL_HANDLE if not allocated yet.
		* \see		IsAllocated()
		*/
		THANDLE GetHandle() const noexcept { return m_handle; };
		
		/*!
		* \brief	Returns true if a handle is allocated.
		*/
		bool IsAllocated() const noexcept { return m_handle != SQL_NULL_HANDLE; };


		/*!
		* \brief	Returns the type of the Handle.
		*/
		SQLSMALLINT GetHandleType() const noexcept { return tHandleType; };

		/*!
		* \brief	Handles are equal if they are of the same type and have the same value.
		*/
		bool operator==(const SqlHandle& other) const noexcept
		{
			return GetHandleType() == other.GetHandleType()
				&& m_handle == other.m_handle;
		};

		/*!
		* \brief	Handles are equal if they are of the same type and have the same value.
		*/
		bool operator!=(const SqlHandle& other) const noexcept
		{
			return !(*this == other);
		};

		/*!
		* \brief	Orders by handle type and then handle value.
		*/
		bool operator<(const SqlHandle& other) const noexcept
		{
			return GetHandleType() < other.GetHandleType() && m_handle < other.m_handle;
		}

	private:
		THANDLE m_handle;
		std::shared_ptr<const TPARENTSQLHANDLE> m_pParentHandle;
		mutable FreeSignal m_freeSignal;
		mutable UnbindColumnsSignal m_unbindColumnsSignal;
		mutable ResetParamsSignal m_resetParamsSignal;
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

	/*!
	* \class SqlHandle<SQLHDESC, SQL_HANDLE_DESC, SqlStmtHandle>
	* \brief SqlHandle specialization for Description handles.
	*/
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
		/*!
		* \brief Creates description handle using SQLGEtStmtAttr.
		*/
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
		* \brief	Get the handle. Might return SQL_NULL_HANDLE if not allocated.
		* \see		IsAllocated()
		*/
		SQLHDESC GetHandle() const noexcept { return m_handle; };

		/*!
		* \brief	Returns true if a handle is allocated.
		*/
		bool IsAllocated() const noexcept { return m_handle != SQL_NULL_HDESC; };


		/*!
		* \brief	Returns the type of the Handle.
		*/
		SQLSMALLINT GetHandleType() const noexcept { return SQL_HANDLE_DESC; };



		/*!
		* \brief	Handles are equal if they are of the same type and have the same value.
		*/
		bool operator==(const SqlHandle& other) const noexcept
		{
			return GetHandleType() == other.GetHandleType()
				&& m_handle == other.m_handle;
		};


		/*!
		* \brief	Handles are equal if they are of the same type and have the same value.
		*/
		bool operator!=(const SqlHandle& other) const noexcept
		{
			return !(*this == other);
		};


		/*!
		* \brief	Orders by handle type and then handle value.
		*/
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
