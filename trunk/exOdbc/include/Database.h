///////////////////////////////////////////////////////////////////////////////
// Name:        wx/db.h
// Purpose:     Header file wxDb class.  The wxDb class represents a connection
//              to an ODBC data source.  The wxDb class allows operations on the data
//              source such as OpenImpling and closing the data source.
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

// Same component headers
#include "exOdbc.h"

// Other headers
#if EXODBC3_TEST
	#include "gtest/gtest_prod.h"
#endif

// System headers
#include <vector>
#include <set>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#include <odbcinst.h>

// BJO 20000503: introduce new GetColumns members which are more database independent and
//               return columns in the order they were created
#define OLD_GETCOLUMNS 1
#define EXPERIMENTAL_WXDB_FUNCTIONS 1

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


// Forward declarations
// --------------------
namespace exodbc
{
	class DbEnvironment;
	class ColumnFormatter;
	class Database;

	// Structs
	// -------

	// This structure forms a node in a linked list.  The linked list of "DbList" objects
	// keeps track of allocated database connections.  This allows the application to
	// OpenImpl more than one database connection through ODBC for multiple transaction support
	// or for multiple database support.
	struct SDbList
	{
		SDbList *PtrPrev;       // Pointer to previous item in the list
		std::wstring  Dsn;           // Data Source Name
		std::wstring  Uid;           // User ID
		std::wstring  AuthStr;       // Authorization string (password)
		std::wstring  ConnectionStr; // Connection string used instead of DSN
		Database     *PtrDb;         // Pointer to the wxDb object
		bool      Free;          // Is item free or in use?
		SDbList *PtrNext;       // Pointer to next item in the list
	};


#ifdef EXODBCDEBUG
	struct STablesInUse
	{
	public:
		std::wstring   tableName;
		ULONG          tableID;
		class Database    *pDb;
	};  // STablesInUse
#endif

	// Classes
	// -------
	class EXODBCAPI ColumnInfo
	{
	public:
		ColumnInfo();
		~ColumnInfo();

		bool Initialize();

		wchar_t			m_catalog[128+1];
		wchar_t			m_schema[128+1];
		wchar_t			m_tableName[DB_MAX_TABLE_NAME_LEN_DEFAULT+1];
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
		wchar_t			m_pkTableName[DB_MAX_TABLE_NAME_LEN_DEFAULT+1]; // Tables that use this PKey as a FKey
		int				m_fkCol;       // Foreign key column       0=No; 1= First Key, 2 = Second Key etc.
		wchar_t			m_fkTableName[DB_MAX_TABLE_NAME_LEN_DEFAULT+1]; // Foreign key table name
		ColumnFormatter* m_pColFor;                              // How should this columns be formatted

	};


	class EXODBCAPI Database
	{
		// Test helpers:
#if EXODBC3_TEST
		friend class DbTest;
		FRIEND_TEST(DbTest, ReadDataTypesInfo); 
#endif

	public:

		// Public member functions
		/*!
		 * \brief	Create a new Database-instance. The instance will be using the passed
		 *			DbEnvironment.
		 * \param	pEnv		The DbEnvironment to use to create this database and its connection.
		 *						Do not free the DbEnvironment before you free the Database.
		*/
		Database(const DbEnvironment* const pEnv);
		~Database();

		/*!
		 * \fn	bool Database::Open(const std::wstring& inConnectStr);
		 *
		 * \brief	Connect using a prepared connection-String.
		 * 			Uses SQLDriverConnect without a window-handle to connect
		 *
		 * \param	connectStr			 		The connect string.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         Open(const std::wstring& inConnectStr);

		/*!
		 * \fn	bool Database::Open(const std::wstring& inConnectStr, SQLHWND parentWnd);
		 *
		 * \brief	This version of Open will display the odbc source selection dialog, using SQLDriverConnect. Cast a
		 * 			wxWindow::GetHandle() to SQLHWND to use.
		 *
		 * \param	inConnectStr			 	The in connect string.
		 * \param	parentWnd				 	The parent window.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         Open(const std::wstring& inConnectStr, SQLHWND parentWnd);

		/*!
		 * \fn	bool Database::Open(const std::wstring& Dsn, const std::wstring& Uid, const std::wstring& AuthStr);
		 *
		 * \brief	Opens the connectiong using SQLConnect with the passed parameters.
		 *
		 * \param	Dsn						 	The dsn.
		 * \param	Uid						 	The UID.
		 * \param	AuthStr					 	The authentication string.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         Open(const std::wstring& Dsn, const std::wstring& Uid, const std::wstring& AuthStr);

		/*!
		 * \fn	bool Database::Open(const DbEnvironment* const pEnv);
		 *
		 * \brief	Opens by using the information from the passed DbEnvironment
		 *
		 * \param [in,out]	pEnv	 	Pointer to the DbEnvironment to use to connect.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         Open(const DbEnvironment* const pEnv);

		/*!
		 * \fn	bool Database::Close();
		 *
		 * \brief	If this database is open, closes the stmt-handle and the connection to the db.
		 *
		 * \return	True if not open or closing the Db-connection works, False if closing the db-
		 * 			connection fails.
		 */
		bool         Close();

		enum ExecFailMode { FailOnNoData, NotFailOnNoData };
		/**
		 * \fn	bool Database::ExecSql(const std::wstring& sqlStmt, ExecFailMode mode = NotFailOnNoData);
		 *
		 * \brief	Executes the SQL operation on the internal stmt-handle.
		 *
		 * \author	Eli
		 * \date	17.08.2014
		 *
		 * \param	sqlStmt	The SQL statement.
		 * \param	mode   	If FailOnNoData is set, false is returned if SQL returns NO_DATA.
		 * 					This happens for example on DB2 if you do a DELETE with a WHERE
		 * 					clause and no records are deleted.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         ExecSql(const std::wstring& sqlStmt, ExecFailMode mode = NotFailOnNoData);

		/*!
		 * \fn	bool Database::CommitTrans();
		 *
		 * \brief	Commits all transaction associated with this database.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         CommitTrans();

		/*!
		 * \fn	bool Database::RollbackTrans();
		 *
		 * \brief	Rolls back all transaction associated with this database.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         RollbackTrans();

		/**
		 * \fn	bool Database::ReadCompleteCatalog(SDbCatalog& catalogInfo);
		 *
		 * \brief	Reads complete catalog. Queries the database using SQLTables with no search-string 
		 * 			set at all. All parameters all NULL. This is due to the fact that SQL_ATTR_METADATA_ID
		 * 			is not really implemented by all databases, so keep it simple.
		 *
		 * \param [in,out]	catalogInfo	Information describing the catalog.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadCompleteCatalog(SDbCatalogInfo& catalogInfo);

		/*!
		 * \fn	bool Database::ReadCatalogs(std::vector<std::wstring>& catalogs)
		 *
		 * \brief	Reads all Catalogs that are defined in the DB. This calls SQLTables with 
		 * 			SQL_ALL_CATALOGS as catalog-name.
		 *
		 * \param [in,out]	catalogs	The catalogs.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadCatalogs(std::vector<std::wstring>& catalogs)		{ return ReadCatalogInfo(AllCatalogs, catalogs); };

		/*!
		 * \fn	bool exodbc::Database::ReadSchemas(std::vector<std::wstring>& schemas)
		 *
		 * \brief	Reads all schemas that are defined in the DB. This calls SQLTbles with
		 * 			SQL_ALL_SCHEMAS as schema-name.
		 *
		 * \param [in,out]	schemas	The schemas.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadSchemas(std::vector<std::wstring>& schemas)			{ return ReadCatalogInfo(AllSchemas, schemas); };

		/*!
		 * \fn	bool exodbc::Database::ReadTableTypes(std::vector<std::wstring>& tableTypes)
		 *
		 * \brief	Reads all table types that are defined by the DB. This call SQLTables with
		 * 			SQL_ALL_TABLE_TYPES as table-type.
		 *
		 * \param [in,out]	tableTypes	List of types of the tables.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadTableTypes(std::vector<std::wstring>& tableTypes)	{ return ReadCatalogInfo(AllTableTypes, tableTypes); };

		/*!
		 * \fn	int Database::ReadColumnCount(const STableInfo& table);
		 *
		 * \brief	Queries the database using SQLColumns to determine the number of columns of
		 * 			the passed Table (which should have been queried from the catalog, using
		 * 			FindTable or similar).
		 * 			Note: No checks are done to ensure the passed table matches only one table
		 * 			of the database. You might get confusing results if you have for example
		 * 			search-patterns set as table name in the passed STableInfo table.
		 *
		 * \param	table	The table.
		 *
		 * \return	The column count, or -1 in case of failure
		 */
		int			ReadColumnCount(const STableInfo& table);

		/*!
		 * \fn	int Database::ReadColumnCount(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName);
		 *
		 * \brief	Reads column count for one table. First the database is queried for a table
		 * 			that matches the passed arguments. If not exactly one such table is found this
		 * 			function fails.
		 * 			If exactly one table is found, the call is forwarded to ReadColumnCount
		 *
		 * \param	tableName  	Name of the table.
		 * \param	schemaName 	Name of the schema.
		 * \param	catalogName	Name of the catalog.
		 *
		 * \return	The column count or -1 in case of failure
		 */
		int			ReadColumnCount(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName);

		/*!
		 * \fn	bool Database::ReadTablePrivileges(const STableInfo& table, std::vector<STablePrivilegesInfo>& privileges);
		 *
		 * \brief	Reads table privileges for the table(s) matching the passed SCatalogTable description.
		 *
		 * \param	table			  	Defines the tablename, type, schema and catalog to search for privileges.
		 * \param [in,out]	privileges	The privileges.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadTablePrivileges(const STableInfo& table, std::vector<STablePrivilegesInfo>& privileges);

		/*!
		 * \fn	bool Database::ReadTablePrivileges(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, std::vector<STablePrivilegesInfo>& privileges);
		 *
		 * \brief	Reads table privileges of exactly one table. This method will fail if not
		 * 			exactly one table is found matching the passed arguments.
		 *
		 * \param	tableName		  	Name of the table.
		 * \param	schemaName		  	Name of the schema.
		 * \param	catalogName		  	Name of the catalog.
		 * \param [in,out]	privileges	The privileges.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadTablePrivileges(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, std::vector<STablePrivilegesInfo>& privileges);

		/*!
		 * \fn	bool Database::ReadTableColumnInfo(const STableInfo& table, std::vector<STableColumnInfo>& columns);
		 *
		 * \brief	Reads table column information for the passed table.
		 *
		 * \param	table		   	The table.
		 * \param [in,out]	columns	The columns.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadTableColumnInfo(const STableInfo& table, std::vector<STableColumnInfo>& columns);

		/*!
		 * \fn	bool exodbc::Database::ReadTableColumnInfo(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, std::vector<STableColumnInfo>& columns);
		 *
		 * \brief	Reads table column information for exactly one table. This method will fail if the passed arguments
		 * 			do not match exactly one table.
		 *
		 * \param	tableName	   	Name of the table.
		 * \param	schemaName	   	Name of the schema.
		 * \param	catalogName	   	Name of the catalog.
		 * \param [in,out]	columns	The columns.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadTableColumnInfo(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, std::vector<STableColumnInfo>& columns);

		/*!
		 * \fn	bool Database::FindTables(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType, std::vector<DbCatalogTable>& tables);
		 *
		 * \brief	Searches for tables using SQLTables. If any of the parameters passed is empty, SQLTables will be called
		 * 			with a NULL value for that parameter, which indicates that we do not care about that param.
		 * 			The attribute SQL_ATTR_METADATA_ID should default to FALSE, so all parameters are treated as pattern value
		 * 			arguments (case sensitive, but you can use search patterns).
		 * 			See: http://msdn.microsoft.com/en-us/library/ms711831%28v=vs.85%29.aspx
		 *
		 * \param	tableName	  	Name of the table.
		 * \param	schemaName	  	Name of the schema.
		 * \param	catalogName   	Name of the catalog.
		 * \param	tableType	  	Type of the table.
		 * \param [in,out]	tables	The tables found that match the search-criteria.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		FindTables(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType, std::vector<STableInfo>& tables);

		/*!
		 * \fn	TransactionMode Database::ReadTransactionMode();
		 *
		 * \brief	Queries the database for the attribute SQL_ATTR_AUTOCOMMIT. The internal flag
		 * 			m_transactionMode is not changed.
		 *
		 * \return	TM_UNKNOWN if fails, else the mode currently set
		 */
		TransactionMode		ReadTransactionMode();

		/*!
		 * \fn	bool Database::SetTransactionMode(TransactionMode mode);
		 *
		 * \brief	Sets transaction mode on the database, using the attribute SQL_ATTR_AUTOCOMMIT.
		 * 			This will also update the internal flag m_transactionMode, if changing the mode
		 * 			was successful.
		 * \see	GetTransactionMode()
		 *
		 * \param	mode	The mode to set.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		SetTransactionMode(TransactionMode mode);

		/*!
		 * \fn	TransactionMode Database::GetTransactionMode()
		 *
		 * \brief	Gets transaction mode from the internally cached value.
		 * \see		SetTransactionMode(TransactionMode mode)
		 *
		 * \return	The transaction mode.
		 */
		TransactionMode GetTransactionMode() { return m_transactionMode; };

		bool         DispAllErrors(HENV aHenv, HDBC aHdbc = SQL_NULL_HDBC, HSTMT aHstmt = SQL_NULL_HSTMT);
//		bool         GetNextError(HENV aHenv, HDBC aHdbc = SQL_NULL_HDBC, HSTMT aHstmt = SQL_NULL_HSTMT);
//		void         DispNextError();
//		bool         CreateView(const std::wstring& viewName, const std::wstring& colList, const std::wstring& pSqlStmt, bool attemptDrop = true);
//		bool         DropView(const std::wstring& viewName);
//		bool         ExecSql(const std::wstring& pSqlStmt, ColumnInfo** columns, short& numcols);
//		bool         GetNext();
//		bool         Grant(int privileges, const std::wstring& tableName, const std::wstring& userList = L"PUBLIC");
		int          TranslateSqlState(const std::wstring& SQLState);
//		bool         Catalog(const wchar_t* userID = NULL, const std::wstring& fileName = SQL_CATALOG_FILENAME);
//		int          GetKeyFields(const std::wstring& tableName, ColumnInfo* colInf, UWORD noCols);

		//ColumnInfo*		GetColumns(wchar_t* tableName[], const wchar_t* userID = NULL);
		//ColumnInfo*		GetColumns(const std::wstring& tableName, UWORD* numCols, const wchar_t* userID=NULL);

//		int					GetColumnCount(const std::wstring& tableName, const wchar_t* userID=NULL);
		const wchar_t*		GetDatabaseName()  {return m_dbInf.m_dbmsName;}
		const wchar_t*		GetDriverVersion() {return m_dbInf.driverVer;}
		const std::wstring& GetDataSource()    {return m_dsn;}
		const std::wstring& GetDatasourceName(){return m_dsn;}
		const std::wstring& GetUsername()      {return m_uid;}
		const std::wstring& GetPassword()      {return m_authStr;}
		const std::wstring& GetConnectionInStr()  {return m_inConnectionStr;}
		const std::wstring& GetConnectionOutStr() {return m_outConnectionStr;}
		bool            IsOpen()           {return m_dbIsOpen;}
		bool            OpenedWithConnectionString() {return m_dbOpenedWithConnectionString;}
		bool			HasHdbc()			{ return m_hdbc != SQL_NULL_HDBC; };
		HDBC            GetHDBC()          {return m_hdbc;}
		HSTMT           GetHSTMT()         {return m_hstmt;}
		int             GetTableCount()        {return nTables;}  // number of tables using this connection
		//SSqlTypeInfo GetTypeInfVarchar()    {return m_typeInfVarchar;}
		//SSqlTypeInfo GetTypeInfInteger()    {return m_typeInfInteger;}
		//SSqlTypeInfo GetTypeInfFloat()      {return m_typeInfFloat;}
		//SSqlTypeInfo GetTypeInfDate()       {return m_typeInfDate;}
		//SSqlTypeInfo GetTypeInfBlob()       {return m_typeInfBlob;}
		//SSqlTypeInfo GetTypeInfMemo()       {return m_typeInfMemo;}
		SDbInfo GetDbInfo()					{return m_dbInf;}

		// tableName can refer to a table, view, alias or synonym
//		bool         TableExists(const std::wstring& tableName, const wchar_t* userID = NULL, const std::wstring& tablePath = std::wstring());
//		bool         TablePrivileges(const std::wstring& tableName, const std::wstring& priv, const wchar_t* userID = NULL, const wchar_t* schema = NULL, const std::wstring& path = std::wstring());

		// These two functions return the table name or column name in a form ready
		// for use in SQL statements.  For example, if the datasource allows spaces
		// in the table name or column name, the returned string will have the
		// correct enclosing marks around the name to allow it to be properly
		// included in a SQL statement
		//const std::wstring  SQLTableName(const wchar_t* tableName);
		//const std::wstring  SQLColumnName(const wchar_t* colName);

//		void         LogError(const std::wstring& errMsg, const std::wstring& SQLState = std::wstring()) { LogErrorImpl(errMsg, SQLState); }
//		void         SetDebugErrorMessages(bool state) { m_silent = !state; }
//		bool         SetSqlLogging(wxDbSqlLogState state, const std::wstring& filename = SQL_LOG_FILENAME, bool append = false);
//		bool         WriteSqlLog(const std::wstring& logMsg);

//		std::vector<std::wstring> GetErrorList() const;

		DatabaseProduct       Dbms();
		//bool         ModifyColumn(const std::wstring& tableName, const std::wstring& columnName, int dataType, ULONG columnLength = 0, const std::wstring& optionalParam = std::wstring());

		bool         FwdOnlyCursors()  {return m_fwdOnlyCursors;}

		// return the string with all special SQL characters escaped
		//std::wstring     EscapeSqlChars(const std::wstring& value);

	private:
		// Private member functions
		bool			ReadDataTypesInfo(std::vector<SSqlTypeInfo>& types);

		void			Initialize();

		bool			ReadDbInfo(SDbInfo& dbInfo);
		bool			SetConnectionAttributes();
//		void			LogErrorImpl(const std::wstring& errMsg, const std::wstring& SQLState);
//		std::wstring	ConvertUserIDImpl(const wchar_t* userID);
		//bool             DetermineDataTypes(bool failOnDataTypeUnsupported);
		bool             OpenImpl();

		enum ReadCatalogInfoMode
		{
			AllCatalogs,
			AllSchemas,
			AllTableTypes
		};
		bool			ReadCatalogInfo(ReadCatalogInfoMode mode, std::vector<std::wstring>& results);

		/*!
		 * \fn	bool Database::FindOneTable(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, std::vector<SCatalogTablePrivilege>& privileges, SDbCatalogTable& table);
		 *
		 * \brief	Searches for tables that match the passed arguments, return true if exactly one such table is found.
		 * 			The table that matches is copied to the argument table.
		 *
		 * \param	tableName		  	Name of the table.
		 * \param	schemaName		  	Name of the schema.
		 * \param	catalogName		  	Name of the catalog.
		 * \param [in,out]	table	  	The table.
		 *
		 * \return	true if exactly one table matches the passed search-criterias name, schema and catalog, false else.
		 */
		bool			FindOneTable(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, STableInfo& table);

		// Members
		SDbInfo				m_dbInf;

		std::vector<SSqlTypeInfo> m_datatypes; // Queried from DB during Open
		bool				m_dbIsOpen;	// Set to true after SQLConnect was successful
		bool				m_dbOpenedWithConnectionString;  // Was the database connection OpenImpled with a connection string
		std::wstring		m_dsn;             // Data source name
		std::wstring		m_uid;             // User ID
		std::wstring		m_authStr;         // Authorization string (password)
		std::wstring		m_inConnectionStr; // Connection string used to connect to the database
		std::wstring		m_outConnectionStr;// Connection string returned by the database when a connection is successfully OpenImpled
//		FILE*				m_fpSqlLog;        // Sql Log file pointer
//		wxDbSqlLogState		m_sqlLogState;     // On or Off
		bool				m_fwdOnlyCursors;
		DatabaseProduct				m_dbmsType;        // Type of datasource - i.e. Oracle, dBase, SQLServer, etc

		// ODBC handles
		HDBC  m_hdbc;        // ODBC DB Connection handle
		HSTMT m_hstmt;       // ODBC Statement handle

		//Error reporting mode
		bool m_silent;

		TransactionMode		m_transactionMode;

		// Information about logical data types VARCHAR, INTEGER, FLOAT and DATE.
		//
		// This information is obtained from the ODBC driver by use of the
		// SQLGetTypeInfo() function.  The key piece of information is the
		// type name the data source uses for each logical data type.
		// e.g. VARCHAR; Oracle calls it VARCHAR2.
		//SSqlTypeInfo m_typeInfVarchar;
		//SSqlTypeInfo m_typeInfInteger;
		//SSqlTypeInfo m_typeInfFloat;
		//SSqlTypeInfo m_typeInfDate;
		//SSqlTypeInfo m_typeInfBlob;
		//SSqlTypeInfo m_typeInfMemo;


	private:
		// These two functions are provided strictly for use by wxDbTable.
		// DO NOT USE THESE FUNCTIONS, OR MEMORY LEAKS MAY OCCUR
		void         incrementTableCount() { nTables++; return; }
		void         decrementTableCount() { nTables--; return; }

		// Number of Ctable objects connected to this db object.  FOR INTERNAL USE ONLY!!!
		unsigned int nTables;

		friend class Table;

	};  // Database


	// Writes a message to the wxLog window (stdout usually) when an internal error
	// situation occurs.  This function only works in DEBUG builds
	//const wchar_t EXODBCAPI *
	//	wxDbLogExtendedErrorMsg(const wchar_t *userText,
	//	Database *pDb,
	//	const wchar_t *ErrFile,
	//	int ErrLine);


	// This function sets the sql log state for all OpenImpl wxDb objects
	//bool EXODBCAPI
	//	wxDbSqlLog(wxDbSqlLogState state, const std::wstring &filename = SQL_LOG_FILENAME);


#if 0
	// MSW/VC6 ONLY!!!  Experimental
	int WXDLLEXPORT wxDbCreateDataSource(const std::wstring &driverName, const std::wstring &dsn, const std::wstring &description=emptyString,
		bool sysDSN=false, const std::wstring &defDir=emptyString, wxWindow *parent=NULL);
#endif

}	// namespace exodbc

#endif // DB_H

