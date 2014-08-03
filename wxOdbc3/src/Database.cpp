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

	// The wxDb::errorList is copied to this variable when the wxDb object
	// is closed.  This way, the error list is still available after the
	// database object is closed.  This is necessary if the database
	// connection fails so the calling application can show the operator
	// why the connection failed.  Note: as each wxDb object is closed, it
	// will overwrite the errors of the previously destroyed wxDb object in
	// this variable.  NOTE: This occurs during a CLOSE, not a FREEing of the
	// connection
	wchar_t DBerrorList[DB_MAX_ERROR_HISTORY][DB_MAX_ERROR_MSG_LEN+1];


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


	/********** wxDbTableInf Constructor ********/
	DbCatalogTable::DbCatalogTable()
	{
		Initialize();
	}  // wxDbTableInf::wxDbTableInf()


	/********** wxDbTableInf Constructor ********/
	DbCatalogTable::~DbCatalogTable()
	{
		if (m_pColInf)
			delete [] m_pColInf;
		m_pColInf = NULL;
	}  // wxDbTableInf::~wxDbTableInf()


	bool DbCatalogTable::Initialize()
	{
		m_tableName[0]    = 0;
		m_tableType[0]    = 0;
		m_tableRemarks[0] = 0;
		m_numCols         = 0;
		m_pColInf         = NULL;

		return true;
	}  // wxDbTableInf::Initialize()


	/********** wxDbInf Constructor *************/
	DbCatalog::DbCatalog()
	{
		Initialize();
	}  // wxDbInf::wxDbInf()


	/********** wxDbInf Destructor *************/
	DbCatalog::~DbCatalog()
	{
		//if (m_pTableInf)
		//	delete [] m_pTableInf;
		//m_pTableInf = NULL;
	}  // wxDbInf::~wxDbInf()


	/********** wxDbInf::Initialize() *************/
	bool DbCatalog::Initialize()
	{
		m_catalog[0]      = 0;
		m_schema[0]       = 0;
//		m_numTables       = 0;
//		m_pTableInf       = NULL;

		return true;
	}  // wxDbInf::Initialize()


	/********** wxDb Constructor **********/
	Database::Database(const HENV &aHenv, bool FwdOnlyCursors)
	{
		// Copy the HENV into the db class
		m_henv = aHenv;
		m_fwdOnlyCursors = FwdOnlyCursors;

		Initialize();
	} // wxDb::wxDb()


	/********** wxDb Destructor **********/
	Database::~Database()
	{
		exASSERT_MSG(!IsCached(), "Cached connections must not be manually deleted, use\nwxDbFreeConnection() or wxDbCloseConnections().");

		if (IsOpen())
		{
			Close();
		}
	}  // wxDb destructor



	/********** PRIVATE! wxDb::initialize PRIVATE! **********/
	/********** wxDb::initialize() **********/
	void Database::Initialize()
		/*
		* Private member function that sets all wxDb member variables to
		* known values at creation of the wxDb
		*/
	{
		int i;

		m_fpSqlLog      = 0;            // Sql Log file pointer
		m_sqlLogState   = sqlLogOFF;    // By default, logging is turned off
		nTables       = 0;
		m_dbmsType      = dbmsUNIDENTIFIED;

		wcscpy(sqlState,emptyString);
		wcscpy(errorMsg,emptyString);
		nativeError = cbErrorMsg = 0;
		for (i = 0; i < DB_MAX_ERROR_HISTORY; i++)
			wcscpy(errorList[i], emptyString);

		// Init typeInf structures
		m_typeInfVarchar.TypeName.empty();
		m_typeInfVarchar.FsqlType      = 0;
		m_typeInfVarchar.Precision     = 0;
		m_typeInfVarchar.CaseSensitive = 0;
		m_typeInfVarchar.MaximumScale  = 0;

		m_typeInfInteger.TypeName.empty();
		m_typeInfInteger.FsqlType      = 0;
		m_typeInfInteger.Precision     = 0;
		m_typeInfInteger.CaseSensitive = 0;
		m_typeInfInteger.MaximumScale  = 0;

		m_typeInfFloat.TypeName.empty();
		m_typeInfFloat.FsqlType      = 0;
		m_typeInfFloat.Precision     = 0;
		m_typeInfFloat.CaseSensitive = 0;
		m_typeInfFloat.MaximumScale  = 0;

		m_typeInfDate.TypeName.empty();
		m_typeInfDate.FsqlType      = 0;
		m_typeInfDate.Precision     = 0;
		m_typeInfDate.CaseSensitive = 0;
		m_typeInfDate.MaximumScale  = 0;

		m_typeInfBlob.TypeName.empty();
		m_typeInfBlob.FsqlType      = 0;
		m_typeInfBlob.Precision     = 0;
		m_typeInfBlob.CaseSensitive = 0;
		m_typeInfBlob.MaximumScale  = 0;

		m_typeInfMemo.TypeName.empty();
		m_typeInfMemo.FsqlType      = 0;
		m_typeInfMemo.Precision     = 0;
		m_typeInfMemo.CaseSensitive = 0;
		m_typeInfMemo.MaximumScale  = 0;

		// Error reporting is turned OFF by default
		m_silent = true;

		// Allocate a data source connection handle
		if (SQLAllocConnect(m_henv, &m_hdbc) != SQL_SUCCESS)
			DispAllErrors(m_henv);

		// Initialize the db status flag
		DB_STATUS = 0;

		// Mark database as not OpenImpl as of yet
		m_dbIsOpen = false;
		m_dbIsCached = false;
		m_dbOpenedWithConnectionString = false;
	}  // wxDb::initialize()


	/********** PRIVATE! wxDb::ConvertUserIDImpl PRIVATE! **********/
	//
	// NOTE: Return value from this function MUST be copied
	//       immediately, as the value is not good after
	//       this function has left scope.
	//
	std::wstring Database::ConvertUserIDImpl(const wchar_t* userID)
	{
		std::wstring UserID;

		if (userID)
		{
			if (!wcslen(userID))
				UserID = m_uid;
			else
				UserID = userID;
		}
		else
			UserID.empty();

		// dBase does not use user names, and some drivers fail if you try to pass one
		if ( Dbms() == dbmsDBASE
			|| Dbms() == dbmsXBASE_SEQUITER )
			UserID.empty();

		// Some databases require user names to be specified in uppercase,
		// so force the name to uppercase
		if ((Dbms() == dbmsORACLE) ||
			(Dbms() == dbmsMAXDB))
			boost::algorithm::to_upper(UserID);

		return UserID;
	}  // wxDb::ConvertUserIDImpl()


	bool Database::DetermineDataTypesImpl(bool failOnDataTypeUnsupported)
	{
		size_t iIndex;

		// These are the possible SQL types we check for use against the datasource we are connected
		// to for the purpose of determining which data type to use for the basic character strings
		// column types
		//
		// NOTE: The first type in this enumeration that is determined to be supported by the
		//       datasource/driver is the one that will be used.
		SWORD PossibleSqlCharTypes[] = {
#if defined(SQL_WVARCHAR)
			SQL_WVARCHAR,
#endif
			SQL_VARCHAR,
#if defined(SQL_WVARCHAR)
			SQL_WCHAR,
#endif
			SQL_CHAR
		};

		// These are the possible SQL types we check for use against the datasource we are connected
		// to for the purpose of determining which data type to use for the basic non-floating point
		// column types
		//
		// NOTE: The first type in this enumeration that is determined to be supported by the
		//       datasource/driver is the one that will be used.
		SWORD PossibleSqlIntegerTypes[] = {
			SQL_INTEGER
		};

		// These are the possible SQL types we check for use against the datasource we are connected
		// to for the purpose of determining which data type to use for the basic floating point number
		// column types
		//
		// NOTE: The first type in this enumeration that is determined to be supported by the
		//       datasource/driver is the one that will be used.
		SWORD PossibleSqlFloatTypes[] = {
			SQL_DOUBLE,
			SQL_REAL,
			SQL_FLOAT,
			SQL_DECIMAL,
			SQL_NUMERIC
		};

		// These are the possible SQL types we check for use agains the datasource we are connected
		// to for the purpose of determining which data type to use for the date/time column types
		//
		// NOTE: The first type in this enumeration that is determined to be supported by the
		//       datasource/driver is the one that will be used.
		SWORD PossibleSqlDateTypes[] = {
			SQL_TIMESTAMP,
			SQL_DATE,
#ifdef SQL_DATETIME
			SQL_DATETIME
#endif
		};

		// These are the possible SQL types we check for use agains the datasource we are connected
		// to for the purpose of determining which data type to use for the BLOB column types.
		//
		// NOTE: The first type in this enumeration that is determined to be supported by the
		//       datasource/driver is the one that will be used.
		SWORD PossibleSqlBlobTypes[] = {
			SQL_LONGVARBINARY,
			SQL_VARBINARY
		};

		// These are the possible SQL types we check for use agains the datasource we are connected
		// to for the purpose of determining which data type to use for the MEMO column types
		// (a type which allow to store large strings; like VARCHAR just with a bigger precision)
		//
		// NOTE: The first type in this enumeration that is determined to be supported by the
		//       datasource/driver is the one that will be used.
		SWORD PossibleSqlMemoTypes[] = {
			SQL_LONGVARCHAR,
		};


		// Query the data source regarding data type information

		//
		// The way it was determined which SQL data types to use was by calling SQLGetInfo
		// for all of the possible SQL data types to see which ones were supported.  If
		// a type is not supported, the SQLFetch() that's called from GetDataTypeInfoImpl()
		// fails with SQL_NO_DATA_FOUND.  This is ugly because I'm sure the three SQL data
		// types I've selected below will not always be what we want.  These are just
		// what happened to work against an Oracle 7/Intersolv combination.  The following is
		// a complete list of the results I got back against the Oracle 7 database:
		//
		// SQL_BIGINT             SQL_NO_DATA_FOUND
		// SQL_BINARY             SQL_NO_DATA_FOUND
		// SQL_BIT                SQL_NO_DATA_FOUND
		// SQL_CHAR               type name = 'CHAR', Precision = 255
		// SQL_DATE               SQL_NO_DATA_FOUND
		// SQL_DECIMAL            type name = 'NUMBER', Precision = 38
		// SQL_DOUBLE             type name = 'NUMBER', Precision = 15
		// SQL_FLOAT              SQL_NO_DATA_FOUND
		// SQL_INTEGER            SQL_NO_DATA_FOUND
		// SQL_LONGVARBINARY      type name = 'LONG RAW', Precision = 2 billion
		// SQL_LONGVARCHAR        type name = 'LONG', Precision = 2 billion
		// SQL_NUMERIC            SQL_NO_DATA_FOUND
		// SQL_REAL               SQL_NO_DATA_FOUND
		// SQL_SMALLINT           SQL_NO_DATA_FOUND
		// SQL_TIME               SQL_NO_DATA_FOUND
		// SQL_TIMESTAMP          type name = 'DATE', Precision = 19
		// SQL_VARBINARY          type name = 'RAW', Precision = 255
		// SQL_VARCHAR            type name = 'VARCHAR2', Precision = 2000
		// =====================================================================
		// Results from a Microsoft Access 7.0 db, using a driver from Microsoft
		//
		// SQL_VARCHAR            type name = 'TEXT', Precision = 255
		// SQL_TIMESTAMP          type name = 'DATETIME'
		// SQL_DECIMAL            SQL_NO_DATA_FOUND
		// SQL_NUMERIC            type name = 'CURRENCY', Precision = 19
		// SQL_FLOAT              SQL_NO_DATA_FOUND
		// SQL_REAL               type name = 'SINGLE', Precision = 7
		// SQL_DOUBLE             type name = 'DOUBLE', Precision = 15
		// SQL_INTEGER            type name = 'LONG', Precision = 10

		// Query the data source for info about itself
		if (!GetDbInfoImpl(failOnDataTypeUnsupported))
			return false;

		// --------------- Varchar - (Variable length character string) ---------------
		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlCharTypes) &&
			!GetDataTypeInfoImpl(PossibleSqlCharTypes[iIndex], m_typeInfVarchar); ++iIndex)
		{}

		if (iIndex < EXSIZEOF(PossibleSqlCharTypes))
			m_typeInfVarchar.FsqlType = PossibleSqlCharTypes[iIndex];
		else if (failOnDataTypeUnsupported)
			return false;

		// --------------- Float ---------------
		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlFloatTypes) &&
			!GetDataTypeInfoImpl(PossibleSqlFloatTypes[iIndex], m_typeInfFloat); ++iIndex)
		{}

		if (iIndex < EXSIZEOF(PossibleSqlFloatTypes))
			m_typeInfFloat.FsqlType = PossibleSqlFloatTypes[iIndex];
		else if (failOnDataTypeUnsupported)
			return false;

		// --------------- Integer -------------
		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlIntegerTypes) &&
			!GetDataTypeInfoImpl(PossibleSqlIntegerTypes[iIndex], m_typeInfInteger); ++iIndex)
		{}

		if (iIndex < EXSIZEOF(PossibleSqlIntegerTypes))
			m_typeInfInteger.FsqlType = PossibleSqlIntegerTypes[iIndex];
		else if (failOnDataTypeUnsupported)
		{
			// If no non-floating point data types are supported, we'll
			// use the type assigned for floats to store integers as well
			if (!GetDataTypeInfoImpl(m_typeInfFloat.FsqlType, m_typeInfInteger))
			{
				if (failOnDataTypeUnsupported)
					return false;
			}
			else
				m_typeInfInteger.FsqlType = m_typeInfFloat.FsqlType;
		}

		// --------------- Date/Time ---------------
		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlDateTypes) &&
			!GetDataTypeInfoImpl(PossibleSqlDateTypes[iIndex], m_typeInfDate); ++iIndex)
		{}

		if (iIndex < EXSIZEOF(PossibleSqlDateTypes))
			m_typeInfDate.FsqlType = PossibleSqlDateTypes[iIndex];
		else if (failOnDataTypeUnsupported)
			return false;

		// --------------- BLOB ---------------
		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlBlobTypes) &&
			!GetDataTypeInfoImpl(PossibleSqlBlobTypes[iIndex], m_typeInfBlob); ++iIndex)
		{}

		if (iIndex < EXSIZEOF(PossibleSqlBlobTypes))
			m_typeInfBlob.FsqlType = PossibleSqlBlobTypes[iIndex];
		else if (failOnDataTypeUnsupported)
			return false;

		// --------------- MEMO ---------------
		for (iIndex = 0; iIndex < EXSIZEOF(PossibleSqlMemoTypes) &&
			!GetDataTypeInfoImpl(PossibleSqlMemoTypes[iIndex], m_typeInfMemo); ++iIndex)
		{}

		if (iIndex < EXSIZEOF(PossibleSqlMemoTypes))
			m_typeInfMemo.FsqlType = PossibleSqlMemoTypes[iIndex];
		else if (failOnDataTypeUnsupported)
			return false;

		return true;
	}  // wxDb::DetermineDataTypesImpl


	bool Database::OpenImpl(bool failOnDataTypeUnsupported)
	{
		/*
		If using Intersolv branded ODBC drivers, this is the place where you would substitute
		your branded driver license information

		SQLSetConnectOption(hdbc, 1041, (UDWORD) emptyString);
		SQLSetConnectOption(hdbc, 1042, (UDWORD) emptyString);
		*/

		// Mark database as OpenImpl
		m_dbIsOpen = true;

		// Allocate a statement handle for the database connection
		if (SQLAllocStmt(m_hdbc, &m_hstmt) != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc));

		// Set Connection Options
		if (!SetConnectionOptionsImpl())
			return false;

		if (!DetermineDataTypesImpl(failOnDataTypeUnsupported))
			return false;

#ifdef DBDEBUG_CONSOLE
		std::wcout << L"VARCHAR DATA TYPE: " << m_typeInfVarchar.TypeName << std::endl;
		std::wcout << L"INTEGER DATA TYPE: " << m_typeInfInteger.TypeName << std::endl;
		std::wcout << L"FLOAT   DATA TYPE: " << m_typeInfFloat.TypeName << std::endl;
		std::wcout << L"DATE    DATA TYPE: " << m_typeInfDate.TypeName << std::endl;
		std::wcout << L"BLOB    DATA TYPE: " << m_typeInfBlob.TypeName << std::endl;
		std::wcout << L"MEMO    DATA TYPE: " << m_typeInfMemo.TypeName << std::endl;
		std::wcout << std::endl;
#endif

		// Completed Successfully
		return true;
	}

	bool Database::Open(const std::wstring& inConnectStr, bool failOnDataTypeUnsupported)
	{
		exASSERT(inConnectStr.length());
		return Open(inConnectStr, NULL, failOnDataTypeUnsupported);
	}

	bool Database::Open(const std::wstring& inConnectStr, SQLHWND parentWnd, bool failOnDataTypeUnsupported)
	{
		m_dsn        = emptyString;
		m_uid        = emptyString;
		m_authStr    = emptyString;

		RETCODE retcode;

		if (!FwdOnlyCursors())
		{
			// Specify that the ODBC cursor library be used, if needed.  This must be
			// specified before the connection is made.
			retcode = SQLSetConnectOption(m_hdbc, SQL_ODBC_CURSORS, SQL_CUR_USE_IF_NEEDED);

#ifdef DBDEBUG_CONSOLE
			if (retcode == SQL_SUCCESS)
				std::wcout << L"SQLSetConnectOption(CURSOR_LIB) successful" << std::endl;
			else
				std::wcout << L"SQLSetConnectOption(CURSOR_LIB) failed" << std::endl;
#else
			//wxUnusedVar(retcode);
#endif
		}

		// Connect to the data source
		SQLTCHAR outConnectBuffer[SQL_MAX_CONNECTSTR_LEN+1];  // MS recommends at least 1k buffer
		short outConnectBufferLen;

		m_inConnectionStr = inConnectStr;

		retcode = SQLDriverConnect(m_hdbc, parentWnd, (SQLTCHAR FAR *)m_inConnectionStr.c_str(),
			(SWORD)m_inConnectionStr.length(), (SQLTCHAR FAR *)outConnectBuffer,
			EXSIZEOF(outConnectBuffer), &outConnectBufferLen, SQL_DRIVER_COMPLETE );

		if ((retcode != SQL_SUCCESS) &&
			(retcode != SQL_SUCCESS_WITH_INFO))
			return(DispAllErrors(m_henv, m_hdbc));

		outConnectBuffer[outConnectBufferLen] = 0;
		m_outConnectionStr = outConnectBuffer;
		m_dbOpenedWithConnectionString = true;

		return OpenImpl(failOnDataTypeUnsupported);
	}

	/********** wxDb::Open() **********/
	bool Database::Open(const std::wstring &Dsn, const std::wstring &Uid, const std::wstring &AuthStr, bool failOnDataTypeUnsupported)
	{
		exASSERT(!Dsn.empty());
		m_dsn        = Dsn;
		m_uid        = Uid;
		m_authStr    = AuthStr;

		m_inConnectionStr = emptyString;
		m_outConnectionStr = emptyString;

		RETCODE retcode;

		if (!FwdOnlyCursors())
		{
			// Specify that the ODBC cursor library be used, if needed.  This must be
			// specified before the connection is made.
			retcode = SQLSetConnectOption(m_hdbc, SQL_ODBC_CURSORS, SQL_CUR_USE_IF_NEEDED);

#ifdef DBDEBUG_CONSOLE
			if (retcode == SQL_SUCCESS)
				std::wcout << L"SQLSetConnectOption(CURSOR_LIB) successful" << std::endl;
			else
				std::wcout << L"SQLSetConnectOption(CURSOR_LIB) failed" << std::endl;
#else
			//wxUnusedVar( retcode );
#endif
		}

		// Connect to the data source
		retcode = SQLConnect(m_hdbc, (SQLTCHAR FAR *) m_dsn.c_str(), SQL_NTS,
			(SQLTCHAR FAR *) m_uid.c_str(), SQL_NTS,
			(SQLTCHAR FAR *) m_authStr.c_str(), SQL_NTS);

		if ((retcode != SQL_SUCCESS) &&
			(retcode != SQL_SUCCESS_WITH_INFO))
			return(DispAllErrors(m_henv, m_hdbc));

		return OpenImpl(failOnDataTypeUnsupported);

	} // wxDb::Open()


	bool Database::Open(DbEnvironment *dbConnectInf, bool failOnDataTypeUnsupported)
	{
		exASSERT(dbConnectInf);

		// Use the connection string if one is present
		if (dbConnectInf->UseConnectionStr())
			return Open(dbConnectInf->GetConnectionStr(), failOnDataTypeUnsupported);
		else
			return Open(dbConnectInf->GetDsn(), dbConnectInf->GetUserID(),
			dbConnectInf->GetPassword(), failOnDataTypeUnsupported);
	}  // wxDb::Open()


	bool Database::Open(Database *copyDb)
	{
		m_dsn              = copyDb->GetDatasourceName();
		m_uid              = copyDb->GetUsername();
		m_authStr          = copyDb->GetPassword();
		m_inConnectionStr  = copyDb->GetConnectionInStr();
		m_outConnectionStr = copyDb->GetConnectionOutStr();

		RETCODE retcode;

		if (!FwdOnlyCursors())
		{
			// Specify that the ODBC cursor library be used, if needed.  This must be
			// specified before the connection is made.
			retcode = SQLSetConnectOption(m_hdbc, SQL_ODBC_CURSORS, SQL_CUR_USE_IF_NEEDED);

#ifdef DBDEBUG_CONSOLE
			if (retcode == SQL_SUCCESS)
				std::wcout << L"SQLSetConnectOption(CURSOR_LIB) successful" << std::endl;
			else
				std::wcout << L"SQLSetConnectOption(CURSOR_LIB) failed" << std::endl;
#else
			//wxUnusedVar( retcode );
#endif
		}

		if (copyDb->OpenedWithConnectionString())
		{
			// Connect to the data source
			SQLTCHAR outConnectBuffer[SQL_MAX_CONNECTSTR_LEN+1];
			short outConnectBufferLen;

			m_inConnectionStr = copyDb->GetConnectionInStr();

			retcode = SQLDriverConnect(m_hdbc, NULL, (SQLTCHAR FAR *)m_inConnectionStr.c_str(),
				(SWORD)m_inConnectionStr.length(), (SQLTCHAR FAR *)outConnectBuffer,
				EXSIZEOF(outConnectBuffer), &outConnectBufferLen, SQL_DRIVER_COMPLETE);

			if ((retcode != SQL_SUCCESS) &&
				(retcode != SQL_SUCCESS_WITH_INFO))
				return(DispAllErrors(m_henv, m_hdbc));

			outConnectBuffer[outConnectBufferLen] = 0;
			m_outConnectionStr = outConnectBuffer;
			m_dbOpenedWithConnectionString = true;
		}
		else
		{
			// Connect to the data source
			retcode = SQLConnect(m_hdbc, (SQLTCHAR FAR *) m_dsn.c_str(), SQL_NTS,
				(SQLTCHAR FAR *) m_uid.c_str(), SQL_NTS,
				(SQLTCHAR FAR *) m_authStr.c_str(), SQL_NTS);
		}

		if ((retcode != SQL_SUCCESS) &&
			(retcode != SQL_SUCCESS_WITH_INFO))
			return(DispAllErrors(m_henv, m_hdbc));

		/*
		If using Intersolv branded ODBC drivers, this is the place where you would substitute
		your branded driver license information

		SQLSetConnectOption(hdbc, 1041, (UDWORD) emptyString);
		SQLSetConnectOption(hdbc, 1042, (UDWORD) emptyString);
		*/

		// Mark database as OpenImpl
		m_dbIsOpen = true;

		// Allocate a statement handle for the database connection
		if (SQLAllocStmt(m_hdbc, &m_hstmt) != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc));

		// Set Connection Options
		if (!SetConnectionOptionsImpl())
			return false;

		// Instead of Querying the data source for info about itself, it can just be copied
		// from the wxDb instance that was passed in (copyDb).
		wcscpy(dbInf.serverName,copyDb->dbInf.serverName);
		wcscpy(dbInf.databaseName,copyDb->dbInf.databaseName);
		wcscpy(dbInf.dbmsName,copyDb->dbInf.dbmsName);
		wcscpy(dbInf.dbmsVer,copyDb->dbInf.dbmsVer);
		dbInf.maxConnections = copyDb->dbInf.maxConnections;
		dbInf.maxStmts = copyDb->dbInf.maxStmts;
		wcscpy(dbInf.driverName,copyDb->dbInf.driverName);
		wcscpy(dbInf.odbcVer,copyDb->dbInf.odbcVer);
		wcscpy(dbInf.drvMgrOdbcVer,copyDb->dbInf.drvMgrOdbcVer);
		wcscpy(dbInf.driverVer,copyDb->dbInf.driverVer);
		dbInf.apiConfLvl = copyDb->dbInf.apiConfLvl;
		dbInf.cliConfLvl = copyDb->dbInf.cliConfLvl;
		dbInf.sqlConfLvl = copyDb->dbInf.sqlConfLvl;
		wcscpy(dbInf.outerJoins,copyDb->dbInf.outerJoins);
		wcscpy(dbInf.procedureSupport,copyDb->dbInf.procedureSupport);
		wcscpy(dbInf.accessibleTables,copyDb->dbInf.accessibleTables);
		dbInf.cursorCommitBehavior = copyDb->dbInf.cursorCommitBehavior;
		dbInf.cursorRollbackBehavior = copyDb->dbInf.cursorRollbackBehavior;
		dbInf.supportNotNullClause = copyDb->dbInf.supportNotNullClause;
		wcscpy(dbInf.supportIEF,copyDb->dbInf.supportIEF);
		dbInf.txnIsolation = copyDb->dbInf.txnIsolation;
		dbInf.txnIsolationOptions = copyDb->dbInf.txnIsolationOptions;
		dbInf.fetchDirections = copyDb->dbInf.fetchDirections;
		dbInf.lockTypes = copyDb->dbInf.lockTypes;
		dbInf.posOperations = copyDb->dbInf.posOperations;
		dbInf.posStmts = copyDb->dbInf.posStmts;
		dbInf.scrollConcurrency = copyDb->dbInf.scrollConcurrency;
		dbInf.scrollOptions = copyDb->dbInf.scrollOptions;
		dbInf.staticSensitivity = copyDb->dbInf.staticSensitivity;
		dbInf.txnCapable = copyDb->dbInf.txnCapable;
		dbInf.loginTimeout = copyDb->dbInf.loginTimeout;

		// VARCHAR = Variable length character string
		m_typeInfVarchar.FsqlType         = copyDb->m_typeInfVarchar.FsqlType;
		m_typeInfVarchar.TypeName         = copyDb->m_typeInfVarchar.TypeName;
		m_typeInfVarchar.Precision        = copyDb->m_typeInfVarchar.Precision;
		m_typeInfVarchar.CaseSensitive    = copyDb->m_typeInfVarchar.CaseSensitive;
		m_typeInfVarchar.MaximumScale     = copyDb->m_typeInfVarchar.MaximumScale;

		// Float
		m_typeInfFloat.FsqlType         = copyDb->m_typeInfFloat.FsqlType;
		m_typeInfFloat.TypeName         = copyDb->m_typeInfFloat.TypeName;
		m_typeInfFloat.Precision        = copyDb->m_typeInfFloat.Precision;
		m_typeInfFloat.CaseSensitive    = copyDb->m_typeInfFloat.CaseSensitive;
		m_typeInfFloat.MaximumScale     = copyDb->m_typeInfFloat.MaximumScale;

		// Integer
		m_typeInfInteger.FsqlType         = copyDb->m_typeInfInteger.FsqlType;
		m_typeInfInteger.TypeName         = copyDb->m_typeInfInteger.TypeName;
		m_typeInfInteger.Precision        = copyDb->m_typeInfInteger.Precision;
		m_typeInfInteger.CaseSensitive    = copyDb->m_typeInfInteger.CaseSensitive;
		m_typeInfInteger.MaximumScale     = copyDb->m_typeInfInteger.MaximumScale;

		// Date/Time
		m_typeInfDate.FsqlType         = copyDb->m_typeInfDate.FsqlType;
		m_typeInfDate.TypeName         = copyDb->m_typeInfDate.TypeName;
		m_typeInfDate.Precision        = copyDb->m_typeInfDate.Precision;
		m_typeInfDate.CaseSensitive    = copyDb->m_typeInfDate.CaseSensitive;
		m_typeInfDate.MaximumScale     = copyDb->m_typeInfDate.MaximumScale;

		// Blob
		m_typeInfBlob.FsqlType         = copyDb->m_typeInfBlob.FsqlType;
		m_typeInfBlob.TypeName         = copyDb->m_typeInfBlob.TypeName;
		m_typeInfBlob.Precision        = copyDb->m_typeInfBlob.Precision;
		m_typeInfBlob.CaseSensitive    = copyDb->m_typeInfBlob.CaseSensitive;
		m_typeInfBlob.MaximumScale     = copyDb->m_typeInfBlob.MaximumScale;

		// Memo
		m_typeInfMemo.FsqlType         = copyDb->m_typeInfMemo.FsqlType;
		m_typeInfMemo.TypeName         = copyDb->m_typeInfMemo.TypeName;
		m_typeInfMemo.Precision        = copyDb->m_typeInfMemo.Precision;
		m_typeInfMemo.CaseSensitive    = copyDb->m_typeInfMemo.CaseSensitive;
		m_typeInfMemo.MaximumScale     = copyDb->m_typeInfMemo.MaximumScale;

#ifdef DBDEBUG_CONSOLE
		std::wcout << L"VARCHAR DATA TYPE: " << m_typeInfVarchar.TypeName << std::endl;
		std::wcout << L"INTEGER DATA TYPE: " << m_typeInfInteger.TypeName << std::endl;
		std::wcout << L"FLOAT   DATA TYPE: " << m_typeInfFloat.TypeName << std::endl;
		std::wcout << L"DATE    DATA TYPE: " << m_typeInfDate.TypeName << std::endl;
		std::wcout << L"BLOB    DATA TYPE: " << m_typeInfBlob.TypeName << std::endl;
		std::wcout << L"MEMO    DATA TYPE: " << m_typeInfMemo.TypeName << std::endl;
		std::wcout << std::endl;
#endif

		// Completed Successfully
		return true;
	} // wxDb::Open() 2


	/********** wxDb::SetConnectionOptionsImpl() **********/
	bool Database::SetConnectionOptionsImpl()
		/*
		* NOTE: The Intersolv/Oracle 7 driver was "Not Capable" of setting the login timeout.
		*/
	{
		SWORD cb;

		// I need to get the DBMS name here, because some of the connection options
		// are database specific and need to call the Dbms() function.
		RETCODE retcode;

		retcode = SQLGetInfo(m_hdbc, SQL_DBMS_NAME, (UCHAR *) dbInf.dbmsName, sizeof(dbInf.dbmsName), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
			return(DispAllErrors(m_henv, m_hdbc));

		/* retcode = */ SQLSetConnectOption(m_hdbc, SQL_AUTOCOMMIT, SQL_AUTOCOMMIT_OFF);
		/* retcode = */ SQLSetConnectOption(m_hdbc, SQL_OPT_TRACE, SQL_OPT_TRACE_OFF);
		//  SQLSetConnectOption(hdbc, SQL_TXN_ISOLATION, SQL_TXN_READ_COMMITTED);  // No dirty reads

		// By default, MS Sql Server closes cursors on commit and rollback.  The following
		// call to SQLSetConnectOption() is needed to force SQL Server to preserve cursors
		// after a transaction.  This is a driver specific option and is not part of the
		// ODBC standard.  Note: this behavior is specific to the ODBC interface to SQL Server.
		// The database settings don't have any effect one way or the other.
		if (Dbms() == dbmsMS_SQL_SERVER)
		{
			const long SQL_PRESERVE_CURSORS = 1204L;
			const long SQL_PC_ON = 1L;
			/* retcode = */ SQLSetConnectOption(m_hdbc, SQL_PRESERVE_CURSORS, SQL_PC_ON);
		}

		// Display the connection options to verify them
#ifdef DBDEBUG_CONSOLE
		long l;
		std::wcout << L"****** CONNECTION OPTIONS ******" << std::endl;

		retcode = SQLGetConnectOption(m_hdbc, SQL_AUTOCOMMIT, &l);
		if (retcode != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc));
		std::wcout << L"AUTOCOMMIT: " << (l == SQL_AUTOCOMMIT_OFF ? L"OFF" : L"ON") << std::endl;

		retcode = SQLGetConnectOption(m_hdbc, SQL_ODBC_CURSORS, &l);
		if (retcode != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc));
		std::wcout << L"ODBC CURSORS: ";
		switch(l)
		{
		case(SQL_CUR_USE_IF_NEEDED):
			std::wcout << L"SQL_CUR_USE_IF_NEEDED";
			break;
		case(SQL_CUR_USE_ODBC):
			std::wcout << L"SQL_CUR_USE_ODBC";
			break;
		case(SQL_CUR_USE_DRIVER):
			std::wcout << L"SQL_CUR_USE_DRIVER";
			break;
		}
		std::wcout << std::endl;

		retcode = SQLGetConnectOption(m_hdbc, SQL_OPT_TRACE, &l);
		if (retcode != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc));
		std::wcout << L"TRACING: " << (l == SQL_OPT_TRACE_OFF ? L"OFF" : L"ON") << std::endl;

		std::wcout << std::endl;
#endif

		// Completed Successfully
		return true;

	} // wxDb::SetConnectionOptionsImpl()


	/********** wxDb::getDbInfo() **********/
	bool Database::GetDbInfoImpl(bool failOnDataTypeUnsupported)
	{
		SWORD cb;
		RETCODE retcode;

		retcode = SQLGetInfo(m_hdbc, SQL_SERVER_NAME, (UCHAR*) dbInf.serverName, sizeof(dbInf.serverName), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_DATABASE_NAME, (UCHAR*) dbInf.databaseName, sizeof(dbInf.databaseName), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_DBMS_NAME, (UCHAR*) dbInf.dbmsName, sizeof(dbInf.dbmsName), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		// 16-Mar-1999
		// After upgrading to MSVC6, the original 20 char buffer below was insufficient,
		// causing database connectivity to fail in some cases.
		retcode = SQLGetInfo(m_hdbc, SQL_DBMS_VER, (UCHAR*) dbInf.dbmsVer, sizeof(dbInf.dbmsVer), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_ACTIVE_CONNECTIONS, (UCHAR*) &dbInf.maxConnections, sizeof(dbInf.maxConnections), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_ACTIVE_STATEMENTS, (UCHAR*) &dbInf.maxStmts, sizeof(dbInf.maxStmts), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_DRIVER_NAME, (UCHAR*) dbInf.driverName, sizeof(dbInf.driverName), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_DRIVER_ODBC_VER, (UCHAR*) dbInf.odbcVer, sizeof(dbInf.odbcVer), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_ODBC_VER, (UCHAR*) dbInf.drvMgrOdbcVer, sizeof(dbInf.drvMgrOdbcVer), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_DRIVER_VER, (UCHAR*) dbInf.driverVer, sizeof(dbInf.driverVer), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_ODBC_API_CONFORMANCE, (UCHAR*) &dbInf.apiConfLvl, sizeof(dbInf.apiConfLvl), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_ODBC_SAG_CLI_CONFORMANCE, (UCHAR*) &dbInf.cliConfLvl, sizeof(dbInf.cliConfLvl), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			// Not all drivers support this call - Nick Gorham(unixODBC)
			dbInf.cliConfLvl = 0;
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_ODBC_SQL_CONFORMANCE, (UCHAR*) &dbInf.sqlConfLvl, sizeof(dbInf.sqlConfLvl), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_OUTER_JOINS, (UCHAR*) dbInf.outerJoins, sizeof(dbInf.outerJoins), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_PROCEDURES, (UCHAR*) dbInf.procedureSupport, sizeof(dbInf.procedureSupport), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_ACCESSIBLE_TABLES, (UCHAR*) dbInf.accessibleTables, sizeof(dbInf.accessibleTables), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_CURSOR_COMMIT_BEHAVIOR, (UCHAR*) &dbInf.cursorCommitBehavior, sizeof(dbInf.cursorCommitBehavior), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_CURSOR_ROLLBACK_BEHAVIOR, (UCHAR*) &dbInf.cursorRollbackBehavior, sizeof(dbInf.cursorRollbackBehavior), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_NON_NULLABLE_COLUMNS, (UCHAR*) &dbInf.supportNotNullClause, sizeof(dbInf.supportNotNullClause), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_ODBC_SQL_OPT_IEF, (UCHAR*) dbInf.supportIEF, sizeof(dbInf.supportIEF), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_DEFAULT_TXN_ISOLATION, (UCHAR*) &dbInf.txnIsolation, sizeof(dbInf.txnIsolation), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_TXN_ISOLATION_OPTION, (UCHAR*) &dbInf.txnIsolationOptions, sizeof(dbInf.txnIsolationOptions), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_FETCH_DIRECTION, (UCHAR*) &dbInf.fetchDirections, sizeof(dbInf.fetchDirections), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_LOCK_TYPES, (UCHAR*) &dbInf.lockTypes, sizeof(dbInf.lockTypes), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_POS_OPERATIONS, (UCHAR*) &dbInf.posOperations, sizeof(dbInf.posOperations), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_POSITIONED_STATEMENTS, (UCHAR*) &dbInf.posStmts, sizeof(dbInf.posStmts), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_SCROLL_CONCURRENCY, (UCHAR*) &dbInf.scrollConcurrency, sizeof(dbInf.scrollConcurrency), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_SCROLL_OPTIONS, (UCHAR*) &dbInf.scrollOptions, sizeof(dbInf.scrollOptions), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_STATIC_SENSITIVITY, (UCHAR*) &dbInf.staticSensitivity, sizeof(dbInf.staticSensitivity), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_TXN_CAPABLE, (UCHAR*) &dbInf.txnCapable, sizeof(dbInf.txnCapable), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

		retcode = SQLGetInfo(m_hdbc, SQL_LOGIN_TIMEOUT, (UCHAR*) &dbInf.loginTimeout, sizeof(dbInf.loginTimeout), &cb);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO )
		{
			DispAllErrors(m_henv, m_hdbc);
			if (failOnDataTypeUnsupported)
				return false;
		}

#ifdef DBDEBUG_CONSOLE
		std::wcout << L"***** DATA SOURCE INFORMATION *****" << std::endl;
		std::wcout << L"SERVER Name: " << dbInf.serverName << std::endl;
		std::wcout << L"DBMS Name: " << dbInf.dbmsName << L"; DBMS Version: " << dbInf.dbmsVer << std::endl;
		std::wcout << L"ODBC Version: " << dbInf.odbcVer << L"; Driver Version: " << dbInf.driverVer << std::endl;

		std::wcout << L"API Conf. Level: ";
		switch(dbInf.apiConfLvl)
		{
		case SQL_OAC_NONE:      std::wcout << L"None";       break;
		case SQL_OAC_LEVEL1:    std::wcout << L"Level 1";    break;
		case SQL_OAC_LEVEL2:    std::wcout << L"Level 2";    break;
		}
		std::wcout << std::endl;

		std::wcout << L"SAG CLI Conf. Level: ";
		switch(dbInf.cliConfLvl)
		{
		case SQL_OSCC_NOT_COMPLIANT:    std::wcout << L"Not Compliant";    break;
		case SQL_OSCC_COMPLIANT:        std::wcout << L"Compliant";        break;
		}
		std::wcout << std::endl;

		std::wcout << L"SQL Conf. Level: ";
		switch(dbInf.sqlConfLvl)
		{
		case SQL_OSC_MINIMUM:     std::wcout << L"Minimum Grammar";     break;
		case SQL_OSC_CORE:        std::wcout << L"Core Grammar";        break;
		case SQL_OSC_EXTENDED:    std::wcout << L"Extended Grammar";    break;
		}
		std::wcout << std::endl;

		std::wcout << L"Max. Connections: "       << dbInf.maxConnections   << std::endl;
		std::wcout << L"Outer Joins: "            << dbInf.outerJoins       << std::endl;
		std::wcout << L"Support for Procedures: " << dbInf.procedureSupport << std::endl;
		std::wcout << L"All tables accessible : " << dbInf.accessibleTables << std::endl;
		std::wcout << L"Cursor COMMIT Behavior: ";
		switch(dbInf.cursorCommitBehavior)
		{
		case SQL_CB_DELETE:        std::wcout << L"Delete cursors";      break;
		case SQL_CB_CLOSE:         std::wcout << L"Close cursors";       break;
		case SQL_CB_PRESERVE:      std::wcout << L"Preserve cursors";    break;
		}
		std::wcout << std::endl;

		std::wcout << L"Cursor ROLLBACK Behavior: ";
		switch(dbInf.cursorRollbackBehavior)
		{
		case SQL_CB_DELETE:      std::wcout << L"Delete cursors";      break;
		case SQL_CB_CLOSE:       std::wcout << L"Close cursors";       break;
		case SQL_CB_PRESERVE:    std::wcout << L"Preserve cursors";    break;
		}
		std::wcout << std::endl;

		std::wcout << L"Support NOT NULL clause: ";
		switch(dbInf.supportNotNullClause)
		{
		case SQL_NNC_NULL:        std::wcout << L"No";        break;
		case SQL_NNC_NON_NULL:    std::wcout << L"Yes";       break;
		}
		std::wcout << std::endl;

		std::wcout << L"Support IEF (Ref. Integrity): " << dbInf.supportIEF   << std::endl;
		std::wcout << L"Login Timeout: "                << dbInf.loginTimeout << std::endl;

		std::wcout << std::endl << std::endl << L"more ..." << std::endl;
		getchar();

		std::wcout << L"Default Transaction Isolation: ";
		switch(dbInf.txnIsolation)
		{
		case SQL_TXN_READ_UNCOMMITTED:  std::wcout << L"Read Uncommitted";    break;
		case SQL_TXN_READ_COMMITTED:    std::wcout << L"Read Committed";      break;
		case SQL_TXN_REPEATABLE_READ:   std::wcout << L"Repeatable Read";     break;
		case SQL_TXN_SERIALIZABLE:      std::wcout << L"Serializable";        break;
#ifdef ODBC_V20
		case SQL_TXN_VERSIONING:        std::wcout << L"Versioning";          break;
#endif
		}
		std::wcout << std::endl;

		std::wcout << L"Transaction Isolation Options: ";
		if (dbInf.txnIsolationOptions & SQL_TXN_READ_UNCOMMITTED)
			std::wcout << L"Read Uncommitted, ";
		if (dbInf.txnIsolationOptions & SQL_TXN_READ_COMMITTED)
			std::wcout << L"Read Committed, ";
		if (dbInf.txnIsolationOptions & SQL_TXN_REPEATABLE_READ)
			std::wcout << L"Repeatable Read, ";
		if (dbInf.txnIsolationOptions & SQL_TXN_SERIALIZABLE)
			std::wcout << L"Serializable, ";
#ifdef ODBC_V20
		if (dbInf.txnIsolationOptions & SQL_TXN_VERSIONING)
			std::wcout << L"Versioning";
#endif
		std::wcout << std::endl;

		std::wcout << L"Fetch Directions Supported:" << std::endl << L"   ";
		if (dbInf.fetchDirections & SQL_FD_FETCH_NEXT)
			std::wcout << L"Next, ";
		if (dbInf.fetchDirections & SQL_FD_FETCH_PRIOR)
			std::wcout << L"Prev, ";
		if (dbInf.fetchDirections & SQL_FD_FETCH_FIRST)
			std::wcout << L"First, ";
		if (dbInf.fetchDirections & SQL_FD_FETCH_LAST)
			std::wcout << L"Last, ";
		if (dbInf.fetchDirections & SQL_FD_FETCH_ABSOLUTE)
			std::wcout << L"Absolute, ";
		if (dbInf.fetchDirections & SQL_FD_FETCH_RELATIVE)
			std::wcout << L"Relative, ";
#ifdef ODBC_V20
		if (dbInf.fetchDirections & SQL_FD_FETCH_RESUME)
			std::wcout << L"Resume, ";
#endif
		if (dbInf.fetchDirections & SQL_FD_FETCH_BOOKMARK)
			std::wcout << L"Bookmark";
		std::wcout << std::endl;

		std::wcout << L"Lock Types Supported (SQLSetPos): ";
		if (dbInf.lockTypes & SQL_LCK_NO_CHANGE)
			std::wcout << L"No Change, ";
		if (dbInf.lockTypes & SQL_LCK_EXCLUSIVE)
			std::wcout << L"Exclusive, ";
		if (dbInf.lockTypes & SQL_LCK_UNLOCK)
			std::wcout << L"UnLock";
		std::wcout << std::endl;

		std::wcout << L"Position Operations Supported (SQLSetPos): ";
		if (dbInf.posOperations & SQL_POS_POSITION)
			std::wcout << L"Position, ";
		if (dbInf.posOperations & SQL_POS_REFRESH)
			std::wcout << L"Refresh, ";
		if (dbInf.posOperations & SQL_POS_UPDATE)
			std::wcout << L"Upd, ";
		if (dbInf.posOperations & SQL_POS_DELETE)
			std::wcout << L"Del, ";
		if (dbInf.posOperations & SQL_POS_ADD)
			std::wcout << L"Add";
		std::wcout << std::endl;

		std::wcout << L"Positioned Statements Supported: ";
		if (dbInf.posStmts & SQL_PS_POSITIONED_DELETE)
			std::wcout << L"Pos delete, ";
		if (dbInf.posStmts & SQL_PS_POSITIONED_UPDATE)
			std::wcout << L"Pos update, ";
		if (dbInf.posStmts & SQL_PS_SELECT_FOR_UPDATE)
			std::wcout << L"Select for update";
		std::wcout << std::endl;

		std::wcout << L"Scroll Concurrency: ";
		if (dbInf.scrollConcurrency & SQL_SCCO_READ_ONLY)
			std::wcout << L"Read Only, ";
		if (dbInf.scrollConcurrency & SQL_SCCO_LOCK)
			std::wcout << L"Lock, ";
		if (dbInf.scrollConcurrency & SQL_SCCO_OPT_ROWVER)
			std::wcout << L"Opt. Rowver, ";
		if (dbInf.scrollConcurrency & SQL_SCCO_OPT_VALUES)
			std::wcout << L"Opt. Values";
		std::wcout << std::endl;

		std::wcout << L"Scroll Options: ";
		if (dbInf.scrollOptions & SQL_SO_FORWARD_ONLY)
			std::wcout << L"Fwd Only, ";
		if (dbInf.scrollOptions & SQL_SO_STATIC)
			std::wcout << L"Static, ";
		if (dbInf.scrollOptions & SQL_SO_KEYSET_DRIVEN)
			std::wcout << L"Keyset Driven, ";
		if (dbInf.scrollOptions & SQL_SO_DYNAMIC)
			std::wcout << L"Dynamic, ";
		if (dbInf.scrollOptions & SQL_SO_MIXED)
			std::wcout << L"Mixed";
		std::wcout << std::endl;

		std::wcout << L"Static Sensitivity: ";
		if (dbInf.staticSensitivity & SQL_SS_ADDITIONS)
			std::wcout << L"Additions, ";
		if (dbInf.staticSensitivity & SQL_SS_DELETIONS)
			std::wcout << L"Deletions, ";
		if (dbInf.staticSensitivity & SQL_SS_UPDATES)
			std::wcout << L"Updates";
		std::wcout << std::endl;

		std::wcout << L"Transaction Capable?: ";
		switch(dbInf.txnCapable)
		{
		case SQL_TC_NONE:          std::wcout << L"No";            break;
		case SQL_TC_DML:           std::wcout << L"DML Only";      break;
		case SQL_TC_DDL_COMMIT:    std::wcout << L"DDL Commit";    break;
		case SQL_TC_DDL_IGNORE:    std::wcout << L"DDL Ignore";    break;
		case SQL_TC_ALL:           std::wcout << L"DDL & DML";     break;
		}
		std::wcout << std::endl;

		std::wcout << std::endl;
#endif

		// Completed Successfully
		return true;

	} // wxDb::getDbInfo()


	/********** wxDb::GetDataTypeInfoImpl() **********/
	bool Database::GetDataTypeInfoImpl(SWORD fSqlType, SSqlTypeInfo &structSQLTypeInfo)
	{
		/*
		* fSqlType will be something like SQL_VARCHAR.  This parameter determines
		* the data type inf. is gathered for.
		*
		* wxDbSqlTypeInfo is a structure that is filled in with data type information,
		*/
		RETCODE retcode;
		SQLLEN  cbRet;

		// Get information about the data type specified
		if (SQLGetTypeInfo(m_hstmt, fSqlType) != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc, m_hstmt));

		// Fetch the record
		retcode = SQLFetch(m_hstmt);
		if (retcode != SQL_SUCCESS)
		{
#ifdef DBDEBUG_CONSOLE
			if (retcode == SQL_NO_DATA_FOUND)
				std::wcout << L"SQL_NO_DATA_FOUND fetching information about data type." << std::endl;
#endif
			DispAllErrors(m_henv, m_hdbc, m_hstmt);
			SQLFreeStmt(m_hstmt, SQL_CLOSE);
			return false;
		}

		wchar_t typeName[DB_TYPE_NAME_LEN+1];

		// Obtain columns from the record
		if (SQLGetData(m_hstmt, 1, SQL_C_WXCHAR, typeName, sizeof(typeName), &cbRet) != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc, m_hstmt));

		structSQLTypeInfo.TypeName = typeName;

		// BJO 20000503: no more needed with new GetColumns...
#if  OLD_GETCOLUMNS
		// BJO 991209
		if (Dbms() == dbmsMY_SQL)
		{
			if (structSQLTypeInfo.TypeName == L"middleint")
				structSQLTypeInfo.TypeName = L"mediumint";
			else if (structSQLTypeInfo.TypeName == L"middleint unsigned")
				structSQLTypeInfo.TypeName = L"mediumint unsigned";
			else if (structSQLTypeInfo.TypeName == L"integer")
				structSQLTypeInfo.TypeName = L"int";
			else if (structSQLTypeInfo.TypeName == L"integer unsigned")
				structSQLTypeInfo.TypeName = L"int unsigned";
			else if (structSQLTypeInfo.TypeName == L"middleint")
				structSQLTypeInfo.TypeName = L"mediumint";
			else if (structSQLTypeInfo.TypeName == L"varchar")
				structSQLTypeInfo.TypeName = L"char";
		}

		// BJO 20000427 : OpenLink driver
		if (!wcsncmp(dbInf.driverName, L"oplodbc", 7) ||
			!wcsncmp(dbInf.driverName, L"OLOD", 4))
		{
			if (structSQLTypeInfo.TypeName == L"double precision")
				structSQLTypeInfo.TypeName = L"real";
		}
#endif

		if (SQLGetData(m_hstmt, 3, SQL_C_LONG, (UCHAR*) &structSQLTypeInfo.Precision, 0, &cbRet) != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc, m_hstmt));
		if (SQLGetData(m_hstmt, 8, SQL_C_SHORT, (UCHAR*) &structSQLTypeInfo.CaseSensitive, 0, &cbRet) != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc, m_hstmt));
		//    if (SQLGetData(hstmt, 14, SQL_C_SHORT, (UCHAR*) &structSQLTypeInfo.MinimumScale, 0, &cbRet) != SQL_SUCCESS)
		//        return(DispAllErrors(henv, hdbc, hstmt));

		if (SQLGetData(m_hstmt, 15, SQL_C_SHORT,(UCHAR*)  &structSQLTypeInfo.MaximumScale, 0, &cbRet) != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc, m_hstmt));

		if (structSQLTypeInfo.MaximumScale < 0)
			structSQLTypeInfo.MaximumScale = 0;

		// Close the statement handle which closes OpenImpl cursors
		if (SQLFreeStmt(m_hstmt, SQL_CLOSE) != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc, m_hstmt));

		// Completed Successfully
		return true;

	} // wxDb::GetDataTypeInfoImpl()


	/********** wxDb::Close() **********/
	void Database::Close()
	{
		// Close the Sql Log file
		if (m_fpSqlLog)
		{
			fclose(m_fpSqlLog);
			m_fpSqlLog = 0;
		}

		// Free statement handle
		if (m_dbIsOpen)
		{
			if (SQLFreeStmt(m_hstmt, SQL_DROP) != SQL_SUCCESS)
				DispAllErrors(m_henv, m_hdbc);
		}

		// Disconnect from the datasource
		if (SQLDisconnect(m_hdbc) != SQL_SUCCESS)
			DispAllErrors(m_henv, m_hdbc);

		// Free the connection to the datasource
		if (SQLFreeConnect(m_hdbc) != SQL_SUCCESS)
			DispAllErrors(m_henv, m_hdbc);

		// There should be zero Ctable objects still connected to this db object
		exASSERT(nTables == 0);

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
					s1 = (boost::wformat(L"(%-20s)     tableID:[%6lu]     pDb:[%p]") % tiu->tableName % tiu->tableID % static_cast<void*>(tiu->pDb)).str();
					s2 = (boost::wformat(L"Orphaned table found using pDb:[%p]") % static_cast<void*>(this)).str();
					BOOST_LOG_TRIVIAL(debug) << s1 << s2;
				}
				it++;
				//pNode = pNode->GetNext();
			}
		}
#endif

		// Copy the error messages to a global variable
		int i;
		for (i = 0; i < DB_MAX_ERROR_HISTORY; i++)
			wcscpy(DBerrorList[i], errorList[i]);

		m_dbmsType = dbmsUNIDENTIFIED;
		m_dbIsOpen = false;

	} // wxDb::Close()


	/********** wxDb::CommitTrans() **********/
	bool Database::CommitTrans()
	{
		if (this)
		{
			// Commit the transaction
			if (SQLTransact(m_henv, m_hdbc, SQL_COMMIT) != SQL_SUCCESS)
				return(DispAllErrors(m_henv, m_hdbc));
		}

		// Completed successfully
		return true;

	} // wxDb::CommitTrans()


	/********** wxDb::RollbackTrans() **********/
	bool Database::RollbackTrans()
	{
		// Rollback the transaction
		if (SQLTransact(m_henv, m_hdbc, SQL_ROLLBACK) != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc));

		// Completed successfully
		return true;

	} // wxDb::RollbackTrans()


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
		std::wstring odbcErrMsg;

		while (SQLError(aHenv, aHdbc, aHstmt, (SQLTCHAR FAR *) sqlState, &nativeError, (SQLTCHAR FAR *) errorMsg, SQL_MAX_MESSAGE_LENGTH - 1, &cbErrorMsg) == SQL_SUCCESS)
		{
			odbcErrMsg = (boost::wformat(L"SQL State = %s\nNative Error Code = %li\nError Message = %s\n") % sqlState % (long)nativeError % errorMsg).str();
			LogErrorImpl(odbcErrMsg, sqlState);
			if (!m_silent)
			{
#ifdef DBDEBUG_CONSOLE
				// When run in console mode, use standard out to display errors.
				std::wcout << odbcErrMsg.c_str() << std::endl;
				std::wcout << L"Press any key to continue..." << std::endl;
				getchar();
#endif

#ifdef EXODBCDEBUG
				BOOST_LOG_TRIVIAL(debug) <<  L"ODBC DEBUG MESSAGE from DispAllErrors(): " << odbcErrMsg;
#endif
			}
		}

		return false;  // This function always returns false.

	} // wxDb::DispAllErrors()


	/********** wxDb::GetNextError() **********/
	bool Database::GetNextError(HENV aHenv, HDBC aHdbc, HSTMT aHstmt)
	{
		if (SQLError(aHenv, aHdbc, aHstmt, (SQLTCHAR FAR *) sqlState, &nativeError, (SQLTCHAR FAR *) errorMsg, SQL_MAX_MESSAGE_LENGTH - 1, &cbErrorMsg) == SQL_SUCCESS)
			return true;
		else
			return false;

	} // wxDb::GetNextError()


	/********** wxDb::DispNextError() **********/
	void Database::DispNextError()
	{
		std::wstring odbcErrMsg;

		odbcErrMsg = (boost::wformat(L"SQL State = %s\nNative Error Code = %li\nError Message = %s\n") % sqlState % (long)nativeError % errorMsg).str();
		LogErrorImpl(odbcErrMsg, sqlState);

		if (m_silent)
			return;

#ifdef DBDEBUG_CONSOLE
		// When run in console mode, use standard out to display errors.
		std::wcout << odbcErrMsg.c_str() << std::endl;
		std::wcout << L"Press any key to continue..."  << std::endl;
		getchar();
#endif

#ifdef EXODBCDEBUG
		BOOST_LOG_TRIVIAL(debug) << L"ODBC DEBUG MESSAGE: " << odbcErrMsg;
#endif  // EXODBCDEBUG

	} // wxDb::DispNextError()


	/********** wxDb::LogErrorImpl() **********/
	void Database::LogErrorImpl(const std::wstring &errMsg, const std::wstring &SQLState)
	{
		exASSERT(errMsg.length());

		static int pLast = -1;
		int dbStatus;

		if (++pLast == DB_MAX_ERROR_HISTORY)
		{
			int i;
			for (i = 0; i < DB_MAX_ERROR_HISTORY-1; i++)
				wcscpy(errorList[i], errorList[i+1]);
			pLast--;
		}

		wcsncpy(errorList[pLast], errMsg.c_str(), DB_MAX_ERROR_MSG_LEN);
		errorList[pLast][DB_MAX_ERROR_MSG_LEN-1] = 0;

		if (SQLState.length())
			if ((dbStatus = TranslateSqlState(SQLState)) != DB_ERR_FUNCTION_SEQUENCE_ERROR)
				DB_STATUS = dbStatus;

		// Add the errmsg to the sql log
		WriteSqlLog(errMsg);

	}  // wxDb::LogErrorImpl()


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


	/**********  wxDb::Grant() **********/
	bool Database::Grant(int privileges, const std::wstring &tableName, const std::wstring &userList)
	{
		std::wstring sqlStmt;

		// Build the grant statement
		sqlStmt  = L"GRANT ";
		if (privileges == DB_GRANT_ALL)
			sqlStmt += L"ALL";
		else
		{
			int c = 0;
			if (privileges & DB_GRANT_SELECT)
			{
				sqlStmt += L"SELECT";
				c++;
			}
			if (privileges & DB_GRANT_INSERT)
			{
				if (c++)
					sqlStmt += L", ";
				sqlStmt += L"INSERT";
			}
			if (privileges & DB_GRANT_UPDATE)
			{
				if (c++)
					sqlStmt += L", ";
				sqlStmt += L"UPDATE";
			}
			if (privileges & DB_GRANT_DELETE)
			{
				if (c++)
					sqlStmt += L", ";
				sqlStmt += L"DELETE";
			}
		}

		sqlStmt += L" ON ";
		sqlStmt += SQLTableName(tableName.c_str());
		sqlStmt += L" TO ";
		sqlStmt += userList;

#ifdef DBDEBUG_CONSOLE
		std::wcout << std::endl << sqlStmt.c_str() << std::endl;
#endif

		WriteSqlLog(sqlStmt);

		return(ExecSql(sqlStmt));

	}  // wxDb::Grant()


	/********** wxDb::CreateView() **********/
	bool Database::CreateView(const std::wstring &viewName, const std::wstring &colList,
		const std::wstring &pSqlStmt, bool attemptDrop)
	{
		std::wstring sqlStmt;

		// Drop the view first
		if (attemptDrop && !DropView(viewName))
			return false;

		// Build the create view statement
		sqlStmt  = L"CREATE VIEW ";
		sqlStmt += viewName;

		if (colList.length())
		{
			sqlStmt += L" (";
			sqlStmt += colList;
			sqlStmt += L")";
		}

		sqlStmt += L" AS ";
		sqlStmt += pSqlStmt;

		WriteSqlLog(sqlStmt);

#ifdef DBDEBUG_CONSOLE
		std::wcout << sqlStmt.c_str() << std::endl;
#endif

		return(ExecSql(sqlStmt));

	}  // wxDb::CreateView()


	/********** wxDb::DropView()  **********/
	bool Database::DropView(const std::wstring &viewName)
	{
		/*
		* NOTE: This function returns true if the View does not exist, but
		*       only for identified databases.  Code will need to be added
		*            below for any other databases when those databases are defined
		*       to handle this situation consistently
		*/
		std::wstring sqlStmt;

		sqlStmt = (boost::wformat(L"DROP VIEW %s") % viewName).str();

		WriteSqlLog(sqlStmt);

#ifdef DBDEBUG_CONSOLE
		std::wcout << std::endl << sqlStmt.c_str() << std::endl;
#endif

		if (SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS) != SQL_SUCCESS)
		{
			// Check for "Base table not found" error and ignore
			GetNextError(m_henv, m_hdbc, m_hstmt);
			if (wcscmp(sqlState, L"S0002"))  // "Base table not found"
			{
				// Check for product specific error codes
				if (!((Dbms() == dbmsSYBASE_ASA    && !wcscmp(sqlState, L"42000"))))  // 5.x (and lower?)
				{
					DispNextError();
					DispAllErrors(m_henv, m_hdbc, m_hstmt);
					RollbackTrans();
					return false;
				}
			}
		}

		// Commit the transaction
		if (!CommitTrans())
			return false;

		return true;

	}  // wxDb::DropView()


	/********** wxDb::ExecSql()  **********/
	bool Database::ExecSql(const std::wstring &pSqlStmt)
	{
		RETCODE retcode;

		SQLFreeStmt(m_hstmt, SQL_CLOSE);

		retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) pSqlStmt.c_str(), SQL_NTS);
		if (retcode == SQL_SUCCESS ||
			(Dbms() == dbmsDB2 && (retcode == SQL_SUCCESS_WITH_INFO || retcode == SQL_NO_DATA_FOUND)))
		{
			return true;
		}
		else
		{
			DispAllErrors(m_henv, m_hdbc, m_hstmt);
			return false;
		}

	}  // wxDb::ExecSql()


	/********** wxDb::ExecSql() with column info **********/
	bool Database::ExecSql(const std::wstring &pSqlStmt, ColumnInfo** columns, short& numcols)
	{
		//execute the statement first
		if (!ExecSql(pSqlStmt))
			return false;

		SWORD noCols;
		if (SQLNumResultCols(m_hstmt, &noCols) != SQL_SUCCESS)
		{
			DispAllErrors(m_henv, m_hdbc, m_hstmt);
			return false;
		}

		if (noCols == 0)
			return false;
		else
			numcols = noCols;

		//  Get column information
		short colNum;
		wchar_t name[DB_MAX_COLUMN_NAME_LEN+1];
		SWORD Sword;
		SQLLEN Sqllen;
		ColumnInfo* pColInf = new ColumnInfo[noCols];

		// Fill in column information (name, datatype)
		for (colNum = 0; colNum < noCols; colNum++)
		{
			if (SQLColAttributes(m_hstmt, (UWORD)(colNum+1), SQL_COLUMN_NAME,
				name, sizeof(name),
				&Sword, &Sqllen) != SQL_SUCCESS)
			{
				DispAllErrors(m_henv, m_hdbc, m_hstmt);
				delete[] pColInf;
				return false;
			}

			wcsncpy(pColInf[colNum].m_colName, name, DB_MAX_COLUMN_NAME_LEN);
			pColInf[colNum].m_colName[DB_MAX_COLUMN_NAME_LEN] = 0;  // Prevent buffer overrun

			if (SQLColAttributes(m_hstmt, (UWORD)(colNum+1), SQL_COLUMN_TYPE,
				NULL, 0, &Sword, &Sqllen) != SQL_SUCCESS)
			{
				DispAllErrors(m_henv, m_hdbc, m_hstmt);
				delete[] pColInf;
				return false;
			}

			switch (Sqllen)
			{
#if defined(SQL_WCHAR)
			case SQL_WCHAR:
#endif
#if defined(SQL_WVARCHAR)
			case SQL_WVARCHAR:
#endif
			case SQL_VARCHAR:
			case SQL_CHAR:
				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_VARCHAR;
				break;
			case SQL_LONGVARCHAR:
				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_MEMO;
				break;
			case SQL_TINYINT:
			case SQL_SMALLINT:
			case SQL_INTEGER:
			case SQL_BIT:
				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_INTEGER;
				break;
			case SQL_DOUBLE:
			case SQL_DECIMAL:
			case SQL_NUMERIC:
			case SQL_FLOAT:
			case SQL_REAL:
				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_FLOAT;
				break;
			case SQL_DATE:
			case SQL_TIMESTAMP:
				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_DATE;
				break;
			case SQL_BINARY:
				pColInf[colNum].m_dbDataType = DB_DATA_TYPE_BLOB;
				break;
#ifdef EXODBCDEBUG
			default:
				std::wstring errMsg;
				errMsg = (boost::wformat(L"SQL Data type %ld currently not supported by wxWidgets") % (long)Sqllen).str();
				BOOST_LOG_TRIVIAL(debug) << L"ODBC DEBUG MESSAGE: " << errMsg;
#endif
			}
		}

		*columns = pColInf;
		return true;
	}  // wxDb::ExecSql()

	/********** wxDb::GetNext()  **********/
	bool Database::GetNext()
	{
		if (SQLFetch(m_hstmt) == SQL_SUCCESS)
			return true;
		else
		{
			DispAllErrors(m_henv, m_hdbc, m_hstmt);
			return false;
		}

	}  // wxDb::GetNext()


	/********** wxDb::GetData()  **********/
	bool Database::GetData(UWORD colNo, SWORD cType, PTR pData, SDWORD maxLen, SQLLEN FAR *cbReturned)
	{
		exASSERT(pData);
		exASSERT(cbReturned);

		long bufferSize = maxLen;

		if (cType == SQL_C_WXCHAR)
			bufferSize = maxLen * sizeof(wchar_t);

		if (SQLGetData(m_hstmt, colNo, cType, pData, bufferSize, cbReturned) == SQL_SUCCESS)
			return true;
		else
		{
			DispAllErrors(m_henv, m_hdbc, m_hstmt);
			return false;
		}

	}  // wxDb::GetData()


	/********** wxDb::GetKeyFields() **********/
	int Database::GetKeyFields(const std::wstring &tableName, ColumnInfo* colInf, UWORD noCols)
	{
		wchar_t       szPkTable[DB_MAX_TABLE_NAME_LEN+1];  /* Primary key table name */
		wchar_t       szFkTable[DB_MAX_TABLE_NAME_LEN+1];  /* Foreign key table name */
		SWORD        iKeySeq;
		wchar_t       szPkCol[DB_MAX_COLUMN_NAME_LEN+1];   /* Primary key column     */
		wchar_t       szFkCol[DB_MAX_COLUMN_NAME_LEN+1];   /* Foreign key column     */
		SQLRETURN    retcode;
		SQLLEN       cb;
		SWORD        i;
		std::wstring     tempStr;
		/*
		* -----------------------------------------------------------------------
		* -- 19991224 : mj10777 : Create                                   ------
		* --          : Three things are done and stored here :            ------
		* --          : 1) which Column(s) is/are Primary Key(s)           ------
		* --          : 2) which tables use this Key as a Foreign Key      ------
		* --          : 3) which columns are Foreign Key and the name      ------
		* --          :     of the Table where the Key is the Primary Key  -----
		* --          : Called from GetColumns(const std::wstring &tableName,  ------
		* --                           int *numCols,const wchar_t *userID ) ------
		* -----------------------------------------------------------------------
		*/

		/*---------------------------------------------------------------------*/
		/* Get the names of the columns in the primary key.                    */
		/*---------------------------------------------------------------------*/
		retcode = SQLPrimaryKeys(m_hstmt,
			NULL, 0,                               /* Catalog name  */
			NULL, 0,                               /* Schema name   */
			(SQLTCHAR FAR *) tableName.c_str(), SQL_NTS); /* Table name    */

		/*---------------------------------------------------------------------*/
		/* Fetch and display the result set. This will be a list of the        */
		/* columns in the primary key of the tableName table.                  */
		/*---------------------------------------------------------------------*/
		while ((retcode == SQL_SUCCESS) || (retcode == SQL_SUCCESS_WITH_INFO))
		{
			retcode = SQLFetch(m_hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				GetData( 4, SQL_C_WXCHAR,  szPkCol,    DB_MAX_COLUMN_NAME_LEN+1, &cb);
				GetData( 5, SQL_C_SSHORT, &iKeySeq,    0,                        &cb);
				//-------
				for (i=0;i<noCols;i++)                          // Find the Column name
					if (!wcscmp(colInf[i].m_colName,szPkCol))   // We have found the Column
						colInf[i].m_pkCol = iKeySeq;              // Which Primary Key is this (first, second usw.) ?
			}  // if
		}  // while
		SQLFreeStmt(m_hstmt, SQL_CLOSE);  /* Close the cursor (the hstmt is still allocated).      */

		/*---------------------------------------------------------------------*/
		/* Get all the foreign keys that refer to tableName primary key.       */
		/*---------------------------------------------------------------------*/
		retcode = SQLForeignKeys(m_hstmt,
			NULL, 0,                            /* Primary catalog */
			NULL, 0,                            /* Primary schema  */
			(SQLTCHAR FAR *)tableName.c_str(), SQL_NTS,/* Primary table   */
			NULL, 0,                            /* Foreign catalog */
			NULL, 0,                            /* Foreign schema  */
			NULL, 0);                           /* Foreign table   */

		/*---------------------------------------------------------------------*/
		/* Fetch and display the result set. This will be all of the foreign   */
		/* keys in other tables that refer to the tableName  primary key.      */
		/*---------------------------------------------------------------------*/
		tempStr.empty();
		std::wstringstream tempStream;
		szPkCol[0] = 0;
		while ((retcode == SQL_SUCCESS) || (retcode == SQL_SUCCESS_WITH_INFO))
		{
			retcode = SQLFetch(m_hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				GetData( 3, SQL_C_WXCHAR,  szPkTable,   DB_MAX_TABLE_NAME_LEN+1,   &cb);
				GetData( 4, SQL_C_WXCHAR,  szPkCol,     DB_MAX_COLUMN_NAME_LEN+1,  &cb);
				GetData( 5, SQL_C_SSHORT, &iKeySeq,     0,                         &cb);
				GetData( 7, SQL_C_WXCHAR,  szFkTable,   DB_MAX_TABLE_NAME_LEN+1,   &cb);
				GetData( 8, SQL_C_WXCHAR,  szFkCol,     DB_MAX_COLUMN_NAME_LEN+1,  &cb);
				tempStream << L'[' << szFkTable << L']';  // [ ] in case there is a blank in the Table name
				//            tempStr << _T('[') << szFkTable << _T(']');  // [ ] in case there is a blank in the Table name
			}  // if
		}  // while

		tempStr = tempStream.str();
		boost::trim_right(tempStr);     // Get rid of any unneeded blanks
		if (!tempStr.empty())
		{
			for (i=0; i<noCols; i++)
			{   // Find the Column name
				if (!wcscmp(colInf[i].m_colName, szPkCol))           // We have found the Column, store the Information
				{
					wcsncpy(colInf[i].m_pkTableName, tempStr.c_str(), DB_MAX_TABLE_NAME_LEN);  // Name of the Tables where this Primary Key is used as a Foreign Key
					colInf[i].m_pkTableName[DB_MAX_TABLE_NAME_LEN] = 0;  // Prevent buffer overrun
				}
			}
		}  // if

		SQLFreeStmt(m_hstmt, SQL_CLOSE);  /* Close the cursor (the hstmt is still allocated). */

		/*---------------------------------------------------------------------*/
		/* Get all the foreign keys in the tablename table.                    */
		/*---------------------------------------------------------------------*/
		retcode = SQLForeignKeys(m_hstmt,
			NULL, 0,                             /* Primary catalog   */
			NULL, 0,                             /* Primary schema    */
			NULL, 0,                             /* Primary table     */
			NULL, 0,                             /* Foreign catalog   */
			NULL, 0,                             /* Foreign schema    */
			(SQLTCHAR *)tableName.c_str(), SQL_NTS);/* Foreign table     */

		/*---------------------------------------------------------------------*/
		/*  Fetch and display the result set. This will be all of the          */
		/*  primary keys in other tables that are referred to by foreign       */
		/*  keys in the tableName table.                                       */
		/*---------------------------------------------------------------------*/
		while ((retcode == SQL_SUCCESS) || (retcode == SQL_SUCCESS_WITH_INFO))
		{
			retcode = SQLFetch(m_hstmt);
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
			{
				GetData( 3, SQL_C_WXCHAR,  szPkTable,   DB_MAX_TABLE_NAME_LEN+1,  &cb);
				GetData( 5, SQL_C_SSHORT, &iKeySeq,     0,                        &cb);
				GetData( 8, SQL_C_WXCHAR,  szFkCol,     DB_MAX_COLUMN_NAME_LEN+1, &cb);
				//-------
				for (i=0; i<noCols; i++)                            // Find the Column name
				{
					if (!wcscmp(colInf[i].m_colName,szFkCol))       // We have found the (Foreign Key) Column
					{
						colInf[i].m_fkCol = iKeySeq;                  // Which Foreign Key is this (first, second usw.) ?
						wcsncpy(colInf[i].m_fkTableName, szFkTable, DB_MAX_TABLE_NAME_LEN);  // Name of the Table where this Foriegn is the Primary Key
						colInf[i].m_fkTableName[DB_MAX_TABLE_NAME_LEN] = 0;  // Prevent buffer overrun
					} // if
				}  // for
			}  // if
		}  // while
		SQLFreeStmt(m_hstmt, SQL_CLOSE);  /* Close the cursor (the hstmt is still allocated). */

		return TRUE;

	}  // wxDb::GetKeyFields()


#if OLD_GETCOLUMNS
	/********** wxDb::GetColumns() **********/
	ColumnInfo *Database::GetColumns(wchar_t *tableName[], const wchar_t *userID)
		/*
		*        1) The last array element of the tableName[] argument must be zero (null).
		*            This is how the end of the array is detected.
		*        2) This function returns an array of wxDbColInf structures.  If no columns
		*            were found, or an error occurred, this pointer will be zero (null).  THE
		*            CALLING FUNCTION IS RESPONSIBLE FOR DELETING THE MEMORY RETURNED WHEN IT
		*            IS FINISHED WITH IT.  i.e.
		*
		*            wxDbColInf *colInf = pDb->GetColumns(tableList, userID);
		*            if (colInf)
		*            {
		*                // Use the column inf
		*                .......
		*                // Destroy the memory
		*                delete [] colInf;
		*            }
		*
		* userID is evaluated in the following manner:
		*        userID == NULL  ... UserID is ignored
		*        userID == ""    ... UserID set equal to 'this->uid'
		*        userID != ""    ... UserID set equal to 'userID'
		*
		* NOTE: ALL column bindings associated with this wxDb instance are unbound
		*       by this function.  This function should use its own wxDb instance
		*       to avoid undesired unbinding of columns.
		*/
	{
		UWORD       noCols = 0;
		UWORD       colNo  = 0;
		ColumnInfo *colInf = 0;

		RETCODE  retcode;
		SQLLEN   cb;

		std::wstring TableName;

		std::wstring UserID = ConvertUserIDImpl(userID);

		// Pass 1 - Determine how many columns there are.
		// Pass 2 - Allocate the wxDbColInf array and fill in
		//                the array with the column information.
		int pass;
		for (pass = 1; pass <= 2; pass++)
		{
			if (pass == 2)
			{
				if (noCols == 0)  // Probably a bogus table name(s)
					break;
				// Allocate n wxDbColInf objects to hold the column information
				colInf = new ColumnInfo[noCols+1];
				if (!colInf)
					break;
				// Mark the end of the array
				wcscpy(colInf[noCols].m_tableName, emptyString);
				wcscpy(colInf[noCols].m_colName, emptyString);
				colInf[noCols].m_sqlDataType = 0;
			}
			// Loop through each table name
			int tbl;
			for (tbl = 0; tableName[tbl]; tbl++)
			{
				TableName = tableName[tbl];
				// Oracle and Interbase table names are uppercase only, so force
				// the name to uppercase just in case programmer forgot to do this
				if ((Dbms() == dbmsORACLE) ||
					(Dbms() == dbmsFIREBIRD) ||
					(Dbms() == dbmsINTERBASE))
					boost::algorithm::to_upper(TableName);

				SQLFreeStmt(m_hstmt, SQL_CLOSE);

				// MySQL, SQLServer, and Access cannot accept a user name when looking up column names, so we
				// use the call below that leaves out the user name
				if (!UserID.empty() &&
					Dbms() != dbmsMY_SQL &&
					Dbms() != dbmsACCESS &&
					Dbms() != dbmsMS_SQL_SERVER)
				{
					retcode = SQLColumns(m_hstmt,
						NULL, 0,                                // All qualifiers
						(SQLTCHAR *) UserID.c_str(), SQL_NTS,      // Owner
						(SQLTCHAR *) TableName.c_str(), SQL_NTS,
						NULL, 0);                               // All columns
				}
				else
				{
					retcode = SQLColumns(m_hstmt,
						NULL, 0,                                // All qualifiers
						NULL, 0,                                // Owner
						(SQLTCHAR *) TableName.c_str(), SQL_NTS,
						NULL, 0);                               // All columns
				}
				if (retcode != SQL_SUCCESS)
				{  // Error occurred, abort
					DispAllErrors(m_henv, m_hdbc, m_hstmt);
					if (colInf)
						delete [] colInf;
					SQLFreeStmt(m_hstmt, SQL_CLOSE);
					return(0);
				}

				while ((retcode = SQLFetch(m_hstmt)) == SQL_SUCCESS)
				{
					if (pass == 1)  // First pass, just add up the number of columns
						noCols++;
					else  // Pass 2; Fill in the array of structures
					{
						if (colNo < noCols)  // Some extra error checking to prevent memory overwrites
						{
							// NOTE: Only the ODBC 1.x fields are retrieved
							GetData( 1, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_catalog,      128+1,                    &cb);
							GetData( 2, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_schema,       128+1,                    &cb);
							GetData( 3, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_tableName,    DB_MAX_TABLE_NAME_LEN+1,  &cb);
							GetData( 4, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_colName,      DB_MAX_COLUMN_NAME_LEN+1, &cb);
							GetData( 5, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_sqlDataType,  0,                        &cb);
							GetData( 6, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_typeName,     128+1,                    &cb);
							GetData( 7, SQL_C_SLONG,  (UCHAR*) &colInf[colNo].m_columnLength, 0,                        &cb);
							GetData( 8, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_bufferSize,   0,                        &cb);
							GetData( 9, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_decimalDigits,0,                        &cb);
							GetData(10, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_numPrecRadix, 0,                        &cb);
							GetData(11, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_nullable,     0,                        &cb);
							GetData(12, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_remarks,      254+1,                    &cb);

							// Determine the wxDb data type that is used to represent the native data type of this data source
							colInf[colNo].m_dbDataType = 0;
							if (!_wcsicmp(m_typeInfVarchar.TypeName.c_str(), colInf[colNo].m_typeName))
							{
#ifdef _IODBC_
								// IODBC does not return a correct columnLength, so we set
								// columnLength = bufferSize if no column length was returned
								// IODBC returns the columnLength in bufferSize. (bug)
								if (colInf[colNo].m_columnLength < 1)
								{
									colInf[colNo].m_columnLength = colInf[colNo].m_bufferSize;
								}
#endif
								colInf[colNo].m_dbDataType = DB_DATA_TYPE_VARCHAR;
							}
							else if (!_wcsicmp(m_typeInfInteger.TypeName.c_str(), colInf[colNo].m_typeName))
								colInf[colNo].m_dbDataType = DB_DATA_TYPE_INTEGER;
							else if (!_wcsicmp(m_typeInfFloat.TypeName.c_str(), colInf[colNo].m_typeName))
								colInf[colNo].m_dbDataType = DB_DATA_TYPE_FLOAT;
							else if (!_wcsicmp(m_typeInfDate.TypeName.c_str(), colInf[colNo].m_typeName))
								colInf[colNo].m_dbDataType = DB_DATA_TYPE_DATE;
							else if (!_wcsicmp(m_typeInfBlob.TypeName.c_str(), colInf[colNo].m_typeName))
								colInf[colNo].m_dbDataType = DB_DATA_TYPE_BLOB;
							colNo++;
						}
					}
				}
				if (retcode != SQL_NO_DATA_FOUND)
				{  // Error occurred, abort
					DispAllErrors(m_henv, m_hdbc, m_hstmt);
					if (colInf)
						delete [] colInf;
					SQLFreeStmt(m_hstmt, SQL_CLOSE);
					return(0);
				}
			}
		}

		SQLFreeStmt(m_hstmt, SQL_CLOSE);
		return colInf;

	}  // wxDb::GetColumns()


	/********** wxDb::GetColumns() **********/

	ColumnInfo *Database::GetColumns(const std::wstring &tableName, UWORD *numCols, const wchar_t *userID)
		//
		// Same as the above GetColumns() function except this one gets columns
		// only for a single table, and if 'numCols' is not NULL, the number of
		// columns stored in the returned wxDbColInf is set in '*numCols'
		//
		// userID is evaluated in the following manner:
		//        userID == NULL  ... UserID is ignored
		//        userID == ""    ... UserID set equal to 'this->uid'
		//        userID != ""    ... UserID set equal to 'userID'
		//
		// NOTE: ALL column bindings associated with this wxDb instance are unbound
		//       by this function.  This function should use its own wxDb instance
		//       to avoid undesired unbinding of columns.

	{
		UWORD       noCols = 0;
		UWORD       colNo  = 0;
		ColumnInfo *colInf = 0;

		RETCODE  retcode;
		SQLLEN   cb;

		std::wstring TableName;

		std::wstring UserID = ConvertUserIDImpl(userID);

		// Pass 1 - Determine how many columns there are.
		// Pass 2 - Allocate the wxDbColInf array and fill in
		//                the array with the column information.
		int pass;
		for (pass = 1; pass <= 2; pass++)
		{
			if (pass == 2)
			{
				if (noCols == 0)  // Probably a bogus table name(s)
					break;
				// Allocate n wxDbColInf objects to hold the column information
				colInf = new ColumnInfo[noCols+1];
				if (!colInf)
					break;
				// Mark the end of the array
				wcscpy(colInf[noCols].m_tableName, emptyString);
				wcscpy(colInf[noCols].m_colName, emptyString);
				colInf[noCols].m_sqlDataType = 0;
			}

			TableName = tableName;
			// Oracle and Interbase table names are uppercase only, so force
			// the name to uppercase just in case programmer forgot to do this
			if ((Dbms() == dbmsORACLE) ||
				(Dbms() == dbmsFIREBIRD) ||
				(Dbms() == dbmsINTERBASE))
				boost::algorithm::to_upper(TableName);

			SQLFreeStmt(m_hstmt, SQL_CLOSE);

			// MySQL, SQLServer, and Access cannot accept a user name when looking up column names, so we
			// use the call below that leaves out the user name
			if (!UserID.empty() &&
				Dbms() != dbmsMY_SQL &&
				Dbms() != dbmsACCESS &&
				Dbms() != dbmsMS_SQL_SERVER)
			{
				retcode = SQLColumns(m_hstmt,
					NULL, 0,                                // All qualifiers
					(SQLTCHAR *) UserID.c_str(), SQL_NTS,    // Owner
					(SQLTCHAR *) TableName.c_str(), SQL_NTS,
					NULL, 0);                               // All columns
			}
			else
			{
				retcode = SQLColumns(m_hstmt,
					NULL, 0,                                 // All qualifiers
					NULL, 0,                                 // Owner
					(SQLTCHAR *) TableName.c_str(), SQL_NTS,
					NULL, 0);                                // All columns
			}
			if (retcode != SQL_SUCCESS)
			{  // Error occurred, abort
				DispAllErrors(m_henv, m_hdbc, m_hstmt);
				if (colInf)
					delete [] colInf;
				SQLFreeStmt(m_hstmt, SQL_CLOSE);
				if (numCols)
					*numCols = 0;
				return(0);
			}

			while ((retcode = SQLFetch(m_hstmt)) == SQL_SUCCESS)
			{
				if (pass == 1)  // First pass, just add up the number of columns
					noCols++;
				else  // Pass 2; Fill in the array of structures
				{
					if (colNo < noCols)  // Some extra error checking to prevent memory overwrites
					{
						// NOTE: Only the ODBC 1.x fields are retrieved
						GetData( 1, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_catalog,      128+1,                     &cb);
						GetData( 2, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_schema,       128+1,                     &cb);
						GetData( 3, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_tableName,    DB_MAX_TABLE_NAME_LEN+1,   &cb);
						GetData( 4, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_colName,      DB_MAX_COLUMN_NAME_LEN+1,  &cb);
						GetData( 5, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_sqlDataType,  0,                         &cb);
						GetData( 6, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_typeName,     128+1,                     &cb);
						GetData( 7, SQL_C_SLONG,  (UCHAR*) &colInf[colNo].m_columnLength, 0,                         &cb);
						// BJO 991214 : SQL_C_SSHORT instead of SQL_C_SLONG, otherwise fails on Sparc (probably all 64 bit architectures)
						GetData( 8, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_bufferSize,   0,                         &cb);
						GetData( 9, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_decimalDigits,0,                         &cb);
						GetData(10, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_numPrecRadix, 0,                         &cb);
						GetData(11, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_nullable,     0,                         &cb);
						GetData(12, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_remarks,      254+1,                     &cb);
						// Start Values for Primary/Foriegn Key (=No)
						colInf[colNo].m_pkCol = 0;           // Primary key column   0=No; 1= First Key, 2 = Second Key etc.
						colInf[colNo].m_pkTableName[0] = 0;  // Tablenames where Primary Key is used as a Foreign Key
						colInf[colNo].m_fkCol = 0;           // Foreign key column   0=No; 1= First Key, 2 = Second Key etc.
						colInf[colNo].m_fkTableName[0] = 0;  // Foreign key table name

						// BJO 20000428 : Virtuoso returns type names with upper cases!
						if (Dbms() == dbmsVIRTUOSO)
						{
							std::wstring s = colInf[colNo].m_typeName;
							boost::algorithm::to_lower(s);
							wcscmp(colInf[colNo].m_typeName, s.c_str());
						}

						// Determine the wxDb data type that is used to represent the native data type of this data source
						colInf[colNo].m_dbDataType = 0;
						if (!_wcsicmp(m_typeInfVarchar.TypeName.c_str(), colInf[colNo].m_typeName))
						{
#ifdef _IODBC_
							// IODBC does not return a correct columnLength, so we set
							// columnLength = bufferSize if no column length was returned
							// IODBC returns the columnLength in bufferSize. (bug)
							if (colInf[colNo].m_columnLength < 1)
							{
								colInf[colNo].m_columnLength = colInf[colNo].m_bufferSize;
							}
#endif

							colInf[colNo].m_dbDataType = DB_DATA_TYPE_VARCHAR;
						}
						else if (!_wcsicmp(m_typeInfInteger.TypeName.c_str(), colInf[colNo].m_typeName))
							colInf[colNo].m_dbDataType = DB_DATA_TYPE_INTEGER;
						else if (!_wcsicmp(m_typeInfFloat.TypeName.c_str(), colInf[colNo].m_typeName))
							colInf[colNo].m_dbDataType = DB_DATA_TYPE_FLOAT;
						else if (!_wcsicmp(m_typeInfDate.TypeName.c_str(), colInf[colNo].m_typeName))
							colInf[colNo].m_dbDataType = DB_DATA_TYPE_DATE;
						else if (!_wcsicmp(m_typeInfBlob.TypeName.c_str(), colInf[colNo].m_typeName))
							colInf[colNo].m_dbDataType = DB_DATA_TYPE_BLOB;

						colNo++;
					}
				}
			}
			if (retcode != SQL_NO_DATA_FOUND)
			{  // Error occurred, abort
				DispAllErrors(m_henv, m_hdbc, m_hstmt);
				if (colInf)
					delete [] colInf;
				SQLFreeStmt(m_hstmt, SQL_CLOSE);
				if (numCols)
					*numCols = 0;
				return(0);
			}
		}

		SQLFreeStmt(m_hstmt, SQL_CLOSE);

		// Store Primary and Foriegn Keys
		GetKeyFields(tableName,colInf,noCols);

		if (numCols)
			*numCols = noCols;
		return colInf;

	}  // wxDb::GetColumns()


#else  // New GetColumns


	/*
	BJO 20000503
	These are tentative new GetColumns members which should be more database
	independent and which always returns the columns in the order they were
	created.

	- The first one (wxDbColInf *wxDb::GetColumns(wchar_t *tableName[], const
	wchar_t* userID)) calls the second implementation for each separate table
	before merging the results. This makes the code easier to maintain as
	only one member (the second) makes the real work
	- wxDbColInf *wxDb::GetColumns(const std::wstring &tableName, int *numCols, const
	wchar_t *userID) is a little bit improved
	- It doesn't anymore rely on the type-name to find out which database-type
	each column has
	- It ends by sorting the columns, so that they are returned in the same
	order they were created
	*/

	typedef struct
	{
		UWORD noCols;
		ColumnInfo *colInf;
	} _TableColumns;


	ColumnInfo *Database::GetColumns(wchar_t *tableName[], const wchar_t *userID)
	{
		int i, j;
		// The last array element of the tableName[] argument must be zero (null).
		// This is how the end of the array is detected.

		UWORD noCols = 0;

		// How many tables ?
		int tbl;
		for (tbl = 0 ; tableName[tbl]; tbl++);

		// Create a table to maintain the columns for each separate table
		_TableColumns *TableColumns = new _TableColumns[tbl];

		// Fill the table
		for (i = 0 ; i < tbl ; i++)

		{
			TableColumns[i].colInf = GetColumns(tableName[i], &TableColumns[i].noCols, userID);
			if (TableColumns[i].colInf == NULL)
				return NULL;
			noCols += TableColumns[i].noCols;
		}

		// Now merge all the separate table infos
		ColumnInfo *colInf = new ColumnInfo[noCols+1];

		// Mark the end of the array
		wcscpy(colInf[noCols].m_tableName, emptyString);
		wcscpy(colInf[noCols].m_colName, emptyString);
		colInf[noCols].m_sqlDataType = 0;

		// Merge ...
		int offset = 0;

		for (i = 0 ; i < tbl ; i++)
		{
			for (j = 0 ; j < TableColumns[i].noCols ; j++)
			{
				colInf[offset++] = TableColumns[i].colInf[j];
			}
		}

		delete [] TableColumns;

		return colInf;
	}  // wxDb::GetColumns()  -- NEW


	ColumnInfo *Database::GetColumns(const std::wstring &tableName, int *numCols, const wchar_t *userID)
		//
		// Same as the above GetColumns() function except this one gets columns
		// only for a single table, and if 'numCols' is not NULL, the number of
		// columns stored in the returned wxDbColInf is set in '*numCols'
		//
		// userID is evaluated in the following manner:
		//        userID == NULL  ... UserID is ignored
		//        userID == ""    ... UserID set equal to 'this->uid'
		//        userID != ""    ... UserID set equal to 'userID'
		//
		// NOTE: ALL column bindings associated with this wxDb instance are unbound
		//       by this function.  This function should use its own wxDb instance
		//       to avoid undesired unbinding of columns.
	{
		UWORD       noCols = 0;
		UWORD       colNo  = 0;
		ColumnInfo *colInf = 0;

		RETCODE  retcode;
		SDWORD   cb;

		std::wstring TableName;

		std::wstring UserID;
		ConvertUserIDImpl(userID,UserID);

		// Pass 1 - Determine how many columns there are.
		// Pass 2 - Allocate the wxDbColInf array and fill in
		//                the array with the column information.
		int pass;
		for (pass = 1; pass <= 2; pass++)
		{
			if (pass == 2)
			{
				if (noCols == 0)  // Probably a bogus table name(s)
					break;
				// Allocate n wxDbColInf objects to hold the column information
				colInf = new ColumnInfo[noCols+1];
				if (!colInf)
					break;
				// Mark the end of the array
				wcscpy(colInf[noCols].m_tableName, emptyString);
				wcscpy(colInf[noCols].m_colName, emptyString);
				colInf[noCols].m_sqlDataType = 0;
			}

			TableName = tableName;
			// Oracle and Interbase table names are uppercase only, so force
			// the name to uppercase just in case programmer forgot to do this
			if ((Dbms() == dbmsORACLE) ||
				(Dbms() == dbmsFIREBIRD) ||
				(Dbms() == dbmsINTERBASE))
				TableName = TableName.Upper();

			SQLFreeStmt(m_hstmt, SQL_CLOSE);

			// MySQL, SQLServer, and Access cannot accept a user name when looking up column names, so we
			// use the call below that leaves out the user name
			if (!UserID.empty() &&
				Dbms() != dbmsMY_SQL &&
				Dbms() != dbmsACCESS &&
				Dbms() != dbmsMS_SQL_SERVER)
			{
				retcode = SQLColumns(m_hstmt,
					NULL, 0,                              // All qualifiers
					(UCHAR *) UserID.c_str(), SQL_NTS,    // Owner
					(UCHAR *) TableName.c_str(), SQL_NTS,
					NULL, 0);                             // All columns
			}
			else
			{
				retcode = SQLColumns(m_hstmt,
					NULL, 0,                              // All qualifiers
					NULL, 0,                              // Owner
					(UCHAR *) TableName.c_str(), SQL_NTS,
					NULL, 0);                             // All columns
			}
			if (retcode != SQL_SUCCESS)
			{  // Error occurred, abort
				DispAllErrors(m_henv, m_hdbc, m_hstmt);
				if (colInf)
					delete [] colInf;
				SQLFreeStmt(m_hstmt, SQL_CLOSE);
				if (numCols)
					*numCols = 0;
				return(0);
			}

			while ((retcode = SQLFetch(m_hstmt)) == SQL_SUCCESS)
			{
				if (pass == 1)  // First pass, just add up the number of columns
					noCols++;
				else  // Pass 2; Fill in the array of structures
				{
					if (colNo < noCols)  // Some extra error checking to prevent memory overwrites
					{
						// NOTE: Only the ODBC 1.x fields are retrieved
						GetData( 1, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_catalog,      128+1,                     &cb);
						GetData( 2, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_schema,       128+1,                     &cb);
						GetData( 3, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_tableName,    DB_MAX_TABLE_NAME_LEN+1,   &cb);
						GetData( 4, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_colName,      DB_MAX_COLUMN_NAME_LEN+1,  &cb);
						GetData( 5, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_sqlDataType,  0,                         &cb);
						GetData( 6, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_typeName,     128+1,                     &cb);
						GetData( 7, SQL_C_SLONG,  (UCHAR*) &colInf[colNo].m_columnLength, 0,                         &cb);
						GetData( 8, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_bufferSize,   0,                         &cb);
						GetData( 9, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_decimalDigits,0,                         &cb);
						GetData(10, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_numPrecRadix, 0,                         &cb);
						GetData(11, SQL_C_SSHORT, (UCHAR*) &colInf[colNo].m_nullable,     0,                         &cb);
						GetData(12, SQL_C_WXCHAR, (UCHAR*)  colInf[colNo].m_remarks,      254+1,                     &cb);
						// Start Values for Primary/Foriegn Key (=No)
						colInf[colNo].m_pkCol = 0;           // Primary key column   0=No; 1= First Key, 2 = Second Key etc.
						colInf[colNo].m_pkTableName[0] = 0;  // Tablenames where Primary Key is used as a Foreign Key
						colInf[colNo].m_fkCol = 0;           // Foreign key column   0=No; 1= First Key, 2 = Second Key etc.
						colInf[colNo].m_fkTableName[0] = 0;  // Foreign key table name

#ifdef _IODBC_
						// IODBC does not return a correct columnLength, so we set
						// columnLength = bufferSize if no column length was returned
						// IODBC returns the columnLength in bufferSize. (bug)
						if (colInf[colNo].m_columnLength < 1)
						{
							colInf[colNo].m_columnLength = colInf[colNo].m_bufferSize;
						}
#endif

						// Determine the wxDb data type that is used to represent the native data type of this data source
						colInf[colNo].m_dbDataType = 0;
						// Get the intern datatype
						switch (colInf[colNo].m_sqlDataType)
						{
						case SQL_WCHAR:
						case SQL_WVARCHAR:
						case SQL_VARCHAR:
						case SQL_CHAR:
							colInf[colNo].m_dbDataType = DB_DATA_TYPE_VARCHAR;
							break;
						case SQL_LONGVARCHAR:
							colInf[colNo].m_dbDataType = DB_DATA_TYPE_MEMO;
							break;
						case SQL_TINYINT:
						case SQL_SMALLINT:
						case SQL_INTEGER:
						case SQL_BIT:
							colInf[colNo].m_dbDataType = DB_DATA_TYPE_INTEGER;
							break;
						case SQL_DOUBLE:
						case SQL_DECIMAL:
						case SQL_NUMERIC:
						case SQL_FLOAT:
						case SQL_REAL:
							colInf[colNo].m_dbDataType = DB_DATA_TYPE_FLOAT;
							break;
						case SQL_DATE:
						case SQL_TIMESTAMP:
							colInf[colNo].m_dbDataType = DB_DATA_TYPE_DATE;
							break;
						case SQL_BINARY:
							colInf[colNo].m_dbDataType = DB_DATA_TYPE_BLOB;
							break;
#ifdef EXODBCDEBUG
						default:
							std::wstring errMsg;
							errMsg.Printf(L"SQL Data type %d currently not supported by wxWidgets", colInf[colNo].m_sqlDataType);
							BOOST_LOG_TRIVIAL(debug) << L"ODBC DEBUG MESSAGE: " << errMsg;
#endif
						}
						colNo++;
					}
				}
			}
			if (retcode != SQL_NO_DATA_FOUND)
			{  // Error occurred, abort
				DispAllErrors(m_henv, m_hdbc, m_hstmt);
				if (colInf)
					delete [] colInf;
				SQLFreeStmt(m_hstmt, SQL_CLOSE);
				if (numCols)
					*numCols = 0;
				return(0);
			}
		}

		SQLFreeStmt(m_hstmt, SQL_CLOSE);

		// Store Primary and Foreign Keys
		GetKeyFields(tableName,colInf,noCols);

		///////////////////////////////////////////////////////////////////////////
		// Now sort the the columns in order to make them appear in the right order
		///////////////////////////////////////////////////////////////////////////

		// Build a generic SELECT statement which returns 0 rows
		std::wstring Stmt;

		Stmt.Printf(L"select * from \"%s\" where 0=1", tableName);

		// Execute query
		if (SQLExecDirect(m_hstmt, (UCHAR FAR *) Stmt.c_str(), SQL_NTS) != SQL_SUCCESS)
		{
			DispAllErrors(m_henv, m_hdbc, m_hstmt);
			return NULL;
		}

		// Get the number of result columns
		if (SQLNumResultCols (m_hstmt, &noCols) != SQL_SUCCESS)
		{
			DispAllErrors(m_henv, m_hdbc, m_hstmt);
			return NULL;
		}

		if (noCols == 0) // Probably a bogus table name
			return NULL;

		//  Get the name
		int i;
		short colNum;
		UCHAR name[100];
		SWORD Sword;
		SDWORD Sdword;
		for (colNum = 0; colNum < noCols; colNum++)
		{
			if (SQLColAttributes(m_hstmt,colNum+1, SQL_COLUMN_NAME,
				name, sizeof(name),
				&Sword, &Sdword) != SQL_SUCCESS)
			{
				DispAllErrors(m_henv, m_hdbc, m_hstmt);
				return NULL;
			}

			std::wstring Name1 = name;
			Name1 = Name1.Upper();

			// Where is this name in the array ?
			for (i = colNum ; i < noCols ; i++)
			{
				std::wstring Name2 =  colInf[i].m_colName;
				Name2 = Name2.Upper();
				if (Name2 == Name1)
				{
					if (colNum != i) // swap to sort
					{
						ColumnInfo tmpColInf = colInf[colNum];
						colInf[colNum] =  colInf[i];
						colInf[i] = tmpColInf;
					}
					break;
				}
			}
		}
		SQLFreeStmt(m_hstmt, SQL_CLOSE);

		///////////////////////////////////////////////////////////////////////////
		// End sorting
		///////////////////////////////////////////////////////////////////////////

		if (numCols)
			*numCols = noCols;
		return colInf;

	}  // wxDb::GetColumns()


#endif  // #else OLD_GETCOLUMNS


	/********** wxDb::GetColumnCount() **********/
	int Database::GetColumnCount(const std::wstring &tableName, const wchar_t *userID)
		/*
		* Returns a count of how many columns are in a table.
		* If an error occurs in computing the number of columns
		* this function will return a -1 for the count
		*
		* userID is evaluated in the following manner:
		*        userID == NULL  ... UserID is ignored
		*        userID == ""    ... UserID set equal to 'this->uid'
		*        userID != ""    ... UserID set equal to 'userID'
		*
		* NOTE: ALL column bindings associated with this wxDb instance are unbound
		*       by this function.  This function should use its own wxDb instance
		*       to avoid undesired unbinding of columns.
		*/
	{
		UWORD    noCols = 0;

		RETCODE  retcode;

		std::wstring TableName;

		std::wstring UserID = ConvertUserIDImpl(userID);

		TableName = tableName;
		// Oracle and Interbase table names are uppercase only, so force
		// the name to uppercase just in case programmer forgot to do this
		if ((Dbms() == dbmsORACLE) ||
			(Dbms() == dbmsFIREBIRD) ||
			(Dbms() == dbmsINTERBASE))
			boost::algorithm::to_upper(TableName);

		SQLFreeStmt(m_hstmt, SQL_CLOSE);

		// MySQL, SQLServer, and Access cannot accept a user name when looking up column names, so we
		// use the call below that leaves out the user name
		if (!UserID.empty() &&
			Dbms() != dbmsMY_SQL &&
			Dbms() != dbmsACCESS &&
			Dbms() != dbmsMS_SQL_SERVER)
		{
			retcode = SQLColumns(m_hstmt,
				NULL, 0,                                // All qualifiers
				(SQLTCHAR *) UserID.c_str(), SQL_NTS,      // Owner
				(SQLTCHAR *) TableName.c_str(), SQL_NTS,
				NULL, 0);                               // All columns
		}
		else
		{
			retcode = SQLColumns(m_hstmt,
				NULL, 0,                                // All qualifiers
				NULL, 0,                                // Owner
				(SQLTCHAR *) TableName.c_str(), SQL_NTS,
				NULL, 0);                               // All columns
		}
		if (retcode != SQL_SUCCESS)
		{  // Error occurred, abort
			DispAllErrors(m_henv, m_hdbc, m_hstmt);
			SQLFreeStmt(m_hstmt, SQL_CLOSE);
			return(-1);
		}

		// Count the columns
		while ((retcode = SQLFetch(m_hstmt)) == SQL_SUCCESS)
			noCols++;

		if (retcode != SQL_NO_DATA_FOUND)
		{  // Error occurred, abort
			DispAllErrors(m_henv, m_hdbc, m_hstmt);
			SQLFreeStmt(m_hstmt, SQL_CLOSE);
			return(-1);
		}

		SQLFreeStmt(m_hstmt, SQL_CLOSE);
		return noCols;

	}  // wxDb::GetColumnCount()


	/********** wxDb::GetCatalog() *******/
	DbCatalog* Database::GetCatalog(const wchar_t *userID)
		/*
		* ---------------------------------------------------------------------
		* -- 19991203 : mj10777 : Create                                 ------
		* --          : Creates a wxDbInf with Tables / Cols Array       ------
		* --          : uses SQLTables and fills pTableInf;              ------
		* --          : pColInf is set to NULL and numCols to 0;         ------
		* --          : returns pDbInf (wxDbInf)                         ------
		* --            - if unsuccessful (pDbInf == NULL)               ------
		* --          : pColInf can be filled with GetColumns(..);       ------
		* --          : numCols   can be filled with GetColumnCount(..); ------
		* ---------------------------------------------------------------------
		*
		* userID is evaluated in the following manner:
		*        userID == NULL  ... UserID is ignored
		*        userID == ""    ... UserID set equal to 'this->uid'
		*        userID != ""    ... UserID set equal to 'userID'
		*
		* NOTE: ALL column bindings associated with this wxDb instance are unbound
		*       by this function.  This function should use its own wxDb instance
		*       to avoid undesired unbinding of columns.
		*/
	{
		RETCODE  retcode;
		SQLLEN   cb;
		std::wstring tblNameSave;

		std::wstring UserID = ConvertUserIDImpl(userID);

		//-------------------------------------------------------------
		// Create the Database vector of catalog entries

		DbCatalog* pDbInf = new DbCatalog;

		SQLFreeStmt(m_hstmt, SQL_CLOSE);   // Close if Open
		tblNameSave.empty();

		if (!UserID.empty() &&
			Dbms() != dbmsMY_SQL &&
			Dbms() != dbmsACCESS &&
			Dbms() != dbmsMS_SQL_SERVER)
		{
			retcode = SQLTables(m_hstmt,
				NULL, 0,                             // All qualifiers
				(SQLTCHAR *) UserID.c_str(), SQL_NTS,   // User specified
				NULL, 0,                             // All tables
				NULL, 0);                            // All columns
		}
		else
		{
			retcode = SQLTables(m_hstmt,
				NULL, 0,           // All qualifiers - CatalogName
				NULL, 0,           // User specified - SchemaName
				NULL, 0,           // All tables - TableName
				NULL, 0);          // All columns - TableType
		}

		if (retcode != SQL_SUCCESS)
		{
			DispAllErrors(m_henv, m_hdbc, m_hstmt);
			pDbInf = NULL;
			SQLFreeStmt(m_hstmt, SQL_CLOSE);
			return pDbInf;
		}

		// TODO:
		// To determine the actual lengths of the TABLE_CAT, TABLE_SCHEM, and TABLE_NAME columns, 
		// an application can call SQLGetInfo with the SQL_MAX_CATALOG_NAME_LEN, SQL_MAX_SCHEMA_NAME_LEN,
		// and SQL_MAX_TABLE_NAME_LEN information types.
		// see: http://msdn.microsoft.com/en-us/library/ms711831%28v=vs.85%29.aspx

		bool first = true;
		while ((retcode = SQLFetch(m_hstmt)) == SQL_SUCCESS)   // Table Information
		{
			if(first)
			{
				GetData( 1, SQL_C_WXCHAR,   (UCHAR*)  pDbInf->m_catalog,  128+1, &cb);
				GetData( 2, SQL_C_WXCHAR,   (UCHAR*)  pDbInf->m_schema,   128+1, &cb);
				first = false;
			}

			// Create the entry
			DbCatalogTable table;
			GetData( 3, SQL_C_WXCHAR,   (UCHAR*)  table.m_tableName,    DB_MAX_TABLE_NAME_LEN+1, &cb);
			GetData( 4, SQL_C_WXCHAR,   (UCHAR*)  table.m_tableType,    30+1,                    &cb);
			GetData( 5, SQL_C_WXCHAR,   (UCHAR*)  table.m_tableRemarks, 254+1,                   &cb);
			pDbInf->m_tables.push_back(table);
		}  // while
		SQLFreeStmt(m_hstmt, SQL_CLOSE);

		// Query how many columns are in each table
		std::vector<DbCatalogTable>::iterator it;
		for(it = pDbInf->m_tables.begin(); it != pDbInf->m_tables.end(); it++)
		{
			(*it).m_numCols = GetColumnCount((*it).m_tableName, UserID.c_str());
		}

		return pDbInf;

	}  // Database::GetCatalog()


	/********** wxDb::Catalog() **********/
	bool Database::Catalog(const wchar_t *userID, const std::wstring &fileName)
		/*
		* Creates the text file specified in 'filename' which will contain
		* a minimal data dictionary of all tables accessible by the user specified
		* in 'userID'
		*
		* userID is evaluated in the following manner:
		*        userID == NULL  ... UserID is ignored
		*        userID == ""    ... UserID set equal to 'this->uid'
		*        userID != ""    ... UserID set equal to 'userID'
		*
		* NOTE: ALL column bindings associated with this wxDb instance are unbound
		*       by this function.  This function should use its own wxDb instance
		*       to avoid undesired unbinding of columns.
		*/
	{
		exASSERT(fileName.length());

		RETCODE   retcode;
		SQLLEN    cb;
		wchar_t    tblName[DB_MAX_TABLE_NAME_LEN+1];
		std::wstring  tblNameSave;
		wchar_t    colName[DB_MAX_COLUMN_NAME_LEN+1];
		SWORD     sqlDataType;
		wchar_t    typeName[30+1];
		SDWORD    precision, length;

		FILE *fp = _wfopen(fileName.c_str(), L"wt");
		if (fp == NULL)
			return false;

		SQLFreeStmt(m_hstmt, SQL_CLOSE);

		std::wstring UserID = ConvertUserIDImpl(userID);

		if (!UserID.empty() &&
			Dbms() != dbmsMY_SQL &&
			Dbms() != dbmsACCESS &&
			Dbms() != dbmsFIREBIRD &&
			Dbms() != dbmsINTERBASE &&
			Dbms() != dbmsMS_SQL_SERVER)
		{
			retcode = SQLColumns(m_hstmt,
				NULL, 0,                                // All qualifiers
				(SQLTCHAR *) UserID.c_str(), SQL_NTS,      // User specified
				NULL, 0,                                // All tables
				NULL, 0);                               // All columns
		}
		else
		{
			retcode = SQLColumns(m_hstmt,
				NULL, 0,    // All qualifiers
				NULL, 0,    // User specified
				NULL, 0,    // All tables
				NULL, 0);   // All columns
		}
		if (retcode != SQL_SUCCESS)
		{
			DispAllErrors(m_henv, m_hdbc, m_hstmt);
			fclose(fp);
			return false;
		}

		std::wstring outStr;
		tblNameSave.empty();
		int cnt = 0;

		while (true)
		{
			retcode = SQLFetch(m_hstmt);
			if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
				break;

			GetData(3,SQL_C_WXCHAR,  (UCHAR *) tblName,     DB_MAX_TABLE_NAME_LEN+1, &cb);
			GetData(4,SQL_C_WXCHAR,  (UCHAR *) colName,     DB_MAX_COLUMN_NAME_LEN+1,&cb);
			GetData(5,SQL_C_SSHORT,  (UCHAR *)&sqlDataType, 0,                       &cb);
			GetData(6,SQL_C_WXCHAR,  (UCHAR *) typeName,    sizeof(typeName),        &cb);
			GetData(7,SQL_C_SLONG,   (UCHAR *)&precision,   0,                       &cb);
			GetData(8,SQL_C_SLONG,   (UCHAR *)&length,      0,                       &cb);

			if (wcscmp(tblName, tblNameSave.c_str()))
			{
				if (cnt)
					fputws(L"\n", fp);
				fputws(L"================================ ", fp);
				fputws(L"================================ ", fp);
				fputws(L"===================== ", fp);
				fputws(L"========= ", fp);
				fputws(L"=========\n", fp);
				outStr = (boost::wformat(L"%-32s %-32s %-21s %9s %9s\n") % L"TABLE NAME" % L"COLUMN NAME" % L"DATA TYPE" % L"PRECISION" % L"LENGTH").str();
				fputws(outStr.c_str(), fp);
				fputws(L"================================ ", fp);
				fputws(L"================================ ", fp);
				fputws(L"===================== ", fp);
				fputws(L"========= ", fp);
				fputws(L"=========\n", fp);
				tblNameSave = tblName;
			}

			outStr = (boost::wformat(L"%-32s %-32s (%04d)%-15s %9ld %9ld\n") % tblName % colName % sqlDataType % typeName % precision % length).str();
			if (fputws(outStr.c_str(), fp) == EOF)
			{
				SQLFreeStmt(m_hstmt, SQL_CLOSE);
				fclose(fp);
				return false;
			}
			cnt++;
		}

		if (retcode != SQL_NO_DATA_FOUND)
			DispAllErrors(m_henv, m_hdbc, m_hstmt);

		SQLFreeStmt(m_hstmt, SQL_CLOSE);

		fclose(fp);
		return(retcode == SQL_NO_DATA_FOUND);

	}  // wxDb::Catalog()


	bool Database::TableExists(const std::wstring &tableName, const wchar_t *userID, const std::wstring &tablePath)
		/*
		* Table name can refer to a table, view, alias or synonym.  Returns true
		* if the object exists in the database.  This function does not indicate
		* whether or not the user has privleges to query or perform other functions
		* on the table.
		*
		* userID is evaluated in the following manner:
		*        userID == NULL  ... UserID is ignored
		*        userID == ""    ... UserID set equal to 'this->uid'
		*        userID != ""    ... UserID set equal to 'userID'
		*/
	{
		exASSERT(tableName.length());

		std::wstring TableName;

		if (Dbms() == dbmsDBASE)
		{
			exFAIL_MSG(L"dbmsDBASE support is not enabled");
			//     std::wstring dbName;
			//     if (tablePath.length())
			//dbName = (boost::wformat(L"%s/%s.dbf") % tablePath % tableName).str();
			//     else
			//dbName = (boost::wformat(L"%s.dbf") % tableName).str();

			//     bool exists;
			//     exists = wxFileExists(dbName);
			//     return exists;
		}

		std::wstring UserID = ConvertUserIDImpl(userID);

		TableName = tableName;
		// Oracle and Interbase table names are uppercase only, so force
		// the name to uppercase just in case programmer forgot to do this
		if ((Dbms() == dbmsORACLE) ||
			(Dbms() == dbmsFIREBIRD) ||
			(Dbms() == dbmsINTERBASE))
			boost::algorithm::to_upper(TableName);

		SQLFreeStmt(m_hstmt, SQL_CLOSE);
		RETCODE retcode;

		// Some databases cannot accept a user name when looking up table names,
		// so we use the call below that leaves out the user name
		if (!UserID.empty() &&
			Dbms() != dbmsMY_SQL &&
			Dbms() != dbmsACCESS &&
			Dbms() != dbmsMS_SQL_SERVER &&
			Dbms() != dbmsDB2 &&
			Dbms() != dbmsFIREBIRD &&
			Dbms() != dbmsINTERBASE &&
			Dbms() != dbmsPERVASIVE_SQL)
		{
			retcode = SQLTables(m_hstmt,
				NULL, 0,                                  // All qualifiers
				(SQLTCHAR *) UserID.c_str(), SQL_NTS,        // Only tables owned by this user
				(SQLTCHAR FAR *)TableName.c_str(), SQL_NTS,
				NULL, 0);                                 // All table types
		}
		else
		{
			retcode = SQLTables(m_hstmt,
				NULL, 0,                                  // All qualifiers
				NULL, 0,                                  // All owners
				(SQLTCHAR FAR *)TableName.c_str(), SQL_NTS,
				NULL, 0);                                 // All table types
		}
		if (retcode != SQL_SUCCESS)
			return(DispAllErrors(m_henv, m_hdbc, m_hstmt));

		retcode = SQLFetch(m_hstmt);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
		{
			SQLFreeStmt(m_hstmt, SQL_CLOSE);
			return(DispAllErrors(m_henv, m_hdbc, m_hstmt));
		}

		SQLFreeStmt(m_hstmt, SQL_CLOSE);

		return true;

	}  // wxDb::TableExists()


	/********** wxDb::TablePrivileges() **********/
	bool Database::TablePrivileges(const std::wstring &tableName, const std::wstring &priv, const wchar_t *userID,
		const wchar_t *schema, const std::wstring& tablePath)
	{
		exASSERT(tableName.length());

		wxDbTablePrivilegeInfo  result;
		SQLLEN  cbRetVal;
		RETCODE retcode;

		// We probably need to be able to dynamically set this based on
		// the driver type, and state.
		wchar_t curRole[] = L"public";

		std::wstring TableName;

		std::wstring UserID = ConvertUserIDImpl(userID);
		std::wstring Schema = ConvertUserIDImpl(schema);

		TableName = tableName;
		// Oracle and Interbase table names are uppercase only, so force
		// the name to uppercase just in case programmer forgot to do this
		if ((Dbms() == dbmsORACLE) ||
			(Dbms() == dbmsFIREBIRD) ||
			(Dbms() == dbmsINTERBASE))
			boost::algorithm::to_upper(TableName);

		SQLFreeStmt(m_hstmt, SQL_CLOSE);

		// Some databases cannot accept a user name when looking up table names,
		// so we use the call below that leaves out the user name
		if (!Schema.empty() &&
			Dbms() != dbmsMY_SQL &&
			Dbms() != dbmsACCESS &&
			Dbms() != dbmsMS_SQL_SERVER)
		{
			retcode = SQLTablePrivileges(m_hstmt,
				NULL, 0,                                    // Catalog
				(SQLTCHAR FAR *)Schema.c_str(), SQL_NTS,               // Schema
				(SQLTCHAR FAR *)TableName.c_str(), SQL_NTS);
		}
		else
		{
			retcode = SQLTablePrivileges(m_hstmt,
				NULL, 0,                                    // Catalog
				NULL, 0,                                    // Schema
				(SQLTCHAR FAR *)TableName.c_str(), SQL_NTS);
		}

#ifdef DBDEBUG_CONSOLE
		std::wcerr << L"SQLTablePrivileges() returned " << retcode << std::endl;
#endif

		if ((retcode != SQL_SUCCESS) && (retcode != SQL_SUCCESS_WITH_INFO))
			return (DispAllErrors(m_henv, m_hdbc, m_hstmt));

		bool failed = false;
		retcode = SQLFetch(m_hstmt);
		while (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
		{
			if (SQLGetData(m_hstmt, 1, SQL_C_WXCHAR, (UCHAR*) result.tableQual, sizeof(result.tableQual), &cbRetVal) != SQL_SUCCESS)
				failed = true;

			if (!failed && SQLGetData(m_hstmt, 2, SQL_C_WXCHAR, (UCHAR*) result.tableOwner, sizeof(result.tableOwner), &cbRetVal) != SQL_SUCCESS)
				failed = true;

			if (!failed && SQLGetData(m_hstmt, 3, SQL_C_WXCHAR, (UCHAR*) result.tableName, sizeof(result.tableName), &cbRetVal) != SQL_SUCCESS)
				failed = true;

			if (!failed && SQLGetData(m_hstmt, 4, SQL_C_WXCHAR, (UCHAR*) result.grantor, sizeof(result.grantor), &cbRetVal) != SQL_SUCCESS)
				failed = true;

			if (!failed && SQLGetData(m_hstmt, 5, SQL_C_WXCHAR, (UCHAR*) result.grantee, sizeof(result.grantee), &cbRetVal) != SQL_SUCCESS)
				failed = true;

			if (!failed && SQLGetData(m_hstmt, 6, SQL_C_WXCHAR, (UCHAR*) result.privilege, sizeof(result.privilege), &cbRetVal) != SQL_SUCCESS)
				failed = true;

			if (!failed && SQLGetData(m_hstmt, 7, SQL_C_WXCHAR, (UCHAR*) result.grantable, sizeof(result.grantable), &cbRetVal) != SQL_SUCCESS)
				failed = true;

			if (failed)
			{
				return(DispAllErrors(m_henv, m_hdbc, m_hstmt));
			}
#ifdef DBDEBUG_CONSOLE
			std::wcerr << "Scanning " << result.privilege <<" privilege on table " << result.tableOwner << "." << result.tableName << " granted by " << result.grantor << " to " << result.grantee << std::endl;
#endif

			if(boost::algorithm::iequals(UserID, result.tableOwner))
			{
				SQLFreeStmt(m_hstmt, SQL_CLOSE);
				return true;
			}

			if(boost::algorithm::iequals(UserID, result.grantee) &&
				!wcscmp(result.privilege, priv.c_str()))
			{
				SQLFreeStmt(m_hstmt, SQL_CLOSE);
				return true;
			}

			if (!wcscmp(result.grantee,curRole) &&
				!wcscmp(result.privilege, priv.c_str()))
			{
				SQLFreeStmt(m_hstmt, SQL_CLOSE);
				return true;
			}

			retcode = SQLFetch(m_hstmt);
		}

		SQLFreeStmt(m_hstmt, SQL_CLOSE);
		return false;

	}  // wxDb::TablePrivileges


	const std::wstring Database::SQLTableName(const wchar_t *tableName)
	{
		std::wstring TableName;

		if (Dbms() == dbmsACCESS)
			TableName = L"\"";
		TableName += tableName;
		if (Dbms() == dbmsACCESS)
			TableName += L"\"";

		return TableName;
	}  // wxDb::SQLTableName()


	const std::wstring Database::SQLColumnName(const wchar_t *colName)
	{
		std::wstring ColName;

		if (Dbms() == dbmsACCESS)
			ColName = L"\"";
		ColName += colName;
		if (Dbms() == dbmsACCESS)
			ColName += L"\"";

		return ColName;
	}  // wxDb::SQLColumnName()


	/********** wxDb::SetSqlLogging() **********/
	bool Database::SetSqlLogging(wxDbSqlLogState state, const std::wstring &filename, bool append)
	{
		exASSERT(state == sqlLogON  || state == sqlLogOFF);
		exASSERT(state == sqlLogOFF || filename.length());

		if (state == sqlLogON)
		{
			if (m_fpSqlLog == 0)
			{
				m_fpSqlLog = _wfopen(filename.c_str(), (append ? L"at" : L"wt"));
				if (m_fpSqlLog == NULL)
					return false;
			}
		}
		else  // sqlLogOFF
		{
			if (m_fpSqlLog)
			{
				if (fclose(m_fpSqlLog))
					return false;
				m_fpSqlLog = 0;
			}
		}

		m_sqlLogState = state;
		return true;

	}  // wxDb::SetSqlLogging()


	/********** wxDb::WriteSqlLog() **********/
	bool Database::WriteSqlLog(const std::wstring &logMsg)
	{
		exASSERT(logMsg.length());

		if (m_fpSqlLog == 0 || m_sqlLogState == sqlLogOFF)
			return false;

		if (fputws(L"\n", m_fpSqlLog) == EOF)
			return false;
		if (fputws(logMsg.c_str(), m_fpSqlLog) == EOF)
			return false;
		if (fputws(L"\n", m_fpSqlLog) == EOF)
			return false;



		return true;

	}  // wxDb::WriteSqlLog()


	std::vector<std::wstring> Database::GetErrorList() const
	{
		std::vector<std::wstring> list;

		for (int i = 0; i < DB_MAX_ERROR_HISTORY; i++)
		{
			if (errorList[i])
			{
				list.push_back(std::wstring(errorList[i]));
			}
		}
		return list;
	}


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
		std::wcout << "Database connecting to: " << dbInf.dbmsName << std::endl;
#endif  // DBDEBUG_CONSOLE

		BOOST_LOG_TRIVIAL(debug) << L"Database connecting to: " << dbInf.dbmsName;

		wchar_t baseName[25+1];
		wcsncpy(baseName, dbInf.dbmsName, 25);
		baseName[25] = 0;

		// RGG 20001025 : add support for Interbase
		// GT : Integrated to base classes on 20001121
		if (!_wcsicmp(dbInf.dbmsName, L"Interbase"))
			return((wxDBMS)(m_dbmsType = dbmsINTERBASE));

		// BJO 20000428 : add support for Virtuoso
		if (!_wcsicmp(dbInf.dbmsName, L"OpenLink Virtuoso VDBMS"))
			return((wxDBMS)(m_dbmsType = dbmsVIRTUOSO));

		if (!_wcsicmp(dbInf.dbmsName, L"Adaptive Server Anywhere"))
			return((wxDBMS)(m_dbmsType = dbmsSYBASE_ASA));

		// BJO 20000427 : The "SQL Server" string is also returned by SQLServer when
		// connected through an OpenLink driver.
		// Is it also returned by Sybase Adapatitve server?
		// OpenLink driver name is OLOD3032.DLL for msw and oplodbc.so for unix
		if (!_wcsicmp(dbInf.dbmsName, L"SQL Server"))
		{
			if (!wcsncmp(dbInf.driverName, L"oplodbc", 7) ||
				!wcsncmp(dbInf.driverName, L"OLOD", 4))
				return ((wxDBMS)(dbmsMS_SQL_SERVER));
			else
				return ((wxDBMS)(m_dbmsType = dbmsSYBASE_ASE));
		}

		if (!_wcsicmp(dbInf.dbmsName, L"Microsoft SQL Server"))
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


	bool Database::ModifyColumn(const std::wstring &tableName, const std::wstring &columnName,
		int dataType, ULONG columnLength,
		const std::wstring &optionalParam)
	{
		exASSERT(tableName.length());
		exASSERT(columnName.length());
		exASSERT((dataType == DB_DATA_TYPE_VARCHAR && columnLength > 0) ||
			dataType != DB_DATA_TYPE_VARCHAR);

		// Must specify a columnLength if modifying a VARCHAR type column
		if (dataType == DB_DATA_TYPE_VARCHAR && !columnLength)
			return false;

		std::wstring dataTypeName;
		std::wstring sqlStmt;
		std::wstring alterSlashModify;

		switch(dataType)
		{
		case DB_DATA_TYPE_VARCHAR :
			dataTypeName = m_typeInfVarchar.TypeName;
			break;
		case DB_DATA_TYPE_INTEGER :
			dataTypeName = m_typeInfInteger.TypeName;
			break;
		case DB_DATA_TYPE_FLOAT :
			dataTypeName = m_typeInfFloat.TypeName;
			break;
		case DB_DATA_TYPE_DATE :
			dataTypeName = m_typeInfDate.TypeName;
			break;
		case DB_DATA_TYPE_BLOB :
			dataTypeName = m_typeInfBlob.TypeName;
			break;
		default:
			return false;
		}

		// Set the modify or alter syntax depending on the type of database connected to
		switch (Dbms())
		{
		case dbmsORACLE :
			alterSlashModify = L"MODIFY";
			break;
		case dbmsMS_SQL_SERVER :
			alterSlashModify = L"ALTER COLUMN";
			break;
		case dbmsUNIDENTIFIED :
			return false;
		case dbmsSYBASE_ASA :
		case dbmsSYBASE_ASE :
		case dbmsMY_SQL :
		case dbmsPOSTGRES :
		case dbmsACCESS :
		case dbmsDBASE :
		case dbmsXBASE_SEQUITER :
		default :
			alterSlashModify = L"MODIFY";
			break;
		}

		// create the SQL statement
		if ( Dbms() == dbmsMY_SQL )
		{
			sqlStmt = (boost::wformat(L"ALTER TABLE %s %s %s %s") % tableName % alterSlashModify % columnName % dataTypeName).str();
		}
		else
		{
			sqlStmt = (boost::wformat(L"ALTER TABLE \"%s\" \"%s\" \"%s\" %s") % tableName % alterSlashModify % columnName % dataTypeName).str();
		}

		// For varchars only, append the size of the column
		if (dataType == DB_DATA_TYPE_VARCHAR &&
			(Dbms() != dbmsMY_SQL || dataTypeName != L"text"))
		{
			std::wstring s;
			s = (boost::wformat(L"(%lu)") % columnLength).str();
			sqlStmt += s;
		}

		// for passing things like "NOT NULL"
		if (optionalParam.length())
		{
			sqlStmt += L" ";
			sqlStmt += optionalParam;
		}

		return ExecSql(sqlStmt);

	} // wxDb::ModifyColumn()

	/********** wxDb::EscapeSqlChars() **********/
	std::wstring Database::EscapeSqlChars(const std::wstring& valueOrig)
	{
		std::wstring value(valueOrig);
		switch (Dbms())
		{
		case dbmsACCESS:
			// Access doesn't seem to care about backslashes, so only escape single quotes.
			boost::algorithm::replace_all(value, L"'", L"''");
			break;

		default:
			// All the others are supposed to be the same for now, add special
			// handling for them if necessary
			boost::algorithm::replace_all(value, L"\\", L"\\\\");
			boost::algorithm::replace_all(value, L"'", L"\\'");
			break;
		}

		return value;
	} // wxDb::EscapeSqlChars()


	/********** wxDbGetConnection() **********/
	Database EXODBCAPI *wxDbGetConnection(DbEnvironment *pDbConfig, bool FwdOnlyCursors)
	{
		SDbList *pList;

		// Used to keep a pointer to a DB connection that matches the requested
		// DSN and FwdOnlyCursors settings, even if it is not FREE, so that the
		// data types can be copied from it (using the wxDb::Open(wxDb *) function)
		// rather than having to re-query the datasource to get all the values
		// using the wxDb::Open(Dsn,Uid,AuthStr) function
		Database *matchingDbConnection = NULL;

		// Scan the linked list searching for an available database connection
		// that's already been OpenImpled but is currently not in use.
		for (pList = PtrBegDbList; pList; pList = pList->PtrNext)
		{
			// The database connection must be for the same datasource
			// name and must currently not be in use.
			if (pList->Free &&
				(pList->PtrDb->FwdOnlyCursors() == FwdOnlyCursors))
			{
				if (pDbConfig->UseConnectionStr())
				{
					if (pList->PtrDb->OpenedWithConnectionString() &&
						(!wcscmp(pDbConfig->GetConnectionStr(), pList->ConnectionStr.c_str())))
					{
						// Found a free connection
						pList->Free = false;
						return(pList->PtrDb);
					}
				}
				else
				{
					if (!pList->PtrDb->OpenedWithConnectionString() &&
						(!wcscmp(pDbConfig->GetDsn(), pList->Dsn.c_str())))
					{
						// Found a free connection
						pList->Free = false;
						return(pList->PtrDb);
					}
				}
			}

			if (pDbConfig->UseConnectionStr())
			{
				if (!wcscmp(pDbConfig->GetConnectionStr(), pList->ConnectionStr.c_str()))
					matchingDbConnection = pList->PtrDb;
			}
			else
			{
				if (!wcscmp(pDbConfig->GetDsn(), pList->Dsn.c_str()) &&
					!wcscmp(pDbConfig->GetUserID(), pList->Uid.c_str()) &&
					!wcscmp(pDbConfig->GetPassword(), pList->AuthStr.c_str()))
					matchingDbConnection = pList->PtrDb;
			}
		}

		// No available connections.  A new connection must be made and
		// appended to the end of the linked list.
		if (PtrBegDbList)
		{
			// Find the end of the list
			for (pList = PtrBegDbList; pList->PtrNext; pList = pList->PtrNext);
			// Append a new list item
			pList->PtrNext = new SDbList;
			pList->PtrNext->PtrPrev = pList;
			pList = pList->PtrNext;
		}
		else  // Empty list
		{
			// Create the first node on the list
			pList = PtrBegDbList = new SDbList;
			pList->PtrPrev = 0;
		}

		// Initialize new node in the linked list
		pList->PtrNext          = 0;
		pList->Free             = false;
		pList->Dsn              = pDbConfig->GetDsn();
		pList->Uid              = pDbConfig->GetUserID();
		pList->AuthStr          = pDbConfig->GetPassword();
		pList->ConnectionStr    = pDbConfig->GetConnectionStr();

		pList->PtrDb = new Database(pDbConfig->GetHenv(), FwdOnlyCursors);

		bool OpenImpled;

		if (!matchingDbConnection)
		{
			if (pDbConfig->UseConnectionStr())
			{
				OpenImpled = pList->PtrDb->Open(pDbConfig->GetConnectionStr());
			}
			else
			{
				OpenImpled = pList->PtrDb->Open(pDbConfig->GetDsn(), pDbConfig->GetUserID(), pDbConfig->GetPassword());
			}
		}
		else
			OpenImpled = pList->PtrDb->Open(matchingDbConnection);

		// Connect to the datasource
		if (OpenImpled)
		{
			pList->PtrDb->setCached(true);  // Prevent a user from deleting a cached connection
			pList->PtrDb->SetSqlLogging(SQLLOGstate, SQLLOGfn, true);
			return(pList->PtrDb);
		}
		else  // Unable to connect, destroy list item
		{
			if (pList->PtrPrev)
				pList->PtrPrev->PtrNext = 0;
			else
				PtrBegDbList = 0;        // Empty list again

			pList->PtrDb->CommitTrans(); // Commit any OpenImpl transactions on wxDb object
			pList->PtrDb->Close();       // Close the wxDb object
			delete pList->PtrDb;         // Deletes the wxDb object
			delete pList;                // Deletes the linked list object
			return(0);
		}

	}  // wxDbGetConnection()


	/********** wxDbFreeConnection() **********/
	bool EXODBCAPI wxDbFreeConnection(Database *pDb)
	{
		SDbList *pList;

		// Scan the linked list searching for the database connection
		for (pList = PtrBegDbList; pList; pList = pList->PtrNext)
		{
			if (pList->PtrDb == pDb)  // Found it, now free it!!!
				return (pList->Free = true);
		}

		// Never found the database object, return failure
		return false;

	}  // wxDbFreeConnection()


	/********** wxDbCloseConnections() **********/
	void EXODBCAPI wxDbCloseConnections()
	{
		SDbList *pList, *pNext;

		// Traverse the linked list closing database connections and freeing memory as I go.
		for (pList = PtrBegDbList; pList; pList = pNext)
		{
			pNext = pList->PtrNext;       // Save the pointer to next
			pList->PtrDb->CommitTrans();  // Commit any OpenImpl transactions on wxDb object
			pList->PtrDb->Close();        // Close the wxDb object
			pList->PtrDb->setCached(false);  // Allows deletion of the wxDb instance
			delete pList->PtrDb;          // Deletes the wxDb object
			delete pList;                 // Deletes the linked list object
		}

		// Mark the list as empty
		PtrBegDbList = 0;

	}  // wxDbCloseConnections()


	/********** wxDbConnectionsInUse() **********/
	int EXODBCAPI wxDbConnectionsInUse()
	{
		SDbList *pList;
		int cnt = 0;

		// Scan the linked list counting db connections that are currently in use
		for (pList = PtrBegDbList; pList; pList = pList->PtrNext)
		{
			if (pList->Free == false)
				cnt++;
		}

		return(cnt);

	}  // wxDbConnectionsInUse()



	/********** wxDbLogExtendedErrorMsg() **********/
	// DEBUG ONLY function
	const wchar_t EXODBCAPI *wxDbLogExtendedErrorMsg(const wchar_t *userText,
		Database *pDb,
		const wchar_t *ErrFile,
		int ErrLine)
	{
		static std::wstring msg;
		msg = userText;

		std::wstring tStr;

		if (ErrFile || ErrLine)
		{
			msg += L"File: ";
			msg += ErrFile;
			msg += L"   Line: ";
			tStr = (boost::wformat(L"%d") % ErrLine).str();
			msg += tStr.c_str();
			msg += L"\n";
		}

		msg.append (L"\nODBC errors:\n");
		msg += L"\n";

		// Display errors for this connection
		int i;
		for (i = 0; i < DB_MAX_ERROR_HISTORY; i++)
		{
			if (pDb->errorList[i])
			{
				msg.append(pDb->errorList[i]);
				if (wcscmp(pDb->errorList[i], emptyString) != 0)
					msg.append(L"\n");
				// Clear the errmsg buffer so the next error will not
				// end up showing the previous error that have occurred
				wcscpy(pDb->errorList[i], emptyString);
			}
		}
		msg += L"\n";

		BOOST_LOG_TRIVIAL(debug) << msg;

		return msg.c_str();
	}  // wxDbLogExtendedErrorMsg()


	/********** wxDbSqlLog() **********/
	bool wxDbSqlLog(wxDbSqlLogState state, const wchar_t *filename)
	{
		bool append = false;
		SDbList *pList;

		for (pList = PtrBegDbList; pList; pList = pList->PtrNext)
		{
			if (!pList->PtrDb->SetSqlLogging(state,filename,append))
				return false;
			append = true;
		}

		SQLLOGstate = state;
		SQLLOGfn = filename;

		return true;

	}  // wxDbSqlLog()


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