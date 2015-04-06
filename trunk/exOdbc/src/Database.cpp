/*!
* \file Database.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 25.07.2014
* \brief Source file for the Database class and its helpers.
* \copyright wxWindows Library Licence, Version 3.1
*
* Source file for the Database class and its helpers.
* This file was originally wx/db.cpp from wxWidgets 2.8.
* Most of the code has been rewritten, a lot of functionality
* not needed and not tested so far has been dropped.
*
* For completion, here follows the old wxWidgets header:
*
* ///////////////////////////////////////////////////////////////////////////////<br>
* // Name:        src/common/db.cpp<br>
* // Purpose:     Implementation of the wxDb class.  The wxDb class represents a connection<br>
* //              to an ODBC data source.  The wxDb class allows operations on the data<br>
* //              source such as OpenImpling and closing the data source.<br>
* // Author:      Doug Card<br>
* // Modified by: George Tasker<br>
* //              Bart Jourquin<br>
* //              Mark Johnson, wxWindows@mj10777.de<br>
* // Mods:        Dec, 1998:<br>
* //                -Added support for SQL statement logging and database cataloging<br>
* // Mods:        April, 1999<br>
* //                -Added QUERY_ONLY mode support to reduce default number of cursors<br>
* //                -Added additional SQL logging code<br>
* //                -Added DEBUG-ONLY tracking of wxTable objects to detect orphaned DB connections<br>
* //                -Set ODBC option to only read committed writes to the DB so all<br>
* //                   databases operate the same in that respect<br>
* // Created:     9.96<br>
* // RCS-ID:      $Id: db.cpp 52489 2008-03-14 14:14:57Z JS $<br>
* // Copyright:   (c) 1996 Remstar International, Inc.<br>
* // Licence:     wxWindows licence<br>
* ///////////////////////////////////////////////////////////////////////////////<br>
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

namespace exodbc
{
	// Construction
	// ------------
	Database::Database()
	{
		// Note: Init will set members to NULL
		Initialize();
	}


	Database::Database(const Environment& env)
	{
		// Note: Init will set members to NULL
		Initialize();

		// Allocate the DBC-Handle and set the member m_pEnv
		AllocateConnectionHandle(env);
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
		catch (Exception ex)
		{
			LOG_ERROR(ex.ToString());
		}
	}


	// Implementation
	// --------------
	void Database::Initialize()
	{
		// Handles created by this db
		m_hdbc = SQL_NULL_HDBC;
		m_hstmt = SQL_NULL_HSTMT;
		m_hstmtExecSql = SQL_NULL_HSTMT;

		// Environment ODBC-Version unknown
		m_envOdbcVersion = OdbcVersion::UNKNOWN;

		// GetDbms unknwon
		m_dbmsType = DatabaseProduct::UNKNOWN;

		// Mark database as not Open as of yet
		m_dbIsOpen = false;
		m_dbOpenedWithConnectionString = false;

		// commit is not known until connected and set
		m_commitMode = CommitMode::UNKNOWN;
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


	void Database::AllocateConnectionHandle(const Environment& env)
	{
		exASSERT(!HasConnectionHandle());
		exASSERT(env.HasEnvironmentHandle());

		// Allocate the DBC-Handle
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DBC, env.GetEnvironmentHandle(), &m_hdbc);
		THROW_IFN_SUCCEEDED(SQLAllocHandle, ret, SQL_HANDLE_ENV, env.GetEnvironmentHandle());

		// Read the environment odbc-version
		m_envOdbcVersion = env.ReadOdbcVersion();
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
			OdbcVersion connectionVersion = GetDriverOdbcVersion();
			if (m_envOdbcVersion > connectionVersion)
			{
				// \todo: Probably we should throw here and fail totally
				LOG_WARNING((boost::wformat(L"ODBC Version missmatch: Environment requested %d, but the driver (name: '%s' version: '%s') reported %d ('%s'). The Database ('%s') will be using %d") % (int) m_envOdbcVersion %m_dbInf.m_driverName %m_dbInf.m_driverVer % (int) connectionVersion %m_dbInf.m_odbcVer %m_dbInf.m_databaseName % (int) connectionVersion).str());
			}

			// Try to detect the type - this will update our internal type
			DetectDbms();

			// Set Connection Options
			SetConnectionAttributes();

			// Default to manual commit, if the Database is able to set a commit mode. Anyway read the currently active mode, we need to know that
			m_commitMode = ReadCommitMode();
			if (GetSupportsTransactions() && m_commitMode != CommitMode::MANUAL)
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
		LOG_DEBUG((boost::wformat(L"Opened connection to database %s") %m_dbInf.m_dbmsName).str());
	}


	void Database::Open(const std::wstring& inConnectStr)
	{
		exASSERT(inConnectStr.length() > 0);
		Open(inConnectStr, NULL);
	}


	void Database::Open(const std::wstring& inConnectStr, SQLHWND parentWnd)
	{
		m_dsn = L"";
		m_uid = L"";
		m_authStr = L"";

		SQLRETURN retcode;

		// TODO: See notes about forwardCursor in Open Method with dsn, user, pass 
		
		// Connect to the data source
		SQLWCHAR outConnectBuffer[SQL_MAX_CONNECTSTR_LEN + 1];  // MS recommends at least 1k buffer
		SQLSMALLINT outConnectBufferLen;

		m_inConnectionStr = inConnectStr;

		// Note: 
		// StringLength1: [Input] Length of *InConnectionString, in characters if the string is Unicode, or bytes if string is ANSI or DBCS.
		// BufferLength: [Input] Length of the *OutConnectionString buffer, in characters.
		retcode = SQLDriverConnect(m_hdbc, parentWnd, 
			(SQLWCHAR*) m_inConnectionStr.c_str(),
			m_inConnectionStr.length(), 
			(SQLWCHAR*) outConnectBuffer, 
			SQL_MAX_CONNECTSTR_LEN, &outConnectBufferLen, SQL_DRIVER_COMPLETE);

		THROW_IFN_SUCCEEDED(SQLDriverConnect, retcode, SQL_HANDLE_DBC, m_hdbc);

		outConnectBuffer[outConnectBufferLen] = 0;
		m_outConnectionStr = outConnectBuffer;
		m_dbOpenedWithConnectionString = true;

		OpenImpl();
	}


	void Database::Open(const std::wstring& dsn, const std::wstring& uid, const std::wstring& authStr)
	{
		exASSERT(!IsOpen());
		exASSERT(!dsn.empty());
		exASSERT(HasConnectionHandle());
		exASSERT(m_hstmt == SQL_NULL_HSTMT);
		exASSERT(m_hstmtExecSql == SQL_NULL_HSTMT);

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


	void Database::SetConnectionAttributes()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		SQLRETURN ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_TRACE, (SQLPOINTER) SQL_OPT_TRACE_OFF, NULL);
		THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_hdbc, L"Cannot set ATTR_TRACE to OPT_TRACE_OFF");

		// Note: This is unsupported SQL_ATTR_METADATA_ID by most drivers. It should default to OFF
		//ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_TRUE, NULL);
		//if(ret != SQL_SUCCESS)
		//{
		//	LOG_ERROR_DBC_MSG(m_hdbc, ret, SQLSetConnectAttr, L"Cannot set ATTR_METADATA_ID to SQL_FALSE");
		//	ok = false;
		//}
	}


	SDbInfo Database::ReadDbInfo()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		SDbInfo dbInf;
		SWORD cb;

		// SQLGetInfo gets null-terminated by the driver. It needs buffer-lengths (not char-lengts), even in unicode
		// see http://msdn.microsoft.com/en-us/library/ms711681%28v=vs.85%29.aspx
		// so it works with sizeof and statically declared arrays
		
		GetInfo(m_hdbc, SQL_SERVER_NAME, dbInf.m_serverName);
		GetInfo(m_hdbc, SQL_DATABASE_NAME, dbInf.m_databaseName);
		GetInfo(m_hdbc, SQL_DBMS_NAME, dbInf.m_dbmsName);
		GetInfo(m_hdbc, SQL_DBMS_VER, dbInf.m_dbmsVer);
		GetInfo(m_hdbc, SQL_MAX_DRIVER_CONNECTIONS, &dbInf.m_maxConnections, sizeof(dbInf.m_maxConnections), &cb);
		GetInfo(m_hdbc, SQL_MAX_CONCURRENT_ACTIVITIES, &dbInf.m_maxStmts, sizeof(dbInf.m_maxStmts), &cb);
		GetInfo(m_hdbc, SQL_DRIVER_NAME, dbInf.m_driverName);
		GetInfo(m_hdbc, SQL_DRIVER_ODBC_VER, dbInf.m_odbcVer);
		GetInfo(m_hdbc, SQL_ODBC_VER, dbInf.m_drvMgrOdbcVer);
		GetInfo(m_hdbc, SQL_DRIVER_VER, dbInf.m_driverVer);
		GetInfo(m_hdbc, SQL_ODBC_SAG_CLI_CONFORMANCE, &dbInf.m_cliConfLvl, sizeof(dbInf.m_cliConfLvl), &cb);
		GetInfo(m_hdbc, SQL_OUTER_JOINS, dbInf.m_outerJoins);
		GetInfo(m_hdbc, SQL_PROCEDURES, dbInf.m_procedureSupport);
		GetInfo(m_hdbc, SQL_ACCESSIBLE_TABLES, dbInf.m_accessibleTables);
		GetInfo(m_hdbc, SQL_CURSOR_COMMIT_BEHAVIOR, &dbInf.m_cursorCommitBehavior, sizeof(dbInf.m_cursorCommitBehavior), &cb);
		GetInfo(m_hdbc, SQL_CURSOR_ROLLBACK_BEHAVIOR, &dbInf.m_cursorRollbackBehavior, sizeof(dbInf.m_cursorRollbackBehavior), &cb);
		GetInfo(m_hdbc, SQL_NON_NULLABLE_COLUMNS, &dbInf.m_supportNotNullClause, sizeof(dbInf.m_supportNotNullClause), &cb);
		GetInfo(m_hdbc, SQL_ODBC_SQL_OPT_IEF, dbInf.m_supportIEF);
		GetInfo(m_hdbc, SQL_DEFAULT_TXN_ISOLATION, &dbInf.m_txnIsolation, sizeof(dbInf.m_txnIsolation), &cb);
		GetInfo(m_hdbc, SQL_TXN_ISOLATION_OPTION, &dbInf.m_txnIsolationOptions, sizeof(dbInf.m_txnIsolationOptions), &cb);
		GetInfo(m_hdbc, SQL_POS_OPERATIONS, &dbInf.m_posOperations, sizeof(dbInf.m_posOperations), &cb);
		GetInfo(m_hdbc, SQL_POSITIONED_STATEMENTS, &dbInf.m_posStmts, sizeof(dbInf.m_posStmts), &cb);
		GetInfo(m_hdbc, SQL_SCROLL_OPTIONS, &dbInf.m_scrollOptions, sizeof(dbInf.m_scrollOptions), &cb);
		GetInfo(m_hdbc, SQL_TXN_CAPABLE, &dbInf.m_txnCapable, sizeof(dbInf.m_txnCapable), &cb);
		GetInfo(m_hdbc, SQL_SEARCH_PATTERN_ESCAPE, dbInf.m_searchPatternEscape);

		// TODO: SQL_LOGIN_TIMEOUT is a Connection-Attribute
		//retcode = SQLGetInfo(m_hdbc, SQL_LOGIN_TIMEOUT, (UCHAR*) &dbInf.loginTimeout, sizeof(dbInf.loginTimeout), &cb);
		//if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		//{
		//	DispAllErrors(SQL_NULL_HENV, m_hdbc);
		//	if (failOnDataTypeUnsupported)
		//		return false;
		//}

		GetInfo(m_hdbc, SQL_MAX_CATALOG_NAME_LEN, &dbInf.m_maxCatalogNameLen, sizeof(dbInf.m_maxCatalogNameLen), &cb);
		GetInfo(m_hdbc,  SQL_MAX_SCHEMA_NAME_LEN, &dbInf.m_maxSchemaNameLen, sizeof(dbInf.m_maxSchemaNameLen), &cb);
		GetInfo(m_hdbc, SQL_MAX_TABLE_NAME_LEN, &dbInf.m_maxTableNameLen, sizeof(dbInf.m_maxTableNameLen), &cb);
		GetInfo(m_hdbc, SQL_MAX_COLUMN_NAME_LEN, &dbInf.m_maxColumnNameLen, sizeof(dbInf.m_maxColumnNameLen), &cb);

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


	STableInfo Database::FindOneTable(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType) const
	{
		// Query the tables that match
		STableInfosVector tables = FindTables(tableName, schemaName, catalogName, tableType);

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

		STableInfo table = tables[0];
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

		LOG_DEBUG((boost::wformat(L"Closed connection to database %s") %m_dbInf.m_dbmsName).str());
		m_dbIsOpen = false;
	}


	void Database::CommitTrans()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		// Commit the transaction
		SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_hdbc, SQL_COMMIT);
		THROW_IFN_SUCCEEDED_MSG(SQLEndTran, ret, SQL_HANDLE_DBC, m_hdbc, L"Failed to Commit Transaction");
	}


	void Database::RollbackTrans()
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


	STableInfosVector Database::FindTables(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType) const
	{
		exASSERT(IsOpen());

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_hstmt, true, true);

		STableInfosVector tables;

		std::unique_ptr<SQLWCHAR[]> buffCatalog(new SQLWCHAR[m_dbInf.GetMaxCatalogNameLen()]);
		std::unique_ptr<SQLWCHAR[]> buffSchema(new SQLWCHAR[m_dbInf.GetMaxSchemaNameLen()]);
		std::unique_ptr<SQLWCHAR[]> buffTableName(new SQLWCHAR[m_dbInf.GetMaxTableNameLen()]);
		std::unique_ptr<SQLWCHAR[]> buffTableType(new SQLWCHAR[DB_MAX_TABLE_TYPE_LEN]);
		std::unique_ptr<SQLWCHAR[]> buffTableRemarks(new SQLWCHAR[DB_MAX_TABLE_REMARKS_LEN]);

		// Query db
		SQLRETURN ret = SQLTables(m_hstmt,
			catalogName.empty() ? NULL : (SQLWCHAR*) catalogName.c_str(), catalogName.empty() ? NULL : SQL_NTS,   // catname                 
			schemaName.empty() ? NULL : (SQLWCHAR*) schemaName.c_str(), schemaName.empty() ? NULL : SQL_NTS,   // schema name
			tableName.empty() ? NULL : (SQLWCHAR*) tableName.c_str(), tableName.empty() ? NULL : SQL_NTS,							// table name
			tableType.empty() ? NULL : (SQLWCHAR*) tableType.c_str(), tableType.empty() ? NULL : SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLTables, ret, SQL_HANDLE_STMT, m_hstmt);

		buffCatalog[0] = 0;
		buffSchema[0] = 0;
		buffTableName[0] = 0;
		buffTableType[0] = 0;
		buffTableRemarks[0] = 0;

		while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)
		{
			STableInfo table;
			SQLLEN cb;
			GetData(m_hstmt, 1, SQL_C_WCHAR, buffCatalog.get(), m_dbInf.GetMaxCatalogNameLen() * sizeof(SQLWCHAR), &cb, &table.m_isCatalogNull);
			GetData(m_hstmt, 2, SQL_C_WCHAR, buffSchema.get(), m_dbInf.GetMaxSchemaNameLen() * sizeof(SQLWCHAR), &cb, &table.m_isSchemaNull);
			GetData(m_hstmt, 3, SQL_C_WCHAR, buffTableName.get(), m_dbInf.GetMaxTableNameLen() * sizeof(SQLWCHAR), &cb, NULL);
			GetData(m_hstmt, 4, SQL_C_WCHAR, buffTableType.get(), DB_MAX_TABLE_TYPE_LEN * sizeof(SQLWCHAR), &cb, NULL);
			GetData(m_hstmt, 5, SQL_C_WCHAR, buffTableRemarks.get(), DB_MAX_TABLE_REMARKS_LEN * sizeof(SQLWCHAR), &cb, NULL);

			if (!table.m_isCatalogNull)
			{
				table.m_catalogName = buffCatalog.get();
			}
			if (!table.m_isSchemaNull)
			{
				table.m_schemaName = buffSchema.get();
			}
			table.m_tableName = buffTableName.get();
			table.m_tableType = buffTableType.get();
			table.m_tableRemarks = buffTableRemarks.get();
			tables.push_back(table);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return tables;
	}


	int Database::ReadColumnCount(const STableInfo& table)
	{
		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_hstmt, true, true);

		// Note: The schema and table name arguments are Pattern Value arguments
		// The catalog name is an ordinary argument. if we do not have one in the
		// DbCatalogTable, we set it to an empty string
		std::wstring catalogQueryName = L"";
		if(!table.m_isCatalogNull)
			catalogQueryName = table.m_catalogName;

		// Query columns, note:
		// we always have a tablename, but only sometimes a schema
		int colCount = 0;
		SQLRETURN ret = SQLColumns(m_hstmt,
				(SQLWCHAR*) catalogQueryName.c_str(), SQL_NTS,	// catalog
				table.m_isSchemaNull ? NULL : (SQLWCHAR*) table.m_schemaName.c_str(), table.m_isSchemaNull ? NULL : SQL_NTS,	// schema
				(SQLWCHAR*) table.m_tableName.c_str(), SQL_NTS,		// tablename
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
		STableInfo table = FindOneTable(tableName, schemaName, catalogName, tableType);

		// Forward the call		
		return ReadColumnCount(table);
	}


	TablePrimaryKeysVector Database::ReadTablePrimaryKeys(const STableInfo& table) const
	{
		exASSERT(IsOpen());
		// Access returns 'SQLSTATE IM001; Native Error: 0; [Microsoft][ODBC Driver Manager] Driver does not support this function' for SQLPrimaryKeys
		exASSERT(GetDbms() != DatabaseProduct::ACCESS);

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_hstmt, true, true);

		TablePrimaryKeysVector primaryKeys;

		SQLRETURN ret = SQLPrimaryKeys(m_hstmt, 
			table.m_catalogName.empty() ? NULL : (SQLWCHAR*)table.m_catalogName.c_str(), table.m_catalogName.empty() ? 0 : SQL_NTS,
			table.m_schemaName.empty() ? NULL : (SQLWCHAR*)table.m_schemaName.c_str(), table.m_schemaName.empty() ? 0 : SQL_NTS,
			(SQLWCHAR*)table.m_tableName.c_str(), SQL_NTS);

		THROW_IFN_SUCCEEDED(SQLPrimaryKeys, ret, SQL_HANDLE_STMT, m_hstmt);

		while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)
		{
			SQLLEN cb;
			STablePrimaryKeyInfo pk;
			GetData(m_hstmt, 1, m_dbInf.GetMaxCatalogNameLen(), pk.m_catalogName, &pk.m_isCatalogNull);
			GetData(m_hstmt, 2, m_dbInf.GetMaxSchemaNameLen(), pk.m_schemaName, &pk.m_isSchemaNull);
			GetData(m_hstmt, 3, m_dbInf.GetMaxTableNameLen(), pk.m_tableName);
			GetData(m_hstmt, 4, m_dbInf.GetMaxColumnNameLen(), pk.m_columnName);
			GetData(m_hstmt, 5, SQL_C_SHORT, &pk.m_keySequence, sizeof(pk.m_keySequence), &cb, NULL);
			GetData(m_hstmt, 6, DB_MAX_PRIMARY_KEY_NAME_LEN, pk.m_primaryKeyName, &pk.m_isPrimaryKeyNameNull);

			primaryKeys.push_back(pk);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return primaryKeys;
	}


	TablePrivilegesVector Database::ReadTablePrivileges(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType) const
	{
		exASSERT(IsOpen());

		// Find one matching table
		STableInfo table = FindOneTable(tableName, schemaName, catalogName, tableType);

		// Forward the call		
		return ReadTablePrivileges(table);
	}


	TablePrivilegesVector Database::ReadTablePrivileges(const STableInfo& table) const
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
		if (!table.m_isCatalogNull)
		{
			catalogQueryName = table.m_catalogName;
		}

		// Query privs
		// we always have a tablename, but only sometimes a schema
		SQLRETURN ret = SQLTablePrivileges(m_hstmt,
			(SQLWCHAR*) catalogQueryName.c_str(), SQL_NTS,
			table.m_isSchemaNull ? NULL : (SQLWCHAR*) table.m_schemaName.c_str(), table.m_isSchemaNull ? NULL : SQL_NTS,
			(SQLWCHAR*) table.m_tableName.c_str(), SQL_NTS);
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
		STableInfo table = FindOneTable(tableName, schemaName, catalogName, tableType);

		// Forward the call		
		return ReadTableColumnInfo(table);
	}


	ColumnInfosVector Database::ReadTableColumnInfo(const STableInfo& table) const
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
		if (!table.m_isCatalogNull)
		{
			catalogQueryName = table.m_catalogName;
		}

		// Query columns
		// we always have a tablename, but only sometimes a schema		
		int colCount = 0;
		SQLRETURN ret = SQLColumns(m_hstmt,
			(SQLWCHAR*)catalogQueryName.c_str(), SQL_NTS,	// catalog
			table.m_isSchemaNull ? NULL : (SQLWCHAR*)table.m_schemaName.c_str(), table.m_isSchemaNull ? NULL : SQL_NTS,	// schema
			(SQLWCHAR*)table.m_tableName.c_str(), SQL_NTS,		// tablename
			NULL, 0);						// All columns

		THROW_IFN_SUCCEEDED(SQLColumns, ret, SQL_HANDLE_STMT, m_hstmt);

		// Iterate rows
		// Ensure ordinal-position is increasing constantly by one, starting at one
		SQLINTEGER m_lastIndex = 0;
		while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)
		{
			// Fetch data from columns

			SQLLEN cb;
			SColumnInfo colInfo;
			GetData(m_hstmt, 1, m_dbInf.GetMaxCatalogNameLen(), colInfo.m_catalogName, &colInfo.m_isCatalogNull);
			GetData(m_hstmt, 2, m_dbInf.GetMaxSchemaNameLen(), colInfo.m_schemaName, &colInfo.m_isSchemaNull);
			GetData(m_hstmt, 3, m_dbInf.GetMaxTableNameLen(), colInfo.m_tableName);
			GetData(m_hstmt, 4, m_dbInf.GetMaxColumnNameLen(), colInfo.m_columnName);
			GetData(m_hstmt, 5, SQL_C_SSHORT, &colInfo.m_sqlType, sizeof(colInfo.m_sqlType), &cb, NULL);
			GetData(m_hstmt, 6, DB_MAX_TYPE_NAME_LEN, colInfo.m_typeName);
			GetData(m_hstmt, 7, SQL_C_SLONG, &colInfo.m_columnSize, sizeof(colInfo.m_columnSize), &cb, &colInfo.m_isColumnSizeNull);
			GetData(m_hstmt, 8, SQL_C_SLONG, &colInfo.m_bufferSize, sizeof(colInfo.m_bufferSize), &cb, &colInfo.m_isBufferSizeNull);
			GetData(m_hstmt, 9, SQL_C_SSHORT, &colInfo.m_decimalDigits, sizeof(colInfo.m_decimalDigits), &cb, &colInfo.m_isDecimalDigitsNull);
			GetData(m_hstmt, 10, SQL_C_SSHORT, &colInfo.m_numPrecRadix, sizeof(colInfo.m_numPrecRadix), &cb, &colInfo.m_isNumPrecRadixNull);
			GetData(m_hstmt, 11, SQL_C_SSHORT, &colInfo.m_nullable, sizeof(colInfo.m_nullable), &cb, NULL);
			GetData(m_hstmt, 12, DB_MAX_COLUMN_REMARKS_LEN, colInfo.m_remarks, &colInfo.m_isRemarksNull);
			GetData(m_hstmt, 13, DB_MAX_COLUMN_DEFAULT_LEN, colInfo.m_defaultValue, &colInfo.m_isDefaultValueNull);
			GetData(m_hstmt, 14, SQL_C_SSHORT, &colInfo.m_sqlDataType, sizeof(colInfo.m_sqlDataType), &cb, NULL);
			GetData(m_hstmt, 15, SQL_C_SSHORT, &colInfo.m_sqlDatetimeSub, sizeof(colInfo.m_sqlDatetimeSub), &cb, &colInfo.m_isDatetimeSubNull);
			GetData(m_hstmt, 16, SQL_C_SLONG, &colInfo.m_charOctetLength, sizeof(colInfo.m_charOctetLength), &cb, &colInfo.m_isCharOctetLengthNull);
			GetData(m_hstmt, 17, SQL_C_SLONG, &colInfo.m_ordinalPosition, sizeof(colInfo.m_ordinalPosition), &cb, NULL);
			GetData(m_hstmt, 18, DB_MAX_YES_NO_LEN, colInfo.m_isNullable, &colInfo.m_isIsNullableNull);

			if (++m_lastIndex != colInfo.m_ordinalPosition)
			{
				Exception ex(L"Columns are not ordered strictly by ordinal position");
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}

			columns.push_back(colInfo);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return columns;
	}


	SDbCatalogInfo Database::ReadCompleteCatalog()
	{
		SDbCatalogInfo dbInf;

		dbInf.m_tables = FindTables(L"", L"", L"", L"");

		STableInfosVector::const_iterator it;
		for(it = dbInf.m_tables.begin(); it != dbInf.m_tables.end(); it++)
		{
			const STableInfo& table = *it;
			if(!table.m_isCatalogNull)
				dbInf.m_catalogs.insert(table.m_catalogName);
			if(!table.m_isSchemaNull)
				dbInf.m_schemas.insert(table.m_schemaName);
		}
		return dbInf;
	}


	CommitMode Database::ReadCommitMode()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_hdbc != SQL_NULL_HDBC);

		CommitMode mode = CommitMode::UNKNOWN;

		SQLUINTEGER modeValue;
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
		SQLUINTEGER modeValue;
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
		return (m_dbInf.m_txnIsolationOptions & (SQLUINTEGER) mode) != 0;
	}


	OdbcVersion Database::GetDriverOdbcVersion() const
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT( ! m_dbInf.m_odbcVer.empty());

		OdbcVersion ov = OdbcVersion::UNKNOWN;
		std::vector<std::wstring> versions;
		boost::split(versions, m_dbInf.m_odbcVer, boost::is_any_of(L"."));
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
			catch (boost::bad_lexical_cast e)
			{
				THROW_WITH_SOURCE(Exception, (boost::wformat(L"Failed to determine odbc version from string '%s'") % m_dbInf.m_odbcVer).str());
			}
		}
		return ov;
	}


	OdbcVersion Database::GetMaxSupportedOdbcVersion() const
	{
		OdbcVersion driverVersion = GetDriverOdbcVersion();
		if (driverVersion >= m_envOdbcVersion)
		{
			return m_envOdbcVersion;
		}
		return driverVersion;
	}


	void Database::DetectDbms()
	{
		if (boost::algorithm::contains(m_dbInf.m_dbmsName, L"Microsoft SQL Server"))
		{
			m_dbmsType = DatabaseProduct::MS_SQL_SERVER;
		}
		else if (boost::algorithm::contains(m_dbInf.m_dbmsName, L"MySQL"))
		{
			m_dbmsType = DatabaseProduct::MY_SQL;
		}
		else if (boost::algorithm::contains(m_dbInf.m_dbmsName, L"DB2"))
		{
			m_dbmsType = DatabaseProduct::DB2;
		}
		else if (boost::algorithm::contains(m_dbInf.m_dbmsName, L"EXCEL"))
		{
			m_dbmsType = DatabaseProduct::EXCEL;
		}
		else if (boost::algorithm::contains(m_dbInf.m_dbmsName, L"ACCESS"))
		{
			m_dbmsType = DatabaseProduct::ACCESS;
		}

		if (m_dbmsType == DatabaseProduct::UNKNOWN)
		{
			LOG_WARNING((boost::wformat(L"Unknown database: %s") % m_dbInf.m_dbmsName).str());
		}
	}


	// OLD STUFF we need to think about re-adding it
	// =============================================


	// SQL Log defaults to be used by GetDbConnection
	//	wxDbSqlLogState SQLLOGstate = sqlLogOFF;

	//	static std::wstring SQLLOGfn = SQL_LOG_FILENAME;

	//// This type defines the return row-struct form
	//// SQLTablePrivileges, and is used by wxDB::TablePrivileges.
	//typedef struct
	//{
	//	wchar_t        tableQual[128+1];
	//	wchar_t        tableOwner[128+1];
	//	wchar_t        tableName[128+1];
	//	wchar_t        grantor[128+1];
	//	wchar_t        grantee[128+1];
	//	wchar_t        privilege[128+1];
	//	wchar_t        grantable[3+1];
	//} wxDbTablePrivilegeInfo;

	// Construction
	// ------------

	///********** wxDbColInf Constructor **********/
	//ColumnInfo::ColumnInfo()
	//{
	//	Initialize();
	//}  // wxDbColInf::wxDbColInf()


	///********** wxDbColInf Destructor ********/
	//ColumnInfo::~ColumnInfo()
	//{
	//	if (m_pColFor)
	//		delete m_pColFor;
	//	m_pColFor = NULL;
	//}  // wxDbColInf::~wxDbColInf()


	//bool ColumnInfo::Initialize()
	//{
	//	m_catalog[0]      = 0;
	//	m_schema[0]       = 0;
	//	m_tableName[0]    = 0;
	//	m_colName[0]      = 0;
	//	m_sqlDataType     = 0;
	//	m_typeName[0]     = 0;
	//	m_columnLength    = 0;
	//	m_bufferSize      = 0;
	//	m_decimalDigits   = 0;
	//	m_numPrecRadix    = 0;
	//	m_nullable        = 0;
	//	m_remarks[0]      = 0;
	//	m_dbDataType      = 0;
	//	m_pkCol           = 0;
	//	m_pkTableName[0]  = 0;
	//	m_fkCol           = 0;
	//	m_fkTableName[0]  = 0;
	//	m_pColFor         = NULL;

	//	return true;
	//}  // wxDbColInf::Initialize()

	///********** wxDb::Catalog() **********/
	//bool Database::Catalog(const wchar_t *userID, const std::wstring &fileName)
	//	/*
	//	* Creates the text file specified in 'filename' which will contain
	//	* a minimal data dictionary of all tables accessible by the user specified
	//	* in 'userID'
	//	*
	//	* userID is evaluated in the following manner:
	//	*        userID == NULL  ... UserID is ignored
	//	*        userID == ""    ... UserID set equal to 'this->uid'
	//	*        userID != ""    ... UserID set equal to 'userID'
	//	*
	//	* NOTE: ALL column bindings associated with this wxDb instance are unbound
	//	*       by this function.  This function should use its own wxDb instance
	//	*       to avoid undesired unbinding of columns.
	//	*/
	//{
	//	exASSERT(fileName.length());

	//	RETCODE   retcode;
	//	SQLLEN    cb;
	//	wchar_t    tblName[DB_MAX_TABLE_NAME_LEN+1];
	//	std::wstring  tblNameSave;
	//	wchar_t    colName[DB_MAX_COLUMN_NAME_LEN+1];
	//	SWORD     sqlDataType;
	//	wchar_t    typeName[30+1];
	//	SDWORD    precision, length;

	//	FILE *fp = _wfopen(fileName.c_str(), L"wt");
	//	if (fp == NULL)
	//		return false;

	//	SQLFreeStmt(m_hstmt, SQL_CLOSE);

	//	std::wstring UserID = ConvertUserIDImpl(userID);

	//	if (!UserID.empty() &&
	//		GetDbms() != dbmsMY_SQL &&
	//		GetDbms() != dbmsACCESS &&
	//		GetDbms() != dbmsFIREBIRD &&
	//		GetDbms() != dbmsINTERBASE &&
	//		GetDbms() != dbmsMS_SQL_SERVER)
	//	{
	//		retcode = SQLColumns(m_hstmt,
	//			NULL, 0,                                // All qualifiers
	//			(SQLTCHAR *) UserID.c_str(), SQL_NTS,      // User specified
	//			NULL, 0,                                // All tables
	//			NULL, 0);                               // All columns
	//	}
	//	else
	//	{
	//		retcode = SQLColumns(m_hstmt,
	//			NULL, 0,    // All qualifiers
	//			NULL, 0,    // User specified
	//			NULL, 0,    // All tables
	//			NULL, 0);   // All columns
	//	}
	//	if (retcode != SQL_SUCCESS)
	//	{
	//		DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
	//		fclose(fp);
	//		return false;
	//	}

	//	std::wstring outStr;
	//	tblNameSave.empty();
	//	int cnt = 0;

	//	while (true)
	//	{
	//		retcode = SQLFetch(m_hstmt);
	//		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	//			break;

	//		GetData(3,SQL_C_WXCHAR,  (UCHAR *) tblName,     DB_MAX_TABLE_NAME_LEN+1, &cb);
	//		GetData(4,SQL_C_WXCHAR,  (UCHAR *) colName,     DB_MAX_COLUMN_NAME_LEN+1,&cb);
	//		GetData(5,SQL_C_SSHORT,  (UCHAR *)&sqlDataType, 0,                       &cb);
	//		GetData(6,SQL_C_WXCHAR,  (UCHAR *) typeName,    sizeof(typeName),        &cb);
	//		GetData(7,SQL_C_SLONG,   (UCHAR *)&precision,   0,                       &cb);
	//		GetData(8,SQL_C_SLONG,   (UCHAR *)&length,      0,                       &cb);

	//		if (wcscmp(tblName, tblNameSave.c_str()))
	//		{
	//			if (cnt)
	//				fputws(L"\n", fp);
	//			fputws(L"================================ ", fp);
	//			fputws(L"================================ ", fp);
	//			fputws(L"===================== ", fp);
	//			fputws(L"========= ", fp);
	//			fputws(L"=========\n", fp);
	//			outStr = (boost::wformat(L"%-32s %-32s %-21s %9s %9s\n") % L"TABLE NAME" % L"COLUMN NAME" % L"DATA TYPE" % L"PRECISION" % L"LENGTH").str();
	//			fputws(outStr.c_str(), fp);
	//			fputws(L"================================ ", fp);
	//			fputws(L"================================ ", fp);
	//			fputws(L"===================== ", fp);
	//			fputws(L"========= ", fp);
	//			fputws(L"=========\n", fp);
	//			tblNameSave = tblName;
	//		}

	//		outStr = (boost::wformat(L"%-32s %-32s (%04d)%-15s %9ld %9ld\n") % tblName % colName % sqlDataType % typeName % precision % length).str();
	//		if (fputws(outStr.c_str(), fp) == EOF)
	//		{
	//			SQLFreeStmt(m_hstmt, SQL_CLOSE);
	//			fclose(fp);
	//			return false;
	//		}
	//		cnt++;
	//	}

	//	if (retcode != SQL_NO_DATA_FOUND)
	//		DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);

	//	SQLFreeStmt(m_hstmt, SQL_CLOSE);

	//	fclose(fp);
	//	return(retcode == SQL_NO_DATA_FOUND);

	//}  // wxDb::Catalog()



	//const std::wstring Database::SQLTableName(const wchar_t *tableName)
	//{
	//	std::wstring TableName;

	//	if (GetDbms() == dbmsACCESS)
	//		TableName = L"\"";
	//	TableName += tableName;
	//	if (GetDbms() == dbmsACCESS)
	//		TableName += L"\"";

	//	return TableName;
	//}  // wxDb::SQLTableName()


	//const std::wstring Database::SQLColumnName(const wchar_t *colName)
	//{
	//	std::wstring ColName;

	//	if (GetDbms() == dbmsACCESS)
	//		ColName = L"\"";
	//	ColName += colName;
	//	if (GetDbms() == dbmsACCESS)
	//		ColName += L"\"";

	//	return ColName;
	//}  // wxDb::SQLColumnName()


	///********** wxDb::SetSqlLogging() **********/
	//bool Database::SetSqlLogging(wxDbSqlLogState state, const std::wstring &filename, bool append)
	//{
	//	exASSERT(state == sqlLogON  || state == sqlLogOFF);
	//	exASSERT(state == sqlLogOFF || filename.length());

	//	if (state == sqlLogON)
	//	{
	//		if (m_fpSqlLog == 0)
	//		{
	//			m_fpSqlLog = _wfopen(filename.c_str(), (append ? L"at" : L"wt"));
	//			if (m_fpSqlLog == NULL)
	//				return false;
	//		}
	//	}
	//	else  // sqlLogOFF
	//	{
	//		if (m_fpSqlLog)
	//		{
	//			if (fclose(m_fpSqlLog))
	//				return false;
	//			m_fpSqlLog = 0;
	//		}
	//	}

	//	m_sqlLogState = state;
	//	return true;

	//}  // wxDb::SetSqlLogging()


	///********** wxDb::WriteSqlLog() **********/
	//bool Database::WriteSqlLog(const std::wstring &logMsg)
	//{
	//	exASSERT(logMsg.length());

	//	if (m_fpSqlLog == 0 || m_sqlLogState == sqlLogOFF)
	//		return false;

	//	if (fputws(L"\n", m_fpSqlLog) == EOF)
	//		return false;
	//	if (fputws(logMsg.c_str(), m_fpSqlLog) == EOF)
	//		return false;
	//	if (fputws(L"\n", m_fpSqlLog) == EOF)
	//		return false;



	//	return true;

	//}  // wxDb::WriteSqlLog()


	//std::vector<std::wstring> Database::GetErrorList() const
	//{
	//	std::vector<std::wstring> list;

	//	for (int i = 0; i < DB_MAX_ERROR_HISTORY; i++)
	//	{
	//		if (errorList[i])
	//		{
	//			list.push_back(std::wstring(errorList[i]));
	//		}
	//	}
	//	return list;
	//}

	//bool Database::ModifyColumn(const std::wstring &tableName, const std::wstring &columnName,
	//	int dataType, ULONG columnLength,
	//	const std::wstring &optionalParam)
	//{
	//	exASSERT(tableName.length());
	//	exASSERT(columnName.length());
	//	exASSERT((dataType == DB_DATA_TYPE_VARCHAR && columnLength > 0) ||
	//		dataType != DB_DATA_TYPE_VARCHAR);

	//	// Must specify a columnLength if modifying a VARCHAR type column
	//	if (dataType == DB_DATA_TYPE_VARCHAR && !columnLength)
	//		return false;

	//	std::wstring dataTypeName;
	//	std::wstring sqlStmt;
	//	std::wstring alterSlashModify;

	//	switch(dataType)
	//	{
	//	case DB_DATA_TYPE_VARCHAR :
	//		dataTypeName = m_typeInfVarchar.TypeName;
	//		break;
	//	case DB_DATA_TYPE_INTEGER :
	//		dataTypeName = m_typeInfInteger.TypeName;
	//		break;
	//	case DB_DATA_TYPE_FLOAT :
	//		dataTypeName = m_typeInfFloat.TypeName;
	//		break;
	//	case DB_DATA_TYPE_DATE :
	//		dataTypeName = m_typeInfDate.TypeName;
	//		break;
	//	case DB_DATA_TYPE_BLOB :
	//		dataTypeName = m_typeInfBlob.TypeName;
	//		break;
	//	default:
	//		return false;
	//	}

	//	// Set the modify or alter syntax depending on the type of database connected to
	//	switch (GetDbms())
	//	{
	//	case dbmsORACLE :
	//		alterSlashModify = L"MODIFY";
	//		break;
	//	case dbmsMS_SQL_SERVER :
	//		alterSlashModify = L"ALTER COLUMN";
	//		break;
	//	case dbmsUNIDENTIFIED :
	//		return false;
	//	case dbmsSYBASE_ASA :
	//	case dbmsSYBASE_ASE :
	//	case dbmsMY_SQL :
	//	case dbmsPOSTGRES :
	//	case dbmsACCESS :
	//	case dbmsDBASE :
	//	case dbmsXBASE_SEQUITER :
	//	default :
	//		alterSlashModify = L"MODIFY";
	//		break;
	//	}

	//	// create the SQL statement
	//	if ( GetDbms() == dbmsMY_SQL )
	//	{
	//		sqlStmt = (boost::wformat(L"ALTER TABLE %s %s %s %s") % tableName % alterSlashModify % columnName % dataTypeName).str();
	//	}
	//	else
	//	{
	//		sqlStmt = (boost::wformat(L"ALTER TABLE \"%s\" \"%s\" \"%s\" %s") % tableName % alterSlashModify % columnName % dataTypeName).str();
	//	}

	//	// For varchars only, append the size of the column
	//	if (dataType == DB_DATA_TYPE_VARCHAR &&
	//		(GetDbms() != dbmsMY_SQL || dataTypeName != L"text"))
	//	{
	//		std::wstring s;
	//		s = (boost::wformat(L"(%lu)") % columnLength).str();
	//		sqlStmt += s;
	//	}

	//	// for passing things like "NOT NULL"
	//	if (optionalParam.length())
	//	{
	//		sqlStmt += L" ";
	//		sqlStmt += optionalParam;
	//	}

	//	return ExecSql(sqlStmt);

	//} // wxDb::ModifyColumn()

	///********** wxDb::EscapeSqlChars() **********/
	//std::wstring Database::EscapeSqlChars(const std::wstring& valueOrig)
	//{
	//	std::wstring value(valueOrig);
	//	switch (GetDbms())
	//	{
	//	case dbmsACCESS:
	//		// Access doesn't seem to care about backslashes, so only escape single quotes.
	//		boost::algorithm::replace_all(value, L"'", L"''");
	//		break;

	//	default:
	//		// All the others are supposed to be the same for now, add special
	//		// handling for them if necessary
	//		boost::algorithm::replace_all(value, L"\\", L"\\\\");
	//		boost::algorithm::replace_all(value, L"'", L"\\'");
	//		break;
	//	}

	//	return value;
	//} // wxDb::EscapeSqlChars()


	///********** wxDbLogExtendedErrorMsg() **********/
	//// DEBUG ONLY function
	//const wchar_t EXODBCAPI *wxDbLogExtendedErrorMsg(const wchar_t *userText,
	//	Database *pDb,
	//	const wchar_t *ErrFile,
	//	int ErrLine)
	//{
	//	static std::wstring msg;
	//	msg = userText;

	//	std::wstring tStr;

	//	if (ErrFile || ErrLine)
	//	{
	//		msg += L"File: ";
	//		msg += ErrFile;
	//		msg += L"   Line: ";
	//		tStr = (boost::wformat(L"%d") % ErrLine).str();
	//		msg += tStr.c_str();
	//		msg += L"\n";
	//	}

	//	msg.append (L"\nODBC errors:\n");
	//	msg += L"\n";

	//	// Display errors for this connection
	//	int i;
	//	for (i = 0; i < DB_MAX_ERROR_HISTORY; i++)
	//	{
	//		if (pDb->errorList[i])
	//		{
	//			msg.append(pDb->errorList[i]);
	//			if (wcscmp(pDb->errorList[i], emptyString) != 0)
	//				msg.append(L"\n");
	//			// Clear the errmsg buffer so the next error will not
	//			// end up showing the previous error that have occurred
	//			wcscpy(pDb->errorList[i], emptyString);
	//		}
	//	}
	//	msg += L"\n";

	//	BOOST_LOG_TRIVIAL(debug) << msg;

	//	return msg.c_str();
	//}  // wxDbLogExtendedErrorMsg()


	///********** wxDbSqlLog() **********/
	//bool wxDbSqlLog(wxDbSqlLogState state, const wchar_t *filename)
	//{
	//	bool append = false;
	//	SDbList *pList;

	//	for (pList = PtrBegDbList; pList; pList = pList->PtrNext)
	//	{
	//		if (!pList->PtrDb->SetSqlLogging(state,filename,append))
	//			return false;
	//		append = true;
	//	}

	//	SQLLOGstate = state;
	//	SQLLOGfn = filename;

	//	return true;

	//}  // wxDbSqlLog()


///********** PRIVATE! wxDb::ConvertUserIDImpl PRIVATE! **********/
////
//// NOTE: Return value from this function MUST be copied
////       immediately, as the value is not good after
////       this function has left scope.
////
//std::wstring Database::ConvertUserIDImpl(const wchar_t* userID)
//{
//	std::wstring UserID;

//	if (userID)
//	{
//		if (!wcslen(userID))
//			UserID = m_uid;
//		else
//			UserID = userID;
//	}
//	else
//		UserID.empty();

//	// dBase does not use user names, and some drivers fail if you try to pass one
//	if ( GetDbms() == dbmsDBASE
//		|| GetDbms() == dbmsXBASE_SEQUITER )
//		UserID.empty();

//	// Some databases require user names to be specified in uppercase,
//	// so force the name to uppercase
//	if ((GetDbms() == dbmsORACLE) ||
//		(GetDbms() == dbmsMAXDB))
//		boost::algorithm::to_upper(UserID);

//	return UserID;
//}  // wxDb::ConvertUserIDImpl()


//	bool Database::DetermineDataTypes(bool failOnDataTypeUnsupported)
//	{
//		size_t iIndex;
//
//		// These are the possible SQL types we check for use against the datasource we are connected
//		// to for the purpose of determining which data type to use for the basic character strings
//		// column types
//		//
//		// NOTE: The first type in this enumeration that is determined to be supported by the
//		//       datasource/driver is the one that will be used.
//		SWORD PossibleSqlCharTypes[] = {
//#if defined(SQL_WVARCHAR)
//			SQL_WVARCHAR,
//#endif
//			SQL_VARCHAR,
//#if defined(SQL_WVARCHAR)
//			SQL_WCHAR,
//#endif
//			SQL_CHAR
//		};
//
//		// These are the possible SQL types we check for use against the datasource we are connected
//		// to for the purpose of determining which data type to use for the basic non-floating point
//		// column types
//		//
//		// NOTE: The first type in this enumeration that is determined to be supported by the
//		//       datasource/driver is the one that will be used.
//		SWORD PossibleSqlIntegerTypes[] = {
//			SQL_INTEGER
//		};
//
//		// These are the possible SQL types we check for use against the datasource we are connected
//		// to for the purpose of determining which data type to use for the basic floating point number
//		// column types
//		//
//		// NOTE: The first type in this enumeration that is determined to be supported by the
//		//       datasource/driver is the one that will be used.
//		SWORD PossibleSqlFloatTypes[] = {
//			SQL_DOUBLE,
//			SQL_REAL,
//			SQL_FLOAT,
//			SQL_DECIMAL,
//			SQL_NUMERIC
//		};
//
//		// These are the possible SQL types we check for use agains the datasource we are connected
//		// to for the purpose of determining which data type to use for the date/time column types
//		//
//		// NOTE: The first type in this enumeration that is determined to be supported by the
//		//       datasource/driver is the one that will be used.
//		SWORD PossibleSqlDateTypes[] = {
//			SQL_TIMESTAMP,
//			SQL_DATE,
//#ifdef SQL_DATETIME
//			SQL_DATETIME
//#endif
//		};
//
//		// These are the possible SQL types we check for use agains the datasource we are connected
//		// to for the purpose of determining which data type to use for the BLOB column types.
//		//
//		// NOTE: The first type in this enumeration that is determined to be supported by the
//		//       datasource/driver is the one that will be used.
//		SWORD PossibleSqlBlobTypes[] = {
//			SQL_LONGVARBINARY,
//			SQL_VARBINARY
//		};
//
//		// These are the possible SQL types we check for use agains the datasource we are connected
//		// to for the purpose of determining which data type to use for the MEMO column types
//		// (a type which allow to store large strings; like VARCHAR just with a bigger precision)
//		//
//		// NOTE: The first type in this enumeration that is determined to be supported by the
//		//       datasource/driver is the one that will be used.
//		SWORD PossibleSqlMemoTypes[] = {
//			SQL_LONGVARCHAR,
//		};
//
//
//		// Query the data source regarding data type information
//
//		//
//		// The way it was determined which SQL data types to use was by calling SQLGetInfo
//		// for all of the possible SQL data types to see which ones were supported.  If
//		// a type is not supported, the SQLFetch() that's called from GetDataTypeInfoImpl()
//		// fails with SQL_NO_DATA_FOUND.  This is ugly because I'm sure the three SQL data
//		// types I've selected below will not always be what we want.  These are just
//		// what happened to work against an Oracle 7/Intersolv combination.  The following is
//		// a complete list of the results I got back against the Oracle 7 database:
//		//
//		// SQL_BIGINT             SQL_NO_DATA_FOUND
//		// SQL_BINARY             SQL_NO_DATA_FOUND
//		// SQL_BIT                SQL_NO_DATA_FOUND
//		// SQL_CHAR               type name = 'CHAR', Precision = 255
//		// SQL_DATE               SQL_NO_DATA_FOUND
//		// SQL_DECIMAL            type name = 'NUMBER', Precision = 38
//		// SQL_DOUBLE             type name = 'NUMBER', Precision = 15
//		// SQL_FLOAT              SQL_NO_DATA_FOUND
//		// SQL_INTEGER            SQL_NO_DATA_FOUND
//		// SQL_LONGVARBINARY      type name = 'LONG RAW', Precision = 2 billion
//		// SQL_LONGVARCHAR        type name = 'LONG', Precision = 2 billion
//		// SQL_NUMERIC            SQL_NO_DATA_FOUND
//		// SQL_REAL               SQL_NO_DATA_FOUND
//		// SQL_SMALLINT           SQL_NO_DATA_FOUND
//		// SQL_TIME               SQL_NO_DATA_FOUND
//		// SQL_TIMESTAMP          type name = 'DATE', Precision = 19
//		// SQL_VARBINARY          type name = 'RAW', Precision = 255
//		// SQL_VARCHAR            type name = 'VARCHAR2', Precision = 2000
//		// =====================================================================
//		// Results from a Microsoft Access 7.0 db, using a driver from Microsoft
//		//
//		// SQL_VARCHAR            type name = 'TEXT', Precision = 255
//		// SQL_TIMESTAMP          type name = 'DATETIME'
//		// SQL_DECIMAL            SQL_NO_DATA_FOUND
//		// SQL_NUMERIC            type name = 'CURRENCY', Precision = 19
//		// SQL_FLOAT              SQL_NO_DATA_FOUND
//		// SQL_REAL               type name = 'SINGLE', Precision = 7
//		// SQL_DOUBLE             type name = 'DOUBLE', Precision = 15
//		// SQL_INTEGER            type name = 'LONG', Precision = 10
//
//		// --------------- Varchar - (Variable length character string) ---------------
//		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlCharTypes) &&
//			!GetDataTypeInfoImpl(PossibleSqlCharTypes[iIndex], m_typeInfVarchar); ++iIndex)
//		{}
//
//		if (iIndex < EXSIZEOF(PossibleSqlCharTypes))
//			m_typeInfVarchar.FsqlType = PossibleSqlCharTypes[iIndex];
//		else if (failOnDataTypeUnsupported)
//			return false;
//
//		// --------------- Float ---------------
//		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlFloatTypes) &&
//			!GetDataTypeInfoImpl(PossibleSqlFloatTypes[iIndex], m_typeInfFloat); ++iIndex)
//		{}
//
//		if (iIndex < EXSIZEOF(PossibleSqlFloatTypes))
//			m_typeInfFloat.FsqlType = PossibleSqlFloatTypes[iIndex];
//		else if (failOnDataTypeUnsupported)
//			return false;
//
//		// --------------- Integer -------------
//		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlIntegerTypes) &&
//			!GetDataTypeInfoImpl(PossibleSqlIntegerTypes[iIndex], m_typeInfInteger); ++iIndex)
//		{}
//
//		if (iIndex < EXSIZEOF(PossibleSqlIntegerTypes))
//			m_typeInfInteger.FsqlType = PossibleSqlIntegerTypes[iIndex];
//		else if (failOnDataTypeUnsupported)
//		{
//			// If no non-floating point data types are supported, we'll
//			// use the type assigned for floats to store integers as well
//			if (!GetDataTypeInfoImpl(m_typeInfFloat.FsqlType, m_typeInfInteger))
//			{
//				if (failOnDataTypeUnsupported)
//					return false;
//			}
//			else
//				m_typeInfInteger.FsqlType = m_typeInfFloat.FsqlType;
//		}
//
//		// --------------- Date/Time ---------------
//		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlDateTypes) &&
//			!GetDataTypeInfoImpl(PossibleSqlDateTypes[iIndex], m_typeInfDate); ++iIndex)
//		{}
//
//		if (iIndex < EXSIZEOF(PossibleSqlDateTypes))
//			m_typeInfDate.FsqlType = PossibleSqlDateTypes[iIndex];
//		else if (failOnDataTypeUnsupported)
//			return false;
//
//		// --------------- BLOB ---------------
//		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlBlobTypes) &&
//			!GetDataTypeInfoImpl(PossibleSqlBlobTypes[iIndex], m_typeInfBlob); ++iIndex)
//		{}
//
//		if (iIndex < EXSIZEOF(PossibleSqlBlobTypes))
//			m_typeInfBlob.FsqlType = PossibleSqlBlobTypes[iIndex];
//		else if (failOnDataTypeUnsupported)
//			return false;
//
//		// --------------- MEMO ---------------
//		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlMemoTypes) &&
//			!GetDataTypeInfoImpl(PossibleSqlMemoTypes[iIndex], m_typeInfMemo); ++iIndex)
//		{}
//
//		if (iIndex < EXSIZEOF(PossibleSqlMemoTypes))
//			m_typeInfMemo.FsqlType = PossibleSqlMemoTypes[iIndex];
//		else if (failOnDataTypeUnsupported)
//			return false;
//
//		return true;
//	}  // wxDb::DetermineDataTypesImpl

//	/********** wxDb::DispAllErrors() **********/
//	bool Database::DispAllErrors(HENV aHenv, HDBC aHdbc, HSTMT aHstmt)
//		/*
//		* This function is called internally whenever an error condition prevents the user's
//		* request from being executed.  This function will query the datasource as to the
//		* actual error(s) that just occurred on the previous request of the datasource.
//		*
//		* The function will retrieve each error condition from the datasource and
//		* Printf the codes/text values into a string which it then logs via LogErrorImpl().
//		* If in DBDEBUG_CONSOLE mode, the constructed string will be displayed in the console
//		* window and program execution will be paused until the user presses a key.
//		*
//		* This function always returns false, so that functions which call this function
//		* can have a line like "return (DispAllErrors(henv, hdbc));" to indicate the failure
//		* of the user's request, so that the calling code can then process the error message log.
//		*/
//	{
//		SErrorInfoVector errs = GetAllErrors(aHenv, aHdbc, aHstmt);
//		for(size_t i = 0; i < errs.size(); i++)
//		{
//			LOG_ERROR((boost::wformat(L"Have ODBC Error #%d: %s") %i %errs[i]).str());
//		}
// 
////		std::wstring odbcErrMsg;
////
////		while (SQLError(aHenv, aHdbc, aHstmt, (SQLTCHAR FAR *) sqlState, &nativeError, (SQLTCHAR FAR *) errorMsg, SQL_MAX_MESSAGE_LENGTH - 1, &cbErrorMsg) == SQL_SUCCESS)
////		{
////			odbcErrMsg = (boost::wformat(L"SQL State = %s\nNative Error Code = %li\nError Message = %s\n") % sqlState % (long)nativeError % errorMsg).str();
////			LogErrorImpl(odbcErrMsg, sqlState);
////			if (!m_silent)
////			{
////#ifdef DBDEBUG_CONSOLE
////				// When run in console mode, use standard out to display errors.
////				std::wcout << odbcErrMsg.c_str() << std::endl;
////				std::wcout << L"Press any key to continue..." << std::endl;
////				getchar();
////#endif
////
////#ifdef EXODBCDEBUG
////				BOOST_LOG_TRIVIAL(debug) <<  L"ODBC DEBUG MESSAGE from DispAllErrors(): " << odbcErrMsg;
////#endif
////			}
////		}
//
//		return false;  // This function always returns false.
//
//	}



///**********wxDb::TranslateSqlState()  **********/
//int Database::TranslateSqlState(const std::wstring &SQLState)
//{
//	if (SQLState == L"01000")
//		return(DB_ERR_GENERAL_WARNING);
//	if (SQLState == L"01002")
//		return(DB_ERR_DISCONNECT_ERROR);
//	if (SQLState == L"01004")
//		return(DB_ERR_DATA_TRUNCATED);
//	if (SQLState == L"01006")
//		return(DB_ERR_PRIV_NOT_REVOKED);
//	if (SQLState == L"01S00")
//		return(DB_ERR_INVALID_CONN_STR_ATTR);
//	if (SQLState == L"01S01")
//		return(DB_ERR_ERROR_IN_ROW);
//	if (SQLState == L"01S02")
//		return(DB_ERR_OPTION_VALUE_CHANGED);
//	if (SQLState == L"01S03")
//		return(DB_ERR_NO_ROWS_UPD_OR_DEL);
//	if (SQLState == L"01S04")
//		return(DB_ERR_MULTI_ROWS_UPD_OR_DEL);
//	if (SQLState == L"07001")
//		return(DB_ERR_WRONG_NO_OF_PARAMS);
//	if (SQLState == L"07006")
//		return(DB_ERR_DATA_TYPE_ATTR_VIOL);
//	if (SQLState == L"08001")
//		return(DB_ERR_UNABLE_TO_CONNECT);
//	if (SQLState == L"08002")
//		return(DB_ERR_CONNECTION_IN_USE);
//	if (SQLState == L"08003")
//		return(DB_ERR_CONNECTION_NOT_OPEN);
//	if (SQLState == L"08004")
//		return(DB_ERR_REJECTED_CONNECTION);
//	if (SQLState == L"08007")
//		return(DB_ERR_CONN_FAIL_IN_TRANS);
//	if (SQLState == L"08S01")
//		return(DB_ERR_COMM_LINK_FAILURE);
//	if (SQLState == L"21S01")
//		return(DB_ERR_INSERT_VALUE_LIST_MISMATCH);
//	if (SQLState == L"21S02")
//		return(DB_ERR_DERIVED_TABLE_MISMATCH);
//	if (SQLState == L"22001")
//		return(DB_ERR_STRING_RIGHT_TRUNC);
//	if (SQLState == L"22003")
//		return(DB_ERR_NUMERIC_VALUE_OUT_OF_RNG);
//	if (SQLState == L"22005")
//		return(DB_ERR_ERROR_IN_ASSIGNMENT);
//	if (SQLState == L"22008")
//		return(DB_ERR_DATETIME_FLD_OVERFLOW);
//	if (SQLState == L"22012")
//		return(DB_ERR_DIVIDE_BY_ZERO);
//	if (SQLState == L"22026")
//		return(DB_ERR_STR_DATA_LENGTH_MISMATCH);
//	if (SQLState == L"23000")
//		return(DB_ERR_INTEGRITY_CONSTRAINT_VIOL);
//	if (SQLState == L"24000")
//		return(DB_ERR_INVALID_CURSOR_STATE);
//	if (SQLState == L"25000")
//		return(DB_ERR_INVALID_TRANS_STATE);
//	if (SQLState == L"28000")
//		return(DB_ERR_INVALID_AUTH_SPEC);
//	if (SQLState == L"34000")
//		return(DB_ERR_INVALID_CURSOR_NAME);
//	if (SQLState == L"37000")
//		return(DB_ERR_SYNTAX_ERROR_OR_ACCESS_VIOL);
//	if (SQLState == L"3C000")
//		return(DB_ERR_DUPLICATE_CURSOR_NAME);
//	if (SQLState == L"40001")
//		return(DB_ERR_SERIALIZATION_FAILURE);
//	if (SQLState == L"42000")
//		return(DB_ERR_SYNTAX_ERROR_OR_ACCESS_VIOL2);
//	if (SQLState == L"70100")
//		return(DB_ERR_OPERATION_ABORTED);
//	if (SQLState == L"IM001")
//		return(DB_ERR_UNSUPPORTED_FUNCTION);
//	if (SQLState == L"IM002")
//		return(DB_ERR_NO_DATA_SOURCE);
//	if (SQLState == L"IM003")
//		return(DB_ERR_DRIVER_LOAD_ERROR);
//	if (SQLState == L"IM004")
//		return(DB_ERR_SQLALLOCENV_FAILED);
//	if (SQLState == L"IM005")
//		return(DB_ERR_SQLALLOCCONNECT_FAILED);
//	if (SQLState == L"IM006")
//		return(DB_ERR_SQLSETCONNECTOPTION_FAILED);
//	if (SQLState == L"IM007")
//		return(DB_ERR_NO_DATA_SOURCE_DLG_PROHIB);
//	if (SQLState == L"IM008")
//		return(DB_ERR_DIALOG_FAILED);
//	if (SQLState == L"IM009")
//		return(DB_ERR_UNABLE_TO_LOAD_TRANSLATION_DLL);
//	if (SQLState == L"IM010")
//		return(DB_ERR_DATA_SOURCE_NAME_TOO_LONG);
//	if (SQLState == L"IM011")
//		return(DB_ERR_DRIVER_NAME_TOO_LONG);
//	if (SQLState == L"IM012")
//		return(DB_ERR_DRIVER_KEYWORD_SYNTAX_ERROR);
//	if (SQLState == L"IM013")
//		return(DB_ERR_TRACE_FILE_ERROR);
//	if (SQLState == L"S0001")
//		return(DB_ERR_TABLE_OR_VIEW_ALREADY_EXISTS);
//	if (SQLState == L"S0002")
//		return(DB_ERR_TABLE_NOT_FOUND);
//	if (SQLState == L"S0011")
//		return(DB_ERR_INDEX_ALREADY_EXISTS);
//	if (SQLState == L"S0012")
//		return(DB_ERR_INDEX_NOT_FOUND);
//	if (SQLState == L"S0021")
//		return(DB_ERR_COLUMN_ALREADY_EXISTS);
//	if (SQLState == L"S0022")
//		return(DB_ERR_COLUMN_NOT_FOUND);
//	if (SQLState == L"S0023")
//		return(DB_ERR_NO_DEFAULT_FOR_COLUMN);
//	if (SQLState == L"S1000")
//		return(DB_ERR_GENERAL_ERROR);
//	if (SQLState == L"S1001")
//		return(DB_ERR_MEMORY_ALLOCATION_FAILURE);
//	if (SQLState == L"S1002")
//		return(DB_ERR_INVALID_COLUMN_NUMBER);
//	if (SQLState == L"S1003")
//		return(DB_ERR_PROGRAM_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1004")
//		return(DB_ERR_SQL_DATA_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1008")
//		return(DB_ERR_OPERATION_CANCELLED);
//	if (SQLState == L"S1009")
//		return(DB_ERR_INVALID_ARGUMENT_VALUE);
//	if (SQLState == L"S1010")
//		return(DB_ERR_FUNCTION_SEQUENCE_ERROR);
//	if (SQLState == L"S1011")
//		return(DB_ERR_OPERATION_INVALID_AT_THIS_TIME);
//	if (SQLState == L"S1012")
//		return(DB_ERR_INVALID_TRANS_OPERATION_CODE);
//	if (SQLState == L"S1015")
//		return(DB_ERR_NO_CURSOR_NAME_AVAIL);
//	if (SQLState == L"S1090")
//		return(DB_ERR_INVALID_STR_OR_BUF_LEN);
//	if (SQLState == L"S1091")
//		return(DB_ERR_DESCRIPTOR_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1092")
//		return(DB_ERR_OPTION_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1093")
//		return(DB_ERR_INVALID_PARAM_NO);
//	if (SQLState == L"S1094")
//		return(DB_ERR_INVALID_SCALE_VALUE);
//	if (SQLState == L"S1095")
//		return(DB_ERR_FUNCTION_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1096")
//		return(DB_ERR_INF_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1097")
//		return(DB_ERR_COLUMN_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1098")
//		return(DB_ERR_SCOPE_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1099")
//		return(DB_ERR_NULLABLE_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1100")
//		return(DB_ERR_UNIQUENESS_OPTION_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1101")
//		return(DB_ERR_ACCURACY_OPTION_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1103")
//		return(DB_ERR_DIRECTION_OPTION_OUT_OF_RANGE);
//	if (SQLState == L"S1104")
//		return(DB_ERR_INVALID_PRECISION_VALUE);
//	if (SQLState == L"S1105")
//		return(DB_ERR_INVALID_PARAM_TYPE);
//	if (SQLState == L"S1106")
//		return(DB_ERR_FETCH_TYPE_OUT_OF_RANGE);
//	if (SQLState == L"S1107")
//		return(DB_ERR_ROW_VALUE_OUT_OF_RANGE);
//	if (SQLState == L"S1108")
//		return(DB_ERR_CONCURRENCY_OPTION_OUT_OF_RANGE);
//	if (SQLState == L"S1109")
//		return(DB_ERR_INVALID_CURSOR_POSITION);
//	if (SQLState == L"S1110")
//		return(DB_ERR_INVALID_DRIVER_COMPLETION);
//	if (SQLState == L"S1111")
//		return(DB_ERR_INVALID_BOOKMARK_VALUE);
//	if (SQLState == L"S1C00")
//		return(DB_ERR_DRIVER_NOT_CAPABLE);
//	if (SQLState == L"S1T00")
//		return(DB_ERR_TIMEOUT_EXPIRED);

//	// No match
//	return(0);

//}  // wxDb::TranslateSqlState()


//	/**********  wxDb::Grant() **********/
//	bool Database::Grant(int privileges, const std::wstring &tableName, const std::wstring &userList)
//	{
//		std::wstring sqlStmt;
//
//		// Build the grant statement
//		sqlStmt  = L"GRANT ";
//		if (privileges == DB_GRANT_ALL)
//			sqlStmt += L"ALL";
//		else
//		{
//			int c = 0;
//			if (privileges & DB_GRANT_SELECT)
//			{
//				sqlStmt += L"SELECT";
//				c++;
//			}
//			if (privileges & DB_GRANT_INSERT)
//			{
//				if (c++)
//					sqlStmt += L", ";
//				sqlStmt += L"INSERT";
//			}
//			if (privileges & DB_GRANT_UPDATE)
//			{
//				if (c++)
//					sqlStmt += L", ";
//				sqlStmt += L"UPDATE";
//			}
//			if (privileges & DB_GRANT_DELETE)
//			{
//				if (c++)
//					sqlStmt += L", ";
//				sqlStmt += L"DELETE";
//			}
//		}
//
//		sqlStmt += L" ON ";
//		sqlStmt += SQLTableName(tableName.c_str());
//		sqlStmt += L" TO ";
//		sqlStmt += userList;
//
//#ifdef DBDEBUG_CONSOLE
//		std::wcout << std::endl << sqlStmt.c_str() << std::endl;
//#endif
//
//		WriteSqlLog(sqlStmt);
//
//		return(ExecSql(sqlStmt));
//
//	}  // wxDb::Grant()


//	/********** wxDb::CreateView() **********/
//	bool Database::CreateView(const std::wstring &viewName, const std::wstring &colList,
//		const std::wstring &pSqlStmt, bool attemptDrop)
//	{
//		std::wstring sqlStmt;
//
//		// Drop the view first
//		if (attemptDrop && !DropView(viewName))
//			return false;
//
//		// Build the create view statement
//		sqlStmt  = L"CREATE VIEW ";
//		sqlStmt += viewName;
//
//		if (colList.length())
//		{
//			sqlStmt += L" (";
//			sqlStmt += colList;
//			sqlStmt += L")";
//		}
//
//		sqlStmt += L" AS ";
//		sqlStmt += pSqlStmt;
//
//		WriteSqlLog(sqlStmt);
//
//#ifdef DBDEBUG_CONSOLE
//		std::wcout << sqlStmt.c_str() << std::endl;
//#endif
//
//		return(ExecSql(sqlStmt));
//
//	}  // wxDb::CreateView()


//	/********** wxDb::DropView()  **********/
//	bool Database::DropView(const std::wstring &viewName)
//	{
//		/*
//		* NOTE: This function returns true if the View does not exist, but
//		*       only for identified databases.  Code will need to be added
//		*            below for any other databases when those databases are defined
//		*       to handle this situation consistently
//		*/
//		std::wstring sqlStmt;
//
//		sqlStmt = (boost::wformat(L"DROP VIEW %s") % viewName).str();
//
//		WriteSqlLog(sqlStmt);
//
//#ifdef DBDEBUG_CONSOLE
//		std::wcout << std::endl << sqlStmt.c_str() << std::endl;
//#endif
//
//		if (SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS) != SQL_SUCCESS)
//		{
//			// Check for "Base table not found" error and ignore
//			GetNextError(m_henv, m_hdbc, m_hstmt);
//			if (wcscmp(sqlState, L"S0002"))  // "Base table not found"
//			{
//				// Check for product specific error codes
//				if (!((GetDbms() == dbmsSYBASE_ASA    && !wcscmp(sqlState, L"42000"))))  // 5.x (and lower?)
//				{
//					DispNextError();
//					DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//					RollbackTrans();
//					return false;
//				}
//			}
//		}
//
//		// Commit the transaction
//		if (!CommitTrans())
//			return false;
//
//		return true;
//
//	}  // wxDb::DropView()


//	/********** wxDb::ExecSql() with column info **********/
//	bool Database::ExecSql(const std::wstring &pSqlStmt, ColumnInfo** columns, short& numcols)
//	{
//		//execute the statement first
//		if (!ExecSql(pSqlStmt))
//			return false;
//
//		SWORD noCols;
//		if (SQLNumResultCols(m_hstmt, &noCols) != SQL_SUCCESS)
//		{
//			DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//			return false;
//		}
//
//		if (noCols == 0)
//			return false;
//		else
//			numcols = noCols;
//
//		//  Get column information
//		short colNum;
//		wchar_t name[DB_MAX_COLUMN_NAME_LEN+1];
//		SWORD Sword;
//		SQLLEN Sqllen;
//		ColumnInfo* pColInf = new ColumnInfo[noCols];
//
//		// Fill in column information (name, datatype)
//		for (colNum = 0; colNum < noCols; colNum++)
//		{
//			if (SQLColAttributes(m_hstmt, (UWORD)(colNum+1), SQL_COLUMN_NAME,
//				name, sizeof(name),
//				&Sword, &Sqllen) != SQL_SUCCESS)
//			{
//				DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//				delete[] pColInf;
//				return false;
//			}
//
//			wcsncpy(pColInf[colNum].m_colName, name, DB_MAX_COLUMN_NAME_LEN);
//			pColInf[colNum].m_colName[DB_MAX_COLUMN_NAME_LEN] = 0;  // Prevent buffer overrun
//
//			if (SQLColAttributes(m_hstmt, (UWORD)(colNum+1), SQL_COLUMN_TYPE,
//				NULL, 0, &Sword, &Sqllen) != SQL_SUCCESS)
//			{
//				DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//				delete[] pColInf;
//				return false;
//			}
//
//			switch (Sqllen)
//			{
//#if defined(SQL_WCHAR)
//			case SQL_WCHAR:
//#endif
//#if defined(SQL_WVARCHAR)
//			case SQL_WVARCHAR:
//#endif
//			case SQL_VARCHAR:
//			case SQL_CHAR:
//				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_VARCHAR;
//				break;
//			case SQL_LONGVARCHAR:
//				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_MEMO;
//				break;
//			case SQL_TINYINT:
//			case SQL_SMALLINT:
//			case SQL_INTEGER:
//			case SQL_BIT:
//				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_INTEGER;
//				break;
//			case SQL_DOUBLE:
//			case SQL_DECIMAL:
//			case SQL_NUMERIC:
//			case SQL_FLOAT:
//			case SQL_REAL:
//				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_FLOAT;
//				break;
//			case SQL_DATE:
//			case SQL_TIMESTAMP:
//				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_DATE;
//				break;
//			case SQL_BINARY:
//				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_BLOB;
//				break;
//#ifdef EXODBCDEBUG
//			default:
//				std::wstring errMsg;
//				errMsg = (boost::wformat(L"SQL Data type %ld currently not supported by wxWidgets") % (long)Sqllen).str();
//				BOOST_LOG_TRIVIAL(debug) << L"ODBC DEBUG MESSAGE: " << errMsg;
//#endif
//			}
//		}
//
//		*columns = pColInf;
//		return true;
//	}  // wxDb::ExecSql()

///********** wxDb::GetNext()  **********/
//bool Database::GetNext()
//{
//	if (SQLFetch(m_hstmt) == SQL_SUCCESS)
//		return true;
//	else
//	{
//		DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//		return false;
//	}

//}  // wxDb::GetNext()



/********** wxDb::GetKeyFields() **********/
//int Database::GetKeyFields(const std::wstring &tableName, ColumnInfo* colInf, UWORD noCols)
//{
//	wchar_t       szPkTable[DB_MAX_TABLE_NAME_LEN+1];  /* Primary key table name */
//	wchar_t       szFkTable[DB_MAX_TABLE_NAME_LEN+1];  /* Foreign key table name */
//	SWORD        iKeySeq;
//	wchar_t       szPkCol[DB_MAX_COLUMN_NAME_LEN+1];   /* Primary key column     */
//	wchar_t       szFkCol[DB_MAX_COLUMN_NAME_LEN+1];   /* Foreign key column     */
//	SQLRETURN    retcode;
//	SQLLEN       cb;
//	SWORD        i;
//	std::wstring     tempStr;
//	/*
//	* -----------------------------------------------------------------------
//	* -- 19991224 : mj10777 : Create                                   ------
//	* --          : Three things are done and stored here :            ------
//	* --          : 1) which Column(s) is/are Primary Key(s)           ------
//	* --          : 2) which tables use this Key as a Foreign Key      ------
//	* --          : 3) which columns are Foreign Key and the name      ------
//	* --          :     of the Table where the Key is the Primary Key  -----
//	* --          : Called from GetColumns(const std::wstring &tableName,  ------
//	* --                           int *numCols,const wchar_t *userID ) ------
//	* -----------------------------------------------------------------------
//	*/

//	/*---------------------------------------------------------------------*/
//	/* Get the names of the columns in the primary key.                    */
//	/*---------------------------------------------------------------------*/
//	retcode = SQLPrimaryKeys(m_hstmt,
//		NULL, 0,                               /* Catalog name  */
//		NULL, 0,                               /* Schema name   */
//		(SQLTCHAR FAR *) tableName.c_str(), SQL_NTS); /* Table name    */

//	/*---------------------------------------------------------------------*/
//	/* Fetch and display the result set. This will be a list of the        */
//	/* columns in the primary key of the tableName table.                  */
//	/*---------------------------------------------------------------------*/
//	while ((retcode == SQL_SUCCESS) || (retcode == SQL_SUCCESS_WITH_INFO))
//	{
//		retcode = SQLFetch(m_hstmt);
//		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
//		{
//			GetData( 4, SQL_C_WXCHAR,  szPkCol,    DB_MAX_COLUMN_NAME_LEN+1, &cb);
//			GetData( 5, SQL_C_SSHORT, &iKeySeq,    0,                        &cb);
//			//-------
//			for (i=0;i<noCols;i++)                          // Find the Column name
//				if (!wcscmp(colInf[i].m_colName,szPkCol))   // We have found the Column
//					colInf[i].m_pkCol = iKeySeq;              // Which Primary Key is this (first, second usw.) ?
//		}  // if
//	}  // while
//	SQLFreeStmt(m_hstmt, SQL_CLOSE);  /* Close the cursor (the hstmt is still allocated).      */

//	/*---------------------------------------------------------------------*/
//	/* Get all the foreign keys that refer to tableName primary key.       */
//	/*---------------------------------------------------------------------*/
//	retcode = SQLForeignKeys(m_hstmt,
//		NULL, 0,                            /* Primary catalog */
//		NULL, 0,                            /* Primary schema  */
//		(SQLTCHAR FAR *)tableName.c_str(), SQL_NTS,/* Primary table   */
//		NULL, 0,                            /* Foreign catalog */
//		NULL, 0,                            /* Foreign schema  */
//		NULL, 0);                           /* Foreign table   */

//	/*---------------------------------------------------------------------*/
//	/* Fetch and display the result set. This will be all of the foreign   */
//	/* keys in other tables that refer to the tableName  primary key.      */
//	/*---------------------------------------------------------------------*/
//	tempStr.empty();
//	std::wstringstream tempStream;
//	szPkCol[0] = 0;
//	while ((retcode == SQL_SUCCESS) || (retcode == SQL_SUCCESS_WITH_INFO))
//	{
//		retcode = SQLFetch(m_hstmt);
//		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
//		{
//			GetData( 3, SQL_C_WXCHAR,  szPkTable,   DB_MAX_TABLE_NAME_LEN+1,   &cb);
//			GetData( 4, SQL_C_WXCHAR,  szPkCol,     DB_MAX_COLUMN_NAME_LEN+1,  &cb);
//			GetData( 5, SQL_C_SSHORT, &iKeySeq,     0,                         &cb);
//			GetData( 7, SQL_C_WXCHAR,  szFkTable,   DB_MAX_TABLE_NAME_LEN+1,   &cb);
//			GetData( 8, SQL_C_WXCHAR,  szFkCol,     DB_MAX_COLUMN_NAME_LEN+1,  &cb);
//			tempStream << L'[' << szFkTable << L']';  // [ ] in case there is a blank in the Table name
//			//            tempStr << _T('[') << szFkTable << _T(']');  // [ ] in case there is a blank in the Table name
//		}  // if
//	}  // while

//	tempStr = tempStream.str();
//	boost::trim_right(tempStr);     // Get rid of any unneeded blanks
//	if (!tempStr.empty())
//	{
//		for (i=0; i<noCols; i++)
//		{   // Find the Column name
//			if (!wcscmp(colInf[i].m_colName, szPkCol))           // We have found the Column, store the Information
//			{
//				wcsncpy(colInf[i].m_pkTableName, tempStr.c_str(), DB_MAX_TABLE_NAME_LEN);  // Name of the Tables where this Primary Key is used as a Foreign Key
//				colInf[i].m_pkTableName[DB_MAX_TABLE_NAME_LEN] = 0;  // Prevent buffer overrun
//			}
//		}
//	}  // if

//	SQLFreeStmt(m_hstmt, SQL_CLOSE);  /* Close the cursor (the hstmt is still allocated). */

//	/*---------------------------------------------------------------------*/
//	/* Get all the foreign keys in the tablename table.                    */
//	/*---------------------------------------------------------------------*/
//	retcode = SQLForeignKeys(m_hstmt,
//		NULL, 0,                             /* Primary catalog   */
//		NULL, 0,                             /* Primary schema    */
//		NULL, 0,                             /* Primary table     */
//		NULL, 0,                             /* Foreign catalog   */
//		NULL, 0,                             /* Foreign schema    */
//		(SQLTCHAR *)tableName.c_str(), SQL_NTS);/* Foreign table     */

//	/*---------------------------------------------------------------------*/
//	/*  Fetch and display the result set. This will be all of the          */
//	/*  primary keys in other tables that are referred to by foreign       */
//	/*  keys in the tableName table.                                       */
//	/*---------------------------------------------------------------------*/
//	while ((retcode == SQL_SUCCESS) || (retcode == SQL_SUCCESS_WITH_INFO))
//	{
//		retcode = SQLFetch(m_hstmt);
//		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
//		{
//			GetData( 3, SQL_C_WXCHAR,  szPkTable,   DB_MAX_TABLE_NAME_LEN+1,  &cb);
//			GetData( 5, SQL_C_SSHORT, &iKeySeq,     0,                        &cb);
//			GetData( 8, SQL_C_WXCHAR,  szFkCol,     DB_MAX_COLUMN_NAME_LEN+1, &cb);
//			//-------
//			for (i=0; i<noCols; i++)                            // Find the Column name
//			{
//				if (!wcscmp(colInf[i].m_colName,szFkCol))       // We have found the (Foreign Key) Column
//				{
//					colInf[i].m_fkCol = iKeySeq;                  // Which Foreign Key is this (first, second usw.) ?
//					wcsncpy(colInf[i].m_fkTableName, szFkTable, DB_MAX_TABLE_NAME_LEN);  // Name of the Table where this Foriegn is the Primary Key
//					colInf[i].m_fkTableName[DB_MAX_TABLE_NAME_LEN] = 0;  // Prevent buffer overrun
//				} // if
//			}  // for
//		}  // if
//	}  // while
//	SQLFreeStmt(m_hstmt, SQL_CLOSE);  /* Close the cursor (the hstmt is still allocated). */

//	return TRUE;

//}  // wxDb::GetKeyFields()


//#if OLD_GETCOLUMNS
//	/********** wxDb::GetColumns() **********/
//	ColumnInfo *Database::GetColumns(wchar_t *tableName[], const wchar_t *userID)
//		/*
//		*        1) The last array element of the tableName[] argument must be zero (null).
//		*            This is how the end of the array is detected.
//		*        2) This function returns an array of wxDbColInf structures.  If no columns
//		*            were found, or an error occurred, this pointer will be zero (null).  THE
//		*            CALLING FUNCTION IS RESPONSIBLE FOR DELETING THE MEMORY RETURNED WHEN IT
//		*            IS FINISHED WITH IT.  i.e.
//		*
//		*            wxDbColInf *colInf = pDb->GetColumns(tableList, userID);
//		*            if (colInf)
//		*            {
//		*                // Use the column inf
//		*                .......
//		*                // Destroy the memory
//		*                delete [] colInf;
//		*            }
//		*
//		* userID is evaluated in the following manner:
//		*        userID == NULL  ... UserID is ignored
//		*        userID == ""    ... UserID set equal to 'this->uid'
//		*        userID != ""    ... UserID set equal to 'userID'
//		*
//		* NOTE: ALL column bindings associated with this wxDb instance are unbound
//		*       by this function.  This function should use its own wxDb instance
//		*       to avoid undesired unbinding of columns.
//		*/
//	{
//		UWORD       noCols = 0;
//		UWORD       colNo  = 0;
//		ColumnInfo *colInf = 0;
//
//		RETCODE  retcode;
//		SQLLEN   cb;
//
//		std::wstring TableName;
//
//		std::wstring UserID = ConvertUserIDImpl(userID);
//
//		// Pass 1 - Determine how many columns there are.
//		// Pass 2 - Allocate the wxDbColInf array and fill in
//		//                the array with the column information.
//		int pass;
//		for (pass = 1; pass <= 2; pass++)
//		{
//			if (pass == 2)
//			{
//				if (noCols == 0)  // Probably a bogus table name(s)
//					break;
//				// Allocate n wxDbColInf objects to hold the column information
//				colInf = new ColumnInfo[noCols+1];
//				if (!colInf)
//					break;
//				// Mark the end of the array
//				wcscpy(colInf[noCols].m_tableName, emptyString);
//				wcscpy(colInf[noCols].m_colName, emptyString);
//				colInf[noCols].m_sqlDataType = 0;
//			}
//			// Loop through each table name
//			int tbl;
//			for (tbl = 0; tableName[tbl]; tbl++)
//			{
//				TableName = tableName[tbl];
//				// Oracle and Interbase table names are uppercase only, so force
//				// the name to uppercase just in case programmer forgot to do this
//				if ((GetDbms() == dbmsORACLE) ||
//					(GetDbms() == dbmsFIREBIRD) ||
//					(GetDbms() == dbmsINTERBASE))
//					boost::algorithm::to_upper(TableName);
//
//				SQLFreeStmt(m_hstmt, SQL_CLOSE);
//
//				// MySQL, SQLServer, and Access cannot accept a user name when looking up column names, so we
//				// use the call below that leaves out the user name
//				if (!UserID.empty() &&
//					GetDbms() != dbmsMY_SQL &&
//					GetDbms() != dbmsACCESS &&
//					GetDbms() != dbmsMS_SQL_SERVER)
//				{
//					retcode = SQLColumns(m_hstmt,
//						NULL, 0,                                // All qualifiers
//						(SQLTCHAR *) UserID.c_str(), SQL_NTS,      // Owner
//						(SQLTCHAR *) TableName.c_str(), SQL_NTS,
//						NULL, 0);                               // All columns
//				}
//				else
//				{
//					retcode = SQLColumns(m_hstmt,
//						NULL, 0,                                // All qualifiers
//						NULL, 0,                                // Owner
//						(SQLTCHAR *) TableName.c_str(), SQL_NTS,
//						NULL, 0);                               // All columns
//				}
//				if (retcode != SQL_SUCCESS)
//				{  // Error occurred, abort
//					DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//					if (colInf)
//						delete [] colInf;
//					SQLFreeStmt(m_hstmt, SQL_CLOSE);
//					return(0);
//				}
//
//				while ((retcode = SQLFetch(m_hstmt)) == SQL_SUCCESS)
//				{
//					if (pass == 1)  // First pass, just add up the number of columns
//						noCols++;
//					else  // Pass 2; Fill in the array of structures
//					{
//						if (colNo < noCols)  // Some extra error checking to prevent memory overwrites
//						{
//							// NOTE: Only the ODBC 1.x fields are retrieved
//							GetData( 1, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_catalog,      128+1,                    &cb);
//							GetData( 2, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_schema,       128+1,                    &cb);
//							GetData( 3, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_tableName,    DB_MAX_TABLE_NAME_LEN+1,  &cb);
//							GetData( 4, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_colName,      DB_MAX_COLUMN_NAME_LEN+1, &cb);
//							GetData( 5, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_sqlDataType,  0,                        &cb);
//							GetData( 6, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_typeName,     128+1,                    &cb);
//							GetData( 7, SQL_C_SLONG,  (UCHAR*) &colInf[colNo].m_columnLength, 0,                        &cb);
//							GetData( 8, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_bufferSize,   0,                        &cb);
//							GetData( 9, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_decimalDigits,0,                        &cb);
//							GetData(10, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_numPrecRadix, 0,                        &cb);
//							GetData(11, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_nullable,     0,                        &cb);
//							GetData(12, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_remarks,      254+1,                    &cb);
//
//							// Determine the wxDb data type that is used to represent the native data type of this data source
//							colInf[colNo].m_dbDataType = 0;
//							if (!_wcsicmp(m_typeInfVarchar.TypeName.c_str(), colInf[colNo].m_typeName))
//							{
//#ifdef _IODBC_
//								// IODBC does not return a correct columnLength, so we set
//								// columnLength = bufferSize if no column length was returned
//								// IODBC returns the columnLength in bufferSize. (bug)
//								if (colInf[colNo].m_columnLength < 1)
//								{
//									colInf[colNo].m_columnLength = colInf[colNo].m_bufferSize;
//								}
//#endif
//								colInf[colNo].m_dbDataType = DB_DATA_TYPE_VARCHAR;
//							}
//							else if (!_wcsicmp(m_typeInfInteger.TypeName.c_str(), colInf[colNo].m_typeName))
//								colInf[colNo].m_dbDataType = DB_DATA_TYPE_INTEGER;
//							else if (!_wcsicmp(m_typeInfFloat.TypeName.c_str(), colInf[colNo].m_typeName))
//								colInf[colNo].m_dbDataType = DB_DATA_TYPE_FLOAT;
//							else if (!_wcsicmp(m_typeInfDate.TypeName.c_str(), colInf[colNo].m_typeName))
//								colInf[colNo].m_dbDataType = DB_DATA_TYPE_DATE;
//							else if (!_wcsicmp(m_typeInfBlob.TypeName.c_str(), colInf[colNo].m_typeName))
//								colInf[colNo].m_dbDataType = DB_DATA_TYPE_BLOB;
//							colNo++;
//						}
//					}
//				}
//				if (retcode != SQL_NO_DATA_FOUND)
//				{  // Error occurred, abort
//					DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//					if (colInf)
//						delete [] colInf;
//					SQLFreeStmt(m_hstmt, SQL_CLOSE);
//					return(0);
//				}
//			}
//		}
//
//		SQLFreeStmt(m_hstmt, SQL_CLOSE);
//		return colInf;
//
//	}  // wxDb::GetColumns()
//
//
//	/********** wxDb::GetColumns() **********/
//
//	ColumnInfo *Database::GetColumns(const std::wstring &tableName, UWORD *numCols, const wchar_t *userID)
//		//
//		// Same as the above GetColumns() function except this one gets columns
//		// only for a single table, and if 'numCols' is not NULL, the number of
//		// columns stored in the returned wxDbColInf is set in '*numCols'
//		//
//		// userID is evaluated in the following manner:
//		//        userID == NULL  ... UserID is ignored
//		//        userID == ""    ... UserID set equal to 'this->uid'
//		//        userID != ""    ... UserID set equal to 'userID'
//		//
//		// NOTE: ALL column bindings associated with this wxDb instance are unbound
//		//       by this function.  This function should use its own wxDb instance
//		//       to avoid undesired unbinding of columns.
//
//	{
//		UWORD       noCols = 0;
//		UWORD       colNo  = 0;
//		ColumnInfo *colInf = 0;
//
//		RETCODE  retcode;
//		SQLLEN   cb;
//
//		std::wstring TableName;
//
//		std::wstring UserID = ConvertUserIDImpl(userID);
//
//		// Pass 1 - Determine how many columns there are.
//		// Pass 2 - Allocate the wxDbColInf array and fill in
//		//                the array with the column information.
//		int pass;
//		for (pass = 1; pass <= 2; pass++)
//		{
//			if (pass == 2)
//			{
//				if (noCols == 0)  // Probably a bogus table name(s)
//					break;
//				// Allocate n wxDbColInf objects to hold the column information
//				colInf = new ColumnInfo[noCols+1];
//				if (!colInf)
//					break;
//				// Mark the end of the array
//				wcscpy(colInf[noCols].m_tableName, emptyString);
//				wcscpy(colInf[noCols].m_colName, emptyString);
//				colInf[noCols].m_sqlDataType = 0;
//			}
//
//			TableName = tableName;
//			// Oracle and Interbase table names are uppercase only, so force
//			// the name to uppercase just in case programmer forgot to do this
//			if ((GetDbms() == dbmsORACLE) ||
//				(GetDbms() == dbmsFIREBIRD) ||
//				(GetDbms() == dbmsINTERBASE))
//				boost::algorithm::to_upper(TableName);
//
//			SQLFreeStmt(m_hstmt, SQL_CLOSE);
//
//			// MySQL, SQLServer, and Access cannot accept a user name when looking up column names, so we
//			// use the call below that leaves out the user name
//			if (!UserID.empty() &&
//				GetDbms() != dbmsMY_SQL &&
//				GetDbms() != dbmsACCESS &&
//				GetDbms() != dbmsMS_SQL_SERVER)
//			{
//				retcode = SQLColumns(m_hstmt,
//					NULL, 0,                                // All qualifiers
//					(SQLTCHAR *) UserID.c_str(), SQL_NTS,    // Owner
//					(SQLTCHAR *) TableName.c_str(), SQL_NTS,
//					NULL, 0);                               // All columns
//			}
//			else
//			{
//				retcode = SQLColumns(m_hstmt,
//					NULL, 0,                                 // All qualifiers
//					NULL, 0,                                 // Owner
//					(SQLTCHAR *) TableName.c_str(), SQL_NTS,
//					NULL, 0);                                // All columns
//			}
//			if (retcode != SQL_SUCCESS)
//			{  // Error occurred, abort
//				DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//				if (colInf)
//					delete [] colInf;
//				SQLFreeStmt(m_hstmt, SQL_CLOSE);
//				if (numCols)
//					*numCols = 0;
//				return(0);
//			}
//
//			while ((retcode = SQLFetch(m_hstmt)) == SQL_SUCCESS)
//			{
//				if (pass == 1)  // First pass, just add up the number of columns
//					noCols++;
//				else  // Pass 2; Fill in the array of structures
//				{
//					if (colNo < noCols)  // Some extra error checking to prevent memory overwrites
//					{
//						// NOTE: Only the ODBC 1.x fields are retrieved
//						GetData( 1, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_catalog,      128+1,                     &cb);
//						GetData( 2, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_schema,       128+1,                     &cb);
//						GetData( 3, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_tableName,    DB_MAX_TABLE_NAME_LEN+1,   &cb);
//						GetData( 4, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_colName,      DB_MAX_COLUMN_NAME_LEN+1,  &cb);
//						GetData( 5, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_sqlDataType,  0,                         &cb);
//						GetData( 6, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_typeName,     128+1,                     &cb);
//						GetData( 7, SQL_C_SLONG,  (UCHAR*) &colInf[colNo].m_columnLength, 0,                         &cb);
//						// BJO 991214 : SQL_C_SSHORT instead of SQL_C_SLONG, otherwise fails on Sparc (probably all 64 bit architectures)
//						GetData( 8, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_bufferSize,   0,                         &cb);
//						GetData( 9, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_decimalDigits,0,                         &cb);
//						GetData(10, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_numPrecRadix, 0,                         &cb);
//						GetData(11, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_nullable,     0,                         &cb);
//						GetData(12, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_remarks,      254+1,                     &cb);
//						// Start Values for Primary/Foriegn Key (=No)
//						colInf[colNo].m_pkCol = 0;           // Primary key column   0=No; 1= First Key, 2 = Second Key etc.
//						colInf[colNo].m_pkTableName[0] = 0;  // Tablenames where Primary Key is used as a Foreign Key
//						colInf[colNo].m_fkCol = 0;           // Foreign key column   0=No; 1= First Key, 2 = Second Key etc.
//						colInf[colNo].m_fkTableName[0] = 0;  // Foreign key table name
//
//						// BJO 20000428 : Virtuoso returns type names with upper cases!
//						if (GetDbms() == dbmsVIRTUOSO)
//						{
//							std::wstring s = colInf[colNo].m_typeName;
//							boost::algorithm::to_lower(s);
//							wcscmp(colInf[colNo].m_typeName, s.c_str());
//						}
//
//						// Determine the wxDb data type that is used to represent the native data type of this data source
//						colInf[colNo].m_dbDataType = 0;
//						if (!_wcsicmp(m_typeInfVarchar.TypeName.c_str(), colInf[colNo].m_typeName))
//						{
//#ifdef _IODBC_
//							// IODBC does not return a correct columnLength, so we set
//							// columnLength = bufferSize if no column length was returned
//							// IODBC returns the columnLength in bufferSize. (bug)
//							if (colInf[colNo].m_columnLength < 1)
//							{
//								colInf[colNo].m_columnLength = colInf[colNo].m_bufferSize;
//							}
//#endif
//
//							colInf[colNo].m_dbDataType = DB_DATA_TYPE_VARCHAR;
//						}
//						else if (!_wcsicmp(m_typeInfInteger.TypeName.c_str(), colInf[colNo].m_typeName))
//							colInf[colNo].m_dbDataType = DB_DATA_TYPE_INTEGER;
//						else if (!_wcsicmp(m_typeInfFloat.TypeName.c_str(), colInf[colNo].m_typeName))
//							colInf[colNo].m_dbDataType = DB_DATA_TYPE_FLOAT;
//						else if (!_wcsicmp(m_typeInfDate.TypeName.c_str(), colInf[colNo].m_typeName))
//							colInf[colNo].m_dbDataType = DB_DATA_TYPE_DATE;
//						else if (!_wcsicmp(m_typeInfBlob.TypeName.c_str(), colInf[colNo].m_typeName))
//							colInf[colNo].m_dbDataType = DB_DATA_TYPE_BLOB;
//
//						colNo++;
//					}
//				}
//			}
//			if (retcode != SQL_NO_DATA_FOUND)
//			{  // Error occurred, abort
//				DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//				if (colInf)
//					delete [] colInf;
//				SQLFreeStmt(m_hstmt, SQL_CLOSE);
//				if (numCols)
//					*numCols = 0;
//				return(0);
//			}
//		}
//
//		SQLFreeStmt(m_hstmt, SQL_CLOSE);
//
//		// Store Primary and Foriegn Keys
//		GetKeyFields(tableName,colInf,noCols);
//
//		if (numCols)
//			*numCols = noCols;
//		return colInf;
//
//	}  // wxDb::GetColumns()
//
//
//#else  // New GetColumns
//
//
//	/*
//	BJO 20000503
//	These are tentative new GetColumns members which should be more database
//	independent and which always returns the columns in the order they were
//	created.
//
//	- The first one (wxDbColInf *wxDb::GetColumns(wchar_t *tableName[], const
//	wchar_t* userID)) calls the second implementation for each separate table
//	before merging the results. This makes the code easier to maintain as
//	only one member (the second) makes the real work
//	- wxDbColInf *wxDb::GetColumns(const std::wstring &tableName, int *numCols, const
//	wchar_t *userID) is a little bit improved
//	- It doesn't anymore rely on the type-name to find out which database-type
//	each column has
//	- It ends by sorting the columns, so that they are returned in the same
//	order they were created
//	*/
//
//	typedef struct
//	{
//		UWORD noCols;
//		ColumnInfo *colInf;
//	} _TableColumns;
//
//
//	ColumnInfo *Database::GetColumns(wchar_t *tableName[], const wchar_t *userID)
//	{
//		int i, j;
//		// The last array element of the tableName[] argument must be zero (null).
//		// This is how the end of the array is detected.
//
//		UWORD noCols = 0;
//
//		// How many tables ?
//		int tbl;
//		for (tbl = 0 ; tableName[tbl]; tbl++);
//
//		// Create a table to maintain the columns for each separate table
//		_TableColumns *TableColumns = new _TableColumns[tbl];
//
//		// Fill the table
//		for (i = 0 ; i < tbl ; i++)
//
//		{
//			TableColumns[i].colInf = GetColumns(tableName[i], &TableColumns[i].noCols, userID);
//			if (TableColumns[i].colInf == NULL)
//				return NULL;
//			noCols += TableColumns[i].noCols;
//		}
//
//		// Now merge all the separate table infos
//		ColumnInfo *colInf = new ColumnInfo[noCols+1];
//
//		// Mark the end of the array
//		wcscpy(colInf[noCols].m_tableName, emptyString);
//		wcscpy(colInf[noCols].m_colName, emptyString);
//		colInf[noCols].m_sqlDataType = 0;
//
//		// Merge ...
//		int offset = 0;
//
//		for (i = 0 ; i < tbl ; i++)
//		{
//			for (j = 0 ; j < TableColumns[i].noCols ; j++)
//			{
//				colInf[offset++] = TableColumns[i].colInf[j];
//			}
//		}
//
//		delete [] TableColumns;
//
//		return colInf;
//	}  // wxDb::GetColumns()  -- NEW
//
//
//	ColumnInfo *Database::GetColumns(const std::wstring &tableName, int *numCols, const wchar_t *userID)
//		//
//		// Same as the above GetColumns() function except this one gets columns
//		// only for a single table, and if 'numCols' is not NULL, the number of
//		// columns stored in the returned wxDbColInf is set in '*numCols'
//		//
//		// userID is evaluated in the following manner:
//		//        userID == NULL  ... UserID is ignored
//		//        userID == ""    ... UserID set equal to 'this->uid'
//		//        userID != ""    ... UserID set equal to 'userID'
//		//
//		// NOTE: ALL column bindings associated with this wxDb instance are unbound
//		//       by this function.  This function should use its own wxDb instance
//		//       to avoid undesired unbinding of columns.
//	{
//		UWORD       noCols = 0;
//		UWORD       colNo  = 0;
//		ColumnInfo *colInf = 0;
//
//		RETCODE  retcode;
//		SDWORD   cb;
//
//		std::wstring TableName;
//
//		std::wstring UserID;
//		ConvertUserIDImpl(userID,UserID);
//
//		// Pass 1 - Determine how many columns there are.
//		// Pass 2 - Allocate the wxDbColInf array and fill in
//		//                the array with the column information.
//		int pass;
//		for (pass = 1; pass <= 2; pass++)
//		{
//			if (pass == 2)
//			{
//				if (noCols == 0)  // Probably a bogus table name(s)
//					break;
//				// Allocate n wxDbColInf objects to hold the column information
//				colInf = new ColumnInfo[noCols+1];
//				if (!colInf)
//					break;
//				// Mark the end of the array
//				wcscpy(colInf[noCols].m_tableName, emptyString);
//				wcscpy(colInf[noCols].m_colName, emptyString);
//				colInf[noCols].m_sqlDataType = 0;
//			}
//
//			TableName = tableName;
//			// Oracle and Interbase table names are uppercase only, so force
//			// the name to uppercase just in case programmer forgot to do this
//			if ((GetDbms() == dbmsORACLE) ||
//				(GetDbms() == dbmsFIREBIRD) ||
//				(GetDbms() == dbmsINTERBASE))
//				TableName = TableName.Upper();
//
//			SQLFreeStmt(m_hstmt, SQL_CLOSE);
//
//			// MySQL, SQLServer, and Access cannot accept a user name when looking up column names, so we
//			// use the call below that leaves out the user name
//			if (!UserID.empty() &&
//				GetDbms() != dbmsMY_SQL &&
//				GetDbms() != dbmsACCESS &&
//				GetDbms() != dbmsMS_SQL_SERVER)
//			{
//				retcode = SQLColumns(m_hstmt,
//					NULL, 0,                              // All qualifiers
//					(UCHAR *) UserID.c_str(), SQL_NTS,    // Owner
//					(UCHAR *) TableName.c_str(), SQL_NTS,
//					NULL, 0);                             // All columns
//			}
//			else
//			{
//				retcode = SQLColumns(m_hstmt,
//					NULL, 0,                              // All qualifiers
//					NULL, 0,                              // Owner
//					(UCHAR *) TableName.c_str(), SQL_NTS,
//					NULL, 0);                             // All columns
//			}
//			if (retcode != SQL_SUCCESS)
//			{  // Error occurred, abort
//				DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//				if (colInf)
//					delete [] colInf;
//				SQLFreeStmt(m_hstmt, SQL_CLOSE);
//				if (numCols)
//					*numCols = 0;
//				return(0);
//			}
//
//			while ((retcode = SQLFetch(m_hstmt)) == SQL_SUCCESS)
//			{
//				if (pass == 1)  // First pass, just add up the number of columns
//					noCols++;
//				else  // Pass 2; Fill in the array of structures
//				{
//					if (colNo < noCols)  // Some extra error checking to prevent memory overwrites
//					{
//						// NOTE: Only the ODBC 1.x fields are retrieved
//						GetData( 1, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_catalog,      128+1,                     &cb);
//						GetData( 2, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_schema,       128+1,                     &cb);
//						GetData( 3, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_tableName,    DB_MAX_TABLE_NAME_LEN+1,   &cb);
//						GetData( 4, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_colName,      DB_MAX_COLUMN_NAME_LEN+1,  &cb);
//						GetData( 5, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_sqlDataType,  0,                         &cb);
//						GetData( 6, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_typeName,     128+1,                     &cb);
//						GetData( 7, SQL_C_SLONG,  (UCHAR*) &colInf[colNo].m_columnLength, 0,                         &cb);
//						GetData( 8, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_bufferSize,   0,                         &cb);
//						GetData( 9, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_decimalDigits,0,                         &cb);
//						GetData(10, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_numPrecRadix, 0,                         &cb);
//						GetData(11, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_nullable,     0,                         &cb);
//						GetData(12, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_remarks,      254+1,                     &cb);
//						// Start Values for Primary/Foriegn Key (=No)
//						colInf[colNo].m_pkCol = 0;           // Primary key column   0=No; 1= First Key, 2 = Second Key etc.
//						colInf[colNo].m_pkTableName[0] = 0;  // Tablenames where Primary Key is used as a Foreign Key
//						colInf[colNo].m_fkCol = 0;           // Foreign key column   0=No; 1= First Key, 2 = Second Key etc.
//						colInf[colNo].m_fkTableName[0] = 0;  // Foreign key table name
//
//#ifdef _IODBC_
//						// IODBC does not return a correct columnLength, so we set
//						// columnLength = bufferSize if no column length was returned
//						// IODBC returns the columnLength in bufferSize. (bug)
//						if (colInf[colNo].m_columnLength < 1)
//						{
//							colInf[colNo].m_columnLength = colInf[colNo].m_bufferSize;
//						}
//#endif
//
//						// Determine the wxDb data type that is used to represent the native data type of this data source
//						colInf[colNo].m_dbDataType = 0;
//						// Get the intern datatype
//						switch (colInf[colNo].m_sqlDataType)
//						{
//						case SQL_WCHAR:
//						case SQL_WVARCHAR:
//						case SQL_VARCHAR:
//						case SQL_CHAR:
//							colInf[colNo].m_dbDataType = DB_DATA_TYPE_VARCHAR;
//							break;
//						case SQL_LONGVARCHAR:
//							colInf[colNo].m_dbDataType = DB_DATA_TYPE_MEMO;
//							break;
//						case SQL_TINYINT:
//						case SQL_SMALLINT:
//						case SQL_INTEGER:
//						case SQL_BIT:
//							colInf[colNo].m_dbDataType = DB_DATA_TYPE_INTEGER;
//							break;
//						case SQL_DOUBLE:
//						case SQL_DECIMAL:
//						case SQL_NUMERIC:
//						case SQL_FLOAT:
//						case SQL_REAL:
//							colInf[colNo].m_dbDataType = DB_DATA_TYPE_FLOAT;
//							break;
//						case SQL_DATE:
//						case SQL_TIMESTAMP:
//							colInf[colNo].m_dbDataType = DB_DATA_TYPE_DATE;
//							break;
//						case SQL_BINARY:
//							colInf[colNo].m_dbDataType = DB_DATA_TYPE_BLOB;
//							break;
//#ifdef EXODBCDEBUG
//						default:
//							std::wstring errMsg;
//							errMsg.Printf(L"SQL Data type %d currently not supported by wxWidgets", colInf[colNo].m_sqlDataType);
//							BOOST_LOG_TRIVIAL(debug) << L"ODBC DEBUG MESSAGE: " << errMsg;
//#endif
//						}
//						colNo++;
//					}
//				}
//			}
//			if (retcode != SQL_NO_DATA_FOUND)
//			{  // Error occurred, abort
//				DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//				if (colInf)
//					delete [] colInf;
//				SQLFreeStmt(m_hstmt, SQL_CLOSE);
//				if (numCols)
//					*numCols = 0;
//				return(0);
//			}
//		}
//
//		SQLFreeStmt(m_hstmt, SQL_CLOSE);
//
//		// Store Primary and Foreign Keys
//		GetKeyFields(tableName,colInf,noCols);
//
//		///////////////////////////////////////////////////////////////////////////
//		// Now sort the the columns in order to make them appear in the right order
//		///////////////////////////////////////////////////////////////////////////
//
//		// Build a generic SELECT statement which returns 0 rows
//		std::wstring Stmt;
//
//		Stmt.Printf(L"select * from \"%s\" where 0=1", tableName);
//
//		// Execute query
//		if (SQLExecDirect(m_hstmt, (UCHAR FAR *) Stmt.c_str(), SQL_NTS) != SQL_SUCCESS)
//		{
//			DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//			return NULL;
//		}
//
//		// Get the number of result columns
//		if (SQLNumResultCols (m_hstmt, &noCols) != SQL_SUCCESS)
//		{
//			DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//			return NULL;
//		}
//
//		if (noCols == 0) // Probably a bogus table name
//			return NULL;
//
//		//  Get the name
//		int i;
//		short colNum;
//		UCHAR name[100];
//		SWORD Sword;
//		SDWORD Sdword;
//		for (colNum = 0; colNum < noCols; colNum++)
//		{
//			if (SQLColAttributes(m_hstmt,colNum+1, SQL_COLUMN_NAME,
//				name, sizeof(name),
//				&Sword, &Sdword) != SQL_SUCCESS)
//			{
//				DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
//				return NULL;
//			}
//
//			std::wstring Name1 = name;
//			Name1 = Name1.Upper();
//
//			// Where is this name in the array ?
//			for (i = colNum ; i < noCols ; i++)
//			{
//				std::wstring Name2 =  colInf[i].m_colName;
//				Name2 = Name2.Upper();
//				if (Name2 == Name1)
//				{
//					if (colNum != i) // swap to sort
//					{
//						ColumnInfo tmpColInf = colInf[colNum];
//						colInf[colNum] =  colInf[i];
//						colInf[i] = tmpColInf;
//					}
//					break;
//				}
//			}
//		}
//		SQLFreeStmt(m_hstmt, SQL_CLOSE);
//
//		///////////////////////////////////////////////////////////////////////////
//		// End sorting
//		///////////////////////////////////////////////////////////////////////////
//
//		if (numCols)
//			*numCols = noCols;
//		return colInf;
//
//	}  // wxDb::GetColumns()
//
//
//#endif  // #else OLD_GETCOLUMNS

//#if 0
//	/********** wxDbCreateDataSource() **********/
//	int wxDbCreateDataSource(const std::wstring &driverName, const std::wstring &dsn, const std::wstring &description,
//		bool sysDSN, const std::wstring &defDir, wxWindow *parent)
//		/*
//		* !!!! ONLY FUNCTIONAL UNDER MSW with VC6 !!!!
//		* Very rudimentary creation of an ODBC data source.
//		*
//		* ODBC driver must be ODBC 3.0 compliant to use this function
//		*/
//	{
//		int result = FALSE;
//
//		//!!!! ONLY FUNCTIONAL UNDER MSW with VC6 !!!!
//#ifdef __VISUALC__
//		int       dsnLocation;
//		std::wstring  setupStr;
//
//		if (sysDSN)
//			dsnLocation = ODBC_ADD_SYS_DSN;
//		else
//			dsnLocation = ODBC_ADD_DSN;
//
//		// NOTE: The decimal 2 is an invalid character in all keyword pairs
//		// so that is why I used it, as std::wstring does not deal well with
//		// embedded nulls in strings
//		setupStr.Printf(L"DSN=%s%cDescription=%s%cDefaultDir=%s%c",dsn,2,description,2,defDir,2);
//
//		// Replace the separator from above with the '\0' separator needed
//		// by the SQLConfigDataSource() function
//		int k;
//		do
//		{
//			k = setupStr.Find((wchar_t)2,true);
//			if (k != wxNOT_FOUND)
//				setupStr[(UINT)k] = L'\0';
//		}
//		while (k != wxNOT_FOUND);
//
//		result = SQLConfigDataSource((HWND)parent->GetHWND(), dsnLocation,
//			driverName, setupStr.c_str());
//
//		if ((result != SQL_SUCCESS) && (result != SQL_SUCCESS_WITH_INFO))
//		{
//			// check for errors caused by ConfigDSN based functions
//			DWORD retcode = 0;
//			WORD cb;
//			wchar_t errMsg[SQL_MAX_MESSAGE_LENGTH];
//			errMsg[0] = L'\0';
//
//			// This function is only supported in ODBC drivers v3.0 compliant and above
//			SQLInstallerError(1,&retcode,errMsg,SQL_MAX_MESSAGE_LENGTH-1,&cb);
//			if (retcode)
//			{
//#ifdef DBDEBUG_CONSOLE
//				// When run in console mode, use standard out to display errors.
//				std::wcout << errMsg << std::endl;
//				std::wcout << L"Press any key to continue..." << std::endl;
//				getchar();
//#endif  // DBDEBUG_CONSOLE
//
//#ifdef EXODBCDEBUG
//				BOOST_LOG_TRIVIAL(debug) << L"ODBC DEBUG MESSAGE: " << errMsg;
//#endif  // EXODBCDEBUG
//			}
//		}
//		else
//			result = TRUE;
//#else
//		// Using iODBC/unixODBC or some other compiler which does not support the APIs
//		// necessary to use this function, so this function is not supported
//#ifdef EXODBCDEBUG
//		BOOST_LOG_TRIVIAL(debug) << L"ODBC DEBUG MESSAGE: " << L"wxDbCreateDataSource() not available except under VC++/MSW";
//#endif
//		result = FALSE;
//#endif  // __VISUALC__
//
//		return result;
//
//	}  // wxDbCreateDataSource()
//#endif
//
}