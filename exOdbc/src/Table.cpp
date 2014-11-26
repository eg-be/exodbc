/*!
* \file Table.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 25.07.2014
* \brief Source file for the Table class and its helpers.
*
* Header file for the Table class and its helpers.
* This file was originally wx/dbtable.cpp from wxWidgets 2.8.
* Most of the code has been rewritten, a lot of functionality
* not needed and not tested so far has been droped.
*
* For completion, here follows the old wxWidgets header:
*
* ///////////////////////////////////////////////////////////////////////////////<br>
* // Name:        src/common/dbtable.cpp<br>
* // Purpose:     Implementation of the wxDbTable class.<br>
* // Author:      Doug Card<br>
* // Modified by: George Tasker<br>
* //              Bart Jourquin<br>
* //              Mark Johnson<br>
* // Created:     9.96<br>
* // RCS-ID:      $Id: dbtable.cpp 48685 2007-09-14 19:02:28Z VZ $<br>
* // Copyright:   (c) 1996 Remstar International, Inc.<br>
* // Licence:     wxWindows licence<br>
* ///////////////////////////////////////////////////////////////////////////////<br>
*/

#include "stdafx.h"

// Own header
#include "Table.h"

// Same component headers
#include "Helpers.h"
#include "ColumnBuffer.h"

// Same component headers

namespace exodbc
{


	/********** wxDbColDef::wxDbColDef() Constructor **********/
	ColumnDefinition::ColumnDefinition()
	{
		Initialize();
	}  // Constructor


	bool ColumnDefinition::Initialize()
	{
		m_colName[0] = 0;
		m_dbDataType = DB_DATA_TYPE_INTEGER;
		m_sqlCtype = SQL_C_LONG;
		m_ptrDataObj = NULL;
		m_szDataObj = 0;
		m_keyField = false;
		m_updateable = false;
		m_insertAllowed = false;
		m_derivedCol = false;
		m_cbValue = 0;
		m_null = false;

		return true;
	}  // wxDbColDef::Initialize()

	// Construction
	// ------------
	Table::Table(Database* pDb, size_t numColumns, const std::wstring& tableName, const std::wstring& schemaName /* = L"" */, const std::wstring& catalogName /* = L"" */, const std::wstring& tableType /* = L"" */, OpenMode openMode /* = READ_WRITE */)
		: m_numCols(numColumns)
		, m_manualColumns(true)
		, m_initialTableName(tableName)
		, m_initialSchemaName(schemaName)
		, m_initialCatalogName(catalogName)
		, m_initialTypeName(tableType)
		, m_openMode(openMode)
		, m_haveTableInfo(false)
	{
		exASSERT(pDb);
		if (!Initialize(pDb))
		{
			cleanup();
		}
	}



	Table::Table(Database* pDb, size_t numColumns, const STableInfo& tableInfo, OpenMode openMode /* = READ_WRITE */)
		: m_numCols(numColumns)
		, m_manualColumns(true)
		, m_initialTableName(L"")
		, m_initialSchemaName(L"")
		, m_initialCatalogName(L"")
		, m_initialTypeName(L"")
		, m_openMode(openMode)
		, m_haveTableInfo(true)
		, m_tableInfo(tableInfo)
	{
		exASSERT(pDb);
		if (!Initialize(pDb))
		{
			cleanup();
		}
	}



	Table::Table(Database* pDb, const std::wstring& tableName, const std::wstring& schemaName /* = L"" */, const std::wstring& catalogName /* = L"" */, const std::wstring& tableType /* = L"" */, const OpenMode openMode /* = READ_WRITE */)
		: m_numCols(0)
		, m_manualColumns(false)
		, m_initialTableName(tableName)
		, m_initialSchemaName(schemaName)
		, m_initialCatalogName(catalogName)
		, m_initialTypeName(tableType)
		, m_openMode(openMode)
		, m_haveTableInfo(false)
	{
		exASSERT(pDb);
		if (!Initialize(pDb))
		{
			cleanup();
		}
	}



	Table::Table(Database* pDb, const STableInfo& tableInfo, OpenMode openMode /* = READ_WRITE */)
		: m_numCols(0)
		, m_manualColumns(false)
		, m_initialTableName(L"")
		, m_initialSchemaName(L"")
		, m_initialCatalogName(L"")
		, m_initialTypeName(L"")
		, m_openMode(openMode)
		, m_haveTableInfo(true)
		, m_tableInfo(tableInfo)
	{
		exASSERT(pDb);
		if (!Initialize(pDb))
		{
			cleanup();
		}
	}


	// Destructor
	// -----------
	Table::~Table()
	{
		this->cleanup();
	}

	// Implementation
	// --------------
	bool Table::Initialize(Database* pDb)
	{
		exASSERT(pDb);
		if (!pDb)
		{
			LOG_ERROR(L"Database is required");
			return false;
		}

		// Initializing member variables
		// Note: m_haveTableInfo must have been set before this function is called - it is not modified by this Initialize
		m_isOpen = false;

		m_pDb = pDb;                    // Pointer to the wxDb object
		m_hStmtCount = SQL_NULL_HSTMT;
		m_hStmtSelect = SQL_NULL_HSTMT;
		m_selectQueryOpen = false;
		m_fieldsStatement = L"";

		// Old handles
		m_hdbc = SQL_NULL_HDBC;
		m_hstmt = SQL_NULL_HSTMT;
		m_hstmtDefault = SQL_NULL_HSTMT;                        // Initialized below
//		m_hstmtCount = SQL_NULL_HSTMT;                        // Initialized first time it is needed
		m_hstmtInsert = SQL_NULL_HSTMT;
		m_hstmtDelete = SQL_NULL_HSTMT;
		m_hstmtUpdate = SQL_NULL_HSTMT;
		m_hstmtInternal = SQL_NULL_HSTMT;
		m_colDefs = 0;	///< TODO: Change
		m_where.empty();                                  // Where clause
		m_orderBy.empty();                                // Order By clause
		m_from.empty();                                   // From clause
		m_selectForUpdate = false;                    // SELECT ... FOR UPDATE; Indicates whether to include the FOR UPDATE phrase

		// TODO: The old code automatically turned table names into upper case for some dbs, does that make sense?

		std::wstring s;

#ifdef EXODBCDEBUG
		m_tableId = m_pDb->RegisterTable(this);
#endif

		//    m_pDb->WriteSqlLog(s);

		// Grab the HENV and HDBC from the wxDb object
		m_hdbc = m_pDb->GetHDBC();

		// Allocate space for column definitions
		if (m_numCols)
			m_colDefs = new ColumnDefinition[m_numCols];  // Points to the first column definition

		// Allocate handles needed
		m_hStmtCount = AllocateStatement();
		m_hStmtSelect = AllocateStatement();
		if (!(m_hStmtSelect && m_hStmtCount))
		{
			return false;
		}

		// Old handle-stuff
		// Allocate statement handles for the table
		if (!IsQueryOnly())
		{
			// Allocate a separate statement handle for performing inserts
			if (SQLAllocStmt(m_hdbc, &m_hstmtInsert) != SQL_SUCCESS)
				m_pDb->DispAllErrors(NULL, m_hdbc);
			// Allocate a separate statement handle for performing deletes
			if (SQLAllocStmt(m_hdbc, &m_hstmtDelete) != SQL_SUCCESS)
				m_pDb->DispAllErrors(NULL, m_hdbc);
			// Allocate a separate statement handle for performing updates
			if (SQLAllocStmt(m_hdbc, &m_hstmtUpdate) != SQL_SUCCESS)
				m_pDb->DispAllErrors(NULL, m_hdbc);
		}
		// Allocate a separate statement handle for internal use
		if (SQLAllocStmt(m_hdbc, &m_hstmtInternal) != SQL_SUCCESS)
			m_pDb->DispAllErrors(NULL, m_hdbc);

		// Set the cursor type for the statement handles
		m_cursorType = SQL_CURSOR_STATIC;

		if (SQLSetStmtOption(m_hstmtInternal, SQL_CURSOR_TYPE, m_cursorType) != SQL_SUCCESS)
		{
			//        // Check to see if cursor type is supported
			//        m_pDb->GetNextError(m_henv, m_hdbc, m_hstmtInternal);
			//        if (! wcscmp(m_pDb->sqlState, L"01S02"))  // Option Value Changed
			//        {
			//            // Datasource does not support static cursors.  Driver
			//            // will substitute a cursor type.  Call SQLGetStmtOption()
			//            // to determine which cursor type was selected.
			//            if (SQLGetStmtOption(m_hstmtInternal, SQL_CURSOR_TYPE, &m_cursorType) != SQL_SUCCESS)
			//                m_pDb->DispAllErrors(m_henv, m_hdbc, m_hstmtInternal);
			//#ifdef DBDEBUG_CONSOLE
			//            std::wcout << L"Static cursor changed to: ";
			//            switch(m_cursorType)
			//            {
			//            case SQL_CURSOR_FORWARD_ONLY:
			//                std::wcout << L"Forward Only";
			//                break;
			//            case SQL_CURSOR_STATIC:
			//                std::wcout << L"Static";
			//                break;
			//            case SQL_CURSOR_KEYSET_DRIVEN:
			//                std::wcout << L"Keyset Driven";
			//                break;
			//            case SQL_CURSOR_DYNAMIC:
			//                std::wcout << L"Dynamic";
			//                break;
			//            }
			//            std::wcout << std::endl << std::endl;
			//#endif
			//            // BJO20000425
			//            if (m_pDb->FwdOnlyCursors() && m_cursorType != SQL_CURSOR_FORWARD_ONLY)
			//            {
			//                // Force the use of a forward only cursor...
			//                m_cursorType = SQL_CURSOR_FORWARD_ONLY;
			//                if (SQLSetStmtOption(m_hstmtInternal, SQL_CURSOR_TYPE, m_cursorType) != SQL_SUCCESS)
			//                {
			//                    // Should never happen
			//                    m_pDb->GetNextError(m_henv, m_hdbc, m_hstmtInternal);
			//                    return false;
			//                }
			//            }
			//        }
			//        else
			//        {
			//            m_pDb->DispNextError();
			//            m_pDb->DispAllErrors(m_henv, m_hdbc, m_hstmtInternal);
			//        }
		}
#ifdef DBDEBUG_CONSOLE
		else
			std::wcout << L"Cursor Type set to STATIC" << std::endl << std::endl;
#endif

		if (!IsQueryOnly())
		{
			// Set the cursor type for the INSERT statement handle
			if (SQLSetStmtOption(m_hstmtInsert, SQL_CURSOR_TYPE, SQL_CURSOR_FORWARD_ONLY) != SQL_SUCCESS)
				m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert);
			// Set the cursor type for the DELETE statement handle
			if (SQLSetStmtOption(m_hstmtDelete, SQL_CURSOR_TYPE, SQL_CURSOR_FORWARD_ONLY) != SQL_SUCCESS)
				m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtDelete);
			// Set the cursor type for the UPDATE statement handle
			if (SQLSetStmtOption(m_hstmtUpdate, SQL_CURSOR_TYPE, SQL_CURSOR_FORWARD_ONLY) != SQL_SUCCESS)
				m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtUpdate);
		}

		// Make the default cursor the active cursor
		m_hstmtDefault = GetNewCursor(false, false);
		exASSERT(m_hstmtDefault);
		m_hstmt = *m_hstmtDefault;

		return true;

	}  // wxDbTable::initialize()


	HSTMT Table::AllocateStatement()
	{
		exASSERT(m_pDb);
		exASSERT(m_pDb->IsOpen());
		exASSERT(m_pDb->GetHDBC());

		// Allocate a statement handle for the database connection
		HSTMT stmt;
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_STMT, m_pDb->GetHDBC(), &stmt);
		if (ret != SQL_SUCCESS)
		{
			// Note: SQLAllocHandle will set the output-handle to SQL_NULL_HDBC, SQL_NULL_HSTMT, or SQL_NULL_HENV in case of failure
			LOG_ERROR_DBC(m_hdbc, ret, SQLAllocHandle);
		}
		return stmt;
	}


	bool Table::FreeStatement(HSTMT hStmt)
	{
		// Free statement handle
		SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
		if (ret != SQL_SUCCESS)
		{
			// if SQL_ERROR is returned, the handle is still valid, error information can be fetched
			if (ret == SQL_ERROR)
			{
				LOG_ERROR_STMT(hStmt, ret, SqlFreeHandle);
			}
			else
			{
				LOG_ERROR_SQL_NO_SUCCESS(ret, SQLFreeHandle);
			}
		}
		else
		{
			hStmt = SQL_NULL_HSTMT;
		}
		return ret == SQL_SUCCESS;
	}


	std::wstring Table::BuildFieldsStatement() const
	{
		exASSERT(m_columnBuffers.size() > 0);

		std::wstring fields = L"";
		std::vector<ColumnBuffer*>::const_iterator it = m_columnBuffers.begin();
		while (it != m_columnBuffers.end())
		{
			fields += (*it)->GetColumnInfo().GetSqlName();
			++it;
			if (it != m_columnBuffers.end())
			{
				fields += L", ";
			}
		}

		return fields;
	}


	void Table::cleanup()
	{
		std::wstring s;

#ifdef EXODBCDEBUG
		m_pDb->UnregisterTable(this);
#endif
		// Delete ColumnBuffers
		std::vector<ColumnBuffer*>::iterator it;
		for (it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
		{
			delete (*it);
		}
		m_columnBuffers.clear();

		// Old Column stuff
		// Delete memory allocated for column definitions
		if (m_colDefs)
			delete[] m_colDefs;

		// Free allocated statements
		FreeStatement(m_hStmtCount);
		FreeStatement(m_hStmtSelect);

		// Old statement stuff
		// Free statement handles
		if (!IsQueryOnly())
		{
			if (m_hstmtInsert)
			{
				/*
				ODBC 3.0 says to use this form
				if (SQLFreeHandle(*hstmtDel, SQL_DROP) != SQL_SUCCESS)
				*/
				if (SQLFreeStmt(m_hstmtInsert, SQL_DROP) != SQL_SUCCESS)
					m_pDb->DispAllErrors(NULL, m_hdbc);
			}

			if (m_hstmtDelete)
			{
				/*
				ODBC 3.0 says to use this form
				if (SQLFreeHandle(*hstmtDel, SQL_DROP) != SQL_SUCCESS)
				*/
				if (SQLFreeStmt(m_hstmtDelete, SQL_DROP) != SQL_SUCCESS)
					m_pDb->DispAllErrors(NULL, m_hdbc);
			}

			if (m_hstmtUpdate)
			{
				/*
				ODBC 3.0 says to use this form
				if (SQLFreeHandle(*hstmtDel, SQL_DROP) != SQL_SUCCESS)
				*/
				if (SQLFreeStmt(m_hstmtUpdate, SQL_DROP) != SQL_SUCCESS)
					m_pDb->DispAllErrors(NULL, m_hdbc);
			}
		}

		if (m_hstmtInternal)
		{
			if (SQLFreeStmt(m_hstmtInternal, SQL_DROP) != SQL_SUCCESS)
				m_pDb->DispAllErrors(NULL, m_hdbc);
		}

		// Delete dynamically allocated cursors
		if (m_hstmtDefault)
			DeleteCursor(m_hstmtDefault);

		//if (m_hstmtCount)
		//	DeleteCursor(m_hstmtCount);

	}  // wxDbTable::cleanup()


	STableInfo Table::GetTableInfo() const
	{
		exASSERT(m_haveTableInfo);
		return m_tableInfo;
	}


	bool Table::Count(const std::wstring& whereStatement, size_t& count)
	{
		exASSERT(IsOpen());
		bool ok = false;
		bool isNull = false;
		std::wstring sqlstmt;
		if ( ! whereStatement.empty())
		{
			sqlstmt = (boost::wformat(L"SELECT COUNT(*) FROM %s WHERE %s") % m_tableInfo.GetSqlName() % whereStatement).str();
		}
		else
		{
			sqlstmt = (boost::wformat(L"SELECT COUNT(*) FROM %s") % m_tableInfo.GetSqlName()).str();
		}
		SQLRETURN ret = SQLExecDirect(m_hStmtCount, (SQLWCHAR*) sqlstmt.c_str(), SQL_NTS);
		if (ret != SQL_SUCCESS)
		{
			LOG_ERROR_DBC(m_pDb->GetHDBC(), ret, SQLExecDirect);
			LOG_ERROR_STMT(m_hStmtCount, ret, SQLExecDirect);
		}
		else
		{
			ret = SQLFetch(m_hStmtCount);
			if (ret != SQL_SUCCESS)
			{
				LOG_ERROR_STMT(m_hStmtCount, ret, SQLFetch);
			}
			else
			{
				SQLINTEGER cb;
				ok = GetData(m_hStmtCount, 1, SQL_C_ULONG, &count, sizeof(count), &cb, &isNull);
				if (ok && isNull)
				{
					LOG_ERROR((boost::wformat(L"Read Value for '%s' is NULL") % sqlstmt).str());
				}
			}
		}

		// Close the Cursor on the internal statement-handle
		CloseStmtHandle(m_hStmtCount, IgnoreNotOpen);

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_pDb->GetCommitMode() != CM_AUTO_COMMIT && !m_pDb->CommitTrans())
		{
			LOG_WARNING(L"Autocommit is off, the extra-call to CommitTrans before changing the Transaction Isolation Mode failed");
		}

		return ok && !isNull;
	}


	bool Table::Select(const std::wstring& whereStatement)
	{
		exASSERT(IsOpen());

		if (IsSelectOpen())
		{
			exASSERT(CloseStmtHandle(m_hStmtSelect, FailIfNotOpen));
			m_selectQueryOpen = false;
		}
		exDEBUG(EnsureStmtIsClosed(m_hStmtSelect, m_pDb->Dbms()));

		bool ok = false;
		std::wstring sqlstmt;
		if (!whereStatement.empty())
		{
			sqlstmt = (boost::wformat(L"SELECT * FROM %s WHERE %s") % m_tableInfo.GetSqlName() % whereStatement).str();
		}
		else
		{
			sqlstmt = (boost::wformat(L"SELECT * FROM %s") % m_tableInfo.GetSqlName()).str();
		}
		SQLRETURN ret = SQLExecDirect(m_hStmtSelect, (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		if (ret != SQL_SUCCESS)
		{
			LOG_ERROR_DBC(m_pDb->GetHDBC(), ret, SQLExecDirect);
			LOG_ERROR_STMT(m_hStmtSelect, ret, SQLExecDirect);
		}
		else
		{
			m_selectQueryOpen = true;
			int row = -1;
			while ((ret = SQLFetch(m_hStmtSelect)) == SQL_SUCCESS)
			{
				row++;
				if (ret != SQL_SUCCESS)
				{
					LOG_ERROR_STMT(m_hStmtSelect, ret, SQLFetch);
				}
				else
				{
//					for (int i = 0; i < m_columnBuffers.size(); i++)
					std::vector<ColumnBuffer*>::const_iterator it;
					int col = 0;
					for (it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
					{
						ColumnBuffer* pBuff = *it;
						SQLSMALLINT si = 0;
						SQLINTEGER i = 0;
						SQLBIGINT bi = 0;
						if (!pBuff->IsNull() && pBuff->IsSmallInt())
						{
							si = pBuff->GetSmallInt();
						}
						else if (!pBuff->IsNull() && pBuff->IsInt())
						{
							i = pBuff->GetInt();
						}
						else if (!pBuff->IsNull() && pBuff->IsBigInt())
						{
							bi = pBuff->GetBigInt();
						}
						col++;
						//int val = m_columnBuffers[i].GetInt();
						//bool isNull = m_columnBuffers[i].IsNull();
						//if (!isNull)
						//{
						//	int p = 3;
						//}
						//int p = 3;
					}
					ok = true;
					SQLINTEGER cb;
					//ok = GetData(m_hStmtCount, 1, SQL_C_ULONG, &count, sizeof(count), &cb, &isNull);
					//if (ok && isNull)
					//{
					//	LOG_ERROR((boost::wformat(L"Read Value for '%s' is NULL") % sqlstmt).str());
					//}
				}
			}
			if (ret != SQL_NO_DATA)
			{
				LOG_ERROR_EXPECTED_SQL_NO_DATA(ret, SQLFetch);
			}
		}

		// Close the Cursor on the internal statement-handle
		CloseStmtHandle(m_hStmtSelect, IgnoreNotOpen);

		// If Auto commit is off, we need to commit on certain db-systems, see #51
		if (m_pDb->GetCommitMode() != CM_AUTO_COMMIT && !m_pDb->CommitTrans())
		{
			LOG_WARNING(L"Autocommit is off, the extra-call to CommitTrans before changing the Transaction Isolation Mode failed");
		}

		return ok;
	}


	//bool Table::SelectClose(CloseMode closeMode)
	//{
	//	
	//}


	/***************************** PRIVATE FUNCTIONS *****************************/



	void Table::setCbValueForColumn(int columnIndex)
	{
		switch (m_colDefs[columnIndex].m_dbDataType)
		{
		case DB_DATA_TYPE_VARCHAR:
		case DB_DATA_TYPE_MEMO:
			if (m_colDefs[columnIndex].m_null)
				m_colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
			else
				m_colDefs[columnIndex].m_cbValue = SQL_NTS;
			break;
		case DB_DATA_TYPE_INTEGER:
			if (m_colDefs[columnIndex].m_null)
				m_colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
			else
				m_colDefs[columnIndex].m_cbValue = 0;
			break;
		case DB_DATA_TYPE_FLOAT:
			if (m_colDefs[columnIndex].m_null)
				m_colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
			else
				m_colDefs[columnIndex].m_cbValue = 0;
			break;
		case DB_DATA_TYPE_DATE:
			if (m_colDefs[columnIndex].m_null)
				m_colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
			else
				m_colDefs[columnIndex].m_cbValue = 0;
			break;
		case DB_DATA_TYPE_BLOB:
			if (m_colDefs[columnIndex].m_null)
				m_colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
			else
				if (m_colDefs[columnIndex].m_sqlCtype == SQL_C_WXCHAR)
					m_colDefs[columnIndex].m_cbValue = SQL_NTS;
				else
					m_colDefs[columnIndex].m_cbValue = SQL_LEN_DATA_AT_EXEC(m_colDefs[columnIndex].m_szDataObj);
			break;
		}
	}

	/********** wxDbTable::bindParams() **********/
	//bool Table::bindParams(bool forUpdate)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    SWORD   fSqlType    = 0;
	//    SDWORD  precision   = 0;
	//    SWORD   scale       = 0;
	//
	//    // Bind each column of the table that should be bound
	//    // to a parameter marker
	//    int i;
	//    UWORD colNumber;
	//
	//    for (i=0, colNumber=1; i < m_numCols; i++)
	//    {
	//        if (forUpdate)
	//        {
	//            if (!m_colDefs[i].m_updateable)
	//                continue;
	//        }
	//        else
	//        {
	//            if (!m_colDefs[i].m_insertAllowed)
	//                continue;
	//        }
	//
	//        switch(m_colDefs[i].m_dbDataType)
	//        {
	//            case DB_DATA_TYPE_VARCHAR:
	//                fSqlType = m_pDb->GetTypeInfVarchar().FsqlType;
	//                precision = m_colDefs[i].m_szDataObj;
	//                scale = 0;
	//                break;
	//            case DB_DATA_TYPE_MEMO:
	//                fSqlType = m_pDb->GetTypeInfMemo().FsqlType;
	//                precision = m_colDefs[i].m_szDataObj;
	//                scale = 0;
	//                break;
	//            case DB_DATA_TYPE_INTEGER:
	//                fSqlType = m_pDb->GetTypeInfInteger().FsqlType;
	//                precision = m_pDb->GetTypeInfInteger().Precision;
	//                scale = 0;
	//                break;
	//            case DB_DATA_TYPE_FLOAT:
	//                fSqlType = m_pDb->GetTypeInfFloat().FsqlType;
	//                precision = m_pDb->GetTypeInfFloat().Precision;
	//                scale = m_pDb->GetTypeInfFloat().MaximumScale;
	//                // SQL Sybase Anywhere v5.5 returned a negative number for the
	//                // MaxScale.  This caused ODBC to kick out an error on ibscale.
	//                // I check for this here and set the scale = precision.
	//                //if (scale < 0)
	//                // scale = (short) precision;
	//                break;
	//            case DB_DATA_TYPE_DATE:
	//                fSqlType = m_pDb->GetTypeInfDate().FsqlType;
	//                precision = m_pDb->GetTypeInfDate().Precision;
	//                scale = 0;
	//                break;
	//            case DB_DATA_TYPE_BLOB:
	//                fSqlType = m_pDb->GetTypeInfBlob().FsqlType;
	//                precision = m_colDefs[i].m_szDataObj;
	//                scale = 0;
	//                break;
	//        }
	//
	//        setCbValueForColumn(i);
	//
	//        if (forUpdate)
	//        {
	//            if (SQLBindParameter(m_hstmtUpdate, colNumber++, SQL_PARAM_INPUT, m_colDefs[i].m_sqlCtype,
	//                                 fSqlType, precision, scale, (UCHAR*) m_colDefs[i].m_ptrDataObj,
	//                                 precision+1, &m_colDefs[i].m_cbValue) != SQL_SUCCESS)
	//            {
	//                return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtUpdate));
	//            }
	//        }
	//        else
	//        {
	//            if (SQLBindParameter(m_hstmtInsert, colNumber++, SQL_PARAM_INPUT, m_colDefs[i].m_sqlCtype,
	//                                 fSqlType, precision, scale, (UCHAR*) m_colDefs[i].m_ptrDataObj,
	//                                 precision+1, &m_colDefs[i].m_cbValue) != SQL_SUCCESS)
	//            {
	//                return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert));
	//            }
	//        }
	//    }
	//
	//    // Completed successfully
	//    return true;
	//
	//}  // wxDbTable::bindParams()


	/********** wxDbTable::bindInsertParams() **********/
	//bool Table::bindInsertParams()
	//{
	//    return bindParams(false);
	//}  // wxDbTable::bindInsertParams()
	//
	//
	///********** wxDbTable::bindUpdateParams() **********/
	//bool Table::bindUpdateParams()
	//{
	//    return bindParams(true);
	//}  // wxDbTable::bindUpdateParams()


	/********** wxDbTable::bindCols() **********/
	bool Table::bindCols(HSTMT cursor)
	{
		// Bind each column of the table to a memory address for fetching data
		UWORD i;
		for (i = 0; i < m_numCols; i++)
		{
			ColumnDefinition def = m_colDefs[i];
			if (SQLBindCol(cursor, (UWORD)(i + 1), m_colDefs[i].m_sqlCtype, (UCHAR*)m_colDefs[i].m_ptrDataObj,
				m_colDefs[i].m_szDataObj, &m_colDefs[i].m_cbValue) != SQL_SUCCESS)
				return (m_pDb->DispAllErrors(NULL, m_hdbc, cursor));
		}

		// Completed successfully
		return true;
	}  // wxDbTable::bindCols()


	/********** wxDbTable::getRec() **********/
	bool Table::getRec(UWORD fetchType)
	{
		RETCODE retcode;

		if (!m_pDb->FwdOnlyCursors())
		{
			// Fetch the NEXT, PREV, FIRST or LAST record, depending on fetchType
			SQLULEN cRowsFetched;
			UWORD   rowStatus;

			retcode = SQLExtendedFetch(m_hstmt, fetchType, 0, &cRowsFetched, &rowStatus);
			if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
			{
				if (retcode == SQL_NO_DATA_FOUND)
					return false;
				else
					return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt));
			}
			else
			{
				// Set the Null member variable to indicate the Null state
				// of each column just read in.
				size_t i;
				for (i = 0; i < m_numCols; i++)
					m_colDefs[i].m_null = (m_colDefs[i].m_cbValue == SQL_NULL_DATA);
			}
		}
		else
		{
			// Fetch the next record from the record set
			retcode = SQLFetch(m_hstmt);
			if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
			{
				if (retcode == SQL_NO_DATA_FOUND)
					return false;
				else
					return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt));
			}
			else
			{
				// Set the Null member variable to indicate the Null state
				// of each column just read in.
				size_t i;
				for (i = 0; i < m_numCols; i++)
					m_colDefs[i].m_null = (m_colDefs[i].m_cbValue == SQL_NULL_DATA);
			}
		}

		// Completed successfully
		return true;

	}  // wxDbTable::getRec()


	/********** wxDbTable::execDelete() **********/
	bool Table::execDelete(const std::wstring &pSqlStmt)
	{
		RETCODE retcode;

		// Execute the DELETE statement
		retcode = SQLExecDirect(m_hstmtDelete, (SQLWCHAR*) pSqlStmt.c_str(), SQL_NTS);

		if (retcode == SQL_SUCCESS ||
			retcode == SQL_NO_DATA_FOUND ||
			retcode == SQL_SUCCESS_WITH_INFO)
		{
			// Record deleted successfully
			return true;
		}

		// Problem deleting record
		return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtDelete));

	}  // wxDbTable::execDelete()


	///********** wxDbTable::execUpdate() **********/
	//bool Table::execUpdate(const std::wstring &pSqlStmt)
	//{
	//    RETCODE retcode;
	//
	//    // Execute the UPDATE statement
	//    retcode = SQLExecDirect(m_hstmtUpdate, (SQLTCHAR FAR *) pSqlStmt.c_str(), SQL_NTS);
	//
	//    if (retcode == SQL_SUCCESS ||
	//        retcode == SQL_NO_DATA_FOUND ||
	//        retcode == SQL_SUCCESS_WITH_INFO)
	//    {
	//        // Record updated successfully
	//        return true;
	//    }
	//    else if (retcode == SQL_NEED_DATA)
	//    {
	//        PTR pParmID;
	//        retcode = SQLParamData(m_hstmtUpdate, &pParmID);
	//        while (retcode == SQL_NEED_DATA)
	//        {
	//            // Find the parameter
	//            int i;
	//            for (i=0; i < m_numCols; i++)
	//            {
	//                if (m_colDefs[i].m_ptrDataObj == pParmID)
	//                {
	//                    // We found it.  Store the parameter.
	//                    retcode = SQLPutData(m_hstmtUpdate, pParmID, m_colDefs[i].m_szDataObj);
	//                    if (retcode != SQL_SUCCESS)
	//                    {
	//                        m_pDb->DispNextError();
	//                        return m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtUpdate);
	//                    }
	//                    break;
	//                }
	//            }
	//            retcode = SQLParamData(m_hstmtUpdate, &pParmID);
	//        }
	//        if (retcode == SQL_SUCCESS ||
	//            retcode == SQL_NO_DATA_FOUND ||
	//            retcode == SQL_SUCCESS_WITH_INFO)
	//        {
	//            // Record updated successfully
	//            return true;
	//        }
	//    }
	//
	//    // Problem updating record
	//    return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtUpdate));
	//
	//}  // wxDbTable::execUpdate()


	// ********** wxDbTable::query() **********/
	bool Table::query(int queryType, bool forUpdate, bool distinct, const std::wstring &pSqlStmt)
	{
		std::wstring sqlStmt;

		if (forUpdate)
			// The user may wish to select for update, but the DBMS may not be capable
			m_selectForUpdate = CanSelectForUpdate();
		else
			m_selectForUpdate = false;

		// Set the SQL SELECT string
		if (queryType != DB_SELECT_STATEMENT)               // A select statement was not passed in,
		{                                                   // so generate a select statement.
			exASSERT(false);
			//        BuildSelectStmt(sqlStmt, queryType, distinct);
			//        m_pDb->WriteSqlLog(sqlStmt);
		}

		// Make sure the cursor is closed first
		if (!CloseCursor(m_hstmt))
			return false;

		// Execute the SQL SELECT statement
		int retcode;
		retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) (queryType == DB_SELECT_STATEMENT ? pSqlStmt.c_str() : sqlStmt.c_str()), SQL_NTS);
		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
			return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt));

		// Completed successfully
		return true;

	}  // wxDbTable::query()


	/***************************** PUBLIC FUNCTIONS *****************************/


	bool Table::Open(bool checkPrivileges, bool checkTableExists)
	{
		exASSERT(m_pDb);
		exASSERT(m_pDb->IsOpen());
		exASSERT(!IsOpen());
		// TODO: Add a check to ensure if in manual mode at least one column has been defined
//		exASSERT_MSG(!m_manualColumns || m_numCols > 0, L"You must Define at least one column if using manual Column mode");

		std::wstring sqlStmt;
		std::wstring s;
		s.empty();

		// If we do not already have a STableInfo for our table, we absolutely must find one
		bool foundTable = false;
		bool searchedTable = false;
		if (!m_haveTableInfo)
		{
			if (!m_pDb->FindOneTable(m_initialTableName, m_initialSchemaName, m_initialCatalogName, m_initialTypeName, m_tableInfo))
			{
				return false;
			}
			m_haveTableInfo = true;
			searchedTable = true;
			foundTable = true;
		}

		// If we are asked to check existence and have not just proved we exist, find table
		if (checkTableExists && !foundTable && !searchedTable)
		{
			if (!m_pDb->FindOneTable(m_initialTableName, m_initialSchemaName, m_initialCatalogName, m_initialTypeName, m_tableInfo))
			{
				return false;
			}
			m_haveTableInfo = true;
			searchedTable = true;
			foundTable = true;
		}
		// not found?
		if (checkTableExists && !foundTable)
		{
			return false;
		}

		// If we are asked to create our columns automatically, read the column information
		if (!m_manualColumns)
		{
			std::vector<SColumnInfo> columns;
			if (!m_pDb->ReadTableColumnInfo(m_tableInfo, columns))
			{
				return false;
			}
			m_numCols = columns.size();
			if (m_numCols == 0)
			{
				LOG_ERROR((boost::wformat(L"No columns found for table '%s'") %m_tableInfo.GetSqlName()).str());
				return false;
			}
			std::vector<SColumnInfo>::const_iterator it;
			for (it = columns.begin(); it != columns.end(); it++)
			{
				SColumnInfo colInfo = *it;
				ColumnBuffer* pColBuff = new ColumnBuffer(colInfo);
				if (!pColBuff->BindColumnBuffer(m_hStmtSelect))
				{
					return false;
				}
				m_columnBuffers.push_back(pColBuff);
			}
			m_fieldsStatement = BuildFieldsStatement();
		}

		if (checkPrivileges)
		{
			exASSERT(false);
			// Verify the user has rights to access the table.
			//bool hasPrivs;

			//if (m_pDb->Dbms() == dbmsPOSTGRES)
			//    hasPrivs = m_pDb->TablePrivileges(m_tableName, L"SELECT", m_pDb->GetUsername().c_str(), NULL, m_tablePath);
			//else
			//    hasPrivs = m_pDb->TablePrivileges(m_tableName, L"SELECT", m_pDb->GetUsername().c_str(), m_pDb->GetUsername().c_str(), m_tablePath);

			//if (!hasPrivs)
			//    s = L"Connecting user does not have sufficient privileges to access this table.\n";
		}

		// Bind the member variables for field exchange between
		// the wxDbTable object and the ODBC record.
		if (!IsQueryOnly())
		{
			// commented because of missing datatype-info
			exASSERT(false);
			//if (!bindInsertParams())                    // Inserts
			//    return false;

			//if (!bindUpdateParams())                    // Updates
			//    return false;
		}

		if (m_manualColumns)
		{
			// The old wx-way of binding params
			 // TODO: Replace with new method once done
			if (!bindCols(*m_hstmtDefault))                   // Selects
				return false;

			if (!bindCols(m_hstmtInternal))                   // Internal use only
				return false;
		}

		/*
		* Do NOT bind the hstmtCount cursor!!!
		*/

		// Build an insert statement using parameter markers
		if (!IsQueryOnly() && m_numCols > 0)
		{
			exASSERT(false);
			//      bool needComma = false;
			//sqlStmt = (boost::wformat(L"INSERT INTO %s (") % m_pDb->SQLTableName(m_tableName.c_str())).str();
			//      for (i = 0; i < m_numCols; i++)
			//      {
			//          if (! m_colDefs[i].m_insertAllowed)
			//              continue;
			//          if (needComma)
			//              sqlStmt += L",";
			//          sqlStmt += m_pDb->SQLColumnName(m_colDefs[i].m_colName);
			//          needComma = true;
			//      }
			//      needComma = false;
			//      sqlStmt += L") VALUES (";

			//      int insertableCount = 0;

			//      for (i = 0; i < m_numCols; i++)
			//      {
			//          if (! m_colDefs[i].m_insertAllowed)
			//              continue;
			//          if (needComma)
			//              sqlStmt += L",";
			//          sqlStmt += L"?";
			//          needComma = true;
			//          insertableCount++;
			//      }
			//      sqlStmt += L")";

			//      // Prepare the insert statement for execution
			//      if (insertableCount)
			//      {
			//          if (SQLPrepare(m_hstmtInsert, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS) != SQL_SUCCESS)
			//              return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert));
			//      }
			//      else
			//          m_insertable = false;
		}

		// Completed successfully
		m_isOpen = true;
		return true;

	}


	///********** wxDbTable::Query() **********/
	//bool Table::Query(bool forUpdate, bool distinct)
	//{
	//
	//    return(query(DB_SELECT_WHERE, forUpdate, distinct));
	//
	//}  // wxDbTable::Query()


	/********** wxDbTable::QueryBySqlStmt() **********/
	bool Table::QueryBySqlStmt(const std::wstring &pSqlStmt)
	{
		//    m_pDb->WriteSqlLog(pSqlStmt);

		return(query(DB_SELECT_STATEMENT, false, false, pSqlStmt));

	}  // wxDbTable::QueryBySqlStmt()


	/********** wxDbTable::QueryMatching() **********/
	bool Table::QueryMatching(bool forUpdate, bool distinct)
	{

		return(query(DB_SELECT_MATCHING, forUpdate, distinct));

	}  // wxDbTable::QueryMatching()


	/********** wxDbTable::QueryOnKeyFields() **********/
	bool Table::QueryOnKeyFields(bool forUpdate, bool distinct)
	{

		return(query(DB_SELECT_KEYFIELDS, forUpdate, distinct));

	}  // wxDbTable::QueryOnKeyFields()


	/********** wxDbTable::GetPrev() **********/
	bool Table::GetPrev()
	{
		if (m_pDb->FwdOnlyCursors())
		{
			exFAIL_MSG(L"GetPrev()::Backward scrolling cursors are not enabled for this instance of wxDbTable");
			return false;
		}
		else
			return(getRec(SQL_FETCH_PRIOR));

	}  // wxDbTable::GetPrev()


	/********** wxDbTable::operator-- **********/
	bool Table::operator--(int)
	{
		if (m_pDb->FwdOnlyCursors())
		{
			exFAIL_MSG(L"operator--:Backward scrolling cursors are not enabled for this instance of wxDbTable");
			return false;
		}
		else
			return(getRec(SQL_FETCH_PRIOR));

	}  // wxDbTable::operator--


	/********** wxDbTable::GetFirst() **********/
	bool Table::GetFirst()
	{
		if (m_pDb->FwdOnlyCursors())
		{
			exFAIL_MSG(L"GetFirst():Backward scrolling cursors are not enabled for this instance of wxDbTable");
			return false;
		}
		else
			return(getRec(SQL_FETCH_FIRST));

	}  // wxDbTable::GetFirst()


	/********** wxDbTable::GetLast() **********/
	bool Table::GetLast()
	{
		if (m_pDb->FwdOnlyCursors())
		{
			exFAIL_MSG(L"GetLast()::Backward scrolling cursors are not enabled for this instance of wxDbTable");
			return false;
		}
		else
			return(getRec(SQL_FETCH_LAST));

	}  // wxDbTable::GetLast()


	///********** wxDbTable::BuildDeleteStmt() **********/
	//void Table::BuildDeleteStmt(std::wstring &pSqlStmt, int typeOfDel, const std::wstring &pWhereClause)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return;
	//
	//    std::wstring whereClause;
	//
	//    whereClause.empty();
	//
	//    // Handle the case of DeleteWhere() and the where clause is blank.  It should
	//    // delete all records from the database in this case.
	//    if (typeOfDel == DB_DEL_WHERE && (pWhereClause.length() == 0))
	//    {
	//		pSqlStmt = (boost::wformat(L"DELETE FROM %s") %m_pDb->SQLTableName(m_tableName.c_str())).str();
	//        return;
	//    }
	//
	//	pSqlStmt = (boost::wformat(L"DELETE FROM %s WHERE ") % m_pDb->SQLTableName(m_tableName.c_str())).str();
	//
	//    // Append the WHERE clause to the SQL DELETE statement
	//    switch(typeOfDel)
	//    {
	//        case DB_DEL_KEYFIELDS:
	//            // If the datasource supports the ROWID column, build
	//            // the where on ROWID for efficiency purposes.
	//            // e.g. DELETE FROM PARTS WHERE ROWID = '111.222.333'
	//            if (CanUpdateByROWID())
	//            {
	//                SQLLEN cb;
	//                wchar_t   rowid[wxDB_ROWID_LEN+1];
	//
	//                // Get the ROWID value.  If not successful retreiving the ROWID,
	//                // simply fall down through the code and build the WHERE clause
	//                // based on the key fields.
	//                if (SQLGetData(m_hstmt, (UWORD)(m_numCols+1), SQL_C_WXCHAR, (UCHAR*) rowid, sizeof(rowid), &cb) == SQL_SUCCESS)
	//                {
	//                    pSqlStmt += L"ROWID = '";
	//                    pSqlStmt += rowid;
	//                    pSqlStmt += L"'";
	//                    break;
	//                }
	//            }
	//            // Unable to delete by ROWID, so build a WHERE
	//            // clause based on the keyfields.
	//            BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS);
	//            pSqlStmt += whereClause;
	//            break;
	//        case DB_DEL_WHERE:
	//            pSqlStmt += pWhereClause;
	//            break;
	//        case DB_DEL_MATCHING:
	//            BuildWhereClause(whereClause, DB_WHERE_MATCHING);
	//            pSqlStmt += whereClause;
	//            break;
	//    }
	//
	//}  // BuildDeleteStmt()


	///***** DEPRECATED: use wxDbTable::BuildDeleteStmt(std::wstring &....) form *****/
	//void Table::BuildDeleteStmt(wchar_t *pSqlStmt, int typeOfDel, const std::wstring &pWhereClause)
	//{
	//    std::wstring tempSqlStmt;
	//    BuildDeleteStmt(tempSqlStmt, typeOfDel, pWhereClause);
	//    wcscpy(pSqlStmt, tempSqlStmt.c_str());
	//}  // wxDbTable::BuildDeleteStmt()


	///********** wxDbTable::BuildSelectStmt() **********/
	//void Table::BuildSelectStmt(std::wstring &pSqlStmt, int typeOfSelect, bool distinct)
	//{
	//    std::wstring whereClause;
	//    whereClause.empty();
	//
	//    // Build a select statement to query the database
	//    pSqlStmt = L"SELECT ";
	//
	//    // SELECT DISTINCT values only?
	//    if (distinct)
	//        pSqlStmt += L"DISTINCT ";
	//
	//    // Was a FROM clause specified to join tables to the base table?
	//    // Available for ::Query() only!!!
	//    bool appendFromClause = false;
	//
	//    if (typeOfSelect == DB_SELECT_WHERE && m_from.length())
	//        appendFromClause = true;
	//
	//    // Add the column list
	//    int i;
	//    std::wstring tStr;
	//    for (i = 0; i < m_numCols; i++)
	//    {
	//        tStr = m_colDefs[i].m_colName;
	//        // If joining tables, the base table column names must be qualified to avoid ambiguity
	//        if ((appendFromClause || m_pDb->Dbms() == dbmsACCESS) && tStr.find(L'.') == std::wstring::npos)
	//        {
	//            pSqlStmt += m_pDb->SQLTableName(m_queryTableName.c_str());
	//            pSqlStmt += L".";
	//        }
	//        pSqlStmt += m_pDb->SQLColumnName(m_colDefs[i].m_colName);
	//        if (i + 1 < m_numCols)
	//            pSqlStmt += L",";
	//    }
	//
	//    // If the datasource supports ROWID, get this column as well.  Exception: Don't retrieve
	//    // the ROWID if querying distinct records.  The rowid will always be unique.
	//    if (!distinct && CanUpdateByROWID())
	//    {
	//        // If joining tables, the base table column names must be qualified to avoid ambiguity
	//        if (appendFromClause || m_pDb->Dbms() == dbmsACCESS)
	//        {
	//            pSqlStmt += L",";
	//            pSqlStmt += m_pDb->SQLTableName(m_queryTableName.c_str());
	//            pSqlStmt += L".ROWID";
	//        }
	//        else
	//            pSqlStmt += L",ROWID";
	//    }
	//
	//    // Append the FROM tablename portion
	//    pSqlStmt += L" FROM ";
	//    pSqlStmt += m_pDb->SQLTableName(m_queryTableName.c_str());
	////    pSqlStmt += queryTableName;
	//
	//    // Sybase uses the HOLDLOCK keyword to lock a record during query.
	//    // The HOLDLOCK keyword follows the table name in the from clause.
	//    // Each table in the from clause must specify HOLDLOCK or
	//    // NOHOLDLOCK (the default).  Note: The "FOR UPDATE" clause
	//    // is parsed but ignored in SYBASE Transact-SQL.
	//    if (m_selectForUpdate && (m_pDb->Dbms() == dbmsSYBASE_ASA || m_pDb->Dbms() == dbmsSYBASE_ASE))
	//        pSqlStmt += L" HOLDLOCK";
	//
	//    if (appendFromClause)
	//        pSqlStmt += m_from;
	//
	//    // Append the WHERE clause.  Either append the where clause for the class
	//    // or build a where clause.  The typeOfSelect determines this.
	//    switch(typeOfSelect)
	//    {
	//        case DB_SELECT_WHERE:
	//
	//            if (m_where.length())   // May not want a where clause!!!
	//            {
	//                pSqlStmt += L" WHERE ";
	//                pSqlStmt += m_where;
	//            }
	//            break;
	//        case DB_SELECT_KEYFIELDS:
	//            BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS);
	//            if (whereClause.length())
	//            {
	//                pSqlStmt += L" WHERE ";
	//                pSqlStmt += whereClause;
	//            }
	//            break;
	//        case DB_SELECT_MATCHING:
	//            BuildWhereClause(whereClause, DB_WHERE_MATCHING);
	//            if (whereClause.length())
	//            {
	//                pSqlStmt += L" WHERE ";
	//                pSqlStmt += whereClause;
	//            }
	//            break;
	//    }
	//
	//    // Append the ORDER BY clause
	//    if (m_orderBy.length())
	//    {
	//        pSqlStmt += L" ORDER BY ";
	//        pSqlStmt += m_orderBy;
	//    }
	//
	//    // SELECT FOR UPDATE if told to do so and the datasource is capable.  Sybase
	//    // parses the FOR UPDATE clause but ignores it.  See the comment above on the
	//    // HOLDLOCK for Sybase.
	//    if (m_selectForUpdate && CanSelectForUpdate())
	//        pSqlStmt += L" FOR UPDATE";
	//
	//}  // wxDbTable::BuildSelectStmt()


	///***** DEPRECATED: use wxDbTable::BuildSelectStmt(std::wstring &....) form *****/
	//void Table::BuildSelectStmt(wchar_t *pSqlStmt, int typeOfSelect, bool distinct)
	//{
	//    std::wstring tempSqlStmt;
	//    BuildSelectStmt(tempSqlStmt, typeOfSelect, distinct);
	//    wcscpy(pSqlStmt, tempSqlStmt.c_str());
	//}  // wxDbTable::BuildSelectStmt()


	///********** wxDbTable::BuildUpdateStmt() **********/
	//void Table::BuildUpdateStmt(std::wstring &pSqlStmt, int typeOfUpdate, const std::wstring &pWhereClause)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return;
	//
	//    std::wstring whereClause;
	//    whereClause.empty();
	//
	//    bool firstColumn = true;
	//
	//	pSqlStmt = (boost::wformat(L"UPDATE %s SET ") % m_pDb->SQLTableName(m_tableName.c_str())).str();
	//
	//    // Append a list of columns to be updated
	//    int i;
	//    for (i = 0; i < m_numCols; i++)
	//    {
	//        // Only append Updateable columns
	//        if (m_colDefs[i].m_updateable)
	//        {
	//            if (!firstColumn)
	//                pSqlStmt += L",";
	//            else
	//                firstColumn = false;
	//
	//            pSqlStmt += m_pDb->SQLColumnName(m_colDefs[i].m_colName);
	////            pSqlStmt += colDefs[i].ColName;
	//            pSqlStmt += L" = ?";
	//        }
	//    }
	//
	//    // Append the WHERE clause to the SQL UPDATE statement
	//    pSqlStmt += L" WHERE ";
	//    switch(typeOfUpdate)
	//    {
	//        case DB_UPD_KEYFIELDS:
	//            // If the datasource supports the ROWID column, build
	//            // the where on ROWID for efficiency purposes.
	//            // e.g. UPDATE PARTS SET Col1 = ?, Col2 = ? WHERE ROWID = '111.222.333'
	//            if (CanUpdateByROWID())
	//            {
	//                SQLLEN cb;
	//                wchar_t rowid[wxDB_ROWID_LEN+1];
	//
	//                // Get the ROWID value.  If not successful retreiving the ROWID,
	//                // simply fall down through the code and build the WHERE clause
	//                // based on the key fields.
	//                if (SQLGetData(m_hstmt, (UWORD)(m_numCols+1), SQL_C_WXCHAR, (UCHAR*) rowid, sizeof(rowid), &cb) == SQL_SUCCESS)
	//                {
	//                    pSqlStmt += L"ROWID = '";
	//                    pSqlStmt += rowid;
	//                    pSqlStmt += L"'";
	//                    break;
	//                }
	//            }
	//            // Unable to delete by ROWID, so build a WHERE
	//            // clause based on the keyfields.
	//            BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS);
	//            pSqlStmt += whereClause;
	//            break;
	//        case DB_UPD_WHERE:
	//            pSqlStmt += pWhereClause;
	//            break;
	//    }
	//}  // BuildUpdateStmt()


	///***** DEPRECATED: use wxDbTable::BuildUpdateStmt(std::wstring &....) form *****/
	//void Table::BuildUpdateStmt(wchar_t *pSqlStmt, int typeOfUpdate, const std::wstring &pWhereClause)
	//{
	//    std::wstring tempSqlStmt;
	//    BuildUpdateStmt(tempSqlStmt, typeOfUpdate, pWhereClause);
	//    wcscpy(pSqlStmt, tempSqlStmt.c_str());
	//}  // BuildUpdateStmt()


	///********** wxDbTable::BuildWhereClause() **********/
	//void Table::BuildWhereClause(std::wstring &pWhereClause, int typeOfWhere,
	//                                 const std::wstring &qualTableName, bool useLikeComparison)
	///*
	// * Note: BuildWhereClause() currently ignores timestamp columns.
	// *       They are not included as part of the where clause.
	// */
	//{
	//    bool moreThanOneColumn = false;
	//    std::wstring colValue;
	//
	//    // Loop through the columns building a where clause as you go
	//    int colNumber;
	//    for (colNumber = 0; colNumber < m_numCols; colNumber++)
	//    {
	//        // Determine if this column should be included in the WHERE clause
	//        if ((typeOfWhere == DB_WHERE_KEYFIELDS && m_colDefs[colNumber].m_keyField) ||
	//             (typeOfWhere == DB_WHERE_MATCHING  && (!IsColNull((UWORD)colNumber))))
	//        {
	//            // Skip over timestamp columns
	//            if (m_colDefs[colNumber].m_sqlCtype == SQL_C_TIMESTAMP)
	//                continue;
	//            // If there is more than 1 column, join them with the keyword "AND"
	//            if (moreThanOneColumn)
	//                pWhereClause += L" AND ";
	//            else
	//                moreThanOneColumn = true;
	//
	//            // Concatenate where phrase for the column
	//            std::wstring tStr = m_colDefs[colNumber].m_colName;
	//
	//            if (qualTableName.length() && tStr.find(L'.') == std::wstring::npos)
	//            {
	//                pWhereClause += m_pDb->SQLTableName(qualTableName.c_str());
	//                pWhereClause += L".";
	//            }
	//            pWhereClause += m_pDb->SQLColumnName(m_colDefs[colNumber].m_colName);
	//
	//            if (useLikeComparison && (m_colDefs[colNumber].m_sqlCtype == SQL_C_WXCHAR))
	//                pWhereClause += L" LIKE ";
	//            else
	//                pWhereClause += L" = ";
	//
	//            switch(m_colDefs[colNumber].m_sqlCtype)
	//            {
	//                case SQL_C_CHAR:
	//#ifdef SQL_C_WCHAR
	//                case SQL_C_WCHAR:
	//#endif
	//                //case SQL_C_WXCHAR:  SQL_C_WXCHAR is covered by either SQL_C_CHAR or SQL_C_WCHAR
	//					colValue = (boost::wformat(L"'%s'") % GetDb()->EscapeSqlChars((wchar_t *)m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_SHORT:
	//                case SQL_C_SSHORT:
	//					colValue = (boost::wformat(L"%hi") % *((SWORD *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_USHORT:
	//					colValue = (boost::wformat(L"%hu") % *((UWORD *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_LONG:
	//                case SQL_C_SLONG:
	//					colValue = (boost::wformat(L"%li") % *((SDWORD *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_ULONG:
	//					colValue = (boost::wformat(L"%lu") % *((UDWORD *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_FLOAT:
	//					colValue = (boost::wformat(L"%.6f") % *((SFLOAT *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_DOUBLE:
	//					colValue = (boost::wformat(L"%.6f") % *((SDOUBLE *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                default:
	//                    {
	//                        std::wstring strMsg;
	//						strMsg = (boost::wformat(L"wxDbTable::bindParams(): Unknown column type for colDefs %d colName %s") % colNumber % m_colDefs[colNumber].m_colName).str();
	//                        exFAIL_MSG(strMsg.c_str());
	//                    }
	//                    break;
	//            }
	//            pWhereClause += colValue;
	//        }
	//    }
	//}  // wxDbTable::BuildWhereClause()


	///***** DEPRECATED: use wxDbTable::BuildWhereClause(std::wstring &....) form *****/
	//void Table::BuildWhereClause(wchar_t *pWhereClause, int typeOfWhere,
	//                                 const std::wstring &qualTableName, bool useLikeComparison)
	//{
	//    std::wstring tempSqlStmt;
	//    BuildWhereClause(tempSqlStmt, typeOfWhere, qualTableName, useLikeComparison);
	//    wcscpy(pWhereClause, tempSqlStmt.c_str());
	//}  // wxDbTable::BuildWhereClause()


	/********** wxDbTable::GetRowNum() **********/
	UWORD Table::GetRowNum()
	{
		UDWORD rowNum;

		if (SQLGetStmtOption(m_hstmt, SQL_ROW_NUMBER, (UCHAR*)&rowNum) != SQL_SUCCESS)
		{
			m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt);
			return(0);
		}

		// Completed successfully
		return((UWORD)rowNum);

	}  // wxDbTable::GetRowNum()


	/********** wxDbTable::CloseCursor() **********/
	bool Table::CloseCursor(HSTMT cursor)
	{
		if (SQLFreeStmt(cursor, SQL_CLOSE) != SQL_SUCCESS)
			return(m_pDb->DispAllErrors(NULL, m_hdbc, cursor));

		// Completed successfully
		return true;

	}  // wxDbTable::CloseCursor()


	/********** wxDbTable::CreateTable() **********/
	//bool Table::CreateTable(bool attemptDrop)
	//{
	//    if (!m_pDb)
	//        return false;
	//
	//    int i, j;
	//    std::wstring sqlStmt;
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << L"Creating Table " << m_tableName << L"..." << std::endl;
	//#endif
	//
	//    // Drop table first
	//    if (attemptDrop && !DropTable())
	//        return false;
	//
	//    // Create the table
	//#ifdef DBDEBUG_CONSOLE
	//    for (i = 0; i < m_numCols; i++)
	//    {
	//        // Exclude derived columns since they are NOT part of the base table
	//        if (m_colDefs[i].m_derivedCol)
	//            continue;
	//        std::wcout << i + 1 << L": " << m_colDefs[i].m_colName << L"; ";
	//        switch(m_colDefs[i].m_dbDataType)
	//        {
	//            case DB_DATA_TYPE_VARCHAR:
	//                std::wcout << m_pDb->GetTypeInfVarchar().TypeName << L"(" << (int)(m_colDefs[i].m_szDataObj / sizeof(wchar_t)) << L")";
	//                break;
	//            case DB_DATA_TYPE_MEMO:
	//                std::wcout << m_pDb->GetTypeInfMemo().TypeName;
	//                break;
	//            case DB_DATA_TYPE_INTEGER:
	//                std::wcout << m_pDb->GetTypeInfInteger().TypeName;
	//                break;
	//            case DB_DATA_TYPE_FLOAT:
	//                std::wcout << m_pDb->GetTypeInfFloat().TypeName;
	//                break;
	//            case DB_DATA_TYPE_DATE:
	//                std::wcout << m_pDb->GetTypeInfDate().TypeName;
	//                break;
	//            case DB_DATA_TYPE_BLOB:
	//                std::wcout << m_pDb->GetTypeInfBlob().TypeName;
	//                break;
	//        }
	//        std::wcout << std::endl;
	//    }
	//#endif
	//
	//    // Build a CREATE TABLE string from the colDefs structure.
	//    bool needComma = false;
	//
	//	sqlStmt = (boost::wformat(L"CREATE TABLE %s (") % m_pDb->SQLTableName(m_tableName.c_str())).str();
	//
	//    for (i = 0; i < m_numCols; i++)
	//    {
	//        // Exclude derived columns since they are NOT part of the base table
	//        if (m_colDefs[i].m_derivedCol)
	//            continue;
	//        // Comma Delimiter
	//        if (needComma)
	//            sqlStmt += L",";
	//        // Column Name
	//        sqlStmt += m_pDb->SQLColumnName(m_colDefs[i].m_colName);
	////        sqlStmt += colDefs[i].ColName;
	//        sqlStmt += L" ";
	//        // Column Type
	//        switch(m_colDefs[i].m_dbDataType)
	//        {
	//            case DB_DATA_TYPE_VARCHAR:
	//                sqlStmt += m_pDb->GetTypeInfVarchar().TypeName;
	//                break;
	//            case DB_DATA_TYPE_MEMO:
	//                sqlStmt += m_pDb->GetTypeInfMemo().TypeName;
	//                break;
	//            case DB_DATA_TYPE_INTEGER:
	//                sqlStmt += m_pDb->GetTypeInfInteger().TypeName;
	//                break;
	//            case DB_DATA_TYPE_FLOAT:
	//                sqlStmt += m_pDb->GetTypeInfFloat().TypeName;
	//                break;
	//            case DB_DATA_TYPE_DATE:
	//                sqlStmt += m_pDb->GetTypeInfDate().TypeName;
	//                break;
	//            case DB_DATA_TYPE_BLOB:
	//                sqlStmt += m_pDb->GetTypeInfBlob().TypeName;
	//                break;
	//        }
	//        // For varchars, append the size of the string
	//        if (m_colDefs[i].m_dbDataType == DB_DATA_TYPE_VARCHAR &&
	//            (m_pDb->Dbms() != dbmsMY_SQL || m_pDb->GetTypeInfVarchar().TypeName != L"text"))// ||
	////            colDefs[i].DbDataType == DB_DATA_TYPE_BLOB)
	//        {
	//            std::wstring s;
	//			s = (boost::wformat(L"(%d)") % (int)(m_colDefs[i].m_szDataObj / sizeof(wchar_t))).str();
	//            sqlStmt += s;
	//        }
	//
	//        if (m_pDb->Dbms() == dbmsDB2 ||
	//            m_pDb->Dbms() == dbmsMY_SQL ||
	//            m_pDb->Dbms() == dbmsSYBASE_ASE  ||
	//            m_pDb->Dbms() == dbmsINTERBASE  ||
	//            m_pDb->Dbms() == dbmsFIREBIRD  ||
	//            m_pDb->Dbms() == dbmsMS_SQL_SERVER)
	//        {
	//            if (m_colDefs[i].m_keyField)
	//            {
	//                sqlStmt += L" NOT NULL";
	//            }
	//        }
	//
	//        needComma = true;
	//    }
	//    // If there is a primary key defined, include it in the create statement
	//    for (i = j = 0; i < m_numCols; i++)
	//    {
	//        if (m_colDefs[i].m_keyField)
	//        {
	//            j++;
	//            break;
	//        }
	//    }
	//    if ( j && (m_pDb->Dbms() != dbmsDBASE)
	//        && (m_pDb->Dbms() != dbmsXBASE_SEQUITER) )  // Found a keyfield
	//    {
	//        switch (m_pDb->Dbms())
	//        {
	//            case dbmsACCESS:
	//            case dbmsINFORMIX:
	//            case dbmsSYBASE_ASA:
	//            case dbmsSYBASE_ASE:
	//            case dbmsMY_SQL:
	//            case dbmsFIREBIRD:
	//            {
	//                // MySQL goes out on this one. We also declare the relevant key NON NULL above
	//                sqlStmt += L",PRIMARY KEY (";
	//                break;
	//            }
	//            default:
	//            {
	//                sqlStmt += L",CONSTRAINT ";
	//                //  DB2 is limited to 18 characters for index names
	//                if (m_pDb->Dbms() == dbmsDB2)
	//                {
	//                    exASSERT_MSG(m_tableName.length() <= 13, "DB2 table/index names must be no longer than 13 characters in length.\n\nTruncating table name to 13 characters.");
	//                    sqlStmt += m_pDb->SQLTableName(m_tableName.substr(0, 13).c_str());
	////                    sqlStmt += tableName.substr(0, 13);
	//                }
	//                else
	//                    sqlStmt += m_pDb->SQLTableName(m_tableName.c_str());
	////                    sqlStmt += tableName;
	//
	//                sqlStmt += L"_PIDX PRIMARY KEY (";
	//                break;
	//            }
	//        }
	//
	//        // List column name(s) of column(s) comprising the primary key
	//        for (i = j = 0; i < m_numCols; i++)
	//        {
	//            if (m_colDefs[i].m_keyField)
	//            {
	//                if (j++) // Multi part key, comma separate names
	//                    sqlStmt += L",";
	//                sqlStmt += m_pDb->SQLColumnName(m_colDefs[i].m_colName);
	//
	//                if (m_pDb->Dbms() == dbmsMY_SQL &&
	//                    m_colDefs[i].m_dbDataType ==  DB_DATA_TYPE_VARCHAR)
	//                {
	//                    std::wstring s;
	//					s = (boost::wformat(L"(%d)") % (int)(m_colDefs[i].m_szDataObj / sizeof(wchar_t))).str();
	//                    sqlStmt += s;
	//                }
	//            }
	//        }
	//        sqlStmt += L")";
	//
	//        if (m_pDb->Dbms() == dbmsINFORMIX ||
	//            m_pDb->Dbms() == dbmsSYBASE_ASA ||
	//            m_pDb->Dbms() == dbmsSYBASE_ASE)
	//        {
	//            sqlStmt += L" CONSTRAINT ";
	//            sqlStmt += m_pDb->SQLTableName(m_tableName.c_str());
	////            sqlStmt += tableName;
	//            sqlStmt += L"_PIDX";
	//        }
	//    }
	//    // Append the closing parentheses for the create table statement
	//    sqlStmt += L")";
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl;
	//#endif
	//
	//    // Execute the CREATE TABLE statement
	//    RETCODE retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
	//    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	//    {
	//        m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt);
	//        m_pDb->RollbackTrans();
	//        CloseCursor(m_hstmt);
	//        return false;
	//    }
	//
	//    // Commit the transaction and close the cursor
	//    if (!m_pDb->CommitTrans())
	//        return false;
	//    if (!CloseCursor(m_hstmt))
	//        return false;
	//
	//    // Database table created successfully
	//    return true;
	//
	//} // wxDbTable::CreateTable()


	/********** wxDbTable::DropTable() **********/
	//bool Table::DropTable()
	//{
	//    // NOTE: This function returns true if the Table does not exist, but
	//    //       only for identified databases.  Code will need to be added
	//    //       below for any other databases when those databases are defined
	//    //       to handle this situation consistently
	//
	//    std::wstring sqlStmt;
	//
	//	sqlStmt = (boost::wformat(L"DROP TABLE %s") % m_pDb->SQLTableName(m_tableName.c_str())).str();
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl;
	//#endif
	//
	//    RETCODE retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
	//    if (retcode != SQL_SUCCESS)
	//    {
	//        // Check for "Base table not found" error and ignore
	//        m_pDb->GetNextError(m_henv, m_hdbc, m_hstmt);
	//        if (wcscmp(m_pDb->sqlState, L"S0002") /*&&
	//            wcscmp(pDb->sqlState, L"S1000")*/)  // "Base table not found"
	//        {
	//            // Check for product specific error codes
	//            if (!((m_pDb->Dbms() == dbmsSYBASE_ASA    && !wcscmp(m_pDb->sqlState, L"42000"))   ||  // 5.x (and lower?)
	//                  (m_pDb->Dbms() == dbmsSYBASE_ASE    && !wcscmp(m_pDb->sqlState, L"37000"))   ||
	//                  (m_pDb->Dbms() == dbmsPERVASIVE_SQL && !wcscmp(m_pDb->sqlState, L"S1000"))   ||  // Returns an S1000 then an S0002
	//                  (m_pDb->Dbms() == dbmsPOSTGRES      && !wcscmp(m_pDb->sqlState, L"08S01"))))
	//            {
	//                m_pDb->DispNextError();
	//                m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt);
	//                m_pDb->RollbackTrans();
	////                CloseCursor(hstmt);
	//                return false;
	//            }
	//        }
	//    }
	//
	//    // Commit the transaction and close the cursor
	//    if (! m_pDb->CommitTrans())
	//        return false;
	//    if (! CloseCursor(m_hstmt))
	//        return false;
	//
	//    return true;
	//}  // wxDbTable::DropTable()


	/********** wxDbTable::CreateIndex() **********/
	//bool Table::CreateIndex(const std::wstring &indexName, bool unique, UWORD numIndexColumns,
	//                                     SIndexDefinition *pIndexDefs, bool attemptDrop)
	//{
	//    std::wstring sqlStmt;
	//
	//    // Drop the index first
	//    if (attemptDrop && !DropIndex(indexName))
	//        return false;
	//
	//    // MySQL (and possibly Sybase ASE?? - gt) require that any columns which are used as portions
	//    // of an index have the columns defined as "NOT NULL".  During initial table creation though,
	//    // it may not be known which columns are necessarily going to be part of an index (e.g. the
	//    // table was created, then months later you determine that an additional index while
	//    // give better performance, so you want to add an index).
	//    //
	//    // The following block of code will modify the column definition to make the column be
	//    // defined with the "NOT NULL" qualifier.
	//    if (m_pDb->Dbms() == dbmsMY_SQL)
	//    {
	//        std::wstring sqlStmt;
	//        int i;
	//        bool ok = true;
	//        for (i = 0; i < numIndexColumns && ok; i++)
	//        {
	//            int   j = 0;
	//            bool  found = false;
	//            // Find the column definition that has the ColName that matches the
	//            // index column name.  We need to do this to get the DB_DATA_TYPE of
	//            // the index column, as MySQL's syntax for the ALTER column requires
	//            // this information
	//            while (!found && (j < this->m_numCols))
	//            {
	//                if (wcscmp(m_colDefs[j].m_colName,pIndexDefs[i].ColName) == 0)
	//                    found = true;
	//                if (!found)
	//                    j++;
	//            }
	//
	//            if (found)
	//            {
	//                ok = m_pDb->ModifyColumn(m_tableName, pIndexDefs[i].ColName,
	//                                        m_colDefs[j].m_dbDataType, (int)(m_colDefs[j].m_szDataObj / sizeof(wchar_t)),
	//                                        L"NOT NULL");
	//
	//                if (!ok)
	//                {
	//                    #if 0
	//                    // retcode is not used
	//                    wxODBC_ERRORS retcode;
	//                    // Oracle returns a DB_ERR_GENERAL_ERROR if the column is already
	//                    // defined to be NOT NULL, but reportedly MySQL doesn't mind.
	//                    // This line is just here for debug checking of the value
	//                    retcode = (wxODBC_ERRORS)pDb->DB_STATUS;
	//                    #endif
	//                }
	//            }
	//            else
	//                ok = false;
	//        }
	//        if (ok)
	//            m_pDb->CommitTrans();
	//        else
	//        {
	//            m_pDb->RollbackTrans();
	//            return false;
	//        }
	//    }
	//
	//    // Build a CREATE INDEX statement
	//    sqlStmt = L"CREATE ";
	//    if (unique)
	//        sqlStmt += L"UNIQUE ";
	//
	//    sqlStmt += L"INDEX ";
	//    sqlStmt += m_pDb->SQLTableName(indexName.c_str());
	//    sqlStmt += L" ON ";
	//
	//    sqlStmt += m_pDb->SQLTableName(m_tableName.c_str());
	////    sqlStmt += tableName;
	//    sqlStmt += L" (";
	//
	//    // Append list of columns making up index
	//    int i;
	//    for (i = 0; i < numIndexColumns; i++)
	//    {
	//        sqlStmt += m_pDb->SQLColumnName(pIndexDefs[i].ColName);
	////        sqlStmt += pIndexDefs[i].ColName;
	//
	//        // MySQL requires a key length on VARCHAR keys
	//        if ( m_pDb->Dbms() == dbmsMY_SQL )
	//        {
	//            // Find the details on this column
	//            int j;
	//            for ( j = 0; j < m_numCols; ++j )
	//            {
	//                if ( wcscmp( pIndexDefs[i].ColName, m_colDefs[j].m_colName ) == 0 )
	//                {
	//                    break;
	//                }
	//            }
	//            if ( m_colDefs[j].m_dbDataType ==  DB_DATA_TYPE_VARCHAR)
	//            {
	//                std::wstring s;
	//				s = (boost::wformat(L"(%d)") % (int)(m_colDefs[i].m_szDataObj / sizeof(wchar_t))).str();
	//                sqlStmt += s;
	//            }
	//        }
	//
	//        // Postgres and SQL Server 7 do not support the ASC/DESC keywords for index columns
	//        if (!((m_pDb->Dbms() == dbmsMS_SQL_SERVER) && (wcsncmp(m_pDb->m_dbInf.dbmsVer, L"07", 2)==0)) &&
	//            !(m_pDb->Dbms() == dbmsFIREBIRD) &&
	//            !(m_pDb->Dbms() == dbmsPOSTGRES))
	//        {
	//            if (pIndexDefs[i].Ascending)
	//                sqlStmt += L" ASC";
	//            else
	//                sqlStmt += L" DESC";
	//        }
	//        else
	//            exASSERT_MSG(pIndexDefs[i].Ascending, "Datasource does not support DESCending index columns");
	//
	//        if ((i + 1) < numIndexColumns)
	//            sqlStmt += L",";
	//    }
	//
	//    // Append closing parentheses
	//    sqlStmt += L")";
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl << std::endl;
	//#endif
	//
	//    // Execute the CREATE INDEX statement
	//    RETCODE retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
	//    if (retcode != SQL_SUCCESS)
	//    {
	//        m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt);
	//        m_pDb->RollbackTrans();
	//        CloseCursor(m_hstmt);
	//        return false;
	//    }
	//
	//    // Commit the transaction and close the cursor
	//    if (! m_pDb->CommitTrans())
	//        return false;
	//    if (! CloseCursor(m_hstmt))
	//        return false;
	//
	//    // Index Created Successfully
	//    return true;
	//
	//}  // wxDbTable::CreateIndex()


	/********** wxDbTable::DropIndex() **********/
	//bool Table::DropIndex(const std::wstring &indexName)
	//{
	//    // NOTE: This function returns true if the Index does not exist, but
	//    //       only for identified databases.  Code will need to be added
	//    //       below for any other databases when those databases are defined
	//    //       to handle this situation consistently
	//
	//    std::wstring sqlStmt;
	//
	//    if (m_pDb->Dbms() == dbmsACCESS || m_pDb->Dbms() == dbmsMY_SQL ||
	//        m_pDb->Dbms() == dbmsDBASE /*|| Paradox needs this syntax too when we add support*/)
	//		sqlStmt = (boost::wformat(L"DROP INDEX %s ON %s") % m_pDb->SQLTableName(indexName.c_str()) % m_pDb->SQLTableName(m_tableName.c_str())).str();
	//    else if ((m_pDb->Dbms() == dbmsMS_SQL_SERVER) ||
	//             (m_pDb->Dbms() == dbmsSYBASE_ASE) ||
	//             (m_pDb->Dbms() == dbmsXBASE_SEQUITER))
	//		sqlStmt = (boost::wformat(L"DROP INDEX %s.%s") % m_pDb->SQLTableName(m_tableName.c_str()) % m_pDb->SQLTableName(indexName.c_str())).str();
	//    else
	//		sqlStmt = (boost::wformat(L"DROP INDEX %s") % m_pDb->SQLTableName(indexName.c_str())).str();
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl;
	//#endif
	//    RETCODE retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
	//    if (retcode != SQL_SUCCESS)
	//    {
	//        // Check for "Index not found" error and ignore
	//        m_pDb->GetNextError(m_henv, m_hdbc, m_hstmt);
	//        if (wcscmp(m_pDb->sqlState, L"S0012"))  // "Index not found"
	//        {
	//            // Check for product specific error codes
	//            if (!((m_pDb->Dbms() == dbmsSYBASE_ASA    && !wcscmp(m_pDb->sqlState, L"42000")) ||  // v5.x (and lower?)
	//                  (m_pDb->Dbms() == dbmsSYBASE_ASE    && !wcscmp(m_pDb->sqlState, L"37000")) ||
	//                  (m_pDb->Dbms() == dbmsMS_SQL_SERVER && !wcscmp(m_pDb->sqlState, L"S1000")) ||
	//                  (m_pDb->Dbms() == dbmsINTERBASE     && !wcscmp(m_pDb->sqlState, L"S1000")) ||
	//                  (m_pDb->Dbms() == dbmsMAXDB         && !wcscmp(m_pDb->sqlState, L"S1000")) ||
	//                  (m_pDb->Dbms() == dbmsFIREBIRD      && !wcscmp(m_pDb->sqlState, L"HY000")) ||
	//                  (m_pDb->Dbms() == dbmsSYBASE_ASE    && !wcscmp(m_pDb->sqlState, L"S0002")) ||  // Base table not found
	//                  (m_pDb->Dbms() == dbmsMY_SQL        && !wcscmp(m_pDb->sqlState, L"42S12")) ||  // tested by Christopher Ludwik Marino-Cebulski using v3.23.21beta
	//                  (m_pDb->Dbms() == dbmsPOSTGRES      && !wcscmp(m_pDb->sqlState, L"08S01"))
	//               ))
	//            {
	//                m_pDb->DispNextError();
	//                m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt);
	//                m_pDb->RollbackTrans();
	//                CloseCursor(m_hstmt);
	//                return false;
	//            }
	//        }
	//    }
	//
	//    // Commit the transaction and close the cursor
	//    if (! m_pDb->CommitTrans())
	//        return false;
	//    if (! CloseCursor(m_hstmt))
	//        return false;
	//
	//    return true;
	//}  // wxDbTable::DropIndex()


	/********** wxDbTable::SetOrderByColNums() **********/
	bool Table::SetOrderByColNums(UWORD first, ...)
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

			tempStr += m_colDefs[colNumber].m_colName;
			colNumber = va_arg(argptr, int);
		}
		va_end(argptr);              /* Reset variable arguments.      */

		SetOrderByClause(tempStr);

		return (!abort);
	}  // wxDbTable::SetOrderByColNums()


	/********** wxDbTable::Insert() **********/
	//int Table::Insert()
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly || !m_insertable)
	//        return(DB_FAILURE);
	//
	//    bindInsertParams();
	//
	//    // Insert the record by executing the already prepared insert statement
	//    RETCODE retcode;
	//    retcode = SQLExecute(m_hstmtInsert);
	//    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO &&
	//        retcode != SQL_NEED_DATA)
	//    {
	//        // Check to see if integrity constraint was violated
	//        m_pDb->GetNextError(m_henv, m_hdbc, m_hstmtInsert);
	//        if (! wcscmp(m_pDb->sqlState, L"23000"))  // Integrity constraint violated
	//            return(DB_ERR_INTEGRITY_CONSTRAINT_VIOL);
	//        else
	//        {
	//            m_pDb->DispNextError();
	//            m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert);
	//            return(DB_FAILURE);
	//        }
	//    }
	//    if (retcode == SQL_NEED_DATA)
	//    {
	//        PTR pParmID;
	//        retcode = SQLParamData(m_hstmtInsert, &pParmID);
	//        while (retcode == SQL_NEED_DATA)
	//        {
	//            // Find the parameter
	//            int i;
	//            for (i=0; i < m_numCols; i++)
	//            {
	//                if (m_colDefs[i].m_ptrDataObj == pParmID)
	//                {
	//                    // We found it.  Store the parameter.
	//                    retcode = SQLPutData(m_hstmtInsert, pParmID, m_colDefs[i].m_szDataObj);
	//                    if (retcode != SQL_SUCCESS)
	//                    {
	//                        m_pDb->DispNextError();
	//                        m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert);
	//                        return(DB_FAILURE);
	//                    }
	//                    break;
	//                }
	//            }
	//            retcode = SQLParamData(m_hstmtInsert, &pParmID);
	//            if (retcode != SQL_SUCCESS &&
	//                retcode != SQL_SUCCESS_WITH_INFO)
	//            {
	//                // record was not inserted
	//                m_pDb->DispNextError();
	//                m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert);
	//                return(DB_FAILURE);
	//            }
	//        }
	//    }
	//
	//    // Record inserted into the datasource successfully
	//    return(DB_SUCCESS);
	//
	//}  // wxDbTable::Insert()


	///********** wxDbTable::Update() **********/
	//bool Table::Update()
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    std::wstring sqlStmt;
	//
	//    // Build the SQL UPDATE statement
	//    BuildUpdateStmt(sqlStmt, DB_UPD_KEYFIELDS);
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl << std::endl;
	//#endif
	//
	//    // Execute the SQL UPDATE statement
	//    return(execUpdate(sqlStmt));
	//
	//}  // wxDbTable::Update()


	///********** wxDbTable::Update(pSqlStmt) **********/
	//bool Table::Update(const std::wstring &pSqlStmt)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    m_pDb->WriteSqlLog(pSqlStmt);
	//
	//    return(execUpdate(pSqlStmt));
	//
	//}  // wxDbTable::Update(pSqlStmt)


	///********** wxDbTable::UpdateWhere() **********/
	//bool Table::UpdateWhere(const std::wstring &pWhereClause)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    std::wstring sqlStmt;
	//
	//    // Build the SQL UPDATE statement
	//    BuildUpdateStmt(sqlStmt, DB_UPD_WHERE, pWhereClause);
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl << std::endl;
	//#endif
	//
	//    // Execute the SQL UPDATE statement
	//    return(execUpdate(sqlStmt));
	//
	//}  // wxDbTable::UpdateWhere()


	///********** wxDbTable::Delete() **********/
	//bool Table::Delete()
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    std::wstring sqlStmt;
	//    sqlStmt.empty();
	//
	//    // Build the SQL DELETE statement
	//    BuildDeleteStmt(sqlStmt, DB_DEL_KEYFIELDS);
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//    // Execute the SQL DELETE statement
	//    return(execDelete(sqlStmt));
	//
	//}  // wxDbTable::Delete()


	///********** wxDbTable::DeleteWhere() **********/
	//bool Table::DeleteWhere(const std::wstring &pWhereClause)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    std::wstring sqlStmt;
	//    sqlStmt.empty();
	//
	//    // Build the SQL DELETE statement
	//    BuildDeleteStmt(sqlStmt, DB_DEL_WHERE, pWhereClause);
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//    // Execute the SQL DELETE statement
	//    return(execDelete(sqlStmt));
	//
	//}  // wxDbTable::DeleteWhere()


	///********** wxDbTable::DeleteMatching() **********/
	//bool Table::DeleteMatching()
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    std::wstring sqlStmt;
	//    sqlStmt.empty();
	//
	//    // Build the SQL DELETE statement
	//    BuildDeleteStmt(sqlStmt, DB_DEL_MATCHING);
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//    // Execute the SQL DELETE statement
	//    return(execDelete(sqlStmt));
	//
	//}  // wxDbTable::DeleteMatching()


	/********** wxDbTable::IsColNull() **********/
	bool Table::IsColNull(UWORD colNumber) const
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
		return (m_colDefs[colNumber].m_null);
	}  // wxDbTable::IsColNull()


	/********** wxDbTable::CanSelectForUpdate() **********/
	bool Table::CanSelectForUpdate()
	{
		// Broken
		//if (IsQueryOnly())
		//    return false;

		//if (m_pDb->Dbms() == dbmsMY_SQL)
		//    return false;

		//if (/*(m_pDb->Dbms() == dbmsORACLE) ||*/
		//    (m_pDb->m_dbInf.posStmts & SQL_PS_SELECT_FOR_UPDATE))
		//    return true;
		//else
		return false;

	}  // wxDbTable::CanSelectForUpdate()


	/********** wxDbTable::CanUpdateByROWID() **********/
	bool Table::CanUpdateByROWID()
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
	bool Table::IsCursorClosedOnCommit()
	{
		//if (m_pDb->m_dbInf.cursorCommitBehavior == SQL_CB_PRESERVE)
		//    return false;
		//else
		return true;

	}  // wxDbTable::IsCursorClosedOnCommit()



	/********** wxDbTable::ClearMemberVar() **********/
	void Table::ClearMemberVar(UWORD colNumber, bool setToNull)
	{
		exASSERT(colNumber < m_numCols);

		switch (m_colDefs[colNumber].m_sqlCtype)
		{
		case SQL_C_CHAR:
#ifdef SQL_C_WCHAR
		case SQL_C_WCHAR:
#endif
			//case SQL_C_WXCHAR:  SQL_C_WXCHAR is covered by either SQL_C_CHAR or SQL_C_WCHAR
			((UCHAR FAR *) m_colDefs[colNumber].m_ptrDataObj)[0] = 0;
			break;
		case SQL_C_SSHORT:
			*((SWORD *)m_colDefs[colNumber].m_ptrDataObj) = 0;
			break;
		case SQL_C_USHORT:
			*((UWORD*)m_colDefs[colNumber].m_ptrDataObj) = 0;
			break;
		case SQL_C_LONG:
		case SQL_C_SLONG:
			*((SDWORD *)m_colDefs[colNumber].m_ptrDataObj) = 0;
			break;
		case SQL_C_ULONG:
			*((UDWORD *)m_colDefs[colNumber].m_ptrDataObj) = 0;
			break;
		case SQL_C_FLOAT:
			*((SFLOAT *)m_colDefs[colNumber].m_ptrDataObj) = 0.0f;
			break;
		case SQL_C_DOUBLE:
			*((SDOUBLE *)m_colDefs[colNumber].m_ptrDataObj) = 0.0f;
			break;
		case SQL_C_TIMESTAMP:
			TIMESTAMP_STRUCT *pDt;
			pDt = (TIMESTAMP_STRUCT *)m_colDefs[colNumber].m_ptrDataObj;
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
			pDtd = (DATE_STRUCT *)m_colDefs[colNumber].m_ptrDataObj;
			pDtd->year = 0;
			pDtd->month = 0;
			pDtd->day = 0;
			break;
		case SQL_C_TIME:
			TIME_STRUCT *pDtt;
			pDtt = (TIME_STRUCT *)m_colDefs[colNumber].m_ptrDataObj;
			pDtt->hour = 0;
			pDtt->minute = 0;
			pDtt->second = 0;
			break;
		}

		if (setToNull)
			SetColNull(colNumber);
	}  // wxDbTable::ClearMemberVar()


	/********** wxDbTable::ClearMemberVars() **********/
	void Table::ClearMemberVars(bool setToNull)
	{
		size_t i;

		// Loop through the columns setting each member variable to zero
		for (i = 0; i < m_numCols; i++)
			ClearMemberVar((UWORD)i, setToNull);

	}  // wxDbTable::ClearMemberVars()


	/********** wxDbTable::SetQueryTimeout() **********/
	bool Table::SetQueryTimeout(UDWORD nSeconds)
	{
		if (SQLSetStmtOption(m_hstmtInsert, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
			return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert));
		if (SQLSetStmtOption(m_hstmtUpdate, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
			return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtUpdate));
		if (SQLSetStmtOption(m_hstmtDelete, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
			return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtDelete));
		if (SQLSetStmtOption(m_hstmtInternal, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
			return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInternal));

		// Completed Successfully
		return true;

	}  // wxDbTable::SetQueryTimeout()


	/********** wxDbTable::SetColDefs() **********/
	bool Table::SetColDefs(UWORD index, const std::wstring &fieldName, int dataType, void *pData,
		SWORD cType, int size, bool keyField, bool updateable,
		bool insertAllowed, bool derivedColumn)
	{
		std::wstring tmpStr;
		if (index >= m_numCols)  // Columns numbers are zero based....
		{
			tmpStr = (boost::wformat(L"Specified column index (%d) exceeds the maximum number of columns (%d) registered for this table definition.  Column definition not added.") % index % m_numCols).str();
			exFAIL_MSG(tmpStr.c_str());
			return false;
		}

		if (!m_colDefs)  // May happen if the database connection fails
			return false;

		if (fieldName.length() > (unsigned int)DB_MAX_COLUMN_NAME_LEN)
		{
			wcsncpy(m_colDefs[index].m_colName, fieldName.c_str(), DB_MAX_COLUMN_NAME_LEN);
			m_colDefs[index].m_colName[DB_MAX_COLUMN_NAME_LEN] = 0;  // Prevent buffer overrun

			tmpStr = (boost::wformat(L"Column name '%s' is too long. Truncated to '%s'.") % fieldName % m_colDefs[index].m_colName).str();
			exFAIL_MSG(tmpStr.c_str());
		}
		else
			wcscpy(m_colDefs[index].m_colName, fieldName.c_str());

		m_colDefs[index].m_dbDataType = dataType;
		m_colDefs[index].m_ptrDataObj = pData;
		m_colDefs[index].m_sqlCtype = cType;
		m_colDefs[index].m_szDataObj = size;  //TODO: glt ??? * sizeof(wchar_t) ???
		m_colDefs[index].m_keyField = keyField;
		m_colDefs[index].m_derivedCol = derivedColumn;
		// Derived columns by definition would NOT be "Insertable" or "Updateable"
		if (derivedColumn)
		{
			m_colDefs[index].m_updateable = false;
			m_colDefs[index].m_insertAllowed = false;
		}
		else
		{
			m_colDefs[index].m_updateable = updateable;
			m_colDefs[index].m_insertAllowed = insertAllowed;
		}

		m_colDefs[index].m_null = false;

		return true;

	}  // wxDbTable::SetColDefs()


	/********** wxDbTable::SetColDefs() **********/
	SColumnDataPtr* Table::SetColDefs(ColumnInfo *pColInfs, UWORD numCols)
	{
		exASSERT(pColInfs);
		SColumnDataPtr *pColDataPtrs = NULL;

		if (pColInfs)
		{
			UWORD index;

			pColDataPtrs = new SColumnDataPtr[numCols + 1];

			for (index = 0; index < numCols; index++)
			{
				// Process the fields
				switch (pColInfs[index].m_dbDataType)
				{
				case DB_DATA_TYPE_VARCHAR:
					pColDataPtrs[index].PtrDataObj = new wchar_t[pColInfs[index].m_bufferSize + (1 * sizeof(wchar_t))];
					pColDataPtrs[index].SzDataObj = pColInfs[index].m_bufferSize + (1 * sizeof(wchar_t));
					pColDataPtrs[index].SqlCtype = SQL_C_WXCHAR;
					break;
				case DB_DATA_TYPE_MEMO:
					pColDataPtrs[index].PtrDataObj = new wchar_t[pColInfs[index].m_bufferSize + (1 * sizeof(wchar_t))];
					pColDataPtrs[index].SzDataObj = pColInfs[index].m_bufferSize + (1 * sizeof(wchar_t));
					pColDataPtrs[index].SqlCtype = SQL_C_WXCHAR;
					break;
				case DB_DATA_TYPE_INTEGER:
					// Can be long or short
					if (pColInfs[index].m_bufferSize == sizeof(long))
					{
						pColDataPtrs[index].PtrDataObj = new long;
						pColDataPtrs[index].SzDataObj = sizeof(long);
						pColDataPtrs[index].SqlCtype = SQL_C_SLONG;
					}
					else
					{
						pColDataPtrs[index].PtrDataObj = new short;
						pColDataPtrs[index].SzDataObj = sizeof(short);
						pColDataPtrs[index].SqlCtype = SQL_C_SSHORT;
					}
					break;
				case DB_DATA_TYPE_FLOAT:
					// Can be float or double
					if (pColInfs[index].m_bufferSize == sizeof(float))
					{
						pColDataPtrs[index].PtrDataObj = new float;
						pColDataPtrs[index].SzDataObj = sizeof(float);
						pColDataPtrs[index].SqlCtype = SQL_C_FLOAT;
					}
					else
					{
						pColDataPtrs[index].PtrDataObj = new double;
						pColDataPtrs[index].SzDataObj = sizeof(double);
						pColDataPtrs[index].SqlCtype = SQL_C_DOUBLE;
					}
					break;
				case DB_DATA_TYPE_DATE:
					pColDataPtrs[index].PtrDataObj = new TIMESTAMP_STRUCT;
					pColDataPtrs[index].SzDataObj = sizeof(TIMESTAMP_STRUCT);
					pColDataPtrs[index].SqlCtype = SQL_C_TIMESTAMP;
					break;
				case DB_DATA_TYPE_BLOB:
					exFAIL_MSG(L"This form of ::SetColDefs() cannot be used with BLOB columns");
					pColDataPtrs[index].PtrDataObj = /*BLOB ADDITION NEEDED*/NULL;
					pColDataPtrs[index].SzDataObj = /*BLOB ADDITION NEEDED*/sizeof(void *);
					pColDataPtrs[index].SqlCtype = SQL_VARBINARY;
					break;
				}
				if (pColDataPtrs[index].PtrDataObj != NULL)
					SetColDefs(index, pColInfs[index].m_colName, pColInfs[index].m_dbDataType, pColDataPtrs[index].PtrDataObj, pColDataPtrs[index].SqlCtype, pColDataPtrs[index].SzDataObj);
				else
				{
					// Unable to build all the column definitions, as either one of
					// the calls to "new" failed above, or there was a BLOB field
					// to have a column definition for.  If BLOBs are to be used,
					// the other form of ::SetColDefs() must be used, as it is impossible
					// to know the maximum size to create the PtrDataObj to be.
					delete[] pColDataPtrs;
					return NULL;
				}
			}
		}

		return (pColDataPtrs);

	} // wxDbTable::SetColDefs()


	/********** wxDbTable::SetCursor() **********/
	void Table::SetCursor(HSTMT *hstmtActivate)
	{
		if (hstmtActivate == wxDB_DEFAULT_CURSOR)
			m_hstmt = *m_hstmtDefault;
		else
			m_hstmt = *hstmtActivate;

	}  // wxDbTable::SetCursor()


	///********** wxDbTable::Count(const std::wstring &) **********/
	//ULONG Table::Count(const std::wstring &args)
	//{
	//    ULONG count;
	//    std::wstring sqlStmt;
	//    SQLLEN cb;
	//
	//    // Build a "SELECT COUNT(*) FROM queryTableName [WHERE whereClause]" SQL Statement
	//    sqlStmt  = L"SELECT COUNT(";
	//    sqlStmt += args;
	//    sqlStmt += L") FROM ";
	//    sqlStmt += m_pDb->SQLTableName(m_queryTableName.c_str());
	////    sqlStmt += queryTableName;
	//    if (m_from.length())
	//        sqlStmt += m_from;
	//
	//    // Add the where clause if one is provided
	//    if (m_where.length())
	//    {
	//        sqlStmt += L" WHERE ";
	//        sqlStmt += m_where;
	//    }
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//    // Initialize the Count cursor if it's not already initialized
	//    if (!m_hstmtCount)
	//    {
	//        m_hstmtCount = GetNewCursor(false,false);
	//        exASSERT(m_hstmtCount);
	//        if (!m_hstmtCount)
	//            return(0);
	//    }
	//
	//    // Execute the SQL statement
	//    if (SQLExecDirect(*m_hstmtCount, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS) != SQL_SUCCESS)
	//    {
	//        m_pDb->DispAllErrors(m_henv, m_hdbc, *m_hstmtCount);
	//        return(0);
	//    }
	//
	//    // Fetch the record
	//    if (SQLFetch(*m_hstmtCount) != SQL_SUCCESS)
	//    {
	//        m_pDb->DispAllErrors(m_henv, m_hdbc, *m_hstmtCount);
	//        return(0);
	//    }
	//
	//    // Obtain the result
	//    if (SQLGetData(*m_hstmtCount, (UWORD)1, SQL_C_ULONG, &count, sizeof(count), &cb) != SQL_SUCCESS)
	//    {
	//        m_pDb->DispAllErrors(m_henv, m_hdbc, *m_hstmtCount);
	//        return(0);
	//    }
	//
	//    // Free the cursor
	//    if (SQLFreeStmt(*m_hstmtCount, SQL_CLOSE) != SQL_SUCCESS)
	//        m_pDb->DispAllErrors(m_henv, m_hdbc, *m_hstmtCount);
	//
	//    // Return the record count
	//    return(count);
	//
	//}  // wxDbTable::Count()


	///********** wxDbTable::Refresh() **********/
	//bool Table::Refresh()
	//{
	//    bool result = true;
	//
	//    // Switch to the internal cursor so any active cursors are not corrupted
	//    HSTMT currCursor = GetCursor();
	//    m_hstmt = m_hstmtInternal;
	//    std::wstring saveWhere = m_where;
	//    std::wstring saveOrderBy = m_orderBy;
	//
	//    // Build a where clause to refetch the record with.  Try and use the
	//    // ROWID if it's available, ow use the key fields.
	//    std::wstring whereClause;
	//    whereClause.empty();
	//
	//    if (CanUpdateByROWID())
	//    {
	//        SQLLEN cb;
	//        wchar_t rowid[wxDB_ROWID_LEN+1];
	//
	//        // Get the ROWID value.  If not successful retreiving the ROWID,
	//        // simply fall down through the code and build the WHERE clause
	//        // based on the key fields.
	//        if (SQLGetData(m_hstmt, (UWORD)(m_numCols+1), SQL_C_WXCHAR, (UCHAR*) rowid, sizeof(rowid), &cb) == SQL_SUCCESS)
	//        {
	//            whereClause += m_pDb->SQLTableName(m_queryTableName.c_str());
	////            whereClause += queryTableName;
	//            whereClause += L".ROWID = '";
	//            whereClause += rowid;
	//            whereClause += L"'";
	//        }
	//    }
	//
	//    // If unable to use the ROWID, build a where clause from the keyfields
	//    if (whereClause.empty())
	//        BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS, m_queryTableName);
	//
	//    // Requery the record
	//    m_where = whereClause;
	//    m_orderBy.empty();
	//    if (!Query())
	//        result = false;
	//
	//    if (result && !GetNext())
	//        result = false;
	//
	//    // Switch back to original cursor
	//    SetCursor(&currCursor);
	//
	//    // Free the internal cursor
	//    if (SQLFreeStmt(m_hstmtInternal, SQL_CLOSE) != SQL_SUCCESS)
	//        m_pDb->DispAllErrors(m_henv, m_hdbc, m_hstmtInternal);
	//
	//    // Restore the original where and order by clauses
	//    m_where   = saveWhere;
	//    m_orderBy = saveOrderBy;
	//
	//    return(result);
	//
	//}  // wxDbTable::Refresh()


	/********** wxDbTable::SetColNull() **********/
	bool Table::SetColNull(UWORD colNumber, bool set)
	{
		if (colNumber < m_numCols)
		{
			m_colDefs[colNumber].m_null = set;
			if (set)  // Blank out the values in the member variable
				ClearMemberVar(colNumber, false);  // Must call with false here, or infinite recursion will happen

			setCbValueForColumn(colNumber);

			return true;
		}
		else
			return false;

	}  // wxDbTable::SetColNull()


	/********** wxDbTable::SetColNull() **********/
	bool Table::SetColNull(const std::wstring &colName, bool set)
	{
		size_t colNumber;
		for (colNumber = 0; colNumber < m_numCols; colNumber++)
		{
			if (!_wcsicmp(colName.c_str(), m_colDefs[colNumber].m_colName))
				break;
		}

		if (colNumber < m_numCols)
		{
			m_colDefs[colNumber].m_null = set;
			if (set)  // Blank out the values in the member variable
				ClearMemberVar((UWORD)colNumber, false);  // Must call with false here, or infinite recursion will happen

			setCbValueForColumn(colNumber);

			return true;
		}
		else
			return false;

	}  // wxDbTable::SetColNull()


	/********** wxDbTable::GetNewCursor() **********/
	HSTMT *Table::GetNewCursor(bool setCursor, bool bindColumns)
	{
		HSTMT *newHSTMT = new HSTMT;
		exASSERT(newHSTMT);
		if (!newHSTMT)
			return(0);

		if (SQLAllocStmt(m_hdbc, newHSTMT) != SQL_SUCCESS)
		{
			m_pDb->DispAllErrors(NULL, m_hdbc);
			delete newHSTMT;
			return(0);
		}

		if (SQLSetStmtOption(*newHSTMT, SQL_CURSOR_TYPE, m_cursorType) != SQL_SUCCESS)
		{
			m_pDb->DispAllErrors(NULL, m_hdbc, *newHSTMT);
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
	bool Table::DeleteCursor(HSTMT *hstmtDel)
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
			m_pDb->DispAllErrors(NULL, m_hdbc);
			result = false;
		}

		delete hstmtDel;

		return(result);

	}  // wxDbTable::DeleteCursor()



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

}