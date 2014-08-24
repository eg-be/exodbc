///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/db.cpp
// Purpose:     Implementation of the wxDb class.  The wxDb class represents a connection
//              to an ODBC data source.  The wxDb class allows operations on the data
//              source such as OpenImpling and closing the data source.
// Author:      Doug Card
// Modified by: George Tasker
//              Bart Jourquin
//              Mark Johnson, wxWindows@mj10777.de
// Mods:        Dec, 1998:
//                -Added support for SQL statement logging and database cataloging
// Mods:        April, 1999
//                -Added QUERY_ONLY mode support to reduce default number of cursors
//                -Added additional SQL logging code
//                -Added DEBUG-ONLY tracking of wxTable objects to detect orphaned DB connections
//                -Set ODBC option to only read committed writes to the DB so all
//                   databases operate the same in that respect
// Created:     9.96
// RCS-ID:      $Id: db.cpp 52489 2008-03-14 14:14:57Z JS $
// Copyright:   (c) 1996 Remstar International, Inc.
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

// Own header
#include "Database.h"

// Same component headers
#include "DbEnvironment.h"
#include "ColumnFormatter.h"
#include "Helpers.h"

// Other headers


namespace exodbc
{
	SDbList* PtrBegDbList = 0;

#ifdef EXODBCDEBUG
	//    #include "wx/thread.h"
	//    extern wxList TablesInUse;
	extern std::vector<STablesInUse*> TablesInUse;
#if wxUSE_THREADS
	extern wxCriticalSection csTablesInUse;
#endif // wxUSE_THREADS
#endif

	// SQL Log defaults to be used by GetDbConnection
	wxDbSqlLogState SQLLOGstate = sqlLogOFF;

	static std::wstring SQLLOGfn = SQL_LOG_FILENAME;

	// This type defines the return row-struct form
	// SQLTablePrivileges, and is used by wxDB::TablePrivileges.
	typedef struct
	{
		wchar_t        tableQual[128+1];
		wchar_t        tableOwner[128+1];
		wchar_t        tableName[128+1];
		wchar_t        grantor[128+1];
		wchar_t        grantee[128+1];
		wchar_t        privilege[128+1];
		wchar_t        grantable[3+1];
	} wxDbTablePrivilegeInfo;


	/********** wxDbColInf Constructor **********/
	ColumnInfo::ColumnInfo()
	{
		Initialize();
	}  // wxDbColInf::wxDbColInf()


	/********** wxDbColInf Destructor ********/
	ColumnInfo::~ColumnInfo()
	{
		if (m_pColFor)
			delete m_pColFor;
		m_pColFor = NULL;
	}  // wxDbColInf::~wxDbColInf()


	bool ColumnInfo::Initialize()
	{
		m_catalog[0]      = 0;
		m_schema[0]       = 0;
		m_tableName[0]    = 0;
		m_colName[0]      = 0;
		m_sqlDataType     = 0;
		m_typeName[0]     = 0;
		m_columnLength    = 0;
		m_bufferSize      = 0;
		m_decimalDigits   = 0;
		m_numPrecRadix    = 0;
		m_nullable        = 0;
		m_remarks[0]      = 0;
		m_dbDataType      = 0;
		m_pkCol           = 0;
		m_pkTableName[0]  = 0;
		m_fkCol           = 0;
		m_fkTableName[0]  = 0;
		m_pColFor         = NULL;

		return true;
	}  // wxDbColInf::Initialize()



	Database::Database(const DbEnvironment* const pEnv)
		: m_fwdOnlyCursors(true)
	{
		exASSERT(pEnv);

		Initialize();

		// Allocate the DBC-Handle
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DBC, pEnv->GetHenv(), &m_hdbc);
		if(ret != SQL_SUCCESS)
		{
			LOG_ERROR_ENV(pEnv->GetHenv(), ret, SQLAllocHandle);
		}
	}

	/********** wxDb Constructor **********/
	Database::Database(const HENV& aHenv, bool FwdOnlyCursors)
	{
		m_fwdOnlyCursors = FwdOnlyCursors;

		Initialize();

		// Allocate the DBC-Handle
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_DBC, aHenv, &m_hdbc);
		if(ret != SQL_SUCCESS)
		{
			LOG_ERROR_ENV(aHenv, ret, SQLAllocHandle);
		}
	}


	Database::~Database()
	{
		if (IsOpen())
		{
			Close();
		}

		// Free the connection-handle
		SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc);
		if(ret != SQL_SUCCESS)
		{
			// if SQL_ERROR is returned, the handle is still valid, error information can be fetched, use our standard logger
			if(ret == SQL_ERROR)
			{
				LOG_ERROR_DBC(m_hdbc, ret, SQLFreeHandle);
			}
			else
			{
				// We cannot get any error-info here
				LOG_ERROR_SQL_NO_SUCCESS(ret, SQLFreeHandle);
			}
		}
		if(ret == SQL_SUCCESS)
		{
			m_hdbc = SQL_NULL_HDBC;
		}
	}

	void Database::Initialize()
	{
		// Handles created by this db
		m_hdbc = SQL_NULL_HDBC;
		m_hstmt = SQL_NULL_HSTMT;

		//m_fpSqlLog      = 0;            // Sql Log file pointer
		//m_sqlLogState   = sqlLogOFF;    // By default, logging is turned off
		nTables       = 0;
		m_dbmsType      = dbmsUNIDENTIFIED;

		// Error reporting is turned OFF by default
		m_silent = true;

		// Mark database as not OpenImpl as of yet
		m_dbIsOpen = false;
		m_dbOpenedWithConnectionString = false;
	}


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
	//	if ( Dbms() == dbmsDBASE
	//		|| Dbms() == dbmsXBASE_SEQUITER )
	//		UserID.empty();

	//	// Some databases require user names to be specified in uppercase,
	//	// so force the name to uppercase
	//	if ((Dbms() == dbmsORACLE) ||
	//		(Dbms() == dbmsMAXDB))
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


	bool Database::OpenImpl(bool failOnDataTypeUnsupported)
	{
		exASSERT(m_hstmt == SQL_NULL_HSTMT);

		// Allocate a statement handle for the database connection
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);
		if(ret != SQL_SUCCESS)
		{
			// Note: SQLAllocHandle will set the output-handle to SQL_NULL_HDBC, SQL_NULL_HSTMT, or SQL_NULL_HENV in case of failure
			LOG_ERROR_DBC(m_hdbc, ret, SQLAllocHandle);
			return false;
		}

		// Set Connection Options
		if (!SetConnectionAttributes())
			return false;

		// Query the data source for info about itself
		if (!ReadDbInfo(m_dbInf))
			return false;

		// Query the datatypes
		if (!GetAllDataTypesInfo(m_datatypes))
			return false;

		// Completed Successfully
		return true;
	}

	bool Database::Open(const std::wstring& inConnectStr, bool failOnDataTypeUnsupported)
	{
		exASSERT(inConnectStr.length() > 0);
		return Open(inConnectStr, NULL, failOnDataTypeUnsupported);
	}

	bool Database::Open(const std::wstring& inConnectStr, SQLHWND parentWnd, bool failOnDataTypeUnsupported)
	{
		m_dsn        = emptyString;
		m_uid        = emptyString;
		m_authStr    = emptyString;

		SQLRETURN retcode;

		// TODO: See notes about forwardCursor in Open Method with dsn, user, pass 
		
		// Connect to the data source
		SQLWCHAR outConnectBuffer[SQL_MAX_CONNECTSTR_LEN + 1];  // MS recommends at least 1k buffer
		SQLSMALLINT outConnectBufferLen;

		m_inConnectionStr = inConnectStr;

		retcode = SQLDriverConnect(m_hdbc, parentWnd, 
			(SQLWCHAR*) m_inConnectionStr.c_str(),
			m_inConnectionStr.length(), 
			(SQLWCHAR*) outConnectBuffer,
			EXSIZEOF(outConnectBuffer), &outConnectBufferLen, SQL_DRIVER_COMPLETE );

		if (retcode != SQL_SUCCESS)
		{
			LOG_ERROR_DBC(m_hdbc, retcode, SQLDriverConnect);
			return false;
		}

		outConnectBuffer[outConnectBufferLen] = 0;
		m_outConnectionStr = outConnectBuffer;
		m_dbOpenedWithConnectionString = true;

		return OpenImpl(failOnDataTypeUnsupported);
	}


	bool Database::Open(const std::wstring& dsn, const std::wstring& uid, const std::wstring& authStr, bool failOnDataTypeUnsupported)
	{
		exASSERT(!dsn.empty());

		m_dsn        = dsn;
		m_uid        = uid;
		m_authStr    = authStr;

		// Not using a connection-string
		m_inConnectionStr = L"";
		m_outConnectionStr = L"";

		// TODO: Note about the forward-only cursors: This will be removed soon, see: 
		// http://msdn.microsoft.com/en-us/library/ms713605%28v=vs.85%29.aspx -> SQL_ATTR_ODBC_CURSORS
		// MS recommends to use the drivers lib (not modify here ? ). There is a statement-attribute to
		// set the cursor-scrolling: http://msdn.microsoft.com/en-us/library/ms710272%28v=vs.85%29.aspx
		//  -> http://msdn.microsoft.com/en-us/library/ms712631%28v=vs.85%29.aspx -> SQL_ATTR_CURSOR_SCROLLABLE

		// Connect to the data source
		SQLRETURN retcode = SQLConnect(m_hdbc, 
			(SQLWCHAR*) m_dsn.c_str(), SQL_NTS,
			(SQLWCHAR*) m_uid.c_str(), SQL_NTS,
			(SQLWCHAR*) m_authStr.c_str(), SQL_NTS);

		if (retcode != SQL_SUCCESS)
		{
			LOG_ERROR_DBC(m_hdbc, retcode, SQLConnect);
			return false;
		}

		// Mark database as Open
		m_dbIsOpen = true;

		return OpenImpl(failOnDataTypeUnsupported);

	}


	bool Database::Open(DbEnvironment *dbConnectInf, bool failOnDataTypeUnsupported)
	{
		exASSERT(dbConnectInf);

		// Use the connection string if one is present
		if (dbConnectInf->UseConnectionStr())
			return Open(dbConnectInf->GetConnectionStr(), failOnDataTypeUnsupported);
		else
			return Open(dbConnectInf->GetDsn(), dbConnectInf->GetUserID(), dbConnectInf->GetPassword(), failOnDataTypeUnsupported);
	}



	bool Database::SetConnectionAttributes()
	{
		exASSERT(m_hdbc != SQL_NULL_HDBC);
		bool ok = true;
		SQLRETURN ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, NULL);
		if(ret != SQL_SUCCESS)
		{
			LOG_ERROR_DBC_MSG(m_hdbc, ret, SQLSetConnectAttr, L"Cannot set ATTR_AUTOCOMMIT to AUTOCOMMIT_OFF");
			ok = false;
		}

		ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_TRACE, (SQLPOINTER) SQL_OPT_TRACE_OFF, NULL);
		if(ret != SQL_SUCCESS)
		{
			LOG_ERROR_DBC_MSG(m_hdbc, ret, SQLSetConnectAttr, L"Cannot set ATTR_TRACE to OPT_TRACE_OFF");
			ok = false;
		}

		// Note: This is unsupported SQL_ATTR_METADATA_ID by most drivers. It should default to OFF
		//ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_TRUE, NULL);
		//if(ret != SQL_SUCCESS)
		//{
		//	LOG_ERROR_DBC_MSG(m_hdbc, ret, SQLSetConnectAttr, L"Cannot set ATTR_METADATA_ID to SQL_FALSE");
		//	ok = false;
		//}

		// Completed Successfully
		return ok;

	}


	bool Database::ReadDbInfo(SDbInfo& dbInf)
	{
		SWORD cb;

		// SQLGetInfo gets null-terminated by the driver. It needs buffer-lengths (not char-lengts), even in unicode
		// see http://msdn.microsoft.com/en-us/library/ms711681%28v=vs.85%29.aspx
		// so it works with sizeof and statically declared arrays
		
		bool ok = true;
		ok = GetInfo(m_hdbc, SQL_SERVER_NAME, dbInf.serverName, sizeof(dbInf.serverName), &cb);
		ok = GetInfo(m_hdbc, SQL_DATABASE_NAME, dbInf.databaseName, sizeof(dbInf.databaseName), &cb);
		ok = GetInfo(m_hdbc, SQL_DBMS_NAME, dbInf.dbmsName, sizeof(dbInf.dbmsName), &cb);
		ok = GetInfo(m_hdbc, SQL_DBMS_VER, dbInf.dbmsVer, sizeof(dbInf.dbmsVer), &cb);
		ok = GetInfo(m_hdbc, SQL_MAX_DRIVER_CONNECTIONS, &dbInf.maxConnections, sizeof(dbInf.maxConnections), &cb);
		ok = GetInfo(m_hdbc, SQL_MAX_CONCURRENT_ACTIVITIES, &dbInf.maxStmts, sizeof(dbInf.maxStmts), &cb);
		ok = GetInfo(m_hdbc, SQL_DRIVER_NAME, dbInf.driverName, sizeof(dbInf.driverName), &cb);
		ok = GetInfo(m_hdbc, SQL_DRIVER_ODBC_VER, dbInf.odbcVer, sizeof(dbInf.odbcVer), &cb);
		ok = GetInfo(m_hdbc, SQL_ODBC_VER, dbInf.drvMgrOdbcVer, sizeof(dbInf.drvMgrOdbcVer), &cb);
		ok = GetInfo(m_hdbc, SQL_DRIVER_VER, dbInf.driverVer, sizeof(dbInf.driverVer), &cb);
		ok = GetInfo(m_hdbc, SQL_ODBC_SAG_CLI_CONFORMANCE, &dbInf.cliConfLvl, sizeof(dbInf.cliConfLvl), &cb);
		ok = GetInfo(m_hdbc, SQL_OUTER_JOINS, dbInf.outerJoins, sizeof(dbInf.outerJoins), &cb);
		ok = GetInfo(m_hdbc, SQL_PROCEDURES, dbInf.procedureSupport, sizeof(dbInf.procedureSupport), &cb);
		ok = GetInfo(m_hdbc, SQL_ACCESSIBLE_TABLES, dbInf.accessibleTables, sizeof(dbInf.accessibleTables), &cb);
		ok = GetInfo(m_hdbc, SQL_CURSOR_COMMIT_BEHAVIOR, &dbInf.cursorCommitBehavior, sizeof(dbInf.cursorCommitBehavior), &cb);
		ok = GetInfo(m_hdbc, SQL_CURSOR_ROLLBACK_BEHAVIOR, &dbInf.cursorRollbackBehavior, sizeof(dbInf.cursorRollbackBehavior), &cb);
		ok = GetInfo(m_hdbc, SQL_NON_NULLABLE_COLUMNS, &dbInf.supportNotNullClause, sizeof(dbInf.supportNotNullClause), &cb);
		ok = GetInfo(m_hdbc, SQL_ODBC_SQL_OPT_IEF, dbInf.supportIEF, sizeof(dbInf.supportIEF), &cb);
		ok = GetInfo(m_hdbc, SQL_DEFAULT_TXN_ISOLATION, &dbInf.txnIsolation, sizeof(dbInf.txnIsolation), &cb);
		ok = GetInfo(m_hdbc, SQL_TXN_ISOLATION_OPTION, &dbInf.txnIsolationOptions, sizeof(dbInf.txnIsolationOptions), &cb);
		ok = GetInfo(m_hdbc, SQL_POS_OPERATIONS, &dbInf.posOperations, sizeof(dbInf.posOperations), &cb);
		ok = GetInfo(m_hdbc, SQL_POSITIONED_STATEMENTS, &dbInf.posStmts, sizeof(dbInf.posStmts), &cb);
		ok = GetInfo(m_hdbc, SQL_SCROLL_OPTIONS, &dbInf.scrollOptions, sizeof(dbInf.scrollOptions), &cb);
		ok = GetInfo(m_hdbc, SQL_TXN_CAPABLE, &dbInf.txnCapable, sizeof(dbInf.txnCapable), &cb);

		// TODO: SQL_LOGIN_TIMEOUT is a Connection-Attribute
		//retcode = SQLGetInfo(m_hdbc, SQL_LOGIN_TIMEOUT, (UCHAR*) &dbInf.loginTimeout, sizeof(dbInf.loginTimeout), &cb);
		//if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		//{
		//	DispAllErrors(SQL_NULL_HENV, m_hdbc);
		//	if (failOnDataTypeUnsupported)
		//		return false;
		//}

		ok = GetInfo(m_hdbc, SQL_MAX_CATALOG_NAME_LEN, &dbInf.maxCatalogNameLen, sizeof(dbInf.maxCatalogNameLen), &cb);
		ok = GetInfo(m_hdbc,  SQL_MAX_SCHEMA_NAME_LEN, &dbInf.maxSchemaNameLen, sizeof(dbInf.maxSchemaNameLen), &cb);
		ok = GetInfo(m_hdbc, SQL_MAX_TABLE_NAME_LEN, &dbInf.maxTableNameLen, sizeof(dbInf.maxTableNameLen), &cb);

		if(!ok)
		{
			BOOST_LOG_TRIVIAL(error) << L"Not all Database Infos could be queried";
		}

		// Completed
		return ok;
	}


	bool Database::ReadCatalogInfo(ReadCatalogInfoMode mode, std::vector<std::wstring>& results)
	{
		results.empty();

		SQLPOINTER catalogName = L"";
		SQLPOINTER schemaName = L"";
		SQLPOINTER tableTypeName = L"";

		SQLSMALLINT colNr = 0;
		SQLUSMALLINT bufferLen = 0;

		switch(mode)
		{
		case AllCatalogs:
			catalogName = SQL_ALL_CATALOGS;
			colNr = 1;
			bufferLen = m_dbInf.GetMaxCatalogNameLen();
			break;
		case AllSchemas:
			schemaName = SQL_ALL_SCHEMAS;
			colNr = 2;
			bufferLen = m_dbInf.GetMaxSchemaNameLen();
			break;
		case AllTableTypes:
			tableTypeName = SQL_ALL_TABLE_TYPES;
			colNr = 4;
			bufferLen = m_dbInf.GetMaxTableTypeNameLen();
			break;
		default:
			exASSERT(false);
		}

		// Close Statement 
		SQLRETURN ret = CloseStmtHandle(m_hstmt, IgnoreNotOpen);
		if(ret != SQL_SUCCESS)
		{
			LOG_ERROR_STMT(m_hstmt, ret, CloseStmtHandle(IgnoreNotOpen) );
			return false;
		}

		ret = SQLTables(m_hstmt,
			(SQLWCHAR*) catalogName, SQL_NTS,   // catname                 
			(SQLWCHAR*) schemaName, SQL_NTS,   // schema name
			L"", SQL_NTS,							// table name
			(SQLWCHAR*) tableTypeName, SQL_NTS);

		if (ret != SQL_SUCCESS)
		{
			LOG_ERROR_STMT(m_hstmt, ret, SQLTables);

			// Silently try to close and fail
			CloseStmtHandle(m_hstmt, IgnoreNotOpen);
			return false;
		}

		// Read data
		SQLWCHAR* buffer = new SQLWCHAR[bufferLen];
		SQLLEN cb;
		bool ok = true;
		while (ok && (ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)   // Table Information
		{
			ok = GetData(m_hstmt, colNr, SQL_C_WXCHAR, buffer, bufferLen, &cb, NULL, true);
			if(ok)
				results.push_back(buffer);
		}

		if(ok && ret != SQL_NO_DATA)
		{
			LOG_ERROR_EXPECTED_SQL_NO_DATA(ret, GetData());
			ok = false;
		}
		
		CloseStmtHandle(m_hstmt, IgnoreNotOpen);
		delete[] buffer;
		
		return ok;
	}



	bool Database::GetAllDataTypesInfo(std::vector<SSqlTypeInfo>& types)
	{
		bool allOk = true;
		types.clear();

		// Close an eventually open cursor, do not care about truncation
		SQLRETURN ret = CloseStmtHandle(m_hstmt, IgnoreNotOpen);
		if( ! SQL_SUCCEEDED(ret))
		{
			LOG_ERROR_STMT(m_hstmt, ret, CloseStmtHandle(IgnoreNotOpen));
			return false;
		}

		ret = SQLGetTypeInfo(m_hstmt, SQL_ALL_TYPES);
		if(ret != SQL_SUCCESS)
		{
			LOG_ERROR_STMT(m_hstmt, ret, SQLGetTypeInfo);
			return false;
		}

		ret = SQLFetch(m_hstmt);
		int count = 0;
		SQLWCHAR typeName[DB_TYPE_NAME_LEN + 1];
		SQLWCHAR literalPrefix[DB_MAX_LITERAL_PREFIX_LEN + 1];
		SQLWCHAR literalSuffix[DB_MAX_LITERAL_SUFFIX_LEN + 1];
		SQLWCHAR createParams[DB_MAX_CREATE_PARAMS_LIST_LEN + 1];
		SQLWCHAR localTypeName[DB_LOCAL_TYPE_NAME_LEN + 1];
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
			bool ok = GetData(m_hstmt, 1, SQL_C_WCHAR, typeName, sizeof(typeName), &cb, NULL, true);
			if(ok)
				info.TypeName = typeName;
			bool haveName = ok;

			ok = ok & GetData(m_hstmt, 2, SQL_C_SSHORT, &info.FsqlType, sizeof(info.FsqlType), &cb, NULL);
			ok = ok & GetData(m_hstmt, 3, SQL_C_SLONG, &info.Precision, sizeof(info.Precision), &cb, &info.PrecisionIsNull);
			ok = ok & GetData(m_hstmt, 4, SQL_C_WCHAR, literalPrefix, sizeof(literalPrefix), &cb, &info.LiteralPrefixIsNull, true);
			info.LiteralPrefix = literalPrefix;
			ok = ok & GetData(m_hstmt, 5, SQL_C_WCHAR, literalSuffix, sizeof(literalSuffix), &cb, &info.LiteralSuffixIsNull, true);
			info.LiteralSuffix = literalSuffix;
			ok = ok & GetData(m_hstmt, 6, SQL_C_WCHAR, createParams, sizeof(createParams), &cb, &info.CreateParamsIsNull, true);
			info.CreateParams = createParams;
			ok = ok & GetData(m_hstmt, 7, SQL_C_SSHORT, &info.Nullable, sizeof(info.Nullable), &cb, NULL);
			ok = ok & GetData(m_hstmt, 8, SQL_C_SSHORT, &info.CaseSensitive, sizeof(info.CaseSensitive), &cb, NULL);
			ok = ok & GetData(m_hstmt, 9, SQL_C_SSHORT, &info.Searchable, sizeof(info.Searchable), &cb, NULL);
			ok = ok & GetData(m_hstmt, 10, SQL_C_SSHORT, &info.Unsigned, sizeof(info.Unsigned), &cb, &info.UnsignedIsNull);
			ok = ok & GetData(m_hstmt, 11, SQL_C_SSHORT, &info.FixedPrecisionScale, sizeof(info.FixedPrecisionScale), &cb, NULL);
			ok = ok & GetData(m_hstmt, 12, SQL_C_SSHORT, &info.AutoUniqueValue, sizeof(info.AutoUniqueValue), &cb, &info.AutoUniqueValueIsNull);
			ok = ok & GetData(m_hstmt, 13, SQL_C_WCHAR, localTypeName, sizeof(localTypeName), &cb, &info.LocalTypeNameIsNull, true);
			info.LocalTypeName = localTypeName;
			ok = ok & GetData(m_hstmt, 14, SQL_C_SSHORT, &info.MinimumScale, sizeof(info.MinimumScale), &cb, &info.MinimumScaleIsNull);
			ok = ok & GetData(m_hstmt, 15, SQL_C_SSHORT, &info.MaximumScale, sizeof(info.MaximumScale), &cb, &info.MaximumScaleIsNull);
			ok = ok & GetData(m_hstmt, 16, SQL_C_SSHORT, &info.SqlDataType, sizeof(info.SqlDataType), &cb, NULL);
			ok = ok & GetData(m_hstmt, 17, SQL_C_SSHORT, &info.SqlDateTimeSub, sizeof(info.SqlDateTimeSub), &cb, &info.SqlDateTimeSubIsNull);
			ok = ok & GetData(m_hstmt, 18, SQL_C_SSHORT, &info.NumPrecRadix, sizeof(info.NumPrecRadix), &cb, &info.NumPrecRadixIsNull);
			ok = ok & GetData(m_hstmt, 19, SQL_C_SSHORT, &info.IntervalPrecision, sizeof(info.IntervalPrecision), &cb, &info.IntervalPrecisionIsNull);

			if(ok)
			{
				types.push_back(info);
			}
			else
			{
				BOOST_LOG_TRIVIAL(error) << L"Failed to determine properties of type '" << (haveName ? info.TypeName : L"unknown-type") << L"'"; 
				allOk = false;
			}

			count++;
			ret = SQLFetch(m_hstmt);
		}

		if(ret != SQL_NO_DATA)
		{
			LOG_ERROR_EXPECTED_SQL_NO_DATA(ret, SQLFetch);
			return false;
		}

		// We are done, close cursor, do not care about truncation
		ret = CloseStmtHandle(m_hstmt, FailIfNotOpen);
		if( ! SQL_SUCCEEDED(ret))
		{
			LOG_ERROR_STMT(m_hstmt, ret, CloseStmtHandle);
			return false;
		}

		return allOk;
	}


	bool Database::Close()
	{
		// There should be zero Ctable objects still connected to this db object
		exASSERT(nTables == 0);

		// Free statement handle
		if (m_dbIsOpen)
		{
			SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
			if(ret != SQL_SUCCESS)
			{
				// if SQL_ERROR is returned, the handle is still valid, error information can be fetched
				if(ret == SQL_ERROR)
				{
					LOG_ERROR_STMT(m_hstmt, ret, SqlFreeHandle);
				}
				else
				{
					LOG_ERROR_SQL_NO_SUCCESS(ret, SQLFreeHandle);
				}
			}

			// Anyway try to disconnect from the datasource
			// This is a critical error.
			ret = SQLDisconnect(m_hdbc);
			if(ret != SQL_SUCCESS)
			{
				LOG_ERROR_DBC(m_hdbc, ret, SQLDisconnect);
				return false;
			}

			m_dbIsOpen = false;
		}

#ifdef EXODBCDEBUG
		{
#if wxUSE_THREADS
			wxCriticalSectionLocker lock(csTablesInUse);
#endif // wxUSE_THREADS
			STablesInUse *tiu;
			std::vector<STablesInUse*>::const_iterator it = TablesInUse.begin();
			//wxList::compatibility_iterator pNode;
			//pNode = TablesInUse.GetFirst();
			std::wstring s1, s2;
			while (it != TablesInUse.end())
			{
				tiu = *it;;
				if (tiu->pDb == this)
				{
					s1 = (boost::wformat(L"Orphaned table found using pDb [%p]: ") % this).str();
					s2 = (boost::wformat(L"(%-20s)     tableID:[%6lu]     pDb:[%p]") % tiu->tableName % tiu->tableID % this).str();
					BOOST_LOG_TRIVIAL(debug) << s1 << s2;
				}
				it++;
				//pNode = pNode->GetNext();
			}
		}
#endif

		return true;
	}


	bool Database::CommitTrans()
	{
		// Commit the transaction
		SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_hdbc, SQL_COMMIT);
		if( ret != SQL_SUCCESS)
		{
			LOG_ERROR_DBC_MSG(m_hdbc, ret, SQLEndTran, L"Failed to Commit Transaction");
			return false;
		}

		// Completed successfully
		return true;
	}



	bool Database::RollbackTrans()
	{
		// Rollback the transaction
		SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_hdbc, SQL_ROLLBACK);
		if( ret != SQL_SUCCESS)
		{
			LOG_ERROR_DBC_MSG(m_hdbc, ret, SQLEndTran, L"Failed to Rollback Transaction");
			return false;
		}

		// Completed successfully
		return true;
	}


	/********** wxDb::DispAllErrors() **********/
	bool Database::DispAllErrors(HENV aHenv, HDBC aHdbc, HSTMT aHstmt)
		/*
		* This function is called internally whenever an error condition prevents the user's
		* request from being executed.  This function will query the datasource as to the
		* actual error(s) that just occurred on the previous request of the datasource.
		*
		* The function will retrieve each error condition from the datasource and
		* Printf the codes/text values into a string which it then logs via LogErrorImpl().
		* If in DBDEBUG_CONSOLE mode, the constructed string will be displayed in the console
		* window and program execution will be paused until the user presses a key.
		*
		* This function always returns false, so that functions which call this function
		* can have a line like "return (DispAllErrors(henv, hdbc));" to indicate the failure
		* of the user's request, so that the calling code can then process the error message log.
		*/
	{
		std::vector<SErrorInfo> errs = GetAllErrors(aHenv, aHdbc, aHstmt);
		for(size_t i = 0; i < errs.size(); i++)
		{
			BOOST_LOG_TRIVIAL(warning) << L"Have ODBC Error #" << i << L": " << errs[i];
		}
 
//		std::wstring odbcErrMsg;
//
//		while (SQLError(aHenv, aHdbc, aHstmt, (SQLTCHAR FAR *) sqlState, &nativeError, (SQLTCHAR FAR *) errorMsg, SQL_MAX_MESSAGE_LENGTH - 1, &cbErrorMsg) == SQL_SUCCESS)
//		{
//			odbcErrMsg = (boost::wformat(L"SQL State = %s\nNative Error Code = %li\nError Message = %s\n") % sqlState % (long)nativeError % errorMsg).str();
//			LogErrorImpl(odbcErrMsg, sqlState);
//			if (!m_silent)
//			{
//#ifdef DBDEBUG_CONSOLE
//				// When run in console mode, use standard out to display errors.
//				std::wcout << odbcErrMsg.c_str() << std::endl;
//				std::wcout << L"Press any key to continue..." << std::endl;
//				getchar();
//#endif
//
//#ifdef EXODBCDEBUG
//				BOOST_LOG_TRIVIAL(debug) <<  L"ODBC DEBUG MESSAGE from DispAllErrors(): " << odbcErrMsg;
//#endif
//			}
//		}

		return false;  // This function always returns false.

	}



	/**********wxDb::TranslateSqlState()  **********/
	int Database::TranslateSqlState(const std::wstring &SQLState)
	{
		if (SQLState == L"01000")
			return(DB_ERR_GENERAL_WARNING);
		if (SQLState == L"01002")
			return(DB_ERR_DISCONNECT_ERROR);
		if (SQLState == L"01004")
			return(DB_ERR_DATA_TRUNCATED);
		if (SQLState == L"01006")
			return(DB_ERR_PRIV_NOT_REVOKED);
		if (SQLState == L"01S00")
			return(DB_ERR_INVALID_CONN_STR_ATTR);
		if (SQLState == L"01S01")
			return(DB_ERR_ERROR_IN_ROW);
		if (SQLState == L"01S02")
			return(DB_ERR_OPTION_VALUE_CHANGED);
		if (SQLState == L"01S03")
			return(DB_ERR_NO_ROWS_UPD_OR_DEL);
		if (SQLState == L"01S04")
			return(DB_ERR_MULTI_ROWS_UPD_OR_DEL);
		if (SQLState == L"07001")
			return(DB_ERR_WRONG_NO_OF_PARAMS);
		if (SQLState == L"07006")
			return(DB_ERR_DATA_TYPE_ATTR_VIOL);
		if (SQLState == L"08001")
			return(DB_ERR_UNABLE_TO_CONNECT);
		if (SQLState == L"08002")
			return(DB_ERR_CONNECTION_IN_USE);
		if (SQLState == L"08003")
			return(DB_ERR_CONNECTION_NOT_OPEN);
		if (SQLState == L"08004")
			return(DB_ERR_REJECTED_CONNECTION);
		if (SQLState == L"08007")
			return(DB_ERR_CONN_FAIL_IN_TRANS);
		if (SQLState == L"08S01")
			return(DB_ERR_COMM_LINK_FAILURE);
		if (SQLState == L"21S01")
			return(DB_ERR_INSERT_VALUE_LIST_MISMATCH);
		if (SQLState == L"21S02")
			return(DB_ERR_DERIVED_TABLE_MISMATCH);
		if (SQLState == L"22001")
			return(DB_ERR_STRING_RIGHT_TRUNC);
		if (SQLState == L"22003")
			return(DB_ERR_NUMERIC_VALUE_OUT_OF_RNG);
		if (SQLState == L"22005")
			return(DB_ERR_ERROR_IN_ASSIGNMENT);
		if (SQLState == L"22008")
			return(DB_ERR_DATETIME_FLD_OVERFLOW);
		if (SQLState == L"22012")
			return(DB_ERR_DIVIDE_BY_ZERO);
		if (SQLState == L"22026")
			return(DB_ERR_STR_DATA_LENGTH_MISMATCH);
		if (SQLState == L"23000")
			return(DB_ERR_INTEGRITY_CONSTRAINT_VIOL);
		if (SQLState == L"24000")
			return(DB_ERR_INVALID_CURSOR_STATE);
		if (SQLState == L"25000")
			return(DB_ERR_INVALID_TRANS_STATE);
		if (SQLState == L"28000")
			return(DB_ERR_INVALID_AUTH_SPEC);
		if (SQLState == L"34000")
			return(DB_ERR_INVALID_CURSOR_NAME);
		if (SQLState == L"37000")
			return(DB_ERR_SYNTAX_ERROR_OR_ACCESS_VIOL);
		if (SQLState == L"3C000")
			return(DB_ERR_DUPLICATE_CURSOR_NAME);
		if (SQLState == L"40001")
			return(DB_ERR_SERIALIZATION_FAILURE);
		if (SQLState == L"42000")
			return(DB_ERR_SYNTAX_ERROR_OR_ACCESS_VIOL2);
		if (SQLState == L"70100")
			return(DB_ERR_OPERATION_ABORTED);
		if (SQLState == L"IM001")
			return(DB_ERR_UNSUPPORTED_FUNCTION);
		if (SQLState == L"IM002")
			return(DB_ERR_NO_DATA_SOURCE);
		if (SQLState == L"IM003")
			return(DB_ERR_DRIVER_LOAD_ERROR);
		if (SQLState == L"IM004")
			return(DB_ERR_SQLALLOCENV_FAILED);
		if (SQLState == L"IM005")
			return(DB_ERR_SQLALLOCCONNECT_FAILED);
		if (SQLState == L"IM006")
			return(DB_ERR_SQLSETCONNECTOPTION_FAILED);
		if (SQLState == L"IM007")
			return(DB_ERR_NO_DATA_SOURCE_DLG_PROHIB);
		if (SQLState == L"IM008")
			return(DB_ERR_DIALOG_FAILED);
		if (SQLState == L"IM009")
			return(DB_ERR_UNABLE_TO_LOAD_TRANSLATION_DLL);
		if (SQLState == L"IM010")
			return(DB_ERR_DATA_SOURCE_NAME_TOO_LONG);
		if (SQLState == L"IM011")
			return(DB_ERR_DRIVER_NAME_TOO_LONG);
		if (SQLState == L"IM012")
			return(DB_ERR_DRIVER_KEYWORD_SYNTAX_ERROR);
		if (SQLState == L"IM013")
			return(DB_ERR_TRACE_FILE_ERROR);
		if (SQLState == L"S0001")
			return(DB_ERR_TABLE_OR_VIEW_ALREADY_EXISTS);
		if (SQLState == L"S0002")
			return(DB_ERR_TABLE_NOT_FOUND);
		if (SQLState == L"S0011")
			return(DB_ERR_INDEX_ALREADY_EXISTS);
		if (SQLState == L"S0012")
			return(DB_ERR_INDEX_NOT_FOUND);
		if (SQLState == L"S0021")
			return(DB_ERR_COLUMN_ALREADY_EXISTS);
		if (SQLState == L"S0022")
			return(DB_ERR_COLUMN_NOT_FOUND);
		if (SQLState == L"S0023")
			return(DB_ERR_NO_DEFAULT_FOR_COLUMN);
		if (SQLState == L"S1000")
			return(DB_ERR_GENERAL_ERROR);
		if (SQLState == L"S1001")
			return(DB_ERR_MEMORY_ALLOCATION_FAILURE);
		if (SQLState == L"S1002")
			return(DB_ERR_INVALID_COLUMN_NUMBER);
		if (SQLState == L"S1003")
			return(DB_ERR_PROGRAM_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1004")
			return(DB_ERR_SQL_DATA_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1008")
			return(DB_ERR_OPERATION_CANCELLED);
		if (SQLState == L"S1009")
			return(DB_ERR_INVALID_ARGUMENT_VALUE);
		if (SQLState == L"S1010")
			return(DB_ERR_FUNCTION_SEQUENCE_ERROR);
		if (SQLState == L"S1011")
			return(DB_ERR_OPERATION_INVALID_AT_THIS_TIME);
		if (SQLState == L"S1012")
			return(DB_ERR_INVALID_TRANS_OPERATION_CODE);
		if (SQLState == L"S1015")
			return(DB_ERR_NO_CURSOR_NAME_AVAIL);
		if (SQLState == L"S1090")
			return(DB_ERR_INVALID_STR_OR_BUF_LEN);
		if (SQLState == L"S1091")
			return(DB_ERR_DESCRIPTOR_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1092")
			return(DB_ERR_OPTION_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1093")
			return(DB_ERR_INVALID_PARAM_NO);
		if (SQLState == L"S1094")
			return(DB_ERR_INVALID_SCALE_VALUE);
		if (SQLState == L"S1095")
			return(DB_ERR_FUNCTION_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1096")
			return(DB_ERR_INF_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1097")
			return(DB_ERR_COLUMN_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1098")
			return(DB_ERR_SCOPE_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1099")
			return(DB_ERR_NULLABLE_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1100")
			return(DB_ERR_UNIQUENESS_OPTION_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1101")
			return(DB_ERR_ACCURACY_OPTION_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1103")
			return(DB_ERR_DIRECTION_OPTION_OUT_OF_RANGE);
		if (SQLState == L"S1104")
			return(DB_ERR_INVALID_PRECISION_VALUE);
		if (SQLState == L"S1105")
			return(DB_ERR_INVALID_PARAM_TYPE);
		if (SQLState == L"S1106")
			return(DB_ERR_FETCH_TYPE_OUT_OF_RANGE);
		if (SQLState == L"S1107")
			return(DB_ERR_ROW_VALUE_OUT_OF_RANGE);
		if (SQLState == L"S1108")
			return(DB_ERR_CONCURRENCY_OPTION_OUT_OF_RANGE);
		if (SQLState == L"S1109")
			return(DB_ERR_INVALID_CURSOR_POSITION);
		if (SQLState == L"S1110")
			return(DB_ERR_INVALID_DRIVER_COMPLETION);
		if (SQLState == L"S1111")
			return(DB_ERR_INVALID_BOOKMARK_VALUE);
		if (SQLState == L"S1C00")
			return(DB_ERR_DRIVER_NOT_CAPABLE);
		if (SQLState == L"S1T00")
			return(DB_ERR_TIMEOUT_EXPIRED);

		// No match
		return(0);

	}  // wxDb::TranslateSqlState()


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
//				if (!((Dbms() == dbmsSYBASE_ASA    && !wcscmp(sqlState, L"42000"))))  // 5.x (and lower?)
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


	bool Database::ExecSql(const std::wstring& sqlStmt, ExecFailMode mode /* = NotFailOnNoData */)
	{
		RETCODE retcode;

		SQLFreeStmt(m_hstmt, SQL_CLOSE);

		retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
		if(retcode != SQL_SUCCESS)
		{
			if( ! (mode == NotFailOnNoData && retcode == SQL_NO_DATA))
			{
				LOG_ERROR_STMT_MSG(m_hstmt, retcode, SQLExecDirect, (boost::wformat(L"Failed to execute Stmt '%s'") %sqlStmt).str());
				return false;
			}
		}

		return true;
	}


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

	/********** wxDb::GetNext()  **********/
	bool Database::GetNext()
	{
		if (SQLFetch(m_hstmt) == SQL_SUCCESS)
			return true;
		else
		{
			DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt);
			return false;
		}

	}  // wxDb::GetNext()



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
//				if ((Dbms() == dbmsORACLE) ||
//					(Dbms() == dbmsFIREBIRD) ||
//					(Dbms() == dbmsINTERBASE))
//					boost::algorithm::to_upper(TableName);
//
//				SQLFreeStmt(m_hstmt, SQL_CLOSE);
//
//				// MySQL, SQLServer, and Access cannot accept a user name when looking up column names, so we
//				// use the call below that leaves out the user name
//				if (!UserID.empty() &&
//					Dbms() != dbmsMY_SQL &&
//					Dbms() != dbmsACCESS &&
//					Dbms() != dbmsMS_SQL_SERVER)
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
//			if ((Dbms() == dbmsORACLE) ||
//				(Dbms() == dbmsFIREBIRD) ||
//				(Dbms() == dbmsINTERBASE))
//				boost::algorithm::to_upper(TableName);
//
//			SQLFreeStmt(m_hstmt, SQL_CLOSE);
//
//			// MySQL, SQLServer, and Access cannot accept a user name when looking up column names, so we
//			// use the call below that leaves out the user name
//			if (!UserID.empty() &&
//				Dbms() != dbmsMY_SQL &&
//				Dbms() != dbmsACCESS &&
//				Dbms() != dbmsMS_SQL_SERVER)
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
//						if (Dbms() == dbmsVIRTUOSO)
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
//			if ((Dbms() == dbmsORACLE) ||
//				(Dbms() == dbmsFIREBIRD) ||
//				(Dbms() == dbmsINTERBASE))
//				TableName = TableName.Upper();
//
//			SQLFreeStmt(m_hstmt, SQL_CLOSE);
//
//			// MySQL, SQLServer, and Access cannot accept a user name when looking up column names, so we
//			// use the call below that leaves out the user name
//			if (!UserID.empty() &&
//				Dbms() != dbmsMY_SQL &&
//				Dbms() != dbmsACCESS &&
//				Dbms() != dbmsMS_SQL_SERVER)
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

	bool Database::FindTables(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType, std::vector<DbCatalogTable>& tables)
	{
		// Clear tables
		tables.clear();

		// Free statement, ignore if already closed
		// Close an eventually open cursor, do not care about truncation
		SQLRETURN ret = CloseStmtHandle(m_hstmt, IgnoreNotOpen);
		if( ! SQL_SUCCEEDED(ret))
		{
			LOG_ERROR_STMT(m_hstmt, ret, CloseStmtHandle(IgnoreNotOpen));
			return false;
		}

		SQLWCHAR* pTableName = NULL;
		SQLWCHAR* pSchemaName = NULL;
		SQLWCHAR* pCatalogName = NULL;
		SQLWCHAR* pTableType = NULL;

		if(tableName.length() > 0)
		{
			pTableName = new SQLWCHAR[tableName.length() + 1];
			wcscpy(pTableName, tableName.c_str());
		}
		if(schemaName.length() > 0)
		{
			pSchemaName = new SQLWCHAR[schemaName.length() + 1];
			wcscpy(pSchemaName, schemaName.c_str());
		}
		if(catalogName.length() > 0)
		{
			pCatalogName = new SQLWCHAR[catalogName.length() + 1];
			wcscpy(pCatalogName, catalogName.c_str());
		}
		if(tableType.length() > 0)
		{
			pTableType = new SQLWCHAR[tableType.length() + 1];
			wcscpy(pTableType, tableType.c_str());
		}

		wchar_t* buffCatalog = new wchar_t[m_dbInf.GetMaxCatalogNameLen()];
		wchar_t* buffSchema = new wchar_t[m_dbInf.GetMaxSchemaNameLen()];
		wchar_t* buffTableName = new wchar_t[m_dbInf.GetMaxTableNameLen()];
		wchar_t* buffTableType = new wchar_t[DB_MAX_TABLE_TYPE_LEN + 1];
		wchar_t* buffTableRemarks = new wchar_t[DB_MAX_TABLE_REMARKS_LEN + 1];

		bool ok = true;
		// Query db
		ret = SQLTables(m_hstmt,
			pCatalogName, pCatalogName ? SQL_NTS : NULL,   // catname                 
			pSchemaName, pSchemaName ? SQL_NTS : NULL,   // schema name
			pTableName, pTableName ? SQL_NTS : NULL,							// table name
			pTableType, pTableType ? SQL_NTS : NULL);

		if(ret != SQL_SUCCESS)
		{
			LOG_ERROR_STMT(m_hstmt, ret, SQLTables);
			ok = false;
		}
		else
		{
			buffCatalog[0] = 0;
			buffSchema[0] = 0;
			buffTableName[0] = 0;
			buffTableType[0] = 0;
			buffTableRemarks[0] = 0;

			while(ok && (ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)
			{
				DbCatalogTable table;
				bool haveAllData = true;
				SQLLEN cb;
				haveAllData = haveAllData & GetData(m_hstmt, 1, SQL_C_WCHAR, buffCatalog, m_dbInf.GetMaxCatalogNameLen(), &cb, &table.m_isCatalogNull, true);
				haveAllData = haveAllData & GetData(m_hstmt, 2, SQL_C_WCHAR, buffSchema, m_dbInf.GetMaxSchemaNameLen(), &cb, &table.m_isSchemaNull, true);
				haveAllData = haveAllData & GetData(m_hstmt, 3, SQL_C_WCHAR, buffTableName, m_dbInf.GetMaxTableNameLen(), &cb, NULL, true);
				haveAllData = haveAllData & GetData(m_hstmt, 4, SQL_C_WCHAR, buffTableType, DB_MAX_TABLE_TYPE_LEN + 1, &cb, NULL, true);
				haveAllData = haveAllData & GetData(m_hstmt, 5, SQL_C_WCHAR, buffTableRemarks, DB_MAX_TABLE_REMARKS_LEN + 1, &cb, NULL, true);

				if(!haveAllData)
				{
					ok = false;
					LOG_ERROR(L"Failed to Read Data from a record while finding tables");
				}
				else
				{
					if(!table.m_isCatalogNull)
						table.m_catalog = buffCatalog;
					if(!table.m_isSchemaNull)
						table.m_schema = buffSchema;
					table.m_tableName = buffTableName;
					table.m_tableType = buffTableType;
					table.m_tableRemarks = buffTableRemarks;
					tables.push_back(table);
				}

			}
			if(ret != SQL_NO_DATA)
			{
				LOG_ERROR_EXPECTED_SQL_NO_DATA(ret, SQLFetch);
				ok = false;
			}
		}

		// Close, ignore all errs
		CloseStmtHandle(m_hstmt, IgnoreNotOpen);

		delete[] buffCatalog;
		delete[] buffSchema;
		delete[] buffTableName;
		delete[] buffTableType;
		delete[] buffTableRemarks;

		if(pTableName)
			delete[] pTableName;
		if(pSchemaName)
			delete[] pSchemaName;
		if(pCatalogName)
			delete[] pCatalogName;
		if(pTableType)
			delete[] pTableType;

		return ok;
	}


	int Database::ReadColumnCount(const std::wstring& tableName, const std::wstring& schemaName /* = L"" */, const std::wstring catalogName /* = L"" */)
	{
		UWORD    noCols = 0;

		// Free statement, ignore if already closed
		// Close an eventually open cursor, do not care about truncation
		SQLRETURN ret = CloseStmtHandle(m_hstmt, IgnoreNotOpen);
		if( ! SQL_SUCCEEDED(ret))
		{
			LOG_ERROR_STMT(m_hstmt, ret, CloseStmtHandle(IgnoreNotOpen));
			return false;
		}

		ret = SQLColumns(m_hstmt,
				(SQLWCHAR*) catalogName.c_str(), SQL_NTS,	// catalog
				(SQLWCHAR*) schemaName.c_str(), SQL_NTS,	// schema
				(SQLWCHAR*) tableName.c_str(), SQL_NTS,		// tablename
				NULL, 0);						// All columns

		if(ret != SQL_SUCCESS)
		{
			CloseStmtHandle(m_hstmt, IgnoreNotOpen);
			LOG_ERROR_STMT(m_hstmt, ret, SQLColumns);
			return -1;
		}

		// Count the columns
		while ((ret = SQLFetch(m_hstmt)) == SQL_SUCCESS)
		{
			noCols++;
		}

		if(ret != SQL_NO_DATA)
		{
			LOG_ERROR_EXPECTED_SQL_NO_DATA(ret, SQLFetch);
			return -1;
		}

		ret = CloseStmtHandle(m_hstmt, IgnoreNotOpen);
		if( ! SQL_SUCCEEDED(ret))
		{
			LOG_ERROR_STMT(m_hstmt, ret, CloseStmtHandle(IgnoreNotOpen));
			return -1;
		}
		return noCols;

	}


	bool Database::ReadCompleteCatalog(SDbCatalog& catalogInfo)
	{
		SDbCatalog dbInf;

		if(!FindTables(L"", L"", L"", L"", dbInf.m_tables))
		{
			return false;
		}

		std::vector<DbCatalogTable>::const_iterator it;
		for(it = dbInf.m_tables.begin(); it != dbInf.m_tables.end(); it++)
		{
			const DbCatalogTable& table = *it;
			if(!table.m_isCatalogNull)
				dbInf.m_catalogs.insert(table.m_catalog);
			if(!table.m_isSchemaNull)
				dbInf.m_schemas.insert(table.m_schema);
		}

		catalogInfo = dbInf;
		return true;
	}



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
	//		Dbms() != dbmsMY_SQL &&
	//		Dbms() != dbmsACCESS &&
	//		Dbms() != dbmsFIREBIRD &&
	//		Dbms() != dbmsINTERBASE &&
	//		Dbms() != dbmsMS_SQL_SERVER)
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


	//bool Database::TableExists(const std::wstring &tableName, const wchar_t *userID, const std::wstring &tablePath)
	//	/*
	//	* Table name can refer to a table, view, alias or synonym.  Returns true
	//	* if the object exists in the database.  This function does not indicate
	//	* whether or not the user has privleges to query or perform other functions
	//	* on the table.
	//	*
	//	* userID is evaluated in the following manner:
	//	*        userID == NULL  ... UserID is ignored
	//	*        userID == ""    ... UserID set equal to 'this->uid'
	//	*        userID != ""    ... UserID set equal to 'userID'
	//	*/
	//{
	//	exASSERT(tableName.length());

	//	std::wstring TableName;

	//	if (Dbms() == dbmsDBASE)
	//	{
	//		exFAIL_MSG("dbmsDBASE support is not enabled");
	//		//     std::wstring dbName;
	//		//     if (tablePath.length())
	//		//dbName = (boost::wformat(L"%s/%s.dbf") % tablePath % tableName).str();
	//		//     else
	//		//dbName = (boost::wformat(L"%s.dbf") % tableName).str();

	//		//     bool exists;
	//		//     exists = wxFileExists(dbName);
	//		//     return exists;
	//	}

	//	std::wstring UserID = ConvertUserIDImpl(userID);

	//	TableName = tableName;
	//	// Oracle and Interbase table names are uppercase only, so force
	//	// the name to uppercase just in case programmer forgot to do this
	//	if ((Dbms() == dbmsORACLE) ||
	//		(Dbms() == dbmsFIREBIRD) ||
	//		(Dbms() == dbmsINTERBASE))
	//		boost::algorithm::to_upper(TableName);

	//	SQLFreeStmt(m_hstmt, SQL_CLOSE);
	//	RETCODE retcode;

	//	// Some databases cannot accept a user name when looking up table names,
	//	// so we use the call below that leaves out the user name
	//	if (!UserID.empty() &&
	//		Dbms() != dbmsMY_SQL &&
	//		Dbms() != dbmsACCESS &&
	//		Dbms() != dbmsMS_SQL_SERVER &&
	//		Dbms() != dbmsDB2 &&
	//		Dbms() != dbmsFIREBIRD &&
	//		Dbms() != dbmsINTERBASE &&
	//		Dbms() != dbmsPERVASIVE_SQL)
	//	{
	//		retcode = SQLTables(m_hstmt,
	//			NULL, 0,                                  // All qualifiers
	//			(SQLTCHAR *) UserID.c_str(), SQL_NTS,        // Only tables owned by this user
	//			(SQLTCHAR FAR *)TableName.c_str(), SQL_NTS,
	//			NULL, 0);                                 // All table types
	//	}
	//	else
	//	{
	//		retcode = SQLTables(m_hstmt,
	//			NULL, 0,                                  // All qualifiers
	//			NULL, 0,                                  // All owners
	//			(SQLTCHAR FAR *)TableName.c_str(), SQL_NTS,
	//			NULL, 0);                                 // All table types
	//	}
	//	if (retcode != SQL_SUCCESS)
	//		return(DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt));

	//	retcode = SQLFetch(m_hstmt);
	//	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	//	{
	//		SQLFreeStmt(m_hstmt, SQL_CLOSE);
	//		return(DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt));
	//	}

	//	SQLFreeStmt(m_hstmt, SQL_CLOSE);

	//	return true;

	//}  // wxDb::TableExists()


//	/********** wxDb::TablePrivileges() **********/
//	bool Database::TablePrivileges(const std::wstring &tableName, const std::wstring &priv, const wchar_t *userID,
//		const wchar_t *schema, const std::wstring& tablePath)
//	{
//		exASSERT(tableName.length());
//
//		wxDbTablePrivilegeInfo  result;
//		SQLLEN  cbRetVal;
//		RETCODE retcode;
//
//		// We probably need to be able to dynamically set this based on
//		// the driver type, and state.
//		wchar_t curRole[] = L"public";
//
//		std::wstring TableName;
//
//		std::wstring UserID = ConvertUserIDImpl(userID);
//		std::wstring Schema = ConvertUserIDImpl(schema);
//
//		TableName = tableName;
//		// Oracle and Interbase table names are uppercase only, so force
//		// the name to uppercase just in case programmer forgot to do this
//		if ((Dbms() == dbmsORACLE) ||
//			(Dbms() == dbmsFIREBIRD) ||
//			(Dbms() == dbmsINTERBASE))
//			boost::algorithm::to_upper(TableName);
//
//		SQLFreeStmt(m_hstmt, SQL_CLOSE);
//
//		// Some databases cannot accept a user name when looking up table names,
//		// so we use the call below that leaves out the user name
//		if (!Schema.empty() &&
//			Dbms() != dbmsMY_SQL &&
//			Dbms() != dbmsACCESS &&
//			Dbms() != dbmsMS_SQL_SERVER)
//		{
//			retcode = SQLTablePrivileges(m_hstmt,
//				NULL, 0,                                    // Catalog
//				(SQLTCHAR FAR *)Schema.c_str(), SQL_NTS,               // Schema
//				(SQLTCHAR FAR *)TableName.c_str(), SQL_NTS);
//		}
//		else
//		{
//			retcode = SQLTablePrivileges(m_hstmt,
//				NULL, 0,                                    // Catalog
//				NULL, 0,                                    // Schema
//				(SQLTCHAR FAR *)TableName.c_str(), SQL_NTS);
//		}
//
//#ifdef DBDEBUG_CONSOLE
//		std::wcerr << L"SQLTablePrivileges() returned " << retcode << std::endl;
//#endif
//
//		if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
//			return (DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt));
//
//		bool failed = false;
//		retcode = SQLFetch(m_hstmt);
//		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
//		{
//			if (SQLGetData(m_hstmt, 1, SQL_C_WXCHAR, (UCHAR*) result.tableQual, sizeof(result.tableQual), &cbRetVal) != SQL_SUCCESS)
//				failed = true;
//
//			if (!failed && SQLGetData(m_hstmt, 2, SQL_C_WXCHAR, (UCHAR*) result.tableOwner, sizeof(result.tableOwner), &cbRetVal) != SQL_SUCCESS)
//				failed = true;
//
//			if (!failed && SQLGetData(m_hstmt, 3, SQL_C_WXCHAR, (UCHAR*) result.tableName, sizeof(result.tableName), &cbRetVal) != SQL_SUCCESS)
//				failed = true;
//
//			if (!failed && SQLGetData(m_hstmt, 4, SQL_C_WXCHAR, (UCHAR*) result.grantor, sizeof(result.grantor), &cbRetVal) != SQL_SUCCESS)
//				failed = true;
//
//			if (!failed && SQLGetData(m_hstmt, 5, SQL_C_WXCHAR, (UCHAR*) result.grantee, sizeof(result.grantee), &cbRetVal) != SQL_SUCCESS)
//				failed = true;
//
//			if (!failed && SQLGetData(m_hstmt, 6, SQL_C_WXCHAR, (UCHAR*) result.privilege, sizeof(result.privilege), &cbRetVal) != SQL_SUCCESS)
//				failed = true;
//
//			if (!failed && SQLGetData(m_hstmt, 7, SQL_C_WXCHAR, (UCHAR*) result.grantable, sizeof(result.grantable), &cbRetVal) != SQL_SUCCESS)
//				failed = true;
//
//			if (failed)
//			{
//				return(DispAllErrors(SQL_NULL_HENV, SQL_NULL_HDBC, m_hstmt));
//			}
//#ifdef DBDEBUG_CONSOLE
//			std::wcerr << "Scanning " << result.privilege <<" privilege on table " << result.tableOwner << "." << result.tableName << " granted by " << result.grantor << " to " << result.grantee << std::endl;
//#endif
//
//			if(boost::algorithm::iequals(UserID, result.tableOwner))
//			{
//				SQLFreeStmt(m_hstmt, SQL_CLOSE);
//				return true;
//			}
//
//			if(boost::algorithm::iequals(UserID, result.grantee) &&
//				!wcscmp(result.privilege, priv.c_str()))
//			{
//				SQLFreeStmt(m_hstmt, SQL_CLOSE);
//				return true;
//			}
//
//			if (!wcscmp(result.grantee,curRole) &&
//				!wcscmp(result.privilege, priv.c_str()))
//			{
//				SQLFreeStmt(m_hstmt, SQL_CLOSE);
//				return true;
//			}
//
//			retcode = SQLFetch(m_hstmt);
//		}
//
//		SQLFreeStmt(m_hstmt, SQL_CLOSE);
//		return false;
//
//	}  // wxDb::TablePrivileges


	//const std::wstring Database::SQLTableName(const wchar_t *tableName)
	//{
	//	std::wstring TableName;

	//	if (Dbms() == dbmsACCESS)
	//		TableName = L"\"";
	//	TableName += tableName;
	//	if (Dbms() == dbmsACCESS)
	//		TableName += L"\"";

	//	return TableName;
	//}  // wxDb::SQLTableName()


	//const std::wstring Database::SQLColumnName(const wchar_t *colName)
	//{
	//	std::wstring ColName;

	//	if (Dbms() == dbmsACCESS)
	//		ColName = L"\"";
	//	ColName += colName;
	//	if (Dbms() == dbmsACCESS)
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


	/********** wxDb::Dbms() **********/
	wxDBMS Database::Dbms()
		/*
		* Be aware that not all database engines use the exact same syntax, and not
		* every ODBC compliant database is compliant to the same level of compliancy.
		* Some manufacturers support the minimum Level 1 compliancy, and others up
		* through Level 3.  Others support subsets of features for levels above 1.
		*
		* If you find an inconsistency between the wxDb class and a specific database
		* engine, and an identifier to this section, and special handle the database in
		* the area where behavior is non-conforming with the other databases.
		*
		*
		* NOTES ABOUT ISSUES SPECIFIC TO EACH DATABASE ENGINE
		* ---------------------------------------------------
		*
		* ORACLE
		*        - Currently the only database supported by the class to support VIEWS
		*
		* DBASE
		*        - Does not support the SQL_TIMESTAMP structure
		*        - Supports only one cursor and one connect (apparently? with Microsoft driver only?)
		*        - Does not automatically create the primary index if the 'keyField' param of SetColDef
		*            is true.  The user must create ALL indexes from their program.
		*        - Table names can only be 8 characters long
		*        - Column names can only be 10 characters long
		*
		* SYBASE (all)
		*        - To lock a record during QUERY functions, the reserved word 'HOLDLOCK' must be added
		*            after every table name involved in the query/join if that tables matching record(s)
		*            are to be locked
		*        - Ignores the keywords 'FOR UPDATE'.  Use the HOLDLOCK functionality described above
		*
		* SYBASE (Enterprise)
		*        - If a column is part of the Primary Key, the column cannot be NULL
		*        - Maximum row size is somewhere in the neighborhood of 1920 bytes
		*
		* MY_SQL
		*        - If a column is part of the Primary Key, the column cannot be NULL
		*        - Cannot support selecting for update [::CanSelectForUpdate()].  Always returns FALSE
		*        - Columns that are part of primary or secondary keys must be defined as being NOT NULL
		*            when they are created.  Some code is added in ::CreateIndex to try to adjust the
		*            column definition if it is not defined correctly, but it is experimental
		*        - Does not support sub-queries in SQL statements
		*
		* POSTGRES
		*        - Does not support the keywords 'ASC' or 'DESC' as of release v6.5.0
		*        - Does not support sub-queries in SQL statements
		*
		* DB2
		*        - Primary keys must be declared as NOT NULL
		*        - Table and index names must not be longer than 13 characters in length (technically
		*          table names can be up to 18 characters, but the primary index is created using the
		*          base table name plus "_PIDX", so the limit if the table has a primary index is 13.
		*
		* PERVASIVE SQL
		*
		* INTERBASE
		*        - Columns that are part of primary keys must be defined as being NOT NULL
		*          when they are created.  Some code is added in ::CreateIndex to try to adjust the
		*          column definition if it is not defined correctly, but it is experimental
		*/
	{
		// Should only need to do this once for each new database connection
		// so return the value we already determined it to be to save time
		// and lots of string comparisons
		if (m_dbmsType != dbmsUNIDENTIFIED)
			return(m_dbmsType);

#ifdef DBDEBUG_CONSOLE
		// When run in console mode, use standard out to display errors.
		std::wcout << "Database connecting to: " << m_dbInf.dbmsName << std::endl;
#endif  // DBDEBUG_CONSOLE

		BOOST_LOG_TRIVIAL(debug) << L"Database connecting to: " << m_dbInf.dbmsName;

		wchar_t baseName[25+1];
		wcsncpy(baseName, m_dbInf.dbmsName, 25);
		baseName[25] = 0;

		// RGG 20001025 : add support for Interbase
		// GT : Integrated to base classes on 20001121
		if (!_wcsicmp(m_dbInf.dbmsName, L"Interbase"))
			return((wxDBMS)(m_dbmsType = dbmsINTERBASE));

		// BJO 20000428 : add support for Virtuoso
		if (!_wcsicmp(m_dbInf.dbmsName, L"OpenLink Virtuoso VDBMS"))
			return((wxDBMS)(m_dbmsType = dbmsVIRTUOSO));

		if (!_wcsicmp(m_dbInf.dbmsName, L"Adaptive Server Anywhere"))
			return((wxDBMS)(m_dbmsType = dbmsSYBASE_ASA));

		// BJO 20000427 : The "SQL Server" string is also returned by SQLServer when
		// connected through an OpenLink driver.
		// Is it also returned by Sybase Adapatitve server?
		// OpenLink driver name is OLOD3032.DLL for msw and oplodbc.so for unix
		if (!_wcsicmp(m_dbInf.dbmsName, L"SQL Server"))
		{
			if (!wcsncmp(m_dbInf.driverName, L"oplodbc", 7) ||
				!wcsncmp(m_dbInf.driverName, L"OLOD", 4))
				return ((wxDBMS)(dbmsMS_SQL_SERVER));
			else
				return ((wxDBMS)(m_dbmsType = dbmsSYBASE_ASE));
		}

		if (!_wcsicmp(m_dbInf.dbmsName, L"Microsoft SQL Server"))
			return((wxDBMS)(m_dbmsType = dbmsMS_SQL_SERVER));

		baseName[10] = 0;
		if (!_wcsicmp(baseName, L"PostgreSQL"))  // v6.5.0
			return((wxDBMS)(m_dbmsType = dbmsPOSTGRES));

		baseName[9] = 0;
		if (!_wcsicmp(baseName, L"Pervasive"))
			return((wxDBMS)(m_dbmsType = dbmsPERVASIVE_SQL));

		baseName[8] = 0;
		if (!_wcsicmp(baseName, L"Informix"))
			return((wxDBMS)(m_dbmsType = dbmsINFORMIX));

		if (!_wcsicmp(baseName, L"Firebird"))
			return((wxDBMS)(m_dbmsType = dbmsFIREBIRD));

		baseName[6] = 0;
		if (!_wcsicmp(baseName, L"Oracle"))
			return((wxDBMS)(m_dbmsType = dbmsORACLE));
		if (!_wcsicmp(baseName, L"ACCESS"))
			return((wxDBMS)(m_dbmsType = dbmsACCESS));
		if (!_wcsicmp(baseName, L"Sybase"))
			return((wxDBMS)(m_dbmsType = dbmsSYBASE_ASE));

		baseName[5] = 0;
		if (!_wcsicmp(baseName, L"DBASE"))
			return((wxDBMS)(m_dbmsType = dbmsDBASE));
		if (!_wcsicmp(baseName, L"xBase"))
			return((wxDBMS)(m_dbmsType = dbmsXBASE_SEQUITER));
		if (!_wcsicmp(baseName, L"MySQL"))
			return((wxDBMS)(m_dbmsType = dbmsMY_SQL));
		if (!_wcsicmp(baseName, L"MaxDB"))
			return((wxDBMS)(m_dbmsType = dbmsMAXDB));

		baseName[3] = 0;
		if (!_wcsicmp(baseName, L"DB2"))
			return((wxDBMS)(m_dbmsType = dbmsDB2));

		return((wxDBMS)(m_dbmsType = dbmsUNIDENTIFIED));

	}  // wxDb::Dbms()


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
	//	switch (Dbms())
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
	//	if ( Dbms() == dbmsMY_SQL )
	//	{
	//		sqlStmt = (boost::wformat(L"ALTER TABLE %s %s %s %s") % tableName % alterSlashModify % columnName % dataTypeName).str();
	//	}
	//	else
	//	{
	//		sqlStmt = (boost::wformat(L"ALTER TABLE \"%s\" \"%s\" \"%s\" %s") % tableName % alterSlashModify % columnName % dataTypeName).str();
	//	}

	//	// For varchars only, append the size of the column
	//	if (dataType == DB_DATA_TYPE_VARCHAR &&
	//		(Dbms() != dbmsMY_SQL || dataTypeName != L"text"))
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
	//	switch (Dbms())
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


#if 0
	/********** wxDbCreateDataSource() **********/
	int wxDbCreateDataSource(const std::wstring &driverName, const std::wstring &dsn, const std::wstring &description,
		bool sysDSN, const std::wstring &defDir, wxWindow *parent)
		/*
		* !!!! ONLY FUNCTIONAL UNDER MSW with VC6 !!!!
		* Very rudimentary creation of an ODBC data source.
		*
		* ODBC driver must be ODBC 3.0 compliant to use this function
		*/
	{
		int result = FALSE;

		//!!!! ONLY FUNCTIONAL UNDER MSW with VC6 !!!!
#ifdef __VISUALC__
		int       dsnLocation;
		std::wstring  setupStr;

		if (sysDSN)
			dsnLocation = ODBC_ADD_SYS_DSN;
		else
			dsnLocation = ODBC_ADD_DSN;

		// NOTE: The decimal 2 is an invalid character in all keyword pairs
		// so that is why I used it, as std::wstring does not deal well with
		// embedded nulls in strings
		setupStr.Printf(L"DSN=%s%cDescription=%s%cDefaultDir=%s%c",dsn,2,description,2,defDir,2);

		// Replace the separator from above with the '\0' separator needed
		// by the SQLConfigDataSource() function
		int k;
		do
		{
			k = setupStr.Find((wchar_t)2,true);
			if (k != wxNOT_FOUND)
				setupStr[(UINT)k] = L'\0';
		}
		while (k != wxNOT_FOUND);

		result = SQLConfigDataSource((HWND)parent->GetHWND(), dsnLocation,
			driverName, setupStr.c_str());

		if ((result != SQL_SUCCESS) && (result != SQL_SUCCESS_WITH_INFO))
		{
			// check for errors caused by ConfigDSN based functions
			DWORD retcode = 0;
			WORD cb;
			wchar_t errMsg[SQL_MAX_MESSAGE_LENGTH];
			errMsg[0] = L'\0';

			// This function is only supported in ODBC drivers v3.0 compliant and above
			SQLInstallerError(1,&retcode,errMsg,SQL_MAX_MESSAGE_LENGTH-1,&cb);
			if (retcode)
			{
#ifdef DBDEBUG_CONSOLE
				// When run in console mode, use standard out to display errors.
				std::wcout << errMsg << std::endl;
				std::wcout << L"Press any key to continue..." << std::endl;
				getchar();
#endif  // DBDEBUG_CONSOLE

#ifdef EXODBCDEBUG
				BOOST_LOG_TRIVIAL(debug) << L"ODBC DEBUG MESSAGE: " << errMsg;
#endif  // EXODBCDEBUG
			}
		}
		else
			result = TRUE;
#else
		// Using iODBC/unixODBC or some other compiler which does not support the APIs
		// necessary to use this function, so this function is not supported
#ifdef EXODBCDEBUG
		BOOST_LOG_TRIVIAL(debug) << L"ODBC DEBUG MESSAGE: " << L"wxDbCreateDataSource() not available except under VC++/MSW";
#endif
		result = FALSE;
#endif  // __VISUALC__

		return result;

	}  // wxDbCreateDataSource()
#endif

}