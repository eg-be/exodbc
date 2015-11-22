/*!
 * \file Database.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 25.07.2014
 * \brief Header file for the Database class and its helpers.
 * \copyright GNU Lesser General Public License Version 3
 *
 * Header file for the Database class and its helpers.
 */

#pragma once
#ifndef DATABASE_H
#define DATABASE_H

// Same component headers
#include "exOdbc.h"
#include "Helpers.h"
#include "InfoObject.h"
#include "Sql2BufferTypeMap.h"
#include "SqlHandle.h"
#include "Environment.h"

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
	class Table;

	// Structs
	// -------

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
		FRIEND_TEST(DatabaseTest, ReadDataTypesInfo); 
		FRIEND_TEST(DatabaseTest, SetConnectionAttributes);
		FRIEND_TEST(DatabaseTest, ReadDbInfo);
#endif
	public:
		/*!
		* \brief	Default Constructor. You will need to manually call Init() later.
		* \details	Default Constructor. You will need to call Init() 
		*			afterwards to allocate the Database-handle.
		*			Creates a new DefaultSql2BufferTypeMap to be used with this Database.
		* \see		Init()
		*/
		Database() throw();


		/*!
		* \brief	Create a new Database-instance. The instance will be using the passed
		*			Environment.
		* \details	The Database will try to create a new Db-Connection handle during construction.
		*			The handle will be freed by the Database on destruction.
		* \param	pEnv		The Environment to use to create this database and its connection.
		*						Do not free the Environment before you free the Database.
		* \throw	Exception If handles cannot be allocated, or passed Environment does not have an
		*			Environment handle to be used.
		*/
		Database(ConstEnvironmentPtr pEnv);

		
		/*!
		* \brief	Copy constructor. Creates a new Database that is dependent on the same
		*			Environment as this Database.
		*/
		Database(const Database& other);


		~Database();


		/*!
		* \brief	Must be called if Database has been created using Default Constructor.
		* \details	Can only be called once. Allocates the Connection handle from the passed
		*			Environment. Do not free the passed Environment before you free the database.
		* \throw	Exception
		*/
		void Init(ConstEnvironmentPtr pEnv);


		/*!
		 * \brief	Connect using a prepared connection-String.
		 * \param	inConnectStr	The connect string.
		 * \return	The output-connection string returned by SQLDriverConnect().
		 * \throw	Exception If Opening the Connection fails, or no Database handle is allocated.
		 */
		std::wstring	Open(const std::wstring& inConnectStr);


		/*!
		 * \brief	If the passed parentWnd is not NULL but the handle of a parent window,
		 *			the dialog to create a connection string is shown.
		 * \param	inConnectStr	The in connect string.
		 * \param	parentWnd		The parent window.
		 * \return	The output-connection string returned by SQLDriverConnect().
		 * \throw	Exception If opening fails.
		 */
		std::wstring	Open(const std::wstring& inConnectStr, SQLHWND parentWnd);


		/*!
		 * \brief	Opens the connection using SQLConnect with the passed parameters.
		 * \param	dsn		The dsn.
		 * \param	uid		The UID.
		 * \param	authStr	The authentication string (password).
		 * \throw	Exception If Opening the Connection fails, or no Database handle is allocated.
		 */
		void         Open(const std::wstring& dsn, const std::wstring& uid, const std::wstring& authStr);


		/*!
		 * \brief		If this database is open, closes the stmt-handle and the connection to the db.
		 * \details	This function will fail if any of the handles cannot be freed.
		 *				This function will rollback any open transaction if manual commit mode is set.
		 * \details	If the database is not open, this function does nothing
		 * \throw		Exception If closing the db-connection fails. 
		 */
		void         Close();


		enum class ExecFailMode 
		{ 
			FailOnNoData,		///< Return False is ExecSql returns SQL_NO_DATA
			NotFailOnNoData		///< Return True if ExecSql returns SLQ_NO_DATA
		};
		/**
		 * \brief	Executes the SQL operation on the internal stmt-handle.
		 * \param	sqlStmt	The SQL statement.
		 * \param	mode   	If FailOnNoData is set, Exception is thrown if SQL returns NO_DATA.
		 * 					This happens for example on DB2 if you do a DELETE with a WHERE
		 * 					clause and no records are deleted.
		 * \throw	Exception If executing SQL failed, or depending on mode if no records are affected.
		 */
		void         ExecSql(const std::wstring& sqlStmt, ExecFailMode mode = ExecFailMode::NotFailOnNoData);


		/*!
		 * \brief	Commits all transaction associated with this database.
		 * \return	true if it succeeds, false if it fails.
		 * \throw	Exception if no connection handle is allocated or SQLEndTran fails.
		 */
		void         CommitTrans() const;


		/*!
		 * \brief	Rolls back all transaction associated with this database.
		 * \return	true if it succeeds, false if it fails.
		 * \throw	Exception if no connection handle is allocated or SQLEndTran fails.
		 */
		void         RollbackTrans() const;


		/**
		 * \brief	Reads complete catalog. Queries the database using SQLTables with no search-string 
		 * 			set at all. All parameters all NULL. This is due to the fact that SQL_ATTR_METADATA_ID
		 * 			is not really implemented by all databases, so keep it simple.
		 * \return	CatalogInfo
		 * \throw	Exception If reading fails.
		 */
		SDbCatalogInfo	ReadCompleteCatalog();


		/*!
		 * \brief	Reads all Catalogs that are defined in the DB. This calls SQLTables with 
		 * 			SQL_ALL_CATALOGS as catalog-name.
		 * \return	Catalog names found.
		 * \throw	Exception If reading catalogs fails.
		 */
		std::vector<std::wstring>	ReadCatalogs()		{ return ReadCatalogInfo(ReadCatalogInfoMode::AllCatalogs); };


		/*!
		 * \brief	Reads all schemas that are defined in the DB. This calls SQLTbles with
		 * 			SQL_ALL_SCHEMAS as schema-name.
		 * \return	Schema names found.
		 * \throw	Exception if reading schemas fails or if the Database is an Access database (Access fails with 
		 *			SQLSTATE HYC00; Native Error: 106; [Microsoft][ODBC Microsoft Access Driver]Optional feature not implemented )
		 */
		std::vector<std::wstring>	ReadSchemas()			{ exASSERT(GetDbms() != DatabaseProduct::ACCESS);  return ReadCatalogInfo(ReadCatalogInfoMode::AllSchemas); };


		/*!
		 * \brief	Reads all table types that are defined by the DB. This call SQLTables with
		 * 			SQL_ALL_TABLE_TYPES as table-type.
		 * \return	Table type names found.
		 * \throw	Exception If reading table types fails.
		 */
		std::vector<std::wstring>	ReadTableTypes()	{ return ReadCatalogInfo(ReadCatalogInfoMode::AllTableTypes); };


		/*!
		 * \brief	Queries the database using SQLColumns to determine the number of columns of
		 * 			the passed Table (which should have been queried from the catalog, using
		 * 			FindTable or similar).
		 * 			Note: No checks are done to ensure the passed table matches only one table
		 * 			of the database. You might get confusing results if you have for example
		 * 			search-patterns set as table name in the passed TableInfo table.
		 * \param	table	The table.
		 * \return	The column count.
		 * \throw	Exception If counting columns fails.
		 */
		int			ReadColumnCount(const TableInfo& table);


		/*!
		 * \brief	Reads column count for one table. First the database is queried for a table
		 * 			that matches the passed arguments. If not exactly one such table is found this
		 * 			function fails.
		 * 			If exactly one table is found, the call is forwarded to ReadColumnCount
		 * \param	tableName  	Name of the table.
		 * \param	schemaName 	Name of the schema.
		 * \param	catalogName	Name of the catalog.
		 * \param	tableType	Table Type name
		 * \return	The column count for the matching table.
		 * \throw	Exception If not exactly one table is found.
		 */
		int			ReadColumnCount(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType);


		/*!
		 * \brief	Reads table privileges for the table(s) matching the passed SCatalogTable description.
		 * \param	table		Defines the tablename, type, schema and catalog to search for privileges.
		 * \return	Privileges	matching passed table.
		 * \throw	Exception	If reading privileges fails.
		 */
		TablePrivilegesVector	ReadTablePrivileges(const TableInfo& table) const;


		/*!
		 * \brief	Reads table privileges of exactly one table. This method will fail if not
		 * 			exactly one table is found matching the passed arguments.
		 * \param	tableName		  	Name of the table.
		 * \param	schemaName		  	Name of the schema.
		 * \param	catalogName		  	Name of the catalog.
		 * \param	tableType	Table Type name
		 * \return	Privileges for exactly one matching table.
		 * \throw	Exception	If reading privileges fails, or not exactly one table matches.
		 */
		TablePrivilegesVector	ReadTablePrivileges(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType) const;


		/*!
		* \brief	Reads table primary keys of exactly one table. 
		* \param	table			  	Table definition to query primary keys.
		* \return	The primary Keys found.
		* \throw	If reading primary keys fails.
		*/
		TablePrimaryKeysVector		ReadTablePrimaryKeys(const TableInfo& table) const;


		/*!
		 * \brief		Reads table column information for the passed table.
		 * \details	Returned table columns are ordered by TABLE_CAT, TABLE_SCHEM, TABLE_NAME, and ORDINAL_POSITION. 
		 * \param		table The table.
		 * \return		Columns of passed table.
		 * \throw Exception If reading ColumnInfo fails.
		 */
		ColumnInfosVector	ReadTableColumnInfo(const TableInfo& table) const;


		/*!
		 * \brief	Reads table column information for exactly one table. This method will fail if the passed arguments
		 * 			do not match exactly one table.
		 * \param	tableName	   	Name of the table.
		 * \param	schemaName	   	Name of the schema.
		 * \param	catalogName	   	Name of the catalog.
		 * \param	tableType	Table Type name
		 * \return		Columns of passed table.
		 * \throw Exception If reading ColumnInfo fails or not exactly one table matches.
		 */
		ColumnInfosVector	ReadTableColumnInfo(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType) const;


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
		 * \return	The tables found that match the search-criteria.
		 * \throw Exception			If querying the database fails.
		 */
		TableInfosVector		FindTables(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType) const;


		/*!
		* \brief	Searches for tables that match the passed arguments, return the table if exactly one such table is found.
		* \param	tableName		  	Name of the table.
		* \param	schemaName		  	Name of the schema.
		* \param	catalogName		  	Name of the catalog.
		* \param	tableType		  	Table Type name.
		* \return	The table info of exactly one table that matches the passed search-criterias name, schema and catalog. Throws otherwise
		* \throw	If not exactly one table can be found
		*/
		TableInfo		FindOneTable(const std::wstring& tableName, const std::wstring& schemaName, const std::wstring& catalogName, const std::wstring& tableType) const;


		/*!
		 * \brief	Queries the database for the attribute SQL_ATTR_AUTOCOMMIT. The internal flag
		 * 			m_commitMode is updated with the value red from the database, if reading
		 *			was successful. In all other cases it is set to CM_UNKNOWN.
		 * \return	The mode currently set.
		 * \throw	Exception If reading the commit mode fails or no connection handle is allocated.
		 */
		CommitMode		ReadCommitMode();

		
		/*!
		 * \brief	Sets transaction mode on the database, using the attribute SQL_ATTR_AUTOCOMMIT.
		 * 			This will also update the internal flag m_commitMode, if changing the mode
		 * 			was successful.
		 *			This will always first Rollback all ongoing transactions.
		 * \see		GetCommitMode()
		 * \param	mode	The mode to set.
		 * \throw	Exception If setting on database fails or no connection handle is allocated. 
		 *			Will not update internal flag in that case.
		 */
		void		SetCommitMode(CommitMode mode);

		
		/*!
		 * \brief	Gets transaction mode from the internally cached value.
		 * \see		SetCommitMode()
		 * \return	The transaction mode.
		 */
		CommitMode GetCommitMode() const { return m_commitMode; };

		
		/*!
		* \brief	Queries the database for the attribute SQL_TXN_ISOLATION.
		* \return	The mode currently set or TI_UNKNOWN if read mode is unknown.
		* \throw	Exception If reading from the database fails.
		*/
		TransactionIsolationMode ReadTransactionIsolationMode();

		
		/*!
		* \brief	Sets transaction mode on the database, using the attribute SQL_TXN_ISOLATION.
		*			Calling this method will first close the internal statement-handle.
		*			If the transaction mode is set to TM_MANUAL it will then Rollback any ongoing transaction.
		* \param	mode	The mode to set.
		* \throw	Exception if setting mode fails.
		*/
		void		SetTransactionIsolationMode(TransactionIsolationMode mode);

		
		/*!
		* \brief	Returns true if the database supports the mode passed.
		* \param	mode	The mode to test
		* \return	true if mode supported by database.
		*/
		bool		CanSetTransactionIsolationMode(TransactionIsolationMode mode) const;
	
	
		/*!
		* \brief	Read the ODBC version supported by the driver.
		* \details	Fails if not connected. Does not read the version from the environment
		*			but from the DatabaseInfo populated during connecting to the database.
		* \return	ODBC version or OV_UNKNOWN.
		* \throw	Exception If parsing the version returned from the driver fails, or the version
		*			from the driver has not yet been read (is read during Open()).
		*/
		OdbcVersion GetDriverOdbcVersion() const;

	
		/*!
		* \brief	Determines which ODBC Version to use.
		* \details	Chooses the max ODBC Version that is supported by the environment and the driver.
		* \return	Max ODBC version supported by driver and env or OV_UNKNOWN.
		* \throw	Exception 
		*/
		OdbcVersion GetMaxSupportedOdbcVersion() const;


		/*!
		* \brief	Get DSN set during Open().
		* \return	DSN set during Open().
		* \throw	Exception
		*/
		const std::wstring& GetDataSourceName() const    { exASSERT(!m_dbOpenedWithConnectionString);  return m_dsn; }
		

		/*!
		* \brief	Get DSN set during Open().
		* \return	DSN set during Open().
		* \throw	Exception
		*/
		const std::wstring& GetDatasourceName() const { exASSERT(!m_dbOpenedWithConnectionString);  return m_dsn; }


		/*!
		* \brief	Get Username set during Open().
		* \return	Username set during Open().
		* \throw	Exception
		*/
		const std::wstring& GetUsername() const     { exASSERT(!m_dbOpenedWithConnectionString);  return m_uid; }


		/*!
		* \brief	Get Password set during Open().
		* \return	Password set during Open().
		* \throw	Exception
		*/
		const std::wstring& GetPassword() const      { exASSERT(!m_dbOpenedWithConnectionString);  return m_authStr; }


		/*!
		* \brief	Get Connection String passed during Open().
		* \return	Connection String passed during Open().
		* \throw	Exception
		*/
		const std::wstring& GetConnectionInStr() const  { exASSERT(m_dbOpenedWithConnectionString); return m_inConnectionStr; }


		/*!
		* \brief	Get Connection String read from database after Open() was successful.
		* \return	Connection String read from database after Open() was successful.
		* \throw	Exception
		*/
		const std::wstring& GetConnectionOutStr() const { exASSERT(m_dbOpenedWithConnectionString); exASSERT(m_dbIsOpen);  return m_outConnectionStr; }


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
		* \see		AllocateConnectionHandle()
		*/
		bool			HasConnectionHandle() const { exASSERT(m_pHDbc); return m_pHDbc->IsAllocated(); };


		/*!
		* \brief	Get an eventually allocated connection handle.
		* \return	Connection handle if allocated, or SQL_NULL_HDBC.
		* \see		AllocateConnectionHandle()
		* \throw	Exception If no handle is allocated.
		*/
		ConstSqlDbcHandlePtr	GetConnectionHandle() const         { exASSERT(HasConnectionHandle()); return m_pHDbc; }


		/*!
		* \brief	Get the DatabaseInfo if it is available.
		* \details	During Open() the database is queried about information about itself,
		*			if successful this information is stored in an DatabaseInfo internally.
		*			DatabaseInfo is empty until Open() was successful.
		* \return	DatabaseInfo with information corresponding to the database connected.
		* \throw	Exception If database is not open yet and therefore DatabaseInfo is unknown.
		*/
		DatabaseInfo GetDbInfo()	const				{ exASSERT(IsOpen());  return m_dbInf; }


		/*!
		* \brief	Try to match the Database Name to a known DatabaseProduct value.
		* \return	DatabaseProduct value or DatabaseProduct::UNKNOWN.
		*/
		DatabaseProduct       GetDbms() const { return m_dbmsType; };


		/*!
		* \brief	Get the Environment this Database was created from.
		* \return	Environment if set.
		*/
		ConstEnvironmentPtr	GetEnvironment() const { return m_pEnv; };


		/*!
		* \brief	Set Attribute SQL_ATTR_TRACEFILE to passed path.
		* \throw	Exception
		*/
		void SetTracefile(const std::wstring path);


		/*!
		* \brief	Get Attribute SQL_ATTR_TRACEFILE.
		* \throw	Exception
		*/
		std::wstring GetTracefile() const;


		/*!
		* \brief	Set Attribute SQL_ATTR_TRACE.
		* \throw	Exception
		*/
		void SetTrace(bool enable);


		/*!
		* \brief	Get Attribute SQL_ATTR_TRACE.
		* \throw	Exception
		*/
		bool GetTrace() const;


#ifdef HAVE_MSODBCSQL_H
		/*!
		* \brief	Set the property SQL_MAX_CONCURRENT_ACTIVITIES.
		*			MS SQL Server specific.
		*			Must be called before the database is opened.
		* \details	Only available if HAVE_MSODBCSQL_H is defined.
		* \throw	Exception
		*/
		void SetMarsEnabled(bool enableMars);
#endif


		/*!
		* \brief	Returns if Multiple Active RecordSets are supported.
		*			For MS SQL Server, if HAVE_MSODBCSQL_H is defined,
		*			queries the property SQL_COPT_SS_MARS_ENABLED.
		*			For all other databases this returns true 
		*			if the value of the connection attribute
		*			SQL_MAX_CONCURRENT_ACTIVITIES read during Open()
		*			is == 0 or > 1.
		* \return	SQL_COPT_SS_MARS_ENABLED or SQL_MAX_CONCURRENT_ACTIVITIES != 1
		*/
		bool IsMarsEnabled();


		/*!
		* \brief	Test if the passed SQL Type has been reported as supported
		*			by the Database.
		* \return	Searches the DataTypes info and compares against SqlType (ODBC 2.0)
		*			and SqlDataType (ODBC 3.0)
		* \throw	Exception If not Open().
		*/
		bool IsSqlTypeSupported(SQLSMALLINT sqlType) const;


		/*!
		* \brief	Set a Sql2BufferTypeMap to be used by this Database from now on.
		* \param pSql2BufferTypeMap	Sql2BufferTypeMap to set.
		*/
		void		SetSql2BufferTypeMap(Sql2BufferTypeMapPtr pSql2BufferTypeMap) throw();


		/*!
		* \brief	Get the Sql2BufferTypeMap set on this Database.
		* \return	const Sql2BufferTypeMap*
		* \throw	Exception If no Sql2BufferTypeMap is set on this Database.
		*/
		Sql2BufferTypeMapPtr GetSql2BufferTypeMap() const;


		/*!
		* \brief	Get the SqlTypeInfo queried during open..
		* \return	SqlTypeInfosVector
		* \throw	Exception If not Open().
		*/
		SqlTypeInfosVector GetTypeInfos() const;


		// Private stuff
		// -------------
	private:
		
		/*!
		* \brief	Set the internal member m_dbmsType by examining m_dbInf.m_dbmsName
		*/
		void		DetectDbms();


		/*!
		* \brief	Queries SQLGetTypeInfo to populate a SSqlTypeInfo struct.
		* \return	Types Filled with data types supported by the db
		* \throw	Exception If the internal statement handle is not allocated, or reading fails.
		*/
		SqlTypeInfosVector ReadDataTypesInfo();


		/*!
		* \brief	Tries to allocate a new DBC-Handle from the internally stored Environment. 
		*			Handle will be freed on destruction.
		* \details	Can only be called if no connection-handle is allocated yet.
		* \throw	Exception If a handle is already allocated or allocating fails,
		*/
		//void		AllocateConnectionHandle();


		/*!
		* \brief	Frees the database connection handle.
		* \throw	SqlResultException if SQLFreeHandle returns SQL_ERROR or SQL_INVALID_HANDLE
		*			Exception If no database connection handle is allocated.
		*/
		//void			FreeConnectionHandle();


		/*!
		* \brief	Query the Database using SQLGetInfo.
		* \return	DatabaseInfo populated with values.
		* \throw	Exception If reading Database info fails or no connection handle is allocated.
		*/
		DatabaseInfo			ReadDbInfo();
		
		
		/*!
		* \brief	Set initial global attributes on the database connection handle.
		* \throw	Exception If no connection handle is allocated, or setting an attribute fails.
		*/
		void			SetConnectionAttributes();
		

		/*!
		* \brief	Do jobs common to all Open() implementations.
		* \detailed	This function will ensure that if it fails no handles
		*			are left in an allocated state.
		* \throw	Exception If anything goes wrong.
		*/
		void             OpenImpl();

		
		enum class ReadCatalogInfoMode
		{
			AllCatalogs,	///< Query all catalog names.
			AllSchemas,		///< Query all schema names.
			AllTableTypes	///< Query all table names.
		};
		/*!
		* \brief	Queries the database about catalog-, schema- or table-names
		* \param mode Determine which names to query.
		* \returns  Result containing of names.
		* \throw	Exception If reading fails.
		*/
		std::vector<std::wstring>	ReadCatalogInfo(ReadCatalogInfoMode mode);


		// Members
		// -------
		ConstEnvironmentPtr		m_pEnv;		///< Environment of this Databaes
		DatabaseInfo			m_dbInf;
		Sql2BufferTypeMapPtr	m_pSql2BufferTypeMap;	///< Sql2BufferTypeMap to be used from this Database. If none is set during OpenImp() a DefaultSql2BufferTypeMap is created.

		SqlTypeInfosVector m_datatypes;	///< Queried from DB during Open
		bool				m_dbIsOpen;			///< Set to true after SQLConnect was successful
		bool				m_dbOpenedWithConnectionString;  ///< Was the database connection Opened with a connection string
		std::wstring		m_dsn;             ///< Data source name
		std::wstring		m_uid;             ///< User ID
		std::wstring		m_authStr;         ///< Authorization string (password)
		std::wstring		m_inConnectionStr; ///< Connection string used to connect to the database
		std::wstring		m_outConnectionStr;///< Connection string returned by the database when a connection is successfully OpenImpled
		DatabaseProduct		m_dbmsType;        ///< Type of datasource - i.e. Oracle, dBase, SQLServer, etc

		// ODBC handles created by the Database

		SqlDbcHandlePtr m_pHDbc;			///< ODBC DB Connection handle
		SqlStmtHandlePtr m_pHStmt;			///< ODBC Statement handle used for all internal functions except ExecSql()
		SqlStmtHandlePtr m_pHStmtExecSql;	///< ODBC Statement handle used for the function ExecSql()
		//SQLHDBC  m_hdbc;			///< ODBC DB Connection handle
		//SQLHSTMT m_hstmt;			///< ODBC Statement handle used for all internal functions except ExecSql()
		//SQLHSTMT m_hstmtExecSql;	///< ODBC Statement handle used for the function ExecSql()

		CommitMode		m_commitMode;	///< Commit Mode set currently

	};  // Database

	typedef std::shared_ptr<Database> DatabasePtr;
	typedef std::shared_ptr<const Database> ConstDatabasePtr;
}	// namespace exodbc

#endif // DATABASE_H

