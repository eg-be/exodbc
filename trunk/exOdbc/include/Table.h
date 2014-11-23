/*!
* \file Table.h
* \author Elias Gerber <eg@zame.ch>
* \date 25.07.2014
* \brief Header file for the Table class and its helpers.
*
* Header file for the Table class and its helpers.
* This file was originally wx/dbtable.h from wxWidgets 2.8.
* Most of the code has been rewritten, a lot of functionality
* not needed and not tested so far has been droped.
*
* For completion, here follows the old wxWidgets header:
* 
* ///////////////////////////////////////////////////////////////////////////////<br>
* // Name:        dbtable.h<br>
* // Purpose:     Declaration of the wxDbTable class.<br>
* // Author:      Doug Card<br>
* // Modified by: George Tasker<br>
* //              Bart Jourquin<br>
* //              Mark Johnson<br>
* // Created:     9.96<br>
* // RCS-ID:      $Id: dbtable.h 61872 2009-09-09 22:37:05Z VZ $<br>
* // Copyright:   (c) 1996 Remstar International, Inc.<br>
* // Licence:     wxWindows licence<br>
* ///////////////////////////////////////////////////////////////////////////////<br>
*/

#pragma once
#ifndef DBTABLE_H
#define DBTABLE_H

// Same component headers
#include "exOdbc.h"
#include "Database.h"

// Other headers
// System headers

namespace exodbc
{
	// Consts
	// ------

	const int   wxDB_DEFAULT_CURSOR  = 0;

	// Used to indicate end of a variable length list of
	// column numbers passed to member functions
	const int   wxDB_NO_MORE_COLUMN_NUMBERS = -1;

	// Structs
	// -------
	struct EXODBCAPI SColumnDataPtr
	{
	public:
		void*		PtrDataObj;
		int			SzDataObj;
		SWORD		SqlCtype;
	};  // SColumnDataPtr


	// This structure is used when creating secondary indexes.
	struct EXODBCAPI SIndexDefinition
	{
	public:
		wchar_t		ColName[DB_MAX_COLUMN_NAME_LEN+1];
		bool		Ascending;
	};  // SIndexDefinition

	// Classes
	// -------

	// The following class is used to define a column of a table.
	// The wxDbTable constructor will dynamically allocate as many of
	// these as there are columns in the table.  The class derived
	// from wxDbTable must initialize these column definitions in it's
	// constructor.  These column definitions provide inf. to the
	// wxDbTable class which allows it to create a table in the data
	// source, exchange data between the data source and the C++
	// object, and so on.
	class EXODBCAPI ColumnDefinition
	{
	public:
		ColumnDefinition();

		bool    Initialize();

		wchar_t	m_colName[DB_MAX_COLUMN_NAME_LEN+1];  // Column Name
		int		m_dbDataType;                         // Logical Data Type; e.g. DB_DATA_TYPE_INTEGER
		SWORD	m_sqlCtype;                           // C data type; e.g. SQL_C_LONG
		void*	m_ptrDataObj;                         // Address of the data object
		int		m_szDataObj;                          // Size, in bytes, of the data object
		bool	m_keyField;                           // true if this column is part of the PRIMARY KEY to the table; Date fields should NOT be KeyFields.
		bool	m_updateable;                         // Specifies whether this column is updateable
		bool	m_insertAllowed;                      // Specifies whether this column should be included in an INSERT statement
		bool	m_derivedCol;                         // Specifies whether this column is a derived value
		SQLLEN	m_cbValue;                            // Internal use only!!!
		bool	m_null;                               // NOT FULLY IMPLEMENTED - Allows NULL values in Inserts and Updates
	};  // ColumnDefinition

	/*!
	* \class Table
	*
	* \brief Represent a Table from a Database. Every Table needs a Database.
	*
	* This class will allocate the Statements-Handles that are required
	* to query and modify the table.
	* Depending on which constructor is used, a table will query the database
	* about its column-definitions later automatically, or the column-definitions
	* must be set manually using SetColDefs().
	* During Open() the table will "bind" a buffer to every defined column.
	* Columns bound to the table will be updated with the values of the current record
	* this table points to. Calling methods like Update(), Delete() or Insert() will
	* modify the table in the database using the current values.
	* 
	* A table that queries the database about its columns will always bind all columns.
	* A table where columns are defined manually will only bind those columns defined.
	* This is handy if you have a large table and are ony interested in the values of
	* a few columns. 
	*
	* The database will always be queried for a table matching the given values in the
	* constructor, except one of the constructors where a STableInfo structure
	* is passed was used.
	*
	* A table must be opened after construction by calling Open().
	*
	* All statements will be freed on destruction of the Table. You should not
	* Destroy the database before the Table is destroyed.
	*/
	class EXODBCAPI Table
	{
	public:

		/*!
		* \enum OpenMode
		* \brief Defines if a table is opened read-only or writable
		*/
		enum OpenMode
		{
			READ_ONLY,	///< Only query the table using SELECT statements
			READ_WRITE	///< Open the table for querying and modifying
		};

		/*!
		* \brief	Create a new Table-instance from the Database pDb using the table definition 
		*			from tableInfo. The table will read its column-definitions from the database
		*			automatically during Open() and bind all columns.
		* \detailed	Note that when this constructor is used, the internal STableInfo object is not
		*			queried from the database, but the passed tableInfo is used for all later operations.
		*			This is handy if you've located the detailed table-information already from the Database
		*			using its Database::FindTables() function and want to avoid that is operation is
		*			executed again during Open().
		* \param	pDb		The Database this Table belongs to. Do not free the Database before
		*					you've freed the Table.
		* \param	tableInfo Definition of the table.
		* \param	openMode Define if the table shall be opened read-only or not
		*
		* \see		Open()
		*/
		Table(Database* pDb, const STableInfo& tableInfo, OpenMode openMode = READ_WRITE);


		/*!
		* \brief	Create a new Table-instance from the Database pDb by querying the database
		*			about a table with the given values for name, schema, catalog and type.
		*			The table will read its column-definitions from the database
		*			automatically during Open() and bind all columns.
		* \detailed During Open() the database will be queried for a table matching the given values.
		*			If any of the values is an empty string it is ignored when searching for the table
		*			in the database.
		*
		* \param	pDb		The Database this Table belongs to. Do not free the Database before
		*					you've freed the Table.
		* \param	tableName	Table name
		* \param	schemaName	Schema name
		* \param	catalogName	Catalog name
		* \param	tableType	Table type
		* \param	openMode Define if the table shall be opened read-only or not
		*
		* \see		Open()
		*/
		Table(Database* pDb, const std::wstring& tableName, const std::wstring& schemaName = L"", const std::wstring& catalogName = L"", const std::wstring& tableType = L"", const OpenMode openMode = READ_WRITE);


		/*!
		* \brief	Create a new Table-instance on which you will later set the ColumnInfo manually.
		*			During Open() only those columns you have set using SetColDefs() will be bound.
		* \detailed During Open() the database will be queried for a table matching the given values.
		*			If any of the values is an empty string it is ignored when searching for the table
		*			in the database.
		*
		* \param	pDb		The Database this Table belongs to. Do not free the Database before
		*					you've freed the Table.
		* \param	numColumns The number of columns of the Table.
		* \param	tableName Table name
		* \param	schemaName	Schema name
		* \param	catalogName	Catalog name
		* \param	tableType	Table type
		* \param	openMode Define if the table shall be opened read-only or not
		*
		* \see		Open()
		* \see		SetColDefs()
		*/
		Table(Database* pDb, size_t numColumns, const std::wstring& tableName, const std::wstring& schemaName = L"", const std::wstring& catalogName = L"", const std::wstring& tableType = L"", OpenMode openMode = READ_WRITE);

		/*!
		* \brief	Create a new Table-instance on which you will later set the ColumnInfo manually.
		*			During Open() only those columns you have set using SetColDefs() will be bound.
		* \detailed During Open the database will be queried for a table named tableName, ignoring
		*			schema and catalog-name and table-type.
		*			Note that when this constructor is used, the internal STableInfo object is not
		*			queried from the database, but the passed tableInfo is used for all later operations.
		*			This is handy if you've located the detailed table-information already from the Database
		*			using its Database::FindTables() function and want to avoid that is operation is
		*			executed again during Open().
		*
		* \param	pDb		The Database this Table belongs to. Do not free the Database before
		*					you've freed the Table.
		* \param	numColumns The number of columns of the Table.
		* \param	tableInfo Definition of the table.
		* \param	openMode Define if the table shall be opened read-only or not
		*
		* \see		Open()
		* \see		SetColDefs()
		*/
		Table(Database* pDb, size_t numColumns, const STableInfo& tableInfo, OpenMode openMode = READ_WRITE);

		virtual ~Table();

		/*!
		* \brief	Opens the Table and either binds the already defined columns or queries the database
		*			about the columns of this table.
		* \detailed If no STableInfo object has been passed during construction the database is first
		*			queried for a table matching the parameters passed. If not exactly one such table is
		*			found Open will fail.
		*			If no columns have been defined using SetColDefs() the database is queried for all
		*			columns of this table. Afterwards the columns are bound to their buffers, allocated
		*			automatically during Open. If columns have been defined manually using SetColDefs(),
		*			the buffers passed there are used to bind only those columns defined manually.
		*
		* \param	checkPrivileges If true, the database will be queried checking if the current user
		*			is allowed to do the required operations like Select, Update, Insert or Delete,
		*			depending on the OpenMode value passed during construction.
		* \param	checkTableExists If true, the database will always be queried if a table matching the
		*			passed definition during constructions actually exists in the database. Setting this 
		*			value to false makes only sense	if you've passed a STableInfo, as else the Table
		*			is required to query the database anyway (and fail if not found).
		*
		* \return	true if it succeeds, false if it fails.
		*/
		bool            Open(bool checkPrivileges = false, bool checkTableExists = true);

		//bool            CreateTable(bool attemptDrop = true);
//		bool            DropTable();
//		bool            CreateIndex(const std::wstring& indexName, bool unique, UWORD numIndexColumns, SIndexDefinition* pIndexDefs, bool attemptDrop = true);
//		bool            DropIndex(const std::wstring& indexName);

		// Accessors

		/*!
		* \brief	Get the database that was set during constructions.
		*
		* \return	Database this Table belongs to.
		*/
		Database*			GetDb() const		{ return m_pDb; }

		/*!
		* \brief	Check if the Table-Information is set on this Table.
		* \detailed	Returns true if the internal member of the STableInfo contains a value either
		*			set during Construction or fetched from the Database during Open().
		* \see		GetTableInfo()
		* \return	Returns true if this table has a STableInfo set that can be fetched using GetTableInfo()
		*/
		bool				HaveTableInfo() const { return m_haveTableInfo;  }

		/*!
		* \brief	Return the Table information of this Table.
		* \detailed	Returns the STableInfo of this table, if one has been set either during construction
		*			or one was read during Open().
		* \see		HaveTableInfo()
		* \return	Returns true if this table has a STableInfo set that can be fetched using GetTableInfo()
		*/
		STableInfo			GetTableInfo() const;

		UWORD           GetNumberOfColumns() { return m_numCols; }  // number of "defined" columns for this wxDbTable instance

		const std::wstring& GetFromClause()      { return m_from; }
		const std::wstring& GetOrderByClause()   { return m_orderBy; }
		const std::wstring& GetWhereClause()     { return m_where; }

		bool            IsQueryOnly()        { return m_openMode == READ_ONLY; }
		void            SetFromClause(const std::wstring& From) { m_from = From; }
		void            SetOrderByClause(const std::wstring& OrderBy) { m_orderBy = OrderBy; }
		bool            SetOrderByColNums(UWORD first, ...);
		void            SetWhereClause(const std::wstring& Where) { m_where = Where; }
		void            From(const std::wstring& From) { m_from = From; }
		void            OrderBy(const std::wstring& OrderBy) { m_orderBy = OrderBy; }
		void            Where(const std::wstring& Where) { m_where = Where; }
		const std::wstring& Where()   { return m_where; }
		const std::wstring& OrderBy() { return m_orderBy; }
		const std::wstring& From()    { return m_from; }

		int             Insert();
		//bool            Update();
		//bool            Update(const std::wstring& pSqlStmt);
		//bool            UpdateWhere(const std::wstring& pWhereClause);
		//bool            Delete();
		//bool            DeleteWhere(const std::wstring& pWhereClause);
		//bool            DeleteMatching();
//		virtual bool    Query(bool forUpdate = false, bool distinct = false);
		bool            QueryBySqlStmt(const std::wstring& pSqlStmt);
		bool            QueryMatching(bool forUpdate = false, bool distinct = false);
		bool            QueryOnKeyFields(bool forUpdate = false, bool distinct = false);
		//bool            Refresh();
		bool            GetNext()		{ return(getRec(SQL_FETCH_NEXT));  }
		bool            operator++(int) { return(getRec(SQL_FETCH_NEXT));  }

		/***** These four functions only work with wxDb instances that are defined  *****
		***** as not being FwdOnlyCursors                                          *****/
		bool            GetPrev();
		bool            operator--(int);
		bool            GetFirst();
		bool            GetLast();

		bool            IsCursorClosedOnCommit();
		UWORD           GetRowNum();

		//void            BuildSelectStmt(std::wstring& pSqlStmt, int typeOfSelect, bool distinct);
		//void            BuildSelectStmt(wchar_t* pSqlStmt, int typeOfSelect, bool distinct);

//		void            BuildDeleteStmt(std::wstring& pSqlStmt, int typeOfDel, const std::wstring& pWhereClause = emptyString);
		//void            BuildDeleteStmt(wchar_t* pSqlStmt, int typeOfDel, const std::wstring& pWhereClause = emptyString);

		//void            BuildUpdateStmt(std::wstring& pSqlStmt, int typeOfUpdate, const std::wstring& pWhereClause = emptyString);
		//void            BuildUpdateStmt(wchar_t* pSqlStmt, int typeOfUpdate, const std::wstring& pWhereClause = emptyString);

		//void            BuildWhereClause(std::wstring& pWhereClause, int typeOfWhere, const std::wstring& qualTableName = emptyString, bool useLikeComparison = false);
		//void            BuildWhereClause(wchar_t* pWhereClause, int typeOfWhere, const std::wstring &qualTableName = emptyString, bool useLikeComparison = false);

		bool            CanSelectForUpdate();
		bool            CanUpdateByROWID();
		void            ClearMemberVar(UWORD colNumber, bool setToNull = false);
		void            ClearMemberVars(bool setToNull = false);
		bool            SetQueryTimeout(UDWORD nSeconds);

		ColumnDefinition* GetColDefs() { return m_colDefs; }
		bool            SetColDefs(UWORD index, const std::wstring& fieldName, int dataType, void* pData, SWORD cType, int size, bool keyField = false, 
									bool updateable = true, bool insertAllowed = true, bool derivedColumn = false);
		SColumnDataPtr* SetColDefs(ColumnInfo* colInfs, UWORD numCols);

		bool            CloseCursor(HSTMT cursor);
		bool            DeleteCursor(HSTMT* hstmtDel);
		void            SetCursor(HSTMT* hstmtActivate = (void **) wxDB_DEFAULT_CURSOR);
		HSTMT           GetCursor() { return(m_hstmt); }
		HSTMT*			GetNewCursor(bool setCursor = false, bool bindColumns = true);

		//ULONG           Count(const std::wstring& args = L"*");

		bool            IsColNull(UWORD colNumber) const;
		bool            SetColNull(UWORD colNumber, bool set = true);
		bool            SetColNull(const std::wstring& colName, bool set = true);

		// Private member variables
		UDWORD      m_cursorType;

		// Private member functions
		bool        Initialize(Database* pDb);
		void        cleanup();

		void        setCbValueForColumn(int columnIndex);
//		bool        bindParams(bool forUpdate);  // called by the other 'bind' functions
		//bool        bindInsertParams();
		//bool        bindUpdateParams();

		bool        bindCols(HSTMT cursor);
		bool        getRec(UWORD fetchType);
		bool        execDelete(const std::wstring& pSqlStmt);
//		bool        execUpdate(const std::wstring& pSqlStmt);
		bool        query(int queryType, bool forUpdate, bool distinct, const std::wstring& pSqlStmt = emptyString);

		// Where, Order By and From clauses
		std::wstring    m_where;               // Standard SQL where clause, minus the word WHERE
		std::wstring    m_orderBy;             // Standard SQL order by clause, minus the ORDER BY
		std::wstring    m_from;                // Allows for joins in a wxDbTable::Query().  Format: ",tbl,tbl..."

		// ODBC Handles
//		HENV        m_henv;           // ODBC Environment handle
		HDBC        m_hdbc;           // ODBC DB Connection handle
		HSTMT       m_hstmt;          // ODBC Statement handle
		HSTMT*		m_hstmtDefault;   // Default cursor
		HSTMT       m_hstmtInsert;    // ODBC Statement handle used specifically for inserts
		HSTMT       m_hstmtDelete;    // ODBC Statement handle used specifically for deletes
		HSTMT       m_hstmtUpdate;    // ODBC Statement handle used specifically for updates
		HSTMT       m_hstmtInternal;  // ODBC Statement handle used internally only
		HSTMT*		m_hstmtCount;     // ODBC Statement handle used by Count() function (No binding of columns)

		// Flags
		bool        m_selectForUpdate;

		// Pointer to the database object this table belongs to
		Database*		m_pDb;

		// Table Information
		bool				m_haveTableInfo;		///< True if m_tableInfo has been set
		STableInfo			m_tableInfo;			///< TableInfo fetched from the db or set through constructor
		const OpenMode		m_openMode;				///< Read-only or writable

		// Table information set during construction, that was used to find the matching STableInfo if none was passed
		const std::wstring  m_initialTableName;		///< Table name set on construction
		const std::wstring	m_initialSchemaName;	///< Schema name set on construction
		const std::wstring	m_initialCatalogName;	///< Catalog name set on construction
		const std::wstring	m_initialTypeName;		////< Type name set on construction

		// Column information
		const bool			m_manualColumns;		///< If true the table was created by passing the number of columns that will be defined later manually
		const size_t		m_numCols;				//< # of columns in the table. Either set from user during constructor, or read from the database

		// Column Definitions
		ColumnDefinition* m_colDefs;         // Array of wxDbColDef structures

#ifdef EXODBCDEBUG
	public:
		size_t				GetTableId() const { return m_tableId; }
	private:
		size_t m_tableId; ///< Given by calling Database::RegisterTable() during Initialization.
#endif

	};  // Table

}	// namespace exodbc

#endif // DBTABLE_H
