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

/* There are too many false positives for this one, particularly when using templates like wxVector<T> */
/* class 'foo' needs to have dll-interface to be used by clients of class 'bar'" */
#pragma warning(disable:4251)

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


	const int wxDB_PATH_MAX                 = 254;

	extern WXDLLIMPEXP_DATA_ODBC(wchar_t const *) SQL_LOG_FILENAME;
	extern WXDLLIMPEXP_DATA_ODBC(wchar_t const *) SQL_CATALOG_FILENAME;

	// Database Globals
	const int DB_TYPE_NAME_LEN            = 40;
	const int DB_MAX_STATEMENT_LEN        = 4096;
	const int DB_MAX_WHERE_CLAUSE_LEN     = 2048;
	const int DB_MAX_ERROR_MSG_LEN        = 512;
	const int DB_MAX_ERROR_HISTORY        = 5;
	const int DB_MAX_TABLE_NAME_LEN       = 128;
	const int DB_MAX_COLUMN_NAME_LEN      = 128;

	const int DB_DATA_TYPE_VARCHAR        = 1;
	const int DB_DATA_TYPE_INTEGER        = 2;
	const int DB_DATA_TYPE_FLOAT          = 3;
	const int DB_DATA_TYPE_DATE           = 4;
	const int DB_DATA_TYPE_BLOB           = 5;
	const int DB_DATA_TYPE_MEMO           = 6;

	const int DB_SELECT_KEYFIELDS         = 1;
	const int DB_SELECT_WHERE             = 2;
	const int DB_SELECT_MATCHING          = 3;
	const int DB_SELECT_STATEMENT         = 4;

	const int DB_UPD_KEYFIELDS            = 1;
	const int DB_UPD_WHERE                = 2;

	const int DB_DEL_KEYFIELDS            = 1;
	const int DB_DEL_WHERE                = 2;
	const int DB_DEL_MATCHING             = 3;

	const int DB_WHERE_KEYFIELDS          = 1;
	const int DB_WHERE_MATCHING           = 2;

	const int DB_GRANT_SELECT             = 1;
	const int DB_GRANT_INSERT             = 2;
	const int DB_GRANT_UPDATE             = 4;
	const int DB_GRANT_DELETE             = 8;
	const int DB_GRANT_ALL                = DB_GRANT_SELECT | DB_GRANT_INSERT | DB_GRANT_UPDATE | DB_GRANT_DELETE;

	// ODBC Error codes (derived from ODBC SqlState codes)
	enum wxODBC_ERRORS
	{
		DB_FAILURE                        = 0,
		DB_SUCCESS                        = 1,
		DB_ERR_NOT_IN_USE,
		DB_ERR_GENERAL_WARNING,                            // SqlState = '01000'
		DB_ERR_DISCONNECT_ERROR,                           // SqlState = '01002'
		DB_ERR_DATA_TRUNCATED,                             // SqlState = '01004'
		DB_ERR_PRIV_NOT_REVOKED,                           // SqlState = '01006'
		DB_ERR_INVALID_CONN_STR_ATTR,                      // SqlState = '01S00'
		DB_ERR_ERROR_IN_ROW,                               // SqlState = '01S01'
		DB_ERR_OPTION_VALUE_CHANGED,                       // SqlState = '01S02'
		DB_ERR_NO_ROWS_UPD_OR_DEL,                         // SqlState = '01S03'
		DB_ERR_MULTI_ROWS_UPD_OR_DEL,                      // SqlState = '01S04'
		DB_ERR_WRONG_NO_OF_PARAMS,                         // SqlState = '07001'
		DB_ERR_DATA_TYPE_ATTR_VIOL,                        // SqlState = '07006'
		DB_ERR_UNABLE_TO_CONNECT,                          // SqlState = '08001'
		DB_ERR_CONNECTION_IN_USE,                          // SqlState = '08002'
		DB_ERR_CONNECTION_NOT_OPEN,                        // SqlState = '08003'
		DB_ERR_REJECTED_CONNECTION,                        // SqlState = '08004'
		DB_ERR_CONN_FAIL_IN_TRANS,                         // SqlState = '08007'
		DB_ERR_COMM_LINK_FAILURE,                          // SqlState = '08S01'
		DB_ERR_INSERT_VALUE_LIST_MISMATCH,                 // SqlState = '21S01'
		DB_ERR_DERIVED_TABLE_MISMATCH,                     // SqlState = '21S02'
		DB_ERR_STRING_RIGHT_TRUNC,                         // SqlState = '22001'
		DB_ERR_NUMERIC_VALUE_OUT_OF_RNG,                   // SqlState = '22003'
		DB_ERR_ERROR_IN_ASSIGNMENT,                        // SqlState = '22005'
		DB_ERR_DATETIME_FLD_OVERFLOW,                      // SqlState = '22008'
		DB_ERR_DIVIDE_BY_ZERO,                             // SqlState = '22012'
		DB_ERR_STR_DATA_LENGTH_MISMATCH,                   // SqlState = '22026'
		DB_ERR_INTEGRITY_CONSTRAINT_VIOL,                  // SqlState = '23000'
		DB_ERR_INVALID_CURSOR_STATE,                       // SqlState = '24000'
		DB_ERR_INVALID_TRANS_STATE,                        // SqlState = '25000'
		DB_ERR_INVALID_AUTH_SPEC,                          // SqlState = '28000'
		DB_ERR_INVALID_CURSOR_NAME,                        // SqlState = '34000'
		DB_ERR_SYNTAX_ERROR_OR_ACCESS_VIOL,                // SqlState = '37000'
		DB_ERR_DUPLICATE_CURSOR_NAME,                      // SqlState = '3C000'
		DB_ERR_SERIALIZATION_FAILURE,                      // SqlState = '40001'
		DB_ERR_SYNTAX_ERROR_OR_ACCESS_VIOL2,               // SqlState = '42000'
		DB_ERR_OPERATION_ABORTED,                          // SqlState = '70100'
		DB_ERR_UNSUPPORTED_FUNCTION,                       // SqlState = 'IM001'
		DB_ERR_NO_DATA_SOURCE,                             // SqlState = 'IM002'
		DB_ERR_DRIVER_LOAD_ERROR,                          // SqlState = 'IM003'
		DB_ERR_SQLALLOCENV_FAILED,                         // SqlState = 'IM004'
		DB_ERR_SQLALLOCCONNECT_FAILED,                     // SqlState = 'IM005'
		DB_ERR_SQLSETCONNECTOPTION_FAILED,                 // SqlState = 'IM006'
		DB_ERR_NO_DATA_SOURCE_DLG_PROHIB,                  // SqlState = 'IM007'
		DB_ERR_DIALOG_FAILED,                              // SqlState = 'IM008'
		DB_ERR_UNABLE_TO_LOAD_TRANSLATION_DLL,             // SqlState = 'IM009'
		DB_ERR_DATA_SOURCE_NAME_TOO_LONG,                  // SqlState = 'IM010'
		DB_ERR_DRIVER_NAME_TOO_LONG,                       // SqlState = 'IM011'
		DB_ERR_DRIVER_KEYWORD_SYNTAX_ERROR,                // SqlState = 'IM012'
		DB_ERR_TRACE_FILE_ERROR,                           // SqlState = 'IM013'
		DB_ERR_TABLE_OR_VIEW_ALREADY_EXISTS,               // SqlState = 'S0001'
		DB_ERR_TABLE_NOT_FOUND,                            // SqlState = 'S0002'
		DB_ERR_INDEX_ALREADY_EXISTS,                       // SqlState = 'S0011'
		DB_ERR_INDEX_NOT_FOUND,                            // SqlState = 'S0012'
		DB_ERR_COLUMN_ALREADY_EXISTS,                      // SqlState = 'S0021'
		DB_ERR_COLUMN_NOT_FOUND,                           // SqlState = 'S0022'
		DB_ERR_NO_DEFAULT_FOR_COLUMN,                      // SqlState = 'S0023'
		DB_ERR_GENERAL_ERROR,                              // SqlState = 'S1000'
		DB_ERR_MEMORY_ALLOCATION_FAILURE,                  // SqlState = 'S1001'
		DB_ERR_INVALID_COLUMN_NUMBER,                      // SqlState = 'S1002'
		DB_ERR_PROGRAM_TYPE_OUT_OF_RANGE,                  // SqlState = 'S1003'
		DB_ERR_SQL_DATA_TYPE_OUT_OF_RANGE,                 // SqlState = 'S1004'
		DB_ERR_OPERATION_CANCELLED,                        // SqlState = 'S1008'
		DB_ERR_INVALID_ARGUMENT_VALUE,                     // SqlState = 'S1009'
		DB_ERR_FUNCTION_SEQUENCE_ERROR,                    // SqlState = 'S1010'
		DB_ERR_OPERATION_INVALID_AT_THIS_TIME,             // SqlState = 'S1011'
		DB_ERR_INVALID_TRANS_OPERATION_CODE,               // SqlState = 'S1012'
		DB_ERR_NO_CURSOR_NAME_AVAIL,                       // SqlState = 'S1015'
		DB_ERR_INVALID_STR_OR_BUF_LEN,                     // SqlState = 'S1090'
		DB_ERR_DESCRIPTOR_TYPE_OUT_OF_RANGE,               // SqlState = 'S1091'
		DB_ERR_OPTION_TYPE_OUT_OF_RANGE,                   // SqlState = 'S1092'
		DB_ERR_INVALID_PARAM_NO,                           // SqlState = 'S1093'
		DB_ERR_INVALID_SCALE_VALUE,                        // SqlState = 'S1094'
		DB_ERR_FUNCTION_TYPE_OUT_OF_RANGE,                 // SqlState = 'S1095'
		DB_ERR_INF_TYPE_OUT_OF_RANGE,                      // SqlState = 'S1096'
		DB_ERR_COLUMN_TYPE_OUT_OF_RANGE,                   // SqlState = 'S1097'
		DB_ERR_SCOPE_TYPE_OUT_OF_RANGE,                    // SqlState = 'S1098'
		DB_ERR_NULLABLE_TYPE_OUT_OF_RANGE,                 // SqlState = 'S1099'
		DB_ERR_UNIQUENESS_OPTION_TYPE_OUT_OF_RANGE,        // SqlState = 'S1100'
		DB_ERR_ACCURACY_OPTION_TYPE_OUT_OF_RANGE,          // SqlState = 'S1101'
		DB_ERR_DIRECTION_OPTION_OUT_OF_RANGE,              // SqlState = 'S1103'
		DB_ERR_INVALID_PRECISION_VALUE,                    // SqlState = 'S1104'
		DB_ERR_INVALID_PARAM_TYPE,                         // SqlState = 'S1105'
		DB_ERR_FETCH_TYPE_OUT_OF_RANGE,                    // SqlState = 'S1106'
		DB_ERR_ROW_VALUE_OUT_OF_RANGE,                     // SqlState = 'S1107'
		DB_ERR_CONCURRENCY_OPTION_OUT_OF_RANGE,            // SqlState = 'S1108'
		DB_ERR_INVALID_CURSOR_POSITION,                    // SqlState = 'S1109'
		DB_ERR_INVALID_DRIVER_COMPLETION,                  // SqlState = 'S1110'
		DB_ERR_INVALID_BOOKMARK_VALUE,                     // SqlState = 'S1111'
		DB_ERR_DRIVER_NOT_CAPABLE,                         // SqlState = 'S1C00'
		DB_ERR_TIMEOUT_EXPIRED                             // SqlState = 'S1T00'
	};

#ifndef MAXNAME
#define MAXNAME         31
#endif

#ifndef SQL_MAX_AUTHSTR_LEN
	// There does not seem to be a standard for this, so I am
	// defaulting to the value that MS uses
#define SQL_MAX_AUTHSTR_LEN MAXNAME
#endif

#ifndef SQL_MAX_CONNECTSTR_LEN
	// There does not seem to be a standard for this, so I am
	// defaulting to the value that MS recommends
#define SQL_MAX_CONNECTSTR_LEN 1024
#endif

	class WXDLLIMPEXP_ODBC wxDbConnectInf
	{
	private:
		bool freeHenvOnDestroy;
		bool useConnectionStr;

	public:
		HENV Henv;
		wchar_t Dsn[SQL_MAX_DSN_LENGTH+1];                  // Data Source Name
		wchar_t Uid[SQL_MAX_USER_NAME_LEN+1];               // User ID
		wchar_t AuthStr[SQL_MAX_AUTHSTR_LEN+1];             // Authorization string (password)
		wchar_t ConnectionStr[SQL_MAX_CONNECTSTR_LEN+1];    // Connection string (password)

		std::wstring Description;                              // Not sure what the max length is
		std::wstring FileType;                                 // Not sure what the max length is

		// Optionals needed for some databases like dBase
		std::wstring DefaultDir;                               // Directory that db file resides in

	public:

		wxDbConnectInf();
		wxDbConnectInf(HENV henv, const std::wstring &dsn, const std::wstring &userID = std::wstring(),
			const std::wstring &password = std::wstring(), const std::wstring &defaultDir = std::wstring(),
			const std::wstring &description = std::wstring(), const std::wstring &fileType = std::wstring());

		~wxDbConnectInf();

		bool             Initialize();

		bool             AllocHenv();
		void             FreeHenv();

		// Accessors
		const HENV       &GetHenv()          { return Henv; }

		const wchar_t    *GetDsn()           { return Dsn; }

		const wchar_t    *GetUid()           { return Uid; }
		const wchar_t    *GetUserID()        { return Uid; }

		const wchar_t    *GetAuthStr()       { return AuthStr; }
		const wchar_t    *GetPassword()      { return AuthStr; }

		const wchar_t    *GetConnectionStr() { return ConnectionStr; }
		bool             UseConnectionStr() { return useConnectionStr; }

		const wchar_t    *GetDescription()   { return Description.c_str(); }
		const wchar_t    *GetFileType()      { return FileType.c_str(); }
		const wchar_t    *GetDefaultDir()    { return DefaultDir.c_str(); }

		void             SetHenv(const HENV henv)               { Henv = henv; }

		void             SetDsn(const std::wstring &dsn);

		void             SetUserID(const std::wstring &userID);
		void             SetUid(const std::wstring &uid)            { SetUserID(uid); }

		void             SetPassword(const std::wstring &password);
		void             SetAuthStr(const std::wstring &authstr)    { SetPassword(authstr); }

		void             SetConnectionStr(const std::wstring &connectStr);

		void             SetDescription(const std::wstring &desc)   { Description   = desc;     }
		void             SetFileType(const std::wstring &fileType)  { FileType      = fileType; }
		void             SetDefaultDir(const std::wstring &defDir)  { DefaultDir    = defDir;   }

		bool			SetSqlAttrOdbcVersion(int version);		
		int				ReadSqlAttrOdbcVersion();
	};  // class wxDbConnectInf


	struct WXDLLIMPEXP_ODBC wxDbSqlTypeInfo
	{
		std::wstring    TypeName;
		SWORD       FsqlType;
		long        Precision;
		short       CaseSensitive;
		short       MaximumScale;
	};


	class WXDLLIMPEXP_ODBC wxDbColFor
	{
	public:
		std::wstring       s_Field;              // Formatted String for Output
		std::wstring       s_Format[7];          // Formatted Objects - TIMESTAMP has the biggest (7)
		std::wstring       s_Amount[7];          // Formatted Objects - amount of things that can be formatted
		int            i_Amount[7];          // Formatted Objects - TT MM YYYY HH MM SS m
		int            i_Nation;             // 0 = timestamp , 1=EU, 2=UK, 3=International, 4=US
		int            i_dbDataType;         // conversion of the 'sqlDataType' to the generic data type used by these classes
		SWORD          i_sqlDataType;

		wxDbColFor();
		~wxDbColFor(){}

		void           Initialize();
		int            Format(int Nation, int dbDataType, SWORD sqlDataType, short columnLength, short decimalDigits);
	};


	class WXDLLIMPEXP_ODBC wxDbColInf
	{
	public:
		wchar_t       catalog[128+1];
		wchar_t       schema[128+1];
		wchar_t       tableName[DB_MAX_TABLE_NAME_LEN+1];
		wchar_t       colName[DB_MAX_COLUMN_NAME_LEN+1];
		SWORD        sqlDataType;
		wchar_t       typeName[128+1];
		SWORD        columnLength;
		SWORD        bufferSize;
		short        decimalDigits;
		short        numPrecRadix;
		short        nullable;
		wchar_t       remarks[254+1];
		int          dbDataType;  // conversion of the 'sqlDataType' to the generic data type used by these classes
		// mj10777.19991224 : new
		int          PkCol;       // Primary key column       0=No; 1= First Key, 2 = Second Key etc.
		wchar_t       PkTableName[DB_MAX_TABLE_NAME_LEN+1]; // Tables that use this PKey as a FKey
		int          FkCol;       // Foreign key column       0=No; 1= First Key, 2 = Second Key etc.
		wchar_t       FkTableName[DB_MAX_TABLE_NAME_LEN+1]; // Foreign key table name
		wxDbColFor  *pColFor;                              // How should this columns be formatted

		wxDbColInf();
		~wxDbColInf();

		bool Initialize();
	};


	class WXDLLIMPEXP_ODBC wxDbTableInf        // Description of a Table
	{
	public:
		wchar_t      tableName[DB_MAX_TABLE_NAME_LEN+1];
		wchar_t      tableType[254+1];           // "TABLE" or "SYSTEM TABLE" etc.
		wchar_t      tableRemarks[254+1];
		UWORD       numCols;                    // How many Columns does this Table have: GetColumnCount(..);
		wxDbColInf *pColInf;                    // pColInf = NULL ; User can later call GetColumns(..);

		wxDbTableInf();
		~wxDbTableInf();

		bool             Initialize();
	};


	class WXDLLIMPEXP_ODBC wxDbInf     // Description of a Database
	{
	public:
		wchar_t        catalog[128+1];
		wchar_t        schema[128+1];
		int           numTables;           // How many tables does this database have
		wxDbTableInf *pTableInf;           // pTableInf = new wxDbTableInf[numTables];

		wxDbInf();
		~wxDbInf();

		bool          Initialize();
	};


	enum wxDbSqlLogState
	{
		sqlLogOFF,
		sqlLogON
	};

	// These are the databases currently tested and working with these classes
	// See the comments in wxDb::Dbms() for exceptions/issues with
	// each of these database engines
	enum wxDBMS
	{
		dbmsUNIDENTIFIED,
		dbmsORACLE,
		dbmsSYBASE_ASA,        // Adaptive Server Anywhere
		dbmsSYBASE_ASE,        // Adaptive Server Enterprise
		dbmsMS_SQL_SERVER,
		dbmsMY_SQL,
		dbmsPOSTGRES,
		dbmsACCESS,
		dbmsDBASE,
		dbmsINFORMIX,
		dbmsVIRTUOSO,
		dbmsDB2,
		dbmsINTERBASE,
		dbmsPERVASIVE_SQL,
		dbmsXBASE_SEQUITER,
		dbmsFIREBIRD,
		dbmsMAXDB,
		dbmsFuture1,
		dbmsFuture2,
		dbmsFuture3,
		dbmsFuture4,
		dbmsFuture5,
		dbmsFuture6,
		dbmsFuture7,
		dbmsFuture8,
		dbmsFuture9,
		dbmsFuture10
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


	class WXDLLIMPEXP_ODBC wxDb
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
		bool             getDataTypeInfo(SWORD fSqlType, wxDbSqlTypeInfo &structSQLTypeInfo);
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
		wxDbSqlTypeInfo typeInfVarchar;
		wxDbSqlTypeInfo typeInfInteger;
		wxDbSqlTypeInfo typeInfFloat;
		wxDbSqlTypeInfo typeInfDate;
		wxDbSqlTypeInfo typeInfBlob;
		wxDbSqlTypeInfo typeInfMemo;

	public:

		void             setCached(bool cached)  { dbIsCached = cached; }  // This function must only be called by wxDbGetConnection() and wxDbCloseConnections!!!
		bool             IsCached() { return dbIsCached; }

		bool             GetDataTypeInfo(SWORD fSqlType, wxDbSqlTypeInfo &structSQLTypeInfo)
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
#ifdef __VMS
		// The DECC compiler chokes when in db.cpp the array is accessed outside
		// its bounds. Maybe this change should also applied for other platforms.
		wchar_t errorList[DB_MAX_ERROR_HISTORY][DB_MAX_ERROR_MSG_LEN+1];
#else
		wchar_t errorList[DB_MAX_ERROR_HISTORY][DB_MAX_ERROR_MSG_LEN];
#endif
		wchar_t errorMsg[SQL_MAX_MESSAGE_LENGTH];
		SQLINTEGER nativeError;
		wchar_t sqlState[20];

		// Public member functions
		wxDb(const HENV &aHenv, bool FwdOnlyCursors=(bool)wxODBC_FWD_ONLY_CURSORS);
		~wxDb();

		// Data Source Name, User ID, Password and whether open should fail on data type not supported
		bool         Open(const std::wstring& inConnectStr, bool failOnDataTypeUnsupported=true);
		///This version of Open will open the odbc source selection dialog. Cast a wxWindow::GetHandle() to SQLHWND to use.
		bool         Open(const std::wstring& inConnectStr, SQLHWND parentWnd, bool failOnDataTypeUnsupported=true);
		bool         Open(const std::wstring &Dsn, const std::wstring &Uid, const std::wstring &AuthStr, bool failOnDataTypeUnsupported=true);
		bool         Open(wxDbConnectInf *dbConnectInf, bool failOnDataTypeUnsupported=true);
		bool         Open(wxDb *copyDb);  // pointer to a wxDb whose connection info should be copied rather than re-queried
		void         Close(void);
		bool         CommitTrans(void);
		bool         RollbackTrans(void);
		bool         DispAllErrors(HENV aHenv, HDBC aHdbc = SQL_NULL_HDBC, HSTMT aHstmt = SQL_NULL_HSTMT);
		bool         GetNextError(HENV aHenv, HDBC aHdbc = SQL_NULL_HDBC, HSTMT aHstmt = SQL_NULL_HSTMT);
		void         DispNextError(void);
		bool         CreateView(const std::wstring &viewName, const std::wstring &colList, const std::wstring &pSqlStmt, bool attemptDrop=true);
		bool         DropView(const std::wstring &viewName);
		bool         ExecSql(const std::wstring &pSqlStmt);
		bool         ExecSql(const std::wstring &pSqlStmt, wxDbColInf** columns, short& numcols);
		bool         GetNext(void);
		bool         GetData(UWORD colNo, SWORD cType, PTR pData, SDWORD maxLen, SQLLEN FAR *cbReturned);
		bool         Grant(int privileges, const std::wstring &tableName, const std::wstring &userList = L"PUBLIC");
		int          TranslateSqlState(const std::wstring &SQLState);
		wxDbInf     *GetCatalog(const wchar_t *userID=NULL);
		bool         Catalog(const wchar_t *userID=NULL, const std::wstring &fileName=SQL_CATALOG_FILENAME);
		int          GetKeyFields(const std::wstring &tableName, wxDbColInf* colInf, UWORD noCols);

		wxDbColInf  *GetColumns(wchar_t *tableName[], const wchar_t *userID=NULL);
		wxDbColInf  *GetColumns(const std::wstring &tableName, UWORD *numCols, const wchar_t *userID=NULL);

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
		wxDbSqlTypeInfo GetTypeInfVarchar()    {return typeInfVarchar;}
		wxDbSqlTypeInfo GetTypeInfInteger()    {return typeInfInteger;}
		wxDbSqlTypeInfo GetTypeInfFloat()      {return typeInfFloat;}
		wxDbSqlTypeInfo GetTypeInfDate()       {return typeInfDate;}
		wxDbSqlTypeInfo GetTypeInfBlob()       {return typeInfBlob;}
		wxDbSqlTypeInfo GetTypeInfMemo()       {return typeInfMemo;}

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
		wxDb     *PtrDb;         // Pointer to the wxDb object
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
		class wxDb    *pDb;
	};  // wxTablesInUse
#endif


	// The following routines allow a user to get new database connections, free them
	// for other code segments to use, or close all of them when the application has
	// completed.
	wxDb  WXDLLIMPEXP_ODBC *wxDbGetConnection(wxDbConnectInf *pDbConfig, bool FwdOnlyCursors=(bool)wxODBC_FWD_ONLY_CURSORS);
	bool  WXDLLIMPEXP_ODBC  wxDbFreeConnection(wxDb *pDb);
	void  WXDLLIMPEXP_ODBC  wxDbCloseConnections(void);
	int   WXDLLIMPEXP_ODBC  wxDbConnectionsInUse(void);


	// Writes a message to the wxLog window (stdout usually) when an internal error
	// situation occurs.  This function only works in DEBUG builds
	const wchar_t WXDLLIMPEXP_ODBC *
		wxDbLogExtendedErrorMsg(const wchar_t *userText,
		wxDb *pDb,
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

