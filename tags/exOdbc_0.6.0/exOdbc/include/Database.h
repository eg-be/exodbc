/*!
 * \file Database.h
 * \author Elias Gerber <eg@zame.ch>
 * \date 25.07.2014
 * \brief Header file for the Database class and its helpers.
 *
 * Header file for the Database class and its helpers.
 * This file was originally wx/db.h from wxWidgets 2.8.
 * Most of the code has been rewritten, a lot of functionality
 * not needed and not tested so far has been droped.
 *
 * For completion, here follows the old wxWidgets header:
 *
 * ///////////////////////////////////////////////////////////////////////////////<br>
 * // Name:        wx/db.h<br>
 * // Purpose:     Header file wxDb class.  The wxDb class represents a connection<br>
 * //              to an ODBC data source.  The wxDb class allows operations on the data<br>
 * //              source such as OpenImpling and closing the data source.<br>
 * // Author:      Doug Card<br>
 * // Modified by: George Tasker<br>
 * //              Bart Jourquin<br>
 * //              Mark Johnson, wxWindows@mj10777.de<br>
 * // Mods:        Dec, 1998:<br>
 * //                -Added support for SQL statement logging and database cataloging<br>
 * //                     April, 1999<br>
 * //                -Added QUERY_ONLY mode support to reduce default number of cursors<br>
 * //                -Added additional SQL logging code<br>
 * //                -Added DEBUG-ONLY tracking of Ctable objects to detect orphaned DB connections<br>
 * //                -Set ODBC option to only read committed writes to the DB so all<br>
 * //                     databases operate the same in that respect<br>
 * //<br>
 * // Created:     9.96<br>
 * // RCS-ID:      $Id: db.h 56697 2008-11-07 22:45:47Z VZ $<br>
 * // Copyright:   (c) 1996 Remstar International, Inc.<br>
 * // Licence:     wxWindows licence<br>
 * ///////////////////////////////////////////////////////////////////////////////<br>
*/

#pragma once
#ifndef DB_H
#define DB_H

// Same component headers
#include "exOdbc.h"
#include "Helpers.h"

// Other headers
#if EXODBC_TEST
	#include "gtest/gtest_prod.h"
#endif

// System headers
#include <vector>
#include <map>
#include <set>
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#include <odbcinst.h>


// Forward declarations
// --------------------
namespace exodbc
{
	class Environment;
	class ColumnFormatter;
	class Table;

	// Structs
	// -------

#if defined EXODBCDEBUG
	struct STablesInUse
	{
	public:
		std::wstring	m_initialTableName;
		std::wstring	m_initialSchema;
		std::wstring	m_initialCatalog;
		std::wstring	m_initialType;
		size_t			m_tableId;

		std::wstring ToString() const { return m_initialCatalog + L"::" + m_initialSchema + L"::" + m_initialTableName ;  };
	};
#endif

	// Classes
	// -------

	/*!
	* \class Database
	*
	* \brief Represents the ODBC-Database. Every Database needs an Environment.
	* 
	* This class will allocate the Database-Handle that is required
	* to connect to the Database. It provides methods for opening and closing a
	* connection to a database - see Open() and Close(). 
	* If a Database is being closed and CommitMode is set to CM_MANUAL_COMMIT the
	* Database will Rollback any ongoin transaction first.
	* There is basic support executing SQL on the Database directly
	* using the generic ExecSql() and CommitTrans() functions.
	* The ExecSQL() function uses its own Statement, so it will never be influenced
	* by any of the Catalog-functions.
	*
	* The class provides methods to access the catalog functions of the database
	* to for example query all available catalogs, schemas and tables, or 
	* read the privileges, etc.
	*
	* There are some naming conventions used with the methods inside this class:
	* A method named ReadXXX will read a value a property from the database and
	* update the internally cached value (if there is one) if reading was successful.
	* A method named GetXXX will return the internally cached value of the property.
	* A method named SetXXX will try to set the property on the database. If successful,
	* an eventually internally cached value will be updated.
	*/
	class EXODBCAPI Database
	{
		// Test helpers:
#if EXODBC_TEST
		friend class DatabaseTest;
		FRIEND_TEST(DatabaseTest, ReadDataTypesInfo); 
		FRIEND_TEST(DatabaseTest, SetConnectionAttributes);
		FRIEND_TEST(DatabaseTest, ReadDbInfo);
#endif
	public:
		/*!
		* \brief	Default Constructor. You will need to manually allocate the DBC-Handle.
		* \detailed	Default Constructor. You will need to call AllocateHenv() afterwards to
		*			allocate the Database-handle.
		* \see		AllocateHenv()
		*/
		Database();


		/*!
		* \brief	Create a new Database-instance. The instance will be using the passed
		*			Environment.
		* \detailed	The Database will try to create a new Db-Connection handle during construction.
		*				No Exception is thrown if doing so fails, you can use HasHdbc() to check if
		*				creating the Db-Connection handle was successful.
		*				The handle will be freed by the Database on destruction.
		*
		* \param	pEnv		The Environment to use to create this database and its connection.
		*						Do not free the Environment before you free the Database.
		*/
		Database(const Environment& env);


		/*!
		 * \brief	Create a new Database-instance. The instance will be using the passed
		 *			Environment. Note: This constructor is here for compatibility with wxWidgets.
		 * \detailed	The Database will try to create a new Db-Connection handle during construction.
		 *				No Exception is thrown if doing so fails, you can use HasHdbc() to check if
		 *				creating the Db-Connection handle was successful.
		 *				The handle will be freed by the Database on destruction.
		 *				Note: This function is here for compatibility with wxWidgets which used pointers
		 *
		 * \param	pEnv		The Environment to use to create this database and its connection.
		 *						Do not free the Environment before you free the Database.
		*/
		Database(const Environment* pEnv);
		

		~Database();


		/*!
		* \brief	Tries to allocate a new DBC-Handle from the passed Environment and stores that
		*			DBC-Handle for later use internally. Will be freed on destruction.
		* \detailed	The Database will try to create a new Db-Connection using the Environment-handle
		*			from the passed Environment. This newly created Connection-Handle is stored 
		*			for later use. 
		*			The DBC-handle will be freed by the Database on destruction.
		*			Can only be called if no handle is allocated yet
		*
		* \param	env		The Environment to use to create this connection-handle.
		*						Do not free the Environment before you free the Database.
		*/
		bool		AllocateHdbc(const Environment& env);


		/*!
		* \deprecated
		* \todo	Untested leftover from wxWidgets.
		 * \brief	Connect using a prepared connection-String.
		 * 			Uses SQLDriverConnect without a window-handle to connect
		 * \param	connectStr			 		The connect string.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         Open(const std::wstring& inConnectStr);


		/*!
		 * \deprecated
		 * \todo	Untested leftover from wxWidgets.
		 * \brief	This version of Open will display the odbc source selection dialog, using SQLDriverConnect. Cast a
		 * 			wxWindow::GetHandle() to SQLHWND to use.
		 * \param	inConnectStr			 	The in connect string.
		 * \param	parentWnd				 	The parent window.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         Open(const std::wstring& inConnectStr, SQLHWND parentWnd);


		/*!
		 * \brief	Opens the connection using SQLConnect with the passed parameters.
		 * \param	Dsn						 	The dsn.
		 * \param	Uid						 	The UID.
		 * \param	AuthStr					 	The authentication string.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         Open(const std::wstring& Dsn, const std::wstring& Uid, const std::wstring& AuthStr);


		/*!
		 * \brief	Opens by using the information from the passed Environment
		 * \param [in,out]	pEnv	 	Pointer to the Environment to use to connect.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         Open(const Environment* const pEnv);


		/*!
		 * \brief	If this database is open, closes the stmt-handle and the connection to the db.
		 * \return	True if not open or closing the Db-connection works, False if closing the db-
		 * 			connection fails.
		 */
		bool         Close();


		enum ExecFailMode 
		{ 
			FailOnNoData,		///< Return False is ExecSql returns SQL_NO_DATA
			NotFailOnNoData		///< Return True if ExecSql returns SLQ_NO_DATA
		};
		/**
		 * \brief	Executes the SQL operation on the internal stmt-handle.
		 * \param	sqlStmt	The SQL statement.
		 * \param	mode   	If FailOnNoData is set, false is returned if SQL returns NO_DATA.
		 * 					This happens for example on DB2 if you do a DELETE with a WHERE
		 * 					clause and no records are deleted.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         ExecSql(const std::wstring& sqlStmt, ExecFailMode mode = NotFailOnNoData);


		/*!
		 * \brief	Commits all transaction associated with this database.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         CommitTrans();


		/*!
		 * \brief	Rolls back all transaction associated with this database.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool         RollbackTrans();


		/**
		 * \brief	Reads complete catalog. Queries the database using SQLTables with no search-string 
		 * 			set at all. All parameters all NULL. This is due to the fact that SQL_ATTR_METADATA_ID
		 * 			is not really implemented by all databases, so keep it simple.
		 * \param [in,out]	catalogInfo	Information describing the catalog.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadCompleteCatalog(SDbCatalogInfo& catalogInfo);


		/*!
		 * \brief	Reads all Catalogs that are defined in the DB. This calls SQLTables with 
		 * 			SQL_ALL_CATALOGS as catalog-name.
		 * \param [in,out]	catalogs	The catalogs.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadCatalogs(std::vector<std::wstring>& catalogs)		{ return ReadCatalogInfo(AllCatalogs, catalogs); };


		/*!
		 * \brief	Reads all schemas that are defined in the DB. This calls SQLTbles with
		 * 			SQL_ALL_SCHEMAS as schema-name.
		 * \param [in,out]	schemas	The schemas.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadSchemas(std::vector<std::wstring>& schemas)			{ return ReadCatalogInfo(AllSchemas, schemas); };


		/*!
		 * \brief	Reads all table types that are defined by the DB. This call SQLTables with
		 * 			SQL_ALL_TABLE_TYPES as table-type.
		 * \param [in,out]	tableTypes	List of types of the tables.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadTableTypes(std::vector<std::wstring>& tableTypes)	{ return ReadCatalogInfo(AllTableTypes, tableTypes); };


		/*!
		 * \brief	Queries the database using SQLColumns to determine the number of columns of
		 * 			the passed Table (which should have been queried from the catalog, using
		 * 			FindTable or similar).
		 * 			Note: No checks are done to ensure the passed table matches only one table
		 * 			of the database. You might get confusing results if you have for example
		 * 			search-patterns set as table name in the passed STableInfo table.
		 * \param	table	The table.
		 * \return	The column count, or -1 in case of failure
		 */
		int			ReadColumnCount(const STableInfo& table);


		/*!
		 * \brief	Reads column count for one table. First the database is queried for a table
		 * 			that matches the passed arguments. If not exactly one such table is found this
		 * 			function fails.
		 * 			If exactly one table is found, the call is forwarded to ReadColumnCount
		 * \param	tableName  	Name of the table.
		 * \param	schemaName 	Name of the schema.
		 * \param	catalogName	Name of the catalog.
		 * \param	tableType	Table Type name
		 *
		 * \return	The column count or -1 in case of failure
		 */
		int			ReadColumnCount(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType);


		/*!
		 * \brief	Reads table privileges for the table(s) matching the passed SCatalogTable description.
		 * \param	table			  	Defines the tablename, type, schema and catalog to search for privileges.
		 * \param [in,out]	privileges	The privileges.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadTablePrivileges(const STableInfo& table, TablePrivilegesVector& privileges);


		/*!
		 * \brief	Reads table privileges of exactly one table. This method will fail if not
		 * 			exactly one table is found matching the passed arguments.
		 * \param	tableName		  	Name of the table.
		 * \param	schemaName		  	Name of the schema.
		 * \param	catalogName		  	Name of the catalog.
		 * \param	tableType	Table Type name
		 * \param [in,out]	privileges	The privileges.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadTablePrivileges(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType, TablePrivilegesVector& privileges);


		/*!
		* \brief	Reads table primary keys of exactly one table. 
		* \param	table			  	Table definition to query primary keys.
		* \param [in,out]	primaryKeys	The primary Keys found.
		* \return	true if it succeeds, false if it fails.
		*/
		bool		ReadTablePrimaryKeys(const STableInfo& table, TablePrimaryKeysVector& primaryKeys);


		/*!
		 * \brief		Reads table column information for the passed table.
		 * \detailed	Returned table columns are ordered by TABLE_CAT, TABLE_SCHEM, TABLE_NAME, and ORDINAL_POSITION. 
		 * \param		table The table.
		 * \param [in,out]	columns	The columns.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadTableColumnInfo(const STableInfo& table, std::vector<SColumnInfo>& columns);


		/*!
		 * \brief	Reads table column information for exactly one table. This method will fail if the passed arguments
		 * 			do not match exactly one table.
		 * \param	tableName	   	Name of the table.
		 * \param	schemaName	   	Name of the schema.
		 * \param	catalogName	   	Name of the catalog.
		 * \param	tableType	Table Type name
		 * \param [in,out]	columns	The columns.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		ReadTableColumnInfo(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType, std::vector<SColumnInfo>& columns);


		/*!
		 * \brief	Searches for tables using SQLTables. If any of the parameters passed is empty, SQLTables will be called
		 * 			with a NULL value for that parameter, which indicates that we do not care about that param.
		 * 			The attribute SQL_ATTR_METADATA_ID should default to FALSE, so all parameters are treated as pattern value
		 * 			arguments (case sensitive, but you can use search patterns).
		 * 			See: http://msdn.microsoft.com/en-us/library/ms711831%28v=vs.85%29.aspx
		 * \param	tableName	  	Name of the table.
		 * \param	schemaName	  	Name of the schema.
		 * \param	catalogName   	Name of the catalog.
		 * \param	tableType	  	Type of the table.
		 * \param [in,out]	tables	The tables found that match the search-criteria.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		FindTables(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType, std::vector<STableInfo>& tables);


		/*!
		* \brief	Searches for tables that match the passed arguments, return true if exactly one such table is found.
		* 			The table that matches is copied to the argument table.
		* \param	tableName		  	Name of the table.
		* \param	schemaName		  	Name of the schema.
		* \param	catalogName		  	Name of the catalog.
		* \param	tableType		  	Table Type name.
		* \param [in,out]	table	  	The table.
		* \return	true if exactly one table matches the passed search-criterias name, schema and catalog, false else.
		*/
		bool			FindOneTable(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType, STableInfo& table);


		/*!
		 * \brief	Queries the database for the attribute SQL_ATTR_AUTOCOMMIT. The internal flag
		 * 			m_commitMode is updated with the value red from the database.
		 * \return	CM_UNKNOWN if fails, else the mode currently set
		 */
		CommitMode		ReadCommitMode();

		
		/*!
		 * \brief	Sets transaction mode on the database, using the attribute SQL_ATTR_AUTOCOMMIT.
		 * 			This will also update the internal flag m_commitMode, if changing the mode
		 * 			was successful.
		 *			This will always first Commit all ongoing transactions.
		 * \see		GetCommitMode()
		 * \param	mode	The mode to set.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool		SetCommitMode(CommitMode mode);

		
		/*!
		 * \brief	Gets transaction mode from the internally cached value.
		 * \see		SetCommitMode()
		 * \return	The transaction mode.
		 */
		CommitMode GetCommitMode() { return m_commitMode; };

		
		/*!
		* \brief	Queries the database for the attribute SQL_TXN_ISOLATION.
		* \return	TI_UNKNOWN if fails, else the mode currently set
		*/
		TransactionIsolationMode ReadTransactionIsolationMode();

		
		/*!
		* \brief	Sets transaction mode on the database, using the attribute SQL_TXN_ISOLATION.
		*			Calling this method will first close the internal statement-handle.
		*			If the transaction mode is set to TM_MANUAL it will then commit any ongoing transaction.
		* \param	mode	The mode to set.
		* \return	true if it succeeds, false if it fails.
		*/
		bool		SetTransactionIsolationMode(TransactionIsolationMode mode);

		
		/*!
		* \brief	Returns true if the database supports the mode passed.
		* \param	mode	The mode to test
		* \return	true if mode supported by database.
		*/
		bool		CanSetTransactionIsolationMode(TransactionIsolationMode mode);

	
		/*!
		* \brief	Get the environment used to Allocate this Database handle.
		* \return	NULL if AllocHdbc() was not called with success.
		*/
		const Environment* GetEnvironment() const { return m_pEnv; };

	
		/*!
		* \brief	Read the ODBC version supported by the driver.
		* \detailed	Fails if not connected. Does not read the version from the environment
		*			but from the SDbInfo populated during connecting to the database.
		* \return	ODBC version or OV_UNKNOWN.
		*/
		OdbcVersion GetDriverOdbcVersion() const;

	
		/*!
		* \brief	Determines which ODBC Version to use.
		* \detailed	Chooses the max ODBC Version that is supported by the environment and the driver.
		* \return	Max ODBC version supported by driver and env or OV_UNKNOWN.
		*/
		OdbcVersion GetMaxSupportedOdbcVersion() const;


		/*!
		* \brief	Get Database name as reported by driver during Open().
		* \return	Database name as reported by driver during Open().
		*/
		const std::wstring&	GetDatabaseName() const  { return m_dbInf.m_dbmsName; }


		/*!
		* \brief	Get DSN set during Open().
		* \return	DSN set during Open().
		*/
		const std::wstring& GetDataSourceName() const    { exDEBUG(!m_dbOpenedWithConnectionString);  return m_dsn; }
		

		/*!
		* \brief	Get DSN set during Open().
		* \return	DSN set during Open().
		*/
		const std::wstring& GetDatasourceName() const { exDEBUG(!m_dbOpenedWithConnectionString);  return m_dsn; }


		/*!
		* \brief	Get Username set during Open().
		* \return	Username set during Open().
		*/
		const std::wstring& GetUsername() const     { exDEBUG(!m_dbOpenedWithConnectionString);  return m_uid; }


		/*!
		* \brief	Get Password set during Open().
		* \return	Password set during Open().
		*/
		const std::wstring& GetPassword() const      { exDEBUG(!m_dbOpenedWithConnectionString);  return m_authStr; }


		/*!
		* \brief	Get Connection String passed during Open().
		* \return	Connection String passed during Open().
		*/
		const std::wstring& GetConnectionInStr() const  { exDEBUG(m_dbOpenedWithConnectionString); return m_inConnectionStr; }


		/*!
		* \brief	Get Connection String read from database after Open() was successful.
		* \return	Connection String read from database after Open() was successful.
		*/
		const std::wstring& GetConnectionOutStr() const { exDEBUG(m_dbOpenedWithConnectionString); exDEBUG(m_dbIsOpen);  return m_outConnectionStr; }


		/*!
		* \brief	Check if Open() was called using a connection string.
		* \return	True if Open() was called with a connection string.
		*/
		bool            OpenedWithConnectionString() const { return m_dbOpenedWithConnectionString; }


		/*!
		* \brief	Check if this database is connected.
		* \return	True if Open() was called successfully.
		*/
		bool            IsOpen() const           { return m_dbIsOpen; }


		/*!
		* \brief	Check if this database has a connection handle allocated.
		* \return	True if Connection handle is allocated.
		* \see		AllocateHdbc()
		*/
		bool			HasHdbc() const			{ return m_hdbc != SQL_NULL_HDBC; };


		/*!
		* \brief	Get an eventually allocated connection handle.
		* \return	Connection handle if allocated, or SQL_NULL_HDBC.
		* \see		AllocateHdbc()
		*/
		SQLHDBC            GetHDBC() const         { exDEBUG(HasHdbc()); return m_hdbc; }


		/*!
		* \brief	Get the SDbInfo if it is available.
		* \detailed	During Open() the database is queried about information about itself,
		*			if successful this information is stored in an SDbInfo internally.
		*			SDbInfo is empty until Open() was successful.
		* \return	SDbInfo with information corresponding to the database connected.
		*/
		SDbInfo GetDbInfo()	const				{ return m_dbInf; }


		/*!
		* \brief	Try to match the Database Name to a known DatabaseProduct value.
		* \return	DatabaseProduct value or dbmsUNIDENTIFIED.
		*/
		DatabaseProduct       Dbms();


		// Private stuff
		// -------------
	private:
		
		/*!
		* \brief	Queries SQLGetTypeInfo to fill the passed SSqlTypeInfo struct.
		* \param [in,out] types Filled with data types supported by the db
		* \return	True on success.
		*/
		bool			ReadDataTypesInfo(std::vector<SSqlTypeInfo>& types);

		
		/*!
		* \brief	Initialize all members to NULL / UNKNOWN.
		*/
		void			Initialize();


		/*!
		* \brief	Query the Database using SQLGetInfo.
		* \param [in,out] dbInfo SDbInfo to populate.
		* \return	True on success.
		*/
		bool			ReadDbInfo(SDbInfo& dbInfo);
		
		
		/*!
		* \brief	Set initial global attributs on the database connection handle.
		*/
		bool			SetConnectionAttributes();
		

		/*!
		* \brief	Do jobs common to all Open() implementations.
		*/
		bool             OpenImpl();

		
		enum ReadCatalogInfoMode
		{
			AllCatalogs,	///< Query all catalog names.
			AllSchemas,		///< Query all schema names.
			AllTableTypes	///< Query all table names.
		};
		/*!
		* \brief	Queries the database about catalog-, schema- or table-names
		* \param mode Determine which names to query.
		* \param [in,out] results names.
		*/
		bool			ReadCatalogInfo(ReadCatalogInfoMode mode, std::vector<std::wstring>& results);


		// Members
		// -------
		SDbInfo				m_dbInf;

		std::vector<SSqlTypeInfo> m_datatypes;	///< Queried from DB during Open
		bool				m_dbIsOpen;			///< Set to true after SQLConnect was successful
		bool				m_dbOpenedWithConnectionString;  ///< Was the database connection Opened with a connection string
		std::wstring		m_dsn;             ///< Data source name
		std::wstring		m_uid;             ///< User ID
		std::wstring		m_authStr;         ///< Authorization string (password)
		std::wstring		m_inConnectionStr; ///< Connection string used to connect to the database
		std::wstring		m_outConnectionStr;///< Connection string returned by the database when a connection is successfully OpenImpled
		DatabaseProduct		m_dbmsType;        ///< Type of datasource - i.e. Oracle, dBase, SQLServer, etc
		const Environment*	m_pEnv;				///< Environment used to create this Database

		// ODBC handles created by the Database
		SQLHDBC  m_hdbc;			///< ODBC DB Connection handle
		SQLHSTMT m_hstmt;			///< ODBC Statement handle used for all internal functions except ExecSql()
		SQLHSTMT m_hstmtExecSql;	///< ODBC Statement handle used for the function ExecSql()

		CommitMode		m_commitMode;	///< Commit Mode set currently

#if defined EXODBCDEBUG
		// Helpers to detect orphaned tables
	private:
		size_t	m_lastTableId;
		std::vector<STablesInUse> m_test;
		std::map<size_t, STablesInUse>	m_tablesInUse;
		size_t RegisterTable(const Table* const pTable);
		bool UnregisterTable(const Table* const pTable);
		friend class Table;
#endif


		// OLD STUFF we need to think about re-adding it
		// =============================================

		// LOTS OF DEFS
		// ============
		// 
		//// BJO 20000503: introduce new GetColumns members which are more database independent and
		////               return columns in the order they were created
		//#define OLD_GETCOLUMNS 1
		//#define EXPERIMENTAL_WXDB_FUNCTIONS 1

		//// This was defined by a macro testing if wxUNICODE is set
		//#define SQL_C_WXCHAR SQL_C_WCHAR

		//typedef float SFLOAT;
		//typedef double SDOUBLE;
		//typedef unsigned int UINT;
		//#define ULONG UDWORD

		//#ifndef wxODBC_FWD_ONLY_CURSORS
		//#define wxODBC_FWD_ONLY_CURSORS 1
		//#endif

		//enum enumDummy {enumDum1};

		//#ifndef SQL_C_BOOLEAN
		//#define SQL_C_BOOLEAN(datatype) (sizeof(datatype) == 1 ? SQL_C_UTINYINT : (sizeof(datatype) == 2 ? SQL_C_USHORT : SQL_C_ULONG))
		//#endif

		//#ifndef SQL_C_ENUM
		//#define SQL_C_ENUM (sizeof(enumDummy) == 2 ? SQL_C_USHORT : SQL_C_ULONG)
		//#endif

		//// NOTE: If SQL_C_BLOB is defined, and it is not SQL_C_BINARY, iODBC 2.x
		////       may not function correctly.  Likely best to use SQL_C_BINARY direct
		//#ifndef SQL_C_BLOB
		//#ifdef SQL_C_BINARY
		//#define SQL_C_BLOB SQL_C_BINARY
		//#endif
		//#endif

		//#ifndef _WIN64
		//#ifndef SQLLEN
		//#define SQLLEN SQLINTEGER
		//#endif
		//#ifndef SQLULEN
		//#define SQLULEN SQLUINTEGER
		//#endif
		//#endif

		// THE OLD COLUMN INFO
		// ===================
		//
		//class EXODBCAPI ColumnInfo
		//{
		//public:
		//	ColumnInfo();
		//	~ColumnInfo();

		//	bool Initialize();

		//	wchar_t			m_catalog[128+1];
		//	wchar_t			m_schema[128+1];
		//	wchar_t			m_tableName[DB_MAX_TABLE_NAME_LEN_DEFAULT+1];
		//	wchar_t			m_colName[DB_MAX_COLUMN_NAME_LEN+1];
		//	SWORD			m_sqlDataType;
		//	wchar_t			m_typeName[128+1];
		//	SWORD			m_columnLength;
		//	SWORD			m_bufferSize;
		//	short			m_decimalDigits;
		//	short			m_numPrecRadix;
		//	short			m_nullable;
		//	wchar_t			m_remarks[254+1];
		//	int				m_dbDataType;  // conversion of the 'sqlDataType' to the generic data type used by these classes
		//	// mj10777.19991224 : new
		//	int				m_pkCol;       // Primary key column       0=No; 1= First Key, 2 = Second Key etc.
		//	wchar_t			m_pkTableName[DB_MAX_TABLE_NAME_LEN_DEFAULT+1]; // Tables that use this PKey as a FKey
		//	int				m_fkCol;       // Foreign key column       0=No; 1= First Key, 2 = Second Key etc.
		//	wchar_t			m_fkTableName[DB_MAX_TABLE_NAME_LEN_DEFAULT+1]; // Foreign key table name
		//	ColumnFormatter* m_pColFor;                              // How should this columns be formatted

		//};
		//
		// AND THIS BELOW HERE WAS ALL PART OF wxDB
		// ========================================
		//
		// public:
		//bool         DispAllErrors(HENV aHenv, HDBC aHdbc = SQL_NULL_HDBC, HSTMT aHstmt = SQL_NULL_HSTMT);
		//		bool         GetNextError(HENV aHenv, HDBC aHdbc = SQL_NULL_HDBC, HSTMT aHstmt = SQL_NULL_HSTMT);
		//		void         DispNextError();
		//		bool         CreateView(const std::wstring& viewName, const std::wstring& colList, const std::wstring& pSqlStmt, bool attemptDrop = true);
		//		bool         DropView(const std::wstring& viewName);
		//		bool         ExecSql(const std::wstring& pSqlStmt, ColumnInfo** columns, short& numcols);
		//		bool         GetNext();
		//		bool         Grant(int privileges, const std::wstring& tableName, const std::wstring& userList = L"PUBLIC");
		//		int          TranslateSqlState(const std::wstring& SQLState);
		//		bool         Catalog(const wchar_t* userID = NULL, const std::wstring& fileName = SQL_CATALOG_FILENAME);
		//		int          GetKeyFields(const std::wstring& tableName, ColumnInfo* colInf, UWORD noCols);
		//
		//ColumnInfo*		GetColumns(wchar_t* tableName[], const wchar_t* userID = NULL);
		//ColumnInfo*		GetColumns(const std::wstring& tableName, UWORD* numCols, const wchar_t* userID=NULL);
		//
		//		int					GetColumnCount(const std::wstring& tableName, const wchar_t* userID=NULL);
		//HSTMT           GetHSTMT()         {return m_hstmt;}
		//SSqlTypeInfo GetTypeInfVarchar()    {return m_typeInfVarchar;}
		//SSqlTypeInfo GetTypeInfInteger()    {return m_typeInfInteger;}
		//SSqlTypeInfo GetTypeInfFloat()      {return m_typeInfFloat;}
		//SSqlTypeInfo GetTypeInfDate()       {return m_typeInfDate;}
		//SSqlTypeInfo GetTypeInfBlob()       {return m_typeInfBlob;}
		//SSqlTypeInfo GetTypeInfMemo()       {return m_typeInfMemo;}
		//
		//// tableName can refer to a table, view, alias or synonym
		//		bool         TableExists(const std::wstring& tableName, const wchar_t* userID = NULL, const std::wstring& tablePath = std::wstring());
		//		bool         TablePrivileges(const std::wstring& tableName, const std::wstring& priv, const wchar_t* userID = NULL, const wchar_t* schema = NULL, const std::wstring& path = std::wstring());
		//
		//// These two functions return the table name or column name in a form ready
		//// for use in SQL statements.  For example, if the datasource allows spaces
		//// in the table name or column name, the returned string will have the
		//// correct enclosing marks around the name to allow it to be properly
		//// included in a SQL statement
		//const std::wstring  SQLTableName(const wchar_t* tableName);
		//const std::wstring  SQLColumnName(const wchar_t* colName);
		//
		//		void         LogError(const std::wstring& errMsg, const std::wstring& SQLState = std::wstring()) { LogErrorImpl(errMsg, SQLState); }
		//		void         SetDebugErrorMessages(bool state) { m_silent = !state; }
		//		bool         SetSqlLogging(wxDbSqlLogState state, const std::wstring& filename = SQL_LOG_FILENAME, bool append = false);
		//		bool         WriteSqlLog(const std::wstring& logMsg);
		//
		//		std::vector<std::wstring> GetErrorList() const;
		//
		//bool         ModifyColumn(const std::wstring& tableName, const std::wstring& columnName, int dataType, ULONG columnLength = 0, const std::wstring& optionalParam = std::wstring());
		//
		//bool         FwdOnlyCursors()  {return m_fwdOnlyCursors;}
		//
		// return the string with all special SQL characters escaped
		//std::wstring     EscapeSqlChars(const std::wstring& value);
		//
		//
		// private:
		//		void			LogErrorImpl(const std::wstring& errMsg, const std::wstring& SQLState);
		//		std::wstring	ConvertUserIDImpl(const wchar_t* userID);
		//		bool             DetermineDataTypes(bool failOnDataTypeUnsupported);
		//		bool				m_fwdOnlyCursors;
		//		FILE*				m_fpSqlLog;        // Sql Log file pointer
		//		wxDbSqlLogState		m_sqlLogState;     // On or Off
		//// Information about logical data types VARCHAR, INTEGER, FLOAT and DATE.
		////
		//// This information is obtained from the ODBC driver by use of the
		//// SQLGetTypeInfo() function.  The key piece of information is the
		//// type name the data source uses for each logical data type.
		//// e.g. VARCHAR; Oracle calls it VARCHAR2.
		//SSqlTypeInfo m_typeInfVarchar;
		//SSqlTypeInfo m_typeInfInteger;
		//SSqlTypeInfo m_typeInfFloat;
		//SSqlTypeInfo m_typeInfDate;
		//SSqlTypeInfo m_typeInfBlob;
		//SSqlTypeInfo m_typeInfMemo;


	};  // Database

}	// namespace exodbc

#endif // DB_H
