///////////////////////////////////////////////////////////////////////////////
// Name:        dbtable.h
// Purpose:     Declaration of the wxDbTable class.
// Author:      Doug Card
// Modified by: George Tasker
//              Bart Jourquin
//              Mark Johnson
//				Elias Gerber, eg@zame.ch
// Created:     9.96
// RCS-ID:      $Id: dbtable.h 61872 2009-09-09 22:37:05Z VZ $
// Copyright:   (c) 1996 Remstar International, Inc.
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/*
// SYNOPSIS START
// SYNOPSIS STOP
*/

#pragma once
#ifndef DBTABLE_H
#define DBTABLE_H

#include "wxOdbc3.h"

#include "Database.h"

#include "GenericKey.h"

namespace exodbc
{
	// Consts
	// ------

	const int   wxDB_ROWID_LEN       = 24;  // 18 is the max, 24 is in case it gets larger
	const int   wxDB_DEFAULT_CURSOR  = 0;
	const bool  wxDB_QUERY_ONLY      = true;
	const bool  wxDB_DISABLE_VIEW    = true;

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


	class EXODBCAPI Table
	{
	public:
		// Public member functions
		Table(Database* pwxDb, const std::wstring& tblName, const UWORD numColumns,
			const std::wstring& qryTblName=emptyString, bool qryOnly = !wxDB_QUERY_ONLY,
			const std::wstring& tblPath=emptyString);

		virtual ~Table();

		bool            Open(bool checkPrivileges = false, bool checkTableExists = true);
		bool            CreateTable(bool attemptDrop = true);
		bool            DropTable();
		bool            CreateIndex(const std::wstring& indexName, bool unique, UWORD numIndexColumns, SIndexDefinition* pIndexDefs, bool attemptDrop = true);
		bool            DropIndex(const std::wstring& indexName);

		// Accessors

		// The member variables returned by these accessors are all
		// set when the wxDbTable instance is created and cannot be
		// changed, hence there is no corresponding SetXxxx function
		Database*			GetDb()              { return m_pDb; }
		const std::wstring& GetTableName()       { return m_tableName; }
		const std::wstring& GetQueryTableName()  { return m_queryTableName; }
		const std::wstring& GetTablePath()       { return m_tablePath; }

		UWORD           GetNumberOfColumns() { return m_numCols; }  // number of "defined" columns for this wxDbTable instance

		const std::wstring& GetFromClause()      { return m_from; }
		const std::wstring& GetOrderByClause()   { return m_orderBy; }
		const std::wstring& GetWhereClause()     { return m_where; }

		bool            IsQueryOnly()        { return m_queryOnly; }
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
		bool            Update();
		bool            Update(const std::wstring& pSqlStmt);
		bool            UpdateWhere(const std::wstring& pWhereClause);
		bool            Delete();
		bool            DeleteWhere(const std::wstring& pWhereClause);
		bool            DeleteMatching();
		virtual bool    Query(bool forUpdate = false, bool distinct = false);
		bool            QueryBySqlStmt(const std::wstring& pSqlStmt);
		bool            QueryMatching(bool forUpdate = false, bool distinct = false);
		bool            QueryOnKeyFields(bool forUpdate = false, bool distinct = false);
		bool            Refresh();
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

		void            BuildSelectStmt(std::wstring& pSqlStmt, int typeOfSelect, bool distinct);
		void            BuildSelectStmt(wchar_t* pSqlStmt, int typeOfSelect, bool distinct);

		void            BuildDeleteStmt(std::wstring& pSqlStmt, int typeOfDel, const std::wstring& pWhereClause = emptyString);
		void            BuildDeleteStmt(wchar_t* pSqlStmt, int typeOfDel, const std::wstring& pWhereClause = emptyString);

		void            BuildUpdateStmt(std::wstring& pSqlStmt, int typeOfUpdate, const std::wstring& pWhereClause = emptyString);
		void            BuildUpdateStmt(wchar_t* pSqlStmt, int typeOfUpdate, const std::wstring& pWhereClause = emptyString);

		void            BuildWhereClause(std::wstring& pWhereClause, int typeOfWhere, const std::wstring& qualTableName = emptyString, bool useLikeComparison = false);
		void            BuildWhereClause(wchar_t* pWhereClause, int typeOfWhere, const std::wstring &qualTableName = emptyString, bool useLikeComparison = false);

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

		ULONG           Count(const std::wstring& args = L"*");
		int             DB_STATUS() { return(m_pDb->DB_STATUS); }

		bool            IsColNull(UWORD colNumber) const;
		bool            SetColNull(UWORD colNumber, bool set = true);
		bool            SetColNull(const std::wstring& colName, bool set = true);

#ifdef EXODBCDEBUG
		ULONG           GetTableID() { return m_tableID; }
#endif

	private:
		ULONG       m_tableID;  // Used for debugging.  This can help to match up mismatched constructors/destructors

		// Private member variables
		UDWORD      m_cursorType;
		bool        m_insertable;

		// Private member functions
		bool        initialize(Database* pwxDb, const std::wstring& tblName, const UWORD numColumns,
								const std::wstring& qryTblName, bool qryOnly, const std::wstring& tblPath);
		void        cleanup();

		void        setCbValueForColumn(int columnIndex);
		bool        bindParams(bool forUpdate);  // called by the other 'bind' functions
		bool        bindInsertParams();
		bool        bindUpdateParams();

		bool        bindCols(HSTMT cursor);
		bool        getRec(UWORD fetchType);
		bool        execDelete(const std::wstring& pSqlStmt);
		bool        execUpdate(const std::wstring& pSqlStmt);
		bool        query(int queryType, bool forUpdate, bool distinct, const std::wstring& pSqlStmt = emptyString);

		// Where, Order By and From clauses
		std::wstring    m_where;               // Standard SQL where clause, minus the word WHERE
		std::wstring    m_orderBy;             // Standard SQL order by clause, minus the ORDER BY
		std::wstring    m_from;                // Allows for joins in a wxDbTable::Query().  Format: ",tbl,tbl..."

		// ODBC Handles
		HENV        m_henv;           // ODBC Environment handle
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

		// Table Inf.
		std::wstring    m_tablePath;                                 // needed for dBase tables
		std::wstring    m_tableName;                                 // Table name
		std::wstring    m_queryTableName;                            // Query Table Name
		UWORD       m_numCols;                               // # of columns in the table
		bool        m_queryOnly;                                 // Query Only, no inserts, updates or deletes

		// Column Definitions
		ColumnDefinition* m_colDefs;         // Array of wxDbColDef structures


		//TODO: Need to Document
		typedef     enum  { WX_ROW_MODE_QUERY , WX_ROW_MODE_INDIVIDUAL } rowmode_t;
		virtual     void         SetRowMode(const rowmode_t rowmode);
		//    virtual     wxVariant    GetColumn(const int colNumber) const ;
		//    virtual     void         SetColumn(const int colNumber, const wxVariant value);
		virtual     GenericKey   GetKey();
		virtual     void         SetKey(const GenericKey &key);

	private:
		HSTMT*		m_hstmtGridQuery;
		rowmode_t   m_rowmode;
		size_t      m_keysize;

		//      typedef enum {unmodified=0, UpdatePending, InsertPending } recStatus;

		//      recStatus  get_ModifiedStatus() { return m_recstatus; }

		//      void modify() {
		//          if (m_recstatus==unmodified)
		//              m_recstatus=UpdatePending;
		//      }
		//  protected:
		//      void insertify() {m_recstatus=InsertPending; }
		//      void unmodify() {m_recstatus=unmodified; }
		//      recStatus m_recstatus;
		//TODO: Need to Document
	};  // Table

}	// namespace exodbc

#endif // DBTABLE_H
