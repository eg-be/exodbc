/*!
* \file Database.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 25.07.2014
* \brief Source file for the Database class and its helpers.
* \copyright wxWindows Library Licence, Version 3.1
*
* Source file for the Database class and its helpers.
*/

#include "stdafx.h"

// Own header
#include "Database.h"

// Same component headers
#include "Environment.h"
#include "Helpers.h"
#include "Table.h"
#include "Exception.h"

// Other headers

// Debug
#include "DebugNew.h"

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	Database::Database()
		: m_pEnv(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_hdbc(SQL_NULL_HDBC)
		, m_hstmt(SQL_NULL_HSTMT)
		, m_hstmtExecSql(SQL_NULL_HSTMT)
		, m_dbmsType(DatabaseProduct::UNKNOWN)
		, m_dbIsOpen(false)
		, m_dbOpenedWithConnectionString(false)
		, m_commitMode(CommitMode::UNKNOWN)
	{
	}


	Database::Database(const Environment* pEnv)
		: m_pEnv(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_hdbc(SQL_NULL_HDBC)
		, m_hstmt(SQL_NULL_HSTMT)
		, m_hstmtExecSql(SQL_NULL_HSTMT)
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
		, m_hdbc(SQL_NULL_HDBC)
		, m_hstmt(SQL_NULL_HSTMT)
		, m_hstmtExecSql(SQL_NULL_HSTMT)
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
			if (HasConnectionHandle())
			{
				FreeConnectionHandle();
			}
		}
		catch (Exception& ex)
		{
			LOG_ERROR(ex.ToString());
		}
	}


	// Implementation
	// --------------
	void Database::Init(const Environment* pEnv)
	{
		exASSERT(pEnv != NULL);

		// remember the environment
		exASSERT(m_pEnv == NULL);
		m_pEnv = pEnv;

		// And allocate connection handle
		AllocateConnectionHandle();
	}


	void Database::FreeConnectionHandle()
	{
		exASSERT(HasConnectionHandle());

		// Returns only SQL_SUCCESS, SQL_ERROR, or SQL_INVALID_HANDLE.
		SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc);
		// if SQL_ERROR is returned, the handle is still valid, error information can be fetched, use our standard logger
		if (ret == SQL_ERROR)
		{
			// if SQL_ERROR is returned, the handle is still valid, error information can be fetched
			SqlResultException ex(L"SQLFreeHandle", ret, SQL_HANDLE_DBC, m_hdbc, L"Freeing ODBC-Connection Handle failed with SQL_ERROR, handle is still valid.");
			SET_EXCEPTION_SOURCE(ex);
			LOG_ERROR(ex.ToString());
		}
		else if (ret == SQL_INVALID_HANDLE)
		{
			// If we've received INVALID_HANDLE our handle has probably already be deleted - anyway, its invalid, reset it.
			m_hdbc = SQL_NULL_HDBC;
			SqlResultException ex(L"SQLFreeHandle", ret, L"Freeing ODBC-Connection Handle failed with SQL_INVALID_HANDLE.");
			SET_EXCEPTION_SOURCE(ex);
			LOG_ERROR(ex.ToString());
		}
		// We have SUCCESS
		m_hdbc = SQL_NULL_HDBC;
	}


	void Database::AllocateConnectionHandle()
	{
		exASSERT(!HasConnectionHandle());
		exASSERT(m_pEnv != NULL);
		exASSERT(m_pEnv->HasEnvironmentHandle());

		// Allocate the DBC-Handle from our environment
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DBC, m_pEnv->GetEnvironmentHandle(), &m_hdbc);
		THROW_IFN_SUCCEEDED(SQLAllocHandle, ret, SQL_HANDLE_ENV, m_pEnv->GetEnvironmentHandle());
	}


	void Database::OpenImpl()
	{
		exASSERT(m_hstmt == SQL_NULL_HSTMT);
		exASSERT(m_hstmtExecSql == SQL_NULL_HSTMT);

		// Allocate a statement handle for the database connection to use internal and the exec-handle
		// Note: SQLAllocHandle will set the output-handle to SQL_NULL_HDBC, SQL_NULL_HSTMT, or SQL_NULL_HENV in case of failure
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);
		// safe to throw directly, no handle has been allocated so far
		THROW_IFN_SUCCEEDED(SQLAllocHandle, ret, SQL_HANDLE_DBC, m_hdbc);

		ret = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmtExecSql);
		// Need to free the m_hstmt handle - okay to throw if we fail, no other handle allocated
		if (!SQL_SUCCEEDED(ret))
		{
			m_hstmt = FreeStatementHandle(m_hstmt);
		}
		THROW_IFN_SUCCEEDED(SQLAllocHandle, ret, SQL_HANDLE_DBC, m_hdbc);

		// Everything else might throw, let this function free the handles in case of failure
		try
		{
			// Query the data source for info about itself
			m_dbInf = ReadDbInfo();

			// Check that our ODBC-Version matches
			// We do not fail, what counts for us is the version of the
			// environment.
			OdbcVersion connectionVersion = GetDriverOdbcVersion();
			OdbcVersion envVersion = m_pEnv->GetOdbcVersion();
			OdbcVersion maxSupportedVersion = GetMaxSupportedOdbcVersion();
			if (envVersion > connectionVersion)
			{
				LOG_WARNING((boost::wformat(L"ODBC Version missmatch: Environment requested %d, but the driver (name: '%s' version: '%s') reported %d ('%s'). The Database ('%s') will be using %d") % (int)envVersion %m_dbInf.GetDriverName() %m_dbInf.GetDriverVersion() % (int)connectionVersion %m_dbInf.GetDriverOdbcVersion() %m_dbInf.GetDbmsName() % (int)connectionVersion).str());
			}
			if (!m_pSql2BufferTypeMap)
			{
				m_pSql2BufferTypeMap = Sql2BufferTypeMapPtr(new DefaultSql2BufferMap(maxSupportedVersion));
			}

			// Try to detect the type - this will update our internal type
			DetectDbms();

			// Set Connection Options
			SetConnectionAttributes();

			// Default to manual commit, if the Database is able to set a commit mode. Anyway read the currently active mode, we need to know that
			m_commitMode = ReadCommitMode();
			if (m_dbInf.GetSupportsTransactions() && m_commitMode != CommitMode::MANUAL)
			{
				SetCommitMode(CommitMode::MANUAL);
			}

			// Query the datatypes
			m_datatypes = ReadDataTypesInfo();

		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			// Free statements and rethrow. Do not allow to throw from freeing
			m_hstmt = FreeStatementHandle(m_hstmt, FSTF_NO_THROW);
			m_hstmtExecSql = FreeStatementHandle(m_hstmtExecSql, FSTF_NO_THROW);
			throw;
		}

		// Completed Successfully
		LOG_DEBUG((boost::wformat(L"Opened connection to database %s") %m_dbInf.GetDbmsName()).str());
	}


	std::wstring Database::Open(const std::wstring& inConnectStr)
	{
		return Open(inConnectStr, NULL);
	}


	std::wstring Database::Open(const std::wstring& inConnectStr, SQLHWND parentWnd)
	{
		exASSERT(!IsOpen());
		exASSERT(!inConnectStr.empty());
		m_dsn = L"";
		m_uid = L"";
		m_authStr = L"";
		m_inConnectionStr = inConnectStr;

		// Connect to the data source
		SQLWCHAR outConnectBuffer[DB_MAX_CONNECTSTR_LEN + 1];
		SQLSMALLINT outConnectBufferLen;

		// Note: 
		// StringLength1: [Input] Length of *InConnectionString, in characters if the string is Unicode, or bytes if string is ANSI or DBCS.
		// BufferLength: [Input] Length of the *OutConnectionString buffer, in characters.
		SQLRETURN ret = SQLDriverConnect(m_hdbc, parentWnd, 
			(SQLWCHAR*) m_inConnectionStr.c_str(),
			(SQLSMALLINT) m_inConnectionStr.length(), 
			(SQLWCHAR*) outConnectBuffer, 
			DB_MAX_CONNECTSTR_LEN, &outConnectBufferLen, parentWnd == NULL ? SQL_DRIVER_NOPROMPT : SQL_DRIVER_COMPLETE);
		THROW_IFN_SUCCEEDED(SQLDriverConnect, ret, SQL_HANDLE_DBC, m_hdbc);

		outConnectBuffer[outConnectBufferLen] = 0;
		m_outConnectionStr = outConnectBuffer;
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
			ret = SQLDisconnect(m_hdbc);
			if (!SQL_SUCCEEDED(ret))
			{
				SErrorInfoVector errs = GetAllErrors(SQL_HANDLE_DBC, m_hdbc);
				SErrorInfoVector::const_iterator it;
				for (it = errs.begin(); it != errs.end(); ++it)
				{
					LOG_ERROR(boost::str(boost::wformat(L"Failed to Disconnect from datasource with %s (%d): %s") % SqlReturn2s(ret) % ret % it->ToString()));
				}
			}

			// and rethrow what went wrong originally
			throw;
		}

		// Mark database as Open
		m_dbIsOpen = true;
		return m_outConnectionStr;
	}


	void Database::Open(const std::wstring& dsn, const std::wstring& uid, const std::wstring& authStr)
	{
		exASSERT(!IsOpen());
		exASSERT(!dsn.empty());
		exASSERT(HasConnectionHandle());

		m_dsn        = dsn;
		m_uid        = uid;
		m_authStr    = authStr;

		// Not using a connection-string
		m_inConnectionStr = L"";
		m_outConnectionStr = L"";

		// Connect to the data source
		SQLRETURN ret = SQLConnect(m_hdbc, 
			(SQLWCHAR*) m_dsn.c_str(), SQL_NTS,
			(SQLWCHAR*) m_uid.c_str(), SQL_NTS,
			(SQLWCHAR*) m_authStr.c_str(), SQL_NTS);

		// Do not reset an eventually allocated connection handle here.
		// The destructor will reset it if one is allocated
		THROW_IFN_SUCCEEDED(SQLConnect, ret, SQL_HANDLE_DBC, m_hdbc);

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
			ret = SQLDisconnect(m_hdbc);
			if ( ! SQL_SUCCEEDED(ret))
			{
				SErrorInfoVector errs = GetAllErrors(SQL_HANDLE_DBC, m_hdbc);
				SErrorInfoVector::const_iterator it;
				for (it = errs.begin(); it != errs.end(); ++it)
				{
					LOG_ERROR(boost::str(boost::wformat(L"Failed to Disconnect from datasource with %s (%d): %s") % SqlReturn2s(ret) % ret % it->ToString()));
				}
			}

			// and rethrow what went wrong originally
			throw;
		}
		
		// Mark database as Open
		m_dbIsOpen = true;
	}


	void Database::SetTracefile(const std::wstring path)
	{
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		// note, from ms-doc: 
		// Character strings pointed to by the ValuePtr argument of SQLSetConnectAttr have a length of StringLength bytes.
		SQLINTEGER cb = 0;
		SQLRETURN ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_TRACEFILE, (SQLPOINTER)path.c_str(), (path.length()) * sizeof(SQLWCHAR));
		THROW_IFN_SUCCEEDED(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc);
	}


	std::wstring Database::GetTracefile() const
	{
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		// Assume some max length, querying the driver about length did not really work
		SQLINTEGER cb = 0;
		size_t charBuffSize = MAX_PATH + 1;
		size_t byteBuffSize = sizeof(SQLWCHAR) * charBuffSize;
		std::unique_ptr<SQLWCHAR[]> buffer(new SQLWCHAR[charBuffSize]);
		memset(buffer.get(), 0, byteBuffSize);
		SQLRETURN ret = SQLGetConnectAttr(m_hdbc, SQL_ATTR_TRACEFILE, (SQLPOINTER)buffer.get(), byteBuffSize, &cb);
		THROW_IFN_SUCCEEDED(SQLGetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc);
		return buffer.get();
	}


	void Database::SetTrace(bool enable)
	{
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		if (enable)
		{
			SQLRETURN ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_TRACE, (SQLPOINTER)SQL_OPT_TRACE_ON, NULL);
			THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc, L"Failed to set Attribute SQL_ATTR_TRACE to SQL_OPT_TRACE_ON");
		}
		else
		{
			SQLRETURN ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_TRACE, (SQLPOINTER)SQL_OPT_TRACE_OFF, NULL);
			THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc, L"Failed to set Attribute SQL_ATTR_TRACE to SQL_OPT_TRACE_OFF");
		}
	}


	bool Database::GetTrace() const
	{
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		SQLUINTEGER value = 0;
		SQLRETURN ret = SQLGetConnectAttr(m_hdbc, SQL_ATTR_TRACE, &value, NULL, NULL);
		THROW_IFN_SUCCEEDED_MSG(SQLGetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc, L"Failed to read Attribute SQL_ATTR_TRACE");

		return value == SQL_OPT_TRACE_ON;
	}


	bool Database::IsMarsEnabled()
	{
		exASSERT(IsOpen());

		// Use the information from the db-inf, except for ms,
		// if we have the additional info from HAVE_MSODBCSQL_H
#ifdef HAVE_MSODBCSQL_H
		if (GetDbms() == DatabaseProduct::MS_SQL_SERVER)
		{
			SQLULEN value = 0;
			SQLINTEGER cb = 0;
			SQLRETURN ret = SQLGetConnectAttr(m_hdbc, SQL_COPT_SS_MARS_ENABLED, &value, sizeof(value), &cb);
			THROW_IFN_SUCCEEDED_MSG(SQLGetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc, L"Cannot read option SQL_COPT_SS_MARS_ENABLED.");
			return value == SQL_MARS_ENABLED_YES;
		}
#endif
		SQLINTEGER maxActiveStatements = m_dbInf.GetUSmallIntProperty(DatabaseInfo::USmallIntProperty::MaxConcurrentActivs);
		return maxActiveStatements == 0 || maxActiveStatements > 1;
	}


#ifdef HAVE_MSODBCSQL_H
	void Database::SetMarsEnabled(bool enableMars)
	{
		exASSERT(m_hdbc != SQL_NULL_HDBC);
		exASSERT(!IsOpen());
		SQLRETURN ret = 0;
		if (enableMars)
		{
			ret = SQLSetConnectAttr(m_hdbc, SQL_COPT_SS_MARS_ENABLED, (SQLPOINTER)SQL_MARS_ENABLED_YES, SQL_IS_UINTEGER);
		}
		else
		{
			ret = SQLSetConnectAttr(m_hdbc, SQL_COPT_SS_MARS_ENABLED, (SQLPOINTER)SQL_MARS_ENABLED_NO, SQL_IS_UINTEGER);
		}
		THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc, L"Cannot set option SQL_COPT_SS_MARS_ENABLED.");
	}
#endif


	bool Database::IsSqlTypeSupported(SQLSMALLINT sqlType) const
	{
		exASSERT(IsOpen());
		SqlTypeInfosVector::const_iterator it = m_datatypes.begin();
		while (it != m_datatypes.end())
		{
			if (it->m_sqlDataType == sqlType || it->m_sqlType == sqlType)
			{
				return true;
			}
			++it;
		}
		return false;
	}


	void Database::SetSql2BufferTypeMap(Sql2BufferTypeMapPtr pSql2BufferTypeMap) throw()
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


	void Database::SetConnectionAttributes()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		SQLRETURN ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_TRACE, (SQLPOINTER) SQL_OPT_TRACE_OFF, NULL);
		THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc, L"Cannot set SQL_ATTR_TRACE to SQL_OPT_TRACE_OFF");

		// Note: This is unsupported SQL_ATTR_METADATA_ID by most drivers. It should default to OFF
		//ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_TRUE, NULL);
		//if(ret != SQL_SUCCESS)
		//{
		//	LOG_ERROR_DBC_MSG(m_hdbc, ret, SQLSetConnectAttr, L"Cannot set ATTR_METADATA_ID to SQL_FALSE");
		//	ok = false;
		//}
	}


	DatabaseInfo Database::ReadDbInfo()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_hdbc != SQL_NULL_HDBC);
		
		DatabaseInfo dbInf;

		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::AccessibleTables);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::DatabaseName);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::DbmsName);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::DbmsVersion);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::DriverName);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::DriverOdbcVersion);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::DriverVersion);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::OdbcSupportIEF);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::OdbcVersion);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::OuterJoins);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::ProcedureSupport);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::SearchPatternEscape);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::WStringProperty::ServerName);

		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::USmallIntProperty::CursorCommitBehavior);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::USmallIntProperty::CursorRollbackBehavior);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::USmallIntProperty::MaxCatalogNameLen);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::USmallIntProperty::MaxColumnNameLen);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::USmallIntProperty::MaxConcurrentActivs);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::USmallIntProperty::MaxConnections);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::USmallIntProperty::MaxSchemaNameLen);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::USmallIntProperty::MaxTableNameLen);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::USmallIntProperty::NonNullableColumns);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::USmallIntProperty::OdbcSagCliConformance);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::USmallIntProperty::TxnCapable);

		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::UIntProperty::DefaultTxnIsolation);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::UIntProperty::ScrollOptions);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::UIntProperty::TxnIsolationOption);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::UIntProperty::CursorSensitity);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::UIntProperty::DynamicCursorAttributes1);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::UIntProperty::ForwardOnlyCursorAttributes1);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::UIntProperty::KeysetCursorAttributes1);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::UIntProperty::StaticCursorAttributes1);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::UIntProperty::KeysetCursorAttributes2);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::UIntProperty::StaticCursorAttributes2);

		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::IntProperty::PositionedStatements);
		dbInf.ReadAndStoryProperty(m_hdbc, DatabaseInfo::IntProperty::PosOperations);

		return dbInf;
	}


	std::vector<std::wstring> Database::ReadCatalogInfo(ReadCatalogInfoMode mode)
	{
		exASSERT(IsOpen());

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_hstmt, true, true);

		std::vector<std::wstring> results;

		SQLPOINTER catalogName = L"";
		SQLPOINTER schemaName = L"";
		SQLPOINTER tableTypeName = L"";

		SQLSMALLINT colNr = 0;
		SQLUSMALLINT charLen = 0;

		switch(mode)
		{
		case ReadCatalogInfoMode::AllCatalogs:
			catalogName = SQL_ALL_CATALOGS;
			colNr = 1;
			charLen = m_dbInf.GetMaxCatalogNameLen();
			break;
		case ReadCatalogInfoMode::AllSchemas:
			schemaName = SQL_ALL_SCHEMAS;
			colNr = 2;
			charLen = m_dbInf.GetMaxSchemaNameLen();
			break;
		case ReadCatalogInfoMode::AllTableTypes:
			tableTypeName = SQL_ALL_TABLE_TYPES;
			colNr = 4;
			charLen = m_dbInf.GetMaxTableTypeNameLen();
			break;
		default:
			exASSERT(false);
		}
		
		std::unique_ptr<SQLWCHAR[]> buffer(new SQLWCHAR[charLen]);
		SQLRETURN ret = SQLTables(m_hstmt,
			(SQLWCHAR*)catalogName, SQL_NTS,   // catname                 
			(SQLWCHAR*)schemaName, SQL_NTS,   // schema name
			L"", SQL_NTS,							// table name
			(SQLWCHAR*)tableTypeName, SQL_NTS);

		THROW_IFN_SUCCEEDED(SQLTables, ret, SQL_HANDLE_STMT, m_hstmt);

		// Read data
		SQLLEN cb;
		while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)   // Table Information
		{
			GetData(m_hstmt, colNr, SQL_C_WCHAR, buffer.get(), charLen * sizeof(SQLWCHAR), &cb, NULL);
			results.push_back(buffer.get());
		}

		THROW_IFN_NO_DATA(SQLFetch, ret);

		return results;
	}


	TableInfo Database::FindOneTable(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType) const
	{
		// Query the tables that match
		TableInfosVector tables = FindTables(tableName, schemaName, catalogName, tableType);

		if(tables.size() == 0)
		{
			Exception ex((boost::wformat(L"No tables found while searching for: tableName: '%s', schemName: '%s', catalogName: '%s', typeName : '%s'") %tableName %schemaName %catalogName %tableType).str());
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		if(tables.size() != 1)
		{
			Exception ex((boost::wformat(L"Not exactly one table found while searching for: tableName: '%s', schemName: '%s', catalogName: '%s', typeName : '%s'") %tableName %schemaName %catalogName %tableType).str());
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}

		TableInfo table = tables[0];
		return table;
	}


	SqlTypeInfosVector Database::ReadDataTypesInfo()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_hstmt != SQL_NULL_HSTMT);

		SqlTypeInfosVector types;

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_hstmt, true, true);

		SQLRETURN ret = SQLGetTypeInfo(m_hstmt, SQL_ALL_TYPES);
		THROW_IFN_SUCCEEDED(SQLGetTypeInfo, ret, SQL_HANDLE_STMT, m_hstmt);

		ret = SQLFetch(m_hstmt);
		int count = 0;
		SQLWCHAR typeName[DB_MAX_TYPE_NAME_LEN + 1];
		SQLWCHAR literalPrefix[DB_MAX_LITERAL_PREFIX_LEN + 1];
		SQLWCHAR literalSuffix[DB_MAX_LITERAL_SUFFIX_LEN + 1];
		SQLWCHAR createParams[DB_MAX_CREATE_PARAMS_LIST_LEN + 1];
		SQLWCHAR localTypeName[DB_MAX_LOCAL_TYPE_NAME_LEN + 1];
		SQLLEN cb;
		while(ret == SQL_SUCCESS)
		{
			typeName[0] = 0;
			literalPrefix[0] = 0;
			literalSuffix[0] = 0;
			createParams[0] = 0;
			localTypeName[0] = 0;
			SSqlTypeInfo info;

			cb = 0;
			GetData(m_hstmt, 1, SQL_C_WCHAR, typeName, sizeof(typeName), &cb, NULL);
			info.m_typeName = typeName;

			GetData(m_hstmt, 2, SQL_C_SSHORT, &info.m_sqlType, sizeof(info.m_sqlType), &cb, NULL);
			GetData(m_hstmt, 3, SQL_C_SLONG, &info.m_columnSize, sizeof(info.m_columnSize), &cb, &info.m_columnSizeIsNull);
			GetData(m_hstmt, 4, SQL_C_WCHAR, literalPrefix, sizeof(literalPrefix), &cb, &info.m_literalPrefixIsNull);
			info.m_literalPrefix = literalPrefix;
			GetData(m_hstmt, 5, SQL_C_WCHAR, literalSuffix, sizeof(literalSuffix), &cb, &info.m_literalSuffixIsNull);
			info.m_literalSuffix = literalSuffix;
			GetData(m_hstmt, 6, SQL_C_WCHAR, createParams, sizeof(createParams), &cb, &info.m_createParamsIsNull);
			info.m_createParams = createParams;
			GetData(m_hstmt, 7, SQL_C_SSHORT, &info.m_nullable, sizeof(info.m_nullable), &cb, NULL);
			GetData(m_hstmt, 8, SQL_C_SSHORT, &info.m_caseSensitive, sizeof(info.m_caseSensitive), &cb, NULL);
			GetData(m_hstmt, 9, SQL_C_SSHORT, &info.m_searchable, sizeof(info.m_searchable), &cb, NULL);
			GetData(m_hstmt, 10, SQL_C_SSHORT, &info.m_unsigned, sizeof(info.m_unsigned), &cb, &info.m_unsignedIsNull);
			GetData(m_hstmt, 11, SQL_C_SSHORT, &info.m_fixedPrecisionScale, sizeof(info.m_fixedPrecisionScale), &cb, NULL);
			GetData(m_hstmt, 12, SQL_C_SSHORT, &info.m_autoUniqueValue, sizeof(info.m_autoUniqueValue), &cb, &info.m_autoUniqueValueIsNull);
			GetData(m_hstmt, 13, SQL_C_WCHAR, localTypeName, sizeof(localTypeName), &cb, &info.m_localTypeNameIsNull);
			info.m_localTypeName = localTypeName;
			GetData(m_hstmt, 14, SQL_C_SSHORT, &info.m_minimumScale, sizeof(info.m_minimumScale), &cb, &info.m_minimumScaleIsNull);
			GetData(m_hstmt, 15, SQL_C_SSHORT, &info.m_maximumScale, sizeof(info.m_maximumScale), &cb, &info.m_maximumScaleIsNull);
			GetData(m_hstmt, 16, SQL_C_SSHORT, &info.m_sqlDataType, sizeof(info.m_sqlDataType), &cb, NULL);
			GetData(m_hstmt, 17, SQL_C_SSHORT, &info.m_sqlDateTimeSub, sizeof(info.m_sqlDateTimeSub), &cb, &info.m_sqlDateTimeSubIsNull);
			GetData(m_hstmt, 18, SQL_C_SSHORT, &info.m_numPrecRadix, sizeof(info.m_numPrecRadix), &cb, &info.m_numPrecRadixIsNull);
			GetData(m_hstmt, 19, SQL_C_SSHORT, &info.m_intervalPrecision, sizeof(info.m_intervalPrecision), &cb, &info.m_intervalPrecisionIsNull);

			types.push_back(info);

			count++;
			ret = SQLFetch(m_hstmt);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return types;
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

		// Free statement handles
		// Let FreeStatementHandle throw, so a user can call its tables and then try to close again.
		m_hstmt = FreeStatementHandle(m_hstmt);
		m_hstmtExecSql = FreeStatementHandle(m_hstmtExecSql);

		// Try to disconnect from the data source
		SQLRETURN ret = SQLDisconnect(m_hdbc);
		THROW_IFN_SUCCEEDED(SQLDisconnect, ret, SQL_HANDLE_DBC, m_hdbc);

		LOG_DEBUG((boost::wformat(L"Closed connection to database %s") %m_dbInf.GetDbmsName()).str());
		m_dbIsOpen = false;
	}


	void Database::CommitTrans() const
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		// Commit the transaction
		SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_hdbc, SQL_COMMIT);
		THROW_IFN_SUCCEEDED_MSG(SQLEndTran, ret, SQL_HANDLE_DBC, m_hdbc, L"Failed to Commit Transaction");
	}


	void Database::RollbackTrans() const
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		// Rollback the transaction
		SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_hdbc, SQL_ROLLBACK);
		THROW_IFN_SUCCEEDED_MSG(SQLEndTran, ret, SQL_HANDLE_DBC, m_hdbc, L"Failed to Rollback Transaction");
	}


	void Database::ExecSql(const std::wstring& sqlStmt, ExecFailMode mode /* = NotFailOnNoData */)
	{
		exASSERT(IsOpen());

		RETCODE retcode;

		CloseStmtHandle(m_hstmtExecSql, StmtCloseMode::IgnoreNotOpen);

		retcode = SQLExecDirect(m_hstmtExecSql, (SQLWCHAR*)sqlStmt.c_str(), SQL_NTS);
		if ( ! SQL_SUCCEEDED(retcode))
		{
			if (!(mode == ExecFailMode::NotFailOnNoData && retcode == SQL_NO_DATA))
			{
				SqlResultException ex(L"SQLExecDirect", retcode, SQL_HANDLE_STMT, m_hstmtExecSql, (boost::wformat(L"Failed to execute Stmt '%s'") % sqlStmt).str());
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
		}
	}


	TableInfosVector Database::FindTables(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType) const
	{
		exASSERT(IsOpen());

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_hstmt, true, true);

		TableInfosVector tables;

		std::unique_ptr<SQLWCHAR[]> buffCatalog(new SQLWCHAR[m_dbInf.GetMaxCatalogNameLen()]);
		std::unique_ptr<SQLWCHAR[]> buffSchema(new SQLWCHAR[m_dbInf.GetMaxSchemaNameLen()]);
		std::unique_ptr<SQLWCHAR[]> buffTableName(new SQLWCHAR[m_dbInf.GetMaxTableNameLen()]);
		std::unique_ptr<SQLWCHAR[]> buffTableType(new SQLWCHAR[DB_MAX_TABLE_TYPE_LEN]);
		std::unique_ptr<SQLWCHAR[]> buffTableRemarks(new SQLWCHAR[DB_MAX_TABLE_REMARKS_LEN]);
		bool isCatalogNull = false;
		bool isSchemaNull = false;

		// Query db
		SQLRETURN ret = SQLTables(m_hstmt,
			catalogName.empty() ? NULL : (SQLWCHAR*) catalogName.c_str(), catalogName.empty() ? NULL : SQL_NTS,   // catname                 
			schemaName.empty() ? NULL : (SQLWCHAR*) schemaName.c_str(), schemaName.empty() ? NULL : SQL_NTS,   // schema name
			tableName.empty() ? NULL : (SQLWCHAR*) tableName.c_str(), tableName.empty() ? NULL : SQL_NTS,							// table name
			tableType.empty() ? NULL : (SQLWCHAR*) tableType.c_str(), tableType.empty() ? NULL : SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLTables, ret, SQL_HANDLE_STMT, m_hstmt);

		while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)
		{
			buffCatalog[0] = 0;
			buffSchema[0] = 0;
			buffTableName[0] = 0;
			buffTableType[0] = 0;
			buffTableRemarks[0] = 0;

			SQLLEN cb;
			GetData(m_hstmt, 1, SQL_C_WCHAR, buffCatalog.get(), m_dbInf.GetMaxCatalogNameLen() * sizeof(SQLWCHAR), &cb, &isCatalogNull);
			GetData(m_hstmt, 2, SQL_C_WCHAR, buffSchema.get(), m_dbInf.GetMaxSchemaNameLen() * sizeof(SQLWCHAR), &cb, &isSchemaNull);
			GetData(m_hstmt, 3, SQL_C_WCHAR, buffTableName.get(), m_dbInf.GetMaxTableNameLen() * sizeof(SQLWCHAR), &cb, NULL);
			GetData(m_hstmt, 4, SQL_C_WCHAR, buffTableType.get(), DB_MAX_TABLE_TYPE_LEN * sizeof(SQLWCHAR), &cb, NULL);
			GetData(m_hstmt, 5, SQL_C_WCHAR, buffTableRemarks.get(), DB_MAX_TABLE_REMARKS_LEN * sizeof(SQLWCHAR), &cb, NULL);

			TableInfo table(buffTableName.get(), buffTableType.get(), buffTableRemarks.get(), buffCatalog.get(), buffSchema.get(), isCatalogNull, isSchemaNull, GetDbms());
			tables.push_back(table);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return tables;
	}


	int Database::ReadColumnCount(const TableInfo& table)
	{
		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_hstmt, true, true);

		// Note: The schema and table name arguments are Pattern Value arguments
		// The catalog name is an ordinary argument. if we do not have one in the
		// DbCatalogTable, we set it to an empty string
		std::wstring catalogQueryName = L"";
		if (table.HasCatalog())
		{
			catalogQueryName = table.GetCatalog();
		}

		// Query columns, note:
		// we always have a tablename, but only sometimes a schema
		int colCount = 0;
		SQLRETURN ret = SQLColumns(m_hstmt,
				(SQLWCHAR*) catalogQueryName.c_str(), SQL_NTS,	// catalog
				table.HasSchema() ? (SQLWCHAR*) table.GetSchema().c_str() : NULL, table.HasSchema() ? SQL_NTS : NULL,	// schema
				(SQLWCHAR*) table.GetPureName().c_str(), SQL_NTS,		// tablename
				NULL, 0);						// All columns

		THROW_IFN_SUCCEEDED(SQLColumns, ret, SQL_HANDLE_STMT, m_hstmt);

		// Count the columns
		while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)
		{
			++colCount;
		}
		THROW_IFN_NO_DATA(SQLColumns, ret);

		return colCount;
	}


	int Database::ReadColumnCount(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType)
	{
		// Find one matching table
		TableInfo table = FindOneTable(tableName, schemaName, catalogName, tableType);

		// Forward the call		
		return ReadColumnCount(table);
	}


	TablePrimaryKeysVector Database::ReadTablePrimaryKeys(const TableInfo& table) const
	{
		exASSERT(IsOpen());
		// Access returns 'SQLSTATE IM001; Native Error: 0; [Microsoft][ODBC Driver Manager] Driver does not support this function' for SQLPrimaryKeys
		exASSERT(GetDbms() != DatabaseProduct::ACCESS);

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_hstmt, true, true);

		TablePrimaryKeysVector primaryKeys;

		SQLRETURN ret = SQLPrimaryKeys(m_hstmt, 
			table.HasCatalog() ? (SQLWCHAR*)table.GetCatalog().c_str() : NULL, table.HasCatalog() ? SQL_NTS : NULL,
			table.HasSchema() ? (SQLWCHAR*)table.GetSchema().c_str() : NULL, table.HasSchema() ? SQL_NTS : NULL,
			(SQLWCHAR*)table.GetPureName().c_str(), SQL_NTS);

		THROW_IFN_SUCCEEDED(SQLPrimaryKeys, ret, SQL_HANDLE_STMT, m_hstmt);

		while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)
		{
			SQLLEN cb;
			std::wstring catalogName, schemaName, tableName, columnName, keyName;
			bool isCatalogNull, isSchemaNull, isKeyNameNull;
			SQLSMALLINT keySequence;
			GetData(m_hstmt, 1, m_dbInf.GetMaxCatalogNameLen(), catalogName, &isCatalogNull);
			GetData(m_hstmt, 2, m_dbInf.GetMaxSchemaNameLen(), schemaName, &isSchemaNull);
			GetData(m_hstmt, 3, m_dbInf.GetMaxTableNameLen(), tableName);
			GetData(m_hstmt, 4, m_dbInf.GetMaxColumnNameLen(), columnName);
			GetData(m_hstmt, 5, SQL_C_SHORT, &keySequence, sizeof(keySequence), &cb, NULL);
			GetData(m_hstmt, 6, DB_MAX_PRIMARY_KEY_NAME_LEN, keyName, &isKeyNameNull);
			TablePrimaryKeyInfo pk(catalogName, schemaName, tableName, columnName, keySequence, keyName, isCatalogNull, isSchemaNull, isKeyNameNull);
			primaryKeys.push_back(pk);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return primaryKeys;
	}


	TablePrivilegesVector Database::ReadTablePrivileges(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType) const
	{
		exASSERT(IsOpen());

		// Find one matching table
		TableInfo table = FindOneTable(tableName, schemaName, catalogName, tableType);

		// Forward the call		
		return ReadTablePrivileges(table);
	}


	TablePrivilegesVector Database::ReadTablePrivileges(const TableInfo& table) const
	{
		exASSERT(IsOpen());
		exASSERT_MSG(GetDbms() != DatabaseProduct::ACCESS, L"Access reports 'SQLSTATE IM001; Driver does not support this function' for SQLTablePrivileges");

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_hstmt, true, true);

		TablePrivilegesVector privileges;

		// Note: The schema and table name arguments are Pattern Value arguments
		// The catalog name is an ordinary argument. if we do not have one in the
		// DbCatalogTable, we set it to an empty string
		std::wstring catalogQueryName = L"";
		if (table.HasCatalog())
		{
			catalogQueryName = table.GetCatalog();
		}

		// Query privs
		// we always have a tablename, but only sometimes a schema
		SQLRETURN ret = SQLTablePrivileges(m_hstmt,
			(SQLWCHAR*) catalogQueryName.c_str(), SQL_NTS,
			table.HasSchema() ? (SQLWCHAR*) table.GetSchema().c_str() : NULL, table.HasSchema() ? SQL_NTS : NULL,
			(SQLWCHAR*) table.GetPureName().c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLTablePrivileges, ret, SQL_HANDLE_STMT, m_hstmt);

		while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)
		{

			STablePrivilegesInfo priv;
			GetData(m_hstmt, 1, m_dbInf.GetMaxCatalogNameLen(), priv.m_catalogName, &priv.m_isCatalogNull);
			GetData(m_hstmt, 2, m_dbInf.GetMaxSchemaNameLen(), priv.m_schemaName, &priv.m_isSchemaNull);
			GetData(m_hstmt, 3, m_dbInf.GetMaxTableNameLen(), priv.m_tableName);
			GetData(m_hstmt, 4, DB_MAX_GRANTOR_LEN, priv.m_grantor, &priv.m_isGrantorNull);
			GetData(m_hstmt, 5, DB_MAX_GRANTEE_LEN, priv.m_grantee);
			GetData(m_hstmt, 6, DB_MAX_PRIVILEGES_LEN, priv.m_privilege);
			GetData(m_hstmt, 7, DB_MAX_IS_GRANTABLE_LEN, priv.m_grantable, &priv.m_isGrantableNull);

			privileges.push_back(priv);
		}

		THROW_IFN_NO_DATA(SQLFetch, ret);

		return privileges;
	}


	ColumnInfosVector Database::ReadTableColumnInfo(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType) const
	{
		exASSERT(IsOpen());

		// Find one matching table
		TableInfo table = FindOneTable(tableName, schemaName, catalogName, tableType);

		// Forward the call		
		return ReadTableColumnInfo(table);
	}


	ColumnInfosVector Database::ReadTableColumnInfo(const TableInfo& table) const
	{
		exASSERT(IsOpen());

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_hstmt, true, true);

		// Clear result
		ColumnInfosVector columns;

		// Note: The schema and table name arguments are Pattern Value arguments
		// The catalog name is an ordinary argument. if we do not have one in the
		// DbCatalogTable, we set it to an empty string
		std::wstring catalogQueryName = L"";
		if (table.HasCatalog())
		{
			catalogQueryName = table.GetCatalog();
		}

		// Query columns
		// we always have a tablename, but only sometimes a schema		
		int colCount = 0;
		SQLRETURN ret = SQLColumns(m_hstmt,
			(SQLWCHAR*)catalogQueryName.c_str(), SQL_NTS,	// catalog
			table.HasSchema() ? (SQLWCHAR*)table.GetSchema().c_str() : NULL, table.HasSchema() ? SQL_NTS : NULL,	// schema
			(SQLWCHAR*)table.GetPureName().c_str(), SQL_NTS,		// tablename
			NULL, 0);						// All columns

		THROW_IFN_SUCCEEDED(SQLColumns, ret, SQL_HANDLE_STMT, m_hstmt);

		// Iterate rows
		std::wstring catalogName, schemaName, tableName, columnName, typeName, remarks, defaultValue, isNullable;
		SQLSMALLINT sqlType, decimalDigits, numPrecRadix, nullable, sqlDataType, sqlDatetimeSub;
		SQLINTEGER columnSize, bufferSize, charOctetLength, ordinalPosition;
		bool isCatalogNull, isSchemaNull, isColumnSizeNull, isBufferSizeNull, isDecimalDigitsNull, isNumPrecRadixNull, isRemarksNull, isDefaultValueNull, isSqlDatetimeSubNull, isCharOctetLengthNull, isIsNullableNull;

		// Ensure ordinal-position is increasing constantly by one, starting at one
		SQLINTEGER m_lastIndex = 0;
		while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)
		{
			// Fetch data from columns

			SQLLEN cb;
			GetData(m_hstmt, 1, m_dbInf.GetMaxCatalogNameLen(), catalogName, &isCatalogNull);
			GetData(m_hstmt, 2, m_dbInf.GetMaxSchemaNameLen(), schemaName, &isSchemaNull);
			GetData(m_hstmt, 3, m_dbInf.GetMaxTableNameLen(), tableName);
			GetData(m_hstmt, 4, m_dbInf.GetMaxColumnNameLen(), columnName);
			GetData(m_hstmt, 5, SQL_C_SSHORT, &sqlType, sizeof(sqlType), &cb, NULL);
			GetData(m_hstmt, 6, DB_MAX_TYPE_NAME_LEN, typeName);
			GetData(m_hstmt, 7, SQL_C_SLONG, &columnSize, sizeof(columnSize), &cb, &isColumnSizeNull);
			GetData(m_hstmt, 8, SQL_C_SLONG, &bufferSize, sizeof(bufferSize), &cb, &isBufferSizeNull);
			GetData(m_hstmt, 9, SQL_C_SSHORT, &decimalDigits, sizeof(decimalDigits), &cb, &isDecimalDigitsNull);
			GetData(m_hstmt, 10, SQL_C_SSHORT, &numPrecRadix, sizeof(numPrecRadix), &cb, &isNumPrecRadixNull);
			GetData(m_hstmt, 11, SQL_C_SSHORT, &nullable, sizeof(nullable), &cb, NULL);
			GetData(m_hstmt, 12, DB_MAX_COLUMN_REMARKS_LEN, remarks, &isRemarksNull);
			GetData(m_hstmt, 13, DB_MAX_COLUMN_DEFAULT_LEN, defaultValue, &isDefaultValueNull);
			GetData(m_hstmt, 14, SQL_C_SSHORT, &sqlDataType, sizeof(sqlDataType), &cb, NULL);
			GetData(m_hstmt, 15, SQL_C_SSHORT, &sqlDatetimeSub, sizeof(sqlDatetimeSub), &cb, &isSqlDatetimeSubNull);
			GetData(m_hstmt, 16, SQL_C_SLONG, &charOctetLength, sizeof(charOctetLength), &cb, &isCharOctetLengthNull);
			GetData(m_hstmt, 17, SQL_C_SLONG, &ordinalPosition, sizeof(ordinalPosition), &cb, NULL);
			GetData(m_hstmt, 18, DB_MAX_YES_NO_LEN, isNullable, &isIsNullableNull);

			if (++m_lastIndex != ordinalPosition)
			{
				Exception ex(L"Columns are not ordered strictly by ordinal position");
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}

			ColumnInfo colInfo(catalogName, schemaName, tableName, columnName, sqlType, typeName, columnSize, bufferSize,
				decimalDigits, numPrecRadix, nullable, remarks, defaultValue, sqlDataType, sqlDatetimeSub, charOctetLength, ordinalPosition, isNullable,
				isCatalogNull, isSchemaNull, isColumnSizeNull, isBufferSizeNull, isDecimalDigitsNull, isNumPrecRadixNull, isRemarksNull, isDefaultValueNull,
				isSqlDatetimeSubNull, isIsNullableNull);
			columns.push_back(colInfo);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return columns;
	}


	SDbCatalogInfo Database::ReadCompleteCatalog()
	{
		SDbCatalogInfo dbInf;

		dbInf.m_tables = FindTables(L"", L"", L"", L"");

		TableInfosVector::const_iterator it;
		for(it = dbInf.m_tables.begin(); it != dbInf.m_tables.end(); it++)
		{
			const TableInfo& table = *it;
			if (table.HasCatalog())
			{
				dbInf.m_catalogs.insert(table.GetCatalog());
			}
			if (table.HasSchema())
			{
				dbInf.m_schemas.insert(table.GetSchema());
			}
		}
		return dbInf;
	}


	CommitMode Database::ReadCommitMode()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		CommitMode mode = CommitMode::UNKNOWN;

		SQLULEN modeValue = 0;
		SQLINTEGER cb;
		SQLRETURN ret = SQLGetConnectAttr(m_hdbc, SQL_ATTR_AUTOCOMMIT, &modeValue, sizeof(modeValue), &cb);
		THROW_IFN_SUCCEEDED_MSG(SQLGetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc, L"Failed to read Attr SQL_ATTR_AUTOCOMMIT");

		if(modeValue == SQL_AUTOCOMMIT_OFF)
			mode = CommitMode::MANUAL;
		else if(modeValue == SQL_AUTOCOMMIT_ON)
			mode = CommitMode::AUTO;

		m_commitMode = mode;

		return mode;
	}


	TransactionIsolationMode Database::ReadTransactionIsolationMode()
	{
		SQLULEN modeValue = 0;
		SQLINTEGER cb;
		SQLRETURN ret = SQLGetConnectAttr(m_hdbc, SQL_ATTR_TXN_ISOLATION, &modeValue, sizeof(modeValue), &cb);
		THROW_IFN_SUCCEEDED_MSG(SQLGetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc, L"Failed to read Attr SQL_ATTR_TXN_ISOLATION");

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
#if HAVE_MSODBCSQL_H
		case SQL_TXN_SS_SNAPSHOT:
			return TransactionIsolationMode::SNAPSHOT;
#endif
		}

		return TransactionIsolationMode::UNKNOWN;
	}

	void Database::SetCommitMode(CommitMode mode)
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		// If Autocommit is off, we need to rollback any ongoing transaction
		// Else at least MS SQL Server will complain that an ongoing transaction has been committed.
		if (GetCommitMode() != CommitMode::AUTO)
		{
			RollbackTrans();
		}

		SQLRETURN ret;
		std::wstring errStringMode;
		if (mode == CommitMode::MANUAL)
		{
			ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, NULL);
			errStringMode = L"SQL_AUTOCOMMIT_OFF";
		}
		else
		{
			ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_ON, NULL);
			errStringMode = L"SQL_AUTOCOMMIT_ON";
		}
		THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc, (boost::wformat(L"Setting ATTR_AUTOCOMMIT to %s failed") % errStringMode).str());

		m_commitMode = mode;
	}


	void Database::SetTransactionIsolationMode(TransactionIsolationMode mode)
	{	
		// We need to ensure cursors are closed:
		// The internal statement should be closed, the exec-statement could be open
		CloseStmtHandle(m_hstmt, StmtCloseMode::IgnoreNotOpen);
		CloseStmtHandle(m_hstmtExecSql, StmtCloseMode::IgnoreNotOpen);

		// If Autocommit is off, we need to Rollback any ongoing transaction
		// Else at least MS SQL Server will complain that an ongoing transaction has been committed.
		if (GetCommitMode() != CommitMode::AUTO)
		{
			RollbackTrans();
		}

		SQLRETURN ret;
		std::wstring errStringMode;
#if HAVE_MSODBCSQL_H
		if (mode == TransactionIsolationMode::SNAPSHOT)
		{
			// Its confusing: MsSql Server 2014 seems to be unable to change the snapshot isolation if the commit mode is not set to autocommit
			// If we do not set it to auto first, the next statement executed will complain that it was started under a different isolation mode than snapshot
			bool wasManualCommit = false;
			if (GetCommitMode() == CommitMode::MANUAL)
			{
				wasManualCommit = true;
				SetCommitMode(CommitMode::AUTO);
			}
			ret = SQLSetConnectAttr(m_hdbc, (SQL_COPT_SS_TXN_ISOLATION), (SQLPOINTER)mode, NULL);
			if (wasManualCommit)
			{
				SetCommitMode(CommitMode::MANUAL);
			}
		}
		else
#endif
		{
			ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)mode, NULL);
		}
		THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc, (boost::wformat(L"Cannot set SQL_ATTR_TXN_ISOLATION to %d") % (int) mode).str());

	}

	
	bool Database::CanSetTransactionIsolationMode(TransactionIsolationMode mode) const
	{
		SQLUINTEGER txnIsolationOpts = m_dbInf.GetUIntProperty(DatabaseInfo::UIntProperty::TxnIsolationOption);
		return (txnIsolationOpts & (SQLUINTEGER)mode) != 0;
	}


	OdbcVersion Database::GetDriverOdbcVersion() const
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		std::wstring driverOdbcVersion = m_dbInf.GetWStringProperty(DatabaseInfo::WStringProperty::DriverOdbcVersion);
		exASSERT( ! driverOdbcVersion.empty());

		OdbcVersion ov = OdbcVersion::UNKNOWN;
		std::vector<std::wstring> versions;
		boost::split(versions, driverOdbcVersion, boost::is_any_of(L"."));
		if (versions.size() == 2)
		{
			try
			{
				short major = boost::lexical_cast<short>(versions[0]);
				short minor = boost::lexical_cast<short>(versions[1]);
				if (major >= 3 && minor >= 80)
				{
					ov = OdbcVersion::V_3_8;
				}
				else if (major >= 3)
				{
					ov = OdbcVersion::V_3;
				}
				else if (major >= 2)
				{
					ov = OdbcVersion::V_2;
				}
			}
			catch (boost::bad_lexical_cast& e)
			{
				HIDE_UNUSED(e);
				THROW_WITH_SOURCE(Exception, (boost::wformat(L"Failed to determine odbc version from string '%s'") % driverOdbcVersion).str());
			}
		}
		return ov;
	}


	OdbcVersion Database::GetMaxSupportedOdbcVersion() const
	{
		OdbcVersion driverVersion = GetDriverOdbcVersion();
		if (driverVersion >= m_pEnv->GetOdbcVersion())
		{
			return m_pEnv->GetOdbcVersion();
		}
		return driverVersion;
	}


	void Database::DetectDbms()
	{
		std::wstring name = m_dbInf.GetDbmsName();
		if (boost::algorithm::contains(name, L"Microsoft SQL Server"))
		{
			m_dbmsType = DatabaseProduct::MS_SQL_SERVER;
		}
		else if (boost::algorithm::contains(name, L"MySQL"))
		{
			m_dbmsType = DatabaseProduct::MY_SQL;
		}
		else if (boost::algorithm::contains(name, L"DB2"))
		{
			m_dbmsType = DatabaseProduct::DB2;
		}
		else if (boost::algorithm::contains(name, L"EXCEL"))
		{
			m_dbmsType = DatabaseProduct::EXCEL;
		}
		else if (boost::algorithm::contains(name, L"ACCESS"))
		{
			m_dbmsType = DatabaseProduct::ACCESS;
		}

		if (m_dbmsType == DatabaseProduct::UNKNOWN)
		{
			LOG_WARNING((boost::wformat(L"Unknown database: %s") % m_dbInf.GetDbmsName()).str());
		}
	}
}