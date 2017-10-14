/*!
* \file Table.h
* \author Elias Gerber <eg@elisium.ch>
* \date 25.07.2014
* \brief Header file for the Table class and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
* Header file for the Table class and its helpers.
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "Exception.h"
#include "Sql2BufferTypeMap.h"
#include "ColumnBuffer.h"
#include "SqlHandle.h"
#include "Database.h"
#include "EnumFlags.h"
#include "ExecutableStatement.h"

// Other headers
// System headers

// Forward declarations
// --------------------

#if EXODBC_TEST
namespace exodbctest
{
	class TableTest_CopyCtr_Test;
	class TableTest_QueryPrimaryKeysAndUpdateColumns_Test;

}
#endif

namespace exodbc
{
	// Consts
	// ------

	// Structs
	// -------

	// Classes
	// -------

	/*!
	* \class Table
	*
	* \brief Represent a Table from a Database. Every Table needs a Database.
	*
	* This class will allocate some Statements-Handles 
	* to query and modify the table. Which statements handles are allocated depends
	* on the AccessFlags set.
	*
	* After creating the Table you can manually set ColumnBuffer objects as Table columns
	* for data exchange using SetColumn().
	*
	* If Open() is called and no ColumnBuffers have been set manually using SetColumn(), the Table will
	* query the Database about the columns of this table and create corresponding ColumnBuffers.
	*
	* Columns bound to the table will be updated with the values of the current record
	* this table points to. The record can be updated using any of the Select() methods.
	* To modify the data in the database, set the values to write on the corresponding
	* ColumnBuffer and then call any of the methods Update(), Delete(),  Insert().
	*
	* A table that queries the database about its columns will always try to bind all columns.
	* A table where columns are defined manually will only bind those columns defined.
	* This is handy if you have a large table and are only interested in the values of
	* a few columns.
	* Note if a name is something like columnIndex it will refer to the zero based index.
	*
	* A table must be opened after construction by calling Open().
	*
	* All statements will be freed on destruction of the Table. You must not
	* Destroy the Database before the Table is destroyed.
	*/
	class EXODBCAPI Table
	{
#if EXODBC_TEST
		friend class exodbctest::TableTest_CopyCtr_Test;
		friend class exodbctest::TableTest_QueryPrimaryKeysAndUpdateColumns_Test;
#endif

	public:

		/*!
		* \brief	Default Constructor.
		* \details	You must call one of the Init() methods if the Table was created using
		*			this Default Constructor.
		*/
		Table() noexcept;


		/*!
		* \brief	Create new table using the passed TableInfo.
		* \see		Init(const Database*, AccessFlags, const TableInfo&)
		* \throw	Exception
		*/
		Table(ConstDatabasePtr pDb, TableAccessFlags afs, const TableInfo& tableInfo);


		/*!
		* \brief	Create a new Table using the passed search-names. If any of the passed names is set to an empty string
		*			it is ignored.
		* \see		Init(const Database* pDb, AccessFlags, const std::string&, const std::string&, const std::string&, const std::string&)
		* \throw	Exception
		*/
		Table(ConstDatabasePtr pDb, TableAccessFlags afs, const std::string& tableName, const std::string& schemaName = u8"", const std::string& catalogName = u8"", const std::string& tableType = u8"");


		/*!
		* \brief	Copy Constructor. Creates a new Table with the same AccessFlags 
		*			as other, depending on the same Database as other.
		* \details	If other already has a TableInfo object the TableInfo-object of other
		*			is copied into this Table and the flag m_haveTableInfo is set to true.
		*			Eventually set table search-names are copied from other.
		*/
		Table(const Table& other);
		

	public:
		virtual ~Table();

		/*!
		* \brief	Initializes the Table with the TableInfo and AccessFlags passed.
		* \details	Note that when this initialization is used, the internal TableInfo object is not
		*			queried from the database, but the passed tableInfo is used for all later operations.
		*			This is handy if you've located the detailed table-information already from the Database
		*			using its Database::FindTables() function and want to avoid that this operation is
		*			executed again during Open().
		* \param	pDb		The Database this Table belongs to. Do not free the Database before
		*					you've freed the Table.
		* \param	tableInfo Definition of the table.
		* \param	afs Define if the table shall be opened read-only or not, determines how many statements are allocated.
		*
		* \see		Open()
		* \throw	Exception If allocating statements fail.
		*/
		void Init(ConstDatabasePtr pDb, TableAccessFlags afs, const TableInfo& tableInfo);


		/*!
		* \brief	Initializes the Table with the names to search for during Open() and the
		*			AccessFlags.
		* \details During Open() the database will be queried for a table matching the given values.
		*			If any of the values is an empty string it is ignored when searching for the table
		*			in the database.
		*
		* \param	pDb		The Database this Table belongs to. Do not free the Database before
		*					you've freed the Table.
		* \param	tableName	Table name
		* \param	schemaName	Schema name
		* \param	catalogName	Catalog name
		* \param	tableType	Table type
		* \param	afs Define if the table shall be opened read-only or not, determines how many statements are allocated.
		*
		* \see		Open()
		* \throw	Exception If allocating statements fail.
		*/
		void Init(ConstDatabasePtr pDb, TableAccessFlags afs, const std::string& tableName, const std::string& schemaName = u8"", const std::string& catalogName = u8"", const std::string& tableType = u8"");


		/*!
		* \brief	Opens the Table and either binds the already defined columns or queries the database
		*			about the columns of this table.
		* \details If no TableInfo object has been passed during construction the database is first
		*			queried for a table matching the parameters passed. If not exactly one such table is
		*			found Open will fail.
		*
		*			If no columns have been defined using SetColumn() the database is queried for all
		*			columns of this table and corresponding ColumnBuffer objects are allocated.
		*			Afterwards the columns are bound to their buffers. 
		*			
		*			If columns have been defined manually using SetColumn(),
		*			the buffers passed there are used to bind only those columns defined manually.
		*
		* \param	openFlags Set flags how to open the Table:
		*  - TableOpenFlag::TOF_CHECK_EXISTANCE:
		*			If set, the database will always be queried if a table matching the
		*			passed definition during constructions actually exists in the database. Unsetting this
		*			flag makes only sense if you've passed a TableInfo during construction, 
		*			as else the Table is required to query the database anyway (and fails if not found).
		*  - TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS:
		*			If set, Columns for which no ColumnBuffer can be created are skipped. If not set an
		*			Exception is thrown if creation of a ColumnBuffer fails (default).\n
		*			If Columns are created automatically, the Database might report a SQL type that is not
		*			implemented (No mapping of SQL-Type o SQL-C-Type). Set this flag to simply skip such columns.\n
		*			If Columns have been set manually, Columns might have been defined using a SQL Type that
		*			is not reported as supported from the Database. Set this flag to simply skip such
		*			columns. See also TableOpenFlag::TOF_IGNORE_DB_TYPE_INFOS.\n
		*			If ColumnBuffers are skipped, the indexes of the bound ColumnBuffers will no longer match the
		*			indexes of the actual table.
		*  - TableOpenFlag::TOF_IGNORE_DB_TYPE_INFOS:
		*			If set, the SQL Types of manually defined columns are not validated against the supported
		*			types reported by the Database. If this flag is set, the flag TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS
		*			does nothing for manually defined columns.
		*  - TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS:
		*			If a table is opened with the AccessFlag::AF_UPDATE_PK or AccessFlag::AF_DELETE_PK set,
		*			Primary keys are required. They are queried from the Database during Open(), unless
		*			this flag is set, or the column indexes of the Table have been passed prior to Open()
		*			using SetColumnPrimaryKeyIndexes().\n
		*			If this flag is set, the primary keys are not queried.\n
		*			This flag is set automatically whenever Open() ing a Table from a Microsoft Access Database,
		*			as the Access Driver I used for testing (ODBCJT32.DLL, v6.01.7601.17632, 
		*			'Microsoft Access Driver (*.mdb')') does not support the SQLPrimaryKeys() method.\n
		*			This flag is sometimes active implicitly, for example if you have manually defined 
		*			primary key columns using SetColumnPrimaryKeyIndexes().\n
		*			If you have set ColumnBuffers manually, and defined the corresponding ColumnFlag::CF_PRIMARY_KEY
		*			on the corresponding ColumnBuffers, you might want to activate the flag.
		*  - TableOpenFlag::TOF_FORWARD_ONLY_CURSORS:
		*			If the Database supports Scrollable Cursors, the Table will try to use Scrollable Cursors
		*			on the Query Statements. If this flag is set, the Table will always use forward-only Cursors.
		* \see		IsOpen()
		* \see		Close()
		* \see		SetColumn()
		* \see		SetColumnPrimaryKeyIndexes()
		* \throw	Exception If already open, table is not found, columns fail to bind, etc.
		*/
		void		Open(TableOpenFlags openFlags = TableOpenFlag::TOF_CHECK_EXISTANCE);


		/*!
		* \brief	Close the table.
		* \details	Resets all statements used. If ColumnBuffers were created automatically during open,
		*			those ColumnBuffers are removed. The TableInfo of this this table queried during Open() 
		*			is kept for future use.
		* \see		Open()
		* \throw	Exception If not Open or unbinding fails.
		*/
		void		Close();


		/*!
		* \brief	Check if the database is Open
		*
		* \return	True if Open() was already called succesfull
		* \see		Open()
		*/
		bool		IsOpen() const noexcept { return m_isOpen; };


		/*!
		* \brief	Checks if we can only read from this table.
		* \return	True if this table has the flag AF_READ set and none of the flags
		*			AF_UPDATE_PK, AF_UPDATE_WHERE, AF_INSERT, AF_DELETE_PK or AF_DELETE_WHERE are set.
		*/
		bool		IsQueryOnly() const noexcept;


		/*!
		* \brief	Set an AccessFlag. Can only be called if Table is Closed, so
		*			when IsOpen() returns false.
		* \details	TableAccessFlags will be evaluated during Open().
		* \throw	AssertionException If Table is already open.
		*/
		void		SetAccessFlag(TableAccessFlag ac);


		/*!
		* \brief	Clear an AccessFlag. Can only be called if Table is Closed, so
		*			when IsOpen() returns false.
		* \details	TableAccessFlags will be evaluated during Open().
		* \throw	AssertionException If Table is already open.
		*/
		void		ClearAccessFlag(TableAccessFlag ac);


		/*!
		* \brief	Set multiple AccessFlags. Can only be called if Table is Closed, so
		*			when IsOpen() returns false.
		* \details	TableAccessFlags will be evaluated during Open().
		* \throw	AssertionException If Table is already open.
		*/
		void		SetAccessFlags(TableAccessFlags acs);


		/*!
		* \brief	Test if an AccessFlag is set.
		*/
		bool		TestAccessFlag(TableAccessFlag ac) const noexcept;


		/*!
		* \brief	Get the AccessFlags bitmask.
		*/
		TableAccessFlags	GetAccessFlags() const noexcept { return m_tableAccessFlags; };


		/*!
		* \brief	Test if a TableOpenFlag is set.
		*/
		bool		TestOpenFlag(TableOpenFlag flag) const noexcept;


		/*!
		* \brief	Check if the Table-Information is set on this Table.
		* \details	Returns true if the internal member of the TableInfo contains a value either
		*			set during Construction or fetched from the Database during Open().
		* \see		GetTableInfo()
		* \return	Returns true if this table has a TableInfo set that can be fetched using GetTableInfo()
		*/
		bool		HasTableInfo() const noexcept { return m_haveTableInfo; }


		/*!
		* \brief	Counts how many rows would be selected in this table by the passed WHERE clause.
		* \details	If whereStatement is empty, no WHERE clause is added
		* \param	whereStatement Do not include 'WHERE' in the passed where clause
		* \return	count The result of a 'SELECT COUNT(*) WHERE whereStatement' on the current table
		* \throw	Exception If failed.
		*/
		SQLUBIGINT	Count(const std::string& whereStatement);


		/*!
		* \brief	Calls Count() with no whereStatement.
		* \see		Count(const std::string& whereStatement);
		*/
		SQLUBIGINT	Count() { return Count(u8""); };


		/*!
		* \brief	Executes a 'SELECT col1, col2, .., colN' for the Table using the passed WHERE clause.
		* \details	The SELECT-Query is built using the column information available to this Table.
		*			It does not use the '*'. Only bound ColumnBuffers names are included in the statement.
		*			If successful, a Select-Query is open. You can iterate the records
		*			using SelectNext() to access the values of the records.
		*			The cursor is positioned before the first records, so you must call
		*			SelectNext() to access the first record.\n
		*			If whereStatement is empty, no WHERE clause is added.\n
		*			If orderStatement is empty, no ORDER clause is added.\n
		*			If a select statement is open, the statement is closed first.
		* \param	whereStatement Do not include 'WHERE' in the passed where clause
		* \param	orderStatement Do not include 'ORDER BY' in the passed oder clause
		* \see		SelectNext()
		* \see		SelectClose()
		* \throw	Exception If failed.
		*/
		void		Select(const std::string& whereStatement = u8"", const std::string& orderStatement = u8"");


		/*!
		* \brief	Executes a 'SELECT col1, col2, .., colN WHERE PK1 = p1 AND PK2 = p2, ..' for the Table.
		* \details	All ColumnBuffers that had the flag flag CF_SELECT set during open are included 
		*			in the SELECT part and all ColumnBuffers that had CF_PRIMARY_KEY set during Open()
		*			are part of the	WHERE clause.
		*			If successful, a Select-Query is open. You can iterate the records
		*			using SelectNext() to access the values of the records.
		*			The cursor is positioned before the first records,  call
		*			SelectNext() to access the first record.\n
		*			If a select statement is open, the statement is closed first.
		* \see		SelectNext()
		* \see		SelectClose()
		* \throw	Exception If failed.
		*/
		void		SelectByPkValues();


		/*!
		* \brief	Executes the passed SQL statement on the open Table.
		* \details	Query by passing the complete SQL statement.
		*			If successful, a Select-Query is open. You can iterate the records
		*			using SelectNext() to access the values of the records.
		*			The cursor is positioned before the first records, so you must call
		*			SelectNext() to access the first record.
		*			If a select statement is open, the statement is closed first.
		* \param	sqlStmt Must be a full SQL statement like 'SELECT foo FROM A WHERE x > 3' and
		*			all bound columns must be included or the retrieved data will be mixed within
		*			the columns.
		* \see		SelectNext()
		* \see		SelectClose();
		* \return	True if successful
		* \throw	Exception If failed.
		*/
		void		SelectBySqlStmt(const std::string& sqlStmt);


		/*!
		* \brief	Fetches the next record fromt the current active Select() recordset.
		* \see		Select()
		* \return	True if next record has been fetched, false if no more records exist.
		* \throw	Exception If no SelectQuery is open.
		*/
		bool		SelectNext();


		/*!
		* \brief	Fetches the previous record fromt the current active Select() recordset.
		* \see		Select()
		* \return	True if previous record has been fetched, false if no more records exist.
		* \throw	Exception If no SelectQuery is open, or if TOF_FORWARD_ONLY_CURSORS is set.
		*/
		bool		SelectPrev();


		/*!
		* \brief	Fetches the first record fromt the current active Select() recordset.
		* \see		Select()
		* \return	True if first record has been fetched, false if no record available.
		* \throw	Exception If no SelectQuery is open, or if TOF_FORWARD_ONLY_CURSORS is set.
		*/
		bool		SelectFirst();


		/*!
		* \brief	Fetches the last record fromt the current active Select() recordset.
		* \see		Select()
		* \return	True if last record has been fetched, false if no record available.
		* \throw	Exception If no SelectQuery is open, or if TOF_FORWARD_ONLY_CURSORS is set.
		*/
		bool		SelectLast();


		/*!
		* \brief	Fetches the record at absolute position fromt the current active Select() recordset.
		* \see		Select()
		* \return	True if record at position has been fetched, false if no record available.
		* \throw	Exception If no SelectQuery is open, or if TOF_FORWARD_ONLY_CURSORS is set.
		*/
		bool		SelectAbsolute(SQLLEN position);


		/*!
		* \brief	Fetches the record at relative position to the currently selected record.
		* \see		Select()
		* \return	True if record at relative position has been fetched, false if no record available.
		* \throw	Exception If no SelectQuery is open, or if TOF_FORWARD_ONLY_CURSORS is set.
		* \note		MySql seems to get confused if no record is selected before doing the first SelectRelative():
		*			If just a statement is executed, I would expect the cursor to be positioned before the 
		*			first row. So a SelectRelative(3) should select the 3rd record. But on MySql a SelectRelative(3)
		*			selects the 4th record. If you do a SelectNext() first and then a SelectRelative(2), the 
		*			3rd record is selected correctly.
		*/
		bool		SelectRelative(SQLLEN offset);


		/*!
		* \brief	Closes an eventually open Select-Query.
		* \details	This function does not fail if no select statement was open.
		* \see		Select()
		* \throw	Exception
		*/
		void		SelectClose();


		/*!
		* \brief	Inserts the current values into the database as a new row.
		* \details	The values in the ColumnBuffer currently bound will be inserted
		*			into the database.
		*			An prepared INSERT statement is used to insert the values identified
		*			by the bound ColumnBuffers that have the flag ColumnFlags::CF_INSERT set. \n
		*			Fails if the table has not been opened using TableAccessFlag::AF_INSERT. \n
		*			This will not commit the transaction.
		* \see		Database::CommitTrans()
		* \throw	Exception on failure.
		*/
		void		Insert() const;


		/*!
		* \brief	Deletes the row identified by the values of the bound primary key columns.
		* \details	A prepared DELETE statement is used to delete the row that matches all
		*			primary keys of this Table. The key values are read from the ColumnBuffers bound
		*			to the primary key columns.
		*			Fails if the table has not been opened using TableAccessFlag::AF_DELETE_PK. \n
		*			This will not commit the transaction.
		* \param	failOnNoData If set to true the function will return false if the result of
		*			the DELETE is SQL_NO_DATA.
		* \see		Database::CommitTrans()
		* \throw	Exception on failure, or depending on failOnNoData, a SqlResultException if the 
		*			call to SQLExecute fails with SQL_NO_DATA.
		*/
		void		DeleteByPkValues(bool failOnNoData = true);


		/*!
		* \brief	Delete the row(s) identified by the passed where statement.
		* \details	A DELETE statement for this Table is created, using the passed
		*			WHERE clause.
		*			This uses an independent statement and will not modify the values in
		*			the ColumnBuffers or any ongoing Select() call. \n
		*			Fails if the table has not been opened using TableAccessFlag::AF_DELETE_WHERE. \n
		*			Fails if where is empty. \n
		*			This will not commit the transaction.
		* \param	where WHERE clause to be used. Do not include 'WHERE', the Table will add this.
		*			Not allowed to be empty.
		* \param	failOnNoData If set to true the function will return false if the result of
		*			the DELETE is SQL_NO_DATA.
		* \see		Database::CommitTrans()
		* \throw	Exception on failure, or depending on failOnNoData, a SqlResultException if the
		*			call to SQLExecute fails with SQL_NO_DATA.
		*/
		void		Delete(const std::string& where, bool failOnNoData = true) const;


		/*!
		* \brief	Updates the row identified by the values of the bound primary key columns with
		*			the values in bound ColumnBuffers, if the ColumnBuffer has the ColumnFlags::CF_UPDATE
		*			set.
		* \details	A prepared UPDATE statement is used to update the row(s) that matches all
		*			primary key values of this Table. The key values are read from the ColumnBuffers bound
		*			to the primary key columns. 
		*			The prepared statement will update the bound columns where the ColumnBuffer has 
		*			the flag ColumnFlags::CF_UPDATE set and the flag ColumnFlags::CF_PRIMARY_KEY is not set. \n
		*			Fails if the table has not been opened using TableAccessFlag::AF_UPDATE_PK. \n
		*			This will not commit the transaction.
		* \see		Database::CommitTrans()
		* \throw	Exception if failed.
		*/
		void		UpdateByPkValues();


		/*!
		* \brief	Updates the row identified by the passed where statement with
		*			the values in bound ColumnBuffers, if the ColumnBuffer has the ColumnFlags::CF_UPDATE
		*			set.
		* \details	A prepared UPDATE statement is used to update the row(s) that match the passed where statement.
		*			A prepared statement is created to update the columns bound to ColumnBuffers that have the 
		*			flag ColumnFlags::CF_UPDATE set with the values stored in the ColumnBuffer.
		*			Note that this will also update the bound primary-key columns (those that have the flag
		*			ColumnFlags::CF_PRIMARY_KEY set), if the column has the flag ColumnFlags::CF_UPDATE set. \n
		*			Fails if the table has not been opened using TableAccessFlag::AF_UPDATE_WHERE. \n
		*			Fails if no columns have the ColumnFlag::CF_UPDATE set. \n
		*			Fails if where is empty. \n
		*			This will not commit the transaction.
		* \param	where WHERE clause to be used. Do not include 'WHERE', the Table will add this.
		* \see		Database::CommitTrans()
		* \throw	Exception if failed.
		*/
		void		Update(const std::string& where);


		/*!
		* \brief	Sets the the length-indicator value of the ColumnBuffer
		*			at columnIndex.
		*/
		void		SetColumnLengthIndicator(SQLSMALLINT columnIndex, SQLLEN cb) const;


		/*!
		* \brief	Set a column to NULL. Fails if the column is not NULLable.
		* \param	columnIndex Zero based ColumnBuffer index.
		* \throw	Exception If ColumnBuffer not found.
		*/
		void		SetColumnNull(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Sets the SQL_NTS value on the length-indicator of the ColumnBuffer
		*			at columnIndex.
		*/
		void		SetColumnNTS(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Get the ColumnBuffer (as shared_ptr) at columnIndex.
		* \details	Fetches the ColumnBufferPtrVariant and tries to extract the concrete ColumnBuffer
		*			from the variant, using boost::get.
		* \throw	Exception If no column is available for passed columnIndex, or if boost::get fails.
		*/
		template<typename T>
		T GetColumnBufferPtr(SQLSMALLINT columnIndex) const
		{
			const ColumnBufferPtrVariant& columnVariant = GetColumnBufferPtrVariant(columnIndex);
			try
			{
				return boost::get<T>(columnVariant);
			}
			catch (const boost::bad_get& ex)
			{
				WrapperException we(ex);
				SET_EXCEPTION_SOURCE(we);
				throw we;
			}
		}


		/*!
		* \brief	Check if the current value of a column is NULL.
		* \details	Queries the length-indicator field of the ColumnBuffer to determine if
		*			a column is NULL.
		* \param	columnIndex Zero based index of a bound column.
		* \return	True if current value of column is NULL.
		* \throw	Exception If columnIndex is invalid.
		*/
		bool		IsColumnNull(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Check if a column can be set to NULL.
		* \param	columnIndex Zero based index of a bound column.
		* \return	True if column given by columnIndex is NULLable.
		* \throw	Exception If columnIndex is invalid.
		*/
		bool		IsColumnNullable(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Check if a ColumnBuffer exists at the given columnIndex.
		* \return	True if a ColumnBuffer can be accessed using passed columnIndex.
		*/
		bool		ColumnBufferExists(SQLSMALLINT columnIndex) const noexcept;


		/*!
		* \brief	Return a defined ColumnBuffer.
		* \details	Searches the internal map of ColumnBuffers for a ColumnBuffer with
		*			the given columnIndex (zero based).
		* \return	ColumnBuffer.
		* \throw	Exception If no ColumnBuffer with the passed columnIndex is found.
		*/
		const ColumnBufferPtrVariant& GetColumnBufferPtrVariant(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Return a set of columnIndexes for the ColumnBuffers of this Table.
		* \details	Returns the keyset of the internal ColumnBufferMap
		*/
		std::set<SQLUSMALLINT> GetColumnBufferIndexes() const noexcept;


		/*
		* \brief	Returns the ColumnBuffer count for this Table.
		*/
		size_t GetColumnBufferCount() const noexcept { return m_columns.size();  };


		/*!
		* \brief	Searches the internal map of ColumnBuffers for a Buffer matching the
		*			passed QueryName.
		* \see		GetColumnBufferIndex(const std::string& columnQueryName, const ColumnBufferPtrVariantMap& columns, bool caseSensitive)
		*/
		SQLUSMALLINT GetColumnBufferIndex(const std::string& columnQueryName, bool caseSensitive = true) const;


		/*!
		* \brief	Searches the passed map of ColumnBuffers for a Buffer matching the
		*			passed QueryName.
		* \param	columnQueryName Name to search for
		* \param	columns The columns to search for a matching columnQueryName
		* \param	caseSensitive search case sensitive or not
		* \throw	NotFoundException If no such ColumnBuffer is found.
		*/
		SQLUSMALLINT GetColumnBufferIndex(const std::string& columnQueryName, const ColumnBufferPtrVariantMap& columns, bool caseSensitive = true) const;


		/*!
		* \brief	Removes all columns in this table. Can only be called if Table is not open yet.
		* \details	This will remove all ColumnBuffers set on this Table (auto-created columns and
		*			manually created columns).
		* \see		IsOpen()
		* \throw	AssertionException If already open.
		*/
		void		ClearColumns();

		
		/*!
		* \brief	Define a column manually.
		* \details	Pass anything that fits into the variant of ColumnBuffers.
		* \throw	Exception If a column is already defined at passed columnIndex or if Table is already open.
		*/
		void		SetColumn(SQLUSMALLINT columnIndex, ColumnBufferPtrVariant column);


		/*!
		* \brief	Define a column manually.
		* \details	Pass a buffer and a description of the buffer to be used as a Column of this table.
		*			Internally, this will create a SqlCPointerBuffer that can be stored in the variant of the columns.
		* \param	columnIndex Zero based index of a bound column.
		* \param	queryName Name of the column matching columnIndex. This name will be used
		*			for queries.
		* \param	sqlType The SQL Type of this column. This is the database type, like SQL_VARCHAR,
		*			SQL_INTEGER, etc.
		* \param	pBuffer BufferPtrVariant that contains an allocated buffer of the type given
		*			by sqlCType.
		* \param	sqlCType SQL_C_TYPE of the buffer hold by pBuffer, like SQL_C_CHAR, SQL_C_SINTEGER, etc.
		*			This information will be forwarded to the ODBC driver while binding the column.
		*			The driver will try to convert the column-value to the given type.
		* \param	bufferSize The size of the buffer pointed to by pBuffer (in bytes).
		* \param	flags Define if a column shall be included in write-operations, is part of primary-key, etc.
		* \param	columnSize The number of digits of a decimal value (including the fractional part).
		*			This is only used if the sqlCType is SQL_C_NUMERIC, to set SQL_DESC_PRECISION.
		* \param	decimalDigits The number of digits of the fractional part of a decimal value.
		*			This is only used if the sqlCType is SQL_C_NUMERIC, to set SQL_DESC_SCALE.
		* \throw	Exception If a column is already defined at passed columnIndex or if Table is already open.
		*/
		void		SetColumn(SQLUSMALLINT columnIndex, const std::string& queryName, SQLSMALLINT sqlType, SQLPOINTER pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags, SQLINTEGER columnSize = 0, SQLSMALLINT decimalDigits = 0);


		/*!
		* \brief	Manually set the column indexes of primary key columns.
		* \details	If a table is Open()ed for AF_UPDATE_PK or AF_DELETE_PK, the primary keys must be known.
		*			Some databases are unable to return them using SQLPrimaryKeys (Access for example),
		*			so this method is provided to set primary keys indexes manually.
		*			If you call this method, the flag TOF_DO_NOT_QUERY_PRIMARY_KEYS is implicitly active
		*			during Open().
		*			During open, the flag CF_PRIMARY_KEY is set on the ColumnBuffers 
		*			that match the passed columnIndexes.
		*			Must be called before the Table is Open()ed.
		* \param	columnIndexes ColumnIndexes that are primary Key. ColumnIndexes is 0-indexed and must match
		*			the columnIndex from the actual Database table.
		*			
		* \throw	Exception If already Open().
		*/
		void		SetColumnPrimaryKeyIndexes(const std::set<SQLUSMALLINT>& columnIndexes);

		
		/*!
		* \brief	Set a Sql2BufferTypeMap. Must be set before Open() is called.
		* \param pSql2BufferTypeMap	Sql2BufferTypeMap to set.
		* \throw Exception If IsOpen() returns true.
		*/
		void		SetSql2BufferTypeMap(Sql2BufferTypeMapPtr pSql2BufferTypeMap);


		/*!
		* \brief	Get the Sql2BufferTypeMap set on this Table.
		* \return	ConstSql2BufferTypeMapPtr
		* \throw	Exception If no Sql2BufferTypeMap is set on this Table.
		*/
		ConstSql2BufferTypeMapPtr GetSql2BufferTypeMap() const;


		/*!
		* \brief	Creates the ColumnBuffers for the table and returns them as a Vector. Depending on the options passed,
		*			the columns are stored on the Table for later use during Open().
		* \details	Will query the Database about the columns of the table and create corresponding ColumnBufferPtrVariant
		*			objects.
		*			If no STableInfo is available, one is fetched from the database and remembered for later use.
		*			Creation of the buffer will fail if the SQL type of that column is not supported. If the flag
		*			skipUnsupportedColumns is set to true, this column is simply ignored.
		*			If this function fails,
		*			On success a vector of ColumnBuffers is returned. The order is the same as encountered when querying
		*			the database. The Table has stored the STableInfo now.
		* \param	skipUnsupportedColumns If set to true, the method will not fail if it encounters a Column
		*			with a SQL-Type that is not supported by the Sql2BufferTypeMap used. It will just ignore that column.
		*			If set to false, the Method will throw a NotSupportedException.
		* \param	setAsTableColumns	If set to true, those columns are set as the Columns for the Table before the vector
		*			is returned. If false, the vector is just returned without changing the columns of the Table.
		* \param	queryPrimaryKeys If true, the Database will be queried for the primary key columns of this Table.
		*			The ColumnBuffers created will be updated with the CF_PRIMARY_KEY flag if they match the returned list.
		*			If a primary key column name returned by the Database is not found in the just created ColumnBuffers,
		*			an Exception is thrown.
		* \throw	Exception If no Columns are found or if not all primary key flags can be set.
		*/
		std::vector<ColumnBufferPtrVariant> CreateAutoColumnBufferPtrs(bool skipUnsupportedColumns, bool setAsTableColumns, bool queryPrimaryKeys);


		/*!
		* \brief	Set a ColumnFlag on a Column defined on this Table.
		* Exception	If Table is already open or columnIndex is not defined.
		*/
		void SetColumnFlag(SQLUSMALLINT columnIndex, ColumnFlag flag) const;


		/*!
		* \brief	Clear a ColumnFlag on a Column defined on this Table.
		* Exception	If Table is already open or columnIndex is not defined.
		*/
		void ClearColumnFlag(SQLUSMALLINT columnIndex, ColumnFlag flag) const;


		/*!
		* \brief	Returns a all ColumnBuffers that have the flag CF_PRIMARY_KEY set 
		*/
		ColumnBufferPtrVariantMap GetPrimaryKeyColumnBuffers() const;


		/*!
		* \brief	Returns the TableInfo for this Table. Throws an AssertionException
		*			if Table has no TableInfo stored. See HasTableInfo().
		* \throw	Exception If no TableInfo is stored in this Table.
		*/
		const TableInfo& GetTableInfo() const;


		// Private stuff
		// -------------
	private:
		/*!
		* \brief	Allocate the statement handles required by this Table.
		* \details  Allocates the statements using the connection handle from the Database
		*			passed in Constructor.
		*			If Allocating one statement fails, all statements are reseted before the
		*			Exception is thrown.
		*			This will also allocate the buffer required for the count statement
		* \see		HasStatements()
		*
		* \throw	Exception If any of the handles to be allocated is not null currently.
		*/
		void AllocateStatements(bool scrollableCursors);

	
		/*!
		* \brief	Frees all handles that are not set to null. Freed handles are set to NULL.
		* \details	This will also free the buffer used by the count statement.
		* \throw	SqlResultException if freeing one of the handles fail. No other handles
		*			will be freed after one handle fails to free.
		*/
		void		FreeStatements();


		/*!
		* \brief	Iterates the columns defined and creates a field-statement of 
		*			the Columns that have the flag CF_SELECT set.
		* \return	A string in the form "Field1, Field2, .., FieldN"
		* \throw	Exception If no Columns are defined.
		*/
		std::string BuildSelectFieldsStatement() const;


		/*!
		* \brief	Prepares an SQL INSERT Statement to insert values for all bound columns.
		* \throw	Exception If no ColumnBuffers are bound, not opened for writing, etc., or binding fails.
		*/
		void		BindInsertParameters();
		
		
		/*!
		* \brief	Prepares an SQL DELETE Statement to delete the row identified by the primary key values.
		* \throw	Exception If no ColumnBuffers are bound, not opened for writing, etc., or binding fails
		*			or this Table has no bound primary key columns.
		*/
		void		BindDeletePkParameters();
		

		/*!
		* \brief	Prepares an SQL UPDATE Statement to update the values of the (non-primary-keys) columns
		*			with the flag CF_UPDATE set. Where statement uses the primary key columns. And the
		*			primary key columns are not updated!
		* \throw	Exception If no ColumnBuffers are bound, not opened for writing, etc., or binding fails
		*			or this Table has no bound primary key columns.
		*/
		void		BindUpdatePkParameters();


		/*!
		* \brief	Prepares an SQL SELECT Statement to select all columns with the flag CF_SELECT set.
		*			WHERE statement is formed using all columns with the flag CF_PRIMARY_KEY set.
		* \throw	Exception
		*/
		void		BindSelectPkParameters();


		/*!
		* \brief	Return a defined ColumnBuffer that is not NULL.
		* \details	Searches the internal map of ColumnBuffers for a ColumnBuffer with
		*			the given columnIndex (zero based).
		* \return	ColumnBuffer.
		* \throw	Exception If no columnBuffer found, or if the ColumnBuffer is Null.
		* \throw	NullValueException if the value is NULL.
		*/
		const ColumnBufferPtrVariant& GetNonNullColumnBufferPtrVariant(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Queries the Database about the primary keys of this table,
		*			tries to identify the corresponding columns (by comparing names)
		*			and sets the flag on the corresponding column.
		* \throw	Exception If querying fails, or not for all queried primary keys column a 
		*			corresponding ColumnBuffer is found in the passed columns map.
		*/
		void	QueryPrimaryKeysAndUpdateColumns(const ColumnBufferPtrVariantMap& columns) const;


		/*!
		* \brief	Throws an Exception if a Column has an flag set that violates the TableAccessFlags.
		*			Like CF_INSERT is set, but TableAccessFlags are only AF_READ
		*/
		void	CheckColumnFlags() const;


		/*!
		* \brief	Checks for every Column if its SQL type matches one of the SQL types reported
		*			as supported types by the Database.
		*			If removeUnsupported is true, a not supported Column is removed from the internal map.
		*			Else, an NotSupportedException is thrown
		*/
		void	CheckSqlTypes(bool removeUnsupported);


		/*!
		* \brief	Returns the internally stored TableInfo for this Table, or queries the Database
		*			about the TableInfo for this table, stores that TableInfo internally and returns it.
		* \details	If the TableInfo has already been queried and is stored internally, the internally
		*			stored TableInfo is returned. Else the Database is queried for a TableInfo matching
		*			the given search-names during construction. If exactly one matching Table is found,
		*			the corresponding TableInfo is stored internally and returned.
		* \throw	Exception If no TableInfo is stored internally and not exactly one matching
		*			TableInfo can be read from the Database.
		*/
		const TableInfo& ReadTableInfo();


		ConstDatabasePtr		m_pDb;	///< Database this table belongs to.
		Sql2BufferTypeMapPtr		m_pSql2BufferTypeMap;	///< Sql2BufferTypeMap to be used by this Table. Set during Construction by reading from Database, or altered using Setters.

		// Executable Statements used
		ExecutableStatement	m_execStmtCountWhere;	///< Statement to COUNT. First Column of Result is bound to m_pSelectCountResultBuffer.
		ExecutableStatement m_execStmtSelect;	///< Statement to SELECT columns with flag CF_SELECT. WHERE clause is passed manually or primary key columns are used.
		ExecutableStatement m_execStmtInsert;	///< Statement to INSERT. Prepared SQL statement bound to all params with flag CF_INSERT.
		ExecutableStatement m_execStmtUpdatePk;	///< Statement to UPDATE columns with flag CF_UPDATE. WHERE clause is formed using primary key columns.
		ExecutableStatement m_execStmtDeletePk; ///< Statement to DELETE. WHERE clause is formed using primary key columns.
		UBigIntColumnBufferPtr m_pSelectCountResultBuffer;	///< The buffer used to retrieve the result of a SELECT COUNT operation.

		// Table Information
		bool				m_haveTableInfo;		///< True if m_tableInfo has been set
		TableInfo			m_tableInfo;			///< TableInfo fetched from the db or set through constructor
		bool				m_isOpen;				///< Set to true after Open has been called
		TableOpenFlags		m_openFlags;			///< Flags used to open the table in the call to Open().
		TableAccessFlags	m_tableAccessFlags;		///< Bit mask for the AccessFlag flags. Column flags are derived from those if not set explicitly.

		// Column information
		std::set<SQLUSMALLINT> m_primaryKeyColumnIndexes;	///< If this set contains values during Open(), the flag TOF_DO_NOT_QUERY_PRIMARY_KEYS is activated implicitly. 
															///< The ColumnBuffers marked in this set will be used as primary key columns.
															///< Used if columns are not defined manually but queried, but the Database does not support SQLPrimaryKeys.

		ColumnBufferPtrVariantMap m_columns;	///< A map with ColumnBuffers, key is the column-Index (starting at 0).

		bool				m_autoCreatedColumns;		///< If true the columns were created during Open() automatically by querying the database about the table.

		// Table information set during construction, that was used to find the matching TableInfo if none was passed
		std::string  m_initialTableName;		///< Table name set on initialization
		std::string	m_initialSchemaName;	///< Schema name set on initialization
		std::string	m_initialCatalogName;	///< Catalog name set on initialization
		std::string	m_initialTypeName;		///< Type name set on initialization

	};  // Table

}	// namespace exodbc

