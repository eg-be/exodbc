/*!
* \file Database.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 25.07.2014
* \brief Source file for the Database class and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
* Source file for the Database class and its helpers.
*/

// Own header
#include "Database.h"

// Same component headers
#include "Environment.h"
#include "Helpers.h"
#include "Table.h"
#include "Exception.h"
#include "SqlStatementCloser.h"
#include "LogManagerOdbcMacros.h"

// Other headers
// Debug
#include "DebugNew.h"

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	Database::Database() noexcept
		: m_pEnv(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_pHDbc(std::make_shared<SqlDbcHandle>())
		, m_pHStmtExecSql(std::make_shared<SqlStmtHandle>())
		, m_dbmsType(DatabaseProduct::UNKNOWN)
		, m_dbIsOpen(false)
		, m_dbOpenedWithConnectionString(false)
		, m_commitMode(CommitMode::UNKNOWN)
	{
	}


	Database::Database(ConstEnvironmentPtr pEnv)
		: m_pEnv(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_pHDbc(std::make_shared<SqlDbcHandle>())
		, m_pHStmtExecSql(std::make_shared<SqlStmtHandle>())
		, m_dbmsType(DatabaseProduct::UNKNOWN)
		, m_dbIsOpen(false)
		, m_dbOpenedWithConnectionString(false)
		, m_commitMode(CommitMode::UNKNOWN)
	{
		// Allocate the DBC-Handle and set the member m_pEnv
		Init(pEnv);
	}


	Database::Database(const Database& other)
		: m_pEnv(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_pHDbc(std::make_shared<SqlDbcHandle>())
		, m_pHStmtExecSql(std::make_shared<SqlStmtHandle>())
		, m_dbmsType(DatabaseProduct::UNKNOWN)
		, m_dbIsOpen(false)
		, m_dbOpenedWithConnectionString(false)
		, m_commitMode(CommitMode::UNKNOWN)
	{
		if (other.GetEnvironment() != NULL)
		{
			Init(other.GetEnvironment());
		}
	}


	// Destructor
	// -----------
	Database::~Database()
	{
		// Do not throw from here
		try
		{
			if (IsOpen())
			{
				Close();
			}
		}
		catch (Exception& ex)
		{
			LOG_ERROR(ex.ToString());
		}
	}


	// Implementation
	// --------------
	std::shared_ptr<Database> Database::Create(ConstEnvironmentPtr pEnv)
	{
		DatabasePtr pDb = std::make_shared<Database>(pEnv);
		return pDb;
	}


	void Database::Init(ConstEnvironmentPtr pEnv)
	{
		exASSERT(pEnv != NULL);
		exASSERT(pEnv->IsEnvHandleAllocated());
		exASSERT(m_pEnv == NULL);

		// Allocate connection handle from passed environment
		m_pHDbc->AllocateWithParent(pEnv->GetSqlEnvHandle());

		// success, remember environment used
		m_pEnv = pEnv;
	}


	void Database::OpenImpl()
	{
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());
		exASSERT(!m_pHStmtExecSql->IsAllocated());

		// Everything might throw, let this function free the handles in case of failure, 
		// so that we are back in the state before this function was called.
		try
		{
			// Allocate a statement handle from the database connection to be used internally and the exec-handle
			m_pHStmtExecSql->AllocateWithParent(m_pHDbc);

			// Try to load all informations we need
			m_props.Init(m_pHDbc);

			// Check that our ODBC-Version matches, warn if driver cannot support request version.
			// If versions do not match, still continue to use the environment version. the driver
			// manger should handle the version difference
			OdbcVersion driverOdbcVersion = m_props.GetDriverOdbcVersion();
			OdbcVersion odbcVersion = m_pEnv->GetOdbcVersion();
			if (odbcVersion > driverOdbcVersion)
			{
				LOG_WARNING(boost::str(boost::format(u8"ODBC Version missmatch: Environment requested %d, but the driver reported %d.") % (unsigned long) odbcVersion % (unsigned long) driverOdbcVersion ));
			}
			if (!m_pSql2BufferTypeMap)
			{
				m_pSql2BufferTypeMap = Sql2BufferTypeMapPtr(new DefaultSql2BufferMap(odbcVersion));
			}

			// Try to detect the type and store it internally for later use
			m_dbmsType = m_props.DetectDbms();

			// Set up the Catalog information
			m_pDbCatalog = std::make_shared<DatabaseCatalog>(m_pHDbc, m_props);

			// Set Connection Options
			SetConnectionAttributes();

			// Default to manual commit, if the Database is able to set a commit mode. Anyway read the currently active mode, we need to know that
			m_commitMode = ReadCommitMode();
			if (m_props.GetSupportsTransactions() && m_commitMode != CommitMode::MANUAL)
			{
				SetCommitMode(CommitMode::MANUAL);
			}

			// Query the datatypes
			m_datatypes = m_pDbCatalog->ReadSqlTypeInfo();

		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			// Try to free what we've allocated and rethrow
			m_pDbCatalog.reset();
			m_props.Reset();
			if (m_pHStmtExecSql->IsAllocated())
			{
				m_pHStmtExecSql->Free();
			}
			throw;
		}
	}


	std::string Database::Open(const std::string& inConnectStr)
	{
		return Open(inConnectStr, NULL);
	}


	std::string Database::Open(const std::string& inConnectStr, SQLHWND parentWnd)
	{
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());
		exASSERT(!IsOpen());
		exASSERT(!inConnectStr.empty());
		m_dsn = u8"";
		m_uid = u8"";
		m_authStr = u8"";
		m_inConnectionStr = inConnectStr;

		// Connect to the data source
		SQLAPICHARTYPE outConnectBuffer[DB_MAX_CONNECTSTR_LEN + 1];
		SQLSMALLINT outConnectBufferLen;

		// Note: 
		// StringLength1: [Input] Length of *InConnectionString, in characters if the string is Unicode, or bytes if string is ANSI or DBCS.
		// BufferLength: [Input] Length of the *OutConnectionString buffer, in characters.
		SQLRETURN ret = SQLDriverConnect(m_pHDbc->GetHandle(), parentWnd, 
			EXODBCSTR_TO_SQLAPICHARPTR(m_inConnectionStr),
			(SQLSMALLINT) m_inConnectionStr.length(), 
			(SQLAPICHARTYPE*) outConnectBuffer, 
			DB_MAX_CONNECTSTR_LEN, &outConnectBufferLen, parentWnd == NULL ? SQL_DRIVER_NOPROMPT : SQL_DRIVER_COMPLETE);
		THROW_IFN_SUCCEEDED(SQLDriverConnect, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle());

		outConnectBuffer[outConnectBufferLen] = 0;
        m_outConnectionStr = SQLAPICHARPTR_TO_EXODBCSTR(outConnectBuffer);
		m_dbOpenedWithConnectionString = true;

		// Do all the common stuff about opening
		// If we fail, disconnect
		try
		{
			OpenImpl();
		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			// Try to disconnect from the data source
			ret = SQLDisconnect(m_pHDbc->GetHandle());
			if (!SQL_SUCCEEDED(ret))
			{
				ErrorHelper::SErrorInfoVector errs = ErrorHelper::GetAllErrors(SQL_HANDLE_DBC, m_pHDbc->GetHandle());
				ErrorHelper::SErrorInfoVector::const_iterator it;
				for (it = errs.begin(); it != errs.end(); ++it)
				{
					LOG_ERROR(boost::str(boost::format(u8"Failed to Disconnect from datasource with %s (%d): %s") % SqlReturn2s(ret) % ret % it->ToString()));
				}
			}

			// and rethrow what went wrong originally
			throw;
		}

		// Mark database as Open
		m_dbIsOpen = true;
		return m_outConnectionStr;
	}


	void Database::Open(const std::string& dsn, const std::string& uid, const std::string& authStr)
	{
		exASSERT(!IsOpen());
		exASSERT(!dsn.empty());
		exASSERT(HasConnectionHandle());

		m_dsn        = dsn;
		m_uid        = uid;
		m_authStr    = authStr;

		// Not using a connection-string
		m_inConnectionStr = u8"";
		m_outConnectionStr = u8"";

		// Connect to the data source
		SQLRETURN ret = SQLConnect(m_pHDbc->GetHandle(),
			EXODBCSTR_TO_SQLAPICHARPTR(m_dsn), SQL_NTS,
			EXODBCSTR_TO_SQLAPICHARPTR(m_uid), SQL_NTS,
			EXODBCSTR_TO_SQLAPICHARPTR(m_authStr), SQL_NTS);

		// Do not reset an eventually allocated connection handle here.
		// The destructor will reset it if one is allocated
		THROW_IFN_SUCCEEDED(SQLConnect, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle());

		// Do all the common stuff about opening
		// If we fail, disconnect
		try
		{
			OpenImpl();
		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			// Try to disconnect from the data source
			ret = SQLDisconnect(m_pHDbc->GetHandle());
			if ( ! SQL_SUCCEEDED(ret))
			{
				ErrorHelper::SErrorInfoVector errs = ErrorHelper::GetAllErrors(SQL_HANDLE_DBC, m_pHDbc->GetHandle());
				ErrorHelper::SErrorInfoVector::const_iterator it;
				for (it = errs.begin(); it != errs.end(); ++it)
				{
					LOG_ERROR(boost::str(boost::format(u8"Failed to Disconnect from datasource with %s (%d): %s") % SqlReturn2s(ret) % ret % it->ToString()));
				}
			}

			// and rethrow what went wrong originally
			throw;
		}
		
		// Mark database as Open
		m_dbIsOpen = true;
	}


	bool Database::IsSqlTypeSupported(SQLSMALLINT sqlType) const
	{
		exASSERT(IsOpen());
		SqlTypeInfosVector::const_iterator it = m_datatypes.begin();
		while (it != m_datatypes.end())
		{
			if (it->GetSqlDataType() == sqlType || it->GetSqlType() == sqlType)
			{
				return true;
			}
			++it;
		}
		return false;
	}


	void Database::SetSql2BufferTypeMap(Sql2BufferTypeMapPtr pSql2BufferTypeMap) noexcept
	{
		m_pSql2BufferTypeMap = pSql2BufferTypeMap;
	}


	Sql2BufferTypeMapPtr Database::GetSql2BufferTypeMap() const
	{
		exASSERT(m_pSql2BufferTypeMap);
		return m_pSql2BufferTypeMap;
	}


	SqlTypeInfosVector Database::GetTypeInfos() const
	{
		exASSERT(IsOpen());
		return m_datatypes;
	}


	DatabaseCatalogPtr Database::GetDbCatalog() const
	{
		exASSERT(IsOpen());
		return m_pDbCatalog;
	}


	void Database::SetConnectionAttributes()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		// but we need to have at least the connection handle
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		SQLRETURN ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TRACE, (SQLPOINTER) SQL_OPT_TRACE_OFF, 0);
		THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Cannot set SQL_ATTR_TRACE to SQL_OPT_TRACE_OFF");

		// For the moment do nothing here. 
		// There is no need to set any connection attributes right now, we assume defaults are fine.
	}


	void Database::Close()
	{
		// Note: We assume that we are fully opened, all statements are allocated, etc.
		// Open() will do its own cleanup in case of failure.
		if (!IsOpen())
		{
			return;
		}

		// Rollback any ongoing Transaction if in Manual mode
		if (GetCommitMode() == CommitMode::MANUAL)
		{
			RollbackTrans();
		}

		// Free statement handles - keep the pointers, but free the handle held
		m_pHStmtExecSql->Free();

		// And also clear all props read and the catalog
		m_props.Reset();
		m_pDbCatalog.reset();

		// Try to disconnect from the data source
		SQLRETURN ret = SQLDisconnect(m_pHDbc->GetHandle());
		THROW_IFN_SUCCEEDED(SQLDisconnect, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle());

		m_dbIsOpen = false;
	}


	void Database::CommitTrans() const
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		// Commit the transaction
		SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_pHDbc->GetHandle(), SQL_COMMIT);
		THROW_IFN_SUCCEEDED_MSG(SQLEndTran, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Failed to Commit Transaction");
	}


	void Database::RollbackTrans() const
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		// Rollback the transaction
		SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_pHDbc->GetHandle(), SQL_ROLLBACK);
		THROW_IFN_SUCCEEDED_MSG(SQLEndTran, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Failed to Rollback Transaction");
	}


	void Database::ExecSql(const std::string& sqlStmt, ExecFailMode mode /* = NotFailOnNoData */)
	{
		exASSERT(IsOpen());

		RETCODE retcode;

		StatementCloser::CloseStmtHandle(m_pHStmtExecSql, StatementCloser::Mode::IgnoreNotOpen);

		retcode = SQLExecDirect(m_pHStmtExecSql->GetHandle(),  EXODBCSTR_TO_SQLAPICHARPTR(sqlStmt), SQL_NTS);
		if ( ! SQL_SUCCEEDED(retcode))
		{
			if (!(mode == ExecFailMode::NotFailOnNoData && retcode == SQL_NO_DATA))
			{
				SqlResultException ex(u8"SQLExecDirect", retcode, SQL_HANDLE_STMT, m_pHStmtExecSql->GetHandle(), (boost::format(u8"Failed to execute Stmt '%s'") % sqlStmt).str());
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
		}
	}


	CommitMode Database::ReadCommitMode()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		CommitMode mode = CommitMode::UNKNOWN;

		SQLULEN modeValue = 0;
		SQLINTEGER cb;
		SQLRETURN ret = SQLGetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_AUTOCOMMIT, &modeValue, sizeof(modeValue), &cb);
		THROW_IFN_SUCCEEDED_MSG(SQLGetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Failed to read Attr SQL_ATTR_AUTOCOMMIT");

		if(modeValue == SQL_AUTOCOMMIT_OFF)
			mode = CommitMode::MANUAL;
		else if(modeValue == SQL_AUTOCOMMIT_ON)
			mode = CommitMode::AUTO;

		m_commitMode = mode;

		return mode;
	}


	TransactionIsolationMode Database::ReadTransactionIsolationMode()
	{
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		SQLULEN modeValue = 0;
		SQLINTEGER cb;
		SQLRETURN ret = SQLGetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TXN_ISOLATION, &modeValue, sizeof(modeValue), &cb);
		THROW_IFN_SUCCEEDED_MSG(SQLGetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Failed to read Attr SQL_ATTR_TXN_ISOLATION");

		switch (modeValue)
		{
		case SQL_TXN_READ_UNCOMMITTED:
			return TransactionIsolationMode::READ_UNCOMMITTED;
		case SQL_TXN_READ_COMMITTED:
			return TransactionIsolationMode::READ_COMMITTED;
		case SQL_TXN_REPEATABLE_READ:
			return TransactionIsolationMode::REPEATABLE_READ;
		case SQL_TXN_SERIALIZABLE:
			return TransactionIsolationMode::SERIALIZABLE;
		}

		return TransactionIsolationMode::UNKNOWN;
	}

	void Database::SetCommitMode(CommitMode mode)
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		// If Autocommit is off, we need to rollback any ongoing transaction
		// Else at least MS SQL Server will complain that an ongoing transaction has been committed.
		if (GetCommitMode() != CommitMode::AUTO)
		{
			RollbackTrans();
		}

		SQLRETURN ret;
		std::string errStringMode;
		if (mode == CommitMode::MANUAL)
		{
			ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0);
			errStringMode = u8"SQL_AUTOCOMMIT_OFF";
		}
		else
		{
			ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_ON, 0);
			errStringMode = u8"SQL_AUTOCOMMIT_ON";
		}
		THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), (boost::format(u8"Setting ATTR_AUTOCOMMIT to %s failed") % errStringMode).str());

		m_commitMode = mode;
	}


	void Database::SetTransactionIsolationMode(TransactionIsolationMode mode)
	{	
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		// We need to ensure cursors are closed:
		StatementCloser::CloseStmtHandle(m_pHStmtExecSql, StatementCloser::Mode::IgnoreNotOpen);

		// If Autocommit is off, we need to Rollback any ongoing transaction
		// Else at least MS SQL Server will complain that an ongoing transaction has been committed.
		if (GetCommitMode() != CommitMode::AUTO)
		{
			RollbackTrans();
		}

		SQLRETURN ret;
		{
			ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)mode, 0);
		}
		THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), (boost::format(u8"Cannot set SQL_ATTR_TXN_ISOLATION to %d") % (int) mode).str());

	}

	
	bool Database::CanSetTransactionIsolationMode(TransactionIsolationMode mode) const
	{
		SQLUINTEGER txnIsolationOpts = boost::get<SQLUINTEGER>(m_props.GetProperty(SQL_TXN_ISOLATION_OPTION).GetValue());
		return (txnIsolationOpts & (SQLUINTEGER)mode) != 0;
	}
}
