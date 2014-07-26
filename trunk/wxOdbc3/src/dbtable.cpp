///////////////////////////////////////////////////////////////////////////////
// Name:        src/common/dbtable.cpp
// Purpose:     Implementation of the wxDbTable class.
// Author:      Doug Card
// Modified by: George Tasker
//              Bart Jourquin
//              Mark Johnson
// Created:     9.96
// RCS-ID:      $Id: dbtable.cpp 48685 2007-09-14 19:02:28Z VZ $
// Copyright:   (c) 1996 Remstar International, Inc.
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

#include "dbtable.h"
#include "Helpers.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"
#include "boost/log/trivial.hpp"
#include <vector>

namespace exodbc
{


ULONG lastTableID = 0;


#ifdef __WXDEBUG__
//    #include "wx/thread.h"
//    wxList TablesInUse;
	std::vector<wxTablesInUse*> TablesInUse;
#if wxUSE_THREADS
    wxCriticalSection csTablesInUse;
#endif // wxUSE_THREADS
#endif


void csstrncpyt(wchar_t *target, const wchar_t *source, int n)
{
    while ( (*target++ = *source++) != '\0' && --n != 0 )
        ;

    *target = '\0';
}



/********** wxDbColDef::wxDbColDef() Constructor **********/
ColumnDefinition::ColumnDefinition()
{
    Initialize();
}  // Constructor


bool ColumnDefinition::Initialize()
{
    m_ColName[0]      = 0;
    m_DbDataType      = DB_DATA_TYPE_INTEGER;
    m_SqlCtype        = SQL_C_LONG;
    m_PtrDataObj      = NULL;
    m_szDataObj       = 0;
    m_keyField        = false;
    m_updateable      = false;
    m_insertAllowed   = false;
    m_derivedCol      = false;
    m_cbValue         = 0;
    m_null            = false;

    return true;
}  // wxDbColDef::Initialize()


/********** wxDbTable::wxDbTable() Constructor **********/
wxDbTable::wxDbTable(Database *pwxDb, const std::wstring &tblName, const UWORD numColumns,
                    const std::wstring &qryTblName, bool qryOnly, const std::wstring &tblPath)
{
    if (!initialize(pwxDb, tblName, numColumns, qryTblName, qryOnly, tblPath))
        cleanup();
}  // wxDbTable::wxDbTable()



/********** wxDbTable::~wxDbTable() **********/
wxDbTable::~wxDbTable()
{
    this->cleanup();
}  // wxDbTable::~wxDbTable()


bool wxDbTable::initialize(Database *pwxDb, const std::wstring &tblName, const UWORD numColumns,
                    const std::wstring &qryTblName, bool qryOnly, const std::wstring &tblPath)
{
    // Initializing member variables
    pDb                 = pwxDb;                    // Pointer to the wxDb object
    henv                = 0;
    hdbc                = 0;
    hstmt               = 0;
    m_hstmtGridQuery               = 0;
    hstmtDefault        = 0;                        // Initialized below
    hstmtCount          = 0;                        // Initialized first time it is needed
    hstmtInsert         = 0;
    hstmtDelete         = 0;
    hstmtUpdate         = 0;
    hstmtInternal       = 0;
    colDefs             = 0;
    tableID             = 0;
    m_numCols           = numColumns;               // Number of columns in the table
    where.empty();                                  // Where clause
    orderBy.empty();                                // Order By clause
    from.empty();                                   // From clause
    selectForUpdate     = false;                    // SELECT ... FOR UPDATE; Indicates whether to include the FOR UPDATE phrase
    queryOnly           = qryOnly;
    insertable          = true;
    tablePath.empty();
    tableName.empty();
    queryTableName.empty();

    exASSERT(tblName.length());
    exASSERT(pDb);

    if (!pDb)
        return false;

    tableName = tblName;                        // Table Name
    if ((pDb->Dbms() == dbmsORACLE) ||
        (pDb->Dbms() == dbmsFIREBIRD) ||
        (pDb->Dbms() == dbmsINTERBASE))
        boost::algorithm::to_upper(tableName);

    if (tblPath.length())
        tablePath = tblPath;                    // Table Path - used for dBase files
    else
        tablePath.empty();

    if (qryTblName.length())                    // Name of the table/view to query
        queryTableName = qryTblName;
    else
        queryTableName = tblName;

    if ((pDb->Dbms() == dbmsORACLE) ||
        (pDb->Dbms() == dbmsFIREBIRD) ||
        (pDb->Dbms() == dbmsINTERBASE))
        boost::algorithm::to_upper(queryTableName);

    pDb->incrementTableCount();

    std::wstring s;
    tableID = ++lastTableID;
	s = (boost::wformat(L"wxDbTable constructor (%-20s) tableID:[%6lu] pDb:[%p]") %tblName %tableID % static_cast<void*>(pDb)).str();

#ifdef __WXDEBUG__
    wxTablesInUse *tableInUse;
    tableInUse            = new wxTablesInUse();
    tableInUse->tableName = tblName.c_str();
    tableInUse->tableID   = tableID;
    tableInUse->pDb       = pDb;
    {
#if wxUSE_THREADS
        wxCriticalSectionLocker lock(csTablesInUse);
#endif // wxUSE_THREADS
        TablesInUse.push_back(tableInUse);
    }
#endif

    pDb->WriteSqlLog(s);

    // Grab the HENV and HDBC from the wxDb object
    henv = pDb->GetHENV();
    hdbc = pDb->GetHDBC();

    // Allocate space for column definitions
    if (m_numCols)
        colDefs = new ColumnDefinition[m_numCols];  // Points to the first column definition

    // Allocate statement handles for the table
    if (!queryOnly)
    {
        // Allocate a separate statement handle for performing inserts
        if (SQLAllocStmt(hdbc, &hstmtInsert) != SQL_SUCCESS)
            pDb->DispAllErrors(henv, hdbc);
        // Allocate a separate statement handle for performing deletes
        if (SQLAllocStmt(hdbc, &hstmtDelete) != SQL_SUCCESS)
            pDb->DispAllErrors(henv, hdbc);
        // Allocate a separate statement handle for performing updates
        if (SQLAllocStmt(hdbc, &hstmtUpdate) != SQL_SUCCESS)
            pDb->DispAllErrors(henv, hdbc);
    }
    // Allocate a separate statement handle for internal use
    if (SQLAllocStmt(hdbc, &hstmtInternal) != SQL_SUCCESS)
        pDb->DispAllErrors(henv, hdbc);

    // Set the cursor type for the statement handles
    cursorType = SQL_CURSOR_STATIC;

    if (SQLSetStmtOption(hstmtInternal, SQL_CURSOR_TYPE, cursorType) != SQL_SUCCESS)
    {
        // Check to see if cursor type is supported
        pDb->GetNextError(henv, hdbc, hstmtInternal);
        if (! wcscmp(pDb->sqlState, L"01S02"))  // Option Value Changed
        {
            // Datasource does not support static cursors.  Driver
            // will substitute a cursor type.  Call SQLGetStmtOption()
            // to determine which cursor type was selected.
            if (SQLGetStmtOption(hstmtInternal, SQL_CURSOR_TYPE, &cursorType) != SQL_SUCCESS)
                pDb->DispAllErrors(henv, hdbc, hstmtInternal);
#ifdef DBDEBUG_CONSOLE
            std::wcout << L"Static cursor changed to: ";
            switch(cursorType)
            {
            case SQL_CURSOR_FORWARD_ONLY:
                std::wcout << L"Forward Only";
                break;
            case SQL_CURSOR_STATIC:
                std::wcout << L"Static";
                break;
            case SQL_CURSOR_KEYSET_DRIVEN:
                std::wcout << L"Keyset Driven";
                break;
            case SQL_CURSOR_DYNAMIC:
                std::wcout << L"Dynamic";
                break;
            }
            std::wcout << std::endl << std::endl;
#endif
            // BJO20000425
            if (pDb->FwdOnlyCursors() && cursorType != SQL_CURSOR_FORWARD_ONLY)
            {
                // Force the use of a forward only cursor...
                cursorType = SQL_CURSOR_FORWARD_ONLY;
                if (SQLSetStmtOption(hstmtInternal, SQL_CURSOR_TYPE, cursorType) != SQL_SUCCESS)
                {
                    // Should never happen
                    pDb->GetNextError(henv, hdbc, hstmtInternal);
                    return false;
                }
            }
        }
        else
        {
            pDb->DispNextError();
            pDb->DispAllErrors(henv, hdbc, hstmtInternal);
        }
    }
#ifdef DBDEBUG_CONSOLE
    else
        std::wcout << L"Cursor Type set to STATIC" << std::endl << std::endl;
#endif

    if (!queryOnly)
    {
        // Set the cursor type for the INSERT statement handle
        if (SQLSetStmtOption(hstmtInsert, SQL_CURSOR_TYPE, SQL_CURSOR_FORWARD_ONLY) != SQL_SUCCESS)
            pDb->DispAllErrors(henv, hdbc, hstmtInsert);
        // Set the cursor type for the DELETE statement handle
        if (SQLSetStmtOption(hstmtDelete, SQL_CURSOR_TYPE, SQL_CURSOR_FORWARD_ONLY) != SQL_SUCCESS)
            pDb->DispAllErrors(henv, hdbc, hstmtDelete);
        // Set the cursor type for the UPDATE statement handle
        if (SQLSetStmtOption(hstmtUpdate, SQL_CURSOR_TYPE, SQL_CURSOR_FORWARD_ONLY) != SQL_SUCCESS)
            pDb->DispAllErrors(henv, hdbc, hstmtUpdate);
    }

    // Make the default cursor the active cursor
    hstmtDefault = GetNewCursor(false,false);
    exASSERT(hstmtDefault);
    hstmt = *hstmtDefault;

    return true;

}  // wxDbTable::initialize()


void wxDbTable::cleanup()
{
    std::wstring s;
    if (pDb)
    {
		s = (boost::wformat(L"wxDbTable destructor (%-20s) tableID:[%6lu] pDb:[%p]") % tableName %tableID %static_cast<void*>(pDb)).str();
        pDb->WriteSqlLog(s);
    }

#ifdef __WXDEBUG__
    if (tableID)
    {
        bool found = false;

        //wxList::compatibility_iterator pNode;
		std::vector<wxTablesInUse*>::iterator it = TablesInUse.begin();
        {
#if wxUSE_THREADS
            wxCriticalSectionLocker lock(csTablesInUse);
#endif // wxUSE_THREADS
            // pNode = TablesInUse.GetFirst();
            while (!found && it != TablesInUse.end())
            {
				wxTablesInUse* pNode = *it;
				if(pNode->tableID == tableID)
//                if (((wxTablesInUse *)pNode->GetData())->tableID == tableID)
                {
                    found = true;
					TablesInUse.erase(it);
					delete pNode;
                    //delete (wxTablesInUse *)pNode->GetData();
                    //TablesInUse.Erase(pNode);
                }
                else
					it++;
                    //pNode = pNode->GetNext();
            }
        }
        if (!found)
        {
            std::wstring msg;
			msg = (boost::wformat(L"Unable to find the tableID in the vector\n of tables in use.\n\n%s") %s).str();
			BOOST_LOG_TRIVIAL(debug) << msg;
        }
    }
#endif

    // Decrement the wxDb table count
    if (pDb)
        pDb->decrementTableCount();

    // Delete memory allocated for column definitions
    if (colDefs)
        delete [] colDefs;

    // Free statement handles
    if (!queryOnly)
    {
        if (hstmtInsert)
        {
/*
ODBC 3.0 says to use this form
            if (SQLFreeHandle(*hstmtDel, SQL_DROP) != SQL_SUCCESS)
*/
            if (SQLFreeStmt(hstmtInsert, SQL_DROP) != SQL_SUCCESS)
                pDb->DispAllErrors(henv, hdbc);
        }

        if (hstmtDelete)
        {
/*
ODBC 3.0 says to use this form
            if (SQLFreeHandle(*hstmtDel, SQL_DROP) != SQL_SUCCESS)
*/
            if (SQLFreeStmt(hstmtDelete, SQL_DROP) != SQL_SUCCESS)
                pDb->DispAllErrors(henv, hdbc);
        }

        if (hstmtUpdate)
        {
/*
ODBC 3.0 says to use this form
            if (SQLFreeHandle(*hstmtDel, SQL_DROP) != SQL_SUCCESS)
*/
            if (SQLFreeStmt(hstmtUpdate, SQL_DROP) != SQL_SUCCESS)
                pDb->DispAllErrors(henv, hdbc);
        }
    }

    if (hstmtInternal)
    {
        if (SQLFreeStmt(hstmtInternal, SQL_DROP) != SQL_SUCCESS)
            pDb->DispAllErrors(henv, hdbc);
    }

    // Delete dynamically allocated cursors
    if (hstmtDefault)
        DeleteCursor(hstmtDefault);

    if (hstmtCount)
        DeleteCursor(hstmtCount);

    if (m_hstmtGridQuery)
        DeleteCursor(m_hstmtGridQuery);

}  // wxDbTable::cleanup()


/***************************** PRIVATE FUNCTIONS *****************************/


void wxDbTable::setCbValueForColumn(int columnIndex)
{
    switch(colDefs[columnIndex].m_DbDataType)
    {
        case DB_DATA_TYPE_VARCHAR:
        case DB_DATA_TYPE_MEMO:
            if (colDefs[columnIndex].m_null)
                colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
            else
                colDefs[columnIndex].m_cbValue = SQL_NTS;
            break;
        case DB_DATA_TYPE_INTEGER:
            if (colDefs[columnIndex].m_null)
                colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
            else
                colDefs[columnIndex].m_cbValue = 0;
            break;
        case DB_DATA_TYPE_FLOAT:
            if (colDefs[columnIndex].m_null)
                colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
            else
                colDefs[columnIndex].m_cbValue = 0;
            break;
        case DB_DATA_TYPE_DATE:
            if (colDefs[columnIndex].m_null)
                colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
            else
                colDefs[columnIndex].m_cbValue = 0;
            break;
        case DB_DATA_TYPE_BLOB:
            if (colDefs[columnIndex].m_null)
                colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
            else
                if (colDefs[columnIndex].m_SqlCtype == SQL_C_WXCHAR)
                    colDefs[columnIndex].m_cbValue = SQL_NTS;
                else
                    colDefs[columnIndex].m_cbValue = SQL_LEN_DATA_AT_EXEC(colDefs[columnIndex].m_szDataObj);
            break;
    }
}

/********** wxDbTable::bindParams() **********/
bool wxDbTable::bindParams(bool forUpdate)
{
    exASSERT(!queryOnly);
    if (queryOnly)
        return false;

    SWORD   fSqlType    = 0;
    SDWORD  precision   = 0;
    SWORD   scale       = 0;

    // Bind each column of the table that should be bound
    // to a parameter marker
    int i;
    UWORD colNumber;

    for (i=0, colNumber=1; i < m_numCols; i++)
    {
        if (forUpdate)
        {
            if (!colDefs[i].m_updateable)
                continue;
        }
        else
        {
            if (!colDefs[i].m_insertAllowed)
                continue;
        }

        switch(colDefs[i].m_DbDataType)
        {
            case DB_DATA_TYPE_VARCHAR:
                fSqlType = pDb->GetTypeInfVarchar().FsqlType;
                precision = colDefs[i].m_szDataObj;
                scale = 0;
                break;
            case DB_DATA_TYPE_MEMO:
                fSqlType = pDb->GetTypeInfMemo().FsqlType;
                precision = colDefs[i].m_szDataObj;
                scale = 0;
                break;
            case DB_DATA_TYPE_INTEGER:
                fSqlType = pDb->GetTypeInfInteger().FsqlType;
                precision = pDb->GetTypeInfInteger().Precision;
                scale = 0;
                break;
            case DB_DATA_TYPE_FLOAT:
                fSqlType = pDb->GetTypeInfFloat().FsqlType;
                precision = pDb->GetTypeInfFloat().Precision;
                scale = pDb->GetTypeInfFloat().MaximumScale;
                // SQL Sybase Anywhere v5.5 returned a negative number for the
                // MaxScale.  This caused ODBC to kick out an error on ibscale.
                // I check for this here and set the scale = precision.
                //if (scale < 0)
                // scale = (short) precision;
                break;
            case DB_DATA_TYPE_DATE:
                fSqlType = pDb->GetTypeInfDate().FsqlType;
                precision = pDb->GetTypeInfDate().Precision;
                scale = 0;
                break;
            case DB_DATA_TYPE_BLOB:
                fSqlType = pDb->GetTypeInfBlob().FsqlType;
                precision = colDefs[i].m_szDataObj;
                scale = 0;
                break;
        }

        setCbValueForColumn(i);

        if (forUpdate)
        {
            if (SQLBindParameter(hstmtUpdate, colNumber++, SQL_PARAM_INPUT, colDefs[i].m_SqlCtype,
                                 fSqlType, precision, scale, (UCHAR*) colDefs[i].m_PtrDataObj,
                                 precision+1, &colDefs[i].m_cbValue) != SQL_SUCCESS)
            {
                return(pDb->DispAllErrors(henv, hdbc, hstmtUpdate));
            }
        }
        else
        {
            if (SQLBindParameter(hstmtInsert, colNumber++, SQL_PARAM_INPUT, colDefs[i].m_SqlCtype,
                                 fSqlType, precision, scale, (UCHAR*) colDefs[i].m_PtrDataObj,
                                 precision+1, &colDefs[i].m_cbValue) != SQL_SUCCESS)
            {
                return(pDb->DispAllErrors(henv, hdbc, hstmtInsert));
            }
        }
    }

    // Completed successfully
    return true;

}  // wxDbTable::bindParams()


/********** wxDbTable::bindInsertParams() **********/
bool wxDbTable::bindInsertParams()
{
    return bindParams(false);
}  // wxDbTable::bindInsertParams()


/********** wxDbTable::bindUpdateParams() **********/
bool wxDbTable::bindUpdateParams()
{
    return bindParams(true);
}  // wxDbTable::bindUpdateParams()


/********** wxDbTable::bindCols() **********/
bool wxDbTable::bindCols(HSTMT cursor)
{
    // Bind each column of the table to a memory address for fetching data
    UWORD i;
    for (i = 0; i < m_numCols; i++)
    {
		ColumnDefinition def = colDefs[i];
        if (SQLBindCol(cursor, (UWORD)(i+1), colDefs[i].m_SqlCtype, (UCHAR*) colDefs[i].m_PtrDataObj,
                       colDefs[i].m_szDataObj, &colDefs[i].m_cbValue ) != SQL_SUCCESS)
          return (pDb->DispAllErrors(henv, hdbc, cursor));
    }

    // Completed successfully
    return true;
}  // wxDbTable::bindCols()


/********** wxDbTable::getRec() **********/
bool wxDbTable::getRec(UWORD fetchType)
{
    RETCODE retcode;

    if (!pDb->FwdOnlyCursors())
    {
        // Fetch the NEXT, PREV, FIRST or LAST record, depending on fetchType
        SQLULEN cRowsFetched;
        UWORD   rowStatus;

        retcode = SQLExtendedFetch(hstmt, fetchType, 0, &cRowsFetched, &rowStatus);
        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
        {
            if (retcode == SQL_NO_DATA_FOUND)
                return false;
            else
                return(pDb->DispAllErrors(henv, hdbc, hstmt));
        }
        else
        {
            // Set the Null member variable to indicate the Null state
            // of each column just read in.
            int i;
            for (i = 0; i < m_numCols; i++)
                colDefs[i].m_null = (colDefs[i].m_cbValue == SQL_NULL_DATA);
        }
    }
    else
    {
        // Fetch the next record from the record set
        retcode = SQLFetch(hstmt);
        if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
        {
            if (retcode == SQL_NO_DATA_FOUND)
                return false;
            else
                return(pDb->DispAllErrors(henv, hdbc, hstmt));
        }
        else
        {
            // Set the Null member variable to indicate the Null state
            // of each column just read in.
            int i;
            for (i = 0; i < m_numCols; i++)
                colDefs[i].m_null = (colDefs[i].m_cbValue == SQL_NULL_DATA);
        }
    }

    // Completed successfully
    return true;

}  // wxDbTable::getRec()


/********** wxDbTable::execDelete() **********/
bool wxDbTable::execDelete(const std::wstring &pSqlStmt)
{
    RETCODE retcode;

    // Execute the DELETE statement
    retcode = SQLExecDirect(hstmtDelete, (SQLTCHAR FAR *) pSqlStmt.c_str(), SQL_NTS);

    if (retcode == SQL_SUCCESS ||
        retcode == SQL_NO_DATA_FOUND ||
        retcode == SQL_SUCCESS_WITH_INFO)
    {
        // Record deleted successfully
        return true;
    }

    // Problem deleting record
    return(pDb->DispAllErrors(henv, hdbc, hstmtDelete));

}  // wxDbTable::execDelete()


/********** wxDbTable::execUpdate() **********/
bool wxDbTable::execUpdate(const std::wstring &pSqlStmt)
{
    RETCODE retcode;

    // Execute the UPDATE statement
    retcode = SQLExecDirect(hstmtUpdate, (SQLTCHAR FAR *) pSqlStmt.c_str(), SQL_NTS);

    if (retcode == SQL_SUCCESS ||
        retcode == SQL_NO_DATA_FOUND ||
        retcode == SQL_SUCCESS_WITH_INFO)
    {
        // Record updated successfully
        return true;
    }
    else if (retcode == SQL_NEED_DATA)
    {
        PTR pParmID;
        retcode = SQLParamData(hstmtUpdate, &pParmID);
        while (retcode == SQL_NEED_DATA)
        {
            // Find the parameter
            int i;
            for (i=0; i < m_numCols; i++)
            {
                if (colDefs[i].m_PtrDataObj == pParmID)
                {
                    // We found it.  Store the parameter.
                    retcode = SQLPutData(hstmtUpdate, pParmID, colDefs[i].m_szDataObj);
                    if (retcode != SQL_SUCCESS)
                    {
                        pDb->DispNextError();
                        return pDb->DispAllErrors(henv, hdbc, hstmtUpdate);
                    }
                    break;
                }
            }
            retcode = SQLParamData(hstmtUpdate, &pParmID);
        }
        if (retcode == SQL_SUCCESS ||
            retcode == SQL_NO_DATA_FOUND ||
            retcode == SQL_SUCCESS_WITH_INFO)
        {
            // Record updated successfully
            return true;
        }
    }

    // Problem updating record
    return(pDb->DispAllErrors(henv, hdbc, hstmtUpdate));

}  // wxDbTable::execUpdate()


/********** wxDbTable::query() **********/
bool wxDbTable::query(int queryType, bool forUpdate, bool distinct, const std::wstring &pSqlStmt)
{
    std::wstring sqlStmt;

    if (forUpdate)
        // The user may wish to select for update, but the DBMS may not be capable
        selectForUpdate = CanSelectForUpdate();
    else
        selectForUpdate = false;

    // Set the SQL SELECT string
    if (queryType != DB_SELECT_STATEMENT)               // A select statement was not passed in,
    {                                                   // so generate a select statement.
        BuildSelectStmt(sqlStmt, queryType, distinct);
        pDb->WriteSqlLog(sqlStmt);
    }

    // Make sure the cursor is closed first
    if (!CloseCursor(hstmt))
        return false;

    // Execute the SQL SELECT statement
    int retcode;
    retcode = SQLExecDirect(hstmt, (SQLTCHAR FAR *) (queryType == DB_SELECT_STATEMENT ? pSqlStmt.c_str() : sqlStmt.c_str()), SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
        return(pDb->DispAllErrors(henv, hdbc, hstmt));

    // Completed successfully
    return true;

}  // wxDbTable::query()


/***************************** PUBLIC FUNCTIONS *****************************/


/********** wxDbTable::Open() **********/
bool wxDbTable::Open(bool checkPrivileges, bool checkTableExists)
{
    if (!pDb)
        return false;

    int i;
    std::wstring sqlStmt;
    std::wstring s;

    // Calculate the maximum size of the concatenated
    // keys for use with wxDbGrid
    m_keysize = 0;
    for (i=0; i < m_numCols; i++)
    {
        if (colDefs[i].m_keyField)
        {
            m_keysize += colDefs[i].m_szDataObj;
        }
    }

    s.empty();

    bool exists = true;
    if (checkTableExists)
    {
        if (pDb->Dbms() == dbmsPOSTGRES)
            exists = pDb->TableExists(tableName, NULL, tablePath);
        else
            exists = pDb->TableExists(tableName, pDb->GetUsername().c_str(), tablePath);
    }

    // Verify that the table exists in the database
    if (!exists)
    {
        s = L"Table/view does not exist in the database";
        if ( *(pDb->dbInf.accessibleTables) == L'Y')
            s += L", or you have no permissions.\n";
        else
            s += L".\n";
    }
    else if (checkPrivileges)
    {
        // Verify the user has rights to access the table.
        bool hasPrivs;

        if (pDb->Dbms() == dbmsPOSTGRES)
            hasPrivs = pDb->TablePrivileges(tableName, L"SELECT", pDb->GetUsername().c_str(), NULL, tablePath);
        else
            hasPrivs = pDb->TablePrivileges(tableName, L"SELECT", pDb->GetUsername().c_str(), pDb->GetUsername().c_str(), tablePath);

        if (!hasPrivs)
            s = L"Connecting user does not have sufficient privileges to access this table.\n";
    }

    if (!s.empty())
    {
        std::wstring p;

        if (!tablePath.empty())
			p = (boost::wformat(L"Error OpenImpling '%s/%s'.\n") %tablePath %tableName).str();
        else
			p = (boost::wformat(L"Error OpenImpling '%s'.\n") %tableName).str();

        p += s;
        pDb->LogError(p);

        return false;
    }

    // Bind the member variables for field exchange between
    // the wxDbTable object and the ODBC record.
    if (!queryOnly)
    {
        if (!bindInsertParams())                    // Inserts
            return false;

        if (!bindUpdateParams())                    // Updates
            return false;
    }

    if (!bindCols(*hstmtDefault))                   // Selects
        return false;

    if (!bindCols(hstmtInternal))                   // Internal use only
        return false;

     /*
     * Do NOT bind the hstmtCount cursor!!!
     */

    // Build an insert statement using parameter markers
    if (!queryOnly && m_numCols > 0)
    {
        bool needComma = false;
		sqlStmt = (boost::wformat(L"INSERT INTO %s (") % pDb->SQLTableName(tableName.c_str())).str();
        for (i = 0; i < m_numCols; i++)
        {
            if (! colDefs[i].m_insertAllowed)
                continue;
            if (needComma)
                sqlStmt += L",";
            sqlStmt += pDb->SQLColumnName(colDefs[i].m_ColName);
            needComma = true;
        }
        needComma = false;
        sqlStmt += L") VALUES (";

        int insertableCount = 0;

        for (i = 0; i < m_numCols; i++)
        {
            if (! colDefs[i].m_insertAllowed)
                continue;
            if (needComma)
                sqlStmt += L",";
            sqlStmt += L"?";
            needComma = true;
            insertableCount++;
        }
        sqlStmt += L")";

        // Prepare the insert statement for execution
        if (insertableCount)
        {
            if (SQLPrepare(hstmtInsert, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS) != SQL_SUCCESS)
                return(pDb->DispAllErrors(henv, hdbc, hstmtInsert));
        }
        else
            insertable = false;
    }

    // Completed successfully
    return true;

}  // wxDbTable::Open()


/********** wxDbTable::Query() **********/
bool wxDbTable::Query(bool forUpdate, bool distinct)
{

    return(query(DB_SELECT_WHERE, forUpdate, distinct));

}  // wxDbTable::Query()


/********** wxDbTable::QueryBySqlStmt() **********/
bool wxDbTable::QueryBySqlStmt(const std::wstring &pSqlStmt)
{
    pDb->WriteSqlLog(pSqlStmt);

    return(query(DB_SELECT_STATEMENT, false, false, pSqlStmt));

}  // wxDbTable::QueryBySqlStmt()


/********** wxDbTable::QueryMatching() **********/
bool wxDbTable::QueryMatching(bool forUpdate, bool distinct)
{

    return(query(DB_SELECT_MATCHING, forUpdate, distinct));

}  // wxDbTable::QueryMatching()


/********** wxDbTable::QueryOnKeyFields() **********/
bool wxDbTable::QueryOnKeyFields(bool forUpdate, bool distinct)
{

    return(query(DB_SELECT_KEYFIELDS, forUpdate, distinct));

}  // wxDbTable::QueryOnKeyFields()


/********** wxDbTable::GetPrev() **********/
bool wxDbTable::GetPrev()
{
    if (pDb->FwdOnlyCursors())
    {
        exFAIL_MSG(L"GetPrev()::Backward scrolling cursors are not enabled for this instance of wxDbTable");
        return false;
    }
    else
        return(getRec(SQL_FETCH_PRIOR));

}  // wxDbTable::GetPrev()


/********** wxDbTable::operator-- **********/
bool wxDbTable::operator--(int)
{
    if (pDb->FwdOnlyCursors())
    {
        exFAIL_MSG(L"operator--:Backward scrolling cursors are not enabled for this instance of wxDbTable");
        return false;
    }
    else
        return(getRec(SQL_FETCH_PRIOR));

}  // wxDbTable::operator--


/********** wxDbTable::GetFirst() **********/
bool wxDbTable::GetFirst()
{
    if (pDb->FwdOnlyCursors())
    {
        exFAIL_MSG(L"GetFirst():Backward scrolling cursors are not enabled for this instance of wxDbTable");
        return false;
    }
    else
        return(getRec(SQL_FETCH_FIRST));

}  // wxDbTable::GetFirst()


/********** wxDbTable::GetLast() **********/
bool wxDbTable::GetLast()
{
    if (pDb->FwdOnlyCursors())
    {
        exFAIL_MSG(L"GetLast()::Backward scrolling cursors are not enabled for this instance of wxDbTable");
        return false;
    }
    else
        return(getRec(SQL_FETCH_LAST));

}  // wxDbTable::GetLast()


/********** wxDbTable::BuildDeleteStmt() **********/
void wxDbTable::BuildDeleteStmt(std::wstring &pSqlStmt, int typeOfDel, const std::wstring &pWhereClause)
{
    exASSERT(!queryOnly);
    if (queryOnly)
        return;

    std::wstring whereClause;

    whereClause.empty();

    // Handle the case of DeleteWhere() and the where clause is blank.  It should
    // delete all records from the database in this case.
    if (typeOfDel == DB_DEL_WHERE && (pWhereClause.length() == 0))
    {
		pSqlStmt = (boost::wformat(L"DELETE FROM %s") %pDb->SQLTableName(tableName.c_str())).str();
        return;
    }

	pSqlStmt = (boost::wformat(L"DELETE FROM %s WHERE ") % pDb->SQLTableName(tableName.c_str())).str();

    // Append the WHERE clause to the SQL DELETE statement
    switch(typeOfDel)
    {
        case DB_DEL_KEYFIELDS:
            // If the datasource supports the ROWID column, build
            // the where on ROWID for efficiency purposes.
            // e.g. DELETE FROM PARTS WHERE ROWID = '111.222.333'
            if (CanUpdateByROWID())
            {
                SQLLEN cb;
                wchar_t   rowid[wxDB_ROWID_LEN+1];

                // Get the ROWID value.  If not successful retreiving the ROWID,
                // simply fall down through the code and build the WHERE clause
                // based on the key fields.
                if (SQLGetData(hstmt, (UWORD)(m_numCols+1), SQL_C_WXCHAR, (UCHAR*) rowid, sizeof(rowid), &cb) == SQL_SUCCESS)
                {
                    pSqlStmt += L"ROWID = '";
                    pSqlStmt += rowid;
                    pSqlStmt += L"'";
                    break;
                }
            }
            // Unable to delete by ROWID, so build a WHERE
            // clause based on the keyfields.
            BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS);
            pSqlStmt += whereClause;
            break;
        case DB_DEL_WHERE:
            pSqlStmt += pWhereClause;
            break;
        case DB_DEL_MATCHING:
            BuildWhereClause(whereClause, DB_WHERE_MATCHING);
            pSqlStmt += whereClause;
            break;
    }

}  // BuildDeleteStmt()


/***** DEPRECATED: use wxDbTable::BuildDeleteStmt(std::wstring &....) form *****/
void wxDbTable::BuildDeleteStmt(wchar_t *pSqlStmt, int typeOfDel, const std::wstring &pWhereClause)
{
    std::wstring tempSqlStmt;
    BuildDeleteStmt(tempSqlStmt, typeOfDel, pWhereClause);
    wcscpy(pSqlStmt, tempSqlStmt.c_str());
}  // wxDbTable::BuildDeleteStmt()


/********** wxDbTable::BuildSelectStmt() **********/
void wxDbTable::BuildSelectStmt(std::wstring &pSqlStmt, int typeOfSelect, bool distinct)
{
    std::wstring whereClause;
    whereClause.empty();

    // Build a select statement to query the database
    pSqlStmt = L"SELECT ";

    // SELECT DISTINCT values only?
    if (distinct)
        pSqlStmt += L"DISTINCT ";

    // Was a FROM clause specified to join tables to the base table?
    // Available for ::Query() only!!!
    bool appendFromClause = false;

    if (typeOfSelect == DB_SELECT_WHERE && from.length())
        appendFromClause = true;

    // Add the column list
    int i;
    std::wstring tStr;
    for (i = 0; i < m_numCols; i++)
    {
        tStr = colDefs[i].m_ColName;
        // If joining tables, the base table column names must be qualified to avoid ambiguity
        if ((appendFromClause || pDb->Dbms() == dbmsACCESS) && tStr.find(L'.') == std::wstring::npos)
        {
            pSqlStmt += pDb->SQLTableName(queryTableName.c_str());
            pSqlStmt += L".";
        }
        pSqlStmt += pDb->SQLColumnName(colDefs[i].m_ColName);
        if (i + 1 < m_numCols)
            pSqlStmt += L",";
    }

    // If the datasource supports ROWID, get this column as well.  Exception: Don't retrieve
    // the ROWID if querying distinct records.  The rowid will always be unique.
    if (!distinct && CanUpdateByROWID())
    {
        // If joining tables, the base table column names must be qualified to avoid ambiguity
        if (appendFromClause || pDb->Dbms() == dbmsACCESS)
        {
            pSqlStmt += L",";
            pSqlStmt += pDb->SQLTableName(queryTableName.c_str());
            pSqlStmt += L".ROWID";
        }
        else
            pSqlStmt += L",ROWID";
    }

    // Append the FROM tablename portion
    pSqlStmt += L" FROM ";
    pSqlStmt += pDb->SQLTableName(queryTableName.c_str());
//    pSqlStmt += queryTableName;

    // Sybase uses the HOLDLOCK keyword to lock a record during query.
    // The HOLDLOCK keyword follows the table name in the from clause.
    // Each table in the from clause must specify HOLDLOCK or
    // NOHOLDLOCK (the default).  Note: The "FOR UPDATE" clause
    // is parsed but ignored in SYBASE Transact-SQL.
    if (selectForUpdate && (pDb->Dbms() == dbmsSYBASE_ASA || pDb->Dbms() == dbmsSYBASE_ASE))
        pSqlStmt += L" HOLDLOCK";

    if (appendFromClause)
        pSqlStmt += from;

    // Append the WHERE clause.  Either append the where clause for the class
    // or build a where clause.  The typeOfSelect determines this.
    switch(typeOfSelect)
    {
        case DB_SELECT_WHERE:

            if (where.length())   // May not want a where clause!!!
            {
                pSqlStmt += L" WHERE ";
                pSqlStmt += where;
            }
            break;
        case DB_SELECT_KEYFIELDS:
            BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS);
            if (whereClause.length())
            {
                pSqlStmt += L" WHERE ";
                pSqlStmt += whereClause;
            }
            break;
        case DB_SELECT_MATCHING:
            BuildWhereClause(whereClause, DB_WHERE_MATCHING);
            if (whereClause.length())
            {
                pSqlStmt += L" WHERE ";
                pSqlStmt += whereClause;
            }
            break;
    }

    // Append the ORDER BY clause
    if (orderBy.length())
    {
        pSqlStmt += L" ORDER BY ";
        pSqlStmt += orderBy;
    }

    // SELECT FOR UPDATE if told to do so and the datasource is capable.  Sybase
    // parses the FOR UPDATE clause but ignores it.  See the comment above on the
    // HOLDLOCK for Sybase.
    if (selectForUpdate && CanSelectForUpdate())
        pSqlStmt += L" FOR UPDATE";

}  // wxDbTable::BuildSelectStmt()


/***** DEPRECATED: use wxDbTable::BuildSelectStmt(std::wstring &....) form *****/
void wxDbTable::BuildSelectStmt(wchar_t *pSqlStmt, int typeOfSelect, bool distinct)
{
    std::wstring tempSqlStmt;
    BuildSelectStmt(tempSqlStmt, typeOfSelect, distinct);
    wcscpy(pSqlStmt, tempSqlStmt.c_str());
}  // wxDbTable::BuildSelectStmt()


/********** wxDbTable::BuildUpdateStmt() **********/
void wxDbTable::BuildUpdateStmt(std::wstring &pSqlStmt, int typeOfUpdate, const std::wstring &pWhereClause)
{
    exASSERT(!queryOnly);
    if (queryOnly)
        return;

    std::wstring whereClause;
    whereClause.empty();

    bool firstColumn = true;

	pSqlStmt = (boost::wformat(L"UPDATE %s SET ") % pDb->SQLTableName(tableName.c_str())).str();

    // Append a list of columns to be updated
    int i;
    for (i = 0; i < m_numCols; i++)
    {
        // Only append Updateable columns
        if (colDefs[i].m_updateable)
        {
            if (!firstColumn)
                pSqlStmt += L",";
            else
                firstColumn = false;

            pSqlStmt += pDb->SQLColumnName(colDefs[i].m_ColName);
//            pSqlStmt += colDefs[i].ColName;
            pSqlStmt += L" = ?";
        }
    }

    // Append the WHERE clause to the SQL UPDATE statement
    pSqlStmt += L" WHERE ";
    switch(typeOfUpdate)
    {
        case DB_UPD_KEYFIELDS:
            // If the datasource supports the ROWID column, build
            // the where on ROWID for efficiency purposes.
            // e.g. UPDATE PARTS SET Col1 = ?, Col2 = ? WHERE ROWID = '111.222.333'
            if (CanUpdateByROWID())
            {
                SQLLEN cb;
                wchar_t rowid[wxDB_ROWID_LEN+1];

                // Get the ROWID value.  If not successful retreiving the ROWID,
                // simply fall down through the code and build the WHERE clause
                // based on the key fields.
                if (SQLGetData(hstmt, (UWORD)(m_numCols+1), SQL_C_WXCHAR, (UCHAR*) rowid, sizeof(rowid), &cb) == SQL_SUCCESS)
                {
                    pSqlStmt += L"ROWID = '";
                    pSqlStmt += rowid;
                    pSqlStmt += L"'";
                    break;
                }
            }
            // Unable to delete by ROWID, so build a WHERE
            // clause based on the keyfields.
            BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS);
            pSqlStmt += whereClause;
            break;
        case DB_UPD_WHERE:
            pSqlStmt += pWhereClause;
            break;
    }
}  // BuildUpdateStmt()


/***** DEPRECATED: use wxDbTable::BuildUpdateStmt(std::wstring &....) form *****/
void wxDbTable::BuildUpdateStmt(wchar_t *pSqlStmt, int typeOfUpdate, const std::wstring &pWhereClause)
{
    std::wstring tempSqlStmt;
    BuildUpdateStmt(tempSqlStmt, typeOfUpdate, pWhereClause);
    wcscpy(pSqlStmt, tempSqlStmt.c_str());
}  // BuildUpdateStmt()


/********** wxDbTable::BuildWhereClause() **********/
void wxDbTable::BuildWhereClause(std::wstring &pWhereClause, int typeOfWhere,
                                 const std::wstring &qualTableName, bool useLikeComparison)
/*
 * Note: BuildWhereClause() currently ignores timestamp columns.
 *       They are not included as part of the where clause.
 */
{
    bool moreThanOneColumn = false;
    std::wstring colValue;

    // Loop through the columns building a where clause as you go
    int colNumber;
    for (colNumber = 0; colNumber < m_numCols; colNumber++)
    {
        // Determine if this column should be included in the WHERE clause
        if ((typeOfWhere == DB_WHERE_KEYFIELDS && colDefs[colNumber].m_keyField) ||
             (typeOfWhere == DB_WHERE_MATCHING  && (!IsColNull((UWORD)colNumber))))
        {
            // Skip over timestamp columns
            if (colDefs[colNumber].m_SqlCtype == SQL_C_TIMESTAMP)
                continue;
            // If there is more than 1 column, join them with the keyword "AND"
            if (moreThanOneColumn)
                pWhereClause += L" AND ";
            else
                moreThanOneColumn = true;

            // Concatenate where phrase for the column
            std::wstring tStr = colDefs[colNumber].m_ColName;

            if (qualTableName.length() && tStr.find(L'.') == std::wstring::npos)
            {
                pWhereClause += pDb->SQLTableName(qualTableName.c_str());
                pWhereClause += L".";
            }
            pWhereClause += pDb->SQLColumnName(colDefs[colNumber].m_ColName);

            if (useLikeComparison && (colDefs[colNumber].m_SqlCtype == SQL_C_WXCHAR))
                pWhereClause += L" LIKE ";
            else
                pWhereClause += L" = ";

            switch(colDefs[colNumber].m_SqlCtype)
            {
                case SQL_C_CHAR:
#ifdef SQL_C_WCHAR
                case SQL_C_WCHAR:
#endif
                //case SQL_C_WXCHAR:  SQL_C_WXCHAR is covered by either SQL_C_CHAR or SQL_C_WCHAR
					colValue = (boost::wformat(L"'%s'") % GetDb()->EscapeSqlChars((wchar_t *)colDefs[colNumber].m_PtrDataObj)).str();
                    break;
                case SQL_C_SHORT:
                case SQL_C_SSHORT:
					colValue = (boost::wformat(L"%hi") % *((SWORD *) colDefs[colNumber].m_PtrDataObj)).str();
                    break;
                case SQL_C_USHORT:
					colValue = (boost::wformat(L"%hu") % *((UWORD *) colDefs[colNumber].m_PtrDataObj)).str();
                    break;
                case SQL_C_LONG:
                case SQL_C_SLONG:
					colValue = (boost::wformat(L"%li") % *((SDWORD *) colDefs[colNumber].m_PtrDataObj)).str();
                    break;
                case SQL_C_ULONG:
					colValue = (boost::wformat(L"%lu") % *((UDWORD *) colDefs[colNumber].m_PtrDataObj)).str();
                    break;
                case SQL_C_FLOAT:
					colValue = (boost::wformat(L"%.6f") % *((SFLOAT *) colDefs[colNumber].m_PtrDataObj)).str();
                    break;
                case SQL_C_DOUBLE:
					colValue = (boost::wformat(L"%.6f") % *((SDOUBLE *) colDefs[colNumber].m_PtrDataObj)).str();
                    break;
                default:
                    {
                        std::wstring strMsg;
						strMsg = (boost::wformat(L"wxDbTable::bindParams(): Unknown column type for colDefs %d colName %s") % colNumber % colDefs[colNumber].m_ColName).str();
                        exFAIL_MSG(strMsg.c_str());
                    }
                    break;
            }
            pWhereClause += colValue;
        }
    }
}  // wxDbTable::BuildWhereClause()


/***** DEPRECATED: use wxDbTable::BuildWhereClause(std::wstring &....) form *****/
void wxDbTable::BuildWhereClause(wchar_t *pWhereClause, int typeOfWhere,
                                 const std::wstring &qualTableName, bool useLikeComparison)
{
    std::wstring tempSqlStmt;
    BuildWhereClause(tempSqlStmt, typeOfWhere, qualTableName, useLikeComparison);
    wcscpy(pWhereClause, tempSqlStmt.c_str());
}  // wxDbTable::BuildWhereClause()


/********** wxDbTable::GetRowNum() **********/
UWORD wxDbTable::GetRowNum()
{
    UDWORD rowNum;

    if (SQLGetStmtOption(hstmt, SQL_ROW_NUMBER, (UCHAR*) &rowNum) != SQL_SUCCESS)
    {
        pDb->DispAllErrors(henv, hdbc, hstmt);
        return(0);
    }

    // Completed successfully
    return((UWORD) rowNum);

}  // wxDbTable::GetRowNum()


/********** wxDbTable::CloseCursor() **********/
bool wxDbTable::CloseCursor(HSTMT cursor)
{
    if (SQLFreeStmt(cursor, SQL_CLOSE) != SQL_SUCCESS)
        return(pDb->DispAllErrors(henv, hdbc, cursor));

    // Completed successfully
    return true;

}  // wxDbTable::CloseCursor()


/********** wxDbTable::CreateTable() **********/
bool wxDbTable::CreateTable(bool attemptDrop)
{
    if (!pDb)
        return false;

    int i, j;
    std::wstring sqlStmt;

#ifdef DBDEBUG_CONSOLE
    std::wcout << L"Creating Table " << tableName << L"..." << std::endl;
#endif

    // Drop table first
    if (attemptDrop && !DropTable())
        return false;

    // Create the table
#ifdef DBDEBUG_CONSOLE
    for (i = 0; i < m_numCols; i++)
    {
        // Exclude derived columns since they are NOT part of the base table
        if (colDefs[i].m_derivedCol)
            continue;
        std::wcout << i + 1 << L": " << colDefs[i].m_ColName << L"; ";
        switch(colDefs[i].m_DbDataType)
        {
            case DB_DATA_TYPE_VARCHAR:
                std::wcout << pDb->GetTypeInfVarchar().TypeName << L"(" << (int)(colDefs[i].m_szDataObj / sizeof(wchar_t)) << L")";
                break;
            case DB_DATA_TYPE_MEMO:
                std::wcout << pDb->GetTypeInfMemo().TypeName;
                break;
            case DB_DATA_TYPE_INTEGER:
                std::wcout << pDb->GetTypeInfInteger().TypeName;
                break;
            case DB_DATA_TYPE_FLOAT:
                std::wcout << pDb->GetTypeInfFloat().TypeName;
                break;
            case DB_DATA_TYPE_DATE:
                std::wcout << pDb->GetTypeInfDate().TypeName;
                break;
            case DB_DATA_TYPE_BLOB:
                std::wcout << pDb->GetTypeInfBlob().TypeName;
                break;
        }
        std::wcout << std::endl;
    }
#endif

    // Build a CREATE TABLE string from the colDefs structure.
    bool needComma = false;

	sqlStmt = (boost::wformat(L"CREATE TABLE %s (") % pDb->SQLTableName(tableName.c_str())).str();

    for (i = 0; i < m_numCols; i++)
    {
        // Exclude derived columns since they are NOT part of the base table
        if (colDefs[i].m_derivedCol)
            continue;
        // Comma Delimiter
        if (needComma)
            sqlStmt += L",";
        // Column Name
        sqlStmt += pDb->SQLColumnName(colDefs[i].m_ColName);
//        sqlStmt += colDefs[i].ColName;
        sqlStmt += L" ";
        // Column Type
        switch(colDefs[i].m_DbDataType)
        {
            case DB_DATA_TYPE_VARCHAR:
                sqlStmt += pDb->GetTypeInfVarchar().TypeName;
                break;
            case DB_DATA_TYPE_MEMO:
                sqlStmt += pDb->GetTypeInfMemo().TypeName;
                break;
            case DB_DATA_TYPE_INTEGER:
                sqlStmt += pDb->GetTypeInfInteger().TypeName;
                break;
            case DB_DATA_TYPE_FLOAT:
                sqlStmt += pDb->GetTypeInfFloat().TypeName;
                break;
            case DB_DATA_TYPE_DATE:
                sqlStmt += pDb->GetTypeInfDate().TypeName;
                break;
            case DB_DATA_TYPE_BLOB:
                sqlStmt += pDb->GetTypeInfBlob().TypeName;
                break;
        }
        // For varchars, append the size of the string
        if (colDefs[i].m_DbDataType == DB_DATA_TYPE_VARCHAR &&
            (pDb->Dbms() != dbmsMY_SQL || pDb->GetTypeInfVarchar().TypeName != L"text"))// ||
//            colDefs[i].DbDataType == DB_DATA_TYPE_BLOB)
        {
            std::wstring s;
			s = (boost::wformat(L"(%d)") % (int)(colDefs[i].m_szDataObj / sizeof(wchar_t))).str();
            sqlStmt += s;
        }

        if (pDb->Dbms() == dbmsDB2 ||
            pDb->Dbms() == dbmsMY_SQL ||
            pDb->Dbms() == dbmsSYBASE_ASE  ||
            pDb->Dbms() == dbmsINTERBASE  ||
            pDb->Dbms() == dbmsFIREBIRD  ||
            pDb->Dbms() == dbmsMS_SQL_SERVER)
        {
            if (colDefs[i].m_keyField)
            {
                sqlStmt += L" NOT NULL";
            }
        }

        needComma = true;
    }
    // If there is a primary key defined, include it in the create statement
    for (i = j = 0; i < m_numCols; i++)
    {
        if (colDefs[i].m_keyField)
        {
            j++;
            break;
        }
    }
    if ( j && (pDb->Dbms() != dbmsDBASE)
        && (pDb->Dbms() != dbmsXBASE_SEQUITER) )  // Found a keyfield
    {
        switch (pDb->Dbms())
        {
            case dbmsACCESS:
            case dbmsINFORMIX:
            case dbmsSYBASE_ASA:
            case dbmsSYBASE_ASE:
            case dbmsMY_SQL:
            case dbmsFIREBIRD:
            {
                // MySQL goes out on this one. We also declare the relevant key NON NULL above
                sqlStmt += L",PRIMARY KEY (";
                break;
            }
            default:
            {
                sqlStmt += L",CONSTRAINT ";
                //  DB2 is limited to 18 characters for index names
                if (pDb->Dbms() == dbmsDB2)
                {
                    exASSERT_MSG(tableName.length() <= 13, "DB2 table/index names must be no longer than 13 characters in length.\n\nTruncating table name to 13 characters.");
                    sqlStmt += pDb->SQLTableName(tableName.substr(0, 13).c_str());
//                    sqlStmt += tableName.substr(0, 13);
                }
                else
                    sqlStmt += pDb->SQLTableName(tableName.c_str());
//                    sqlStmt += tableName;

                sqlStmt += L"_PIDX PRIMARY KEY (";
                break;
            }
        }

        // List column name(s) of column(s) comprising the primary key
        for (i = j = 0; i < m_numCols; i++)
        {
            if (colDefs[i].m_keyField)
            {
                if (j++) // Multi part key, comma separate names
                    sqlStmt += L",";
                sqlStmt += pDb->SQLColumnName(colDefs[i].m_ColName);

                if (pDb->Dbms() == dbmsMY_SQL &&
                    colDefs[i].m_DbDataType ==  DB_DATA_TYPE_VARCHAR)
                {
                    std::wstring s;
					s = (boost::wformat(L"(%d)") % (int)(colDefs[i].m_szDataObj / sizeof(wchar_t))).str();
                    sqlStmt += s;
                }
            }
        }
        sqlStmt += L")";

        if (pDb->Dbms() == dbmsINFORMIX ||
            pDb->Dbms() == dbmsSYBASE_ASA ||
            pDb->Dbms() == dbmsSYBASE_ASE)
        {
            sqlStmt += L" CONSTRAINT ";
            sqlStmt += pDb->SQLTableName(tableName.c_str());
//            sqlStmt += tableName;
            sqlStmt += L"_PIDX";
        }
    }
    // Append the closing parentheses for the create table statement
    sqlStmt += L")";

    pDb->WriteSqlLog(sqlStmt);

#ifdef DBDEBUG_CONSOLE
    std::wcout << std::endl << sqlStmt.c_str() << std::endl;
#endif

    // Execute the CREATE TABLE statement
    RETCODE retcode = SQLExecDirect(hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
    {
        pDb->DispAllErrors(henv, hdbc, hstmt);
        pDb->RollbackTrans();
        CloseCursor(hstmt);
        return false;
    }

    // Commit the transaction and close the cursor
    if (!pDb->CommitTrans())
        return false;
    if (!CloseCursor(hstmt))
        return false;

    // Database table created successfully
    return true;

} // wxDbTable::CreateTable()


/********** wxDbTable::DropTable() **********/
bool wxDbTable::DropTable()
{
    // NOTE: This function returns true if the Table does not exist, but
    //       only for identified databases.  Code will need to be added
    //       below for any other databases when those databases are defined
    //       to handle this situation consistently

    std::wstring sqlStmt;

	sqlStmt = (boost::wformat(L"DROP TABLE %s") % pDb->SQLTableName(tableName.c_str())).str();

    pDb->WriteSqlLog(sqlStmt);

#ifdef DBDEBUG_CONSOLE
    std::wcout << std::endl << sqlStmt.c_str() << std::endl;
#endif

    RETCODE retcode = SQLExecDirect(hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
    if (retcode != SQL_SUCCESS)
    {
        // Check for "Base table not found" error and ignore
        pDb->GetNextError(henv, hdbc, hstmt);
        if (wcscmp(pDb->sqlState, L"S0002") /*&&
            wcscmp(pDb->sqlState, L"S1000")*/)  // "Base table not found"
        {
            // Check for product specific error codes
            if (!((pDb->Dbms() == dbmsSYBASE_ASA    && !wcscmp(pDb->sqlState, L"42000"))   ||  // 5.x (and lower?)
                  (pDb->Dbms() == dbmsSYBASE_ASE    && !wcscmp(pDb->sqlState, L"37000"))   ||
                  (pDb->Dbms() == dbmsPERVASIVE_SQL && !wcscmp(pDb->sqlState, L"S1000"))   ||  // Returns an S1000 then an S0002
                  (pDb->Dbms() == dbmsPOSTGRES      && !wcscmp(pDb->sqlState, L"08S01"))))
            {
                pDb->DispNextError();
                pDb->DispAllErrors(henv, hdbc, hstmt);
                pDb->RollbackTrans();
//                CloseCursor(hstmt);
                return false;
            }
        }
    }

    // Commit the transaction and close the cursor
    if (! pDb->CommitTrans())
        return false;
    if (! CloseCursor(hstmt))
        return false;

    return true;
}  // wxDbTable::DropTable()


/********** wxDbTable::CreateIndex() **********/
bool wxDbTable::CreateIndex(const std::wstring &indexName, bool unique, UWORD numIndexColumns,
                                     wxDbIdxDef *pIndexDefs, bool attemptDrop)
{
    std::wstring sqlStmt;

    // Drop the index first
    if (attemptDrop && !DropIndex(indexName))
        return false;

    // MySQL (and possibly Sybase ASE?? - gt) require that any columns which are used as portions
    // of an index have the columns defined as "NOT NULL".  During initial table creation though,
    // it may not be known which columns are necessarily going to be part of an index (e.g. the
    // table was created, then months later you determine that an additional index while
    // give better performance, so you want to add an index).
    //
    // The following block of code will modify the column definition to make the column be
    // defined with the "NOT NULL" qualifier.
    if (pDb->Dbms() == dbmsMY_SQL)
    {
        std::wstring sqlStmt;
        int i;
        bool ok = true;
        for (i = 0; i < numIndexColumns && ok; i++)
        {
            int   j = 0;
            bool  found = false;
            // Find the column definition that has the ColName that matches the
            // index column name.  We need to do this to get the DB_DATA_TYPE of
            // the index column, as MySQL's syntax for the ALTER column requires
            // this information
            while (!found && (j < this->m_numCols))
            {
                if (wcscmp(colDefs[j].m_ColName,pIndexDefs[i].ColName) == 0)
                    found = true;
                if (!found)
                    j++;
            }

            if (found)
            {
                ok = pDb->ModifyColumn(tableName, pIndexDefs[i].ColName,
                                        colDefs[j].m_DbDataType, (int)(colDefs[j].m_szDataObj / sizeof(wchar_t)),
                                        L"NOT NULL");

                if (!ok)
                {
                    #if 0
                    // retcode is not used
                    wxODBC_ERRORS retcode;
                    // Oracle returns a DB_ERR_GENERAL_ERROR if the column is already
                    // defined to be NOT NULL, but reportedly MySQL doesn't mind.
                    // This line is just here for debug checking of the value
                    retcode = (wxODBC_ERRORS)pDb->DB_STATUS;
                    #endif
                }
            }
            else
                ok = false;
        }
        if (ok)
            pDb->CommitTrans();
        else
        {
            pDb->RollbackTrans();
            return false;
        }
    }

    // Build a CREATE INDEX statement
    sqlStmt = L"CREATE ";
    if (unique)
        sqlStmt += L"UNIQUE ";

    sqlStmt += L"INDEX ";
    sqlStmt += pDb->SQLTableName(indexName.c_str());
    sqlStmt += L" ON ";

    sqlStmt += pDb->SQLTableName(tableName.c_str());
//    sqlStmt += tableName;
    sqlStmt += L" (";

    // Append list of columns making up index
    int i;
    for (i = 0; i < numIndexColumns; i++)
    {
        sqlStmt += pDb->SQLColumnName(pIndexDefs[i].ColName);
//        sqlStmt += pIndexDefs[i].ColName;

        // MySQL requires a key length on VARCHAR keys
        if ( pDb->Dbms() == dbmsMY_SQL )
        {
            // Find the details on this column
            int j;
            for ( j = 0; j < m_numCols; ++j )
            {
                if ( wcscmp( pIndexDefs[i].ColName, colDefs[j].m_ColName ) == 0 )
                {
                    break;
                }
            }
            if ( colDefs[j].m_DbDataType ==  DB_DATA_TYPE_VARCHAR)
            {
                std::wstring s;
				s = (boost::wformat(L"(%d)") % (int)(colDefs[i].m_szDataObj / sizeof(wchar_t))).str();
                sqlStmt += s;
            }
        }

        // Postgres and SQL Server 7 do not support the ASC/DESC keywords for index columns
        if (!((pDb->Dbms() == dbmsMS_SQL_SERVER) && (wcsncmp(pDb->dbInf.dbmsVer, L"07", 2)==0)) &&
            !(pDb->Dbms() == dbmsFIREBIRD) &&
            !(pDb->Dbms() == dbmsPOSTGRES))
        {
            if (pIndexDefs[i].Ascending)
                sqlStmt += L" ASC";
            else
                sqlStmt += L" DESC";
        }
        else
            exASSERT_MSG(pIndexDefs[i].Ascending, "Datasource does not support DESCending index columns");

        if ((i + 1) < numIndexColumns)
            sqlStmt += L",";
    }

    // Append closing parentheses
    sqlStmt += L")";

    pDb->WriteSqlLog(sqlStmt);

#ifdef DBDEBUG_CONSOLE
    std::wcout << std::endl << sqlStmt.c_str() << std::endl << std::endl;
#endif

    // Execute the CREATE INDEX statement
    RETCODE retcode = SQLExecDirect(hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
    if (retcode != SQL_SUCCESS)
    {
        pDb->DispAllErrors(henv, hdbc, hstmt);
        pDb->RollbackTrans();
        CloseCursor(hstmt);
        return false;
    }

    // Commit the transaction and close the cursor
    if (! pDb->CommitTrans())
        return false;
    if (! CloseCursor(hstmt))
        return false;

    // Index Created Successfully
    return true;

}  // wxDbTable::CreateIndex()


/********** wxDbTable::DropIndex() **********/
bool wxDbTable::DropIndex(const std::wstring &indexName)
{
    // NOTE: This function returns true if the Index does not exist, but
    //       only for identified databases.  Code will need to be added
    //       below for any other databases when those databases are defined
    //       to handle this situation consistently

    std::wstring sqlStmt;

    if (pDb->Dbms() == dbmsACCESS || pDb->Dbms() == dbmsMY_SQL ||
        pDb->Dbms() == dbmsDBASE /*|| Paradox needs this syntax too when we add support*/)
		sqlStmt = (boost::wformat(L"DROP INDEX %s ON %s") % pDb->SQLTableName(indexName.c_str()) % pDb->SQLTableName(tableName.c_str())).str();
    else if ((pDb->Dbms() == dbmsMS_SQL_SERVER) ||
             (pDb->Dbms() == dbmsSYBASE_ASE) ||
             (pDb->Dbms() == dbmsXBASE_SEQUITER))
		sqlStmt = (boost::wformat(L"DROP INDEX %s.%s") % pDb->SQLTableName(tableName.c_str()) % pDb->SQLTableName(indexName.c_str())).str();
    else
		sqlStmt = (boost::wformat(L"DROP INDEX %s") % pDb->SQLTableName(indexName.c_str())).str();

    pDb->WriteSqlLog(sqlStmt);

#ifdef DBDEBUG_CONSOLE
    std::wcout << std::endl << sqlStmt.c_str() << std::endl;
#endif
    RETCODE retcode = SQLExecDirect(hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
    if (retcode != SQL_SUCCESS)
    {
        // Check for "Index not found" error and ignore
        pDb->GetNextError(henv, hdbc, hstmt);
        if (wcscmp(pDb->sqlState, L"S0012"))  // "Index not found"
        {
            // Check for product specific error codes
            if (!((pDb->Dbms() == dbmsSYBASE_ASA    && !wcscmp(pDb->sqlState, L"42000")) ||  // v5.x (and lower?)
                  (pDb->Dbms() == dbmsSYBASE_ASE    && !wcscmp(pDb->sqlState, L"37000")) ||
                  (pDb->Dbms() == dbmsMS_SQL_SERVER && !wcscmp(pDb->sqlState, L"S1000")) ||
                  (pDb->Dbms() == dbmsINTERBASE     && !wcscmp(pDb->sqlState, L"S1000")) ||
                  (pDb->Dbms() == dbmsMAXDB         && !wcscmp(pDb->sqlState, L"S1000")) ||
                  (pDb->Dbms() == dbmsFIREBIRD      && !wcscmp(pDb->sqlState, L"HY000")) ||
                  (pDb->Dbms() == dbmsSYBASE_ASE    && !wcscmp(pDb->sqlState, L"S0002")) ||  // Base table not found
                  (pDb->Dbms() == dbmsMY_SQL        && !wcscmp(pDb->sqlState, L"42S12")) ||  // tested by Christopher Ludwik Marino-Cebulski using v3.23.21beta
                  (pDb->Dbms() == dbmsPOSTGRES      && !wcscmp(pDb->sqlState, L"08S01"))
               ))
            {
                pDb->DispNextError();
                pDb->DispAllErrors(henv, hdbc, hstmt);
                pDb->RollbackTrans();
                CloseCursor(hstmt);
                return false;
            }
        }
    }

    // Commit the transaction and close the cursor
    if (! pDb->CommitTrans())
        return false;
    if (! CloseCursor(hstmt))
        return false;

    return true;
}  // wxDbTable::DropIndex()


/********** wxDbTable::SetOrderByColNums() **********/
bool wxDbTable::SetOrderByColNums(UWORD first, ... )
{
    int         colNumber = first;  // using 'int' to be able to look for wxDB_NO_MORE_COLUN_NUMBERS
    va_list     argptr;

    bool        abort = false;
    std::wstring    tempStr;

    va_start(argptr, first);     /* Initialize variable arguments. */
    while (!abort && (colNumber != wxDB_NO_MORE_COLUMN_NUMBERS))
    {
        // Make sure the passed in column number
        // is within the valid range of columns
        //
        // Valid columns are 0 thru m_numCols-1
        if (colNumber >= m_numCols || colNumber < 0)
        {
            abort = true;
            continue;
        }

        if (colNumber != first)
            tempStr += L",";

        tempStr += colDefs[colNumber].m_ColName;
        colNumber = va_arg (argptr, int);
    }
    va_end (argptr);              /* Reset variable arguments.      */

    SetOrderByClause(tempStr);

    return (!abort);
}  // wxDbTable::SetOrderByColNums()


/********** wxDbTable::Insert() **********/
int wxDbTable::Insert()
{
    exASSERT(!queryOnly);
    if (queryOnly || !insertable)
        return(DB_FAILURE);

    bindInsertParams();

    // Insert the record by executing the already prepared insert statement
    RETCODE retcode;
    retcode = SQLExecute(hstmtInsert);
    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO &&
        retcode != SQL_NEED_DATA)
    {
        // Check to see if integrity constraint was violated
        pDb->GetNextError(henv, hdbc, hstmtInsert);
        if (! wcscmp(pDb->sqlState, L"23000"))  // Integrity constraint violated
            return(DB_ERR_INTEGRITY_CONSTRAINT_VIOL);
        else
        {
            pDb->DispNextError();
            pDb->DispAllErrors(henv, hdbc, hstmtInsert);
            return(DB_FAILURE);
        }
    }
    if (retcode == SQL_NEED_DATA)
    {
        PTR pParmID;
        retcode = SQLParamData(hstmtInsert, &pParmID);
        while (retcode == SQL_NEED_DATA)
        {
            // Find the parameter
            int i;
            for (i=0; i < m_numCols; i++)
            {
                if (colDefs[i].m_PtrDataObj == pParmID)
                {
                    // We found it.  Store the parameter.
                    retcode = SQLPutData(hstmtInsert, pParmID, colDefs[i].m_szDataObj);
                    if (retcode != SQL_SUCCESS)
                    {
                        pDb->DispNextError();
                        pDb->DispAllErrors(henv, hdbc, hstmtInsert);
                        return(DB_FAILURE);
                    }
                    break;
                }
            }
            retcode = SQLParamData(hstmtInsert, &pParmID);
            if (retcode != SQL_SUCCESS &&
                retcode != SQL_SUCCESS_WITH_INFO)
            {
                // record was not inserted
                pDb->DispNextError();
                pDb->DispAllErrors(henv, hdbc, hstmtInsert);
                return(DB_FAILURE);
            }
        }
    }

    // Record inserted into the datasource successfully
    return(DB_SUCCESS);

}  // wxDbTable::Insert()


/********** wxDbTable::Update() **********/
bool wxDbTable::Update()
{
    exASSERT(!queryOnly);
    if (queryOnly)
        return false;

    std::wstring sqlStmt;

    // Build the SQL UPDATE statement
    BuildUpdateStmt(sqlStmt, DB_UPD_KEYFIELDS);

    pDb->WriteSqlLog(sqlStmt);

#ifdef DBDEBUG_CONSOLE
    std::wcout << std::endl << sqlStmt.c_str() << std::endl << std::endl;
#endif

    // Execute the SQL UPDATE statement
    return(execUpdate(sqlStmt));

}  // wxDbTable::Update()


/********** wxDbTable::Update(pSqlStmt) **********/
bool wxDbTable::Update(const std::wstring &pSqlStmt)
{
    exASSERT(!queryOnly);
    if (queryOnly)
        return false;

    pDb->WriteSqlLog(pSqlStmt);

    return(execUpdate(pSqlStmt));

}  // wxDbTable::Update(pSqlStmt)


/********** wxDbTable::UpdateWhere() **********/
bool wxDbTable::UpdateWhere(const std::wstring &pWhereClause)
{
    exASSERT(!queryOnly);
    if (queryOnly)
        return false;

    std::wstring sqlStmt;

    // Build the SQL UPDATE statement
    BuildUpdateStmt(sqlStmt, DB_UPD_WHERE, pWhereClause);

    pDb->WriteSqlLog(sqlStmt);

#ifdef DBDEBUG_CONSOLE
    std::wcout << std::endl << sqlStmt.c_str() << std::endl << std::endl;
#endif

    // Execute the SQL UPDATE statement
    return(execUpdate(sqlStmt));

}  // wxDbTable::UpdateWhere()


/********** wxDbTable::Delete() **********/
bool wxDbTable::Delete()
{
    exASSERT(!queryOnly);
    if (queryOnly)
        return false;

    std::wstring sqlStmt;
    sqlStmt.empty();

    // Build the SQL DELETE statement
    BuildDeleteStmt(sqlStmt, DB_DEL_KEYFIELDS);

    pDb->WriteSqlLog(sqlStmt);

    // Execute the SQL DELETE statement
    return(execDelete(sqlStmt));

}  // wxDbTable::Delete()


/********** wxDbTable::DeleteWhere() **********/
bool wxDbTable::DeleteWhere(const std::wstring &pWhereClause)
{
    exASSERT(!queryOnly);
    if (queryOnly)
        return false;

    std::wstring sqlStmt;
    sqlStmt.empty();

    // Build the SQL DELETE statement
    BuildDeleteStmt(sqlStmt, DB_DEL_WHERE, pWhereClause);

    pDb->WriteSqlLog(sqlStmt);

    // Execute the SQL DELETE statement
    return(execDelete(sqlStmt));

}  // wxDbTable::DeleteWhere()


/********** wxDbTable::DeleteMatching() **********/
bool wxDbTable::DeleteMatching()
{
    exASSERT(!queryOnly);
    if (queryOnly)
        return false;

    std::wstring sqlStmt;
    sqlStmt.empty();

    // Build the SQL DELETE statement
    BuildDeleteStmt(sqlStmt, DB_DEL_MATCHING);

    pDb->WriteSqlLog(sqlStmt);

    // Execute the SQL DELETE statement
    return(execDelete(sqlStmt));

}  // wxDbTable::DeleteMatching()


/********** wxDbTable::IsColNull() **********/
bool wxDbTable::IsColNull(UWORD colNumber) const
{
/*
    This logic is just not right.  It would indicate true
    if a numeric field were set to a value of 0.

    switch(colDefs[colNumber].SqlCtype)
    {
        case SQL_C_CHAR:
        case SQL_C_WCHAR:
        //case SQL_C_WXCHAR:  SQL_C_WXCHAR is covered by either SQL_C_CHAR or SQL_C_WCHAR
            return(((UCHAR FAR *) colDefs[colNumber].PtrDataObj)[0] == 0);
        case SQL_C_SSHORT:
            return((  *((SWORD *) colDefs[colNumber].PtrDataObj))   == 0);
        case SQL_C_USHORT:
            return((   *((UWORD*) colDefs[colNumber].PtrDataObj))   == 0);
        case SQL_C_SLONG:
            return(( *((SDWORD *) colDefs[colNumber].PtrDataObj))   == 0);
        case SQL_C_ULONG:
            return(( *((UDWORD *) colDefs[colNumber].PtrDataObj))   == 0);
        case SQL_C_FLOAT:
            return(( *((SFLOAT *) colDefs[colNumber].PtrDataObj))   == 0);
        case SQL_C_DOUBLE:
            return((*((SDOUBLE *) colDefs[colNumber].PtrDataObj))   == 0);
        case SQL_C_TIMESTAMP:
            TIMESTAMP_STRUCT *pDt;
            pDt = (TIMESTAMP_STRUCT *) colDefs[colNumber].PtrDataObj;
            if (pDt->year == 0 && pDt->month == 0 && pDt->day == 0)
                return true;
            else
                return false;
        default:
            return true;
    }
*/
    return (colDefs[colNumber].m_null);
}  // wxDbTable::IsColNull()


/********** wxDbTable::CanSelectForUpdate() **********/
bool wxDbTable::CanSelectForUpdate()
{
    if (queryOnly)
        return false;

    if (pDb->Dbms() == dbmsMY_SQL)
        return false;

    if ((pDb->Dbms() == dbmsORACLE) ||
        (pDb->dbInf.posStmts & SQL_PS_SELECT_FOR_UPDATE))
        return true;
    else
        return false;

}  // wxDbTable::CanSelectForUpdate()


/********** wxDbTable::CanUpdateByROWID() **********/
bool wxDbTable::CanUpdateByROWID()
{
/*
 * NOTE: Returning false for now until this can be debugged,
 *        as the ROWID is not getting updated correctly
 */
    return false;
/*
    if (pDb->Dbms() == dbmsORACLE)
        return true;
    else
        return false;
*/
}  // wxDbTable::CanUpdateByROWID()


/********** wxDbTable::IsCursorClosedOnCommit() **********/
bool wxDbTable::IsCursorClosedOnCommit()
{
    if (pDb->dbInf.cursorCommitBehavior == SQL_CB_PRESERVE)
        return false;
    else
        return true;

}  // wxDbTable::IsCursorClosedOnCommit()



/********** wxDbTable::ClearMemberVar() **********/
void wxDbTable::ClearMemberVar(UWORD colNumber, bool setToNull)
{
    exASSERT(colNumber < m_numCols);

    switch(colDefs[colNumber].m_SqlCtype)
    {
        case SQL_C_CHAR:
#ifdef SQL_C_WCHAR
        case SQL_C_WCHAR:
#endif
        //case SQL_C_WXCHAR:  SQL_C_WXCHAR is covered by either SQL_C_CHAR or SQL_C_WCHAR
            ((UCHAR FAR *) colDefs[colNumber].m_PtrDataObj)[0]    = 0;
            break;
        case SQL_C_SSHORT:
            *((SWORD *) colDefs[colNumber].m_PtrDataObj)          = 0;
            break;
        case SQL_C_USHORT:
            *((UWORD*) colDefs[colNumber].m_PtrDataObj)           = 0;
            break;
        case SQL_C_LONG:
        case SQL_C_SLONG:
            *((SDWORD *) colDefs[colNumber].m_PtrDataObj)         = 0;
            break;
        case SQL_C_ULONG:
            *((UDWORD *) colDefs[colNumber].m_PtrDataObj)         = 0;
            break;
        case SQL_C_FLOAT:
            *((SFLOAT *) colDefs[colNumber].m_PtrDataObj)         = 0.0f;
            break;
        case SQL_C_DOUBLE:
            *((SDOUBLE *) colDefs[colNumber].m_PtrDataObj)        = 0.0f;
            break;
        case SQL_C_TIMESTAMP:
            TIMESTAMP_STRUCT *pDt;
            pDt = (TIMESTAMP_STRUCT *) colDefs[colNumber].m_PtrDataObj;
            pDt->year = 0;
            pDt->month = 0;
            pDt->day = 0;
            pDt->hour = 0;
            pDt->minute = 0;
            pDt->second = 0;
            pDt->fraction = 0;
            break;
        case SQL_C_DATE:
            DATE_STRUCT *pDtd;
            pDtd = (DATE_STRUCT *) colDefs[colNumber].m_PtrDataObj;
            pDtd->year = 0;
            pDtd->month = 0;
            pDtd->day = 0;
            break;
        case SQL_C_TIME:
            TIME_STRUCT *pDtt;
            pDtt = (TIME_STRUCT *) colDefs[colNumber].m_PtrDataObj;
            pDtt->hour = 0;
            pDtt->minute = 0;
            pDtt->second = 0;
            break;
    }

    if (setToNull)
        SetColNull(colNumber);
}  // wxDbTable::ClearMemberVar()


/********** wxDbTable::ClearMemberVars() **********/
void wxDbTable::ClearMemberVars(bool setToNull)
{
    int i;

    // Loop through the columns setting each member variable to zero
    for (i=0; i < m_numCols; i++)
        ClearMemberVar((UWORD)i,setToNull);

}  // wxDbTable::ClearMemberVars()


/********** wxDbTable::SetQueryTimeout() **********/
bool wxDbTable::SetQueryTimeout(UDWORD nSeconds)
{
    if (SQLSetStmtOption(hstmtInsert, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
        return(pDb->DispAllErrors(henv, hdbc, hstmtInsert));
    if (SQLSetStmtOption(hstmtUpdate, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
        return(pDb->DispAllErrors(henv, hdbc, hstmtUpdate));
    if (SQLSetStmtOption(hstmtDelete, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
        return(pDb->DispAllErrors(henv, hdbc, hstmtDelete));
    if (SQLSetStmtOption(hstmtInternal, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
        return(pDb->DispAllErrors(henv, hdbc, hstmtInternal));

    // Completed Successfully
    return true;

}  // wxDbTable::SetQueryTimeout()


/********** wxDbTable::SetColDefs() **********/
bool wxDbTable::SetColDefs(UWORD index, const std::wstring &fieldName, int dataType, void *pData,
                           SWORD cType, int size, bool keyField, bool updateable,
                           bool insertAllowed, bool derivedColumn)
{
    std::wstring tmpStr;
    if (index >= m_numCols)  // Columns numbers are zero based....
    {
		tmpStr = (boost::wformat(L"Specified column index (%d) exceeds the maximum number of columns (%d) registered for this table definition.  Column definition not added.") % index % m_numCols).str();
        exFAIL_MSG(tmpStr);
        return false;
    }

    if (!colDefs)  // May happen if the database connection fails
        return false;

    if (fieldName.length() > (unsigned int) DB_MAX_COLUMN_NAME_LEN)
    {
        wcsncpy(colDefs[index].m_ColName, fieldName.c_str(), DB_MAX_COLUMN_NAME_LEN);
        colDefs[index].m_ColName[DB_MAX_COLUMN_NAME_LEN] = 0;  // Prevent buffer overrun

		tmpStr = (boost::wformat(L"Column name '%s' is too long. Truncated to '%s'.") %	fieldName % colDefs[index].m_ColName).str();
        exFAIL_MSG(tmpStr);
    }
    else
        wcscpy(colDefs[index].m_ColName, fieldName.c_str());

    colDefs[index].m_DbDataType       = dataType;
    colDefs[index].m_PtrDataObj       = pData;
    colDefs[index].m_SqlCtype         = cType;
    colDefs[index].m_szDataObj        = size;  //TODO: glt ??? * sizeof(wchar_t) ???
    colDefs[index].m_keyField         = keyField;
    colDefs[index].m_derivedCol       = derivedColumn;
    // Derived columns by definition would NOT be "Insertable" or "Updateable"
    if (derivedColumn)
    {
        colDefs[index].m_updateable       = false;
        colDefs[index].m_insertAllowed    = false;
    }
    else
    {
        colDefs[index].m_updateable       = updateable;
        colDefs[index].m_insertAllowed    = insertAllowed;
    }

    colDefs[index].m_null                 = false;

    return true;

}  // wxDbTable::SetColDefs()


/********** wxDbTable::SetColDefs() **********/
wxDbColDataPtr* wxDbTable::SetColDefs(ColumnInfo *pColInfs, UWORD numCols)
{
    exASSERT(pColInfs);
    wxDbColDataPtr *pColDataPtrs = NULL;

    if (pColInfs)
    {
        UWORD index;

        pColDataPtrs = new wxDbColDataPtr[numCols+1];

        for (index = 0; index < numCols; index++)
        {
            // Process the fields
            switch (pColInfs[index].m_dbDataType)
            {
                case DB_DATA_TYPE_VARCHAR:
                   pColDataPtrs[index].PtrDataObj = new wchar_t[pColInfs[index].m_bufferSize+(1*sizeof(wchar_t))];
                   pColDataPtrs[index].SzDataObj  = pColInfs[index].m_bufferSize+(1*sizeof(wchar_t));
                   pColDataPtrs[index].SqlCtype   = SQL_C_WXCHAR;
                   break;
                case DB_DATA_TYPE_MEMO:
                   pColDataPtrs[index].PtrDataObj = new wchar_t[pColInfs[index].m_bufferSize+(1*sizeof(wchar_t))];
                   pColDataPtrs[index].SzDataObj  = pColInfs[index].m_bufferSize+(1*sizeof(wchar_t));
                   pColDataPtrs[index].SqlCtype   = SQL_C_WXCHAR;
                   break;
                case DB_DATA_TYPE_INTEGER:
                    // Can be long or short
                    if (pColInfs[index].m_bufferSize == sizeof(long))
                    {
                      pColDataPtrs[index].PtrDataObj = new long;
                      pColDataPtrs[index].SzDataObj  = sizeof(long);
                      pColDataPtrs[index].SqlCtype   = SQL_C_SLONG;
                    }
                    else
                    {
                        pColDataPtrs[index].PtrDataObj = new short;
                        pColDataPtrs[index].SzDataObj  = sizeof(short);
                        pColDataPtrs[index].SqlCtype   = SQL_C_SSHORT;
                    }
                    break;
                case DB_DATA_TYPE_FLOAT:
                    // Can be float or double
                    if (pColInfs[index].m_bufferSize == sizeof(float))
                    {
                        pColDataPtrs[index].PtrDataObj = new float;
                        pColDataPtrs[index].SzDataObj  = sizeof(float);
                        pColDataPtrs[index].SqlCtype   = SQL_C_FLOAT;
                    }
                    else
                    {
                        pColDataPtrs[index].PtrDataObj = new double;
                        pColDataPtrs[index].SzDataObj  = sizeof(double);
                        pColDataPtrs[index].SqlCtype   = SQL_C_DOUBLE;
                    }
                    break;
                case DB_DATA_TYPE_DATE:
                    pColDataPtrs[index].PtrDataObj = new TIMESTAMP_STRUCT;
                    pColDataPtrs[index].SzDataObj  = sizeof(TIMESTAMP_STRUCT);
                    pColDataPtrs[index].SqlCtype   = SQL_C_TIMESTAMP;
                    break;
                case DB_DATA_TYPE_BLOB:
                    exFAIL_MSG(L"This form of ::SetColDefs() cannot be used with BLOB columns");
                    pColDataPtrs[index].PtrDataObj = /*BLOB ADDITION NEEDED*/NULL;
                    pColDataPtrs[index].SzDataObj  = /*BLOB ADDITION NEEDED*/sizeof(void *);
                    pColDataPtrs[index].SqlCtype   = SQL_VARBINARY;
                    break;
            }
            if (pColDataPtrs[index].PtrDataObj != NULL)
                SetColDefs (index,pColInfs[index].m_colName,pColInfs[index].m_dbDataType, pColDataPtrs[index].PtrDataObj, pColDataPtrs[index].SqlCtype, pColDataPtrs[index].SzDataObj);
            else
            {
                // Unable to build all the column definitions, as either one of
                // the calls to "new" failed above, or there was a BLOB field
                // to have a column definition for.  If BLOBs are to be used,
                // the other form of ::SetColDefs() must be used, as it is impossible
                // to know the maximum size to create the PtrDataObj to be.
                delete [] pColDataPtrs;
                return NULL;
            }
        }
    }

    return (pColDataPtrs);

} // wxDbTable::SetColDefs()


/********** wxDbTable::SetCursor() **********/
void wxDbTable::SetCursor(HSTMT *hstmtActivate)
{
    if (hstmtActivate == wxDB_DEFAULT_CURSOR)
        hstmt = *hstmtDefault;
    else
        hstmt = *hstmtActivate;

}  // wxDbTable::SetCursor()


/********** wxDbTable::Count(const std::wstring &) **********/
ULONG wxDbTable::Count(const std::wstring &args)
{
    ULONG count;
    std::wstring sqlStmt;
    SQLLEN cb;

    // Build a "SELECT COUNT(*) FROM queryTableName [WHERE whereClause]" SQL Statement
    sqlStmt  = L"SELECT COUNT(";
    sqlStmt += args;
    sqlStmt += L") FROM ";
    sqlStmt += pDb->SQLTableName(queryTableName.c_str());
//    sqlStmt += queryTableName;
    if (from.length())
        sqlStmt += from;

    // Add the where clause if one is provided
    if (where.length())
    {
        sqlStmt += L" WHERE ";
        sqlStmt += where;
    }

    pDb->WriteSqlLog(sqlStmt);

    // Initialize the Count cursor if it's not already initialized
    if (!hstmtCount)
    {
        hstmtCount = GetNewCursor(false,false);
        exASSERT(hstmtCount);
        if (!hstmtCount)
            return(0);
    }

    // Execute the SQL statement
    if (SQLExecDirect(*hstmtCount, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS) != SQL_SUCCESS)
    {
        pDb->DispAllErrors(henv, hdbc, *hstmtCount);
        return(0);
    }

    // Fetch the record
    if (SQLFetch(*hstmtCount) != SQL_SUCCESS)
    {
        pDb->DispAllErrors(henv, hdbc, *hstmtCount);
        return(0);
    }

    // Obtain the result
    if (SQLGetData(*hstmtCount, (UWORD)1, SQL_C_ULONG, &count, sizeof(count), &cb) != SQL_SUCCESS)
    {
        pDb->DispAllErrors(henv, hdbc, *hstmtCount);
        return(0);
    }

    // Free the cursor
    if (SQLFreeStmt(*hstmtCount, SQL_CLOSE) != SQL_SUCCESS)
        pDb->DispAllErrors(henv, hdbc, *hstmtCount);

    // Return the record count
    return(count);

}  // wxDbTable::Count()


/********** wxDbTable::Refresh() **********/
bool wxDbTable::Refresh()
{
    bool result = true;

    // Switch to the internal cursor so any active cursors are not corrupted
    HSTMT currCursor = GetCursor();
    hstmt = hstmtInternal;
    std::wstring saveWhere = where;
    std::wstring saveOrderBy = orderBy;

    // Build a where clause to refetch the record with.  Try and use the
    // ROWID if it's available, ow use the key fields.
    std::wstring whereClause;
    whereClause.empty();

    if (CanUpdateByROWID())
    {
        SQLLEN cb;
        wchar_t rowid[wxDB_ROWID_LEN+1];

        // Get the ROWID value.  If not successful retreiving the ROWID,
        // simply fall down through the code and build the WHERE clause
        // based on the key fields.
        if (SQLGetData(hstmt, (UWORD)(m_numCols+1), SQL_C_WXCHAR, (UCHAR*) rowid, sizeof(rowid), &cb) == SQL_SUCCESS)
        {
            whereClause += pDb->SQLTableName(queryTableName.c_str());
//            whereClause += queryTableName;
            whereClause += L".ROWID = '";
            whereClause += rowid;
            whereClause += L"'";
        }
    }

    // If unable to use the ROWID, build a where clause from the keyfields
    if (whereClause.empty())
        BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS, queryTableName);

    // Requery the record
    where = whereClause;
    orderBy.empty();
    if (!Query())
        result = false;

    if (result && !GetNext())
        result = false;

    // Switch back to original cursor
    SetCursor(&currCursor);

    // Free the internal cursor
    if (SQLFreeStmt(hstmtInternal, SQL_CLOSE) != SQL_SUCCESS)
        pDb->DispAllErrors(henv, hdbc, hstmtInternal);

    // Restore the original where and order by clauses
    where   = saveWhere;
    orderBy = saveOrderBy;

    return(result);

}  // wxDbTable::Refresh()


/********** wxDbTable::SetColNull() **********/
bool wxDbTable::SetColNull(UWORD colNumber, bool set)
{
    if (colNumber < m_numCols)
    {
        colDefs[colNumber].m_null = set;
        if (set)  // Blank out the values in the member variable
           ClearMemberVar(colNumber, false);  // Must call with false here, or infinite recursion will happen

        setCbValueForColumn(colNumber);

        return true;
    }
    else
        return false;

}  // wxDbTable::SetColNull()


/********** wxDbTable::SetColNull() **********/
bool wxDbTable::SetColNull(const std::wstring &colName, bool set)
{
    int colNumber;
    for (colNumber = 0; colNumber < m_numCols; colNumber++)
    {
        if (!_wcsicmp(colName.c_str(), colDefs[colNumber].m_ColName))
            break;
    }

    if (colNumber < m_numCols)
    {
        colDefs[colNumber].m_null = set;
        if (set)  // Blank out the values in the member variable
           ClearMemberVar((UWORD)colNumber,false);  // Must call with false here, or infinite recursion will happen

        setCbValueForColumn(colNumber);

        return true;
    }
    else
        return false;

}  // wxDbTable::SetColNull()


/********** wxDbTable::GetNewCursor() **********/
HSTMT *wxDbTable::GetNewCursor(bool setCursor, bool bindColumns)
{
    HSTMT *newHSTMT = new HSTMT;
    exASSERT(newHSTMT);
    if (!newHSTMT)
        return(0);

    if (SQLAllocStmt(hdbc, newHSTMT) != SQL_SUCCESS)
    {
        pDb->DispAllErrors(henv, hdbc);
        delete newHSTMT;
        return(0);
    }

    if (SQLSetStmtOption(*newHSTMT, SQL_CURSOR_TYPE, cursorType) != SQL_SUCCESS)
    {
        pDb->DispAllErrors(henv, hdbc, *newHSTMT);
        delete newHSTMT;
        return(0);
    }

    if (bindColumns)
    {
        if (!bindCols(*newHSTMT))
        {
            delete newHSTMT;
            return(0);
        }
    }

    if (setCursor)
        SetCursor(newHSTMT);

    return(newHSTMT);

}   // wxDbTable::GetNewCursor()


/********** wxDbTable::DeleteCursor() **********/
bool wxDbTable::DeleteCursor(HSTMT *hstmtDel)
{
    bool result = true;

    if (!hstmtDel)  // Cursor already deleted
        return(result);

/*
ODBC 3.0 says to use this form
    if (SQLFreeHandle(*hstmtDel, SQL_DROP) != SQL_SUCCESS)

*/
    if (SQLFreeStmt(*hstmtDel, SQL_DROP) != SQL_SUCCESS)
    {
        pDb->DispAllErrors(henv, hdbc);
        result = false;
    }

    delete hstmtDel;

    return(result);

}  // wxDbTable::DeleteCursor()

//////////////////////////////////////////////////////////////
// wxDbGrid support functions
//////////////////////////////////////////////////////////////

void wxDbTable::SetRowMode(const rowmode_t rowmode)
{
    if (!m_hstmtGridQuery)
    {
        m_hstmtGridQuery = GetNewCursor(false,false);
        if (!bindCols(*m_hstmtGridQuery))
            return;
    }

    m_rowmode = rowmode;
    switch (m_rowmode)
    {
        case WX_ROW_MODE_QUERY:
            SetCursor(m_hstmtGridQuery);
            break;
        case WX_ROW_MODE_INDIVIDUAL:
            SetCursor(hstmtDefault);
            break;
        default:
           exASSERT(0);
    }
}  // wxDbTable::SetRowMode()


//wxVariant wxDbTable::GetColumn(const int colNumber) const
//{
//    wxVariant val;
//    if ((colNumber < m_numCols) && (!IsColNull((UWORD)colNumber)))
//    {
//        switch (colDefs[colNumber].SqlCtype)
//        {
//#if wxUSE_UNICODE
//    #if defined(SQL_WCHAR)
//            case SQL_WCHAR:
//    #endif
//    #if defined(SQL_WVARCHAR)
//            case SQL_WVARCHAR:
//    #endif
//#endif
//            case SQL_CHAR:
//            case SQL_VARCHAR:
//                val = (wchar_t *)(colDefs[colNumber].PtrDataObj);
//                break;
//            case SQL_C_LONG:
//            case SQL_C_SLONG:
//                val = *(long *)(colDefs[colNumber].PtrDataObj);
//                break;
//            case SQL_C_SHORT:
//            case SQL_C_SSHORT:
//                val = (long int )(*(short *)(colDefs[colNumber].PtrDataObj));
//                break;
//            case SQL_C_ULONG:
//                val = (long)(*(unsigned long *)(colDefs[colNumber].PtrDataObj));
//                break;
//            case SQL_C_TINYINT:
//                val = (long)(*(wchar_t *)(colDefs[colNumber].PtrDataObj));
//                break;
//            case SQL_C_UTINYINT:
//                val = (long)(*(wchar_t *)(colDefs[colNumber].PtrDataObj));
//                break;
//            case SQL_C_USHORT:
//                val = (long)(*(UWORD *)(colDefs[colNumber].PtrDataObj));
//                break;
//            case SQL_C_DATE:
//                val = (DATE_STRUCT *)(colDefs[colNumber].PtrDataObj);
//                break;
//            case SQL_C_TIME:
//                val = (TIME_STRUCT *)(colDefs[colNumber].PtrDataObj);
//                break;
//            case SQL_C_TIMESTAMP:
//                val = (TIMESTAMP_STRUCT *)(colDefs[colNumber].PtrDataObj);
//                break;
//            case SQL_C_DOUBLE:
//                val = *(double *)(colDefs[colNumber].PtrDataObj);
//                break;
//            default:
//                assert(0);
//        }
//    }
//    return val;
//}  // wxDbTable::GetCol()


//void wxDbTable::SetColumn(const int colNumber, const wxVariant val)
//{
//    //FIXME: Add proper wxDateTime support to wxVariant..
//    wxDateTime dateval;
//
//    SetColNull((UWORD)colNumber, val.IsNull());
//
//    if (!val.IsNull())
//    {
//        if ((colDefs[colNumber].SqlCtype == SQL_C_DATE)
//            || (colDefs[colNumber].SqlCtype == SQL_C_TIME)
//            || (colDefs[colNumber].SqlCtype == SQL_C_TIMESTAMP))
//        {
//            //Returns null if invalid!
//            if (!dateval.ParseDate(val.GetString()))
//                SetColNull((UWORD)colNumber, true);
//        }
//
//        switch (colDefs[colNumber].SqlCtype)
//        {
//#if wxUSE_UNICODE
//    #if defined(SQL_WCHAR)
//            case SQL_WCHAR:
//    #endif
//    #if defined(SQL_WVARCHAR)
//            case SQL_WVARCHAR:
//    #endif
//#endif
//            case SQL_CHAR:
//            case SQL_VARCHAR:
//                csstrncpyt((wchar_t *)(colDefs[colNumber].PtrDataObj),
//                           val.GetString().c_str(),
//                           colDefs[colNumber].SzDataObj-1);  //TODO: glt ??? * sizeof(wchar_t) ???
//                break;
//            case SQL_C_LONG:
//            case SQL_C_SLONG:
//                *(long *)(colDefs[colNumber].PtrDataObj) = val;
//                break;
//            case SQL_C_SHORT:
//            case SQL_C_SSHORT:
//                *(short *)(colDefs[colNumber].PtrDataObj) = (short)val.GetLong();
//                break;
//            case SQL_C_ULONG:
//                *(unsigned long *)(colDefs[colNumber].PtrDataObj) = val.GetLong();
//                break;
//            case SQL_C_TINYINT:
//                *(wchar_t *)(colDefs[colNumber].PtrDataObj) = val.GetChar();
//                break;
//            case SQL_C_UTINYINT:
//                *(wchar_t *)(colDefs[colNumber].PtrDataObj) = val.GetChar();
//                break;
//            case SQL_C_USHORT:
//                *(unsigned short *)(colDefs[colNumber].PtrDataObj) = (unsigned short)val.GetLong();
//                break;
//            //FIXME: Add proper wxDateTime support to wxVariant..
//            case SQL_C_DATE:
//                {
//                    DATE_STRUCT *dataptr =
//                        (DATE_STRUCT *)colDefs[colNumber].PtrDataObj;
//
//                    dataptr->year   = (SWORD)dateval.GetYear();
//                    dataptr->month  = (UWORD)(dateval.GetMonth()+1);
//                    dataptr->day    = (UWORD)dateval.GetDay();
//                }
//                break;
//            case SQL_C_TIME:
//                {
//                    TIME_STRUCT *dataptr =
//                        (TIME_STRUCT *)colDefs[colNumber].PtrDataObj;
//
//                    dataptr->hour   = dateval.GetHour();
//                    dataptr->minute = dateval.GetMinute();
//                    dataptr->second = dateval.GetSecond();
//                }
//                break;
//            case SQL_C_TIMESTAMP:
//                {
//                    TIMESTAMP_STRUCT *dataptr =
//                        (TIMESTAMP_STRUCT *)colDefs[colNumber].PtrDataObj;
//                    dataptr->year   = (SWORD)dateval.GetYear();
//                    dataptr->month  = (UWORD)(dateval.GetMonth()+1);
//                    dataptr->day    = (UWORD)dateval.GetDay();
//
//                    dataptr->hour   = dateval.GetHour();
//                    dataptr->minute = dateval.GetMinute();
//                    dataptr->second = dateval.GetSecond();
//                }
//                break;
//            case SQL_C_DOUBLE:
//                *(double *)(colDefs[colNumber].PtrDataObj) = val;
//                break;
//            default:
//                assert(0);
//        }  // switch
//    }  // if (!val.IsNull())
//}  // wxDbTable::SetCol()


GenericKey wxDbTable::GetKey()
{
    void *blk;
    wchar_t *blkptr;

    blk = malloc(m_keysize);
    blkptr = (wchar_t *) blk;

    int i;
    for (i=0; i < m_numCols; i++)
    {
        if (colDefs[i].m_keyField)
        {
            memcpy(blkptr,colDefs[i].m_PtrDataObj, colDefs[i].m_szDataObj);
            blkptr += colDefs[i].m_szDataObj;
        }
    }

    GenericKey k = GenericKey(blk, m_keysize);
    free(blk);

    return k;
}  // wxDbTable::GetKey()


void wxDbTable::SetKey(const GenericKey& k)
{
    void    *blk;
    wchar_t  *blkptr;

    blk = k.GetBlk();
    blkptr = (wchar_t *)blk;

    int i;
    for (i=0; i < m_numCols; i++)
    {
        if (colDefs[i].m_keyField)
        {
            SetColNull((UWORD)i, false);
            memcpy(colDefs[i].m_PtrDataObj, blkptr, colDefs[i].m_szDataObj);
            blkptr += colDefs[i].m_szDataObj;
        }
    }
}  // wxDbTable::SetKey()

}