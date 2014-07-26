///////////////////////////////////////////////////////////////////////////////
// Name:        wx/db.h
// Purpose:     Header file wxDb class.  The wxDb class represents a connection
//              to an ODBC data source.  The wxDb class allows operations on the data
//              source such as opening and closing the data source.
// Author:      Doug Card
// Modified by: George Tasker
//              Bart Jourquin
//              Mark Johnson, wxWindows@mj10777.de
//				Elias Gerber, eg@zame.ch
// Mods:        Dec, 1998:
//                -Added support for SQL statement logging and database cataloging
//                     April, 1999
//                -Added QUERY_ONLY mode support to reduce default number of cursors
//                -Added additional SQL logging code
//                -Added DEBUG-ONLY tracking of Ctable objects to detect orphaned DB connections
//                -Set ODBC option to only read committed writes to the DB so all
//                     databases operate the same in that respect
//
// Created:     9.96
// RCS-ID:      $Id: db.h 56697 2008-11-07 22:45:47Z VZ $
// Copyright:   (c) 1996 Remstar International, Inc.
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef DB_H
#define DB_H

#include "wxOdbc3.h"

#include <vector>

// BJO 20000503: introduce new GetColumns members which are more database independent and
//               return columns in the order they were created
#define OLD_GETCOLUMNS 1
#define EXPERIMENTAL_WXDB_FUNCTIONS 1

#include <windows.h>

#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#include <odbcinst.h>

// This was defined by a macro testing if wxUNICODE is set
#define SQL_C_WXCHAR SQL_C_WCHAR

typedef float SFLOAT;
typedef double SDOUBLE;
typedef unsigned int UINT;
#define ULONG UDWORD

#ifndef wxODBC_FWD_ONLY_CURSORS
#define wxODBC_FWD_ONLY_CURSORS 1
#endif

enum enumDummy {enumDum1};

#ifndef SQL_C_BOOLEAN
#define SQL_C_BOOLEAN(datatype) (sizeof(datatype) == 1 ? SQL_C_UTINYINT : (sizeof(datatype) == 2 ? SQL_C_USHORT : SQL_C_ULONG))
#endif

#ifndef SQL_C_ENUM
#define SQL_C_ENUM (sizeof(enumDummy) == 2 ? SQL_C_USHORT : SQL_C_ULONG)
#endif

// NOTE: If SQL_C_BLOB is defined, and it is not SQL_C_BINARY, iODBC 2.x
//       may not function correctly.  Likely best to use SQL_C_BINARY direct
#ifndef SQL_C_BLOB
#ifdef SQL_C_BINARY
#define SQL_C_BLOB SQL_C_BINARY
#endif
#endif

#ifndef _WIN64
#ifndef SQLLEN
#define SQLLEN SQLINTEGER
#endif
#ifndef SQLULEN
#define SQLULEN SQLUINTEGER
#endif
#endif


namespace exodbc
{
	// Forward declarations
	// --------------------
	class DbEnvironment;
	class ColumnFormatter;

	// Structs
	// ------
	struct WXDLLIMPEXP_ODBC SqlTypeInfo
	{
		std::wstring	TypeName;
		SWORD			FsqlType;
		long			Precision;
		short			CaseSensitive;
		short			MaximumScale;
	};

	// Classes
	// -------
	class WXDLLIMPEXP_ODBC ColumnInfo
	{
	public:
		ColumnInfo();
		~ColumnInfo();

		bool Initialize();

		wchar_t			m_catalog[128+1];
		wchar_t			m_schema[128+1];
		wchar_t			m_tableName[DB_MAX_TABLE_NAME_LEN+1];
		wchar_t			m_colName[DB_MAX_COLUMN_NAME_LEN+1];
		SWORD			m_sqlDataType;
		wchar_t			m_typeName[128+1];
		SWORD			m_columnLength;
		SWORD			m_bufferSize;
		short			m_decimalDigits;
		short			m_numPrecRadix;
		short			m_nullable;
		wchar_t			m_remarks[254+1];
		int				m_dbDataType;  // conversion of the 'sqlDataType' to the generic data type used by these classes
		// mj10777.19991224 : new
		int				m_pkCol;       // Primary key column       0=No; 1= First Key, 2 = Second Key etc.
		wchar_t			m_pkTableName[DB_MAX_TABLE_NAME_LEN+1]; // Tables that use this PKey as a FKey
		int				m_fkCol;       // Foreign key column       0=No; 1= First Key, 2 = Second Key etc.
		wchar_t			m_fkTableName[DB_MAX_TABLE_NAME_LEN+1]; // Foreign key table name
		ColumnFormatter* m_pColFor;                              // How should this columns be formatted

	};


	class WXDLLIMPEXP_ODBC DbCatalogTable        // Description of a Table: Used only in the Description of a database, (catalog info)
	{
	public:
		DbCatalogTable();
		~DbCatalogTable();

		bool             Initialize();

		wchar_t		m_tableName[DB_MAX_TABLE_NAME_LEN+1];
		wchar_t		m_tableType[254+1];           // "TABLE" or "SYSTEM TABLE" etc.
		wchar_t		m_tableRemarks[254+1];
		UWORD		m_numCols;                    // How many Columns does this Table have: GetColumnCount(..);
		ColumnInfo*	m_pColInf;                    // pColInf = NULL ; User can later call GetColumns(..);
	};


	class WXDLLIMPEXP_ODBC DbCatalog     // Description of a Database: Used so far only when fetching the "catalog"
	{
	public:
		DbCatalog();
		~DbCatalog();

		bool          Initialize();

		wchar_t			m_catalog[128+1];
		wchar_t			m_schema[128+1];
		int				m_numTables;           // How many tables does this database have
		DbCatalogTable*	m_pTableInf;           // pTableInf = new wxDbTableInf[numTables];
	};

	// The wxDb::errorList is copied to this variable when the wxDb object
	// is closed.  This way, the error list is still available after the
	// database object is closed.  This is necessary if the database
	// connection fails so the calling application can show the operator
	// why the connection failed.  Note: as each wxDb object is closed, it
	// will overwrite the errors of the previously destroyed wxDb object in
	// this variable.

	extern WXDLLIMPEXP_DATA_ODBC(wchar_t)
		DBerrorList[DB_MAX_ERROR_HISTORY][DB_MAX_ERROR_MSG_LEN+1];


	class WXDLLIMPEXP_ODBC Database
	{
		friend class wxDbTable;
	private:
		bool             dbIsOpen;
		bool             dbIsCached;      // Was connection created by caching functions
		bool             dbOpenedWithConnectionString;  // Was the database connection opened with a connection string
		std::wstring         dsn;             // Data source name
		std::wstring         uid;             // User ID
		std::wstring         authStr;         // Authorization string (password)
		std::wstring         inConnectionStr; // Connection string used to connect to the database
		std::wstring         outConnectionStr;// Connection string returned by the database when a connection is successfully opened
		FILE            *fpSqlLog;        // Sql Log file pointer
		wxDbSqlLogState  sqlLogState;     // On or Off
		bool             fwdOnlyCursors;
		wxDBMS           dbmsType;        // Type of datasource - i.e. Oracle, dBase, SQLServer, etc

		// Private member functions
		bool             getDbInfo(bool failOnDataTypeUnsupported=true);
		bool             getDataTypeInfo(SWORD fSqlType, SqlTypeInfo &structSQLTypeInfo);
		bool             setConnectionOptions(void);
		void             logError(const std::wstring &errMsg, const std::wstring &SQLState);
		const wchar_t    *convertUserID(const wchar_t *userID, std::wstring &UserID);
		bool             determineDataTypes(bool failOnDataTypeUnsupported);
		void             initialize();
		bool             open(bool failOnDataTypeUnsupported=true);

		// ODBC handles
		HENV  henv;        // ODBC Environment handle
		HDBC  hdbc;        // ODBC DB Connection handle
		HSTMT hstmt;       // ODBC Statement handle

		//Error reporting mode
		bool silent;

		// Number of Ctable objects connected to this db object.  FOR INTERNAL USE ONLY!!!
		unsigned int nTables;

		// Information about logical data types VARCHAR, INTEGER, FLOAT and DATE.
		//
		// This information is obtained from the ODBC driver by use of the
		// SQLGetTypeInfo() function.  The key piece of information is the
		// type name the data source uses for each logical data type.
		// e.g. VARCHAR; Oracle calls it VARCHAR2.
		SqlTypeInfo typeInfVarchar;
		SqlTypeInfo typeInfInteger;
		SqlTypeInfo typeInfFloat;
		SqlTypeInfo typeInfDate;
		SqlTypeInfo typeInfBlob;
		SqlTypeInfo typeInfMemo;

	public:

		void             setCached(bool cached)  { dbIsCached = cached; }  // This function must only be called by wxDbGetConnection() and wxDbCloseConnections!!!
		bool             IsCached() { return dbIsCached; }

		bool             GetDataTypeInfo(SWORD fSqlType, SqlTypeInfo &structSQLTypeInfo)
		{ return getDataTypeInfo(fSqlType, structSQLTypeInfo); }


		// The following structure contains database information gathered from the
		// datasource when the datasource is first opened.
		struct
		{
			wchar_t dbmsName[40];                             // Name of the dbms product
			wchar_t dbmsVer[64];                              // Version # of the dbms product
			wchar_t driverName[40];                           // Driver name
			wchar_t odbcVer[60];                              // ODBC version of the driver
			wchar_t drvMgrOdbcVer[60];                        // ODBC version of the driver manager
			wchar_t driverVer[60];                            // Driver version
			wchar_t serverName[80];                           // Server Name, typically a connect string
			wchar_t databaseName[128];                        // Database filename
			wchar_t outerJoins[2];                            // Indicates whether the data source supports outer joins
			wchar_t procedureSupport[2];                      // Indicates whether the data source supports stored procedures
			wchar_t accessibleTables[2];                      // Indicates whether the data source only reports accessible tables in SQLTables.
			UWORD  maxConnections;                           // Maximum # of connections the data source supports
			UWORD  maxStmts;                                 // Maximum # of HSTMTs per HDBC
			UWORD  apiConfLvl;                               // ODBC API conformance level
			UWORD  cliConfLvl;                               // Indicates whether the data source is SAG compliant
			UWORD  sqlConfLvl;                               // SQL conformance level
			UWORD  cursorCommitBehavior;                     // Indicates how cursors are affected by a db commit
			UWORD  cursorRollbackBehavior;                   // Indicates how cursors are affected by a db rollback
			UWORD  supportNotNullClause;                     // Indicates if data source supports NOT NULL clause
			wchar_t supportIEF[2];                            // Integrity Enhancement Facility (Referential Integrity)
			UDWORD txnIsolation;                             // Default transaction isolation level supported by the driver
			UDWORD txnIsolationOptions;                      // Transaction isolation level options available
			UDWORD fetchDirections;                          // Fetch directions supported
			UDWORD lockTypes;                                // Lock types supported in SQLSetPos
			UDWORD posOperations;                            // Position operations supported in SQLSetPos
			UDWORD posStmts;                                 // Position statements supported
			UDWORD scrollConcurrency;                        // Concurrency control options supported for scrollable cursors
			UDWORD scrollOptions;                            // Scroll Options supported for scrollable cursors
			UDWORD staticSensitivity;                        // Indicates if additions, deletions and updates can be detected
			UWORD  txnCapable;                               // Indicates if the data source supports transactions
			UDWORD loginTimeout;                             // Number seconds to wait for a login request
		} dbInf;

		// ODBC Error Inf.
		SWORD  cbErrorMsg;
		int    DB_STATUS;
		wchar_t errorList[DB_MAX_ERROR_HISTORY][DB_MAX_ERROR_MSG_LEN];
		wchar_t errorMsg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER nativeError;
		wchar_t sqlState[20];

		// Public member functions
		Database(const HENV &aHenv, bool FwdOnlyCursors=(bool)wxODBC_FWD_ONLY_CURSORS);
		~Database();

		// Data Source Name, User ID, Password and whether open should fail on data type not supported
		bool         Open(const std::wstring& inConnectStr, bool failOnDataTypeUnsupported=true);
		///This version of Open will open the odbc source selection dialog. Cast a wxWindow::GetHandle() to SQLHWND to use.
		bool         Open(const std::wstring& inConnectStr, SQLHWND parentWnd, bool failOnDataTypeUnsupported=true);
		bool         Open(const std::wstring &Dsn, const std::wstring &Uid, const std::wstring &AuthStr, bool failOnDataTypeUnsupported=true);
		bool         Open(DbEnvironment *dbConnectInf, bool failOnDataTypeUnsupported=true);
		bool         Open(Database *copyDb);  // pointer to a wxDb whose connection info should be copied rather than re-queried
		void         Close(void);
		bool         CommitTrans(void);
		bool         RollbackTrans(void);
		bool         DispAllErrors(HENV aHenv, HDBC aHdbc = SQL_NULL_HDBC, HSTMT aHstmt = SQL_NULL_HSTMT);
		bool         GetNextError(HENV aHenv, HDBC aHdbc = SQL_NULL_HDBC, HSTMT aHstmt = SQL_NULL_HSTMT);
		void         DispNextError(void);
		bool         CreateView(const std::wstring &viewName, const std::wstring &colList, const std::wstring &pSqlStmt, bool attemptDrop=true);
		bool         DropView(const std::wstring &viewName);
		bool         ExecSql(const std::wstring &pSqlStmt);
		bool         ExecSql(const std::wstring &pSqlStmt, ColumnInfo** columns, short& numcols);
		bool         GetNext(void);
		bool         GetData(UWORD colNo, SWORD cType, PTR pData, SDWORD maxLen, SQLLEN FAR *cbReturned);
		bool         Grant(int privileges, const std::wstring &tableName, const std::wstring &userList = L"PUBLIC");
		int          TranslateSqlState(const std::wstring &SQLState);
		DbCatalog     *GetCatalog(const wchar_t *userID=NULL);
		bool         Catalog(const wchar_t *userID=NULL, const std::wstring &fileName=SQL_CATALOG_FILENAME);
		int          GetKeyFields(const std::wstring &tableName, ColumnInfo* colInf, UWORD noCols);

		ColumnInfo  *GetColumns(wchar_t *tableName[], const wchar_t *userID=NULL);
		ColumnInfo  *GetColumns(const std::wstring &tableName, UWORD *numCols, const wchar_t *userID=NULL);

		int             GetColumnCount(const std::wstring &tableName, const wchar_t *userID=NULL);
		const wchar_t   *GetDatabaseName(void)  {return dbInf.dbmsName;}
		const wchar_t   *GetDriverVersion(void) {return dbInf.driverVer;};
		const std::wstring &GetDataSource(void)    {return dsn;}
		const std::wstring &GetDatasourceName(void){return dsn;}
		const std::wstring &GetUsername(void)      {return uid;}
		const std::wstring &GetPassword(void)      {return authStr;}
		const std::wstring &GetConnectionInStr(void)  {return inConnectionStr;}
		const std::wstring &GetConnectionOutStr(void) {return outConnectionStr;}
		bool            IsOpen(void)           {return dbIsOpen;}
		bool            OpenedWithConnectionString(void) {return dbOpenedWithConnectionString;}
		HENV            GetHENV(void)          {return henv;}
		HDBC            GetHDBC(void)          {return hdbc;}
		HSTMT           GetHSTMT(void)         {return hstmt;}
		int             GetTableCount()        {return nTables;}  // number of tables using this connection
		SqlTypeInfo GetTypeInfVarchar()    {return typeInfVarchar;}
		SqlTypeInfo GetTypeInfInteger()    {return typeInfInteger;}
		SqlTypeInfo GetTypeInfFloat()      {return typeInfFloat;}
		SqlTypeInfo GetTypeInfDate()       {return typeInfDate;}
		SqlTypeInfo GetTypeInfBlob()       {return typeInfBlob;}
		SqlTypeInfo GetTypeInfMemo()       {return typeInfMemo;}

		// tableName can refer to a table, view, alias or synonym
		bool         TableExists(const std::wstring &tableName, const wchar_t *userID=NULL,
			const std::wstring &tablePath = std::wstring());
		bool         TablePrivileges(const std::wstring &tableName, const std::wstring &priv,
			const wchar_t *userID=NULL, const wchar_t *schema=NULL,
			const std::wstring &path = std::wstring());

		// These two functions return the table name or column name in a form ready
		// for use in SQL statements.  For example, if the datasource allows spaces
		// in the table name or column name, the returned string will have the
		// correct enclosing marks around the name to allow it to be properly
		// included in a SQL statement
		const std::wstring  SQLTableName(const wchar_t *tableName);
		const std::wstring  SQLColumnName(const wchar_t *colName);

		void         LogError(const std::wstring &errMsg, const std::wstring &SQLState = std::wstring())
		{ logError(errMsg, SQLState); }
		void         SetDebugErrorMessages(bool state) { silent = !state; }
		bool         SetSqlLogging(wxDbSqlLogState state, const std::wstring &filename = SQL_LOG_FILENAME,
			bool append = false);
		bool         WriteSqlLog(const std::wstring &logMsg);

		std::vector<std::wstring> GetErrorList() const;

		wxDBMS       Dbms(void);
		bool         ModifyColumn(const std::wstring &tableName, const std::wstring &columnName,
			int dataType, ULONG columnLength=0,
			const std::wstring &optionalParam = std::wstring());

		bool         FwdOnlyCursors(void)  {return fwdOnlyCursors;}

		// return the string with all special SQL characters escaped
		std::wstring     EscapeSqlChars(const std::wstring& value);

	private:
		// These two functions are provided strictly for use by wxDbTable.
		// DO NOT USE THESE FUNCTIONS, OR MEMORY LEAKS MAY OCCUR
		void         incrementTableCount() { nTables++; return; }
		void         decrementTableCount() { nTables--; return; }
	};  // wxDb


	// This structure forms a node in a linked list.  The linked list of "DbList" objects
	// keeps track of allocated database connections.  This allows the application to
	// open more than one database connection through ODBC for multiple transaction support
	// or for multiple database support.
	struct wxDbList
	{
		wxDbList *PtrPrev;       // Pointer to previous item in the list
		std::wstring  Dsn;           // Data Source Name
		std::wstring  Uid;           // User ID
		std::wstring  AuthStr;       // Authorization string (password)
		std::wstring  ConnectionStr; // Connection string used instead of DSN
		Database     *PtrDb;         // Pointer to the wxDb object
		bool      Free;          // Is item free or in use?
		wxDbList *PtrNext;       // Pointer to next item in the list
	};


#ifdef __WXDEBUG__
	//#include "wx/object.h"
	struct wxTablesInUse
	{
	public:
		const wchar_t  *tableName;
		ULONG          tableID;
		class Database    *pDb;
	};  // wxTablesInUse
#endif


	// The following routines allow a user to get new database connections, free them
	// for other code segments to use, or close all of them when the application has
	// completed.
	Database  WXDLLIMPEXP_ODBC *wxDbGetConnection(DbEnvironment *pDbConfig, bool FwdOnlyCursors=(bool)wxODBC_FWD_ONLY_CURSORS);
	bool  WXDLLIMPEXP_ODBC  wxDbFreeConnection(Database *pDb);
	void  WXDLLIMPEXP_ODBC  wxDbCloseConnections(void);
	int   WXDLLIMPEXP_ODBC  wxDbConnectionsInUse(void);


	// Writes a message to the wxLog window (stdout usually) when an internal error
	// situation occurs.  This function only works in DEBUG builds
	const wchar_t WXDLLIMPEXP_ODBC *
		wxDbLogExtendedErrorMsg(const wchar_t *userText,
		Database *pDb,
		const wchar_t *ErrFile,
		int ErrLine);


	// This function sets the sql log state for all open wxDb objects
	bool WXDLLIMPEXP_ODBC
		wxDbSqlLog(wxDbSqlLogState state, const std::wstring &filename = SQL_LOG_FILENAME);


#if 0
	// MSW/VC6 ONLY!!!  Experimental
	int WXDLLEXPORT wxDbCreateDataSource(const std::wstring &driverName, const std::wstring &dsn, const std::wstring &description=emptyString,
		bool sysDSN=false, const std::wstring &defDir=emptyString, wxWindow *parent=NULL);
#endif

	// This routine allows you to query a driver manager
	// for a list of available datasources.  Call this routine
	// the first time using SQL_FETCH_FIRST.  Continue to call it
	// using SQL_FETCH_NEXT until you've exhausted the list.
	bool WXDLLIMPEXP_ODBC
		wxDbGetDataSource(HENV henv, wchar_t *Dsn, SWORD DsnMaxLength, wchar_t *DsDesc,
		SWORD DsDescMaxLength, UWORD direction = SQL_FETCH_NEXT);

}	// namespace exodbc

#endif // DB_H

