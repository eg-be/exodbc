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

// Same component headers
#include "exOdbc.h"
#include "TableInfo.h"
#include "DatabaseCatalog.h"
#include "Sql2BufferTypeMap.h"
#include "SqlHandle.h"
#include "Environment.h"
#include "SqlInfoProperty.h"

// Other headers
#if EXODBC_TEST
	#include <gtest/gtest_prod.h>
#endif

// System headers
#include <vector>
#include <map>
#include <set>
#include <sql.h>
#include <sqlext.h>
#include <sqlucode.h>
#include <odbcinst.h>

#if EXODBC_TEST
namespace exodbctest
{
	class DatabaseTest_ReadDataTypesInfo_Test;
	class DatabaseTest_SetConnectionAttributes_Test;
	class DatabaseTest_ReadDbInfo_Test;
	class DatabaseTest_CommitTransaction_Test;
	class DatabaseTest_RollbackTransaction_Test;
}
#endif

// Forward declarations
// --------------------
namespace exodbc
{
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
	* Database will Rollback any ongoing transaction first.
	* There is basic support executing SQL on the Database directly,
	* using the generic ExecSql() and CommitTrans() functions.
	* The ExecSql() function uses its own Statement, so it will never be influenced
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
		// see: http://stackoverflow.com/questions/13171077/friend-test-in-google-test-possible-circular-dependency
		// and: http://www.codeproject.com/Articles/484817/Google-e2-80-99splusgtestplusnotplushandlingplusfr
		friend class exodbctest::DatabaseTest_ReadDataTypesInfo_Test;
		friend class exodbctest::DatabaseTest_SetConnectionAttributes_Test;
		friend class exodbctest::DatabaseTest_ReadDbInfo_Test;
		friend class exodbctest::DatabaseTest_CommitTransaction_Test;
		friend class exodbctest::DatabaseTest_RollbackTransaction_Test;
#endif
	public:
		/*!
		* \enum	CommitMode
		* \brief	Defines whether auto commit is on or off.
		* 			see: http://msdn.microsoft.com/en-us/library/ms713600%28v=vs.85%29.aspx
		*/
		enum class CommitMode
		{
			UNKNOWN = 50000,			///< Unknown Commit mode
			AUTO = SQL_AUTOCOMMIT,		///< Autocommit on
			MANUAL = SQL_AUTOCOMMIT_OFF	///< Autocommit off
		};


		/*!
		* \enum	TransactionIsolationMode
		*
		* \brief	Defines the Transaction Isolation Mode
		*			see: http://msdn.microsoft.com/en-us/library/ms709374%28v=vs.85%29.aspx
		*/
		enum class TransactionIsolationMode
		{
			UNKNOWN = 50000,								///< Unknown Transaction Isolation Level
			READ_UNCOMMITTED = SQL_TXN_READ_UNCOMMITTED,	///< Read Uncommitted
			READ_COMMITTED = SQL_TXN_READ_COMMITTED,		///< Read Committed
			REPEATABLE_READ = SQL_TXN_REPEATABLE_READ,		///< Repeatable Read
			SERIALIZABLE = SQL_TXN_SERIALIZABLE				///< Serializable
		};


		/*!
		* \brief	Default Constructor. You will need to manually call Init() later.
		* \details	Default Constructor. You will need to call Init() 
		*			afterwards to allocate the Database-handle.
		*			Creates a new DefaultSql2BufferTypeMap to be used with this Database.
		* \see		Init()
		*/
		Database() noexcept;


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
		* \brief	Create new Database using passed ODBC Environment. Created Database
		*			is wrapped into a shared_ptr.
		*/
		static std::shared_ptr<Database> Create(ConstEnvironmentPtr pEnv);


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
		std::string	Open(const std::string& inConnectStr);


		/*!
		 * \brief	If the passed parentWnd is not NULL but the handle of a parent window,
		 *			the dialog to create a connection string is shown.
		 * \param	inConnectStr	The in connect string.
		 * \param	parentWnd		The parent window.
		 * \return	The output-connection string returned by SQLDriverConnect().
		 * \throw	Exception If opening fails.
		 */
		std::string	Open(const std::string& inConnectStr, SQLHWND parentWnd);


		/*!
		 * \brief	Opens the connection using SQLConnect with the passed parameters.
		 * \param	dsn		The dsn.
		 * \param	uid		The UID.
		 * \param	authStr	The authentication string (password).
		 * \throw	Exception If Opening the Connection fails, or no Database handle is allocated.
		 */
		void         Open(const std::string& dsn, const std::string& uid, const std::string& authStr);


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
		 * \brief	Executes the SQL operation on the internal exec Sql statement handle.
		 * \param	sqlStmt	The SQL statement.
		 * \param	mode   	If FailOnNoData is set, Exception is thrown if SQL returns NO_DATA.
		 * 					This happens for example on DB2 if you do a DELETE with a WHERE
		 * 					clause and no records are deleted.
		 * \throw	Exception If executing SQL failed, or depending on mode if no records are affected.
		 */
		void         ExecSql(const std::string& sqlStmt, ExecFailMode mode = ExecFailMode::NotFailOnNoData);


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
		* \brief	Returns the SqlInfoProperties set of this Database. If the Database is not 
		*			open yet, the properties will contain only default values.
		*/
		SqlInfoProperties GetProperties() const noexcept { return m_props; };


		/*!
		* \brief	Get DSN set during Open().
		* \return	DSN set during Open().
		* \throw	Exception
		*/
		const std::string& GetDataSourceName() const    { exASSERT(!m_dbOpenedWithConnectionString);  return m_dsn; }


		/*!
		* \brief	Get Username set during Open().
		* \return	Username set during Open().
		* \throw	Exception
		*/
		const std::string& GetUsername() const     { exASSERT(!m_dbOpenedWithConnectionString);  return m_uid; }


		/*!
		* \brief	Get Password set during Open().
		* \return	Password set during Open().
		* \throw	Exception
		*/
		const std::string& GetPassword() const      { exASSERT(!m_dbOpenedWithConnectionString);  return m_authStr; }


		/*!
		* \brief	Get Connection String passed during Open().
		* \return	Connection String passed during Open().
		* \throw	Exception
		*/
		const std::string& GetConnectionInStr() const  { exASSERT(m_dbOpenedWithConnectionString); return m_inConnectionStr; }


		/*!
		* \brief	Get Connection String read from database after Open() was successful.
		* \return	Connection String read from database after Open() was successful.
		* \throw	Exception
		*/
		const std::string& GetConnectionOutStr() const { exASSERT(m_dbOpenedWithConnectionString); exASSERT(m_dbIsOpen);  return m_outConnectionStr; }


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
		* \brief	Returns the shared_ptr to the SqlDbcHandle holding the ODBC Connection handle.
		*			Note that this does not guarantee that the SqlDbcHandle returned actually has an
		*			ODBC Connection Handle allocated or that the pointer is not NULL.
		*/
		ConstSqlDbcHandlePtr	GetSqlDbcHandle() const noexcept       { return m_pHDbc; }


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
		void		SetSql2BufferTypeMap(Sql2BufferTypeMapPtr pSql2BufferTypeMap) noexcept;


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


		/*!
		* \brief	Get the DatabaseCatalog associated with this Database.
		* \throw	Exception if not Open()
		*/
		DatabaseCatalogPtr GetDbCatalog() const;

		// Private stuff
		// -------------
	private:
		
		/*!
		* \brief	Returns the SqlStmtHandlePtr used by the ExecSql function.
		*/
		SqlStmtHandlePtr GetExecSqlHandle() const noexcept { return m_pHStmtExecSql; };
		
		
		/*!
		* \brief	Set initial global attributes on the database connection handle.
		* \throw	Exception If no connection handle is allocated, or setting an attribute fails.
		*/
		void			SetConnectionAttributes();
		

		/*!
		* \brief	Do jobs common to all Open() implementations.
		* \details	This function will ensure that if it fails no handles
		*			are left in an allocated state.
		* \throw	Exception If anything goes wrong.
		*/
		void             OpenImpl();

		

		// Members
		// -------
		ConstEnvironmentPtr		m_pEnv;		///< Environment of this Database
		SqlInfoProperties		m_props;	///< Properties read from SqlGetInfo
		Sql2BufferTypeMapPtr	m_pSql2BufferTypeMap;	///< Sql2BufferTypeMap to be used from this Database. If none is set during OpenImp() a DefaultSql2BufferTypeMap is created.
		DatabaseCatalogPtr		m_pDbCatalog;	///< The catalog of this Database. Initialized during OpenImpl(), freed on Close()

		SqlTypeInfosVector m_datatypes;	///< Queried from DB during Open
		bool				m_dbIsOpen;			///< Set to true after SQLConnect was successful
		bool				m_dbOpenedWithConnectionString;  ///< Was the database connection Opened with a connection string
		std::string		m_dsn;             ///< Data source name
		std::string		m_uid;             ///< User ID
		std::string		m_authStr;         ///< Authorization string (password)
		std::string		m_inConnectionStr; ///< Connection string used to connect to the database
		std::string		m_outConnectionStr;///< Connection string returned by the database when a connection is successfully OpenImpled
		DatabaseProduct		m_dbmsType;        ///< Type of datasource - i.e. Oracle, dBase, SQLServer, etc

		// ODBC handles created by the Database

		SqlDbcHandlePtr m_pHDbc;			///< ODBC DB Connection handle
		SqlStmtHandlePtr m_pHStmtExecSql;	///< ODBC Statement handle used for the function ExecSql()

		CommitMode		m_commitMode;	///< Commit Mode set currently

	};  // Database

	typedef std::shared_ptr<Database> DatabasePtr;
	typedef std::shared_ptr<const Database> ConstDatabasePtr;
}	// namespace exodbc


