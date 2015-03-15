/*!
* \file Table.h
* \author Elias Gerber <eg@zame.ch>
* \date 25.07.2014
* \brief Header file for the Table class and its helpers.
* \copyright wxWindows Library Licence, Version 3.1
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
#include "ColumnBuffer.h"
#include "TablePrivileges.h"
#include "TablePrimaryKeys.h"

// Other headers
#include "boost/any.hpp"

// System headers

// Forward declarations
// --------------------
namespace exodbc
{
	class ColumnBuffer;
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
	* to query and modify the table.
	* Depending on which constructor is used, a table will query the database
	* about its column-definitions later automatically, or the column-definitions
	* must be set manually using SetColumn().
	* During Open() the table will "bind" a buffer to every defined column.
	* Columns bound to the table will be updated with the values of the current record
	* this table points to. Calling methods like Update(), Delete() or Insert() will
	* modify the table in the database using the current values.
	*
	* A table that queries the database about its columns will always bind all columns.
	* A table where columns are defined manually will only bind those columns defined.
	* This is handy if you have a large table and are only interested in the values of
	* a few columns.
	* Note if a name is something like columnIndex it will refer to the zero based index.
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
		* \enum		CharTrimOption
		* \brief	Define whether you want to trim string and wstring values returned from the Table functions GetColumnValue()
		* \detailed	Note that setting this flag will not modify the actual data-buffer, but only the string-values
		*			returned by the GetColumnValue() functions.
		*			Default is TRIM_NO
		*/
		enum CharTrimOption
		{
			TRIM_NO = 0x0L,		///< No Trimming of wstring or string (default)
			TRIM_LEFT = 0x1L,	///< Trim left
			TRIM_RIGHT = 0x2L	///< Trim right
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
		* \param	db		The Database this Table belongs to. Do not free the Database before
		*					you've freed the Table.
		* \param	tableInfo Definition of the table.
		* \param	openMode Define if the table shall be opened read-only or not
		*
		* \see		Open()
		* \throw	Exception If allocating statements fail.
		*/
		Table(const Database& db, const STableInfo& tableInfo, OpenMode openMode = READ_WRITE);


		/*!
		* \brief	Create a new Table-instance from the Database pDb by querying the database
		*			about a table with the given values for name, schema, catalog and type.
		*			The table will read its column-definitions from the database
		*			automatically during Open() and bind all columns.
		* \detailed During Open() the database will be queried for a table matching the given values.
		*			If any of the values is an empty string it is ignored when searching for the table
		*			in the database.
		*
		* \param	db		The Database this Table belongs to. Do not free the Database before
		*					you've freed the Table.
		* \param	tableName	Table name
		* \param	schemaName	Schema name
		* \param	catalogName	Catalog name
		* \param	tableType	Table type
		* \param	openMode Define if the table shall be opened read-only or not
		*
		* \see		Open()
		* \throw	Exception If allocating statements fail.
		*/
		Table(const Database& db, const std::wstring& tableName, const std::wstring& schemaName = L"", const std::wstring& catalogName = L"", const std::wstring& tableType = L"", const OpenMode openMode = READ_WRITE);


		/*!
		* \brief	Create a new Table-instance on which you will later set the ColumnInfo manually.
		*			During Open() only those columns you have set using SetColumn() will be bound.
		* \detailed During Open() the database will be queried for a table matching the given values.
		*			If any of the values is an empty string it is ignored when searching for the table
		*			in the database.
		*
		* \param	db		The Database this Table belongs to. Do not free the Database before
		*					you've freed the Table.
		* \param	numColumns The number of columns of the Table. Note: This must not be equal
		*			with the number of Columns you define later (but it must be larger or equal).
		*			It should be the number of columns the Table really has in the database.
		* \param	tableName Table name
		* \param	schemaName	Schema name
		* \param	catalogName	Catalog name
		* \param	tableType	Table type
		* \param	openMode Define if the table shall be opened read-only or not
		*
		* \see		Open()
		* \see		SetColumn()
		* \throw	Exception If allocating statements fail.
		*/
		Table(const Database& db, SQLSMALLINT numColumns, const std::wstring& tableName, const std::wstring& schemaName = L"", const std::wstring& catalogName = L"", const std::wstring& tableType = L"", OpenMode openMode = READ_WRITE);


		/*!
		* \brief	Create a new Table-instance on which you will later set the ColumnInfo manually.
		*			During Open() only those columns you have set using SetColumn() will be bound.
		* \detailed Note that when this constructor is used, the internal STableInfo object is not
		*			queried from the database, but the passed tableInfo is used for all later operations.
		*			This is handy if you've located the detailed table-information already from the Database
		*			using its Database::FindTables() function and want to avoid that is operation is
		*			executed again during Open().
		*
		* \param	db		The Database this Table belongs to. Do not free the Database before
		*					you've freed the Table.
		* \param	numColumns The number of columns of the Table. Note: This must not be equal
		*			with the number of Columns you define later (but it must be larger or equal).
		*			It should be the number of columns the Table really has in the database.
		* \param	tableInfo Definition of the table.
		* \param	openMode Define if the table shall be opened read-only or not
		*
		* \see		Open()
		* \see		SetColumn()
		* \throw	Exception If allocating statements fail.
		*/
		Table(const Database& db, SQLSMALLINT numColumns, const STableInfo& tableInfo, OpenMode openMode = READ_WRITE);


		virtual ~Table();


		/*!
		* \brief	Allocate the statement handles required by this Table.
		* \detailed Allocates the statements using the connection handle from the passed Database.
		*			Do not free the Database before freeing this Table.
		* \param	db Database this Table belongs to.
		* \see		HasStatements()
		*
		* \throw	Exception If any of the handles to be allocated is not null currently.
		*/
		void AllocateStatements(const Database& db);


		/*!
		* \brief	Test if the required statements (depending on OpenMode) are allocated.
		* \return	True if all statements are allocated
		* \see		AllocateStatements()
		*/
		bool HasStatements() const throw();


		/*!
		* \brief	Opens the Table and either binds the already defined columns or queries the database
		*			about the columns of this table.
		* \detailed If no STableInfo object has been passed during construction the database is first
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
		* \param	checkPrivileges If true, the database will be queried checking if the current user
		*			is allowed to do the required operations like Select, Update, Insert or Delete,
		*			depending on the OpenMode value passed during construction.
		* \param	checkTableExists If true, the database will always be queried if a table matching the
		*			passed definition during constructions actually exists in the database. Setting this
		*			value to false makes only sense	if you've passed a STableInfo, as else the Table
		*			is required to query the database anyway (and fail if not found).
		* \see		IsOpen()
		* \see		Close()
		* \see		SetColumn()
		* \throw	Exception If already open, table is not found, columns fail to bind..
		*/
		void		Open(const Database& db, bool checkPrivileges = false, bool checkTableExists = true);


		/*!
		* \brief	Close the table.
		* \detailed	Unbinds and deletes all ColumnBuffers bound to this table. The information about
		*			this table in the STableInfo queried during Open() is kept for future use.
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
		* \brief	Get the OpenMode of this Table
		* \return	True if this table was created using READ_ONLY
		*/
		bool		IsQueryOnly() const throw()  { return m_openMode == READ_ONLY; }


		/*!
		* \brief	Set the AutoBindingMode. Must be called before Open().
		* \detailed	This will set the AutoBindingMode globally for this table.
		*			It can still be overridden for specific columns by defining
		*			it for that column.
		* \return	AutoBindingMode
		* \see		GetAutoBindingMode()
		* \throw	Exception If already open.
		*/
		void		SetAutoBindingMode(AutoBindingMode mode);


		/*!
		* \brief	Get the AutoBindingMode of this table.
		* \return	AutoBindingMode
		* \see		SetAutoBindingMode()
		*/
		AutoBindingMode	GetAutoBindingMode() const throw() { return m_autoBindingMode; };


		/*!
		* \brief	Set a CharTrimOption. Setting to TRIM_NO will clear all other flags.
		*/
		void		SetCharTrimOption(CharTrimOption option) throw() { option == TRIM_NO ? m_charTrimFlags = TRIM_NO : m_charTrimFlags |= option; };


		/*!
		* \brief	Test if a CharTrimOption is set.
		*/
		bool		TestCharTrimOption(CharTrimOption option) const throw() { return (m_charTrimFlags & option) == option;  };


		/*!
		* \brief	Check if the Table-Information is set on this Table.
		* \detailed	Returns true if the internal member of the STableInfo contains a value either
		*			set during Construction or fetched from the Database during Open().
		* \see		GetTableInfo()
		* \return	Returns true if this table has a STableInfo set that can be fetched using GetTableInfo()
		*/
		bool		HaveTableInfo() const throw() { return m_haveTableInfo; }


		/*!
		* \brief	Return the Table information of this Table.
		* \detailed	Returns the STableInfo of this table, if one has been set either during construction
		*			or one was read during Open().
		* \see		HaveTableInfo()
		* \throw	Exception if no table info is available.
		*/
		STableInfo	GetTableInfo() const;


		/*!
		* \brief	Counts how many rows would be selected in this table by the passed WHERE clause.
		* \detailed	If whereStatement is empty, no WHERE clause is added
		* \param	whereStatement Do not include 'WHERE' in the passed where clause
		* \return	count The result of a 'SELECT COUNT(*) WHERE whereStatement' on the current table
		* \throw	Exception If failed.
		*/
		SQLUBIGINT		Count(const std::wstring& whereStatement);


		/*!
		* \brief	Executes a 'SELECT col1, col2, .., colN' for the Table using the passed WHERE clause.
		* \detailed	The SELECT-Query is built using the column information available to this Table.
		*			It does not use the '*'. Only bound ColumnBuffers names are included in the statement.
		*			If successful, a Select-Query is open. You can iterate the records
		*			using SelectNext() to access the values of the records.
		*			The cursor is positioned before the first records, so you must call
		*			SelectNext() to access the first record.
		*			If whereStatement is empty, no WHERE clause is added.
		*			If a select statement is open, the statement is closed first.
		* \param	whereStatement Do not include 'WHERE' in the passed where clause
		* \see		SelectNext()
		* \see		SelectClose();
		* \throw	Exception If failed.
		*/
		void		Select(const std::wstring& whereStatement = L"");


		/*!
		* \brief	Executes the passed SQL statement on the open Table.
		* \detailed	Query by passing the complete SQL statement.
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
		* \detailed	If successful, the ColumnBuffer(s) bound to this table will contain 
		*			the field-values of the currently selected record.
		*			Fails if no Select-Query is open.
		* \see		SelectNext()
		* \return	True if next record has been fetched, false if no more records exist.
		*/
		bool		SelectNext();


		/*!
		* \brief	Closes an eventually open Select-Query.
		* \detailed	This function does not fail if no select statement was open.
		* \see		Select()
		* \throw	Exception
		*/
		void		SelectClose();


		/*!
		* \brief	A wrapper to SQLColAttributes, to fetch attributes of the columns of an open result set.
		* \detailed	Can only be called if a Select() is open. Only for numeric attributes.
		* \param	columnIndex zero based index of the column available in the result set.
		* \see		http://msdn.microsoft.com/en-us/library/ms713558%28v=vs.85%29.aspx
		* \return	Attribute value.
		* \throw	Exception
		*/
		SQLINTEGER	SelectColumnAttribute(SQLSMALLINT columnIndex, ColumnAttribute attr);


		/*!
		* \brief	Check if a Select() Query is open.
		* \return	True if a Select() Query is open and rows can be iterated using SelectNext()
		*/
		bool		IsSelectOpen() const { return m_selectQueryOpen; };


		/*!
		* \brief	Inserts the current values into the database as a new row.
		* \detailed	The values in the ColumnBuffer currently bound will be inserted
		*			into the database.
		*			An prepared INSERT statement is used to insert the values identified
		*			by the bound ColumnBuffers that have the flag ColumnFlags::CF_INSERT set.
		*			Fails if the table has not been opened using READ_WRITE.
		*			This will not commit the transaction.
		* \see		Database::CommitTrans()
		* \throw	Exception on failure.
		*/
		void		Insert();


		/*!
		* \brief	Deletes the row identified by the values of the bound primary key columns.
		* \detailed	A prepared DELETE statement is used to delete the row that matches all
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
		* \detailed	A DELETE statement for this Table is created, using the passed
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
		void		Delete(const std::wstring& where, bool failOnNoData = true);


		/*!
		* \brief	Updates the row identified by the values of the bound primary key columns with
		*			the values in bound ColumnBuffers, if the ColumnBuffer has the ColumnFlags::CF_UPDATE
		*			set.
		* \detailed	A prepared UPDATE statement is used to update the row(s) that matches all
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
		* \detailed	A prepared UPDATE statement is used to update the row(s) that match the passed where statement.
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
		* \brief	Access the current value of columnIndex as SQLSMALLINT.
		* \detailed	Casts the value if casting is possible without loosing data.
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] smallInt Reference to variable to copy value to.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		void		GetColumnValue(SQLSMALLINT columnIndex, SQLSMALLINT& smallInt) const;


		/*!
		* \brief	Access the current value of columnIndex as SQLINTEGER.
		* \detailed	Casts the value if casting is possible without loosing data.
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] i Reference to variable to copy value to.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		void		GetColumnValue(SQLSMALLINT columnIndex, SQLINTEGER& i) const;


		/*!
		* \brief	Access the current value of columnIndex as SQLBIGINT.
		* \detailed	Casts the value if casting is possible without loosing data.
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] bigInt Reference to variable to copy value to.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		void		GetColumnValue(SQLSMALLINT columnIndex, SQLBIGINT& bigInt) const;


		/*!
		* \brief	Access the current value of columnIndex as std::wstring.
		* \detailed	Casts the value if casting is possible without loosing data.
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] str Reference to variable to copy value to.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		void		GetColumnValue(SQLSMALLINT columnIndex, std::wstring& str) const;


		/*!
		* \brief	Access the current value of columnIndex as std::string.
		* \detailed	Casts the value if casting is possible without loosing data.
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] str Reference to variable to copy value to.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		void		GetColumnValue(SQLSMALLINT columnIndex, std::string& str) const;


		/*!
		* \brief	Access the current value of columnIndex as SQLDOUBLE.
		* \detailed	Casts the value if casting is possible without loosing data.
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] d Reference to variable to copy value to.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		void		GetColumnValue(SQLSMALLINT columnIndex, SQLDOUBLE& d) const;


		/*!
		* \brief	Access the current value of columnIndex as SQL_DATE_STRUCT.
		* \detailed	Casts the value if casting is possible without loosing data.
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] date Reference to variable to copy value to.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		void		GetColumnValue(SQLSMALLINT columnIndex, SQL_DATE_STRUCT& date) const;


		/*!
		* \brief	Access the current value of columnIndex as SQL_TIME_STRUCT.
		* \detailed	Casts the value if casting is possible without loosing data.
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] time Reference to variable to copy value to.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		void		GetColumnValue(SQLSMALLINT columnIndex, SQL_TIME_STRUCT& time) const;
		
		
		/*!
		* \brief	Access the current value of columnIndex as SQL_TIMESTAMP_STRUCT.
		* \detailed	Casts the value if casting is possible without loosing data.
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] timestamp Reference to variable to copy value to.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		void		GetColumnValue(SQLSMALLINT columnIndex, SQL_TIMESTAMP_STRUCT& timestamp) const;


#if HAVE_MSODBCSQL_H
		/*!
		* \brief	Access the current value of columnIndex as SQL_SS_TIME2_STRUCT.
		* \detailed	Casts the value if casting is possible without loosing data.
		*			This function is only available if HAVE_MSODBCSQL_H is defined to 1
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] time2 Reference to variable to copy value to.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		void		GetColumnValue(SQLSMALLINT columnIndex, SQL_SS_TIME2_STRUCT& time2) const;
#endif


		/*!
		* \brief	Access the current value of columnIndex as SQL_NUMERIC_STRUCT.
		* \detailed	Casts the value if casting is possible without loosing data.
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] numeric Reference to variable to copy value to.
		* \throw Exception If columnIndex is invalid, or the column value is NULL, or casting fails
		*/
		void		GetColumnValue(SQLSMALLINT columnIndex, SQL_NUMERIC_STRUCT& numeric) const;


		/*!
		* \brief	Check if the current value of a column is NULL.
		* \detailed	Queries the length-indicator field of the ColumnBuffer to determine if
		*			a column is NULL.
		* \param	columnIndex Zero based index of a bound column.
		* \return	True if current value of column is NULL.
		*/
		bool		IsColumnNull(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Access the buffer of a bound column.
		* \detailed Points passed pointer to the buffer used to transfer data for the given column.
		*			
		* \param	columnIndex Zero based index of a bound column.
		* \param [in,out] pBuffer Reference to point to column buffer.
		* \param [in,out] bufferSize Reference to size of buffer, will be set on success to match
		*			length of buffer pointed to by pBuffer.
		* \param [in,out] lengthIndicator Will be set to the length Indicator value of the ColumnBuffer.
		* throw	Exception If the column value is NULL, or columnIndex is inavlid
		*/
		void		GetBuffer(SQLSMALLINT columnIndex, const SQLCHAR*& pBuffer, SQLINTEGER& bufferSize, SQLINTEGER& lengthIndicator) const;


		/*!
		* \brief	Return a defined ColumnBuffer or NULL.
		* \detailed	Searches the internal map of ColumnBuffers for a ColumnBuffer with
		*			the given columnIndex (zero based).
		* \return	ColumnBuffer.
		* \throw	Exception
		*/
		ColumnBuffer* GetColumnBuffer(SQLSMALLINT columnIndex) const;


		/*!
		* \brief	Returns the number of columns this table has.
		* \detailed	If columns were set manually on the table, this is the value that has been 
		*			passed as the total number of columns of the table. It must not be equal
		*			with the number of columns bound.
		*			If columns were created automatically, this is the total of columns reported
		*			by the database for this table and should be equal with the number of
		*			columns bound.
		* \return	Numbers of column this table has.
		*/
		SQLSMALLINT	GetNumberOfColumns() const { return m_numCols; };


		/*!
		* \brief	Define a column manually. The column is bound during Open().
		* \detailed	Pass a buffer and a description of the buffer that shall be bound to a
		*			table column once Open() is called.
		* \param	columnIndex Zero based index of a bound column.
		* \param	queryName Name of the column matching columnIndex. This name will be used
		*			for queries.
		* \param	pBuffer BufferPtrVariant that contains an allocated buffer of the type given
		*			by sqlCType.
		* \param	sqlCType SQL_C_TYPE of the buffer hold by pBuffer, like SQL_C_CHAR, SQL_C_SINTEGER, etc.
		*			This information will be forwarded to the ODBC driver while binding the column.
		*			The driver will try to convert the column-value to the given type.
		* \param	column columnSize The number of digits of a decimal value (including the fractional part).
		*			This is only used if the sqlCType is SQL_C_NUMERIC, to set SQL_DESC_PRECISION.
		* \param	decimalDigits The number of digits of the fractional part of a decimal value.
		*			This is only used if the sqlCType is SQL_C_NUMERIC, to set SQL_DESC_SCALE.
		* \param	bufferSize The size of the buffer pointed to by pBuffer.
		* \param	flags Define if a column shall be included in write-operations, is part of primary-key, etc.
		*/
		void		SetColumn(SQLUSMALLINT columnIndex, const std::wstring& queryName, BufferPtrVariant pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags = CF_SELECT, SQLINTEGER columnSize = -1, SQLSMALLINT decimalDigits = -1);


		// Private stuff
		// -------------
	private:

		/*!
		* \brief	Initializes the member-vars to null or defaults.
		* \throw	Exception
		*/
		void        Initialize();


		/*!
		* \brief	Allocates a new Statement-handle using the Database of this Table.
		* \return	New Statement handle.
		* \throw	Exception If allocating handle fails.
		*/
		SQLHSTMT		AllocateStatement(const Database& db) const;


		/*!
		* \brief	Frees the passed Statement handle.
		* \detailed If the passed statement can be freed successfully, SQL_NULL_HSTMT is returned.
		*			If the passed statement cannot be freed successfully because SQL_INVALID_HANDLE 
		*			or SQL_ERROR is returned, an Exception is thrown.
		* \return	SQL_NULL_HSTMT is statement was freed successfully.
		* \throw	Exception If freeing statement fails.
		*/
		SQLHSTMT	FreeStatement(SQLHSTMT stmt);

		
		/*!
		* \brief	Frees the statement handles allocated during AllocateStatements().
		* \throw	SqlResultException if freeing one of the handles fail.
		*/
		void		FreeStatements();


		/*!
		* \brief	Iterates the bound columns and returns the field part of a statement.
		* \detailed	Queries each bound column for its SqlName.
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


		// ODBC Handles
		SQLHSTMT		m_hStmtSelect;	///< Statement-handle used to do SELECTs. Columns are bound.
		SQLHSTMT		m_hStmtCount;	///< Statement-handle used to do COUNTs. Columns are not bound.
		SQLHSTMT		m_hStmtInsert;	///< Statement-handle used to do INSERTs. Columns are bound, a prepared statement using column-markers is created.
		SQLHSTMT		m_hStmtDelete;	///< Statement-handle used to do DELETs. Primary key columns are bound, a prepared statement using column-markers is created.
		SQLHSTMT		m_hStmtUpdate;	///< Statement-handle used to do UPDATEs. Primary key columns are bound, a prepared statement using column-markers is created.
		SQLHSTMT		m_hStmtDeleteWhere;	///< Statement-handle to do DELETEs using a passed WHERE clause.
		SQLHSTMT		m_hStmtUpdateWhere;	///< Statement-handle to do UPDATEs using a passed WHERE clause.

		bool		m_selectQueryOpen;	///< Set to True once a successful Select(), set to false on SelectClose()

		// Table Information
		bool				m_haveTableInfo;		///< True if m_tableInfo has been set
		STableInfo			m_tableInfo;			///< TableInfo fetched from the db or set through constructor
		const OpenMode		m_openMode;				///< Read-only or writable
		AutoBindingMode		m_autoBindingMode;		///< Store the auto-binding of this table. TODO: Can still be overridden by specifying it on a column
		bool				m_isOpen;				///< Set to true after Open has been called
		unsigned int		m_charTrimFlags;		///< Bitmask for the CharTrimOption Flags
		TablePrivileges		m_tablePrivileges;		///< Table Privileges read during open if checkPermission was set.
		TablePrimaryKeys	m_tablePrimaryKeys;		///< Table Primary Keys read during Open if table was opened READ_WRITE.

		// Column information
		ColumnBufferPtrMap	m_columnBuffers;	///< A map with ColumnBuffers, key is the column-Index (starting at 0). Either read from the db during Open(), or set manually using SetColumn().
		std::wstring		m_fieldsStatement;		///< Created during Open, after the columns have been bound. Contains the names of all columns separated by ',  ', to be used in a SELECT statement (avoid building it again and again)
		const bool			m_manualColumns;		///< If true the table was created by passing the number of columns that will be defined later manually
		SQLSMALLINT			m_numCols;				//< # of columns in the table. Either set from user during constructor, or read from the database

		// Table information set during construction, that was used to find the matching STableInfo if none was passed
		// Note: We make them public, as they are all const
	public:
		const std::wstring  m_initialTableName;		///< Table name set on construction
		const std::wstring	m_initialSchemaName;	///< Schema name set on construction
		const std::wstring	m_initialCatalogName;	///< Catalog name set on construction
		const std::wstring	m_initialTypeName;		///< Type name set on construction

#ifdef EXODBCDEBUG
	public:
		size_t				GetTableId() const { return m_tableId; }
	private:
		size_t m_tableId; ///< Given by calling Database::RegisterTable() during Initialization.
#endif

	};  // Table


	// OLD STUFF we need to think about re-adding it
	// =============================================

	// Consts
	// ------

	// const int   wxDB_DEFAULT_CURSOR = 0;

	//// Used to indicate end of a variable length list of
	//// column numbers passed to member functions
	//const int   wxDB_NO_MORE_COLUMN_NUMBERS = -1;

	// Structs
	// -------
	//
	//struct EXODBCAPI SColumnDataPtr
	//{
	//public:
	//	void*		PtrDataObj;
	//	int			SzDataObj;
	//	SWORD		SqlCtype;
	//};  // SColumnDataPtr


	//// This structure is used when creating secondary indexes.
	//struct EXODBCAPI SIndexDefinition
	//{
	//public:
	//	wchar_t		ColName[DB_MAX_COLUMN_NAME_LEN + 1];
	//	bool		Ascending;
	//};  // SIndexDefinition

	// Classes
	// -------

	//class EXODBCAPI ColDef
	//{
	//public:
	//	ColDef(bool allocateBuffer);
	//	// boost::any somehow:
	//	//SQL_TIMESTAMP_STRUCT ts;
	//	//ts.fraction = 26;
	//	//ts.hour = 7;
	//	//ts.year = 2017;
	//	//boost::any a1 = ts;
	//	//size_t l1 = sizeof(ts);
	//	//size_t l2 = sizeof(a1);
	//	//SQL_TIMESTAMP_STRUCT* t2 = &(boost::any_cast<SQL_TIMESTAMP_STRUCT>(a1));

	//private:
	//	boost::any m_value;
	//	SColumnInfo columnInfo;
	//};

	//// The following class is used to define a column of a table.
	//// The wxDbTable constructor will dynamically allocate as many of
	//// these as there are columns in the table.  The class derived
	//// from wxDbTable must initialize these column definitions in it's
	//// constructor.  These column definitions provide inf. to the
	//// wxDbTable class which allows it to create a table in the data
	//// source, exchange data between the data source and the C++
	//// object, and so on.
	//class EXODBCAPI ColumnDefinition
	//{
	//public:
	//	ColumnDefinition();

	//	bool    Initialize();

	//	wchar_t	m_colName[DB_MAX_COLUMN_NAME_LEN + 1];  // Column Name
	//	int		m_dbDataType;                         // Logical Data Type; e.g. DB_DATA_TYPE_INTEGER
	//	SWORD	m_sqlCtype;                           // C data type; e.g. SQL_C_LONG
	//	void*	m_ptrDataObj;                         // Address of the data object
	//	int		m_szDataObj;                          // Size, in bytes, of the data object
	//	bool	m_keyField;                           // true if this column is part of the PRIMARY KEY to the table; Date fields should NOT be KeyFields.
	//	bool	m_updateable;                         // Specifies whether this column is updateable
	//	bool	m_insertAllowed;                      // Specifies whether this column should be included in an INSERT statement
	//	bool	m_derivedCol;                         // Specifies whether this column is a derived value
	//	SQLLEN	m_cbValue;                            // Internal use only!!!
	//	bool	m_null;                               // NOT FULLY IMPLEMENTED - Allows NULL values in Inserts and Updates
	//};  // ColumnDefinition

	// ALL OF THIS DOWN HERE WAS PART OF wxDbTable
	// ===========================================

	// public:
		//
		//bool            CreateTable(bool attemptDrop = true);
		//		bool            DropTable();
		//		bool            CreateIndex(const std::wstring& indexName, bool unique, UWORD numIndexColumns, SIndexDefinition* pIndexDefs, bool attemptDrop = true);
		//		bool            DropIndex(const std::wstring& indexName);
		//const std::wstring& GetFromClause()      { return m_from; }
		//const std::wstring& GetOrderByClause()   { return m_orderBy; }
		//const std::wstring& GetWhereClause()     { return m_where; }

		//void            SetFromClause(const std::wstring& From) { m_from = From; }
		//void            SetOrderByClause(const std::wstring& OrderBy) { m_orderBy = OrderBy; }
		//		bool            SetOrderByColNums(UWORD first, ...);
		//void            SetWhereClause(const std::wstring& Where) { m_where = Where; }
		//void            From(const std::wstring& From) { m_from = From; }
		//void            OrderBy(const std::wstring& OrderBy) { m_orderBy = OrderBy; }
		//void            Where(const std::wstring& Where) { m_where = Where; }
		//const std::wstring& Where()   { return m_where; }
		//const std::wstring& OrderBy() { return m_orderBy; }
		//const std::wstring& From()    { return m_from; }

		//int             Insert();
		//bool            Update();
		//bool            Update(const std::wstring& pSqlStmt);
		//bool            UpdateWhere(const std::wstring& pWhereClause);
		//bool            Delete();
		//bool            DeleteWhere(const std::wstring& pWhereClause);
		//bool            DeleteMatching();
		//		virtual bool    Query(bool forUpdate = false, bool distinct = false);
		//		bool            QueryBySqlStmt(const std::wstring& pSqlStmt);
		//		bool            QueryMatching(bool forUpdate = false, bool distinct = false);
		//		bool            QueryOnKeyFields(bool forUpdate = false, bool distinct = false);
		//bool            Refresh();
		//		bool            GetNext()		{ return(getRec(SQL_FETCH_NEXT)); }
		//		bool            operator++(int) { return(getRec(SQL_FETCH_NEXT)); }

		/***** These four functions only work with wxDb instances that are defined  *****
		***** as not being FwdOnlyCursors                                          *****/
		//		bool            GetPrev();
		//		bool            operator--(int);
		//		bool            GetFirst();
		//		bool            GetLast();

		//bool            IsCursorClosedOnCommit();
		//UWORD           GetRowNum();

		//void            BuildSelectStmt(std::wstring& pSqlStmt, int typeOfSelect, bool distinct);
		//void            BuildSelectStmt(wchar_t* pSqlStmt, int typeOfSelect, bool distinct);

		//		void            BuildDeleteStmt(std::wstring& pSqlStmt, int typeOfDel, const std::wstring& pWhereClause = emptyString);
		//void            BuildDeleteStmt(wchar_t* pSqlStmt, int typeOfDel, const std::wstring& pWhereClause = emptyString);

		//void            BuildUpdateStmt(std::wstring& pSqlStmt, int typeOfUpdate, const std::wstring& pWhereClause = emptyString);
		//void            BuildUpdateStmt(wchar_t* pSqlStmt, int typeOfUpdate, const std::wstring& pWhereClause = emptyString);

		//void            BuildWhereClause(std::wstring& pWhereClause, int typeOfWhere, const std::wstring& qualTableName = emptyString, bool useLikeComparison = false);
		//void            BuildWhereClause(wchar_t* pWhereClause, int typeOfWhere, const std::wstring &qualTableName = emptyString, bool useLikeComparison = false);

		//bool            CanSelectForUpdate();
		//bool            CanUpdateByROWID();
		//		void            ClearMemberVar(UWORD colNumber, bool setToNull = false);
		//		void            ClearMemberVars(bool setToNull = false);
		//bool            SetQueryTimeout(UDWORD nSeconds);

		//ColumnDefinition* GetColDefs() { return m_colDefs; }
		//bool            SetColDefs(UWORD index, const std::wstring& fieldName, int dataType, void* pData, SWORD cType, int size, bool keyField = false,
		//	bool updateable = true, bool insertAllowed = true, bool derivedColumn = false);
		//SColumnDataPtr* SetColDefs(ColumnInfo* colInfs, UWORD numCols);

		//bool            CloseCursor(SQLHSTMT cursor);
		//bool            DeleteCursor(SQLHSTMT* hstmtDel);
		//void            SetCursor(SQLHSTMT* hstmtActivate = (void **)wxDB_DEFAULT_CURSOR);
		//HSTMT           GetCursor() { return(m_hstmt); }
		//HSTMT*			GetNewCursor(bool setCursor = false, bool bindColumns = true);

		//ULONG           Count(const std::wstring& args = L"*");

		//		bool            IsColNull(UWORD colNumber) const;
		//		bool            SetColNull(UWORD colNumber, bool set = true);
		//		bool            SetColNull(const std::wstring& colName, bool set = true);

	//private:
	//
		//UDWORD      m_cursorType;
		//		void        setCbValueForColumn(int columnIndex);
		//		bool        bindParams(bool forUpdate);  // called by the other 'bind' functions
		//bool        bindInsertParams();
		//bool        bindUpdateParams();

		//		bool        bindCols(SQLHSTMT cursor);
		//		bool        getRec(UWORD fetchType);
		//bool        execDelete(const std::wstring& pSqlStmt);
		//		bool        execUpdate(const std::wstring& pSqlStmt);
		//		bool        query(int queryType, bool forUpdate, bool distinct, const std::wstring& pSqlStmt = emptyString);

		// Where, Order By and From clauses
		//std::wstring    m_where;               // Standard SQL where clause, minus the word WHERE
		//std::wstring    m_orderBy;             // Standard SQL order by clause, minus the ORDER BY
		//std::wstring    m_from;                // Allows for joins in a wxDbTable::Query().  Format: ",tbl,tbl..."

		//// Old handles below, should all be replaced by new implementations
		////		HENV        m_henv;           // ODBC Environment handle
		//HSTMT       m_hstmt;          // ODBC Statement handle
		//HSTMT*		m_hstmtDefault;   // Default cursor
		//HDBC        m_hdbc;           // ODBC DB Connection handle
		//HSTMT       m_hstmtInsert;    // ODBC Statement handle used specifically for inserts
		//HSTMT       m_hstmtDelete;    // ODBC Statement handle used specifically for deletes
		//HSTMT       m_hstmtUpdate;    // ODBC Statement handle used specifically for updates
		//HSTMT       m_hstmtInternal;  // ODBC Statement handle used internally only
		//		HSTMT*		m_hstmtCount;     // ODBC Statement handle used by Count() function (No binding of columns)

		// Flags
		//bool        m_selectForUpdate;

		// Column Definitions
		//		ColumnDefinition* m_colDefs;         // Array of wxDbColDef structures

}	// namespace exodbc

#endif // DBTABLE_H
