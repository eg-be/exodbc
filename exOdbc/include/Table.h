/*!
* \file Table.h
* \author Elias Gerber <eg@elisium.ch>
* \date 25.07.2014
* \brief Header file for the Table class and its helpers.
* \copyright wxWindows Library Licence, Version 3.1
*
* Header file for the Table class and its helpers.
*/

#pragma once
#ifndef TABLE_H
#define TABLE_H

// Same component headers
#include "exOdbc.h"
#include "ColumnBuffer.h"
#include "TablePrivileges.h"
#include "Exception.h"
#include "ObjectName.h"
#include "Sql2BufferTypeMap.h"

// Other headers
#include "boost/any.hpp"

// System headers

// Forward declarations
// --------------------
namespace exodbc
{
	class Database;
}

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
	* This class will allocate the Statements-Handles that are required
	* to query and modify the table. Which statements handles are allocated depends
	* on the AccessFlags set.
	*
	* After creating the Table you can manually define which columns are bound
	* for data-exchange using SetColumn().
	*
	* If no Columns have been bound manually when Open() is called, the Table will
	* query the Database about the Columns and bind all Columns of the Table.
	*
	* Columns bound to the table will be updated with the values of the current record
	* this table points to. Calling methods like Update(), Delete() or Insert() will
	* modify the table in the database using the current values.
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
		FRIEND_TEST(TableTest, CopyCtr);
#endif

	public:

		/*!
		* \brief	Default Constructor.
		* \details	You must call one of the Init() methods if the Table was created using
		*			this Default Constructor.
		*/
		Table() throw();


		/*!
		* \brief	Create new table using the passed TableInfo.
		* \see		Init(const Database*, AccessFlags, const TableInfo&)
		* \throw	Exception
		*/
		Table(const Database* pDb, AccessFlags afs, const TableInfo& tableInfo);


		/*!
		* \brief	Create a new Table using the passed search-names.
		* \see		Init(const Database* pDb, AccessFlags, const std::wstring&, const std::wstring&, const std::wstring&, const std::wstring&)
		* \throw	Exception
		*/
		Table(const Database* pDb, AccessFlags afs, const std::wstring& tableName, const std::wstring& schemaName = L"", const std::wstring& catalogName = L"", const std::wstring& tableType = L"");


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
		void Init(const Database* pDb, AccessFlags afs, const TableInfo& tableInfo);


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
		void Init(const Database* pDb, AccessFlags afs, const std::wstring& tableName, const std::wstring& schemaName = L"", const std::wstring& catalogName = L"", const std::wstring& tableType = L"");
			

		/*!
		* \brief	Test if the required statements (depending on OpenFlags) are allocated.
		* \return	True if all statements are allocated
		* \see		AllocateStatements()
		*/
		bool HasAllStatements() const throw();


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
		*  - TOF_CHECK_PRIVILEGES  
		*			If set, the database will be queried checking if the current user
		*			is allowed to do the required operations like Select, Update, Insert or Delete,
		*			depending on the AccessFlags value passed during construction or set later.
		*  - TOF_CHECK_EXISTANCE:
		*			If set, the database will always be queried if a table matching the
		*			passed definition during constructions actually exists in the database. Unsetting this
		*			flag makes only sense if you've passed a TableInfo during construction, 
		*			as else the Table is required to query the database anyway (and fails if not found).
		*  - TOF_SKIP_UNSUPPORTED_COLUMNS:
		*			If set, Columns for which no ColumnBuffer can be created are skipped. If not set an
		*			Exception is thrown if creation of a ColumnBuffer fails (default).\n
		*			If Columns are created automatically, the Database might report a SQL type that is not
		*			implemented (No mapping of SQL-Type o SQL-C-Type). Set this flag to simply skip such columns.\n
		*			If Columns have been set manually, Columns might have been defined using a SQL Type that
		*			is not reported as supported from the Database. Set this flag to simply skip such
		*			columns. See also TOF_IGNORE_DB_TYPE_INFOS.\n
		*			If ColumnBuffers are skipped, the indexes of the bound ColumnBuffers will still match the 
		*			indexes of the actual table, but there are gaps in the key-sequence of the ColumnBuffer map.
		*  - TOF_IGNORE_DB_TYPE_INFOS:
		*			If set, the SQL Types of manually defined columns are not validated against the supported
		*			types reported by the Database. If this flag is set, the flag TOF_SKIP_UNSUPPORTED_COLUMNS
		*			does nothing for manually defined columns.
		*  - TOF_CHAR_TRIM_LEFT:
		*			If set, values retrieved using GetStringValue(SQLSMALLINT columnIndex)
		*			or GetWStringValue(SQLSMALLINT columnIndex) are trimmed on the left
		*			before being set on str.
		*  - TOF_CHAR_TRIM_RIGHT:
		*			If set, values retrieved using GetStringValue(SQLSMALLINT columnIndex)
		*			or GetWStringValue(SQLSMALLINT columnIndex) are trimmed on the right
		*			before being set on str.
		*  - TOF_DO_NOT_QUERY_PRIMARY_KEYS:
		*			If a table is opened with the AccessFlag::AF_UPDATE_PK or AccessFlag::AF_DELETE_PK set,
		*			Primary keys are required. They are queried from the Database during Open(), unless
		*			this flag is set, or the column indexes of the Table have been passed prior to Open()
		*			using SetColumnPrimaryKeyIndexes().\n
		*			If this flag is set, the primary keys are not queried.\n
		*			This flag is set automatically whenever Open() ing a Table from a Microsoft Access Database,
		*			as the Access Driver I used for testing (ODBCJT32.DLL, v6.01.7601.17632, 
		*			'Microsoft Access Driver (*.mdb')') does not support the SQLPrimaryKeys() method.
		*			This flag is sometimes active implicitly, for example if you have manually defined 
		*			primary key columns using SetColumnPrimaryKeyIndexes().
		*  - TOF_FORWARD_ONLY_CURSORS:
		*			If the Database supports Scrollable Cursors, the Table will try to use Scrollable Cursors
		*			on the Query Statements. If this flag is set, the Table will always use forward-only Cursors.
		* \see		IsOpen()
		* \see		Close()
		* \see		SetColumn()
		* \throw	Exception If already open, table is not found, columns fail to bind..
		*/
		void		Open(TableOpenFlags openFlags = TOF_CHECK_EXISTANCE);


		/*!
		* \brief	Close the table.
		* \details	Unbinds and deletes all ColumnBuffers bound to this table. The information about
		*			this table in the TableInfo queried during Open() is kept for future use.
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
		bool		IsOpen() const throw() { return m_isOpen; };


		/*!
		* \brief	Checks if we can only read from this table.
		* \return	True if this table has the flag AF_READ set and none of the flags
		*			AF_UPDATE_PK, AF_UPDATE_WHERE, AF_INSERT, AF_DELETE_PK or AF_DELETE_WHERE are set.
		*/
		bool		IsQueryOnly() const throw();


		/*!
		* \brief	Checks if this table is a table with manually defined columns.
		* \return	True if this table was constructed using a Constructor for a table
		*			with manual columns.
		*/
		bool		IsManualColumns() const throw() { return m_manualColumns; };


		/*!
		* \brief	Set an AccessFlag. Can only be called if Table is Closed, so
		*			when IsOpen() returns false.
		* \details	If the flag passed is already set, this function will do nothing.
		*			If a change is detected, and the statements were already allocated,
		*			the function will free all statements and re-allocate the statements
		*			required for the current AccessFlags set.
		* \throw	Exception If Table is already open, or freeing / allocating statement handles fail.
		*/
		void		SetAccessFlag(AccessFlag ac);


		/*!
		* \brief	Clear an AccessFlag. Can only be called if Table is Closed, so
		*			when IsOpen() returns false.
		* \details	If the flag passed is already cleared, this function will do nothing.
		*			If a change is detected, and the statements were already allocated,
		*			the function will free all statements and re-allocate the statements
		*			required for the current AccessFlags set.
		* \throw	Exception If Table is already open, or freeing / allocating statement handles fail.
		*/
		void		ClearAccessFlag(AccessFlag ac);


		/*!
		* \brief	Set multiple AccessFlags. Can only be called if Table is Closed, so
		*			when IsOpen() returns false.
		* \details	If the flags passed are the same as already set on the table, this
		*			function will do nothing.
		*
		*			If a change is detected, and the statements were already allocated,
		*			the function will free all statements and re-allocate the statements
		*			required for the current AccessFlags set.
		* \throw	Exception If Table is already open, or freeing / allocating statement handles fail.
		*/
		void		SetAccessFlags(AccessFlags acs);


		/*!
		* \brief	Test if an AccessFlag is set.
		*/
		bool		TestAccessFlag(AccessFlag ac) const throw();


		/*!
		* \brief	Get the AccessFlags bitmask.
		*/
		AccessFlags	GetAccessFlags() const throw() { return m_accessFlags; };


		/*!
		* \brief	Test if a TableOpenFlag is set.
		*/
		bool		TestOpenFlag(TableOpenFlag flag) const throw();


		/*!
		* \brief	Sets or Clears the TOF_TRIM_RIGHT flag.
		* \details	Note that the value set here is overriden by the value
		*			passed to Open() in the TableOpenFlags.
		*/
		void		SetCharTrimRight(bool trimRight) throw();


		/*!
		* \brief	Sets or Clears the TOF_TRIM_LEFT flag.
		* \details	Note that the value set here is overriden by the value
		*			passed to Open() in the TableOpenFlags.
		*/
		void		SetCharTrimLeft(bool trimLeft) throw();


		/*!
		* \brief	Check if the Table-Information is set on this Table.
		* \details	Returns true if the internal member of the TableInfo contains a value either
		*			set during Construction or fetched from the Database during Open().
		* \see		GetTableInfo()
		* \return	Returns true if this table has a TableInfo set that can be fetched using GetTableInfo()
		*/
		bool		HasTableInfo() const throw() { return m_haveTableInfo; }


		/*!
		* \brief	Return the Table information of this Table.
		* \details	Returns the TableInfo of this table, if one has been set either during construction
		*			or one was read during Open().
		* \see		HasTableInfo()
		* \throw	Exception if no table info is available.
		*/
		TableInfo	GetTableInfo() const;


		/*!
		* \brief	Counts how many rows would be selected in this table by the passed WHERE clause.
		* \details	If whereStatement is empty, no WHERE clause is added
		* \param	whereStatement Do not include 'WHERE' in the passed where clause
		* \return	count The result of a 'SELECT COUNT(*) WHERE whereStatement' on the current table
		* \throw	Exception If failed.
		*/
		SQLUBIGINT		Count(const std::wstring& whereStatement);


		/*!
		* \brief	Executes a 'SELECT col1, col2, .., colN' for the Table using the passed WHERE clause.
		* \details	The SELECT-Query is built using the column information available to this Table.
		*			It does not use the '*'. Only bound ColumnBuffers names are included in the statement.
		*			If successful, a Select-Query is open. You can iterate the records
		*			using SelectNext() to access the values of the records.
		*			The cursor is positioned before the first records, so you must call
		*			SelectNext() to access the first record.
		*			If whereStatement is empty, no WHERE clause is added.
		*			If a select statement is open, the statement is closed first.
		* \param	whereStatement Do not include 'WHERE' in the passed where clause
		* \see		SelectNext()
		* \see		SelectClose()
		* \throw	Exception If failed.
		*/
		void		Select(const std::wstring& whereStatement = L"");


		/*!
		* \brief	Executes the passed SQL statement on the open Table.
		* \details	Query by passing the complete SQL statement.
		*			If successful, a Select-Query is open. You can iterate the records
		*			using SelectNext() to access the values of the records.
		*			The cursor is positioned before the first records, so you must call
		*			SelectNext() to access the first record.
		*			If a select statement is open, the statement is closed first.
		* \param	sqlStmt Must be a full SQL statement like 'SELECT foo FROM A WHERE x > 3'
		* \see		SelectNext()
		* \see		SelectClose();
		* \return	True if successful
		* \throw	Exception If failed.
		*/
		void		SelectBySqlStmt(const std::wstring& sqlStmt);


		/*!
		* \brief	Fetches the next record fromt the current active Select() recordset.
		* \seee		Select()
		* \return	True if next record has been fetched, false if no more records exist.
		* \throw	Exception If no SelectQuery is open.
		*/
		bool		SelectNext();


		/*!
		* \brief	Fetches the previous record fromt the current active Select() recordset.
		* \seee		Select()
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
		* \brief	A wrapper to SQLColAttributes, to fetch attributes of the columns of an open result set.
		* \details	Can only be called if a Select() is open. Only for numeric attributes.
		* \param	columnIndex zero based index of the column available in the result set.
		* \param	attr Value to set.
		* \see		http://msdn.microsoft.com/en-us/library/ms713558%28v=vs.85%29.aspx
		* \return	Attribute value.
		* \throw	Exception
		*/
		SQLLEN		SelectColumnAttribute(SQLSMALLINT columnIndex, ColumnAttribute attr);


		/*!
		* \brief	Check if a Select() Query is open.
		* \return	True if a Select() Query is open and rows can be iterated using SelectNext()
		*/
		bool		IsSelectOpen() const { return m_selectQueryOpen; };


		/*!
		* \brief	Inserts the current values into the database as a new row.
		* \details	The values in the ColumnBuffer currently bound will be inserted
		*			into the database.
		*			An prepared INSERT statement is used to insert the values identified
		*			by the bound ColumnBuffers that have the flag ColumnFlags::CF_INSERT set.
		*			Fails if the table has not been opened using READ_WRITE.
		*			This will not commit the transaction.
		* \see		Database::CommitTrans()
		* \throw	Exception on failure.
		*/
		void		Insert() const;


		/*!
		* \brief	Deletes the row identified by the values of the bound primary key columns.
		* \details	A prepared DELETE statement is used to delete the row that matches all
		*			primary keys of this Table. The key values are read from the ColumnBuffers bound
		*			to the primary key columns. You can either use the Select(), SelectNext(), etc.
		*			functions to load the key values of a record to delete into the buffer, or
		*			you can manually set the values in the buffers that match the primary key columns.
		*			Fails if not all primary keys are bound.
		*			Fails if no primary keys are set on the table.
		*			Fails if the table has not been opened using READ_WRITE.
		*			This will not commit the transaction.
		* \param	failOnNoData If set to true the function will return false if the result of
		*			the DELETE is SQL_NO_DATA.
		* \see		Database::CommitTrans()
		* \throw	Exception on failure, or depending on failOnNoData, a SqlResultException if the 
		*			call to SQLExecute fails with SQL_NO_DATA.
		*/
		void		Delete(bool failOnNoData = true);


		/*!
		* \brief	Delete the row(s) identified by the passed where statement.
		* \details	A DELETE statement for this Table is created, using the passed
		*			WHERE clause.
		*			This uses an independent statement and will not modify the values in
		*			the ColumnBuffers or any ongoing Select() call.
		*			Fails if the table has not been opened using READ_WRITE.
		*			Fails if where is empty.
		*			This will not commit the transaction.
		* \param	where WHERE clause to be used. Do not include 'WHERE', the Table will add this.
		*			Not allowed to be empty.
		* \param	failOnNoData If set to true the function will return false if the result of
		*			the DELETE is SQL_NO_DATA.
		* \see		Database::CommitTrans()
		* \throw	Exception on failure, or depending on failOnNoData, a SqlResultException if the
		*			call to SQLExecute fails with SQL_NO_DATA.
		*/
		void		Delete(const std::wstring& where, bool failOnNoData = true) const;


		/*!
		* \brief	Updates the row identified by the values of the bound primary key columns with
		*			the values in bound ColumnBuffers, if the ColumnBuffer has the ColumnFlags::CF_UPDATE
		*			set.
		* \details	A prepared UPDATE statement is used to update the row(s) that matches all
		*			primary key values of this Table. The key values are read from the ColumnBuffers bound
		*			to the primary key columns. You can either use the Select(), SelectNext(), etc.
		*			functions to load the key values of a record to delete into the buffer, or
		*			you can manually set the values in the buffers that match the primary key columns.
		*			The prepared statement will update the bound columns where the ColumnBuffer has 
		*			the flag ColumnFlags::CF_UPDATE set and the flag ColumnFlags::CF_PRIMARY_KEY is not set.
		*			Fails if not all primary keys are bound.
		*			Fails if no primary keys are set on the table.
		*			Fails if the table has not been opened using READ_WRITE.
		*			Fails if no bound columns have the flag CF_UPDATE set.
		*			This will not commit the transaction.
		* \see		Database::CommitTrans()
		* \throw	Exception if failed.
		*/
		void		Update();


		/*!
		* \brief	Updates the row identified by the passed where statement with
		*			the values in bound ColumnBuffers, if the ColumnBuffer has the ColumnFlags::CF_UPDATE
		*			set.
		* \details	A prepared UPDATE statement is used to update the row(s) that match the passed where statement.
		*			A prepared statement is created to update the columns bound to ColumnBuffers that have the 
		*			flag ColumnFlags::CF_UPDATE set with the values stored in the ColumnBuffer.
		*			Note that this will also update the bound primary-key columns (those that have the flag
		*			ColumnFlags::CF_PRIMARY_KEY set), if the column has the flag ColumnFlags::CF_UPDATE set.
		*			Fails if the table has not been opened using READ_WRITE.
		*			Fails if where is empty.
		*			This will not commit the transaction.
		* \param	where WHERE clause to be used. Do not include 'WHERE', the Table will add this.
		* \see		Database::CommitTrans()
		* \throw	Exception if failed.
		*/
		void		Update(const std::wstring& where);


		/*!
		* \brief	Set the value of the ColumnBuffer given by columnIndex.
		* \param	columnIndex Zero based ColumnBuffer index.
		* \param	value Value to set.
		* \throw	Exception If ColumnBuffer not found, or setting the value fails, for
		*			example because it does not match the type of the buffer allocated, or
		*			the value is NULL but the column not NULLABLE, etc.
		*/
		void		SetColumnValue(SQLSMALLINT columnIndex, const BufferVariant& value) const;


		/*!
		* \brief	Set a binary value of the ColumnBuffer given by columnIndex.
		* \details	This will fail if the corresponding ColumnBuffer is not bound as a
		*			binary buffer.
		* \param	columnIndex Zero based ColumnBuffer index.
		* \param	buffer Binary value to set.
		* \param	bufferSize length of buffer.
		* \throw	Exception If ColumnBuffer not found, or setting the value fails.
		*/
		void		SetBinaryValue(SQLSMALLINT columnIndex, const SQLCHAR* buffer, SQLINTEGER bufferSize);


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
		* \brief	Access the current value of columnIndex as SQLSMALLINT.
		* \details	Casts the value if casting is possible without loosing data 
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		SQLSMALLINT GetSmallInt(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Access the current value of columnIndex as SQLINTEGER.
		* \details	Casts the value if casting is possible without loosing data
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		SQLINTEGER	GetInt(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Access the current value of columnIndex as SQLBIGINT.
		* \details	Casts the value if casting is possible without loosing data
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		SQLBIGINT	GetBigInt(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Access the current value of columnIndex as SQL_DATE_STRUCT.
		* \details	Casts the value if casting is possible without loosing data
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		SQL_DATE_STRUCT GetDate(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Access the current value of columnIndex as SQL_TIME_STRUCT.
		* \details	Casts the value if casting is possible without loosing data
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		SQL_TIME_STRUCT GetTime(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Access the current value of columnIndex as SQL_TIMESTAMP_STRUCT.
		* \details	Casts the value if casting is possible without loosing data
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		SQL_TIMESTAMP_STRUCT GetTimeStamp(SQLSMALLINT columnIndex) const;

#if HAVE_MSODBCSQL_H
		/*!
		* \brief	Access the current value of columnIndex as SQL_SS_TIME2_STRUCT.
		* \details	Casts the value if casting is possible without loosing data
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		SQL_SS_TIME2_STRUCT GetTime2(SQLSMALLINT columnIndex) const;
#endif

		/*!
		* \brief	Access the current value of columnIndex as std::string.
		* \details	Casts the value if casting is possible without loosing data
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		std::string GetString(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Access the current value of columnIndex as std::wstring.
		* \details	Casts the value if casting is possible without loosing data
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		std::wstring GetWString(SQLSMALLINT columnIndex) const;
		

		/*!
		* \brief	Access the current value of columnIndex as SQLDOUBLE.
		* \details	Casts the value if casting is possible without loosing data
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		SQLDOUBLE GetDouble(SQLSMALLINT columnIndex) const;

		
		/*!
		* \brief	Access the current value of columnIndex as SQLREAL.
		* \details	Casts the value if casting is possible without loosing data
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		SQLREAL GetReal(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Access the current value of columnIndex as SQL_NUMERIC_STRUCT.
		* \details	Casts the value if casting is possible without loosing data
		* \param	columnIndex Zero based index of a bound column.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		SQL_NUMERIC_STRUCT GetNumeric(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Get the value of the ColumnBuffer given by columnIndex as BufferVariant.
		* \param	columnIndex Zero based ColumnBuffer index.
		* \throw	Exception If ColumnBuffer not found, or the value held by the ColumnBuffer
		*			cannot be returned as BufferVariant.
		*			Note that it will not throw if the value is a NULL value, the BufferVariant will hold
		*			the NullValue::IS_NULL indicator then.
		* \see		ColumnBuffer::GetValue()
		* \see		BufferVariant
		*/
		BufferVariant GetColumnValue(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Get the value of a binary ColumnBuffer given by columnIndex.
		* \details	Returns a pointer to the buffer of the corresponding ColumnBuffer.
		*			Every column, regardless of its type, can be retrieved.
		* \param	columnIndex Zero based ColumnBuffer index.
		* \param	bufferSize Will be filled with the size of the buffer returned.
		* \param	lengthIndicator Will be filled with the length of the data in the buffer returned.
		* \throw	Exception If ColumnBuffer not found, or if the Column is NULL.
		*/
		const SQLCHAR* GetBinaryValue(SQLSMALLINT columnIndex, SQLLEN& bufferSize, SQLLEN& lengthIndicator) const;


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
		bool		ColumnBufferExists(SQLSMALLINT columnIndex) const throw();


		/*!
		* \brief	Return a defined ColumnBuffer.
		* \details	Searches the internal map of ColumnBuffers for a ColumnBuffer with
		*			the given columnIndex (zero based).
		* \return	ColumnBuffer.
		* \throw	Exception If no ColumnBuffer with the passed columnIndex is found.
		*/
		ColumnBuffer* GetColumnBuffer(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Return a set of columnIndexes for the ColumnBuffers of this Table.
		* \details	Returns the keyset of the internal ColumnBufferMap
		*/
		std::set<SQLSMALLINT> GetColumnBufferIndexes() const throw();


		/*!
		* \brief	Searches the internal map of ColumnBuffers for a Buffer matching the
		*			passed QueryName.
		* \param	caseSensitive 
		* \throw	NotFoundException If no such ColumnBuffer is found.
		*/
		SQLSMALLINT GetColumnBufferIndex(const std::wstring& columnQueryName, bool caseSensitive = true) const;


		/*!
		* \brief	Define a column manually. The column is bound during Open().
		* \details	Pass a buffer and a description of the buffer that shall be bound to a
		*			table column once Open() is called.
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
		* \param	bufferSize The size of the buffer pointed to by pBuffer.
		* \param	flags Define if a column shall be included in write-operations, is part of primary-key, etc.
		* \param	columnSize The number of digits of a decimal value (including the fractional part).
		*			This is only used if the sqlCType is SQL_C_NUMERIC, to set SQL_DESC_PRECISION.
		* \param	decimalDigits The number of digits of the fractional part of a decimal value.
		*			This is only used if the sqlCType is SQL_C_NUMERIC, to set SQL_DESC_SCALE.
		* \throw	Exception If CF_INSERT or CF_UPDATE is set as ColumnFlags.
		*/
		void		SetColumn(SQLUSMALLINT columnIndex, const std::wstring& queryName, SQLSMALLINT sqlType, BufferPtrVariant pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags, SQLINTEGER columnSize = -1, SQLSMALLINT decimalDigits = -1);


		/*!
		* \brief	Manually set the column indexes of primary key columns.
		* \details	If a table is Open()ed for AF_UPDATE or AF_DELETE, the primary keys must be known.
		*			Some databases are unable to return them using SQLPrimaryKeys (Access for example),
		*			so this method is provided to set primary keys indexes manually.
		*			If you call this method, the flag TOF_DO_NOT_QUERY_PRIMARY_KEYS is implicitly active
		*			during Open().
		*			After ColumnBuffers have been created, the flag CF_PRIMARY_KEY is set on the ColumnBuffers 
		*			that match the passed columnIndexes.
		*			Must be called before the Table is Open()ed.
		* \param	columnIndexes ColumnIndexes that are primary Key. ColumnIndexes is 0-indexed and must match
		*			the columnIndex from the actual Database table.
		*			
		* \throw	Exception If already Open().
		*/
		void		SetColumnPrimaryKeyIndexes(const std::set<SQLUSMALLINT>& columnIndexes);

		
		/*!
		* \brief	Set a Sql2BufferTypeMap. Must be set before Open() is called. Object must not be
		*			deleted before Table is deleted.
		* \param pSql2BufferTypeMap	Sql2BufferTypeMap to set.
		* \throw Exception If IsOpen() returns true.
		*/
		void		SetSql2BufferTypeMap(Sql2BufferTypeMapPtr pSql2BufferTypeMap);


		/*!
		* \brief	Get the Sql2BufferTypeMap set on this Table.
		* \return	const Sql2BufferTypeMap*
		* \throw	Exception If no Sql2BufferTypeMap is set on this Table.
		*/
		const Sql2BufferTypeMapPtr GetSql2BufferTypeMap() const;


		// Private stuff
		// -------------
	private:
		/*!
		* \brief	Allocate the statement handles required by this Table.
		* \details  Allocates the statements using the connection handle from the Database
		*			passed in Constructor.
		* \see		HasStatements()
		*
		* \throw	Exception If any of the handles to be allocated is not null currently.
		*/
		void AllocateStatements();


		/*!
		* \brief	Set Options on Statement handles related to Cursor things.
		* \throw Exception
		*/
		void SetCursorOptions(bool forwardOnlyCursors);


		/*!
		* \brief	Creates the ColumnBuffers for the table.
		* \detailed	Can only be called if m_manulColumns is set to false. Will query the Database about the
		*			column of the table and allocate corresponding column buffers.
		*			Creation of the buffer will fail if the SQL type of that column is not supported. If the flag
		*			skipUnsupportedColumns is set to true, this column is simply ignored. Else the function fails.
		*			If this function fails, all allocated buffers are deleted and m_columnBuffers is cleared.
		*			On success, after this function returns m_columnBuffers contains the allocated Buffers to be used
		*			with this table.
		*			Note that in case columns have been skipped, the keys to the table might contain "gaps", say columns 2 failed
		*			to bind, the keys will so something like 1, 3, 4, .. 
		* \throw	Exception If m_manualColumns is set to true, or m_columnBuffers is not empty, or creation of ColumnBuffers fails.
		*/
		void		CreateAutoColumnBuffers(const TableInfo& tableInfo, bool skipUnsupportedColumns);

		
		/*!
		* \brief	Frees all handles that are not set to null. Freed handles are set to NULL.
		* \throw	SqlResultException if freeing one of the handles fail. No other handles
		*			will be freed after one handle fails to free.
		*/
		void		FreeStatements();


		/*!
		* \brief	Wrapper to SQLFetchScroll
		* \throw	Exception if TOF_FORWARD_ONLY_CURSORS is set, or no Select-Statement is open,
		*			or if SQLFetchScroll does not return with SQL_SUCCEEDED or SQL_NO_DATA.
		*/
		bool		SelectFetchScroll(SQLSMALLINT fetchOrientation, SQLLEN fetchOffset);


		/*!
		* \brief	Iterates the bound columns and returns the field part of a statement.
		* \details	Queries each bound column for its SqlName.
		* \return	A string in the form "Field1, Field2, .., FieldN"
		* \throw	Exception If no ColumnBuffers are bound.
		*/
		std::wstring BuildFieldsStatement() const;


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
		void		BindDeleteParameters();
		

		/*!
		* \brief	Prepares an SQL UPDATE Statement to update the values of the (non-primary-keys) bound columns.
		*			The rows matching the bound primary keys will be bound.
		* \throw	Exception If no ColumnBuffers are bound, not opened for writing, etc., or binding fails
		*			or this Table has no bound primary key columns.
		*/
		void		BindUpdateParameters();


		/*!
		* \brief	Return a defined ColumnBuffer that is not NULL.
		* \details	Searches the internal map of ColumnBuffers for a ColumnBuffer with
		*			the given columnIndex (zero based).
		* \return	ColumnBuffer.
		* \throw	Exception If no columnBuffer found, or if the ColumnBuffer is Null.
		* \throw	NullValueException if the value is NULL.
		*/
		ColumnBuffer* GetNonNullColumnBuffer(SQLSMALLINT columnIndex) const;

		const Database*				m_pDb;	///< Database this table belongs to.
		Sql2BufferTypeMapPtr		m_pSql2BufferTypeMap;	///< Sql2BufferTypeMap to be used by this Table. Set during Construction by reading from Database, or altered using Setters.

		// ODBC Handles
		SQLHSTMT		m_hStmtSelect;	///< Statement-handle used to do SELECTs. Columns are bound.
		SQLHSTMT		m_hStmtCount;	///< Statement-handle used to do COUNTs. Columns are not bound.
		SQLHSTMT		m_hStmtInsert;	///< Statement-handle used to do INSERTs. Columns are bound, a prepared statement using column-markers is created.
		SQLHSTMT		m_hStmtDeletePk;	///< Statement-handle used to do DELETs. Primary key columns are bound, a prepared statement using column-markers is created.
		SQLHSTMT		m_hStmtUpdatePk;	///< Statement-handle used to do UPDATEs. Primary key columns are bound, a prepared statement using column-markers is created.
		SQLHSTMT		m_hStmtDeleteWhere;	///< Statement-handle to do DELETEs using a passed WHERE clause.
		SQLHSTMT		m_hStmtUpdateWhere;	///< Statement-handle to do UPDATEs using a passed WHERE clause.

		bool		m_selectQueryOpen;	///< Set to True once a successful Select(), set to false on SelectClose()

		// Table Information
		bool				m_haveTableInfo;		///< True if m_tableInfo has been set
		TableInfo			m_tableInfo;			///< TableInfo fetched from the db or set through constructor
		bool				m_isOpen;				///< Set to true after Open has been called
		TableOpenFlags		m_openFlags;			///< Flags used to open the table in the call to Open().
		AccessFlags			m_accessFlags;			///< Bitmask for the AccessFlag flags.
		TablePrivileges		m_tablePrivileges;		///< Table Privileges read during open if checkPermission was set.

		// Column information
		std::set<SQLUSMALLINT> m_primaryKeyColumnIndexes;	///< If this set contains values during Open(), the flag TOF_DO_NOT_QUERY_PRIMARY_KEYS is activated implicitly. 
															///< The ColumnBuffers marked in this set will be used as primary key columns.
															///< Used if columns are not defined manually but queried, but the Database does not support SQLPrimaryKeys.

		ColumnBufferPtrMap	m_columnBuffers;	///< A map with ColumnBuffers, key is the column-Index (starting at 0). Either read from the db during Open(), or set manually using SetColumn().
		std::wstring		m_fieldsStatement;		///< Created during Open, after the columns have been bound. Contains the names of all columns separated by ',  ', to be used in a SELECT statement (avoid building it again and again)
		bool				m_manualColumns;		///< If true the table was initialized by passing the number of columns that will be defined later manually

		// Table information set during construction, that was used to find the matching TableInfo if none was passed
		std::wstring  m_initialTableName;		///< Table name set on initialization
		std::wstring	m_initialSchemaName;	///< Schema name set on initialization
		std::wstring	m_initialCatalogName;	///< Catalog name set on initialization
		std::wstring	m_initialTypeName;		///< Type name set on initialization

#ifdef EXODBCDEBUG
	public:
		size_t				GetTableId() const { return m_tableId; }
	private:
		size_t m_tableId; ///< Given by calling Database::RegisterTable() during Initialization.
#endif

	};  // Table

}	// namespace exodbc

#endif // TABLE_H
